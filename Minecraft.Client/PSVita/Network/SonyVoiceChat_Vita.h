#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <voice.h>
#include "Common/Network/Sony/SQRNetworkPlayer.h"

static const int sc_maxVoiceDataSize = 2048;


class VoicePacket
{
	static const int MAX_LOCAL_PLAYER_COUNT = 1;

public:
	struct Flags
	{
		bool m_bTalking  : 1; 
		bool m_bHasMicConnected : 1;
	};	
	
	Flags m_localPlayerFlags[MAX_LOCAL_PLAYER_COUNT];
	uint32_t m_frameSendIndex;
	uint32_t m_numFrames;
	uint32_t m_checkSum;
	uint32_t m_playerIndexFlags;
	char m_data[sc_maxVoiceDataSize];

	static int getPacketSize(int dataSize) { return (uint64_t)&((VoicePacket*)0)->m_data[dataSize];}
	void setChecksum(int dataSize)
	{
		m_checkSum = 0;
		for(int i=0;i<dataSize;i++)
			m_checkSum += m_data[i];
	}
	bool verifyData(int packetSize, int frameSize) 
	{ 
		if(m_numFrames == 0)
			return false;
		int dataSize = m_numFrames*frameSize; 
		if(packetSize != getPacketSize(dataSize))
			return false;
		int checkSum = 0;
		for(int i=0;i<dataSize;i++)
			checkSum += m_data[i];
		if(checkSum != m_checkSum)
			return false;
		return true;
	}
};


//--------------------------------------------------------------------------
// read-write-safe ring buffer implementation: does not use mutex protection
// the writer thread changes pointer <buf_free>, 
// the reader thread changes pointer <buf_full>
class RingBuffer
{
public:
	RingBuffer(int sizeBytes);
	~RingBuffer()							{	delete buffer;	}
	void Reset(void)						{	buf_full = buf_free = 0;		}
	unsigned int DataSize(void)				{	return (buf_free - buf_full);	}
	int Write(char* data, int len_);
	int Read(char* data, int max_bytes_);
	int getDataSize()						{	return buf_free - buf_full; }
	void ResetByWriter(void)				{	buf_free = buf_full;	}
	void ResetByReader(void)				{	buf_full = buf_free;	}

private:
	char*   buffer;
	unsigned int buf_size;
	unsigned int buf_full;
	unsigned int buf_free;
};


static const int sc_ringBufferSize = 16384;

class SQRLocalVoiceDevice
{
public:
	uint32_t    m_headsetPort;
	uint32_t    m_microphonePort;
//	int32_t		m_micAudioDevicePort;
	uint8_t		m_localConnections[4];		// connection between this devices mic and other local player's headsets 
	bool		m_bChatRestricted;

	bool		m_bValid;

public:
	SQRLocalVoiceDevice() 
		: m_headsetPort(SCE_VOICE_INVALID_PORT_ID)
		, m_microphonePort(SCE_VOICE_INVALID_PORT_ID)
 		, m_bValid(false)
// 		, m_micAudioDevicePort(-1)
	{
		for(int i=0;i<4;i++)
			m_localConnections[i] = 0;
	}

	void init(bool bChatRestricted);

	void shutdown();
	bool isValid() { return m_bValid; }
// 	void setBitRate()
// 	{
// 		int err = sceVoiceSetBitRate(uint32_t portId,
// 			SceVoiceBitRate bitrate
// 			);
// 	}
};

#define VOICE_ENCODED_FORMAT                  SCEVOICE_BITRATE_7300



class SQRVoiceConnection
{
	static const int MAX_LOCAL_PLAYER_COUNT = 1;

	static const int SNP_MAX_PAYLOAD = 1346;				// This is the default RUDP payload size - if we want to change this we'll need to use cellRudpSetOption to set something else & adjust segment size
	class QueuedSendBlock
	{
	public:
		unsigned char *start;
		unsigned char *end;
		unsigned char *current;
	};

	std::queue<QueuedSendBlock>	m_sendQueue;
	CRITICAL_SECTION			m_csQueue;

public:
	int			m_rudpCtx;
	bool		m_bConnected;
	uint32_t    m_voiceInPort;		// 1 input port per connection, incoming UDP packets are written to each of these, and then they're connected out to all headsets
	int			m_headsetConnectionMask;		// 1 bit per player, if the headset connection has been made
	RingBuffer	m_playRingBuffer;
	SceNpMatching2RoomMemberId m_remoteRoomMemberId; // Assigned by Matching2 lib, we can use to indicate which machine this player belongs to (note - 16 bits)
	std::queue<VoicePacket> m_receivedVoicePackets;
	CRITICAL_SECTION			m_csPacketQueue;
	VoicePacket::Flags m_remotePlayerFlags[MAX_LOCAL_PLAYER_COUNT];
	uint32_t	m_nextExpectedFrameIndex;
	bool		m_bFlaggedForShutdown;
	SQRVoiceConnection(int rudpCtx, SceNpMatching2RoomMemberId remoteRoomMemberId);
	~SQRVoiceConnection();

	void SendInternal(const void *data, unsigned int dataSize);
	void SendMoreInternal();
	void readRemoteData();
	bool getNextPacket(VoicePacket& packet);
	void addPacket(VoicePacket& packet);


};

class SonyVoiceChat_Vita
{
public:

	static void init();
	static void start();
	static void shutdown();
	static void tick();
	static void checkFinished();
	static void setEnabled(bool bEnabled);
	static bool hasMicConnected(SQRNetworkPlayer* pNetPlayer);
	static bool isTalking(SQRNetworkPlayer* pNetPlayer);
	static void mute(bool bMute);			//Turn chat audio on or off
	static void mutePlayer(const SceNpMatching2RoomMemberId member_id, bool bMute);		//Turn chat audio from a specified player on or off;
	static void muteLocalPlayer(bool bMute); //Turn microphone input on or off;

	static bool isMuted();
	static bool isMutedPlayer(const PlayerUID& memberUID);
	static bool isMutedLocalPlayer(); //Turn microphone input on or off;

	static void initLocalPlayer(int playerIndex);

	static SQRVoiceConnection* addRemoteConnection(int RudpCxt, SceNpMatching2RoomMemberId peerMemberId);
	static void connectPlayer(SQRVoiceConnection* pConnection, int playerIndex);
	static void connectPlayerToAll(int playerIndex);
	static void disconnectLocalPlayer(int localIdx);
	static void disconnectRemoteConnection( SQRVoiceConnection* pVoice );

	static void VoiceEventCallback( SceVoiceEventData* pEvent );

	static std::vector<SQRVoiceConnection*> m_remoteConnections;
	static void connectPorts(uint32_t inPort, uint32_t outPort);
	static void disconnectPorts(uint32_t inPort, uint32_t outPort);
	static void makeLocalConnections();
	static void breakLocalConnections(int playerIdx);

	static void sendAllVoiceData();
	static void playAllReceivedData();

	static void sendPCMMicData();
	static SQRVoiceConnection* getVoiceConnectionFromRoomMemberID(SceNpMatching2RoomMemberId roomMemberID);

	static SQRVoiceConnection* GetVoiceConnectionFromRudpCtx(int RudpCtx);
	static void setConnected(int RudpCtx);

private:

	static const int MAX_LOCAL_PLAYER_COUNT = 1;

	static bool m_bVoiceStarted;
	static int					m_numLocalDevicesConnected;
	static SQRLocalVoiceDevice	m_localVoiceDevices[MAX_LOCAL_PLAYER_COUNT];

	static uint32_t     m_voiceOutPort;		// single output port that all local devices are mixed to, and then sent out to all other remote machines

	static RingBuffer m_recordRingBuffer;
	static RingBuffer m_playRingBuffer;
	static VoicePacket::Flags m_localPlayerFlags[MAX_LOCAL_PLAYER_COUNT];
	static bool	m_forceSendPacket; // force a packet across the network, even if there's no data, so we can update flags
	static bool m_bInitialised;
	static CRITICAL_SECTION			m_csRemoteConnections;


};