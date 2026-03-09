#include "stdafx.h"
#include "SQRNetworkPlayer.h"

#ifdef __PS3__
#include <cell/rudp.h>
#include "PS3/Network/SonyVoiceChat.h"

#elif defined __ORBIS__
#include <rudp.h>
#include "Orbis/Network/SonyVoiceChat_Orbis.h"

#else // __PSVITA__
#include <rudp.h>
#include <adhoc_matching.h>
#include "PSVita/Network/SonyVoiceChat_Vita.h"

#endif

//#define PRINT_ACK_STATS

#ifdef __PS3__
static const int sc_wouldBlockFlag =  CELL_RUDP_ERROR_WOULDBLOCK;
#else // __ORBIS__
static const int sc_wouldBlockFlag =  SCE_RUDP_ERROR_WOULDBLOCK;
#endif



static const bool sc_verbose = false;

int	SQRNetworkPlayer::GetSmallId()
{
	return m_ISD.m_smallId;
}

wchar_t *SQRNetworkPlayer::GetName()
{
	return m_name;
}

bool SQRNetworkPlayer::IsRemote()
{
	return !IsLocal();
}

bool SQRNetworkPlayer::IsHost()
{
	return (m_type == SNP_TYPE_HOST);
}

bool SQRNetworkPlayer::IsLocal()
{
	// m_host determines whether this *machine* is hosting the game, not this player (which is determined by m_type)
	if( m_host )
	{
		// If we are the hosting machine, then both the host & local players are local to this machine
		return (m_type == SNP_TYPE_HOST) || (m_type == SNP_TYPE_LOCAL);
	}
	else
	{
		// Not hosting, just local players are actually physically local
		return (m_type == SNP_TYPE_LOCAL) ;
	}
}

int SQRNetworkPlayer::GetLocalPlayerIndex()
{
	return m_localPlayerIdx;
}

bool SQRNetworkPlayer::IsSameSystem(SQRNetworkPlayer *other)
{
	return (m_roomMemberId == other->m_roomMemberId);
}

uintptr_t	SQRNetworkPlayer::GetCustomDataValue()
{
	return m_customData;
}

void	SQRNetworkPlayer::SetCustomDataValue(uintptr_t data)
{
	m_customData = data;
}

SQRNetworkPlayer::SQRNetworkPlayer(SQRNetworkManager *manager, eSQRNetworkPlayerType playerType, bool onHost, SceNpMatching2RoomMemberId roomMemberId, int localPlayerIdx, int rudpCtx, PlayerUID *pUID)
{
	m_roomMemberId		= roomMemberId;
	m_localPlayerIdx	= localPlayerIdx;
	m_rudpCtx			= rudpCtx;
	m_flags				= 0;
	m_type				= playerType;
	m_host				= onHost;
	m_manager			= manager;
	m_customData		= 0;
	m_acksOutstanding	= 0;
	m_totalBytesInSendQueue = 0;
	if( pUID )
	{
		memcpy(&m_ISD.m_UID,pUID,sizeof(PlayerUID));
#ifdef __PSVITA__
		if(CGameNetworkManager::usingAdhocMode() && pUID->getOnlineID()[0] == 0)
		{
			assert(localPlayerIdx == 0);
			// player doesn't have an online UID, set it from the player name
			m_ISD.m_UID.setForAdhoc();
	}
#endif // __PSVITA__
	}
	else
	{
		memset(&m_ISD.m_UID,0,sizeof(PlayerUID));
	}
	SetNameFromUID();
	InitializeCriticalSection(&m_csQueue);
	InitializeCriticalSection(&m_csAcks);
#ifdef __ORBIS__
	if(IsLocal())
	{
		SonyVoiceChat_Orbis::initLocalPlayer(m_localPlayerIdx);
	}
#endif

#ifndef _CONTENT_PACKAGE
	m_minAckTime = INT_MAX;
	m_maxAckTime = 0;
	m_totalAcks = 0;
	m_totalAckTime = 0;
	m_averageAckTime = 0;
#endif

}

SQRNetworkPlayer::~SQRNetworkPlayer()
{
#ifdef __ORBIS__
	SQRNetworkManager_Orbis* pMan = (SQRNetworkManager_Orbis*)m_manager;
// 	pMan->removePlayerFromVoiceChat(this);
// 	m_roomMemberId = -1;
#endif
	DeleteCriticalSection(&m_csQueue);
}

bool SQRNetworkPlayer::IsReady()
{
	return ( ( m_flags & SNP_FLAG_READY_MASK ) == SNP_FLAG_READY_MASK );
}

PlayerUID SQRNetworkPlayer::GetUID()
{
	return m_ISD.m_UID;
}

void SQRNetworkPlayer::SetUID(PlayerUID UID)
{
	m_ISD.m_UID = UID;
	SetNameFromUID();
}

bool SQRNetworkPlayer::HasConnectionAndSmallId()
{
	const int reqFlags = ( SNP_FLAG_CONNECTION_COMPLETE | SNP_FLAG_SMALLID_ALLOCATED );
	return (( m_flags & reqFlags) == reqFlags);
}

void SQRNetworkPlayer::ConnectionComplete()
{
	m_host ? app.DebugPrintf(sc_verbose, "host : ") : app.DebugPrintf(sc_verbose, "client:");
	app.DebugPrintf(sc_verbose, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>    ConnectionComplete\n");
	m_flags |= SNP_FLAG_CONNECTION_COMPLETE;
}

void SQRNetworkPlayer::SmallIdAllocated(unsigned char smallId)
{
	m_ISD.m_smallId = smallId;
	m_flags |= SNP_FLAG_SMALLID_ALLOCATED;
	m_host ? app.DebugPrintf(sc_verbose, "host : ") : app.DebugPrintf(sc_verbose, "client:");
	app.DebugPrintf(sc_verbose, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>    Small ID allocated\n");


	// If this is a non-network sort of player then flag now as having its small id confirmed
	if( ( m_type == SNP_TYPE_HOST ) ||
		( m_host &&  ( m_type == SNP_TYPE_LOCAL ) ) ||
		( !m_host && ( m_type == SNP_TYPE_REMOTE ) ) )
	{
		m_host ? app.DebugPrintf(sc_verbose, "host : ") : app.DebugPrintf(sc_verbose, "client:");
		app.DebugPrintf(sc_verbose, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>    Small ID confirmed\n");

		m_flags |= SNP_FLAG_SMALLID_CONFIRMED;
	}
}

void SQRNetworkPlayer::InitialDataReceived(SQRNetworkPlayer::InitSendData *ISD)
{
	assert(m_ISD.m_smallId == ISD->m_smallId);
	memcpy(&m_ISD, ISD, sizeof(InitSendData) );
#ifdef __PSVITA__
	SetNameFromUID();
#endif
	m_host ? app.DebugPrintf(sc_verbose, "host : ") : app.DebugPrintf(sc_verbose, "client:");
	app.DebugPrintf(sc_verbose, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>    Small ID confirmed\n");
	m_flags |= SNP_FLAG_SMALLID_CONFIRMED;
}

bool SQRNetworkPlayer::HasSmallIdConfirmed()
{
	return ( m_flags & SNP_FLAG_SMALLID_CONFIRMED );
}

// To confirm to the host that we are ready, send a single byte with our small id.
void SQRNetworkPlayer::ConfirmReady()
{
	SendInternal(&m_ISD, sizeof(InitSendData), e_flag_AckNotRequested);

	// Final flag for a local player on the client, as we are now safe to send data on to the host
	m_host ? app.DebugPrintf(sc_verbose, "host : ") : app.DebugPrintf(sc_verbose, "client:");
	app.DebugPrintf(sc_verbose, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>    Small ID confirmed\n");
	m_flags |= SNP_FLAG_SMALLID_CONFIRMED;
}

// Attempt to send data, of any size, from this player to that specified by pPlayerTarget. This may not be possible depending on the two players, due to
// our star shaped network connectivity. Data may be any size, and is copied so on returning from this method it does not need to be preserved.
void SQRNetworkPlayer::SendData( SQRNetworkPlayer *pPlayerTarget, const void *data, unsigned int dataSize, bool ack )
{
	AckFlags ackFlags = ack ? e_flag_AckRequested : e_flag_AckNotRequested;
	// Our network is connected as a star. If we are the host, then we can send to any remote player. If we're a client, we can send only to the host.
	// The host can also send to other local players, but this doesn't need to go through Rudp.
	if( m_host )
	{
		if( ( m_type == SNP_TYPE_HOST ) && ( pPlayerTarget->m_type == SNP_TYPE_LOCAL ) )
		{
			// Special internal communication from host to local player
			m_manager->LocalDataSend( this, pPlayerTarget, data, dataSize );
		}
		else if( ( m_type == SNP_TYPE_LOCAL ) && ( pPlayerTarget->m_type == SNP_TYPE_HOST ) )
		{
			// Special internal communication from local player to host
			m_manager->LocalDataSend( this, pPlayerTarget, data, dataSize );
		}
		else if( ( m_type == SNP_TYPE_HOST ) && ( pPlayerTarget->m_type == SNP_TYPE_REMOTE ) )
		{
			// Rudp communication from host to remote player - handled by remote player instance
			pPlayerTarget->SendInternal(data,dataSize, ackFlags);
		}
		else
		{
			// Can't do any other types of communications
			assert(false);
		}
	}
	else
	{
		if( ( m_type == SNP_TYPE_LOCAL ) && ( pPlayerTarget->m_type == SNP_TYPE_HOST ) )
		{
			// Rudp communication from client to host - handled by this player instace
			SendInternal(data, dataSize, ackFlags);
		}
		else
		{
			// Can't do any other types of communications
			assert(false);
		}
	}
}

// Internal send function - to simplify the number of mechanisms we have for sending data, this method just adds the data to be send to the player's internal queue,
// and then calls SendMoreInternal. This method can take any size of data, which it will split up into payload size chunks before sending. All input data is copied
// into internal buffers.
void SQRNetworkPlayer::SendInternal(const void *data, unsigned int dataSize, AckFlags ackFlags)
{
	EnterCriticalSection(&m_csQueue);
	bool bOutstandingPackets = (m_sendQueue.size() > 0);		// check if there are still packets in the queue, we won't be calling SendMoreInternal here if there are
	QueuedSendBlock sendBlock;

	unsigned char *dataCurrent = (unsigned char *)data;
	unsigned int dataRemaining = dataSize;

	if(ackFlags == e_flag_AckReturning)
	{
		// no data, just the flag
		assert(dataSize == 0);
		assert(data == nullptr);
		int dataSize = dataRemaining;
		if( dataSize > SNP_MAX_PAYLOAD ) dataSize = SNP_MAX_PAYLOAD;
		sendBlock.start = nullptr;
		sendBlock.end = nullptr;
		sendBlock.current = nullptr;
		sendBlock.ack = ackFlags;
		m_sendQueue.push(sendBlock);
	}
	else
	{
	while( dataRemaining )
	{
		int dataSize = dataRemaining;
		if( dataSize > SNP_MAX_PAYLOAD ) dataSize = SNP_MAX_PAYLOAD;
		sendBlock.start = new unsigned char [dataSize];
		sendBlock.end = sendBlock.start + dataSize;
		sendBlock.current = sendBlock.start;
			sendBlock.ack = ackFlags;
		memcpy( sendBlock.start, dataCurrent, dataSize);
		m_sendQueue.push(sendBlock);
		dataRemaining -= dataSize;
		dataCurrent += dataSize;
	}

	}
	m_totalBytesInSendQueue += dataSize;

	// if the queue had something in it already, then the UDP callback will fire and call SendMoreInternal
	// so we don't call it here, to avoid a deadlock
	if(!bOutstandingPackets)
	{
		// Now try and send as much as we can
		SendMoreInternal();
	}

	LeaveCriticalSection(&m_csQueue);
}


int SQRNetworkPlayer::WriteDataPacket(const void* data, int dataSize, AckFlags ackFlags)
	{
	DataPacketHeader header(dataSize, ackFlags);
	int headerSize = sizeof(header);
	int packetSize = dataSize+headerSize;
	unsigned char* packetData = new unsigned char[packetSize];
	*((DataPacketHeader*)packetData) = header;
	memcpy(&packetData[headerSize], data, dataSize);

#ifndef _CONTENT_PACKAGE
	if(ackFlags == e_flag_AckRequested)
		m_ackStats.push_back(System::currentTimeMillis());
#endif

#ifdef __PS3__
	int ret = cellRudpWrite( m_rudpCtx, packetData, packetSize, 0);//CELL_RUDP_MSG_LATENCY_CRITICAL  );
#else // __ORBIS__ && __PSVITA__
	int ret = sceRudpWrite( m_rudpCtx, packetData, packetSize, 0);//SCE_RUDP_MSG_LATENCY_CRITICAL  );
#endif
	if(ret == sc_wouldBlockFlag)
	{
		// nothing was sent!
	}
	else
	{
		assert(ret==packetSize || ret > headerSize); // we must make sure we've sent the entire packet or the header and some data at least
		ret -= headerSize;
		if(ackFlags == e_flag_AckRequested)
		{
			EnterCriticalSection(&m_csAcks);
			m_acksOutstanding++;
			LeaveCriticalSection(&m_csAcks);
		}
	}
	delete packetData;

	return ret;
}

int SQRNetworkPlayer::GetPacketDataSize()
{
	unsigned int ackFlag;
	int headerSize = sizeof(ackFlag);
#ifdef __PS3__
	unsigned int packetSize = cellRudpGetSizeReadable(m_rudpCtx);
#else
	unsigned int packetSize = sceRudpGetSizeReadable(m_rudpCtx);
#endif
	if(packetSize == 0)
		return 0;

	unsigned int dataSize = packetSize - headerSize;
	assert(dataSize >= 0);
	if(dataSize == 0)
	{
		// header only, must just be an ack returning
		ReadAck();
	}
	return dataSize;
}

int SQRNetworkPlayer::ReadDataPacket(void* data, int dataSize)
{
	int headerSize = sizeof(DataPacketHeader);
	int packetSize = dataSize+headerSize;

	unsigned char* packetData = new unsigned char[packetSize];
#ifdef __PS3__
	int bytesRead = cellRudpRead( m_rudpCtx, packetData, packetSize, 0, nullptr );
#else // __ORBIS__ && __PSVITA__
	int bytesRead = sceRudpRead( m_rudpCtx, packetData, packetSize, 0, nullptr );
#endif
	if(bytesRead == sc_wouldBlockFlag)
	{
		delete packetData;
		return 0;
	}
	// check the header, and see if we need to send back an ack
	DataPacketHeader header = *((DataPacketHeader*)packetData);
	if(header.GetAckFlags() == e_flag_AckRequested)
	{
		// Don't send the ack back directly from here, as this is called from a rudp event callback, and we end up in a thread lock situation between the lock librudp uses
		// internally (which is locked already here since we are being called in the event handler), and our own lock that we do for processing our write queue
		m_manager->RequestWriteAck(GetSmallId());
	}
	else
	{
		assert(header.GetAckFlags() == e_flag_AckNotRequested);
	}
	if(bytesRead > 0)
	{
		bytesRead -= headerSize;
		memcpy(data, &packetData[headerSize], bytesRead);
	}
	assert(header.GetDataSize() == bytesRead);

	delete packetData;

	return bytesRead;
}



void SQRNetworkPlayer::ReadAck()
{
	DataPacketHeader header;
#ifdef __PS3__
	int bytesRead = cellRudpRead( m_rudpCtx, &header, sizeof(header), 0, nullptr );
#else // __ORBIS__ && __PSVITA__
	int bytesRead = sceRudpRead( m_rudpCtx, &header, sizeof(header), 0, nullptr );
#endif
	if(bytesRead == sc_wouldBlockFlag)
	{
		return;
	}

	assert(header.GetAckFlags() == e_flag_AckReturning);
	EnterCriticalSection(&m_csAcks);
	m_acksOutstanding--;
	assert(m_acksOutstanding >=0);
	LeaveCriticalSection(&m_csAcks);

#ifndef _CONTENT_PACKAGE
#ifdef PRINT_ACK_STATS
	int64_t timeTaken = System::currentTimeMillis() - m_ackStats[0];
	if(timeTaken < m_minAckTime)
		m_minAckTime = timeTaken;
	if(timeTaken > m_maxAckTime)
		m_maxAckTime = timeTaken;
	m_totalAcks++;
	m_totalAckTime += timeTaken;
	m_averageAckTime = m_totalAckTime / m_totalAcks;
	app.DebugPrintf("RUDP ctx : %d : Time taken for ack - %4d ms : min - %4d : max %4d : avg %4d\n", m_rudpCtx, timeTaken, m_minAckTime, m_maxAckTime, m_averageAckTime);
	m_ackStats.erase(m_ackStats.begin());
#endif
#endif
}

void SQRNetworkPlayer::WriteAck()
{
	SendInternal(nullptr, 0, e_flag_AckReturning);
}

int SQRNetworkPlayer::GetOutstandingAckCount()
{
	return m_manager->GetOutstandingAckCount(this);
}

int SQRNetworkPlayer::GetTotalOutstandingAckCount()
{
	return m_acksOutstanding;
}

int SQRNetworkPlayer::GetTotalSendQueueBytes()
{
	return m_totalBytesInSendQueue;
}

int SQRNetworkPlayer::GetTotalSendQueueMessages()
{
	CriticalSectionScopeLock lock(&m_csQueue);
	return m_sendQueue.size();

}

int SQRNetworkPlayer::GetSendQueueSizeBytes()
{
	return m_manager->GetSendQueueSizeBytes();
}

int SQRNetworkPlayer::GetSendQueueSizeMessages()
{
	return m_manager->GetSendQueueSizeMessages();
}



// Internal send function. This attempts to send as many elements in the queue as possible until the write function tells us that we can't send any more. This way,
// we are guaranteed that if there *is* anything more in the queue left to send, we'll get a CELL_RUDP_CONTEXT_EVENT_WRITABLE event when whatever we've managed to
// send here is complete, and can continue on.
void SQRNetworkPlayer::SendMoreInternal()
{
	EnterCriticalSection(&m_csQueue);
	assert(m_sendQueue.size() > 0);  // this should never be called with an empty queue.

	bool keepSending;
	do
	{
		keepSending = false;
		if( m_sendQueue.size() > 0)
		{
			// Attempt to send the full data in the first element in our queue
			unsigned char *data= m_sendQueue.front().current;
			int dataSize = m_sendQueue.front().end - m_sendQueue.front().current;
			int ret = WriteDataPacket(data, dataSize, m_sendQueue.front().ack);

			if( ret == dataSize )
			{
				// Fully sent, remove from queue - will loop in the while loop to see if there's anything else in the queue we could send
				m_totalBytesInSendQueue -= ret;
				delete [] m_sendQueue.front().start;
				m_sendQueue.pop();
				if( m_sendQueue.size() )
				{
					keepSending = true;
				}
			}
			else if( ( ret >= 0 ) || ( ret == sc_wouldBlockFlag ) )
			{

				// Things left to send - adjust this element in the queue
				int	remainingBytes;
				if( ret >= 0 )
				{
					// Only ret bytes sent so far
					m_totalBytesInSendQueue -= ret;
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
	} while (keepSending);
	LeaveCriticalSection(&m_csQueue);
}

void SQRNetworkPlayer::SetNameFromUID()
{
	mbstowcs(m_name, m_ISD.m_UID.getOnlineID(), 16);
	m_name[16] = 0;
#ifdef __PS3__ // only 1 player on vita, and they have to be online (or adhoc), and with PS4 all local players need to be signed in
	// Not an online player? Add a suffix with the controller ID on
	if( m_ISD.m_UID.isSignedIntoPSN() == 0)
	{
		int pos = wcslen(m_name);
		swprintf(&m_name[pos], 5, L" (%d)", m_ISD.m_UID.getQuadrant() + 1 );
	}
#endif
}

void SQRNetworkPlayer::SetName(char *name)
{
	mbstowcs(m_name, name, 20);
	m_name[20] = 0;
}

int SQRNetworkPlayer::GetSessionIndex()
{
	return m_manager->GetSessionIndex(this);
}

bool SQRNetworkPlayer::HasVoice()
{
#ifdef __ORBIS__
	return SonyVoiceChat_Orbis::hasMicConnected(this);
#elif defined __PSVITA__
	return SonyVoiceChat_Vita::hasMicConnected(this);
#else
	return SonyVoiceChat::hasMicConnected(&m_roomMemberId);
#endif
}

bool SQRNetworkPlayer::IsTalking()
{
#ifdef __ORBIS__
	return SonyVoiceChat_Orbis::isTalking(this);
#elif defined __PSVITA__
	return SonyVoiceChat_Vita::isTalking(this);
#else
	return SonyVoiceChat::isTalking(&m_roomMemberId);
#endif
}

bool SQRNetworkPlayer::IsMutedByLocalUser(int userIndex)
{
#ifdef __ORBIS__
// 	assert(0); // this is never called, so isn't implemented in the PS4 voice stuff at the moment
	return false;
#elif defined __PSVITA__
	return false;// this is never called, so isn't implemented in the Vita voice stuff at the moment
#else
	SQRNetworkManager_PS3* pMan = (SQRNetworkManager_PS3*)m_manager;
	return SonyVoiceChat::isMutedPlayer(pMan->m_roomSyncData.players[userIndex].m_roomMemberId);
#endif
}
