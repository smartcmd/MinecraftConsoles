#include "stdafx.h"

#include "SQRNetworkManager.h"

bool SQRNetworkManager::s_safeToRespondToGameBootInvite = false;

void SQRNetworkManager::SafeToRespondToGameBootInvite()
{
	s_safeToRespondToGameBootInvite = true;
}

int SQRNetworkManager::GetSendQueueSizeBytes()
{
	int queueSize = 0;
	int playerCount = GetPlayerCount();
	for(int i = 0; i < playerCount; ++i)
	{
		SQRNetworkPlayer *player = GetPlayerByIndex( i );
		if( player != nullptr )
		{
			queueSize += player->GetTotalSendQueueBytes();
		}
	}
	return queueSize;
}

int SQRNetworkManager::GetSendQueueSizeMessages()
{
	int queueSize = 0;
	int playerCount = GetPlayerCount();
	for(int i = 0; i < playerCount; ++i)
	{
		SQRNetworkPlayer *player = GetPlayerByIndex( i );
		if( player != nullptr )
		{
			queueSize += player->GetTotalSendQueueMessages();
		}
	}
	return queueSize;
}

int SQRNetworkManager::GetOutstandingAckCount(SQRNetworkPlayer *pSQRPlayer)
{
	int ackCount = 0;
	int playerCount = GetPlayerCount();
	for(int i = 0; i < playerCount; ++i)
	{
		SQRNetworkPlayer *pSQRPlayer2 = GetPlayerByIndex( i );
		if( pSQRPlayer2 )
		{
			if( ( pSQRPlayer == pSQRPlayer2 ) || (pSQRPlayer->IsSameSystem(pSQRPlayer2) ) )
			{
				ackCount += pSQRPlayer2->m_acksOutstanding;
			}
		}
	}
	return ackCount;
}

void SQRNetworkManager::RequestWriteAck(int smallId)
{
	EnterCriticalSection(&m_csAckQueue);
	m_queuedAckRequests.push(smallId);
	LeaveCriticalSection(&m_csAckQueue);
}

void SQRNetworkManager::TickWriteAcks()
{
	EnterCriticalSection(&m_csAckQueue);
	while(m_queuedAckRequests.size() > 0)
	{
		int smallId = m_queuedAckRequests.front();
		m_queuedAckRequests.pop();
		SQRNetworkPlayer *player = GetPlayerBySmallId(smallId);
		if( player )
		{
			LeaveCriticalSection(&m_csAckQueue);
			player->WriteAck();
			EnterCriticalSection(&m_csAckQueue);
		}
	}
	LeaveCriticalSection(&m_csAckQueue);
}