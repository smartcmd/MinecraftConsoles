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

// This method is called when bytes have been received that are to be passed on to the game itself. The data is associated with a small id so we can specify which network player
// that it was received for.
void DQRNetworkManager::BytesReceived(int smallId, BYTE *bytes, int byteCount)
{
	DQRNetworkPlayer *host = GetPlayerBySmallId(m_hostSmallId);
	DQRNetworkPlayer *client = GetPlayerBySmallId(smallId);

	if( ( host == nullptr ) || ( client == nullptr ) )
	{
		return;
	}

	if( m_isHosting )
	{
		m_listener->HandleDataReceived(client, host, bytes, byteCount );
	}
	else
	{
		m_listener->HandleDataReceived(host, client, bytes, byteCount );
	}
//	app.DebugPrintf("%d bytes received: %s\n", byteCount, bytes);
}

// This method is called when network data is received, that is to be processed by the DQRNetworkManager itself. This is for handling internal
// updates such as assigning & unassigning of small Ids, transmission of the table of players currently in the session etc.
// Processing of these things is handled as a state machine so that we can receive a message split over more than one call to this method should
// the underlying communcation layer split data up somehow.
void DQRNetworkManager::BytesReceivedInternal(DQRConnectionInfo *connectionInfo, unsigned int sessionAddress, BYTE *bytes, int byteCount)
{
	BYTE *pNextByte = bytes;
	BYTE *pEndByte = pNextByte + byteCount;

	do
	{
		BYTE byte = *pNextByte;
		switch( connectionInfo->m_internalDataState )
		{
			case DQRConnectionInfo::ConnectionState_InternalHeaderByte:
				switch( byte )
				{
					case DQR_INTERNAL_ASSIGN_SMALL_IDS:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAssignSmallIdMask;
						break;
					case DQR_INTERNAL_UNASSIGN_SMALL_ID:
						// Host only
						if( connectionInfo->m_channelActive[connectionInfo->m_currentChannel] )
						{
							int smallId = connectionInfo->m_smallId[connectionInfo->m_currentChannel];
							connectionInfo->m_channelActive[connectionInfo->m_currentChannel] = false;
							m_sessionAddressFromSmallId[smallId] = 0;
							DQRNetworkPlayer *pPlayer = GetPlayerBySmallId(smallId);
							if( pPlayer )
							{
								RemoveRoomSyncPlayer(pPlayer);
								SendRoomSyncInfo();
							}
						}
						break;
					case DQR_INTERNAL_PLAYER_TABLE:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalRoomSyncData;
						connectionInfo->m_pucRoomSyncData = new unsigned char[4];
						connectionInfo->m_roomSyncDataBytesToRead = 0;
						connectionInfo->m_roomSyncDataBytesRead = 0;
						break;
					case DQR_INTERNAL_ADD_PLAYER_FAILED:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAddPlayerFailedData;
						connectionInfo->m_pucAddFailedPlayerData = new unsigned char[4];
						connectionInfo->m_addFailedPlayerDataBytesToRead = 0;
						connectionInfo->m_addFailedPlayerDataBytesRead = 0;
						break;
					default:
						break;
				}
				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_InternalAssignSmallIdMask:
				// Up to 4 smallIds are assigned at once, with the ones that are being assigned dictated by a mask byte which is passed in first.
				// The small Ids themselves follow, always 4 bytes, and any that are masked as being assigned are processed, the other bytes ignored.
				// In order work around a bug with the networking library, this particular packet (being the first this that is sent from a client)
				// is at first sent unreliably, with retries, until a message is received back to the client, or it times out. We therefore have to be able
				// to handle (and ignore) this being received more than once
				DQRNetworkManager::LogCommentFormat(L"Small Ids being received");
				connectionInfo->m_smallIdReadMask = byte;
				// Create a uniquely allocated byte to which names have been resolved, as another one of these packets could be received whilst that asyncronous process is going o n
				connectionInfo->m_pucsmallIdReadMaskResolved = new unsigned char;
				*connectionInfo->m_pucsmallIdReadMaskResolved = 0;
				connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAssignSmallId0;
				connectionInfo->m_initialPacketReceived = true;

				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_InternalAssignSmallId0:
			case DQRConnectionInfo::ConnectionState_InternalAssignSmallId1:
			case DQRConnectionInfo::ConnectionState_InternalAssignSmallId2:
			case DQRConnectionInfo::ConnectionState_InternalAssignSmallId3:
				{
					int channel = static_cast<int>(connectionInfo->m_internalDataState) - DQRConnectionInfo::ConnectionState_InternalAssignSmallId0;

					if( ( connectionInfo->m_smallIdReadMask & ( 1 << channel ) ) && ( !connectionInfo->m_channelActive[channel] ) )
					{
						// HOST ONLY
						// Store the small Id that is associated with this send channel. In order work around a bug with the networking library, this particular packet
						// (being the first this that is sent from a client) is sent unreliably, with retries, until a message is received back to the client, or it times out.
						// We therefore have to be able to handle (and ignore) this being received more than once - hence the check of the bool above.
						// At this point, the connection is considered properly active from the point of view of the host.

						int sessionIndex = GetSessionIndexForSmallId(byte);
						if( sessionIndex != -1 )
						{
							connectionInfo->m_channelActive[channel] = true;
							connectionInfo->m_smallId[channel] = byte;
					
							m_sessionAddressFromSmallId[byte] = sessionAddress;
							m_channelFromSmallId[byte] = channel;

							auto pAsyncOp = m_primaryUserXboxLiveContext->ProfileService->GetUserProfileAsync(m_multiplayerSession->Members->GetAt(sessionIndex)->XboxUserId);
							DQRNetworkManager::LogCommentFormat(L"Session index of %d found for player with small id %d - attempting to resolve display name\n",sessionIndex,byte);

							DQRNetworkPlayer *pPlayer = new DQRNetworkPlayer(this, DQRNetworkPlayer::DNP_TYPE_REMOTE, true, 0, sessionAddress);
							pPlayer->SetSmallId(byte);
							pPlayer->SetUID(static_cast<PlayerUID>(m_multiplayerSession->Members->GetAt(sessionIndex)->XboxUserId->Data()));

							HostGamertagResolveDetails *resolveDetails = new HostGamertagResolveDetails();
							resolveDetails->m_pPlayer = pPlayer;
							resolveDetails->m_sessionAddress = sessionAddress;
							resolveDetails->m_channel = channel;
							resolveDetails->m_sync = false;

							int mask = 1 << channel;
							unsigned char *pucsmallIdReadMaskResolved = connectionInfo->m_pucsmallIdReadMaskResolved;
							unsigned char ucsmallIdReadMask = connectionInfo->m_smallIdReadMask;
							create_task( pAsyncOp ).then( [this,resolveDetails,mask,pucsmallIdReadMaskResolved,ucsmallIdReadMask] (task<Microsoft::Xbox::Services::Social::XboxUserProfile^> resultTask)
							{
								try
								{
									Microsoft::Xbox::Services::Social::XboxUserProfile^ result = resultTask.get();

									resolveDetails->m_name.assign(result->Gamertag->Data());		// Use the gamertag for this data, as it is synchronised round all the machines and so we can't use a display name that could be a real name

									EnterCriticalSection(&m_csHostGamertagResolveResults);
									// Update flags for which names have been resolved, and if this completes this set, then set the flag to say that we should synchronise these out to the clients
									*pucsmallIdReadMaskResolved |= mask;
									LogCommentFormat(L"<<>> Compare %d to %d",*pucsmallIdReadMaskResolved,ucsmallIdReadMask);
									if(ucsmallIdReadMask == *pucsmallIdReadMaskResolved)
									{
										resolveDetails->m_sync = true;
										delete pucsmallIdReadMaskResolved;
									}
									m_hostGamertagResolveResults.push(resolveDetails);
									LeaveCriticalSection(&m_csHostGamertagResolveResults);
								}

								catch (Platform::Exception^ ex)
								{
									LogComment("Name resolve exception raised");
									// TODO - handle errors more usefully than just not setting the name...
									EnterCriticalSection(&m_csHostGamertagResolveResults);
									// Update flags for which names have been resolved, and if this completes this set, then set the flag to say that we should synchronise these out to the clients
									*pucsmallIdReadMaskResolved |= mask;
									if(ucsmallIdReadMask == *pucsmallIdReadMaskResolved)
									{
										resolveDetails->m_sync = true;
										delete pucsmallIdReadMaskResolved;
									}
									m_hostGamertagResolveResults.push(resolveDetails);
									LeaveCriticalSection(&m_csHostGamertagResolveResults);
								}
							});
						}
					}
				}

				switch(connectionInfo->m_internalDataState)
				{
					case DQRConnectionInfo::ConnectionState_InternalAssignSmallId0:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAssignSmallId1;
						break;
					case DQRConnectionInfo::ConnectionState_InternalAssignSmallId1:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAssignSmallId2;
						break;
					case DQRConnectionInfo::ConnectionState_InternalAssignSmallId2:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalAssignSmallId3;
						break;
					case DQRConnectionInfo::ConnectionState_InternalAssignSmallId3:
						connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalHeaderByte;
						break;
				}

				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_InternalRoomSyncData:
				connectionInfo->m_pucRoomSyncData[connectionInfo->m_roomSyncDataBytesRead++] = byte;
				// The room sync info is sent as a 4 byte count of the length of XUID strings, then the RoomSyncData, then the XUID strings
				if( connectionInfo->m_roomSyncDataBytesToRead == 0 )
				{
					// At first stage of reading the 4 byte count
					if( connectionInfo->m_roomSyncDataBytesRead == 4 )
					{
						memcpy( &connectionInfo->m_roomSyncDataBytesToRead, connectionInfo->m_pucRoomSyncData, 4);
						delete [] connectionInfo->m_pucRoomSyncData;
						connectionInfo->m_roomSyncDataBytesToRead += sizeof(RoomSyncData);
						connectionInfo->m_pucRoomSyncData = new unsigned char[ connectionInfo->m_roomSyncDataBytesToRead ];
						connectionInfo->m_roomSyncDataBytesRead = 0;
					}
				}
				else if( connectionInfo->m_roomSyncDataBytesRead == connectionInfo->m_roomSyncDataBytesToRead )
				{
					// Second stage of reading the variable length data - when we've read this all, we can created storage for the XUID strings and copy them all in
					RoomSyncData *roomSyncData = (RoomSyncData *)connectionInfo->m_pucRoomSyncData;
					wchar_t *pwcsData = (wchar_t *)((unsigned char *)connectionInfo->m_pucRoomSyncData + sizeof(RoomSyncData));
					for( int i = 0; i < roomSyncData->playerCount; i++ )
					{
						unsigned int thisWchars = ( wcslen(pwcsData) + 1 );
						roomSyncData->players[i].m_XUID = new wchar_t[thisWchars];
						wcsncpy(roomSyncData->players[i].m_XUID, pwcsData, thisWchars);
						pwcsData += thisWchars;
					}
					// Update the room sync data with this new data. This will handle notification of new and removed players
					UpdateRoomSyncPlayers((RoomSyncData *)connectionInfo->m_pucRoomSyncData);
					
					delete connectionInfo->m_pucRoomSyncData;
					connectionInfo->m_pucRoomSyncData = nullptr;
					connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalHeaderByte;

					// If we haven't actually established a connection yet for this channel, then this is the point where we can consider this active
					if( !connectionInfo->m_channelActive[connectionInfo->m_currentChannel] )
					{
						DQRNetworkManager::LogCommentFormat(L"Received data from host, channel %d considered active (%d bytes)\n",connectionInfo->m_currentChannel,connectionInfo->m_bytesRemaining);
						connectionInfo->m_channelActive[connectionInfo->m_currentChannel] = true;

						// This is also the time (as a client) to inform the chat integration layer of the host's session address, since we can now (reliably) send data to it
						if(connectionInfo->m_currentChannel == 0)
						{
							if( m_chat )
							{
								m_chat->OnNewSessionAddressAdded(m_hostSessionAddress);
							}
						}
					}

					// Move to starting & playing states, if we are still joining rather than adding an additional player from this client, and we have all the local players here.
					// We need to check that they are all here because we could have received a broadcast room sync data caused by another machine joining, and and so we can't assume
					// that we're ready to go just yet.
					if( m_state == DQRNetworkManager::DNM_INT_STATE_JOINING_SENDING_UNRELIABLE )
					{
						bool allLocalPlayersHere = true;
						for( int i = 0; i < MAX_LOCAL_PLAYERS; i++ )
						{
							if( m_currentUserMask & ( 1 << i ) )
							{
								if( GetLocalPlayerByUserIndex(i) == nullptr )
								{
									allLocalPlayersHere = false;
								}
							}
						}
						if( allLocalPlayersHere )
						{
							DQRNetworkManager::LogComment(L"All local players present");
							SetState(DQRNetworkManager::DNM_INT_STATE_STARTING);
							SetState(DQRNetworkManager::DNM_INT_STATE_PLAYING);
						}
						else
						{
							// Our players aren't all here yet. Going to keep on sending unreliable packets though until the connection is up and running
							DQRNetworkManager::LogComment(L"All local players not yet present");
						}
					}
				}
				pNextByte++;
				break;
			case DQRConnectionInfo::ConnectionState_InternalAddPlayerFailedData:
				connectionInfo->m_pucAddFailedPlayerData[connectionInfo->m_addFailedPlayerDataBytesRead++] = byte;
				// The failed player info is sent as a 4 byte count of the length of XUID string, then the string itself
				if( connectionInfo->m_addFailedPlayerDataBytesToRead == 0 )
				{
					// At first stage of reading the 4 byte count
					if( connectionInfo->m_addFailedPlayerDataBytesRead == 4 )
					{
						memcpy( &connectionInfo->m_addFailedPlayerDataBytesToRead, connectionInfo->m_pucAddFailedPlayerData, 4);
						delete [] connectionInfo->m_pucAddFailedPlayerData;
						connectionInfo->m_pucAddFailedPlayerData = new unsigned char[ connectionInfo->m_addFailedPlayerDataBytesToRead ];
						connectionInfo->m_addFailedPlayerDataBytesRead = 0;
					}
				}
				else if( connectionInfo->m_addFailedPlayerDataBytesRead == connectionInfo->m_addFailedPlayerDataBytesToRead )
				{
					// XUID fully read, can now handle what to do with it
					AddPlayerFailed(ref new Platform::String( (wchar_t *)connectionInfo->m_pucAddFailedPlayerData ) );
					delete [] connectionInfo->m_pucAddFailedPlayerData;
					connectionInfo->m_pucAddFailedPlayerData = nullptr;
					connectionInfo->m_internalDataState = DQRConnectionInfo::ConnectionState_InternalHeaderByte;
				}

				pNextByte++;
				break;
		}
	} while (pNextByte != pEndByte);
}

// This method directly sends bytes via the network communication layer, used to send both game data & data internal to the DQRNetworkManager itself.
// This is used by higher level sending methods that wrap communications up with headers that can be processed at the receiving end. 
void DQRNetworkManager::SendBytesRaw(int smallId, BYTE *bytes, int byteCount, bool reliableAndSequential)
{
	bool broadcast;
	unsigned int sessionAddress;

//	app.DebugPrintf("{%d,%d - %d}\n",smallId,reliableAndSequential,byteCount);

	if( smallId == -1 )
	{
		LogCommentFormat(L"Attempting broadcast, exception of address m_XRNS_Session->LocalSessionAddress %d %d %d", smallId, byteCount, reliableAndSequential);
		// Broadcast, used from host only
		broadcast = true;
		sessionAddress = 0;
	}
	else
	{
		// Send to individual session address
		broadcast = false;
		if( m_isHosting )
		{
			sessionAddress = m_sessionAddressFromSmallId[ smallId ];
		}
		else
		{
			sessionAddress = m_hostSessionAddress;
		}
	}
	RTS_SendData(bytes, byteCount, sessionAddress, reliableAndSequential, reliableAndSequential, reliableAndSequential, broadcast, true);
}

// This method is called by the chat integration layer to be able to send data
void DQRNetworkManager::SendBytesChat(unsigned int address, BYTE *bytes, int byteCount, bool reliable, bool sequential, bool broadcast)
{
	unsigned int sessionAddress;

	if( broadcast )
	{
		sessionAddress = 0;
	}
	else
	{
		// Send to individual session address
		sessionAddress = address;
	}
	
	RTS_SendData(bytes, byteCount, sessionAddress, reliable, sequential, false, broadcast, false);
}

// This is the higher level sending method for sending game data - this prefixes the send with a header so that it will get routed to the correct player.
void DQRNetworkManager::SendBytes(int smallId, BYTE *bytes, int byteCount)
{
	EnterCriticalSection(&m_csSendBytes);
	unsigned char *tempSendBuffer = static_cast<unsigned char *>(malloc(8191 + 2));

	BYTE *data = bytes;
	BYTE *dataEnd = bytes + byteCount;

	// Data to be sent has a header to say which of our own internal channels it is on (2 bits), and number of bytes that are in the message.
	// The number of bytes has to be stored in 13 bits, and so a maximum of 8191 bytes can be send at a time. Split up longer messages into
	// blocks of this size.
	do
	{
		int bytesToSend = static_cast<int>(dataEnd - data);
		if( bytesToSend > 8191 ) bytesToSend = 8191;

		// Send header with data sending mode - see full comment in DQRNetworkManagerEventHandlers::DataReceivedHandler
		tempSendBuffer[0] = ( m_channelFromSmallId[smallId] << 5 ) | ( bytesToSend >> 8 );
		tempSendBuffer[1] = bytesToSend & 0xff;
		memcpy(&tempSendBuffer[2], data, bytesToSend);

		SendBytesRaw(smallId, tempSendBuffer, bytesToSend + 2, true);

		data += bytesToSend;
	} while (data != dataEnd);

	free(tempSendBuffer);
	LeaveCriticalSection(&m_csSendBytes);
}

int DQRNetworkManager::GetQueueSizeBytes()
{
	return m_RTS_Stat_totalBytes;
}

int DQRNetworkManager::GetQueueSizeMessages()
{
	return m_RTS_Stat_totalSends;
}