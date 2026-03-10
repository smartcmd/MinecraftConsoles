#pragma once
#include "SQRNetworkManager.h"
#include <queue>

// This is the lowest level class for handling the concept of a player on Sony platforms. This is managed by SQRNetworkManager. The game shouldn't directly communicate
// with this class, as it is wrapped by NetworkPlayerSony which is an implementation of a platform-independent interface INetworkPlayer.

class SQRNetworkPlayer
{
#ifdef __ORBIS__
	friend class SQRNetworkManager_Orbis;
	friend class SonyVoiceChat_Orbis;
#elif defined __PS3__
	friend class SQRNetworkManager_PS3;
#else // __PSVITA__
	friend class SQRNetworkManager_Vita;
	friend class SQRNetworkManager_AdHoc_Vita;
	friend class SonyVoiceChat_Vita;
#endif
	friend class SQRNetworkManager;
	friend class NetworkPlayerSony;
	friend class CPlatformNetworkManagerSony;

	int				GetSmallId();
	wchar_t			*GetName();
	bool			IsRemote();
	bool			IsHost();
	bool			IsLocal();
	int				GetLocalPlayerIndex();
	bool			IsSameSystem(SQRNetworkPlayer *other);
	uintptr_t		GetCustomDataValue();
	void			SetCustomDataValue(uintptr_t data);
	bool			HasVoice();
	bool			IsTalking();
	bool			IsMutedByLocalUser(int userIndex);

	static const int SNP_FLAG_CONNECTION_COMPLETE = 1;		// This player has a fully connected Rudp or other local link established (to a remote player if this is on the host, to the host if this is a client) - or isn't expected to have one
	static const int SNP_FLAG_SMALLID_ALLOCATED = 2;		// This player has a small id allocated
	static const int SNP_FLAG_SMALLID_CONFIRMED = 4;		// This player's small id has been confirmed as received by the client (only relevant for players using network communications, others set at the same time as allocating)
	static const int SNP_FLAG_READY_MASK = 7;				// Mask indicated all bits which must be set in the flags for this player to be considered "ready"

	static const int SNP_MAX_PAYLOAD = 1346;				// This is the default RUDP payload size - if we want to change this we'll need to use cellRudpSetOption to set something else & adjust segment size

	typedef enum
	{
		SNP_TYPE_HOST,		// This player represents the host
		SNP_TYPE_LOCAL,		// On host - this player is a local player that needs communicated with specially not using rudp. On clients - this is a local player, where m_rudpCtx is the context used to communicate from this player to/from the host
		SNP_TYPE_REMOTE,	// On host - this player's m_rupdCtx can be used to communicate from between the host and this player. On clients - this is a remote player that cannot be communicated with
	} eSQRNetworkPlayerType;

	enum AckFlags
	{
		e_flag_AckUnknown,
		e_flag_AckNotRequested,
		e_flag_AckRequested,
		e_flag_AckReturning
	};

	class DataPacketHeader
	{
		unsigned short m_dataSize;
		unsigned short m_ackFlags;
	public:
		DataPacketHeader() : m_dataSize(0), m_ackFlags(e_flag_AckUnknown) {}
		DataPacketHeader(int dataSize, AckFlags ackFlags) : m_dataSize(dataSize), m_ackFlags(ackFlags) { }
		AckFlags GetAckFlags() { return static_cast<AckFlags>(m_ackFlags);}
		int GetDataSize() { return m_dataSize; }
	};

#ifndef _CONTENT_PACKAGE
	std::vector<int64_t> m_ackStats;
	int m_minAckTime;
	int m_maxAckTime;
	int m_totalAcks;
	int64_t m_totalAckTime;
	int m_averageAckTime;
#endif

	class QueuedSendBlock
	{
	public:
		unsigned char *start;
		unsigned char *end;
		unsigned char *current;
		AckFlags ack;
	};

	class InitSendData
	{
	public:
		unsigned char				m_smallId;				// Id to uniquely and permanently identify this player between machines - assigned by the server
		PlayerUID					m_UID;
	};

	SQRNetworkPlayer(SQRNetworkManager *manager, eSQRNetworkPlayerType playerType, bool onHost, SceNpMatching2RoomMemberId roomMemberId, int localPlayerIdx, int rudpCtx, PlayerUID *pUID);
	~SQRNetworkPlayer();

	PlayerUID					GetUID();
	void						SetUID(PlayerUID UID);
	bool						HasConnectionAndSmallId();
	bool						IsReady();
	void						ConnectionComplete();
	void						SmallIdAllocated(unsigned char smallId);
	void						InitialDataReceived(InitSendData *ISD);	// Only for remote players as viewed from the host, this is set when the host has received confirmation that the client has received the small id for this player, ie it is now safe to send data to
	bool						HasSmallIdConfirmed();

	void						SendData( SQRNetworkPlayer *pPlayerTarget, const void *data, unsigned int dataSize, bool ack );

	void						ConfirmReady();
	void						SendInternal(const void *data, unsigned int dataSize, AckFlags ackFlags);
	void						SendMoreInternal();
	int							GetPacketDataSize();
	int							ReadDataPacket(void* data, int dataSize);
	int							WriteDataPacket(const void* data, int dataSize, AckFlags ackFlags);
	void						ReadAck();
	void						WriteAck();

	int							GetOutstandingAckCount();
	int							GetSendQueueSizeBytes();
	int							GetSendQueueSizeMessages();

	int							GetTotalOutstandingAckCount();
	int							GetTotalSendQueueBytes();
	int							GetTotalSendQueueMessages();


#ifdef __PSVITA__
	void						SendInternal_VitaAdhoc(const void *data, unsigned int dataSize, EAdhocDataTag tag = e_dataTag_Normal);
	void						SendMoreInternal_VitaAdhoc();
#endif
	void						SetNameFromUID();
	void						SetName(char *name);
	int							GetSessionIndex();

	eSQRNetworkPlayerType		m_type;					// The player type
	bool						m_host;					// Whether this actual player class is stored on a host (not whether it represents the host, or a player on the host machine)
	int							m_flags;				// Flags reflecting current state of this player
	int							m_rudpCtx;				// Rudp context that can be used to communicate between this player & the host (see comments for eSQRNetworkPlayerType above)
	int							m_localPlayerIdx;		// Index of this player on the machine to which it belongs
	SceNpMatching2RoomMemberId	m_roomMemberId;			// The room member id, effectively a per machine id
	InitSendData				m_ISD;					// Player UID & ID that get sent together to the host when connection is established
	SQRNetworkManager			*m_manager;				// Pointer back to the manager that is managing this player
	wchar_t						m_name[21];
	uintptr_t					m_customData;
	CRITICAL_SECTION			m_csQueue;
	CRITICAL_SECTION			m_csAcks;
	std::queue<QueuedSendBlock>	m_sendQueue;
	int							m_totalBytesInSendQueue;

	int							m_acksOutstanding;
};
