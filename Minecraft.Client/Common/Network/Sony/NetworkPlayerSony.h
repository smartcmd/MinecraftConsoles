#pragma once

#include "..\..\Common\Network\NetworkPlayerInterface.h"
#include "SQRNetworkPlayer.h"

// This is an implementation of the INetworkPlayer interface, for Sony platforms. It effectively wraps the SQRNetworkPlayer class in a non-platform-specific way.

class NetworkPlayerSony : public INetworkPlayer
{
public:
	// Common player interface
	NetworkPlayerSony(SQRNetworkPlayer *sqrPlayer);
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
	virtual wstring GetDisplayName();
	virtual PlayerUID GetUID();

	void SetUID(PlayerUID UID);

	virtual void SentChunkPacket();
	virtual int GetTimeSinceLastChunkPacket_ms();
private:
	SQRNetworkPlayer *m_sqrPlayer;
	Socket *m_pSocket;
	int64_t m_lastChunkPacketTime;
};
