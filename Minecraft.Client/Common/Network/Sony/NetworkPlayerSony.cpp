#include "stdafx.h"
#include "NetworkPlayerSony.h"

NetworkPlayerSony::NetworkPlayerSony(SQRNetworkPlayer *qnetPlayer)
{
	m_sqrPlayer = qnetPlayer;
	m_pSocket = nullptr;
	m_lastChunkPacketTime = 0;
}

unsigned char NetworkPlayerSony::GetSmallId()
{
	return m_sqrPlayer->GetSmallId();
}

void NetworkPlayerSony::SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority, bool ack)
{
	// TODO - handle priority
	m_sqrPlayer->SendData( static_cast<NetworkPlayerSony *>(player)->m_sqrPlayer, pvData, dataSize, ack );
}

bool NetworkPlayerSony::IsSameSystem(INetworkPlayer *player)
{
	return m_sqrPlayer->IsSameSystem(static_cast<NetworkPlayerSony *>(player)->m_sqrPlayer);
}

int NetworkPlayerSony::GetOutstandingAckCount()
{
	return m_sqrPlayer->GetOutstandingAckCount();
}

int NetworkPlayerSony::GetSendQueueSizeBytes( INetworkPlayer *player, bool lowPriority )
{
	return m_sqrPlayer->GetSendQueueSizeBytes();
}

int NetworkPlayerSony::GetSendQueueSizeMessages( INetworkPlayer *player, bool lowPriority )
{
	return m_sqrPlayer->GetSendQueueSizeMessages();
}

int NetworkPlayerSony::GetCurrentRtt()
{
	return 0;			// TODO
}

bool NetworkPlayerSony::IsHost()
{
	return m_sqrPlayer->IsHost();
}

bool NetworkPlayerSony::IsGuest()
{
	return false;		// TODO
}

bool NetworkPlayerSony::IsLocal()
{
	return m_sqrPlayer->IsLocal();
}

int NetworkPlayerSony::GetSessionIndex()
{
	return m_sqrPlayer->GetSessionIndex();
}

bool NetworkPlayerSony::IsTalking()
{
	return m_sqrPlayer->IsTalking();
}

bool NetworkPlayerSony::IsMutedByLocalUser(int userIndex)
{
	return m_sqrPlayer->IsMutedByLocalUser(userIndex);
}

bool NetworkPlayerSony::HasVoice()
{
	return m_sqrPlayer->HasVoice();
}

bool NetworkPlayerSony::HasCamera()
{
	return false;	// TODO
}

int NetworkPlayerSony::GetUserIndex()
{
	return m_sqrPlayer->GetLocalPlayerIndex();
}

void NetworkPlayerSony::SetSocket(Socket *pSocket)
{
	m_pSocket = pSocket;
}

Socket *NetworkPlayerSony::GetSocket()
{
	return m_pSocket;
}

const wchar_t *NetworkPlayerSony::GetOnlineName()
{
	return m_sqrPlayer->GetName();
}

wstring NetworkPlayerSony::GetDisplayName()
{
	return m_sqrPlayer->GetName();
}

PlayerUID NetworkPlayerSony::GetUID()
{
	return m_sqrPlayer->GetUID();
}

void NetworkPlayerSony::SetUID(PlayerUID UID)
{
	m_sqrPlayer->SetUID(UID);
}

void NetworkPlayerSony::SentChunkPacket()
{
	m_lastChunkPacketTime = System::currentTimeMillis();
}

int NetworkPlayerSony::GetTimeSinceLastChunkPacket_ms()
{
	// If we haven't ever sent a packet, return maximum
	if( m_lastChunkPacketTime == 0 )
	{
		return INT_MAX;
	}

	int64_t currentTime = System::currentTimeMillis();
	return static_cast<int>(currentTime - m_lastChunkPacketTime);
}
