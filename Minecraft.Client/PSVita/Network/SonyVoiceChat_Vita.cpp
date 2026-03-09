#include "stdafx.h"
#include <libsysmodule.h>
#include <rudp.h>
#include <audioin.h>
#include <audioout.h>
#include <fios2.h>

#include "SonyVoiceChat_Vita.h"

std::vector<SQRVoiceConnection*>	SonyVoiceChat_Vita::m_remoteConnections;
bool								SonyVoiceChat_Vita::m_bVoiceStarted = false;
int									SonyVoiceChat_Vita::m_numLocalDevicesConnected = 0; 
SQRLocalVoiceDevice					SonyVoiceChat_Vita::m_localVoiceDevices[MAX_LOCAL_PLAYER_COUNT];
uint32_t							SonyVoiceChat_Vita::m_voiceOutPort;
bool								SonyVoiceChat_Vita::m_forceSendPacket = false; // force a packet across the network, even if there's no data, so we can update flags
RingBuffer							SonyVoiceChat_Vita::m_recordRingBuffer(sc_ringBufferSize);
VoicePacket::Flags					SonyVoiceChat_Vita::m_localPlayerFlags[MAX_LOCAL_PLAYER_COUNT];
bool								SonyVoiceChat_Vita::m_bInitialised = false;
CRITICAL_SECTION					SonyVoiceChat_Vita::m_csRemoteConnections;

// sample related variables
SceVoiceStartParam  startParam;
int32_t		 playSize = 0;

static const int sc_thresholdValue = 100;

static const bool sc_verbose = false;

// #define _USE_PCM_AUDIO_
//#define LOOPBACK_TEST



int g_loadedPCMVoiceDataSizes[4];
int g_loadedPCMVoiceDataPos[4];
char* g_loadedPCMVoiceData[4];

static void CreatePort(uint32_t *portId, const SceVoicePortParam *pArg)
{
// 	C4JThread::PushAffinityAllCores();	// PS4 only

	int err = sceVoiceCreatePort( portId, pArg );
	assert(err == SCE_OK);
	assert(*portId != SCE_VOICE_INVALID_PORT_ID);
// 	C4JThread::PopAffinity();	// PS4 only
}

static void DeletePort(uint32_t& port)
{
	int32_t result;
	if (port != SCE_VOICE_INVALID_PORT_ID)
	{
		result = sceVoiceDeletePort( port );
		if (result != SCE_OK)
		{
			app.DebugPrintf("sceVoiceDeletePort failed %0x\n", result);
			assert(0);
		}
		port = SCE_VOICE_INVALID_PORT_ID;
	}
}


void LoadPCMVoiceData()
{
	for(int i=0;i<4;i++)
	{
		char filename[64];
		sprintf(filename, "voice%d.pcm", i+1);
		HANDLE file = CreateFile(filename, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		DWORD dwHigh=0;
		g_loadedPCMVoiceDataSizes[i] = GetFileSize(file,&dwHigh);

		if(g_loadedPCMVoiceDataSizes[i]!=0)
		{
			g_loadedPCMVoiceData[i] = new char[g_loadedPCMVoiceDataSizes[i]];
			DWORD bytesRead;
			BOOL bSuccess = ReadFile(file, g_loadedPCMVoiceData[i], g_loadedPCMVoiceDataSizes[i], &bytesRead, nullptr);
			assert(bSuccess);
		}
		g_loadedPCMVoiceDataPos[i] = 0;
	}
}


void SonyVoiceChat_Vita::init()
{
	int returnCode = SCE_OK;

	returnCode = sceSysmoduleLoadModule(SCE_SYSMODULE_VOICE);
	if (returnCode < 0) 
	{
		app.DebugPrintf("Error: sceSysmoduleLoadModule(SCE_SYSMODULE_VOICE), ret 0x%08x\n", returnCode);
		assert(0);
	}


	SceVoiceInitParam           params;
	SceVoicePortParam           portArgs;
	memset( &params, 0, sizeof(params) );
	params.appType = SCEVOICE_APPTYPE_GAME;
	params.onEvent = 0;
	returnCode = sceVoiceInit( &params , SCEVOICE_VERSION_100);
	if (returnCode < 0) 
	{
		app.DebugPrintf("Error: sceVoiceInit(), ret 0x%08x\n", returnCode);
		assert(0);
	}

#ifdef _USE_PCM_AUDIO_
	portArgs.portType                  = SCEVOICE_PORTTYPE_OUT_PCMAUDIO;
	portArgs.bMute                     = false;
	portArgs.threshold                 = 0;
	portArgs.volume                    = 1.0f;
	portArgs.pcmaudio.format.dataType = SCEVOICE_PCM_SHORT_LITTLE_ENDIAN;
	portArgs.pcmaudio.format.sampleRate = SCE_VOICE_SAMPLINGRATE_16000;
	portArgs.pcmaudio.bufSize			= 4096;
#else
	portArgs.portType                  = SCEVOICE_PORTTYPE_OUT_VOICE;
	portArgs.bMute                     = false;
	portArgs.threshold                 = 0;
	portArgs.volume                    = 1.0f;
	portArgs.voice.bitrate             = VOICE_ENCODED_FORMAT;
#endif
	CreatePort( &m_voiceOutPort, &portArgs );

	start();
	m_bInitialised = true;
#ifdef LOOPBACK_TEST
	// 	LoadPCMVoiceData();
	initLocalPlayer(0);
	connectPorts(m_localVoiceDevices[0].m_microphonePort, m_localVoiceDevices[0].m_headsetPort);
	//	SQRVoiceConnection* pConnection = addRemoteConnection(0, 0);
	//	connectPlayer(pConnection, 0);
#endif
	InitializeCriticalSection(&m_csRemoteConnections);
}

void SonyVoiceChat_Vita::shutdown()
{
	m_bInitialised = false;
	int32_t result;

	DeletePort( m_voiceOutPort);
	result = sceVoiceStop();
	assert(result == SCE_OK);
	result = sceVoiceEnd();
	assert(result == SCE_OK);

	m_bVoiceStarted=false;
	sceKernelFreeMemBlock(startParam.container);

	int returnCode = sceSysmoduleUnloadModule(SCE_SYSMODULE_VOICE);
	if (returnCode < 0) 
	{
		app.DebugPrintf("Error: sceSysmoduleUnloadModule(SCE_SYSMODULE_VOICE), ret 0x%08x\n", returnCode);
		assert(0);
	}

	DeleteCriticalSection(&m_csRemoteConnections);
}


void SonyVoiceChat_Vita::start()
{
	if( m_bVoiceStarted == false)
	{
		startParam.container = sceKernelAllocMemBlock("SceUserVoiceEvent", SCE_KERNEL_MEMBLOCK_TYPE_USER_RWDATA, SCE_VOICE_MEMORY_CONTAINER_SIZE, SCE_NULL);
		int err;

// 		C4JThread::PushAffinityAllCores(); // PS4 only
		err = sceVoiceStart(&startParam);
		assert(err == SCE_OK);
// 		C4JThread::PopAffinity();	// PS4 only

		m_bVoiceStarted = true;
	}
}

void SonyVoiceChat_Vita::checkFinished()
{
	EnterCriticalSection(&m_csRemoteConnections);

	for(int i=0;i<m_remoteConnections.size();i++)
	{
		if(!m_remoteConnections[i]->m_bFlaggedForShutdown)
			m_remoteConnections[i]->m_bFlaggedForShutdown = true;
	}
// 	assert(m_numLocalDevicesConnected == 0);

	LeaveCriticalSection(&m_csRemoteConnections);
}

void SonyVoiceChat_Vita::setEnabled( bool bEnabled )
{
}



// Internal send function. This attempts to send as many elements in the queue as possible until the write function tells us that we can't send any more. This way,
// we are guaranteed that if there *is* anything more in the queue left to send, we'll get a CELL_RUDP_CONTEXT_EVENT_WRITABLE event when whatever we've managed to
// send here is complete, and can continue on.
void SQRVoiceConnection::SendMoreInternal()
{
	bool keepSending;
	do
	{
		EnterCriticalSection(&m_csQueue);
		keepSending = false;
		if( m_sendQueue.size() > 0)
		{
			// Attempt to send the full data in the first element in our queue
			unsigned char *data= m_sendQueue.front().current;
			int dataSize = m_sendQueue.front().end - m_sendQueue.front().current;
			int ret = sceRudpWrite( m_rudpCtx, data, dataSize, 0);//CELL_RUDP_MSG_LATENCY_CRITICAL  );
			int wouldBlockFlag =  SCE_RUDP_ERROR_WOULDBLOCK;

			if( ret == dataSize )
			{
				// Fully sent, remove from queue - will loop in the while loop to see if there's anything else in the queue we could send
				delete [] m_sendQueue.front().start;
				m_sendQueue.pop();
				if( m_sendQueue.size() )
				{
					keepSending = true;
				}
			} 
			else if( ( ret >= 0 ) || ( ret == wouldBlockFlag ) )
			{


				// Things left to send - adjust this element in the queue
				int	remainingBytes;
				if( ret >= 0 )
				{
					// Only ret bytes sent so far
					remainingBytes = dataSize - ret;
					assert(remainingBytes > 0 );
				}
				else
				{
					// Is CELL_RUDP_ERROR_WOULDBLOCK, nothing has yet been sent
					remainingBytes = dataSize;
				}
				m_sendQueue.front().current = m_sendQueue.front().end - remainingBytes;	
			}
		}
		LeaveCriticalSection(&m_csQueue);
	} while (keepSending);
}

void SQRVoiceConnection::SendInternal(const void *data, unsigned int dataSize)
{
 	EnterCriticalSection(&m_csQueue);

	QueuedSendBlock sendBlock;

	unsigned char *dataCurrent = (unsigned char *)data;
	unsigned int dataRemaining = dataSize;

	while( dataRemaining )
	{
		int dataSize = dataRemaining;
		if( dataSize > SNP_MAX_PAYLOAD ) dataSize = SNP_MAX_PAYLOAD;
		sendBlock.start = new unsigned char [dataSize];
		sendBlock.end = sendBlock.start + dataSize;
		sendBlock.current = sendBlock.start;
		memcpy( sendBlock.start, dataCurrent, dataSize);
		m_sendQueue.push(sendBlock);		
		dataRemaining -= dataSize;
		dataCurrent += dataSize;
	}

//	app.DebugPrintf("voice sent %d bytes\n", dataSize);

	// Now try and send as much as we can
	SendMoreInternal();

 	LeaveCriticalSection(&m_csQueue);
}

void SQRVoiceConnection::readRemoteData()
{
	unsigned int dataSize = sceRudpGetSizeReadable(m_rudpCtx);
	if( dataSize > 0 )
	{
		VoicePacket packet;
		unsigned int bytesRead = sceRudpRead( m_rudpCtx, &packet, dataSize, 0, nullptr );
		unsigned int writeSize;
		if( bytesRead > 0 )
		{
//			app.DebugPrintf("voice received %d bytes\n", bytesRead);
			writeSize = bytesRead;
			if(packet.verifyData(bytesRead, 19))
				addPacket(packet);
// 			m_playRingBuffer.Write((char*)data, writeSize);

		}
	}

}



SQRVoiceConnection::SQRVoiceConnection( int rudpCtx, SceNpMatching2RoomMemberId remoteRoomMemberId ) 
	: m_rudpCtx(rudpCtx)
	, m_remoteRoomMemberId(remoteRoomMemberId)
	, m_bConnected(false)
	, m_headsetConnectionMask(0)
	, m_playRingBuffer(sc_ringBufferSize)
{
	InitializeCriticalSection(&m_csQueue);
	InitializeCriticalSection(&m_csPacketQueue);

	SceVoiceInitParam           params;
	SceVoicePortParam           portArgs;
#ifdef _USE_PCM_AUDIO_
	portArgs.portType                  = SCEVOICE_PORTTYPE_IN_PCMAUDIO;
	portArgs.bMute                     = false;
	portArgs.threshold                 = 100;
	portArgs.volume                    = 1.0f;
	portArgs.pcmaudio.format.sampleRate= SCEVOICE_SAMPLINGRATE_16000;
	portArgs.pcmaudio.format.dataType  = SCEVOICE_PCM_SHORT_LITTLE_ENDIAN;
	portArgs.pcmaudio.bufSize          = 4096;
#else
	portArgs.portType                  = SCEVOICE_PORTTYPE_IN_VOICE;
	portArgs.bMute                     = false;
	portArgs.threshold                 = sc_thresholdValue;     // compensate network jitter
	portArgs.volume                    = 1.0f;
	portArgs.voice.bitrate             = VOICE_ENCODED_FORMAT;
#endif
	CreatePort( &m_voiceInPort, &portArgs );
	m_nextExpectedFrameIndex = 0;
	m_bFlaggedForShutdown = false;
}

SQRVoiceConnection::~SQRVoiceConnection()
{
	DeleteCriticalSection(&m_csQueue);
	DeleteCriticalSection(&m_csPacketQueue);
	sceRudpTerminate( m_rudpCtx );
	app.DebugPrintf("-----------------------------\n");
	app.DebugPrintf("Voice rudp context deleted %d\n", m_rudpCtx);
	app.DebugPrintf("-----------------------------\n");

	DeletePort(m_voiceInPort);

}

bool SQRVoiceConnection::getNextPacket( VoicePacket& packet )
{
	EnterCriticalSection(&m_csPacketQueue); 
	bool retVal = false;
	if(m_receivedVoicePackets.size() > 0)
	{
		retVal = true;
		packet = m_receivedVoicePackets.front();
		m_receivedVoicePackets.pop();
	}
	LeaveCriticalSection(&m_csPacketQueue);
	return retVal;
}

void SQRVoiceConnection::addPacket( VoicePacket& packet )
{
	EnterCriticalSection(&m_csPacketQueue);
	m_receivedVoicePackets.push(packet);
	LeaveCriticalSection(&m_csPacketQueue);
}

int g_frameNum = 0;
bool g_bRecording = false;


uint32_t					frameSendIndex = 0;
uint32_t					lastReadFrameCnt = 0;


void PrintAllOutputVoiceStates( std::vector<SQRVoiceConnection*>& connections)
{
	for(size_t rIdx=0;rIdx<connections.size(); rIdx++)
	{
		SQRVoiceConnection* pVoice = connections[rIdx];
		SceVoiceBasePortInfo        portInfo;
		int result = sceVoiceGetPortInfo(pVoice->m_voiceInPort, &portInfo );
		static SceVoicePortState lastPortState = SCEVOICE_PORTSTATE_IDLE;
		if(portInfo.state != lastPortState)
		{
			lastPortState = portInfo.state;
			switch(portInfo.state)
			{
			case SCEVOICE_PORTSTATE_IDLE:
				app.DebugPrintf(" ----- SCE_VOICE_PORTSTATE_IDLE\n");
				break;
			case SCEVOICE_PORTSTATE_BUFFERING:
				app.DebugPrintf(" ----- SCE_VOICE_PORTSTATE_BUFFERING\n");
				break;
			case SCEVOICE_PORTSTATE_RUNNING:
				app.DebugPrintf(" ----- SCE_VOICE_PORTSTATE_RUNNING\n");
				break;
			case SCEVOICE_PORTSTATE_READY:
				app.DebugPrintf(" ----- SCE_VOICE_PORTSTATE_READY\n");
				break;
			case SCEVOICE_PORTSTATE_NULL:
			default:
				app.DebugPrintf(" ----- SCE_VOICE_PORTSTATE_NULL\n");
				break;
			}
		}
	}

}


void SonyVoiceChat_Vita::sendPCMMicData()
{
	int32_t                     result;
	uint32_t                    outputPortBytes;
	VoicePacket					packetToSend;
	uint32_t					readSize;
	SceVoiceBasePortInfo        portInfo;
	memset( &portInfo, 0, sizeof(portInfo) );
	uint16_t					frameGap = 0;

	DWORD tick = GetTickCount();
	static DWORD lastTick = 0;
	int numFrames = ceilf((tick - lastTick)/16.0f);
	lastTick = tick;
	readSize = 512 * numFrames;

	if(g_loadedPCMVoiceDataPos[0] + readSize < g_loadedPCMVoiceDataSizes[0])
	{
		for(int i=0;i<MAX_LOCAL_PLAYER_COUNT;i++)
		{
			if(m_localVoiceDevices[i].isValid())
			{
				result = sceVoiceWriteToIPort(m_localVoiceDevices[i].m_microphonePort, &g_loadedPCMVoiceData[0][g_loadedPCMVoiceDataPos[0]], &readSize, 0);
			}
		}
	}
	g_loadedPCMVoiceDataPos[0] += readSize;
	if(g_loadedPCMVoiceDataPos[0] > (g_loadedPCMVoiceDataSizes[0] + 8192))
		g_loadedPCMVoiceDataPos[0] = 0;


}

void SonyVoiceChat_Vita::sendAllVoiceData()
{
	int32_t                     result;
	uint32_t                    outputPortBytes;
	VoicePacket					packetToSend;
	uint32_t					readSize;
	SceVoiceBasePortInfo        portInfo;
	memset( &portInfo, 0, sizeof(portInfo) );
	uint16_t					frameGap = 0;

	VoicePacket::Flags lastPlayerFlags[MAX_LOCAL_PLAYER_COUNT];

	for(int i=0; i<MAX_LOCAL_PLAYER_COUNT;i++)
		lastPlayerFlags[i] = m_localPlayerFlags[i];

	bool flagsChanged = false;


	
	// grab the status of all the local voices
	for(int i=0; i<MAX_LOCAL_PLAYER_COUNT;i++)
	{
		if(m_localVoiceDevices[i].isValid())
		{
			bool bChatRestricted = false;
			ProfileManager.GetChatAndContentRestrictions(i,true,&bChatRestricted,nullptr,nullptr);

			if(bChatRestricted)
			{
				m_localPlayerFlags[i].m_bHasMicConnected = false;
			}
			else
			{
				m_localPlayerFlags[i].m_bHasMicConnected = true;
			}
	


			SceVoiceBasePortInfo        portInfo;
			int result = sceVoiceGetPortInfo(m_localVoiceDevices[i].m_microphonePort, &portInfo );
			assert(result == SCE_OK);
			switch(portInfo.state)
			{
			case SCEVOICE_PORTSTATE_READY:
			case SCEVOICE_PORTSTATE_BUFFERING:
			case SCEVOICE_PORTSTATE_IDLE:
				m_localPlayerFlags[i].m_bTalking = false;
				break;
			case SCEVOICE_PORTSTATE_RUNNING:
				m_localPlayerFlags[i].m_bTalking = true;
				break;
			default:
				assert(0);
			}
		}
		else
		{
			m_localPlayerFlags[i].m_bHasMicConnected = false;
			m_localPlayerFlags[i].m_bTalking = false;
		}		
		packetToSend.m_localPlayerFlags[i] = m_localPlayerFlags[i];
		if(m_localPlayerFlags[i].m_bHasMicConnected != lastPlayerFlags[i].m_bHasMicConnected ||
			m_localPlayerFlags[i].m_bTalking != lastPlayerFlags[i].m_bTalking)
			flagsChanged = true;
	}


	if(sc_verbose)
	{
		EnterCriticalSection(&m_csRemoteConnections);
		PrintAllOutputVoiceStates(m_remoteConnections);
		LeaveCriticalSection(&m_csRemoteConnections);
	}
	result = sceVoiceGetPortInfo(m_voiceOutPort, &portInfo );
	if (result != SCE_OK)
	{
		app.DebugPrintf("sceVoiceGetPortInfo failed %x\n", result);
		assert(0);
	}




	outputPortBytes = portInfo.numByte;
	outputPortBytes = (outputPortBytes>sizeof(packetToSend.m_data))?sizeof(packetToSend.m_data):outputPortBytes;
	if( outputPortBytes || flagsChanged || m_forceSendPacket)
	{
		frameSendIndex += lastReadFrameCnt;
		if(outputPortBytes)
		{
			readSize = outputPortBytes;
			result = sceVoiceReadFromOPort(m_voiceOutPort, packetToSend.m_data, &readSize );
			if (result != SCE_OK)
			{
				app.DebugPrintf("sceVoiceReadFromOPort failed %0x\n", result);
				assert(0);
				return;
			}
			lastReadFrameCnt = readSize/portInfo.frameSize;
			assert(readSize%portInfo.frameSize == 0);

			packetToSend.m_numFrames = lastReadFrameCnt;
			packetToSend.m_frameSendIndex = frameSendIndex;
			packetToSend.setChecksum(readSize);
		}
		else
		{
			readSize = 0;
			packetToSend.m_numFrames = 0;
			packetToSend.m_frameSendIndex = frameSendIndex;
			packetToSend.setChecksum(readSize);

		}

					
		int packetSize = packetToSend.getPacketSize(readSize);

		EnterCriticalSection(&m_csRemoteConnections);

		// send this packet out to all our remote connections
		for(size_t rIdx=0;rIdx<m_remoteConnections.size(); rIdx++)
		{
			SQRVoiceConnection* pVoice = m_remoteConnections[rIdx];
			if(pVoice->m_bConnected)
 				m_remoteConnections[rIdx]->SendInternal(&packetToSend, packetSize);
		}

		LeaveCriticalSection(&m_csRemoteConnections);
	}
	m_forceSendPacket = false; 
}

bool g_bPlaying = false;

void SonyVoiceChat_Vita::playAllReceivedData()
{
	EnterCriticalSection(&m_csRemoteConnections);
	// write all the incoming data from the network to each of the input voices
	for(int rIdx=0;rIdx<m_remoteConnections.size(); rIdx++)
	{
		SQRVoiceConnection* pVoice = m_remoteConnections[rIdx];
		VoicePacket packet;
		while(pVoice->getNextPacket(packet))	// MGH -  changed to a while loop, so all the packets are sent to the voice port, and it can handle delayed packets due to the size of it's internal buffer
		{
			int frameGap;
			if (pVoice->m_nextExpectedFrameIndex == packet.m_frameSendIndex)
			{
				// no voice frame drop, continuous frames
				frameGap = 0;
				if(sc_verbose)
					app.DebugPrintf("index@%d gets expected frame\n",pVoice->m_nextExpectedFrameIndex);
				pVoice->m_nextExpectedFrameIndex = packet.m_frameSendIndex + packet.m_numFrames;
			}
			else if (pVoice->m_nextExpectedFrameIndex < packet.m_frameSendIndex)
			{
				// has voice frame drop, dropped forwarding frames
				frameGap = packet.m_frameSendIndex - pVoice->m_nextExpectedFrameIndex;
				if(sc_verbose)
					app.DebugPrintf("index@%d gets dropped forwarding frames %d\n",pVoice->m_nextExpectedFrameIndex, frameGap);
				pVoice->m_nextExpectedFrameIndex = packet.m_frameSendIndex + packet.m_numFrames;
			}
			else if (pVoice->m_nextExpectedFrameIndex > packet.m_frameSendIndex)
			{
				// has voice frame drop, dropped preceding frames, no reset on pVoice->m_nextExpectedFrameIndex
				frameGap = packet.m_frameSendIndex - pVoice->m_nextExpectedFrameIndex;
				if(sc_verbose)
					app.DebugPrintf("index@%d gets dropped forwarding frames %d\n", pVoice->m_nextExpectedFrameIndex, frameGap);
			}

			SceVoiceBasePortInfo        portInfo;
			int result = sceVoiceGetPortInfo(pVoice->m_voiceInPort, &portInfo );
			if (result != SCE_OK)
			{
				if(sc_verbose)
					app.DebugPrintf("sceVoiceGetPortInfo LoopbackVoiceInPort failed %x\n", result);
				assert(0);
				LeaveCriticalSection(&m_csRemoteConnections);
				return;
			}
			uint32_t writeSize = packet.m_numFrames * portInfo.frameSize;
			int inputPortBytes = portInfo.numByte;
			inputPortBytes = (inputPortBytes>writeSize)?writeSize:inputPortBytes;
			writeSize = inputPortBytes;
			result = sceVoiceWriteToIPort(pVoice->m_voiceInPort, packet.m_data, &writeSize, frameGap);
			if (result != SCE_OK)
			{
				if(sc_verbose)
					app.DebugPrintf("sceVoiceWriteToIPort failed %0x\n", result);
				assert(0);
				LeaveCriticalSection(&m_csRemoteConnections);
				return;
			}
			if (writeSize != inputPortBytes)
			{
				// libvoice internal voice in port buffer fulls
				if(sc_verbose)
					app.DebugPrintf("internal voice in port buffer fulls. \n");
			}
			packet.m_numFrames = 0;

			// copy the flags
			for(int flagIndex=0;flagIndex<MAX_LOCAL_PLAYER_COUNT;flagIndex++)
				pVoice->m_remotePlayerFlags[flagIndex] = packet.m_localPlayerFlags[flagIndex];
		}
	}
	LeaveCriticalSection(&m_csRemoteConnections);

}

void SonyVoiceChat_Vita::tick()
{
	if(m_bInitialised)
	{
// 		DWORD tick = GetTickCount();
// 		static DWORD lastTick = 0;
// 		app.DebugPrintf("Time since last voice tick : %d ms\n", tick - lastTick);
// 		lastTick = tick;
		g_frameNum++;
		sendAllVoiceData();
		playAllReceivedData();
		
		EnterCriticalSection(&m_csRemoteConnections);
		
		for(int i = (int)m_remoteConnections.size() - 1; i >= 0; i--)
		{
			if(m_remoteConnections[i]->m_bFlaggedForShutdown)
			{
				delete m_remoteConnections[i];
				m_remoteConnections.erase(m_remoteConnections.begin() + i);
			}
		}

		LeaveCriticalSection(&m_csRemoteConnections);

	}
}



bool SonyVoiceChat_Vita::hasMicConnected(SQRNetworkPlayer* pNetPlayer)
{
	if(CGameNetworkManager::usingAdhocMode())  // no voice chat in adhoc
		return false;

	if(pNetPlayer->IsLocal())
	{
		return m_localPlayerFlags[pNetPlayer->GetLocalPlayerIndex()].m_bHasMicConnected;
	}
	else
	{
		EnterCriticalSection(&m_csRemoteConnections);
		for(int i=0;i<m_remoteConnections.size();i++)
		{
			SQRVoiceConnection* pVoice = m_remoteConnections[i];
			if(pVoice->m_remoteRoomMemberId == pNetPlayer->m_roomMemberId)
			{
				bool bMicConnected = pVoice->m_remotePlayerFlags[pNetPlayer->GetLocalPlayerIndex()].m_bHasMicConnected;
				LeaveCriticalSection(&m_csRemoteConnections);
				return bMicConnected;
			}
		}
		LeaveCriticalSection(&m_csRemoteConnections);
	}
	// if we get here we've not found the player, panic!!
	assert(0);
	return false;
}

void SonyVoiceChat_Vita::mute( bool bMute )
{
}

void SonyVoiceChat_Vita::mutePlayer( const SceNpMatching2RoomMemberId member_id, bool bMute ) /*Turn chat audio from a specified player on or off */
{
}

void SonyVoiceChat_Vita::muteLocalPlayer( bool bMute ) /*Turn microphone input on or off */
{
}

bool SonyVoiceChat_Vita::isMuted()
{
	return false;
}

bool SonyVoiceChat_Vita::isMutedPlayer( const PlayerUID& memberUID) 
{
 	return false;
}

bool SonyVoiceChat_Vita::isMutedLocalPlayer() 
{
 	return false;
}


bool SonyVoiceChat_Vita::isTalking(SQRNetworkPlayer* pNetPlayer)
{
	if(CGameNetworkManager::usingAdhocMode())  // no voice chat in adhoc
		return false;

	if(pNetPlayer->IsLocal())
	{
		return m_localPlayerFlags[pNetPlayer->GetLocalPlayerIndex()].m_bTalking;
	}
	else
	{
		EnterCriticalSection(&m_csRemoteConnections);
		for(int i=0;i<m_remoteConnections.size();i++)
		{
			SQRVoiceConnection* pVoice = m_remoteConnections[i];
			if(pVoice->m_remoteRoomMemberId == pNetPlayer->m_roomMemberId)
			{
				bool bTalking = pVoice->m_remotePlayerFlags[pNetPlayer->GetLocalPlayerIndex()].m_bTalking;
				LeaveCriticalSection(&m_csRemoteConnections);
				return bTalking;
			}
		}
		LeaveCriticalSection(&m_csRemoteConnections);
	}
	// if we get here we've not found the player, panic!!
	assert(0);
	return false;
}


void SQRLocalVoiceDevice::init(bool bChatRestricted)
{
	SceVoiceInitParam           params;
	SceVoicePortParam           portArgs;

	int returnCode = 0;
	m_bChatRestricted = bChatRestricted;

	portArgs.portType                  = SCEVOICE_PORTTYPE_IN_DEVICE;
	portArgs.bMute                     = false;
	portArgs.threshold                 = 0;
	portArgs.volume                    = 1.0f;
	portArgs.device.playerId		   = 0;
// 	portArgs.device.type			   = SCE_AUDIO_IN_TYPE_VOICE;
// 	portArgs.device.index			   = 0;
	CreatePort( &m_microphonePort, &portArgs );
// 	m_micAudioDevicePort = sceAudioInOpen(localUserID, 
// 		SCE_AUDIO_IN_TYPE_VOICE, 0,
// 		SCE_AUDIO_IN_GRAIN_DEFAULT, 
// 		SCE_AUDIO_IN_FREQ_DEFAULT,
// 		SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);
// 	assert(m_micAudioDevicePort >= 0);

	portArgs.portType                  = SCEVOICE_PORTTYPE_OUT_DEVICE;
	portArgs.bMute                     = false;
	portArgs.threshold                 = 0;
	portArgs.volume                    = 1.0f;
	portArgs.device.playerId		   = 0;
// 	portArgs.device.type			   = SCE_AUDIO_OUT_PORT_TYPE_VOICE;
// 	portArgs.device.index			   = 0;
	CreatePort( &m_headsetPort, &portArgs );

	m_bValid = true;

}



void SQRLocalVoiceDevice::shutdown()
{

	assert(isValid());
 	m_bValid = false;
	DeletePort(m_microphonePort);
	DeletePort(m_headsetPort);
// 	int err = sceAudioInClose(m_micAudioDevicePort);
// 	assert(err == SCE_OK);
// 	m_micAudioDevicePort = -1;
}



SQRVoiceConnection* SonyVoiceChat_Vita::addRemoteConnection( int RudpCxt, SceNpMatching2RoomMemberId peerMemberId)
{
	EnterCriticalSection(&m_csRemoteConnections);
	SQRVoiceConnection* pConn = new SQRVoiceConnection(RudpCxt, peerMemberId);
	m_remoteConnections.push_back(pConn);
	m_forceSendPacket = true;  // new connection, so we'll force a packet through for the flags
	LeaveCriticalSection(&m_csRemoteConnections);
	
	return pConn;
}

void SonyVoiceChat_Vita::connectPorts(uint32_t inPort, uint32_t outPort)
{
	int returnCode = sceVoiceConnectIPortToOPort(inPort, outPort);
	if (returnCode != SCE_OK )
	{
		app.DebugPrintf("sceVoiceConnectIPortToOPort failed (0x%08x), inPort 0x%08x, outPort 0x%08x\n", returnCode, inPort, outPort);
		assert(0);
	}
}
void SonyVoiceChat_Vita::disconnectPorts(uint32_t inPort, uint32_t outPort)
{
	int returnCode = sceVoiceDisconnectIPortFromOPort(inPort, outPort);
	if (returnCode != SCE_OK )
	{
		app.DebugPrintf("sceVoiceDisconnectIPortFromOPort failed (0x%08x), inPort 0x%08x, outPort 0x%08x\n", returnCode, inPort, outPort);
		assert(0);
	}
}


void SonyVoiceChat_Vita::makeLocalConnections()
{
	// connect all mics to other devices headsets, for local chat 
	for(int i=0;i<MAX_LOCAL_PLAYER_COUNT;i++)
	{
		SQRLocalVoiceDevice* pConnectFrom = &m_localVoiceDevices[i];
		if(pConnectFrom->isValid())
		{
			for(int j=0;j<MAX_LOCAL_PLAYER_COUNT;j++)
			{
				SQRLocalVoiceDevice* pConnectTo = &m_localVoiceDevices[j];
				if( (pConnectFrom!=pConnectTo) && pConnectTo->isValid())
				{
					if(pConnectFrom->m_localConnections[j] == false)
					{
						if(pConnectTo->m_bChatRestricted == false && pConnectFrom->m_bChatRestricted == false)
						{
							connectPorts(pConnectFrom->m_microphonePort, pConnectTo->m_headsetPort);
							pConnectFrom->m_localConnections[j] = true;
						}
					}
				}
			}
		}
	}
}

void SonyVoiceChat_Vita::breakLocalConnections(int playerIdx)
{
	// break any connections with devices that are no longer valid
	for(int i=0;i<MAX_LOCAL_PLAYER_COUNT;i++)
	{
		SQRLocalVoiceDevice* pConnectedFrom = &m_localVoiceDevices[i];
		for(int j=0;j<MAX_LOCAL_PLAYER_COUNT;j++)
		{
			if(pConnectedFrom->m_localConnections[j] == true)
			{
				SQRLocalVoiceDevice* pConnectedTo = &m_localVoiceDevices[j];
				if(i==playerIdx || j==playerIdx)
				{
					if(pConnectedTo->m_bChatRestricted == false && pConnectedFrom->m_bChatRestricted == false)
					{
						disconnectPorts(pConnectedFrom->m_microphonePort, pConnectedTo->m_headsetPort);
						pConnectedFrom->m_localConnections[j] = false;
					}
				}
			}
		}
	}
}


void SonyVoiceChat_Vita::initLocalPlayer(int playerIndex)
{
	if(m_localVoiceDevices[playerIndex].isValid() == false)
	{
		bool chatRestricted = false;
		ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,&chatRestricted,nullptr,nullptr);

		// create all device ports required
		m_localVoiceDevices[playerIndex].init(chatRestricted);
		m_numLocalDevicesConnected++;
		if(m_localVoiceDevices[playerIndex].m_bChatRestricted == false)
		{
			connectPorts(m_localVoiceDevices[playerIndex].m_microphonePort, m_voiceOutPort);
		}
		m_forceSendPacket = true;  // new local device, so we'll force a packet through for the flags

	}
	makeLocalConnections();
}

void SonyVoiceChat_Vita::connectPlayer(SQRVoiceConnection* pConnection, int playerIndex)
{
	if((pConnection->m_headsetConnectionMask & (1 << playerIndex)) == 0)
	{
		initLocalPlayer(playerIndex); // added this as we can get a client->client connection coming in first, and the network player hasn't been created yet (so this hasn't been initialised)
		if(m_localVoiceDevices[playerIndex].m_bChatRestricted == false)
		{
			connectPorts(pConnection->m_voiceInPort, m_localVoiceDevices[playerIndex].m_headsetPort);
		}
		pConnection->m_headsetConnectionMask |= (1 << playerIndex);
		app.DebugPrintf("Connecting player %d to rudp context %d\n", playerIndex, pConnection->m_rudpCtx);
		m_forceSendPacket = true;  // new connection, so we'll force a packet through for the flags
	}
}

SQRVoiceConnection* SonyVoiceChat_Vita::GetVoiceConnectionFromRudpCtx( int RudpCtx )
{
	for(int i=0;i<m_remoteConnections.size();i++)
	{
		if(m_remoteConnections[i]->m_rudpCtx == RudpCtx)
			return m_remoteConnections[i];
	}
	return nullptr;
}

void SonyVoiceChat_Vita::connectPlayerToAll( int playerIndex )
{
	EnterCriticalSection(&m_csRemoteConnections);

	for(int i=0;i<m_remoteConnections.size();i++)
	{
		SonyVoiceChat_Vita::connectPlayer(m_remoteConnections[i], playerIndex);
	}

	LeaveCriticalSection(&m_csRemoteConnections);
}

SQRVoiceConnection* SonyVoiceChat_Vita::getVoiceConnectionFromRoomMemberID( SceNpMatching2RoomMemberId roomMemberID )
{
	for(int i=0;i<m_remoteConnections.size();i++)
	{
		if(m_remoteConnections[i]->m_remoteRoomMemberId == roomMemberID)
		{
			return m_remoteConnections[i];
		}
	}

	return nullptr;
}

void SonyVoiceChat_Vita::disconnectLocalPlayer( int localIdx )
{
	if(m_localVoiceDevices[localIdx].isValid() == false)
		return;

	EnterCriticalSection(&m_csRemoteConnections);

	if(m_localVoiceDevices[localIdx].m_bChatRestricted == false)
	{
		disconnectPorts(m_localVoiceDevices[localIdx].m_microphonePort, m_voiceOutPort);

		for(int i=0;i<m_remoteConnections.size();i++)
		{
			disconnectPorts(m_remoteConnections[i]->m_voiceInPort, m_localVoiceDevices[localIdx].m_headsetPort);
			m_remoteConnections[i]->m_headsetConnectionMask &= (~(1 << localIdx));
			app.DebugPrintf("disconnecting player %d from rudp context %d\n", localIdx, m_remoteConnections[i]->m_rudpCtx);
		}
	}
	m_numLocalDevicesConnected--;

 	if(m_numLocalDevicesConnected == 0) // no more local players, kill all the remote connections
 	{
	for(size_t i=0;i<m_remoteConnections.size();i++)
 		{
 			delete m_remoteConnections[i];
 		}
 		m_remoteConnections.clear();
 	}

	LeaveCriticalSection(&m_csRemoteConnections);
		
	breakLocalConnections(localIdx);
	m_localVoiceDevices[localIdx].shutdown();
}


void SonyVoiceChat_Vita::disconnectRemoteConnection( SQRVoiceConnection* pVoice )
{
	EnterCriticalSection(&m_csRemoteConnections);

	int voiceIdx = -1;
	for(int i=0;i<m_remoteConnections.size();i++)
	{
		if(m_remoteConnections[i] == pVoice)
		voiceIdx = i;
	}
	assert(voiceIdx>=0);
	if(voiceIdx>=0)
	{
		m_remoteConnections[voiceIdx]->m_bFlaggedForShutdown = true;
	}

	LeaveCriticalSection(&m_csRemoteConnections);

}

void SonyVoiceChat_Vita::setConnected( int RudpCtx )
{
	SQRVoiceConnection* pVoice = GetVoiceConnectionFromRudpCtx(RudpCtx);
	if(pVoice)
	{
		pVoice->m_bConnected = true;
		m_forceSendPacket = true;
	}
	else
	{
		assert(false);
	}
}




RingBuffer::RingBuffer( int sizeBytes )
{
	buffer = new char[sizeBytes];
	buf_size = sizeBytes;
	buf_full = buf_free = 0;
}


int RingBuffer::Write( char* data, int len_ )
{
	if (len_ <= 0) return len_;
	unsigned int len = (unsigned int)len_;
	unsigned int data_size = buf_size - (buf_free - buf_full);
	if (len > data_size)
		len = data_size;
	data_size = buf_size - (buf_free % buf_size);
	if (data_size > len)
		data_size = len;
	memcpy(buffer + (buf_free % buf_size), data, data_size);
	if (data_size != len)
		memcpy(buffer, data + data_size, len - data_size);
	buf_free += len;
	return len;
}

int RingBuffer::Read( char* data, int max_bytes_ )
{
	if (max_bytes_ <= 0) return max_bytes_;
	unsigned int max_bytes = (unsigned int)max_bytes_;
	unsigned int result = buf_free - buf_full;
	if (result > max_bytes)
		result = max_bytes;
	unsigned int chunk = buf_size - (buf_full % buf_size);
	if (chunk > result)
		chunk = result;
	memcpy(data, buffer + (buf_full % buf_size), chunk);
	if (chunk != result)
		memcpy(data + chunk, buffer, result - chunk);
	buf_full += result;
	return result;
}
