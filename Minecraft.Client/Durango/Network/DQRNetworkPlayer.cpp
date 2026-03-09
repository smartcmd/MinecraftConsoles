#include "stdafx.h"
#include "DQRNetworkPlayer.h"
#include "ChatIntegrationLayer.h"

DQRNetworkPlayer::DQRNetworkPlayer()
{
}

DQRNetworkPlayer::DQRNetworkPlayer(DQRNetworkManager *manager, eDQRNetworkPlayerType playerType, bool onHost, int localPlayerIdx, unsigned int sessionAddress)
{
	m_localPlayerIdx	= localPlayerIdx;
	m_type				= playerType;
	m_host				= onHost;
	m_manager			= manager;
	m_customData		= 0;
	m_sessionAddress	= sessionAddress;
}

DQRNetworkPlayer::~DQRNetworkPlayer()
{
}

PlayerUID	DQRNetworkPlayer::GetUID()
{
	return m_UID;
}

void	DQRNetworkPlayer::SetUID(PlayerUID UID)
{
	m_UID = UID;
}

int DQRNetworkPlayer::GetLocalPlayerIndex()
{
	return m_localPlayerIdx;
}

uintptr_t	DQRNetworkPlayer::GetCustomDataValue()
{
	return m_customData;
}

void	DQRNetworkPlayer::SetCustomDataValue(uintptr_t data)
{
	m_customData = data;
}

bool DQRNetworkPlayer::IsRemote()
{
	return !IsLocal();
}

bool DQRNetworkPlayer::IsHost()
{
	return (m_type == DNP_TYPE_HOST);
}

bool DQRNetworkPlayer::IsLocal()
{
	// m_host determines whether this *machine* is hosting the game, not this player (which is determined by m_type)
	if( m_host )
	{
		// If we are the hosting machine, then both the host & local players are local to this machine
		return (m_type == DNP_TYPE_HOST) || (m_type == DNP_TYPE_LOCAL);
	}
	else
	{
		// Not hosting, just local players are actually physically local
		return (m_type == DNP_TYPE_LOCAL) ;
	}
}

bool	DQRNetworkPlayer::IsSameSystem(DQRNetworkPlayer *other)
{
	return ( m_sessionAddress == other->m_sessionAddress );
}

bool	DQRNetworkPlayer::IsTalking()
{
	if(m_manager->m_chat == nullptr) return false;
	Microsoft::Xbox::GameChat::ChatUser^ chatUser = m_manager->m_chat->GetChatUserByXboxUserId(ref new Platform::String(m_UID.toString().c_str()));

	if( chatUser == nullptr ) return false;
	if( chatUser->TalkingMode == Microsoft::Xbox::GameChat::ChatUserTalkingMode::NotTalking )
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool	DQRNetworkPlayer::HasVoice()
{
	if(m_manager->m_chat == nullptr) return false;
	Microsoft::Xbox::GameChat::ChatUser^ chatUser = m_manager->m_chat->GetChatUserByXboxUserId(ref new Platform::String(m_UID.toString().c_str()));

	if( chatUser == nullptr ) return false;
	if( static_cast<int>(chatUser->ParticipantType) & static_cast<int>(Windows::Xbox::Chat::ChatParticipantTypes::Talker) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool	DQRNetworkPlayer::HasCamera()
{
	return false;
}

LPCWSTR	DQRNetworkPlayer::GetGamertag()
{
	return m_name;
}

int	DQRNetworkPlayer::GetSmallId()
{
	return (int)m_smallId;
}

void DQRNetworkPlayer::SetSmallId(unsigned char smallId)
{
	m_smallId = smallId;
}

int DQRNetworkPlayer::GetSessionIndex()
{
	return m_manager->GetSessionIndex(this);
}

// Attempt to send data, of any size, from this player to that specified by pPlayerTarget. This may not be possible depending on the two players, due to
// our star shaped network connectivity. Data may be any size, and is copied so on returning from this method it does not need to be preserved.
void DQRNetworkPlayer::SendData( DQRNetworkPlayer *pPlayerTarget, const void *data, unsigned int dataSize )
{
	// Our network is connected as a star. If we are the host, then we can send to any remote player. If we're a client, we can send only to the host.
	// The host can also send to other local players, but this doesn't need to go through Rudp. 
	if( m_host )
	{
		if( ( m_type == DNP_TYPE_HOST ) && ( pPlayerTarget->m_type == DNP_TYPE_REMOTE ) )
		{
			// Rudp communication from host to remote player - handled by remote player instance
			pPlayerTarget->SendInternal(data,dataSize);
		}
		else
		{
			// Can't do any other types of communications
			assert(false);
		}
	}
	else
	{
		if( ( m_type == DNP_TYPE_LOCAL ) && ( pPlayerTarget->m_type == DNP_TYPE_HOST ) )
		{
			// Rudp communication from client to host - handled by this player instace
			SendInternal(data, dataSize);
		}
		else
		{
			// Can't do any other types of communications
			assert(false);
		}
	}
}

void DQRNetworkPlayer::SendInternal(const void *data, unsigned int dataSize)
{
	m_manager->SendBytes(m_smallId, (BYTE *)data, dataSize);
}

int DQRNetworkPlayer::GetSendQueueSizeBytes()
{
	return m_manager->GetQueueSizeBytes();
}

int DQRNetworkPlayer::GetSendQueueSizeMessages()
{
	return m_manager->GetQueueSizeMessages();
}

wchar_t	*DQRNetworkPlayer::GetName()
{
	return m_name;
}

void DQRNetworkPlayer::SetName(const wchar_t *name)
{
	wcscpy_s(m_name, name);
}

// Return display name (if display name is not set, return name instead)
wstring	DQRNetworkPlayer::GetDisplayName()
{
	return (m_displayName == L"") ? m_name : m_displayName;
}

// Set display name
void DQRNetworkPlayer::SetDisplayName(wstring displayName)
{
	m_displayName = displayName;
}