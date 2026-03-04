#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include <assert.h>
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileIO.h"
#include "..\..\LocalPlayer.h"
#include "..\..\Minecraft.h"
#include "..\..\..\Minecraft.World\AABB.h"
#include "..\..\..\Minecraft.World\Vec3.h"
#include "..\..\..\Minecraft.World\ArrayWithLength.h"
#include "..\..\..\Minecraft.World\File.h"
#include "..\..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Options.h"
#include "XUI_Ctrl_4JList.h"
#include "XUI_MultiGameInfo.h"
#include "XUI_MultiGameJoinLoad.h"
#include "XUI_Ctrl_4JIcon.h"
#include "XUI_LoadSettings.h"
#include "..\..\..\Minecraft.World\LevelSettings.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\GameRules\ConsoleGameRules.h"
#include "..\..\StringTable.h"
#include "..\..\DLCTexturePack.h"

#define GAME_CREATE_ONLINE_TIMER_ID 0
#define GAME_CREATE_ONLINE_TIMER_TIME 100
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID 1
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME 100

int CScene_LoadGameSettings::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};


HRESULT CScene_LoadGameSettings::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_hXuiBrush = NULL;
	m_bSetup = false;
	m_texturePackDescDisplayed = false;
	m_iConfigA=NULL;

	WCHAR TempString[256];

	m_params = (LoadMenuInitData *)pInitData->pvInitData;

	m_MoreOptionsParams.bGenerateOptions=FALSE;
	m_MoreOptionsParams.bPVP = TRUE;
	m_MoreOptionsParams.bTrust = TRUE;
	m_MoreOptionsParams.bFireSpreads = TRUE;
	m_MoreOptionsParams.bTNT = TRUE;
	m_MoreOptionsParams.bHostPrivileges = FALSE;
	m_MoreOptionsParams.bResetNether = FALSE;
	m_MoreOptionsParams.iPad = m_params->iPad;

	// 4J-JEV: Fix for:
	// TU12: Content: Gameplay: New "Mass Effect World" remembers and uses the settings of another - lately created - World.
	m_MoreOptionsParams.bBonusChest = FALSE;
	m_MoreOptionsParams.bFlatWorld = FALSE;
	m_MoreOptionsParams.bStructures = TRUE;

	m_iPad=m_params->iPad;
	m_iSaveGameInfoIndex=m_params->iSaveGameInfoIndex;
	m_levelGen = m_params->levelGen;

	MapChildControls();

	XuiControlSetText(m_MoreOptions,app.GetString(IDS_MORE_OPTIONS));
	XuiControlSetText(m_ButtonLoad,app.GetString(IDS_LOAD));
	XuiControlSetText(m_pTexturePacksList->m_hObj,app.GetString(IDS_DLC_MENU_TEXTUREPACKS));

	m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);
	// 4J-PB - read the settings for the online flag. We'll only save this setting if the user changed it.
	bool bGameSetting_Online=(app.GetGameSettings(m_iPad,eGameSetting_Online)!=0);
	m_MoreOptionsParams.bOnlineSettingChangedBySystem=false;

	// Set the text for friends of friends, and default to on
	if( m_bMultiplayerAllowed)
	{
		m_MoreOptionsParams.bOnlineGame = bGameSetting_Online?TRUE:FALSE;
		if(bGameSetting_Online)
		{
			m_MoreOptionsParams.bInviteOnly = (app.GetGameSettings(m_iPad,eGameSetting_InviteOnly)!=0)?TRUE:FALSE;
			m_MoreOptionsParams.bAllowFriendsOfFriends = (app.GetGameSettings(m_iPad,eGameSetting_FriendsOfFriends)!=0)?TRUE:FALSE;
		}
		else
		{
			m_MoreOptionsParams.bInviteOnly = FALSE;
			m_MoreOptionsParams.bAllowFriendsOfFriends = FALSE;
		}
	}
	else
	{
		m_MoreOptionsParams.bOnlineGame = FALSE;
		m_MoreOptionsParams.bInviteOnly = FALSE;
		m_MoreOptionsParams.bAllowFriendsOfFriends = FALSE;
		if(bGameSetting_Online)
		{
			// The profile settings say Online, but either the player is offline, or they are not allowed to play online
			m_MoreOptionsParams.bOnlineSettingChangedBySystem=true;
		}	
	}

	XuiSetTimer(m_hObj,GAME_CREATE_ONLINE_TIMER_ID,GAME_CREATE_ONLINE_TIMER_TIME);
	XuiSetTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);

	m_ButtonGameMode.SetText(app.GetString(IDS_GAMEMODE_SURVIVAL));
	m_bGameModeSurvival=true;
	m_CurrentDifficulty=app.GetGameSettings(m_iPad,eGameSetting_Difficulty);
	m_SliderDifficulty.SetValue(m_CurrentDifficulty);
	swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[m_CurrentDifficulty]));	
	m_SliderDifficulty.SetText(TempString);

	m_bHasBeenInCreative = false;

	if(m_levelGen)
	{
		m_GameName.SetText(m_levelGen->getDisplayName());
		if(m_levelGen->requiresTexturePack())
		{
			m_MoreOptionsParams.dwTexturePack = m_levelGen->getRequiredTexturePackId();
			m_pTexturePacksList->SetEnable(FALSE);

			// retrieve the save icon from the texture pack, if there is one
			TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);
			DWORD dwImageBytes;
			PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

			if(dwImageBytes > 0 && pbImageData)
			{
				XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hXuiBrush);
			}

			// Set this level as created in creative mode, so that people can't use the themed worlds as an easy way to get achievements
			m_bHasBeenInCreative = true;
			m_GameCreatedMode.SetText( app.GetString(IDS_CREATED_IN_CREATIVE) );
		}
	}
	else
	{
		// set the save icon
		PBYTE pbImageData=NULL;
		DWORD dwImageBytes=0;

		StorageManager.GetSaveCacheFileInfo(m_params->iSaveGameInfoIndex,m_XContentData);
		StorageManager.GetSaveCacheFileInfo(m_params->iSaveGameInfoIndex,&pbImageData,&dwImageBytes);

		// if there is no thumbnail, retrieve the default one from the file. 
		// Don't delete the image data after creating the xuibrush, since we'll use it in the rename of the save
		bool bHostOptionsRead = false;
		unsigned int uiHostOptions = 0;
		if(pbImageData==NULL)
		{
			DWORD dwResult=XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&m_XContentData,NULL,&dwImageBytes,NULL);
			if(dwResult==ERROR_SUCCESS)
			{
				pbImageData = new BYTE[dwImageBytes];
				XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&m_XContentData,pbImageData,&dwImageBytes,NULL);
				XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hXuiBrush);
			}
		}
		else
		{
			// retrieve the seed value from the image metadata
			ZeroMemory(m_szSeed,50);
			app.GetImageTextData(pbImageData,dwImageBytes,(unsigned char *)&m_szSeed,uiHostOptions,bHostOptionsRead,m_MoreOptionsParams.dwTexturePack);
			XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hXuiBrush);

// #ifdef _DEBUG
// 			// dump out the thumbnail
// 			HANDLE hThumbnail = CreateFile("GAME:\\thumbnail.png", GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);
// 			DWORD dwBytes;
// 			WriteFile(hThumbnail,pbImageData,dwImageBytes,&dwBytes,NULL);
// 			XCloseHandle(hThumbnail);
// #endif

			if(m_szSeed[0]!=0)
			{
				swprintf( (WCHAR *)TempString, 256, L"%ls: %hs", app.GetString( IDS_SEED ),m_szSeed);	
				m_GameSeed.SetText(TempString);
			}
			else
			{
				m_GameSeed.SetText(L"");
			}
		}

		// Setup all the text and checkboxes to match what the game was saved with on
		if(bHostOptionsRead)
		{
			m_MoreOptionsParams.bPVP = app.GetGameHostOption(uiHostOptions,eGameHostOption_PvP)>0?TRUE:FALSE;
			m_MoreOptionsParams.bTrust = app.GetGameHostOption(uiHostOptions,eGameHostOption_TrustPlayers)>0?TRUE:FALSE;
			m_MoreOptionsParams.bFireSpreads = app.GetGameHostOption(uiHostOptions,eGameHostOption_FireSpreads)>0?TRUE:FALSE;
			m_MoreOptionsParams.bTNT = app.GetGameHostOption(uiHostOptions,eGameHostOption_TNT)>0?TRUE:FALSE;
			m_MoreOptionsParams.bHostPrivileges = app.GetGameHostOption(uiHostOptions,eGameHostOption_CheatsEnabled)>0?TRUE:FALSE;

			m_bHasBeenInCreative = app.GetGameHostOption(uiHostOptions,eGameHostOption_HasBeenInCreative)>0;
			if(app.GetGameHostOption(uiHostOptions,eGameHostOption_HasBeenInCreative)>0)
			{
				m_GameCreatedMode.SetText( app.GetString(IDS_CREATED_IN_CREATIVE) );
			}
			else
			{
				m_GameCreatedMode.SetText( app.GetString(IDS_CREATED_IN_SURVIVAL) );
			}

			if(app.GetGameHostOption(uiHostOptions,eGameHostOption_GameType)>0)
			{			
				m_ButtonGameMode.SetText(app.GetString(IDS_GAMEMODE_CREATIVE));
				m_bGameModeSurvival=false;
			}

			if(app.GetGameHostOption(uiHostOptions,eGameHostOption_FriendsOfFriends) && !(m_bMultiplayerAllowed && bGameSetting_Online))
			{
				m_MoreOptionsParams.bAllowFriendsOfFriends = TRUE;
			}
		}

		m_GameName.SetText(m_XContentData.szDisplayName);
	}

	// 4J-PB - Load up any texture pack data we have locally in the XZP
	for(int i=0;i<TMS_COUNT;i++)
	{
		if(app.TMSFileA[i].eTMSType==eTMSFileType_TexturePack)
		{
			app.LoadLocalTMSFile(app.TMSFileA[i].wchFilename,app.TMSFileA[i].eEXT);
			app.AddMemoryTPDFile(app.TMSFileA[i].iConfig, app.TMSFileA[i].pbData,app.TMSFileA[i].uiSize);
		}
	}

	int iRB=-1;

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1);

	//if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );
	}
	
	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_LoadMenu, 0);
	m_iTexturePacksNotInstalled=0;
	
	// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
	if(app.StartInstallDLCProcess(m_iPad)==true)
	{
		// not doing a mount, so enable input
		m_bIgnoreInput=true;
	}
	else
	{
		m_bIgnoreInput = false;

		Minecraft *pMinecraft = Minecraft::GetInstance();
		m_pTexturePacksList->SetSelectionChangedHandle(m_hObj);

		int texturePacksCount = pMinecraft->skins->getTexturePackCount();
		CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
		HRESULT hr;
		for(unsigned int i = 0; i < texturePacksCount; ++i)
		{
			TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
			ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));


			DWORD dwImageBytes;
			PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

			if(dwImageBytes > 0 && pbImageData)
			{
				ListInfo.fEnabled = TRUE;			
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tp;
				if(pDLCTexPack)
				{
					int id=pDLCTexPack->getDLCParentPackId();

					if(id==0)
					{
						// default texture pack - should come first
						ListInfo.iSortIndex=0x0FFFFFFF;
					}
					else
					{
						ListInfo.iSortIndex=id;
						ListInfo.iData=id;
					}
				}				
#ifdef _DEBUG
				app.DebugPrintf("TP - ");
				OutputDebugStringW(tp->getName().c_str());
				app.DebugPrintf(", sort index - %d\n",ListInfo.iSortIndex);
#endif				
				hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&ListInfo.hXuiBrush);

				m_pTexturePacksList->AddData(ListInfo,0,CXuiCtrl4JList::eSortList_Index);
			}
		}
		
		m_currentTexturePackIndex = m_pTexturePacksList->GetIndexByUserData(m_MoreOptionsParams.dwTexturePack);
		m_pTexturePacksList->SetCurSel(m_currentTexturePackIndex);
		m_pTexturePacksList->SetTopItem(m_currentTexturePackIndex); // scroll the item into view if it's not visible
		UpdateTexturePackDescription(m_currentTexturePackIndex);


		// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this
		DLC_INFO *pDLCInfo=NULL;

		// first pass - look to see if there are any that are not in the list
		bool bTexturePackAlreadyListed;
		bool bNeedToGetTPD=false;
		m_iTexturePacksNotInstalled=0;

		for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
		{
			bTexturePackAlreadyListed=false;
			ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
			pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
			for(unsigned int i = 0; i < texturePacksCount; ++i)
			{
				TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
				if(pDLCInfo->iConfig==tp->getDLCParentPackId())
				{
					bTexturePackAlreadyListed=true;
				}
			}
			if(bTexturePackAlreadyListed==false)
			{
				// some missing
				bNeedToGetTPD=true;

				m_iTexturePacksNotInstalled++;
			}
		}

		if(bNeedToGetTPD==true)
		{
			// add a TMS request for them
			app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
			m_iConfigA= new int [m_iTexturePacksNotInstalled];
			m_iTexturePacksNotInstalled=0;

			for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
			{
				bTexturePackAlreadyListed=false;
				ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
				pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
				for(unsigned int i = 0; i < texturePacksCount; ++i)
				{
					TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
					if(pDLCInfo->iConfig==tp->getDLCParentPackId())
					{
						bTexturePackAlreadyListed=true;
					}
				}
				if(bTexturePackAlreadyListed==false)
				{
					m_iConfigA[m_iTexturePacksNotInstalled++]=pDLCInfo->iConfig;
				}
			}
		}
	}

	m_bSetup = true;

	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_LoadGameSettings::LaunchGame(void)
{
	// stop the timer running that causes a check for new texture packs in TMS but not installed, since this will run all through the load game, and will crash if it tries to create an hbrush
	XuiKillTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);

	if( (m_bGameModeSurvival != true || m_bHasBeenInCreative) || m_MoreOptionsParams.bHostPrivileges == TRUE)
	{			
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_OK;
		uiIDA[1]=IDS_CONFIRM_CANCEL;
		if(m_bGameModeSurvival != true || m_bHasBeenInCreative)
		{
			// 4J-PB - Need different text for Survival mode with a level that has been saved in Creative
			if(m_bGameModeSurvival)
			{
				StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_SAVEDINCREATIVE, uiIDA, 2, m_iPad,&CScene_LoadGameSettings::ConfirmLoadReturned,this,app.GetStringTable());
			}
			else // it's creative mode
			{
				// has it previously been saved in creative?
				if(m_bHasBeenInCreative)
				{
					// 4J-PB - We don't really need to tell the user this will have achievements disabled, since they already saved it in creative
					// and they got the warning then
					// inform them that leaderboard writes and achievements will be disabled
					//StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_SAVEDINCREATIVE_CONTINUE, uiIDA, 1, m_iPad,&CScene_LoadGameSettings::ConfirmLoadReturned,this,app.GetStringTable());
					
					if(m_levelGen != NULL)
					{
						LoadLevelGen(m_levelGen);
					}
					else
					{
						C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&m_XContentData,CScene_LoadGameSettings::LoadSaveDataReturned,this);

						if(eLoadStatus==C4JStorage::ELoadGame_DeviceRemoved)
						{
							// disable saving 
							StorageManager.SetSaveDisabled(true);
							StorageManager.SetSaveDeviceSelected(m_iPad,false);
							UINT uiIDA[1];
							uiIDA[0]=IDS_OK;
							StorageManager.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 1, m_iPad,&CScene_LoadGameSettings::DeviceRemovedDialogReturned,this);

						}
					}
				}
				else
				{
					// ask if they're sure they want to turn this into a creative map
					StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_CREATIVE, uiIDA, 2, m_iPad,&CScene_LoadGameSettings::ConfirmLoadReturned,this,app.GetStringTable());
				}
			}
		}
		else
		{
			StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_HOST_PRIVILEGES, uiIDA, 2, m_iPad,&CScene_LoadGameSettings::ConfirmLoadReturned,this,app.GetStringTable());
		}
	}
	else
	{
		if(m_levelGen != NULL)
		{
			LoadLevelGen(m_levelGen);
		}
		else
		{
			C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&m_XContentData,CScene_LoadGameSettings::LoadSaveDataReturned,this);

			if(eLoadStatus==C4JStorage::ELoadGame_DeviceRemoved)
			{
				// disable saving 
				StorageManager.SetSaveDisabled(true);
				StorageManager.SetSaveDeviceSelected(m_iPad,false);
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				StorageManager.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 1, m_iPad,&CScene_LoadGameSettings::DeviceRemovedDialogReturned,this);
			}
		}
	}
	return 0;
}

int CScene_LoadGameSettings::CheckResetNetherReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		// continue and reset the nether
		pClass->LaunchGame();
	}
	else 
	{
		// turn off the reset nether and continue
		pClass->m_MoreOptionsParams.bResetNether=FALSE;
		pClass->LaunchGame();
	}
	return 0;
}

HRESULT CScene_LoadGameSettings::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// 4J-PB - stop people double pressing this
	if(m_bIgnoreInput) return S_OK;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_ButtonLoad)
	{
		// Check if we need to upsell the texture pack
		if(m_MoreOptionsParams.dwTexturePack!=0)
		{
			// texture pack hasn't been set yet, so check what it will be
			TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);

			if(pTexturePack==NULL)
			{
				// They've selected a texture pack they don't have yet
				// upsell
				CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
				// get the current index of the list, and then get the data
				ListItem=m_pTexturePacksList->GetData(m_currentTexturePackIndex);


				// upsell the texture pack

				ULONGLONG ullOfferID_Full;
				app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

				// 4J-PB - if the full offer id is 0, then the texture pack dlc load failed
				if(ullOfferID_Full!=0LL)
				{			
					// tell sentient about the upsell of the full version of the skin pack				
					TelemetryManager->RecordUpsellPresented(ProfileManager.GetPrimaryPad(), eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

					UINT uiIDA[3];

					// Need to check if the texture pack has both Full and Trial versions - we may do some as free ones, so only Full
					DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);

					if(pDLCInfo->ullOfferID_Trial!=0LL)
					{
						uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
						uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
						uiIDA[2]=IDS_CONFIRM_CANCEL;
						// Give the player a warning about the texture pack missing
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&CScene_LoadGameSettings::TexturePackDialogReturned,this,app.GetStringTable());
					}
					else
					{
						uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
						uiIDA[1]=IDS_CONFIRM_CANCEL;
						// Give the player a warning about the texture pack missing
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CScene_LoadGameSettings::TexturePackDialogReturned,this,app.GetStringTable());
					}

					return S_OK;
				}
			}
		}

		// if the profile data has been changed, then force a profile write (we save the online/invite/friends of friends settings)
		// It seems we're allowed to break the 5 minute rule if it's the result of a user action
		// check the checkboxes

		// Only save the online setting if the user changed it - we may change it because we're offline, but don't want that saved
		if(!m_MoreOptionsParams.bOnlineSettingChangedBySystem)
		{
			app.SetGameSettings(m_iPad,eGameSetting_Online,m_MoreOptionsParams.bOnlineGame?1:0);
		}
		app.SetGameSettings(m_iPad,eGameSetting_InviteOnly,m_MoreOptionsParams.bInviteOnly?1:0);
		app.SetGameSettings(m_iPad,eGameSetting_FriendsOfFriends,m_MoreOptionsParams.bAllowFriendsOfFriends?1:0);

		app.CheckGameSettingsChanged(true,pNotifyPressData->UserIndex);

		SetShow( FALSE );
		m_bIgnoreInput = true;

		// Check that we have the rights to use a texture pack we have selected.
		if(m_MoreOptionsParams.dwTexturePack!=0)
		{
			// texture pack hasn't been set yet, so check what it will be
			TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);
			
			if(pTexturePack==NULL)
			{
				// DLC corrupt, so use the default textures
				m_MoreOptionsParams.dwTexturePack=0;
			}
			else
			{		
				m_pDLCPack=pTexturePack->getDLCPack();
				// do we have a license?
				if(m_pDLCPack && !m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{
					// no
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;

					if(!ProfileManager.IsSignedInLive(pNotifyPressData->UserIndex))
					{
						// need to be signed in to live
						StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
						return S_OK;
					}
					else
					{
						// upsell

						DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(m_pDLCPack->getPurchaseOfferId());
						ULONGLONG ullOfferID_Full;

						if(pDLCInfo!=NULL)
						{
							ullOfferID_Full=pDLCInfo->ullOfferID_Full;
						}
						else
						{
							ullOfferID_Full=pTexturePack->getDLCPack()->getPurchaseOfferId();
						}

						// tell sentient about the upsell of the full version of the texture pack
						TelemetryManager->RecordUpsellPresented(pNotifyPressData->UserIndex, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

						UINT uiIDA[2];
						uiIDA[0]=IDS_CONFIRM_OK;
						uiIDA[1]=IDS_CONFIRM_CANCEL;

						//StorageManager.RequestMessageBox(IDS_UNLOCK_DLC_TITLE, IDS_UNLOCK_DLC_SKIN, uiIDA, 2, pInputData->UserIndex,&CScene_SkinSelect::UnlockSkinReturned,this,app.GetStringTable());
						StorageManager.RequestMessageBox(IDS_UNLOCK_DLC_TEXTUREPACK_TITLE, IDS_UNLOCK_DLC_TEXTUREPACK_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_LoadGameSettings::UnlockTexturePackReturned,this,app.GetStringTable());
						return S_OK;
					}
				}		
			}
		}

		// Reset the background downloading, in case we changed it by attempting to download a texture pack
		XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);

		// Check if they have the Reset Nether flag set, and confirm they want to do this
		if(m_MoreOptionsParams.bResetNether==TRUE)
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_DONT_RESET_NETHER;
			uiIDA[1]=IDS_RESET_NETHER;

			StorageManager.RequestMessageBox(IDS_RESETNETHER_TITLE, IDS_RESETNETHER_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_LoadGameSettings::CheckResetNetherReturned,this,app.GetStringTable());
		}
		else
		{
			LaunchGame();
		}
	}
	else if(hObjPressed==m_MoreOptions)
	{	
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_LaunchMoreOptionsMenu,&m_MoreOptionsParams);
	}
	else if(hObjPressed == m_ButtonGameMode)
	{
		if(m_bGameModeSurvival)
		{
			m_ButtonGameMode.SetText(app.GetString(IDS_GAMEMODE_CREATIVE));
			m_bGameModeSurvival=false;
		}
		else
		{
			m_ButtonGameMode.SetText(app.GetString(IDS_GAMEMODE_SURVIVAL));
			m_bGameModeSurvival=true;
		}
	}
	else if(hObjPressed == m_pTexturePacksList->m_hObj)
	{
		UpdateCurrentTexturePack();
	}
	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		app.SetCorruptSaveDeleted(false);
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	case VK_PAD_RSHOULDER:
/*		if(app.m_bTransferSavesToXboxOne && ProfileManager.IsFullVersion())
		{
			app.NavigateToScene(m_iPad,eUIScene_TransferToXboxOne,m_params);
		}
		*/
		break;
	}

	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnFontRendererChange()
{
	int iRB=-1;

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1,-1,-1,-1,-1,-1,true);

	return S_OK;
}

int CScene_LoadGameSettings::ConfirmLoadReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept) 
	{
		if(pClass->m_levelGen != NULL)
		{
			pClass->LoadLevelGen(pClass->m_levelGen);
		}
		else
		{
			C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&pClass->m_XContentData,CScene_LoadGameSettings::LoadSaveDataReturned,pClass);

			if(eLoadStatus==C4JStorage::ELoadGame_DeviceRemoved)
			{
				// disable saving 
				StorageManager.SetSaveDisabled(true);
				StorageManager.SetSaveDeviceSelected(pClass->m_iPad,false);
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				StorageManager.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 1, pClass->m_iPad,&CScene_LoadGameSettings::DeviceRemovedDialogReturned,pClass);
			}		
		}
	}
	else
	{
		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

HRESULT CScene_LoadGameSettings::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// 4J-PB - TODO - Don't think we can do this - if a 2nd player signs in here with an offline profile, the signed in LIVE player gets re-logged in, and bMultiplayerAllowed is false briefly 
	switch(pTimer->nId)
	{
		case GAME_CREATE_ONLINE_TIMER_ID:
		{
			bool bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);

			if(bMultiplayerAllowed != m_bMultiplayerAllowed)
			{
				if( bMultiplayerAllowed )
				{
					bool bGameSetting_Online=(app.GetGameSettings(m_iPad,eGameSetting_Online)!=0);
					m_MoreOptionsParams.bOnlineGame = bGameSetting_Online?TRUE:FALSE;
					if(bGameSetting_Online)
					{
						m_MoreOptionsParams.bInviteOnly = (app.GetGameSettings(m_iPad,eGameSetting_InviteOnly)!=0)?TRUE:FALSE;
						m_MoreOptionsParams.bAllowFriendsOfFriends = (app.GetGameSettings(m_iPad,eGameSetting_FriendsOfFriends)!=0)?TRUE:FALSE;
					}
					else
					{
						m_MoreOptionsParams.bInviteOnly = FALSE;
						m_MoreOptionsParams.bAllowFriendsOfFriends = FALSE;
					}
				}
				else
				{
					m_MoreOptionsParams.bOnlineGame = FALSE;
					m_MoreOptionsParams.bInviteOnly = FALSE;
					m_MoreOptionsParams.bAllowFriendsOfFriends = FALSE;
				}

				m_bMultiplayerAllowed = bMultiplayerAllowed;
			}
		}
		break;

		case CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID:
		{
			// also check for any new texture packs info being available
			// for each item in the mem list, check it's in the data list

			CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
			// for each iConfig, check if the data is available, and add it to the List, then remove it from the viConfig
			for(int i=0;i<m_iTexturePacksNotInstalled;i++)
			{
				if(m_iConfigA[i]!=-1)
				{
					DWORD dwBytes=0;
					PBYTE pbData=NULL;
					app.GetTPD(m_iConfigA[i],&pbData,&dwBytes);

					ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));
					if(dwBytes > 0 && pbData)
					{
						DWORD dwImageBytes=0;
						PBYTE pbImageData=NULL;

						app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbImageData,&dwImageBytes );
						ListInfo.fEnabled = TRUE;	
						ListInfo.iData = m_iConfigA[i];
						HRESULT hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&ListInfo.hXuiBrush);

						ListInfo.iSortIndex=m_iConfigA[i];
						m_pTexturePacksList->AddData(ListInfo,0,CXuiCtrl4JList::eSortList_Index);

						m_iConfigA[i]=-1;

						m_currentTexturePackIndex = m_pTexturePacksList->GetIndexByUserData(m_MoreOptionsParams.dwTexturePack);
						m_pTexturePacksList->SetCurSel(m_currentTexturePackIndex);
						m_pTexturePacksList->SetTopItem(m_currentTexturePackIndex); // scroll the item into view if it's not visible
					}
				}
			}
			bool bAllDone=true;
			for(int i=0;i<m_iTexturePacksNotInstalled;i++)
			{
				if(m_iConfigA[i]!=-1) 
				{
					bAllDone = false;
				}
			}

			if(bAllDone )
			{
				// kill this timer
				XuiKillTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
			}
		}
		break;
	}

	return S_OK;
}
		PBYTE pbData;

int CScene_LoadGameSettings::Progress(void *pParam,float fProgress)
{
	app.DebugPrintf("Progress - %f\n",fProgress);

	return 0;
}

int CScene_LoadGameSettings::LoadSaveDataReturned(void *pParam,bool bContinue)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	if(bContinue==true)
	{
		bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;

		// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
		DWORD connectedControllers = 0;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
		}

		if(!isClientSide || connectedControllers == 1 || !RenderManager.IsHiDef())
		{
			// Check if user-created content is allowed, as we cannot play multiplayer if it's not
			bool noUGC = false;
			BOOL pccAllowed = TRUE;
			BOOL pccFriendsAllowed = TRUE;
			ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;
			
			if(isClientSide && noUGC )
			{
				pClass->SetShow( TRUE );
				pClass->m_bIgnoreInput=false;
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
			}
			else
			{
				DWORD dwLocalUsersMask = CGameNetworkManager::GetLocalPlayerMask(ProfileManager.GetPrimaryPad());

				// No guest problems so we don't need to force a sign-in of players here
				StartGameFromSave(pClass, dwLocalUsersMask);
			}
		}
		else
		{
			ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_LoadGameSettings::StartGame_SignInReturned, pParam,ProfileManager.GetPrimaryPad());
		}
	}
	else
	{
		// the save is corrupt!

		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;

		// give the option to delete the save
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_CANCEL;
		uiIDA[1]=IDS_CONFIRM_OK;
		StorageManager.RequestMessageBox(IDS_CORRUPT_OR_DAMAGED_SAVE_TITLE, IDS_CORRUPT_OR_DAMAGED_SAVE_TEXT, uiIDA, 2, 
			pClass->m_iPad,&CScene_LoadGameSettings::DeleteSaveDialogReturned,pClass, app.GetStringTable());

	}
	return 0;
}

int CScene_LoadGameSettings::DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
#ifdef _XBOX
 		StorageManager.DeleteSaveData(&pClass->m_XContentData,CScene_LoadGameSettings::DeleteSaveDataReturned,pClass);
#endif
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

int CScene_LoadGameSettings::DeleteSaveDataReturned(void *pParam,bool bSuccess)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	app.SetCorruptSaveDeleted(true);
	app.NavigateBack(pClass->m_iPad);

	return 0;
}

int CScene_LoadGameSettings::DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	// need to back out of this scene
	app.NavigateBack(iPad);

	return 0;
}

// 4J Stu - Shared functionality that is the same whether we needed a quadrant sign-in or not
void CScene_LoadGameSettings::StartGameFromSave(CScene_LoadGameSettings* pClass, DWORD dwLocalUsersMask)
{
	INT saveOrCheckpointId = 0;
	bool validSave = StorageManager.GetSaveUniqueNumber(&saveOrCheckpointId);
	TelemetryManager->RecordLevelResume(pClass->m_iPad, eSen_FriendOrMatch_Playing_With_Invited_Friends, eSen_CompeteOrCoop_Coop_and_Competitive, app.GetGameSettings(pClass->m_iPad,eGameSetting_Difficulty), app.GetLocalPlayerCount(), g_NetworkManager.GetOnlinePlayerCount(), saveOrCheckpointId);

	bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
	bool isPrivate = (app.GetGameSettings(pClass->m_iPad,eGameSetting_InviteOnly)>0)?true:false;

	g_NetworkManager.HostGame(dwLocalUsersMask,isClientSide,isPrivate,MINECRAFT_NET_MAX_PLAYERS,0);

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = 0;
	param->saveData = NULL;
	param->texturePackId = pClass->m_MoreOptionsParams.dwTexturePack;
	
	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->skins->selectTexturePackById(pClass->m_MoreOptionsParams.dwTexturePack);
	//pMinecraft->skins->updateUI();
	
	app.SetGameHostOption(eGameHostOption_Difficulty,Minecraft::GetInstance()->options->difficulty);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,app.GetGameSettings(pClass->m_iPad,eGameSetting_FriendsOfFriends));
	app.SetGameHostOption(eGameHostOption_Gamertags,app.GetGameSettings(pClass->m_iPad,eGameSetting_GamertagsVisible));

	app.SetGameHostOption(eGameHostOption_BedrockFog,app.GetGameSettings(pClass->m_iPad,eGameSetting_BedrockFog)?1:0);

	app.SetGameHostOption(eGameHostOption_PvP,pClass->m_MoreOptionsParams.bPVP);
	app.SetGameHostOption(eGameHostOption_TrustPlayers,pClass->m_MoreOptionsParams.bTrust );
	app.SetGameHostOption(eGameHostOption_FireSpreads,pClass->m_MoreOptionsParams.bFireSpreads );
	app.SetGameHostOption(eGameHostOption_TNT,pClass->m_MoreOptionsParams.bTNT );
	app.SetGameHostOption(eGameHostOption_HostCanFly,pClass->m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,pClass->m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,pClass->m_MoreOptionsParams.bHostPrivileges );

	// flag if the user wants to reset the Nether to force a Fortress with netherwart etc.
	app.SetResetNether((pClass->m_MoreOptionsParams.bResetNether==TRUE)?true:false);
	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	app.SetGameHostOption(eGameHostOption_GameType,pClass->m_bGameModeSurvival?GameType::SURVIVAL->getId():GameType::CREATIVE->getId());

	param->settings = app.GetGameHostOption( eGameHostOption_All );

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	// Reset the autosave timer
	app.SetAutosaveTimerTime();

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

int CScene_LoadGameSettings::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_LoadGameSettings* pClass = (CScene_LoadGameSettings*)pParam;

	if(bContinue==true)
	{
		// It's possible that the player has not signed in - they can back out
		if(ProfileManager.IsSignedIn(iPad))
		{
			DWORD dwLocalUsersMask = 0;

			bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
			bool noPrivileges = false;
			for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
			{
				if(ProfileManager.IsSignedIn(index) )
				{
					if( !ProfileManager.AllowedToPlayMultiplayer(index) ) noPrivileges = true;
					dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(index);
				}
			}

			// Check if user-created content is allowed, as we cannot play multiplayer if it's not
			bool noUGC = false;
			BOOL pccAllowed = TRUE;
			BOOL pccFriendsAllowed = TRUE;
			ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(), false, &pccAllowed,&pccFriendsAllowed);
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

			if(isClientSide && (noPrivileges || noUGC) )
			{
				if( noUGC )
				{
					pClass->SetShow( TRUE );
					pClass->m_bIgnoreInput=false;
					//pClass->m_bAbortSearch=false;
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
				}
				else
				{
					pClass->SetShow( TRUE );
					pClass->m_bIgnoreInput=false;
					//pClass->m_bAbortSearch=false;
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_HOST_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
				}
			}
			else
			{
				StartGameFromSave(pClass, dwLocalUsersMask);
			}
		}
	}
	else
	{
		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
		//pClass->m_bAbortSearch=false;
	}
	return 0;
}


HRESULT CScene_LoadGameSettings::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{
	if( pGetSourceImageData->bItemData )
	{
		pGetSourceImageData->hBrush = m_hXuiBrush;	
		bHandled = TRUE;
	}
	return S_OK;
}


HRESULT CScene_LoadGameSettings::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged* pNotifyValueChanged, BOOL& bHandled )
{
	WCHAR TempString[256];

	if(hObjSource==m_SliderDifficulty.GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Difficulty,pNotifyValueChanged->nValue);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[pNotifyValueChanged->nValue]));		
		m_SliderDifficulty.SetText(TempString);
	}
	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ||
		pTransition->dwTransType == XUI_TRANSITION_FROM  || pTransition->dwTransType == XUI_TRANSITION_BACKFROM)
	{
		// 4J Stu - We may have had to unload our font renderer in this scene if one of the save files
		// uses characters not in our font (eg asian chars) so restore our font renderer
		// This will not do anything if our font renderer is already loaded
		app.OverrideFontRenderer(true,true);
	}
	else if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{	
		m_SliderDifficulty.SetValueDisplay(FALSE);
		// 4J-PB - Need to check for installed DLC, which might have happened while you were on the More Options scene
		if(pTransition->dwTransType == XUI_TRANSITION_BACKTO)
		{
			// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
			if(app.StartInstallDLCProcess(m_iPad)==false)
			{
				// not doing a mount, so re-enable input
				m_bIgnoreInput=false;
			}
			else
			{
				m_bIgnoreInput=true;
				m_pTexturePacksList->RemoveAllData();
			}
		}
	}

	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ||
		pTransition->dwTransType == XUI_TRANSITION_FROM  || pTransition->dwTransType == XUI_TRANSITION_BACKFROM)
	{
	}
	else if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{	
		if(m_bSetup && m_texturePackDescDisplayed)
		{
			XUITimeline *timeline;
			XUINamedFrame *startFrame, *endFrame;
			GetTimeline( &timeline );
			startFrame = timeline->FindNamedFrame( L"SlideOutEnd" );
			endFrame = timeline->FindNamedFrame( L"SlideOutEnd" );
			timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, endFrame->m_dwFrame, FALSE, FALSE );
			m_texturePackDescDisplayed = true;
		}
	}

	return S_OK;
}

int CScene_LoadGameSettings::UnlockTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_LoadGameSettings* pScene = (CScene_LoadGameSettings*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
			ULONGLONG ullIndexA[1];
			DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(pScene->m_pDLCPack->getPurchaseOfferId());

			if(pDLCInfo!=NULL)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Full;
			}
			else
			{
				ullIndexA[0]=pScene->m_pDLCPack->getPurchaseOfferId();
			}

			StorageManager.InstallOffer(1,ullIndexA,NULL,NULL);

			// the license change coming in when the offer has been installed will cause this scene to refresh	
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( pScene->m_pDLCPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}

	pScene->m_bIgnoreInput = false;
	pScene->SetShow( TRUE );

	return 0;
}

HRESULT CScene_LoadGameSettings::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if(hObjSource == m_pTexturePacksList->m_hObj)
	{
		UpdateTexturePackDescription(pNotifySelChangedData->iItem);

		if(m_bSetup && !m_texturePackDescDisplayed)
		{
			XUITimeline *timeline;
			XUINamedFrame *startFrame, *endFrame;
			GetTimeline( &timeline );
			startFrame = timeline->FindNamedFrame( L"SlideOut" );
			endFrame = timeline->FindNamedFrame( L"SlideOutEnd" );
			timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, endFrame->m_dwFrame, FALSE, FALSE );
			m_texturePackDescDisplayed = true;
		}
	}

	return S_OK;
}

HRESULT CScene_LoadGameSettings::OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	HXUIOBJ hSourceParent, hDestParent;
	XuiElementGetParent(hObjSource,&hSourceParent);
	XuiElementGetParent(pNotifyFocusData->hObjOther,&hDestParent);
	if(hSourceParent != hDestParent && pNotifyFocusData->hObjOther != m_pTexturePacksList->m_hObj && hSourceParent == m_pTexturePacksList->m_hObj)
	{
		m_pTexturePacksList->SetCurSel(m_currentTexturePackIndex);
		m_pTexturePacksList->SetTopItem(m_currentTexturePackIndex); // scroll the item into view if it's not visible
	}
	else if(!m_texturePackDescDisplayed && pNotifyFocusData->hObjOther == m_pTexturePacksList->m_hObj)
	{
		int texturePacksCount = Minecraft::GetInstance()->skins->getTexturePackCount();
		if(texturePacksCount == 1)
		{
			XUITimeline *timeline;
			XUINamedFrame *startFrame, *endFrame;
			GetTimeline( &timeline );
			startFrame = timeline->FindNamedFrame( L"SlideOut" );
			endFrame = timeline->FindNamedFrame( L"SlideOutEnd" );
			timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, endFrame->m_dwFrame, FALSE, FALSE );
			m_texturePackDescDisplayed = true;
		}
	}

	return S_OK;
}

void CScene_LoadGameSettings::UpdateTexturePackDescription(int index)
{
	int iTexPackId=m_pTexturePacksList->GetData(index).iData;
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(iTexPackId);

	if(tp==NULL)
	{
		// this is probably a texture pack icon added from TMS
		DWORD dwBytes=0,dwFileBytes=0;
		PBYTE pbData=NULL,pbFileData=NULL;

		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=m_pTexturePacksList->GetData(index);

		app.GetTPD(ListItem.iData,&pbData,&dwBytes);

		app.GetFileFromTPD(eTPDFileType_Loc,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes > 0 && pbFileData)
		{
			StringTable *pStringTable = new StringTable(pbFileData, dwFileBytes);
			m_texturePackTitle.SetText(pStringTable->getString(L"IDS_DISPLAY_NAME"));
			m_texturePackDescription.SetText(pStringTable->getString(L"IDS_TP_DESCRIPTION"));
		}

		app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes >= 0 && pbFileData)
		{
			XuiCreateTextureBrushFromMemory(pbFileData,dwFileBytes,&m_hTexturePackIconBrush);
			m_texturePackIcon->UseBrush(m_hTexturePackIconBrush);
		}
		app.GetFileFromTPD(eTPDFileType_Comparison,pbData,dwBytes,&pbFileData,&dwFileBytes );
		if(dwFileBytes >= 0 && pbFileData)
		{
			XuiCreateTextureBrushFromMemory(pbFileData,dwFileBytes,&m_hTexturePackComparisonBrush);
			m_texturePackComparison->UseBrush(m_hTexturePackComparisonBrush);
		}
		else
		{
			m_texturePackComparison->UseBrush(NULL);
		}
	}
	else
	{
		m_texturePackTitle.SetText(tp->getName().c_str());
		m_texturePackDescription.SetText(tp->getDesc1().c_str());

		DWORD dwImageBytes;
		PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hTexturePackIconBrush);
			m_texturePackIcon->UseBrush(m_hTexturePackIconBrush);
		}
		else
		{
			m_texturePackIcon->UseBrush(NULL);
		}

		pbImageData = tp->getPackComparison(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hTexturePackComparisonBrush);
			m_texturePackComparison->UseBrush(m_hTexturePackComparisonBrush);
		}
		else
		{
			m_texturePackComparison->UseBrush(NULL);
		}
	}
}

void CScene_LoadGameSettings::ClearTexturePackDescription()
{
	m_texturePackTitle.SetText(L" ");
	m_texturePackDescription.SetText(L" ");
	m_texturePackComparison->UseBrush(NULL);
	m_texturePackIcon->UseBrush(NULL);
}

void CScene_LoadGameSettings::UpdateCurrentTexturePack()
{
	m_currentTexturePackIndex = m_pTexturePacksList->GetCurSel();
	int iTexPackId=m_pTexturePacksList->GetData(m_currentTexturePackIndex).iData;
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(iTexPackId);

	// if the texture pack is null, you don't have it yet
	if(tp==NULL)
	{
		// Upsell

		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=m_pTexturePacksList->GetData(m_currentTexturePackIndex);


		// upsell the texture pack
		// tell sentient about the upsell of the full version of the skin pack
		ULONGLONG ullOfferID_Full;
		app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

		TelemetryManager->RecordUpsellPresented(ProfileManager.GetPrimaryPad(), eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

		UINT uiIDA[3];

		// Need to check if the texture pack has both Full and Trial versions - we may do some as free ones, so only Full
		DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);

		if(pDLCInfo->ullOfferID_Trial!=0LL)
		{
			uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
			uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
			uiIDA[2]=IDS_CONFIRM_CANCEL;
			// Give the player a warning about the texture pack missing
			StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&CScene_LoadGameSettings::TexturePackDialogReturned,this,app.GetStringTable());
		}
		else
		{
			uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			// Give the player a warning about the texture pack missing
			StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CScene_LoadGameSettings::TexturePackDialogReturned,this,app.GetStringTable());
		}

		// do set the texture pack id, and on the user pressing create world, check they have it	
		m_MoreOptionsParams.dwTexturePack = ListItem.iData;

		return ;	
	}
	else
	{
		m_MoreOptionsParams.dwTexturePack = tp->getId();
	}
}

HRESULT CScene_LoadGameSettings::OnDestroy()
{
	if( m_hXuiBrush )
	{
		XuiDestroyBrush( m_hXuiBrush );
	}

	// clear out the texture pack data
	for(int i=0;i<TMS_COUNT;i++)
	{
		if(app.TMSFileA[i].eTMSType==eTMSFileType_TexturePack)
		{
			app.RemoveMemoryTPDFile(app.TMSFileA[i].iConfig);
		}
	}
	app.FreeLocalTMSFiles(eTMSFileType_TexturePack);

	return S_OK;
}

void CScene_LoadGameSettings::LoadLevelGen(LevelGenerationOptions *levelGen)
{
	bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && m_MoreOptionsParams.bOnlineGame;

	// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
	DWORD connectedControllers = 0;
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
	}

	if(!isClientSide || connectedControllers == 1 || !RenderManager.IsHiDef())
	{
		// Check if user-created content is allowed, as we cannot play multiplayer if it's not
		bool noUGC = false;
		BOOL pccAllowed = TRUE;
		BOOL pccFriendsAllowed = TRUE;
		ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
		if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

		if(isClientSide && noUGC )
		{
			SetShow( TRUE );
			m_bIgnoreInput=false;
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
			return;
		}
	}

	DWORD dwLocalUsersMask = 0;

	dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(ProfileManager.GetPrimaryPad());
	// Load data from disc
	//File saveFile( L"Tutorial\\Tutorial" );
	//LoadSaveFromDisk(&saveFile);
		
	StorageManager.ResetSaveData();
	// Make our next save default to the name of the level
	StorageManager.SetSaveTitle(levelGen->getDefaultSaveName().c_str());
	
	bool isPrivate = (app.GetGameSettings(m_iPad,eGameSetting_InviteOnly)>0)?true:false;

	g_NetworkManager.HostGame(dwLocalUsersMask,isClientSide,isPrivate,MINECRAFT_NET_MAX_PLAYERS,0);

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = 0;
	param->saveData = NULL;
	param->levelGen = levelGen;

	if(levelGen->requiresTexturePack())
	{
		param->texturePackId = levelGen->getRequiredTexturePackId();
	
		Minecraft *pMinecraft = Minecraft::GetInstance();
		pMinecraft->skins->selectTexturePackById(param->texturePackId);
		//pMinecraft->skins->updateUI();
	}
	
	
	app.SetGameHostOption(eGameHostOption_Difficulty,Minecraft::GetInstance()->options->difficulty);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,app.GetGameSettings(m_iPad,eGameSetting_FriendsOfFriends));
	app.SetGameHostOption(eGameHostOption_Gamertags,app.GetGameSettings(m_iPad,eGameSetting_GamertagsVisible));

	app.SetGameHostOption(eGameHostOption_BedrockFog,app.GetGameSettings(m_iPad,eGameSetting_BedrockFog)?1:0);

	// 4J-JEV: Fix for:
	// TU12: Content: Gameplay: New "Mass Effect World" remembers and uses the settings of another - lately created - World.
	app.SetGameHostOption(eGameHostOption_LevelType,m_MoreOptionsParams.bFlatWorld);
	app.SetGameHostOption(eGameHostOption_Structures,m_MoreOptionsParams.bStructures);
	app.SetGameHostOption(eGameHostOption_BonusChest,m_MoreOptionsParams.bBonusChest);

	app.SetGameHostOption(eGameHostOption_PvP,m_MoreOptionsParams.bPVP);
	app.SetGameHostOption(eGameHostOption_TrustPlayers,m_MoreOptionsParams.bTrust );
	app.SetGameHostOption(eGameHostOption_FireSpreads,m_MoreOptionsParams.bFireSpreads );
	app.SetGameHostOption(eGameHostOption_TNT,m_MoreOptionsParams.bTNT );
	app.SetGameHostOption(eGameHostOption_HostCanFly,m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,m_MoreOptionsParams.bHostPrivileges );

	// flag if the user wants to reset the Nether to force a Fortress with netherwart etc.
	app.SetResetNether((m_MoreOptionsParams.bResetNether==TRUE)?true:false);
	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	app.SetGameHostOption(eGameHostOption_GameType,m_bGameModeSurvival?GameType::SURVIVAL->getId():GameType::CREATIVE->getId());

	param->settings = app.GetGameHostOption( eGameHostOption_All );

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	// Reset the autosave timer
	app.SetAutosaveTimerTime();

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}


HRESULT CScene_LoadGameSettings::OnCustomMessage_DLCInstalled()
{
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		m_bIgnoreInput=false;
	}
	else
	{
		m_bIgnoreInput=true;
		// clear out the texture pack list and do again
		m_pTexturePacksList->RemoveAllData();
		m_iTexturePacksNotInstalled=0;

	}

	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}


HRESULT CScene_LoadGameSettings::OnCustomMessage_DLCMountingComplete()
{	
	m_pTexturePacksList->SetSelectionChangedHandle(m_hObj);
	Minecraft *pMinecraft = Minecraft::GetInstance();
	int texturePacksCount = pMinecraft->skins->getTexturePackCount();	
	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
	HRESULT hr;
	for(unsigned int i = 0; i < texturePacksCount; ++i)
	{
		TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
		ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));

		DWORD dwImageBytes;
		PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			ListInfo.fEnabled = TRUE;			
			hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&ListInfo.hXuiBrush);

			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tp;
			if(pDLCTexPack)
			{
				int id=pDLCTexPack->getDLCParentPackId();

				if(id==0)
				{
					// default texture pack - should come first
					ListInfo.iSortIndex=0x0FFFFFFF;
				}
				else
				{
					ListInfo.iSortIndex=id;
					ListInfo.iData=id;
				}
			}
			m_pTexturePacksList->AddData(ListInfo,0,CXuiCtrl4JList::eSortList_Index);
		}
	}
	m_currentTexturePackIndex = m_pTexturePacksList->GetIndexByUserData(m_MoreOptionsParams.dwTexturePack);
	m_pTexturePacksList->SetCurSel(m_currentTexturePackIndex);
	m_pTexturePacksList->SetTopItem(m_currentTexturePackIndex); // scroll the item into view if it's not visible

	m_iTexturePacksNotInstalled=0;

	// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this
	DLC_INFO *pDLCInfo=NULL;

	// first pass - look to see if there are any that are not in the list
	bool bTexturePackAlreadyListed;
	bool bNeedToGetTPD=false;

	for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
	{
		bTexturePackAlreadyListed=false;
		ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
		pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
		for(unsigned int i = 0; i < texturePacksCount; ++i)
		{
			TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
			if(pDLCInfo->iConfig==tp->getDLCParentPackId())
			{
				bTexturePackAlreadyListed=true;
			}
		}
		if(bTexturePackAlreadyListed==false)
		{
			// some missing
			bNeedToGetTPD=true;

			m_iTexturePacksNotInstalled++;
		}
	}

	if(bNeedToGetTPD==true)
	{
		// add a TMS request for them
		app.DebugPrintf("+++ Adding TMSPP request for texture pack data\n");
		app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
		if(m_iConfigA!=NULL)
		{
			delete m_iConfigA;
		}
		m_iConfigA= new int [m_iTexturePacksNotInstalled];
		m_iTexturePacksNotInstalled=0;

		for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
		{
			bTexturePackAlreadyListed=false;
			ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
			pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
			for(unsigned int i = 0; i < texturePacksCount; ++i)
			{
				TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
				if(pDLCInfo->iConfig==tp->getDLCParentPackId())
				{
					bTexturePackAlreadyListed=true;
				}
			}
			if(bTexturePackAlreadyListed==false)
			{
				m_iConfigA[m_iTexturePacksNotInstalled++]=pDLCInfo->iConfig;
			}
		}
	}

	UpdateTexturePackDescription(m_currentTexturePackIndex);

	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	return S_OK;
}

int CScene_LoadGameSettings::TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_LoadGameSettings *pClass = (CScene_LoadGameSettings *)pParam;
#ifdef _XBOX
	pClass->m_currentTexturePackIndex = pClass->m_pTexturePacksList->GetCurSel();
	// Exit with or without saving
	// Decline means install full version of the texture pack in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultAccept) 
	{
		// we need to enable background downloading for the DLC
		XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

		ULONGLONG ullOfferID_Full;
		ULONGLONG ullIndexA[1];
		CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
		// get the current index of the list, and then get the data
		ListItem=pClass->m_pTexturePacksList->GetData(pClass->m_currentTexturePackIndex);
		app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

		if( result==C4JStorage::EMessage_ResultAccept ) // Full version
		{
			ullIndexA[0]=ullOfferID_Full;
			StorageManager.InstallOffer(1,ullIndexA,NULL,NULL);

		}
		else // trial version
		{
			// if there is no trial version, this is a Cancel
			DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);
			if(pDLCInfo->ullOfferID_Trial!=0LL)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Trial;
				StorageManager.InstallOffer(1,ullIndexA,NULL,NULL);
			}
		}		
	}
#endif
	pClass->m_bIgnoreInput=false;
	return 0;
}

HRESULT CScene_LoadGameSettings::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	// update the tooltips
	int iRB=-1;
#ifdef _XBOX

#endif
	CXuiSceneBase::SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1,-1,-1,-1,iRB);
	return S_OK;
}