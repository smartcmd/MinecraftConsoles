#include "stdafx.h"
#include "IUIScene_PauseMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\MinecraftServer.h"
#include "..\..\MultiPlayerLevel.h"
#include "..\..\ProgressRenderer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\..\DLCTexturePack.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

#ifndef _XBOX
#include "UI.h"
#endif


int IUIScene_PauseMenu::ExitGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
#ifdef _XBOX
	IUIScene_PauseMenu *pScene = (IUIScene_PauseMenu *)pParam;
#else
	IUIScene_PauseMenu *pScene = dynamic_cast<IUIScene_PauseMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));
#endif

	// Results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		if(pScene) pScene->SetIgnoreInput(true);
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}


int IUIScene_PauseMenu::ExitGameSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
#ifdef _XBOX
	IUIScene_PauseMenu *pScene = (IUIScene_PauseMenu *)pParam;
#else
	IUIScene_PauseMenu *pScene = dynamic_cast<IUIScene_PauseMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));
#endif

	// Exit with or without saving
	// Decline means save in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultThirdOption) 
	{
		if( result==C4JStorage::EMessage_ResultDecline ) // Save
		{
			// 4J-PB - Is the player trying to save but they are using a trial texturepack ?
			if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tPack);

				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{					
#ifdef _XBOX
					// upsell
					ULONGLONG ullOfferID_Full;
					// get the dlc texture pack
					DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

					app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

					// tell sentient about the upsell of the full version of the skin pack
					TelemetryManager->RecordUpsellPresented(iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					// Give the player a warning about the trial version of the texture pack
					ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad() , &IUIScene_PauseMenu::WarningTrialTexturePackReturned, pParam);

					return S_OK;					
				}
			}

			// does the save exist?
			bool bSaveExists;
			StorageManager.DoesSaveExist(&bSaveExists);
			// 4J-PB - we check if the save exists inside the libs
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(), &IUIScene_PauseMenu::ExitGameAndSaveReturned, pParam);
				return 0;
			}
			else
			{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
				StorageManager.SetSaveDisabled(false);
#endif
				MinecraftServer::getInstance()->setSaveOnExit( true );
			}
		}
		else
		{
			// been a few requests for a confirm on exit without saving
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestAlertMessage(IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(), &IUIScene_PauseMenu::ExitGameDeclineSaveReturned, pParam);
			return 0;
		}

		if(pScene) pScene->SetIgnoreInput(true);

		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}


int IUIScene_PauseMenu::ExitGameAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// 4J-PB - we won't come in here if we have a trial texture pack
#ifdef _XBOX
	IUIScene_PauseMenu *pScene = (IUIScene_PauseMenu *)pParam;
#else
	IUIScene_PauseMenu *pScene = dynamic_cast<IUIScene_PauseMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));
#endif

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		//INT saveOrCheckpointId = 0;
		//bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
		//SentientManager.RecordLevelSaveOrCheckpoint(ProfileManager.GetPrimaryPad(), saveOrCheckpointId);
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		StorageManager.SetSaveDisabled(false);
#endif
		if(pScene) pScene->SetIgnoreInput(true);
		MinecraftServer::getInstance()->setSaveOnExit( true );
		// flag a app action of exit game
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	else
	{
		// has someone disconnected the ethernet here, causing the pause menu to shut?
		if(ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			UINT uiIDA[3];
			// you cancelled the save on exit after choosing exit and save? You go back to the Exit choices then.
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_EXIT_GAME_SAVE;
			uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

			if(g_NetworkManager.GetPlayerCount()>1)
			{
				ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, ProfileManager.GetPrimaryPad(), &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
			}
			else
			{
				ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, ProfileManager.GetPrimaryPad(), &IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
			}
		}
	}
	return 0;
}



int IUIScene_PauseMenu::ExitGameDeclineSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
#ifdef _XBOX
	IUIScene_PauseMenu *pScene = (IUIScene_PauseMenu *)pParam;
#else
	IUIScene_PauseMenu *pScene = dynamic_cast<IUIScene_PauseMenu *>(ui.GetSceneFromCallbackId((size_t)pParam));
#endif

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		// Don't do this here, as it will still try and save some things even though it shouldn't!
		//StorageManager.SetSaveDisabled(false);
#endif
		if(pScene) pScene->SetIgnoreInput(true);
		MinecraftServer::getInstance()->setSaveOnExit( false );
		// flag a app action of exit game
		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	else
	{
		// has someone disconnected the ethernet here, causing the pause menu to shut?
		if(ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()))
		{
			UINT uiIDA[3];
			// you cancelled the save on exit after choosing exit and save? You go back to the Exit choices then.
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_EXIT_GAME_SAVE;
			uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

			if(g_NetworkManager.GetPlayerCount()>1)
			{
				ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
			}
			else
			{
				ui.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameSaveDialogReturned, pParam);
			}
		}

	}
	return 0;
}



int IUIScene_PauseMenu::WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(!ProfileManager.IsSignedInLive(iPad))
		{
			// you're not signed in to PSN!

		}
		else
		{
			// 4J-PB - need to check this user can access the store
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(iPad,true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);
			}
			else
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s\n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);		

				if(pSONYDLCInfo!=nullptr)
				{
					char chName[42];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want
#ifdef __ORBIS__
					sprintf(chName,"%s",pSONYDLCInfo->chDLCKeyname);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),pSONYDLCInfo->chDLCKeyname);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);
					// 4J-PB - need to check for an empty store
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{					
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);	
						}
					}
				}
			}
		}
	}
#endif		//	

#ifdef _XBOX_ONE
	IUIScene_PauseMenu* pScene = (IUIScene_PauseMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
			if (ProfileManager.IsSignedInLive(iPad))
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				// get the dlc texture pack
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();

				DLC_INFO *pDLCInfo=app.GetDLCInfoForProductName((WCHAR *)pDLCPack->getName().c_str());

				StorageManager.InstallOffer(1,(WCHAR *)pDLCInfo->wsProductId.c_str(),nullptr,nullptr);

				// the license change coming in when the offer has been installed will cause this scene to refresh	
			}
			else
			{	
				// 4J-JEV: Fix for XB1: #165863 - XR-074: Compliance: With no active network connection user is unable to convert from Trial to Full texture pack and is not messaged why.
				UINT uiIDA[1] = { IDS_CONFIRM_OK };
				ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad); 
			}
		}
	}
	
#endif

#ifdef _XBOX	
	IUIScene_PauseMenu* pScene = (IUIScene_PauseMenu*)pParam;

	//pScene->m_bIgnoreInput = false;
	pScene->ShowScene( true );
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
			ULONGLONG ullIndexA[1];

			TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
			// get the dlc texture pack
			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

			// Need to get the parent packs id, since this may be one of many child packs with their own ids
			app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullIndexA[0]);
	
			// need to allow downloads here, or the player would need to quit the game to let the download of a texture pack happen. This might affect the network traffic, since the download could take all the bandwidth...
			XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( pScene->m_pDLCPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}
#endif


	return 0;
}


int IUIScene_PauseMenu::SaveWorldThreadProc( LPVOID lpParameter )
{
	bool bAutosave=static_cast<bool>(lpParameter);
	if(bAutosave)
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_AutoSaveGame);
	}
	else
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_SaveGame);
	}

	// Share AABB & Vec3 pools with default (main thread) - should be ok as long as we don't tick the main thread whilst this thread is running
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	//wprintf(L"Loading world on thread\n");

	if(ProfileManager.IsFullVersion())
	{	
		app.SetGameStarted(false);

		while( app.GetXuiServerAction(ProfileManager.GetPrimaryPad() ) != eXuiServerAction_Idle && !MinecraftServer::serverHalted() )
		{
			Sleep(10);
		}

		if(!MinecraftServer::serverHalted() && !app.GetChangingSessionType() ) app.SetGameStarted(true);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
		if(app.GetGameHostOption(eGameHostOption_DisableSaving)) StorageManager.SetSaveDisabled(true);
#endif
	}

	HRESULT hr = S_OK;
	if(app.GetChangingSessionType())
	{
		// 4J Stu - This causes the fullscreenprogress scene to ignore the action it was given
		hr = ERROR_CANCELLED;
	}
	return hr;
}

int IUIScene_PauseMenu::ExitWorldThreadProc( void* lpParameter )
{
	// Share AABB & Vec3 pools with default (main thread) - should be ok as long as we don't tick the main thread whilst this thread is running
	AABB::UseDefaultThreadStorage();
	Vec3::UseDefaultThreadStorage();
	Compression::UseDefaultThreadStorage();

	//app.SetGameStarted(false);

	_ExitWorld(lpParameter);

	return S_OK;
}

// This function performs the meat of exiting from a level. It should be called from a thread other than the main thread.
void IUIScene_PauseMenu::_ExitWorld(LPVOID lpParameter)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	int exitReasonStringId = pMinecraft->progressRenderer->getCurrentTitle();
	int exitReasonTitleId = IDS_CONNECTION_LOST;

	bool saveStats = true;
	if (pMinecraft->isClientSide() || g_NetworkManager.IsInSession())
	{
		if(lpParameter != nullptr )
		{
			// 4J-PB - check if we have lost connection to Live
			if(ProfileManager.GetLiveConnectionStatus()!=XONLINE_S_LOGON_CONNECTION_ESTABLISHED )
			{
				exitReasonStringId = IDS_CONNECTION_LOST_LIVE;
			}
			else
			{			
				switch( app.GetDisconnectReason() )
				{
				case DisconnectPacket::eDisconnect_Kicked:
					exitReasonStringId = IDS_DISCONNECTED_KICKED;
					break;
				case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
				case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#if defined(__PS3__) || defined(__ORBIS__)
				case DisconnectPacket::eDisconnect_ContentRestricted_AllLocal:
					exitReasonStringId = IDS_CONTENT_RESTRICTION_MULTIPLAYER;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
				case DisconnectPacket::eDisconnect_ContentRestricted_Single_Local:
					exitReasonStringId = IDS_CONTENT_RESTRICTION;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#endif
#ifdef _XBOX
				case DisconnectPacket::eDisconnect_NoUGC_Remote:
					exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#endif
				case DisconnectPacket::eDisconnect_NoFlying:
					exitReasonStringId = IDS_DISCONNECTED_FLYING;
					break;
				case DisconnectPacket::eDisconnect_Quitting:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
					break;
#ifdef __ORBIS__
				case DisconnectPacket::eDisconnect_NetworkError:
					exitReasonStringId = IDS_ERROR_NETWORK_EXIT;
					exitReasonTitleId = IDS_ERROR_NETWORK_TITLE;
					break;
#endif
				case DisconnectPacket::eDisconnect_NoFriendsInGame:
					exitReasonStringId = IDS_DISCONNECTED_NO_FRIENDS_IN_GAME;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_Banned:
					exitReasonStringId = IDS_DISCONNECTED_BANNED;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_NotFriendsWithHost:
					exitReasonStringId = IDS_NOTALLOWED_FRIENDSOFFRIENDS;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_OutdatedServer:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_OutdatedClient:
					exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
				case DisconnectPacket::eDisconnect_ServerFull:
					exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
					exitReasonTitleId = IDS_CANTJOIN_TITLE;
					break;
#ifdef _XBOX_ONE
				case DisconnectPacket::eDisconnect_ExitedGame:
					exitReasonTitleId = IDS_EXIT_GAME;
					exitReasonStringId = IDS_DISCONNECTED_EXITED_GAME;
					break;
#endif

#if defined __ORBIS__ || defined __PS3__ || defined __PSVITA__
				case DisconnectPacket::eDisconnect_NATMismatch:
					exitReasonStringId = IDS_DISCONNECTED_NAT_TYPE_MISMATCH;
					exitReasonTitleId = IDS_CONNECTION_FAILED;
					break;
#endif
				default:
					exitReasonStringId = IDS_CONNECTION_LOST_SERVER;
				}
			}
			//pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );
			
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			// 4J Stu - Fix for #48669 - TU5: Code: Compliance: TCR #15: Incorrect/misleading messages after signing out a profile during online game session.
			// If the primary player is signed out, then that is most likely the cause of the disconnection so don't display a message box. This will allow the message box requested by the libraries to be brought up
			if( ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad())) ui.RequestErrorMessage( exitReasonTitleId, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad());
			exitReasonStringId = -1;

			// 4J - Force a disconnection, this handles the situation that the server has already disconnected
			if( pMinecraft->levels[0] != nullptr ) pMinecraft->levels[0]->disconnect(false);
			if( pMinecraft->levels[1] != nullptr ) pMinecraft->levels[1]->disconnect(false);
			if( pMinecraft->levels[2] != nullptr ) pMinecraft->levels[2]->disconnect(false);
		}
		else
		{
			exitReasonStringId = IDS_EXITING_GAME;
			pMinecraft->progressRenderer->progressStartNoAbort( IDS_EXITING_GAME );
			if( pMinecraft->levels[0] != nullptr ) pMinecraft->levels[0]->disconnect();
			if( pMinecraft->levels[1] != nullptr ) pMinecraft->levels[1]->disconnect();
			if( pMinecraft->levels[2] != nullptr ) pMinecraft->levels[2]->disconnect();
		}

		// 4J Stu - This only does something if we actually have a server, so don't need to do any other checks
		MinecraftServer::HaltServer();

		// We need to call the stats & leaderboards save before we exit the session
		// 4J We need to do this in a QNet callback where it is safe
		//pMinecraft->forceStatsSave();
		saveStats = false;

		// 4J Stu - Leave the session once the disconnect packet has been sent
		g_NetworkManager.LeaveGame(FALSE);
	}
	else
	{
		if(lpParameter != nullptr && ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()) )
		{
			switch( app.GetDisconnectReason() )
			{
			case DisconnectPacket::eDisconnect_Kicked:
				exitReasonStringId = IDS_DISCONNECTED_KICKED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
			case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#if defined(__PS3__) || defined(__ORBIS__)
			case DisconnectPacket::eDisconnect_ContentRestricted_AllLocal:
				exitReasonStringId = IDS_CONTENT_RESTRICTION_MULTIPLAYER;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
			case DisconnectPacket::eDisconnect_ContentRestricted_Single_Local:
				exitReasonStringId = IDS_CONTENT_RESTRICTION;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#endif
#ifdef _XBOX
			case DisconnectPacket::eDisconnect_NoUGC_Remote:
				exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#endif
			case DisconnectPacket::eDisconnect_Quitting:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
				break;
#ifdef __ORBIS__
			case DisconnectPacket::eDisconnect_NetworkError:
				exitReasonStringId = IDS_ERROR_NETWORK_EXIT;
				exitReasonTitleId = IDS_ERROR_NETWORK_TITLE;
				break;
#endif
			case DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesJoin:
				exitReasonStringId = IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT;
				break;
			case DisconnectPacket::eDisconnect_OutdatedServer:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
			case DisconnectPacket::eDisconnect_OutdatedClient:
				exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
			case DisconnectPacket::eDisconnect_ServerFull:
				exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
				exitReasonTitleId = IDS_CANTJOIN_TITLE;
				break;
#if defined __ORBIS__ || defined __PS3__ || defined __PSVITA__
			case DisconnectPacket::eDisconnect_NATMismatch:
				exitReasonStringId = IDS_DISCONNECTED_NAT_TYPE_MISMATCH;
				exitReasonTitleId = IDS_CONNECTION_FAILED;
				break;
#endif
			default:
				exitReasonStringId = IDS_DISCONNECTED;
			}
			//pMinecraft->progressRenderer->progressStartNoAbort( exitReasonStringId );
			
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			ui.RequestErrorMessage( exitReasonTitleId, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad());
			exitReasonStringId = -1;
		}
	}
	// Fix for #93148 - TCR 001: BAS Game Stability: Title will crash for the multiplayer client if host of the game will exit during the clients loading to created world.
	while( g_NetworkManager.IsNetworkThreadRunning() )
	{
		Sleep(1);
	}
	pMinecraft->setLevel(nullptr,exitReasonStringId,nullptr,saveStats);

	TelemetryManager->Flush();
	
	app.m_gameRules.unloadCurrentGameRules();
	//app.m_Audio.unloadCurrentAudioDetails();

	MinecraftServer::resetFlags();
	
	// Fix for #48385 - BLACK OPS :TU5: Functional: Client becomes pseudo soft-locked when returned to the main menu after a remote disconnect
	// Make sure there is text explaining why the player is waiting
	pMinecraft->progressRenderer->progressStart(IDS_EXITING_GAME);

	// Fix for #13259 - CRASH: Gameplay: loading process is halted when player loads saved data
	// We can't start/join a new game until the session is destroyed, so wait for it to be idle again
	while( g_NetworkManager.IsInSession() )
	{
		Sleep(1);
	}

	app.SetChangingSessionType(false);
	app.SetReallyChangingSessionType(false);

#if defined(_XBOX_ONE) || defined(__ORBIS__)
	// Make sure we don't think saving is disabled in the menus
	StorageManager.SetSaveDisabled(false);
#endif
}


int IUIScene_PauseMenu::SaveGameDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_CANCEL;
		uiIDA[1]=IDS_CONFIRM_OK;
		ui.RequestAlertMessage(IDS_TITLE_ENABLE_AUTOSAVE, IDS_CONFIRM_ENABLE_AUTOSAVE, uiIDA, 2, iPad,&IUIScene_PauseMenu::EnableAutosaveDialogReturned,pParam);
#else
		// flag a app action of save game
		app.SetAction(iPad,eAppAction_SaveGame);
#endif
	}
	return 0;
}

int IUIScene_PauseMenu::EnableAutosaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// Set the global flag, so that we don't disable saving again once the save is complete
		app.SetGameHostOption(eGameHostOption_DisableSaving, 0);
	}
	else
	{
		// Set the global flag, so that we do disable saving again once the save is complete
		// We need to set this on as we may have only disabled it due to having a trial texture pack
		app.SetGameHostOption(eGameHostOption_DisableSaving, 1);
	}
	// Re-enable saving temporarily
	StorageManager.SetSaveDisabled(false);

	// flag a app action of save game
	app.SetAction(iPad,eAppAction_SaveGame);
	return 0;
}

int IUIScene_PauseMenu::DisableAutosaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// Set the global flag, so that we disable saving again once the save is complete
		app.SetGameHostOption(eGameHostOption_DisableSaving, 1);
		StorageManager.SetSaveDisabled(false);
		
		// flag a app action of save game
		app.SetAction(iPad,eAppAction_SaveGame);
	}
	return 0;
}