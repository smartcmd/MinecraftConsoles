#include "stdafx.h"
#include "NetworkPlayerXbox.h"

NetworkPlayerXbox::NetworkPlayerXbox(IQNetPlayer *qnetPlayer)
{
	m_qnetPlayer = qnetPlayer;
	m_pSocket = NULL;
}

unsigned char NetworkPlayerXbox::GetSmallId()
{
	return m_qnetPlayer->GetSmallId();
}

void NetworkPlayerXbox::SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority, bool ack)
{
	DWORD flags;
	flags = QNET_SENDDATA_RELIABLE | QNET_SENDDATA_SEQUENTIAL;
	if( lowPriority ) flags |= QNET_SENDDATA_LOW_PRIORITY | QNET_SENDDATA_SECONDARY;
	m_qnetPlayer->SendData(((NetworkPlayerXbox *)player)->m_qnetPlayer, pvData, dataSize, flags);
}

int NetworkPlayerXbox::GetOutstandingAckCount()
{
	return 0;
}

bool NetworkPlayerXbox::IsSameSystem(INetworkPlayer *player)
{
	return ( m_qnetPlayer->IsSameSystem(((NetworkPlayerXbox *)player)->m_qnetPlayer) == TRUE );
}

int NetworkPlayerXbox::GetSendQueueSizeBytes( INetworkPlayer *player, bool lowPriority )
{
	DWORD flags = QNET_GETSENDQUEUESIZE_BYTES;
	if( lowPriority ) flags |= QNET_GETSENDQUEUESIZE_SECONDARY_TYPE;
	return m_qnetPlayer->GetSendQueueSize(player ? ((NetworkPlayerXbox *)player)->m_qnetPlayer : NULL , flags);
}

int NetworkPlayerXbox::GetSendQueueSizeMessages( INetworkPlayer *player, bool lowPriority )
{
	DWORD flags = QNET_GETSENDQUEUESIZE_MESSAGES;
	if( lowPriority ) flags |= QNET_GETSENDQUEUESIZE_SECONDARY_TYPE;
	return m_qnetPlayer->GetSendQueueSize(player ? ((NetworkPlayerXbox *)player)->m_qnetPlayer : NULL , flags);
}

int NetworkPlayerXbox::GetCurrentRtt()
{
	return m_qnetPlayer->GetCurrentRtt();
}

bool NetworkPlayerXbox::IsHost()
{
	return ( m_qnetPlayer->IsHost() == TRUE );
}

bool NetworkPlayerXbox::IsGuest()
{
	return ( m_qnetPlayer->IsGuest() == TRUE );
}

bool NetworkPlayerXbox::IsLocal()
{
	return ( m_qnetPlayer->IsLocal() == TRUE );
}

int NetworkPlayerXbox::GetSessionIndex()
{
	return m_qnetPlayer->GetSessionIndex();
}

bool NetworkPlayerXbox::IsTalking()
{
	return ( m_qnetPlayer->IsTalking() == TRUE );
}

bool NetworkPlayerXbox::IsMutedByLocalUser(int userIndex)
{
	return ( m_qnetPlayer->IsMutedByLocalUser(userIndex) == TRUE );
}

bool NetworkPlayerXbox::HasVoice()
{
	return ( m_qnetPlayer->HasVoice() == TRUE );
}

bool NetworkPlayerXbox::HasCamera()
{
	return ( m_qnetPlayer->HasCamera() == TRUE );
}

int NetworkPlayerXbox::GetUserIndex()
{
	return m_qnetPlayer->GetUserIndex();
}

void NetworkPlayerXbox::SetSocket(Socket *pSocket)
{
	m_pSocket = pSocket;
}

Socket *NetworkPlayerXbox::GetSocket()
{
	return m_pSocket;
}

PlayerUID NetworkPlayerXbox::GetUID()
{
	return m_qnetPlayer->GetXuid();
}

const wchar_t *NetworkPlayerXbox::GetOnlineName()
{
	return m_qnetPlayer->GetGamertag();
}

std::wstring NetworkPlayerXbox::GetDisplayName()
{
	return m_qnetPlayer->GetGamertag();
}

IQNetPlayer *NetworkPlayerXbox::GetQNetPlayer()
{
	return m_qnetPlayer;
}

void NetworkPlayerXbox::SentChunkPacket()
{
	m_lastChunkPacketTime = System::currentTimeMillis();
}

int NetworkPlayerXbox::GetTimeSinceLastChunkPacket_ms()
{
	// If we haven't ever sent a packet, return maximum
	if( m_lastChunkPacketTime == 0 )
	{
		return INT_MAX;
	}

	int64_t currentTime = System::currentTimeMillis();
	return (int)( currentTime - m_lastChunkPacketTime );
}