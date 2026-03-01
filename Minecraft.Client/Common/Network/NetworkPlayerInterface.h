#pragma once

class Socket;

// This is the platform independent interface for dealing with players within a network game. This should be used directly by game code (and GameNetworkManager) rather than the platform-specific implementations.

class INetworkPlayer
{
public:
	virtual ~INetworkPlayer() {}
	virtual unsigned char GetSmallId() = 0;
	virtual void SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority, bool ack) = 0;
	virtual bool IsSameSystem(INetworkPlayer *player) = 0;
	virtual int GetOutstandingAckCount() = 0;
	virtual int GetSendQueueSizeBytes( INetworkPlayer *player, bool lowPriority ) = 0;
	virtual int GetSendQueueSizeMessages( INetworkPlayer *player, bool lowPriority ) = 0;
	virtual int GetCurrentRtt() = 0;
	virtual bool IsHost() = 0;
	virtual bool IsGuest() = 0;
	virtual bool IsLocal() = 0;
	virtual int GetSessionIndex() = 0;
	virtual bool IsTalking() = 0;
	virtual bool IsMutedByLocalUser(int userIndex) = 0;
	virtual bool HasVoice() = 0;
	virtual bool HasCamera() = 0;
	virtual int GetUserIndex() = 0;
	virtual void SetSocket(Socket *pSocket) = 0;
	virtual Socket *GetSocket() = 0;
	virtual const wchar_t *GetOnlineName() = 0;
	virtual wstring GetDisplayName() = 0;
	virtual PlayerUID GetUID() = 0;
	virtual void SentChunkPacket() = 0;
	virtual int GetTimeSinceLastChunkPacket_ms() = 0;
};
