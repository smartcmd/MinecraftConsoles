#include "stdafx.h"

#include "DQRNetworkManager.h"
#include "PartyController.h"
#include <collection.h>
#include <ppltasks.h>
#include <ws2tcpip.h>
#include "../Minecraft.World/StringHelpers.h"
#include "base64.h"

#ifdef _DURANGO
#include "../Minecraft.World/DurangoStats.h"
#endif

#include "ChatIntegrationLayer.h"

using namespace Concurrency;
using namespace Windows::Foundation::Collections;

DQRNetworkManagerEventHandlers::DQRNetworkManagerEventHandlers(DQRNetworkManager *pDQRNet)
{
	m_pDQRNet = pDQRNet;
}

void DQRNetworkManagerEventHandlers::Setup(WXNRs::Session^ session)
{
	try
	{
		m_dataReceivedToken   = session->DataReceived			   += ref new Windows::Foundation::EventHandler<WXNRs::DataReceivedEventArgs^>(this, &DQRNetworkManagerEventHandlers::DataReceivedHandler);
		m_sessionStatusToken  = session->SessionStatusUpdate       += ref new Windows::Foundation::EventHandler<WXNRs::SessionStatusUpdateEventArgs^>(this, &DQRNetworkManagerEventHandlers::SessionStatusUpdateHandler);
		m_sessionAddressToken = session->SessionAddressDataChanged += ref new Windows::Foundation::EventHandler<WXNRs::SessionAddressDataChangedEventArgs^>(this, &DQRNetworkManagerEventHandlers::SessionAddressDataChangedHandler);
		m_addedSessionToken   = session->AddedSessionAddress       += ref new Windows::Foundation::EventHandler<WXNRs::AddedSessionAddressEventArgs^>(this, &DQRNetworkManagerEventHandlers::AddedSessionAddressHandler);
		m_removedSessionToken = session->RemovedSessionAddress     += ref new Windows::Foundation::EventHandler<WXNRs::RemovedSessionAddressEventArgs^>(this, &DQRNetworkManagerEventHandlers::RemovedSessionAddressHandler);
		m_globalDataToken     = session->GlobalSessionDataChanged  += ref new Windows::Foundation::EventHandler<WXNRs::GlobalSessionDataChangedEventArgs^>(this, &DQRNetworkManagerEventHandlers::GlobalSessionDataChangedHandler);
	}
	catch(Platform::COMException^ ex)
	{
		// swallow exceptions
	}
	catch(...)
	{
		// swallow exceptions
	}
}

void DQRNetworkManagerEventHandlers::Pulldown(WXNRs::Session^ session)
{
	try
	{
		session->DataReceived				-=	m_dataReceivedToken; 
		session->SessionStatusUpdate		-=	m_sessionStatusToken;
		session->SessionAddressDataChanged	-=	m_sessionAddressToken;
		session->AddedSessionAddress		-=  m_addedSessionToken;   
		session->RemovedSessionAddress		-=	m_removedSessionToken;
		session->GlobalSessionDataChanged	-=	m_globalDataToken;
	}
	catch(Platform::COMException^ ex)
	{
		// swallow exceptions
	}
	catch(...)
	{
		// swallow exceptions
	}
}

// This event handler is called directly by the realtime session layer, when data is received. We split this data into into data that is meant to be
// handled internally by the DQRNetworkManager, and that which is meant to be passed on to the game itself as communication between players.
void DQRNetworkManagerEventHandlers::DataReceivedHandler(Platform::Object^ session, WXNRs::DataReceivedEventArgs^ args)
{
//	DQRNetworkManager::LogCommentFormat(L"DataReceivedHandler session addr: 0x%x (%d bytes)",args->SessionAddress,args->Data->Length);

    if (session == m_pDQRNet->m_XRNS_Session)
    {
		EnterCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
		DQRNetworkManager::RTS_Message rtsMessage;
		if( args->ChannelId == WXNRs::ChannelId::DefaultChatReceive )
		{
			rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_DATA_RECEIVED_CHAT;
		}
		else
		{
			rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_DATA_RECEIVED;
		}
		rtsMessage.m_sessionAddress = args->SessionAddress;
		rtsMessage.m_dataSize = args->Data->Length;
		rtsMessage.m_pucData = static_cast<unsigned char *>(malloc(rtsMessage.m_dataSize));
		memcpy( rtsMessage.m_pucData, args->Data->Data, rtsMessage.m_dataSize );
		m_pDQRNet->m_RTSMessageQueueIncoming.push(rtsMessage);
		LeaveCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
	}
}

// This event handler is called by the realtime session layer, when session address data is updated. We don't currently use session address data.
void DQRNetworkManagerEventHandlers::SessionAddressDataChangedHandler(Platform::Object^ session, WXNRs::SessionAddressDataChangedEventArgs^ args)
{
	DQRNetworkManager::LogComment(L"SessionAddressDataChangedHandler");
}

// This event handler is called by the realtime session layer when a session changes status. We use this to determine that a connection has been made made to the host,
// and the case when a connection has been terminated.
void DQRNetworkManagerEventHandlers::SessionStatusUpdateHandler(Platform::Object^ session, WXNRs::SessionStatusUpdateEventArgs^ args)
{
	DQRNetworkManager::LogComment(L"SessionStatusUpdateHandler");
    if (m_pDQRNet->m_XRNS_Session == session)
    {
        switch(args->NewStatus)
        {
            case WXNRs::SessionStatus::Active:
				{
					DQRNetworkManager::LogComment(L"Session active");
					m_pDQRNet->m_XRNS_LocalAddress = m_pDQRNet->m_XRNS_Session->LocalSessionAddress;
					m_pDQRNet->m_XRNS_OldestAddress = m_pDQRNet->m_XRNS_Session->OldestSessionAddress;
					EnterCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
					DQRNetworkManager::RTS_Message rtsMessage;
					rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_STATUS_ACTIVE;
					rtsMessage.m_sessionAddress = 0;
					rtsMessage.m_dataSize = 0;
					rtsMessage.m_pucData = 0;
					m_pDQRNet->m_RTSMessageQueueIncoming.push(rtsMessage);
					LeaveCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
				}
                break;
            case WXNRs::SessionStatus::Terminated:
				{
					DQRNetworkManager::LogComment(L"Session terminated");
					EnterCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
					DQRNetworkManager::RTS_Message rtsMessage;
					rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_STATUS_TERMINATED;
					rtsMessage.m_sessionAddress = 0;
					rtsMessage.m_dataSize = 0;
					rtsMessage.m_pucData = 0;
					m_pDQRNet->m_RTSMessageQueueIncoming.push(rtsMessage);
					LeaveCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
				}
				break;
            case WXNRs::SessionStatus::Activating:
                DQRNetworkManager::LogComment(L"Session activating");
                break;
			case WXNRs::SessionStatus::Terminating:
                DQRNetworkManager::LogComment(L"Session terminating");
                break;
        }
    }
}

// This event is called from the realtime session layer to notify any clients that a new endpoint has been connected into the network mesh.
void DQRNetworkManagerEventHandlers::AddedSessionAddressHandler(Platform::Object^ session, WXNRs::AddedSessionAddressEventArgs^ args)
{
	DQRNetworkManager::LogCommentFormat(L"AddedSessionAddressHandler session address 0x%x",args->SessionAddress);
	EnterCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
	DQRNetworkManager::RTS_Message rtsMessage;
	rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_ADDED_SESSION_ADDRESS;
	rtsMessage.m_sessionAddress = args->SessionAddress;
	rtsMessage.m_dataSize = 0;
	rtsMessage.m_pucData = 0;
	m_pDQRNet->m_RTSMessageQueueIncoming.push(rtsMessage);
	LeaveCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
}

// This event is called from the realtime session layer to notify any clients that an endpoint has been removed from the network mesh.
void DQRNetworkManagerEventHandlers::RemovedSessionAddressHandler(Platform::Object^ session, WXNRs::RemovedSessionAddressEventArgs^ args)
{
	DQRNetworkManager::LogCommentFormat(L"RemovedSessionAddressHandler session address 0x%x", args->SessionAddress);
	EnterCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
	DQRNetworkManager::RTS_Message rtsMessage;
	rtsMessage.m_eType = DQRNetworkManager::eRTSMessageType::RTS_MESSAGE_REMOVED_SESSION_ADDRESS;
	rtsMessage.m_sessionAddress = args->SessionAddress;
	rtsMessage.m_dataSize = 0;
	rtsMessage.m_pucData = 0;
	m_pDQRNet->m_RTSMessageQueueIncoming.push(rtsMessage);
	LeaveCriticalSection(&m_pDQRNet->m_csRTSMessageQueueIncoming);
}

// This event is called from the realtime session layer when session global data has been updated. We don't currently use global session data.
void DQRNetworkManagerEventHandlers::GlobalSessionDataChangedHandler(Platform::Object^ session, WXNRs::GlobalSessionDataChangedEventArgs^ args)
{
	DQRNetworkManager::LogComment(L"GlobalSessionDataChangedHandler");
}

void DQRNetworkManager::UpdateRTSStats()
{
	Platform::Array<unsigned int> ^sessionAddresses = nullptr;
	try
	{
		sessionAddresses = m_XRNS_Session->GetAllRemoteSessionAddresses(WXNRs::RemoteSessionAddressStateOptions::All, WXNRs::RemoteSessionAddressConnectivityOptions::All);
	}
	catch(Platform::COMException^ ex)
	{
		// swallow exceptions
	}
	catch(...)
	{
		// swallow exceptions
	}

	if( sessionAddresses )
	{
		unsigned int totalBytes = 0;
		unsigned int totalSends = 0;
		for( unsigned int i = 0; i < sessionAddresses->Length; i++ )
		{
			try
			{
				totalBytes += m_XRNS_Session->GetSendChannelOutstandingBytes(sessionAddresses->get(i), WXNRs::ChannelId::DefaultGameSend );
				totalSends += m_XRNS_Session->GetSendChannelOutstandingSends(sessionAddresses->get(i), WXNRs::ChannelId::DefaultGameSend );
			}
			catch(Platform::COMException^ ex)
			{
				// swallow exceptions
			}
			catch(...)
			{
				// swallow exceptions
			}
		}
		m_RTS_Stat_totalBytes = totalBytes;
		m_RTS_Stat_totalSends = totalSends;
	}
	else
	{
		m_RTS_Stat_totalBytes = 0;
		m_RTS_Stat_totalSends = 0;
	}
}

void DQRNetworkManager::ProcessRTSMessagesIncoming()
{
	EnterCriticalSection(&m_csRTSMessageQueueIncoming);
	while(m_RTSMessageQueueIncoming.size() > 0 )
	{
		RTS_Message message = m_RTSMessageQueueIncoming.front();
		switch( message.m_eType )
		{
			case eRTSMessageType::RTS_MESSAGE_DATA_RECEIVED:
				Process_RTS_MESSAGE_DATA_RECEIVED(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_DATA_RECEIVED_CHAT:
				Process_RTS_MESSAGE_DATA_RECEIVED_CHAT(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_ADDED_SESSION_ADDRESS:
				Process_RTS_MESSAGE_ADDED_SESSION_ADDRESS(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_REMOVED_SESSION_ADDRESS:
				Process_RTS_MESSAGE_REMOVED_SESSION_ADDRESS(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_STATUS_ACTIVE:
				Process_RTS_MESSAGE_STATUS_ACTIVE(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_STATUS_TERMINATED:
				Process_RTS_MESSAGE_STATUS_TERMINATED(message);
				break;
			default:
				break;
		}
		m_RTSMessageQueueIncoming.pop();
	}
	LeaveCriticalSection(&m_csRTSMessageQueueIncoming);
};

void DQRNetworkManager::Process_RTS_MESSAGE_DATA_RECEIVED(RTS_Message &message)
{
	DQRConnectionInfo *connectionInfo;
	if( m_isHosting )
	{
		connectionInfo = m_sessionAddressToConnectionInfoMapHost[message.m_sessionAddress];
	}
	else
	{
		connectionInfo = &m_connectionInfoClient;
	}

	// Handle any header data, and actual data, in our stream. Data is as follows:
	// Byte 0    Byte 1
	// fccsssss  ssssssss
	//
	// Where: f				 is 0 if this is normal data send (to be passed up to the game), or is 1 if this is to be internally processed
	//		  cc             is the channel number that the data belongs to (0 to 3 representing actual player indices)
	//		  sssssssssssss  is the count of data bytes to follow (range 0 - 8191)
	BYTE *pNextByte = message.m_pucData;
	BYTE *pEndByte = pNextByte + message.m_dataSize;
	do
	{
		BYTE byte = *pNextByte;
		switch( connectionInfo->m_state )
		{
			case DQRConnectionInfo::ConnectionState_HeaderByte0:
				connectionInfo->m_currentChannel = ( byte >> 5 ) & 3;
				connectionInfo->m_internalFlag = ( ( byte & 0x80 ) == 0x80 );

				// Byte transfer mode. Bits 0-4 of this byte represent the upper 5 bits of our count of bytes to transfer... lower 8-bits will follow
				connectionInfo->m_bytesRemaining = ((int)( byte & 0x1f )) << 8;
				connectionInfo->m_state = DQRConnectionInfo::ConnectionState_HeaderByte1;
				connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalHeaderByte;
				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_HeaderByte1:
				// Add in the lower 8 bits of our byte count, the upper 5 were obtained from the first header byte.
				connectionInfo->m_bytesRemaining |= byte;

				// If there isn't any data following, then just go back to the initial state expecting another header byte.
				if( connectionInfo->m_bytesRemaining == 0 )
				{
					connectionInfo->m_state = DQRConnectionInfo::ConnectionState_HeaderByte0;
				}
				else
				{
					connectionInfo->m_state = DQRConnectionInfo::ConnectionState_ReadBytes;
				}
				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_ReadBytes:
				// At this stage we can send up to connectionInfo->m_bytesRemaining bytes, or the number of bytes that we have remaining in the data received, whichever is lowest.
				int bytesInBuffer = static_cast<int>(pEndByte - pNextByte);
				int bytesToReceive = ( ( connectionInfo->m_bytesRemaining < bytesInBuffer ) ? connectionInfo->m_bytesRemaining : bytesInBuffer );

				if( connectionInfo->m_internalFlag )
				{
					BytesReceivedInternal(connectionInfo, message.m_sessionAddress, pNextByte, bytesToReceive );
				}
				else
				{
					BytesReceived(connectionInfo->m_smallId[connectionInfo->m_currentChannel], pNextByte, bytesToReceive );
				}

				// Adjust counts and pointers
				pNextByte += bytesToReceive;
				connectionInfo->m_bytesRemaining -= bytesToReceive;

				// Set state back to expect a header if there is no more data bytes to receive
				if( connectionInfo->m_bytesRemaining == 0 )
				{
					connectionInfo->m_state = DQRConnectionInfo::ConnectionState_HeaderByte0;
				}
				break;
		}
	} while (pNextByte != pEndByte);

	free(message.m_pucData);
}

void DQRNetworkManager::Process_RTS_MESSAGE_DATA_RECEIVED_CHAT(RTS_Message &message)
{
	if( m_chat )
	{
		m_chat->OnIncomingChatMessage(message.m_sessionAddress, Platform::ArrayReference<BYTE>(message.m_pucData, message.m_dataSize) );
		free(message.m_pucData);
	}
}

void DQRNetworkManager::Process_RTS_MESSAGE_ADDED_SESSION_ADDRESS(RTS_Message &message)
{
	if( m_chat )
	{
		m_chat->OnNewSessionAddressAdded(message.m_sessionAddress);
	}

	// New session address - add a mapping for it
	if( m_isHosting )
	{
		auto it = m_sessionAddressToConnectionInfoMapHost.find(message.m_sessionAddress);
		DQRConnectionInfo *connectionInfo;
		if( it == m_sessionAddressToConnectionInfoMapHost.end() )
		{
			connectionInfo = new DQRConnectionInfo();

			m_sessionAddressToConnectionInfoMapHost[message.m_sessionAddress] = connectionInfo;
		}
		else
		{
			// This shouldn't happen as we should be removing mappings as session addresses are removed.
			connectionInfo = it->second;
			connectionInfo->Reset();
		}

	}
}

void DQRNetworkManager::Process_RTS_MESSAGE_REMOVED_SESSION_ADDRESS(RTS_Message &message)
{
	if( m_chat )
	{
		m_chat->RemoveRemoteConsole(message.m_sessionAddress);
	}

	if( m_isHosting )
	{
		auto it = m_sessionAddressToConnectionInfoMapHost.find(message.m_sessionAddress);
		
		if( it != m_sessionAddressToConnectionInfoMapHost.end() )
		{
			delete it->second;
			m_sessionAddressToConnectionInfoMapHost.erase(it);
			RemoveRoomSyncPlayersWithSessionAddress(message.m_sessionAddress);
			SendRoomSyncInfo();
		}
	}
	else
	{
		// As the client, if we are disonnected from the host, then it is all over. Proceed as if leaving the room.
		if( message.m_sessionAddress == m_hostSessionAddress )
		{
			LeaveRoom();
		}
	}
}

void DQRNetworkManager::Process_RTS_MESSAGE_STATUS_ACTIVE(RTS_Message &message)
{
	// When we detect that the session has become active, we start sending unreliable packets, until we get some data back. This is because there is an issue with the
	// realtime session layer where it is telling us that the connection is active a bit to early, and it will disconnect if it receives a packet that must be reliable in this
	// state.
	if( !m_isHosting )
	{
		m_firstUnreliableSendTime = 0;
		m_hostSessionAddress = m_XRNS_OldestAddress;
		// Also initialise the status of this connection
		m_connectionInfoClient.Reset();
		SetState(DQRNetworkManager::DNM_INT_STATE_JOINING_SENDING_UNRELIABLE);
	}
}

void DQRNetworkManager::Process_RTS_MESSAGE_STATUS_TERMINATED(RTS_Message &message)
{
	if( m_state == DQRNetworkManager::DNM_INT_STATE_JOINING_WAITING_FOR_ACTIVE_SESSION )
	{
		m_joinCreateSessionAttempts++;
		if( m_joinCreateSessionAttempts > DQRNetworkManager::JOIN_CREATE_SESSION_MAX_ATTEMPTS )
		{
			SetState(DQRNetworkManager::DNM_INT_STATE_JOINING_FAILED);
		}
		else
		{
			SetState(DQRNetworkManager::DNM_INT_STATE_JOINING_GET_SDA);
		}
	}
}

int DQRNetworkManager::_RTSDoWorkThread(void* lpParameter)
{
	DQRNetworkManager *pDQR = static_cast<DQRNetworkManager *>(lpParameter);
	return pDQR->RTSDoWorkThread();
}

static const DWORD XRNS_TERMINATE_LOCAL_SESSION_FLAG_IMMEDIATE  = 0x00000001;
int	DQRNetworkManager::RTSDoWorkThread()
{
	do
	{
		if( m_XRNS_Session )
		{
			try
			{
				m_XRNS_Session->DoWork(20);
			}
			catch(Platform::COMException^ ex)
			{
				// swallow exceptions
			}
			catch(...)
			{
				// swallow exceptions
			}
			UpdateRTSStats();
		}
		else
		{
			Sleep(20);
		}
		ProcessRTSMessagesOutgoing();
	} while(true);
}

void DQRNetworkManager::ProcessRTSMessagesOutgoing()
{
	EnterCriticalSection(&m_csRTSMessageQueueOutgoing);
	while(m_RTSMessageQueueOutgoing.size() > 0 )
	{
		RTS_Message message = m_RTSMessageQueueOutgoing.front();
		switch( message.m_eType )
		{
			case eRTSMessageType::RTS_MESSAGE_START_CLIENT:
				Process_RTS_MESSAGE_START_CLIENT(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_START_HOST:
				Process_RTS_MESSAGE_START_HOST(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_TERMINATE:
				Process_RTS_MESSAGE_TERMINATE(message);
				break;
			case eRTSMessageType::RTS_MESSAGE_SEND_DATA:
				Process_RTS_MESSAGE_SEND_DATA(message);
				break;
			default:
				break;
		}
		m_RTSMessageQueueOutgoing.pop();
	}
	LeaveCriticalSection(&m_csRTSMessageQueueOutgoing);
};

void DQRNetworkManager::Process_RTS_MESSAGE_START_CLIENT(RTS_Message &message)
{
	if( m_XRNS_Session )
	{
		m_eventHandlers->Pulldown(m_XRNS_Session);
		// Close XRNS session
		try
		{
			m_XRNS_Session->TerminateLocalSession(XRNS_TERMINATE_LOCAL_SESSION_FLAG_IMMEDIATE);
		}
		catch(Platform::COMException^ ex)
		{
			// swallow exceptions
		}
		catch(...)
		{
			// swallow exceptions
		}

		m_XRNS_Session = nullptr;
	}

	m_XRNS_Session = ref new WXNRs::Session( m_localSocketAddress, m_remoteSocketAddress, MAX_PLAYERS_IN_TEMPLATE, 0);
	m_XRNS_Session->MinSendRate = 512000;

	LogCommentFormat(L"connect retry period %d retries %d, data retry count %d, data retry timeout %d\n",m_XRNS_Session->ConnectRetryPeriod,m_XRNS_Session->MaxConnectRetries,m_XRNS_Session->MaxDataRetries,m_XRNS_Session->MinDataRetryTimeout);

	m_XRNS_Session->MaxConnectRetries = 50;			// 50 at 100ms intervals = 5 seconds of attempting to connect

	m_eventHandlers->Setup(m_XRNS_Session);
}

void DQRNetworkManager::Process_RTS_MESSAGE_START_HOST(RTS_Message &message)
{
	m_XRNS_Session = ref new WXNRs::Session( m_localSocketAddress, MAX_PLAYERS_IN_TEMPLATE, 0);
	m_XRNS_Session->MinSendRate = 512000;
	m_XRNS_Session->MaxConnectRetries = 50;			// 50 at 100ms intervals = 5 seconds of attempting to connect
	m_eventHandlers->Setup(m_XRNS_Session);
}

void DQRNetworkManager::Process_RTS_MESSAGE_TERMINATE(RTS_Message &message)
{
	if( m_XRNS_Session )
	{
		m_eventHandlers->Pulldown(m_XRNS_Session);
		// Close XRNS session
		try
		{
			m_XRNS_Session->TerminateLocalSession(XRNS_TERMINATE_LOCAL_SESSION_FLAG_IMMEDIATE);
		}
		catch(Platform::COMException^ ex)
		{
			// swallow exceptions
		}
		catch(...)
		{
			// swallow exceptions
		}

		m_XRNS_Session = nullptr;
	}
}

void DQRNetworkManager::Process_RTS_MESSAGE_SEND_DATA(RTS_Message &message)
{
	if( m_XRNS_Session )
	{
		unsigned int sessionAddress = message.m_sessionAddress;

		try
		{
			if( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_BROADCAST_MODE )
			{
				sessionAddress = m_XRNS_Session->LocalSessionAddress;
			}

			m_XRNS_Session->Send( ( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_GAME_CHANNEL ) ? WXNRs::ChannelId::DefaultGameSend : WXNRs::ChannelId::DefaultChatSend,
								( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_BROADCAST_MODE ) ? WXNRs::SendExceptionType::ExcludedAddresses : WXNRs::SendExceptionType::IncludedAddresses,
								Platform::ArrayReference<unsigned int>(&message.m_sessionAddress, 1),
								Platform::ArrayReference<BYTE>(message.m_pucData, message.m_dataSize),
								0,
								( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_RELIABLE ) ? WXNRs::Send_Reliability::Reliable : WXNRs::Send_Reliability::NonReliable,
								( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_SEQUENTIAL ) ? WXNRs::Send_Sequence::Sequential : WXNRs::Send_Sequence::NonSequential,
								WXNRs::Send_Ack::AckNormal,
								( message.m_flags & eRTSFlags::RTS_MESSAGE_FLAG_COALESCE ) ? WXNRs::Send_Coalesce::CoalesceDelay : WXNRs::Send_Coalesce::CoalesceNever,
								WXNRs::Send_MiscState::NoMiscState );
		}
		catch(Platform::COMException^ ex)
		{
			// swallow exceptions
		}
		catch(...)
		{
			// swallow exceptions
		}
	}
	free(message.m_pucData);
}

void DQRNetworkManager::RTS_StartCient()
{
	EnterCriticalSection(&m_csRTSMessageQueueOutgoing);
	RTS_Message message;
	message.m_eType = eRTSMessageType::RTS_MESSAGE_START_CLIENT;
	message.m_pucData = nullptr;
	message.m_dataSize = 0;
	m_RTSMessageQueueOutgoing.push(message);
	LeaveCriticalSection(&m_csRTSMessageQueueOutgoing);
}

void DQRNetworkManager::RTS_StartHost()
{
	EnterCriticalSection(&m_csRTSMessageQueueOutgoing);
	RTS_Message message;
	message.m_eType = eRTSMessageType::RTS_MESSAGE_START_HOST;
	message.m_pucData = nullptr;
	message.m_dataSize = 0;
	m_RTSMessageQueueOutgoing.push(message);
	LeaveCriticalSection(&m_csRTSMessageQueueOutgoing);
}

void DQRNetworkManager::RTS_Terminate()
{
	EnterCriticalSection(&m_csRTSMessageQueueOutgoing);
	RTS_Message message;
	message.m_eType = eRTSMessageType::RTS_MESSAGE_TERMINATE;
	message.m_pucData = nullptr;
	message.m_dataSize = 0;
	m_RTSMessageQueueOutgoing.push(message);
	LeaveCriticalSection(&m_csRTSMessageQueueOutgoing);
}

void DQRNetworkManager::RTS_SendData(unsigned char *pucData, unsigned int dataSize, unsigned int sessionAddress, bool reliable, bool sequential, bool coalesce, bool broadcastMode, bool gameChannel )
{
	EnterCriticalSection(&m_csRTSMessageQueueOutgoing);
	RTS_Message message;
	message.m_eType = eRTSMessageType::RTS_MESSAGE_SEND_DATA;
	message.m_pucData = static_cast<unsigned char *>(malloc(dataSize));
	memcpy(message.m_pucData, pucData, dataSize);
	message.m_dataSize = dataSize;
	message.m_sessionAddress = sessionAddress;
	message.m_flags = 0;
	if( reliable ) message.m_flags |= eRTSFlags::RTS_MESSAGE_FLAG_RELIABLE;
	if( sequential ) message.m_flags |= eRTSFlags::RTS_MESSAGE_FLAG_SEQUENTIAL;
	if( coalesce ) message.m_flags |= eRTSFlags::RTS_MESSAGE_FLAG_COALESCE;
	if( broadcastMode ) message.m_flags |= eRTSFlags::RTS_MESSAGE_FLAG_BROADCAST_MODE;
	if( gameChannel ) message.m_flags |= eRTSFlags::RTS_MESSAGE_FLAG_GAME_CHANNEL;
	m_RTSMessageQueueOutgoing.push(message);
	LeaveCriticalSection(&m_csRTSMessageQueueOutgoing);
}