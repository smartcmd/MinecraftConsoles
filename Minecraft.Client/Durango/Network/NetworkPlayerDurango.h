#pragma once

#include "../../Common/Network/NetworkPlayerInterface.h"
#include "DQRNetworkPlayer.h"

// This is an implementation of the INetworkPlayer interface, for Durango. It effectively wraps the DQRNetworkPlayer class in a non-platform-specific way.

class NetworkPlayerDurango : public INetworkPlayer
{
public:
	// Common player interface
	NetworkPlayerDurango(DQRNetworkPlayer *sqrPlayer);
	virtual unsigned char GetSmallId();
	virtual void SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority);
	virtual bool IsSameSystem(INetworkPlayer *player);
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

private:
	DQRNetworkPlayer *m_dqrPlayer;
	Socket			 *m_pSocket;
};