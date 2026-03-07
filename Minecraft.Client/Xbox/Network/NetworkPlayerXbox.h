#pragma once

#include "../../Common/Network/NetworkPlayerInterface.h"

// This is an implementation of the INetworkPlayer interface, for Xbox 360. It effectively wraps the IQNetPlayer class in a non-platform-specific way. It is
// managed by PlatformNetworkManagerXbox.

class NetworkPlayerXbox : public INetworkPlayer
{
public:
	// Common player interface
	NetworkPlayerXbox(IQNetPlayer *qnetPlayer);
	virtual unsigned char GetSmallId();
	virtual void SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority, bool ack);
	virtual bool IsSameSystem(INetworkPlayer *player);
	virtual int GetOutstandingAckCount();
	virtual int GetSendQueueSizeBytes( INetworkPlayer *player, bool lowPriority );
	virtual int GetSendQueueSizeMessages( INetworkPlayer *player, bool lowPriority );
	virtual int GetCurrentRtt();
	virtual bool IsHost();
	virtual bool IsGuest();
	virtual bool IsLocal();
	virtual int GetSessionIndex();
	virtual bool IsTalking();
	virtual bool IsMutedByLocalUser(int userIndex);
	virtual bool HasVoice();
	virtual bool HasCamera();
	virtual int GetUserIndex();
	virtual void SetSocket(Socket *pSocket);
	virtual Socket *GetSocket();
	virtual const wchar_t *GetOnlineName();
	virtual std::wstring GetDisplayName();
	virtual PlayerUID GetUID();
	virtual void SentChunkPacket();
	virtual int GetTimeSinceLastChunkPacket_ms();

	// Extra xbox-specific things
	IQNetPlayer *GetQNetPlayer();
private:
	IQNetPlayer *m_qnetPlayer;
	Socket *m_pSocket;
	int64_t m_lastChunkPacketTime;
};