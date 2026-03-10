#include "stdafx.h"
#include "NetworkPlayerDurango.h"

NetworkPlayerDurango::NetworkPlayerDurango(DQRNetworkPlayer *qnetPlayer)
{
	m_dqrPlayer = qnetPlayer;
	m_pSocket = nullptr;
}

unsigned char NetworkPlayerDurango::GetSmallId()
{
	return m_dqrPlayer->GetSmallId();
}

void NetworkPlayerDurango::SendData(INetworkPlayer *player, const void *pvData, int dataSize, bool lowPriority)
{
	m_dqrPlayer->SendData( static_cast<NetworkPlayerDurango *>(player)->m_dqrPlayer, pvData, dataSize );
}

bool NetworkPlayerDurango::IsSameSystem(INetworkPlayer *player)
{
	return m_dqrPlayer->IsSameSystem(static_cast<NetworkPlayerDurango *>(player)->m_dqrPlayer);
}

int NetworkPlayerDurango::GetSendQueueSizeBytes( INetworkPlayer *player, bool lowPriority )
{
	return m_dqrPlayer->GetSendQueueSizeBytes();
}

int NetworkPlayerDurango::GetSendQueueSizeMessages( INetworkPlayer *player, bool lowPriority )
{
	return m_dqrPlayer->GetSendQueueSizeMessages();
}

int NetworkPlayerDurango::GetCurrentRtt()
{
	return 0;			// TODO
}

bool NetworkPlayerDurango::IsHost()
{
	return m_dqrPlayer->IsHost();
}

bool NetworkPlayerDurango::IsGuest()
{
	return false;		// TODO
}

bool NetworkPlayerDurango::IsLocal()
{
	return m_dqrPlayer->IsLocal();
}

int NetworkPlayerDurango::GetSessionIndex()
{
	return m_dqrPlayer->GetSessionIndex();
}

bool NetworkPlayerDurango::IsTalking()
{
	return m_dqrPlayer->IsTalking();
}

bool NetworkPlayerDurango::IsMutedByLocalUser(int userIndex)
{
	return false;
}

bool NetworkPlayerDurango::HasVoice()
{
	return m_dqrPlayer->HasVoice();
}

bool NetworkPlayerDurango::HasCamera()
{
	return false;	// TODO
}

int NetworkPlayerDurango::GetUserIndex()
{
	return m_dqrPlayer->GetLocalPlayerIndex();
}

void NetworkPlayerDurango::SetSocket(Socket *pSocket)
{
	m_pSocket = pSocket;
}

Socket *NetworkPlayerDurango::GetSocket()
{
	return m_pSocket;
}

const wchar_t *NetworkPlayerDurango::GetOnlineName()
{
	return m_dqrPlayer->GetName();
}

wstring NetworkPlayerDurango::GetDisplayName()
{
	return m_dqrPlayer->GetDisplayName();
}

PlayerUID NetworkPlayerDurango::GetUID()
{
	return m_dqrPlayer->GetUID();
}

void NetworkPlayerDurango::SetUID(PlayerUID UID)
{
	m_dqrPlayer->SetUID(UID);
}