#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include <assert.h>
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileIO.h"
#include "..\..\LocalPlayer.h"
#include "..\..\Minecraft.h"
#include "..\..\ProgressRenderer.h"
#include "..\..\..\Minecraft.World\AABB.h"
#include "..\..\..\Minecraft.World\Vec3.h"
#include "..\..\..\Minecraft.World\ArrayWithLength.h"
#include "..\..\..\Minecraft.World\File.h"
#include "..\..\..\Minecraft.World\InputOutputStream.h"
#include "XUI_Ctrl_4JList.h"
#include "XUI_Ctrl_4JIcon.h"
#include "XUI_LoadSettings.h"
#include "XUI_MultiGameInfo.h"
#include "XUI_MultiGameJoinLoad.h"
#include "XUI_MultiGameCreate.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Options.h"

#include "..\GameRules\LevelGenerationOptions.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\..\..\Minecraft.World\LevelSettings.h"

#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID 3
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME 100

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameJoinLoad::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{

	m_iPad=*(int *)pInitData->pvInitData;
	m_bReady=false;
	MapChildControls();

	m_iTexturePacksNotInstalled=0;
	m_iConfigA=NULL;

	XuiControlSetText(m_LabelNoGames,app.GetString(IDS_NO_GAMES_FOUND));
	XuiControlSetText(m_GamesList,app.GetString(IDS_JOIN_GAME));
	XuiControlSetText(m_SavesList,app.GetString(IDS_START_GAME));


	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);
	swprintf(szResourceLocator, LOCATOR_SIZE ,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/TexturePackIcon.png");

	m_DefaultMinecraftIconSize = 0;
	HRESULT hr = XuiResourceLoadAllNoLoc(szResourceLocator, &m_DefaultMinecraftIconData, &m_DefaultMinecraftIconSize);

	m_localPlayers = 1;
	m_bKillSaveInfoEnumerate=false;

	m_bShowingPartyGamesOnly = false;

	m_bRetrievingSaveInfo=false;
	m_bSaveTransferInProgress=false;

	// check for a default custom cloak in the global storage
	// 4J-PB - changed to a config file
// 	if(ProfileManager.IsSignedInLive( m_iPad ))
// 	{
// 		app.InstallDefaultCape();
// 	}

	m_initData= new JoinMenuInitData();
	m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);

	XPARTY_USER_LIST partyList;

	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		m_bInParty=true;
	}
	else
	{
		m_bInParty=false;
	}

	int iLB = -1;
	if(m_bInParty) iLB = IDS_TOOLTIPS_PARTY_GAMES;

	XuiSetTimer(m_hObj,JOIN_LOAD_ONLINE_TIMER_ID,JOIN_LOAD_ONLINE_TIMER_TIME);

	m_iSaveInfoC=0;

	VOID *pObj;
	XuiObjectFromHandle( m_SavesList, &pObj );
	m_pSavesList = (CXuiCtrl4JList *)pObj;

	XuiObjectFromHandle( m_GamesList, &pObj );
	m_pGamesList = (CXuiCtrl4JList *)pObj;

	// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
	if(app.StartInstallDLCProcess(m_iPad)==true)
	{
		// not doing a mount, so enable input
		m_bIgnoreInput=true;
	}
	else
	{
		// if we're waiting for DLC to mount, don't fill the save list. The custom message on end of dlc mounting will do that
		m_bIgnoreInput=false;



		m_iChangingSaveGameInfoIndex = 0;

		m_generators = app.getLevelGenerators();
		m_iDefaultButtonsC = 0;
		m_iMashUpButtonsC=0;

		// check if we're in the trial version
		if(ProfileManager.IsFullVersion()==false)
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1, -1, -1,iLB);

			AddDefaultButtons();

			m_pSavesList->SetCurSelVisible(0);
		}
		else if(StorageManager.GetSaveDisabled())
		{
			if(StorageManager.GetSaveDeviceSelected(m_iPad))
			{
				// saving is disabled, but we should still be able to load from a selected save device

				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,-1,-1,-1,iLB,IDS_TOOLTIPS_DELETESAVE);

				GetSaveInfo();
			}
			else
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,-1,-1,-1,iLB);

				AddDefaultButtons();
				m_SavesListTimer.SetShow( FALSE );

				m_pSavesList->SetCurSelVisible(0);
			}
		}
		else
		{
			// 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)
			bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,-1,-1,-1,-1,bCanRename?IDS_TOOLTIPS_SAVEOPTIONS:IDS_TOOLTIPS_DELETESAVE);

			GetSaveInfo();
		}
	}
	//XuiElementSetDisableFocusRecursion( m_pGamesList->m_hObj, TRUE);

	UpdateGamesList();

	g_NetworkManager.SetSessionsUpdatedCallback( &CScene_MultiGameJoinLoad::UpdateGamesListCallback, this );

	// 4J Stu - Fix for #12530 -TCR 001 BAS Game Stability: Title will crash if the player disconnects while starting a new world and then opts to play the tutorial once they have been returned to the Main Menu.
	MinecraftServer::resetFlags();

	// If we're not ignoring input, then we aren't still waiting for the DLC to mount, and can now check for corrupt dlc. Otherwise this will happen when the dlc has finished mounting.
	if( !m_bIgnoreInput)
	{
		app.m_dlcManager.checkForCorruptDLCAndAlert();
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

	// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this

	DLC_INFO *pDLCInfo=NULL;

	// first pass - look to see if there are any that are not in the list
	bool bTexturePackAlreadyListed;
	bool bNeedToGetTPD=false;
	Minecraft *pMinecraft = Minecraft::GetInstance();
	int texturePacksCount = pMinecraft->skins->getTexturePackCount();
	//CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
	//HRESULT hr;


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

	XuiSetTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);

	return S_OK;
}

void CScene_MultiGameJoinLoad::AddDefaultButtons()
{
	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;

	// Add two for New Game and Tutorial
	ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));

	ListInfo.pwszText = app.GetString(IDS_CREATE_NEW_WORLD);
	ListInfo.fEnabled = TRUE;
	ListInfo.iData = -1;
	m_pSavesList->AddData(ListInfo);

	int iSavesListIndex = 0;
	int iGeneratorIndex = 0;
	m_iMashUpButtonsC=0;

    for (LevelGenerationOptions *levelGen : *m_generators )
    {
		ListInfo.pwszText = levelGen->getWorldName();
		ListInfo.fEnabled = TRUE;
		ListInfo.iData = iGeneratorIndex++; // used to index into the list of generators

		// need to check if the user has disabled this pack in the save display list
		unsigned int uiTexturePackID=levelGen->getRequiredTexturePackId();

		if(uiTexturePackID!=0)
		{
			unsigned int uiMashUpWorldsBitmask=app.GetMashupPackWorlds(m_iPad);

			if((uiMashUpWorldsBitmask & (1<<(uiTexturePackID-1024)))==0)
			{
				// this world is hidden, so skip
				continue;
			}
		}
		m_pSavesList->AddData(ListInfo);

		// retrieve the save icon from the texture pack, if there is one
		if(uiTexturePackID!=0)
		{
			// increment the count of the mash-up pack worlds in the save list
			m_iMashUpButtonsC++;
			TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(levelGen->getRequiredTexturePackId());
			DWORD dwImageBytes;
			PBYTE pbImageData = tp->getPackIcon(dwImageBytes);
			HXUIBRUSH hXuiBrush;

			if(dwImageBytes > 0 && pbImageData)
			{
				XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&hXuiBrush);
				// the index inside the list item for this will be i+1 because they start at m_vListData.size(), so the first etry (tutorial) is 1
				m_pSavesList->UpdateGraphic(iSavesListIndex+1,hXuiBrush);
			}
		}

		++iSavesListIndex;
	}

	m_iDefaultButtonsC = iSavesListIndex + 1;
}


HRESULT CScene_MultiGameJoinLoad::GetSaveInfo(  )
{
	unsigned int uiSaveC=0;

	// This will return with the number retrieved in uiSaveC

	if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
	{
		uiSaveC = 0;
		File savesDir(L"GAME:\\Saves");
		if( savesDir.exists() )
		{
			m_saves = savesDir.listFiles();
			uiSaveC = (unsigned int)m_saves->size();
		}
		// add the New Game and Tutorial after the saves list is retrieved, if there are any saves

		// Add two for New Game and Tutorial
		unsigned int listItems = uiSaveC;

		CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
		ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));

		AddDefaultButtons();

		for(unsigned int i=0;i<listItems;i++)
		{

			wstring wName = m_saves->at(i)->getName();
			wchar_t *name = new wchar_t[wName.size()+1];
			for(unsigned int j = 0; j < wName.size(); ++j)
			{
				name[j] = wName[j];
			}
			name[wName.size()] = 0;
			ListInfo.pwszText = name;
			ListInfo.fEnabled=TRUE;
			ListInfo.iData = -1;
			m_pSavesList->AddData(ListInfo);
		}
		m_pSavesList->SetCurSelVisible(0);
	}
	else
	{
		m_bRetrievingSaveInfo=true; // we're blocking the exit from this scene until complete

		// clear the saves list
		m_pSavesList->RemoveAllData();

		m_iSaveInfoC=0;
#ifdef _XBOX
		C4JStorage::ESGIStatus eSGIStatus=StorageManager.GetSavesInfo(ProfileManager.GetPrimaryPad(),&CScene_MultiGameJoinLoad::GetSavesInfoCallback,this,"savegame.dat");

		if(eSGIStatus==C4JStorage::ESGIStatus_NoSaves)
		{
			uiSaveC=0;
			m_SavesListTimer.SetShow( FALSE );
			m_SavesList.SetEnable(TRUE);
		}
#else

		//C4JStorage::ESaveGameState eStatus=StorageManager.GetSavesInfo(ProfileManager.GetPrimaryPad(),&CScene_MultiGameJoinLoad::GetSavesInfoCallback,this,"savegame.dat");

#endif
	}

	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnDestroy()
{
	g_NetworkManager.SetSessionsUpdatedCallback( NULL, NULL );

    for (auto& it : currentSessions )
    {
		delete it;
	}

	if(m_bSaveTransferInProgress)
	{
		CancelSaveUploadCallback(this);
	}

	// Reset the background downloading, in case we changed it by attempting to download a texture pack
	XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);

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


int CScene_MultiGameJoinLoad::DeviceRemovedDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline)
	{
		StorageManager.SetSaveDisabled(true);
		StorageManager.SetSaveDeviceSelected(ProfileManager.GetPrimaryPad(),false);
		// use the device select returned function to wipe the saves list and change the tooltip
		CScene_MultiGameJoinLoad::DeviceSelectReturned(pClass,true);
	}
	else // continue without saving
	{
		// Change device
		StorageManager.SetSaveDevice(&CScene_MultiGameJoinLoad::DeviceSelectReturned,pClass,true);
	}

	pClass->m_bIgnoreInput=false;
	return 0;
}
//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameJoinLoad::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// if we're retrieving save info, ignore key presses
	if(m_bRetrievingSaveInfo)
	{
		return S_OK;
	}

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if ( hObjPressed == m_GamesList )
	{
		m_bIgnoreInput=true;

		DWORD nIndex = m_pGamesList->GetCurSel();

		if( m_pGamesList->GetItemCount() > 0 && nIndex < currentSessions.size() )
		{
			//CScene_MultiGameInfo::JoinMenuInitData *initData = new CScene_MultiGameInfo::JoinMenuInitData();
			m_initData->iPad = m_iPad;
			m_initData->selectedSession = currentSessions.at( nIndex );

			// check that we have the texture pack available
			// If it's not the default texture pack
			if(m_initData->selectedSession->data.texturePackParentId!=0)
			{
				int texturePacksCount = Minecraft::GetInstance()->skins->getTexturePackCount();
				bool bHasTexturePackInstalled=false;

				for(int i=0;i<texturePacksCount;i++)
				{
					TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackByIndex(i);
					if(tp->getDLCParentPackId()==m_initData->selectedSession->data.texturePackParentId)
					{
						bHasTexturePackInstalled=true;
						break;
					}
				}

				if(bHasTexturePackInstalled==false)
				{
					// upsell the texture pack
					// tell sentient about the upsell of the full version of the skin pack
					ULONGLONG ullOfferID_Full;
					app.GetDLCFullOfferIDForPackID(m_initData->selectedSession->data.texturePackParentId,&ullOfferID_Full);

					TelemetryManager->RecordUpsellPresented(pNotifyPressData->UserIndex, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

					UINT uiIDA[3];

					// Need to check if the texture pack has both Full and Trial versions - we may do some as free ones, so only Full
					DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);

					if(pDLCInfo->ullOfferID_Trial!=0LL)
					{
						uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
						uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
						uiIDA[2]=IDS_CONFIRM_CANCEL;
						// Give the player a warning about the texture pack missing
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&CScene_MultiGameJoinLoad::TexturePackDialogReturned,this,app.GetStringTable());
					}
					else
					{
						uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
						uiIDA[1]=IDS_CONFIRM_CANCEL;
						// Give the player a warning about the texture pack missing
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CScene_MultiGameJoinLoad::TexturePackDialogReturned,this,app.GetStringTable());
					}

					return S_OK;
				}
			}

			m_NetGamesListTimer.SetShow( FALSE );

			// Reset the background downloading, in case we changed it by attempting to download a texture pack
			XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);

			// kill the texture pack check timer
			XuiKillTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
			app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_JoinMenu,m_initData);
		}
	}
	else if(hObjPressed==m_SavesList)
	{
		m_bIgnoreInput=true;

		CXuiControl pItem;
		int iIndex;
		// get the selected item
		iIndex=m_SavesList.GetCurSel(&pItem);

		CXuiCtrl4JList::LIST_ITEM_INFO info = m_pSavesList->GetData(iIndex);

		if(iIndex == JOIN_LOAD_CREATE_BUTTON_INDEX)
		{
			app.SetTutorialMode( false );
			m_NetGamesListTimer.SetShow( FALSE );

			app.SetCorruptSaveDeleted(false);

			CreateWorldMenuInitData *params = new CreateWorldMenuInitData();
			params->iPad = m_iPad;
			app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_CreateWorldMenu,(void *)params);
		}
		else if(info.iData >= 0)
		{
			LevelGenerationOptions *levelGen = m_generators->at(info.iData);
			app.SetTutorialMode( levelGen->isTutorial() );
			// Reset the autosave time
			app.SetAutosaveTimerTime();

			if(levelGen->isTutorial())
			{
				LoadLevelGen(levelGen);
			}
			else
			{
				LoadMenuInitData *params = new LoadMenuInitData();
				params->iPad = m_iPad;
				// need to get the iIndex from the list item, since the position in the list doesn't correspond to the GetSaveGameInfo list because of sorting
				params->iSaveGameInfoIndex=-1;
				//params->pbSaveRenamed=&m_bSaveRenamed;
				params->levelGen = levelGen;

				// navigate to the settings scene
				app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LoadMenu, params);
			}
		}
		else
		{
			// check if this is a damaged save
			if(m_pSavesList->GetData(iIndex).bIsDamaged)
			{
				// give the option to delete the save
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox(IDS_CORRUPT_OR_DAMAGED_SAVE_TITLE, IDS_CORRUPT_OR_DAMAGED_SAVE_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_MultiGameJoinLoad::DeleteSaveDialogReturned,this, app.GetStringTable());
			}
			else
			{
				app.SetTutorialMode( false );
				if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
				{
					LoadSaveFromDisk(m_saves->at(iIndex-m_iDefaultButtonsC));
				}
				else
				{
					LoadMenuInitData *params = new LoadMenuInitData();
					params->iPad = m_iPad;
					// need to get the iIndex from the list item, since the position in the list doesn't correspond to the GetSaveGameInfo list because of sorting
					params->iSaveGameInfoIndex=m_pSavesList->GetData(iIndex).iIndex-m_iDefaultButtonsC;
					//params->pbSaveRenamed=&m_bSaveRenamed;
					params->levelGen = NULL;

					// kill the texture pack timer
					XuiKillTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
					// navigate to the settings scene
					app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LoadMenu, params);
				}
			}
		}
	}

	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// if we're retrieving save info, ignore key presses
	if(m_bRetrievingSaveInfo)
	{
		return S_OK;
	}

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr = S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		m_NetGamesListTimer.SetShow( FALSE );

		app.NavigateBack(XUSER_INDEX_ANY);
		rfHandled = TRUE;
		break;
	case VK_PAD_X:

		// Change device
		// Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage
		// device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
		m_bIgnoreInput=true;
		StorageManager.SetSaveDevice(&CScene_MultiGameJoinLoad::DeviceSelectReturned,this,true);
		CXuiSceneBase::PlayUISFX(eSFX_Press);
		break;
	case VK_PAD_Y:
		if(m_pGamesList->TreeHasFocus() && m_pGamesList->GetItemCount() > 0)
		{
			DWORD nIndex = m_pGamesList->GetCurSel();
			FriendSessionInfo *pSelectedSession = currentSessions.at( nIndex );

			PlayerUID xuid = pSelectedSession->data.hostPlayerUID;
			if( xuid != INVALID_XUID )
				hr = XShowGamerCardUI(ProfileManager.GetLockedProfile(), xuid);
			CXuiSceneBase::PlayUISFX(eSFX_Press);
		}
		else if(DoesSavesListHaveFocus())
		{
			// save transfer - make sure they want to overwrite a save that is up there
			if(ProfileManager.IsSignedInLive( m_iPad ))
			{
				// 4J-PB - required for a delete of the save if it's found to be a corrupted save
				DWORD nIndex = m_pSavesList->GetCurSel();
				m_iChangingSaveGameInfoIndex=m_pSavesList->GetData(nIndex).iIndex;

				UINT uiIDA[2];
				uiIDA[0]=IDS_UPLOAD_SAVE;
				uiIDA[1]=IDS_CONFIRM_CANCEL;

				ui.RequestMessageBox(IDS_SAVE_TRANSFER_TITLE, IDS_SAVE_TRANSFER_TEXT, uiIDA, 2, pInputData->UserIndex,&CScene_MultiGameJoinLoad::SaveTransferDialogReturned,this, app.GetStringTable());
			}
		}
		break;
	case VK_PAD_RSHOULDER:
		if(DoesSavesListHaveFocus())
		{
			m_bIgnoreInput = true;

			int iIndex=m_SavesList.GetCurSel();
			m_iChangingSaveGameInfoIndex=m_pSavesList->GetData(iIndex).iIndex;

			// Could be delete save or Save Options
			if(StorageManager.GetSaveDisabled())
			{
				// delete the save game
				// Have to ask the player if they are sure they want to delete this game
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, pInputData->UserIndex,&CScene_MultiGameJoinLoad::DeleteSaveDialogReturned,this, app.GetStringTable());
			}
			else
			{
				if(StorageManager.EnoughSpaceForAMinSaveGame())
				{
					UINT uiIDA[3];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_TITLE_RENAMESAVE;
					uiIDA[2]=IDS_TOOLTIPS_DELETESAVE;
					StorageManager.RequestMessageBox(IDS_TOOLTIPS_SAVEOPTIONS, IDS_TEXT_SAVEOPTIONS, uiIDA, 3, pInputData->UserIndex,&CScene_MultiGameJoinLoad::SaveOptionsDialogReturned,this, app.GetStringTable());
				}
				else
				{
					// delete the save game
					// Have to ask the player if they are sure they want to delete this game
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, pInputData->UserIndex,&CScene_MultiGameJoinLoad::DeleteSaveDialogReturned,this, app.GetStringTable());
				}
			}
			CXuiSceneBase::PlayUISFX(eSFX_Press);

		}
		else if(DoesMashUpWorldHaveFocus())
		{
			// hiding a mash-up world
			// get the mash-up pack id
			CXuiControl pItem;
			int iIndex;
			iIndex=m_SavesList.GetCurSel(&pItem);

			CXuiCtrl4JList::LIST_ITEM_INFO info = m_pSavesList->GetData(iIndex);
			if((iIndex != JOIN_LOAD_CREATE_BUTTON_INDEX) && (info.iData >= 0))
			{
				LevelGenerationOptions *levelGen = m_generators->at(info.iData);

				if(!levelGen->isTutorial())
				{
					if(levelGen->requiresTexturePack())
					{
						unsigned int uiPackID=levelGen->getRequiredTexturePackId();

						m_bIgnoreInput = true;
						app.HideMashupPackWorld(m_iPad,uiPackID);

						// update the saves list
						m_pSavesList->RemoveAllData();
						m_iSaveInfoC=0;
						GetSaveInfo();
						m_bIgnoreInput = false;
					}
				}
			}

			CXuiSceneBase::PlayUISFX(eSFX_Press);

		}
		break;
	case VK_PAD_LSHOULDER:
		if( m_bInParty )
		{
			m_bShowingPartyGamesOnly = !m_bShowingPartyGamesOnly;
			UpdateGamesList();
			CXuiSceneBase::PlayUISFX(eSFX_Press);
		}
		break;
	}

	return hr;
}

HRESULT CScene_MultiGameJoinLoad::OnNavReturn(HXUIOBJ hSceneFrom,BOOL& rfHandled)
{

	CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
	// start the texture pack timer again
	XuiSetTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);

	m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);

	// re-enable button presses
	m_bIgnoreInput=false;

	if( m_bMultiplayerAllowed )
	{
		HXUICLASS hClassFullscreenProgress = XuiFindClass( L"CScene_FullscreenProgress" );
		HXUICLASS hClassConnectingProgress = XuiFindClass( L"CScene_ConnectingProgress" );

		// If we are navigating back from a full screen progress scene, then that means a connection attempt failed
		if( XuiIsInstanceOf( hSceneFrom, hClassFullscreenProgress ) || XuiIsInstanceOf( hSceneFrom, hClassConnectingProgress ) )
		{
			UpdateGamesList();
		}
	}
	else
	{
		m_pGamesList->RemoveAllData();
		//m_GamesList.DeleteItems(0, m_GamesList.GetItemCount() );
		m_pGamesList->SetEnable(FALSE);
		//XuiElementSetDisableFocusRecursion( m_pGamesList->m_hObj, TRUE);
		m_NetGamesListTimer.SetShow( TRUE );
		m_LabelNoGames.SetShow( FALSE );
		m_SavesList.InitFocus(m_iPad);
	}

	// are we back here because of a delete of a corrupt save?

	if(app.GetCorruptSaveDeleted())
	{
		// need to re-get the saves list and update the display
		// clear the saves list
		m_pSavesList->RemoveAllData();
		m_iSaveInfoC=0;
		GetSaveInfo();
		app.SetCorruptSaveDeleted(false);
	}

	int iY = -1;
	int iRB=-1;
	if( DoesGamesListHaveFocus() )
	{
		iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
	}
	else if(DoesSavesListHaveFocus())
	{
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
		}

		if(StorageManager.GetSaveDisabled())
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
		else
		{
			// 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)

			if(StorageManager.EnoughSpaceForAMinSaveGame())
			{
				iRB=IDS_TOOLTIPS_SAVEOPTIONS;
			}
			else
			{
				iRB=IDS_TOOLTIPS_DELETESAVE;

			}
		}
	}
	else if(DoesMashUpWorldHaveFocus())
	{
		// If it's a mash-up pack world, give the Hide option
		iRB=IDS_TOOLTIPS_HIDE;
	}

	int iLB = -1;
	if(m_bInParty)
	{
		if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
		else iLB = IDS_TOOLTIPS_PARTY_GAMES;
	}

	if(ProfileManager.IsFullVersion()==false )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1,-1,-1,iLB);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		// clear out the saves list, since the disable save may have happened in the load screen because of a device removal

		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE, iY,-1,-1,iLB,iRB);
	}

	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{

	if(m_bReady)
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}

	return S_OK;
}


HRESULT CScene_MultiGameJoinLoad::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ||
		pTransition->dwTransType == XUI_TRANSITION_FROM  || pTransition->dwTransType == XUI_TRANSITION_BACKFROM)
	{
		// 4J Stu - We may have had to unload our font renderer in this scene if one of the save files
		// uses characters not in our font (eg asian chars) so restore our font renderer
		// This will not do anything if our font renderer is already loaded
		app.OverrideFontRenderer(true,true);

		KillTimer(JOIN_LOAD_ONLINE_TIMER_ID);
	}
	else if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		SetTimer(JOIN_LOAD_ONLINE_TIMER_ID,JOIN_LOAD_ONLINE_TIMER_TIME);
		// 4J-PB - Need to check for installed DLC, which might have happened while you were on the info scene
		if(pTransition->dwTransType == XUI_TRANSITION_BACKTO)
		{
			// Can't call this here because if you back out of the load info screen and then go back in and load a game, it will attempt to use the dlc as it's running a mount of the dlc

			// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
			if(app.StartInstallDLCProcess(m_iPad)==false)
			{
				// not doing a mount, so re-enable input
				m_bIgnoreInput=false;
			}
			else
			{
				m_bIgnoreInput=true;
				m_pSavesList->RemoveAllData();
				m_SavesListTimer.SetShow( TRUE );
			}
		}
	}

	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnFontRendererChange()
{
	// update the tooltips
	// if the saves list has focus, then we should show the Delete Save tooltip
	// if the games list has focus, then we should the the View Gamercard tooltip
	int iRB=-1;
	int iY = -1;
	if( DoesGamesListHaveFocus() )
	{
		iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
	}
	else if(DoesSavesListHaveFocus())
	{
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
		}
		if(StorageManager.GetSaveDisabled())
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
		else
		{
			if(StorageManager.EnoughSpaceForAMinSaveGame())
			{
				iRB=IDS_TOOLTIPS_SAVEOPTIONS;
			}
			else
			{
				iRB=IDS_TOOLTIPS_DELETESAVE;
			}
		}
	}
	else if(DoesMashUpWorldHaveFocus())
	{
		// If it's a mash-up pack world, give the Hide option
		iRB=IDS_TOOLTIPS_HIDE;
	}

	int iLB = -1;
	if(m_bInParty)
	{
		if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
		else iLB = IDS_TOOLTIPS_PARTY_GAMES;
	}

	if(ProfileManager.IsFullVersion()==false )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, iY,-1,-1,iLB,-1,-1,true);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB,-1,true);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE, iY,-1,-1,iLB,iRB,-1,true);
	}
	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	// update the tooltips
	// if the saves list has focus, then we should show the Delete Save tooltip
	// if the games list has focus, then we should the the View Gamercard tooltip
	int iRB=-1;
	int iY = -1;
	if( DoesGamesListHaveFocus() )
	{
		iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
	}
	else if(DoesSavesListHaveFocus())
	{
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
		}
		if(StorageManager.GetSaveDisabled())
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
		else
		{
			if(StorageManager.EnoughSpaceForAMinSaveGame())
			{
				iRB=IDS_TOOLTIPS_SAVEOPTIONS;
			}
			else
			{
				iRB=IDS_TOOLTIPS_DELETESAVE;
			}
		}
	}
	else if(DoesMashUpWorldHaveFocus())
	{
		// If it's a mash-up pack world, give the Hide option
		iRB=IDS_TOOLTIPS_HIDE;
	}

	int iLB = -1;
	if(m_bInParty)
	{
		if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
		else iLB = IDS_TOOLTIPS_PARTY_GAMES;
	}

	if(ProfileManager.IsFullVersion()==false )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, iY,-1,-1,iLB,-1);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE, iY,-1,-1,iLB,iRB);
	}
	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	return S_OK;
}

bool CScene_MultiGameJoinLoad::DoesSavesListHaveFocus()
{
	HXUIOBJ hParentObj,hObj=TreeGetFocus();

	if(hObj!=NULL)
	{
		// get the parent and see if it's the saves list
		XuiElementGetParent(hObj,&hParentObj);
		if(hParentObj==m_SavesList.m_hObj)
		{
			// check it's not the first or second element (new world or tutorial)
			if(m_SavesList.GetCurSel()>(m_iDefaultButtonsC-1))
			{
				return true;
			}
		}
	}
	return false;
}

bool CScene_MultiGameJoinLoad::DoesMashUpWorldHaveFocus()
{
	HXUIOBJ hParentObj,hObj=TreeGetFocus();

	if(hObj!=NULL)
	{
		// get the parent and see if it's the saves list
		XuiElementGetParent(hObj,&hParentObj);
		if(hParentObj==m_SavesList.m_hObj)
		{
			// check it's not the first or second element (new world or tutorial)
			if(m_SavesList.GetCurSel()>(m_iDefaultButtonsC-1))
			{
				return false;
			}

			if(m_SavesList.GetCurSel()>(m_iDefaultButtonsC - 1 - m_iMashUpButtonsC))
			{
				return true;
			}
			else return false;
		}
		else return false;
	}
	return false;
}

bool CScene_MultiGameJoinLoad::DoesGamesListHaveFocus()
{
	HXUIOBJ hParentObj,hObj=TreeGetFocus();

	if(hObj!=NULL)
	{
		// get the parent and see if it's the saves list
		XuiElementGetParent(hObj,&hParentObj);
		if(hParentObj==m_pGamesList->m_hObj)
		{
			return true;
		}
	}
	return false;
}

void CScene_MultiGameJoinLoad::UpdateGamesListCallback(LPVOID lpParam)
{
	if(lpParam != NULL)
	{
		CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad *) lpParam;
		// check this there's no save transfer in progress
		if(!pClass->m_bSaveTransferInProgress)
		{
				pClass->UpdateGamesList();
		}
	}
}

void CScene_MultiGameJoinLoad::UpdateGamesList()
{
	if( m_bIgnoreInput ) return;

	// if we're retrieving save info, don't show the list yet as we will be ignoring press events
	if(m_bRetrievingSaveInfo)
	{
		return;
	}

	DWORD nIndex = -1;
	FriendSessionInfo *pSelectedSession = NULL;
	if(m_pGamesList->TreeHasFocus() && m_pGamesList->GetItemCount() > 0)
	{
		nIndex = m_pGamesList->GetCurSel();
		pSelectedSession = currentSessions.at( nIndex );
	}

	SessionID selectedSessionId;
	if( pSelectedSession != NULL )selectedSessionId = pSelectedSession->sessionId;
	pSelectedSession = NULL;

    for (auto& it : currentSessions )
    {
		delete it;
	}
	currentSessions.clear();

	m_NetGamesListTimer.SetShow( FALSE );

	// if the saves list has focus, then we should show the Delete Save tooltip
	// if the games list has focus, then we should show the View Gamercard tooltip
	int iRB=-1;
	int iY = -1;

	if( DoesGamesListHaveFocus() )
	{
		iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
	}
	else if(DoesSavesListHaveFocus())
	{
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
		}
		if(StorageManager.GetSaveDisabled())
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
		else
		{
			if(StorageManager.EnoughSpaceForAMinSaveGame())
			{
				iRB=IDS_TOOLTIPS_SAVEOPTIONS;
			}
			else
			{
				iRB=IDS_TOOLTIPS_DELETESAVE;
			}
		}
	}
	else if(DoesMashUpWorldHaveFocus())
	{
		// If it's a mash-up pack world, give the Hide option
		iRB=IDS_TOOLTIPS_HIDE;
	}

	int iLB = -1;
	if(m_bInParty)
	{
		if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
		else iLB = IDS_TOOLTIPS_PARTY_GAMES;
	}

	if(ProfileManager.IsFullVersion()==false )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, iY,-1,-1,iLB,-1);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE, iY,-1,-1,iLB,iRB);
	}

	currentSessions = *g_NetworkManager.GetSessionList( m_iPad, m_localPlayers, m_bShowingPartyGamesOnly );

	// Update the xui list displayed
	unsigned int xuiListSize = m_pGamesList->GetItemCount();
	unsigned int filteredListSize = (unsigned int)currentSessions.size();

	BOOL gamesListHasFocus = m_pGamesList->TreeHasFocus();

	if(filteredListSize > 0)
	{
		if( !m_pGamesList->IsEnabled() )
		{
			m_pGamesList->SetEnable(TRUE);
			//XuiElementSetDisableFocusRecursion( m_pGamesList->m_hObj, FALSE);
			m_pGamesList->SetCurSel( 0 );
		}
		m_LabelNoGames.SetShow( FALSE );
		m_NetGamesListTimer.SetShow( FALSE );
	}
	else
	{
		m_pGamesList->SetEnable(FALSE);
		//XuiElementSetDisableFocusRecursion(m_pGamesList->m_hObj, TRUE);
		m_NetGamesListTimer.SetShow( FALSE );
		m_LabelNoGames.SetShow( TRUE );

		if( gamesListHasFocus ) m_pGamesList->InitFocus(m_iPad);
	}

	// clear out the games list and re-fill
	m_pGamesList->RemoveAllData();

	if( filteredListSize > 0 )
	{
		// Reset the focus to the selected session if it still exists
		unsigned int sessionIndex = 0;
		m_pGamesList->SetCurSel(0);

        for ( FriendSessionInfo *sessionInfo : currentSessions )
        {
			HXUIBRUSH hXuiBrush;
			CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;

			ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));

			ListInfo.pwszText = sessionInfo->displayLabel;
			ListInfo.fEnabled = TRUE;
			ListInfo.iData = sessionIndex;
			m_pGamesList->AddData(ListInfo);
			// display an icon too

			// Is this a default game or a texture pack game?
			if(sessionInfo->data.texturePackParentId!=0)
			{
				// Do we have the texture pack
				Minecraft *pMinecraft = Minecraft::GetInstance();
				TexturePack *tp = pMinecraft->skins->getTexturePackById(sessionInfo->data.texturePackParentId);
				HRESULT hr;

				DWORD dwImageBytes=0;
				PBYTE pbImageData=NULL;

				if(tp==NULL)
				{
					DWORD dwBytes=0;
					PBYTE pbData=NULL;
					app.GetTPD(sessionInfo->data.texturePackParentId,&pbData,&dwBytes);

					// is it in the tpd data ?
					app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbImageData,&dwImageBytes );
					if(dwImageBytes > 0 && pbImageData)
					{
						hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&hXuiBrush);
						m_pGamesList->UpdateGraphic(sessionIndex,hXuiBrush);
					}
				}
				else
				{
					pbImageData = tp->getPackIcon(dwImageBytes);
					if(dwImageBytes > 0 && pbImageData)
					{
						hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&hXuiBrush);
						m_pGamesList->UpdateGraphic(sessionIndex,hXuiBrush);
					}
				}
			}
			else
			{
				// default texture pack
				XuiCreateTextureBrushFromMemory(m_DefaultMinecraftIconData,m_DefaultMinecraftIconSize,&hXuiBrush);
				m_pGamesList->UpdateGraphic(sessionIndex,hXuiBrush);
			}


			if(memcmp( &selectedSessionId, &sessionInfo->sessionId, sizeof(SessionID) ) == 0)
			{
				m_pGamesList->SetCurSel(sessionIndex);
				break;
			}
			++sessionIndex;
		}
	}
}

void CScene_MultiGameJoinLoad::UpdateGamesList(DWORD dwNumResults, IQNetGameSearch *pGameSearch)
{
	// We don't use the QNet callback, but could resurrect this if we ever do normal matchmaking, but updated to work as the function above
#if 0
	const XSESSION_SEARCHRESULT *pSearchResult;
    const XNQOSINFO * pxnqi;

	if(m_searches>0)
		--m_searches;

	if(m_searches==0)
	{
		m_NetGamesListTimer.SetShow( FALSE );

		// if the saves list has focus, then we should show the Delete Save tooltip
		// if the games list has focus, then we should show the View Gamercard tooltip
		int iRB=-1;
		int iY = -1;

		if( DoesGamesListHaveFocus() )
		{
			iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
		}
		else if(DoesSavesListHaveFocus())
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}

		int iLB = -1;
		if(m_bInParty)
		{
			if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES
			else iLB = IDS_TOOLTIPS_PARTY_GAMES;
		}

		if(ProfileManager.IsFullVersion()==false )
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, iY,-1,-1,iLB,iRB);
		}
		else if(StorageManager.GetSaveDisabled())
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB);
		}
		else
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE, iY,-1,-1,iLB,iRB);
		}
	}

	if( dwNumResults == 0 )
	{
		if(m_searches==0 && m_GamesList.GetItemCount() == 0)
		{
			m_LabelNoGames.SetShow( TRUE );
		}
		return;
	}

	unsigned int startOffset = m_GamesList.GetItemCount();
	//m_GamesList.InsertItems(startOffset,dwNumResults);
	//m_GamesList.SetEnable(TRUE);
	//XuiElementSetDisableFocusRecursion( m_GamesList.m_hObj, FALSE);

	// Loop through all the results.
    for( DWORD dwResult = 0; dwResult < pGameSearch->GetNumResults(); dwResult++ )
    {

        pSearchResult = pGameSearch->GetSearchResultAtIndex( dwResult );

		// No room for us, so ignore it
		if(pSearchResult->dwOpenPublicSlots < m_localPlayers)
			continue;

		FriendSessionInfo *sessionInfo = NULL;
		bool foundSession = false;
		for( auto it = friendsSessions.begin(); it != friendsSessions.end(); ++it)
		{
			sessionInfo = *it;
			if(memcmp( &pSearchResult->info.sessionID, &sessionInfo->sessionId, sizeof(SessionID) ) == 0)
			{
				sessionInfo->searchResult = *pSearchResult;
				sessionInfo->displayLabel = new wchar_t[100];
				foundSession = true;
				break;
			}
		}

		// We received a search result for a session no longer in our list of friends sessions
		if(!foundSession)
			continue;

        // Print some info about this result.
        //printf( "Search result %u:\n", dwResult );
        //printf( "    public slots open = %u, filled = %u\n", pSearchResult->dwOpenPublicSlots, pSearchResult->dwFilledPublicSlots );
        //printf( "    private slots open = %u, filled = %u\n", pSearchResult->dwOpenPrivateSlots, pSearchResult->dwFilledPrivateSlots );

        // See if this result was contacted successfully via QoS probes.
        pxnqi = pGameSearch->GetQosInfoAtIndex( dwResult );
        if( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_CONTACTED )
        {
            // Print the round trip time and the rough estimation of
            // bandwidth.
            app.DebugPrintf( "    RTT min = %u, med = %u\n", pxnqi->wRttMinInMsecs, pxnqi->wRttMedInMsecs );
            app.DebugPrintf( "    bps up = %u, down = %u\n", pxnqi->dwUpBitsPerSec, pxnqi->dwDnBitsPerSec );

			if(pxnqi->cbData > 0)
			{
				sessionInfo->data = *(GameSessionData *)pxnqi->pbData;

				wstring gamerName = convStringToWstring(sessionInfo->data.hostName);
				swprintf(sessionInfo->displayLabel,L"%ls's Game", gamerName.c_str() );
			}
			else
			{
				swprintf(sessionInfo->displayLabel,L"Unknown host Game");
			}

            // If this host wasn't disabled use this one.
            if( !( pxnqi->bFlags & XNET_XNQOSINFO_TARGET_DISABLED ) && sessionInfo->data.netVersion == MINECRAFT_NET_VERSION )
            {
				//printf("This game is valid\n");
				filteredResults.push_back(sessionInfo);
				m_GamesList.InsertItems(startOffset,1);
				m_GamesList.SetText(startOffset,sessionInfo->displayLabel);
				startOffset++;
            }
#ifndef _CONTENT_PACKAGE
			if( sessionInfo->data.netVersion != MINECRAFT_NET_VERSION )
			{
				wprintf(L"%ls version of %d does not match our version of %d\n", sessionInfo->displayLabel, sessionInfo->data.netVersion, MINECRAFT_NET_VERSION);
			}
#endif
        }
    }

	if( m_GamesList.GetItemCount() == 0)
	{
		m_LabelNoGames.SetShow( TRUE );
	}
	else
	{
		m_GamesList.SetEnable(TRUE);
		XuiElementSetDisableFocusRecursion( m_GamesList.m_hObj, FALSE);
		if( DoesGamesListHaveFocus() )
		{
			m_GamesList.SetCurSel(0);
		}
	}
#endif
}

/*void CScene_MultiGameJoinLoad::UpdateGamesListLabels()
{
	for( unsigned int i = 0; i < currentSessions.size(); ++i )
	{
		FriendSessionInfo *sessionInfo = currentSessions.at(i);
		m_GamesList.SetText(i,sessionInfo->displayLabel);
		HXUIBRUSH hBrush;
		CXuiCtrl4JList::LIST_ITEM_INFO info = m_pGamesList->GetData(i);

		// display an icon too
		XuiCreateTextureBrushFromMemory(m_DefaultMinecraftIconData,m_DefaultMinecraftIconSize,&hBrush);
		m_pGamesList->UpdateGraphic(i,hBrush);
	}
#if 0
	XUIRect xuiRect;
	HXUIOBJ item = XuiListGetItemControl(m_GamesList,0);

	HXUIOBJ hObj=NULL;
	HXUIOBJ hTextPres=NULL;
 	HRESULT hr=XuiControlGetVisual(item,&hObj);
	hr=XuiElementGetChildById(hObj,L"text_Label",&hTextPres);

	unsigned char displayLabelViewableStartIndex = 0;
	for( unsigned int i = 0; i < currentSessions.size(); ++i )
	{
		FriendSessionInfo *sessionInfo = currentSessions.at(i);

		if(hTextPres != NULL )
		{
			hr=XuiTextPresenterMeasureText(hTextPres, sessionInfo->displayLabel, &xuiRect);

			float fWidth, fHeight;
			XuiElementGetBounds(hTextPres,&fWidth,&fHeight);
			int characters = (fWidth/xuiRect.right) * sessionInfo->displayLabelLength;

			if( characters < sessionInfo->displayLabelLength )
			{
				static wchar_t temp[100];
				ZeroMemory(temp, (100)*sizeof(wchar_t));
				wcsncpy_s( temp, sessionInfo->displayLabel+sessionInfo->displayLabelViewableStartIndex, characters );
				m_GamesList.SetText(i,temp);
				sessionInfo->displayLabelViewableStartIndex++;
				if( sessionInfo->displayLabelViewableStartIndex >= sessionInfo->displayLabelLength ) sessionInfo->displayLabelViewableStartIndex = 0;
			}
		}
	}
#endif
}*/

void CScene_MultiGameJoinLoad::SearchForGameCallback(void *param, DWORD dwNumResults, IQNetGameSearch *pGameSearch)
{
#if 0
	HXUIOBJ hObj = (HXUIOBJ)param;

	void *pObj;
	XuiObjectFromHandle( hObj, &pObj);
	CScene_MultiGameJoinLoad *MultiGameJoinLoad = (CScene_MultiGameJoinLoad *)pObj;

	MultiGameJoinLoad->UpdateGamesList(dwNumResults, pGameSearch);
#endif
}

int CScene_MultiGameJoinLoad::DeviceSelectReturned(void *pParam,bool bContinue)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
	//HRESULT hr;

	if(bContinue==true)
	{
		// if the saves list has focus, then we should show the Delete Save tooltip
		// if the games list has focus, then we should show the View Gamercard tooltip
		int iRB=-1;
		int iY = -1;
		if( pClass->DoesGamesListHaveFocus() )
		{
			iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
		}
		else if(pClass->DoesSavesListHaveFocus())
		{
			if(ProfileManager.IsSignedInLive( pClass->m_iPad ))
			{
				iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
			}
			if(StorageManager.GetSaveDisabled())
			{
				iRB=IDS_TOOLTIPS_DELETESAVE;
			}
			else
			{
				if(StorageManager.EnoughSpaceForAMinSaveGame())
				{
					iRB=IDS_TOOLTIPS_SAVEOPTIONS;
				}
				else
				{
					iRB=IDS_TOOLTIPS_DELETESAVE;
				}
			}
		}
		else if(pClass->DoesMashUpWorldHaveFocus())
		{
			// If it's a mash-up pack world, give the Hide option
			iRB=IDS_TOOLTIPS_HIDE;
		}

		int iLB = -1;
		if(pClass->m_bInParty)
		{
			if( pClass->m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
			else iLB = IDS_TOOLTIPS_PARTY_GAMES;
		}

		//BOOL bOnlineGame=pClass->m_CheckboxOnline.IsChecked();

		// refresh the saves list (if there is a device selected)

		// clear out the list first

		if(StorageManager.GetSaveDisabled())
		{
			if(StorageManager.GetSaveDeviceSelected(pClass->m_iPad))
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,iY,-1,-1,iLB,iRB);
				// saving is disabled, but we should still be able to load from a selected save device
				pClass->GetSaveInfo();
			}
			else
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB,iRB);
				// clear the saves list
				pClass->m_pSavesList->RemoveAllData();

				pClass->m_iSaveInfoC=0;
				//pClass->m_iThumbnailsLoadedC=0;

				pClass->AddDefaultButtons();

				pClass->m_SavesListTimer.SetShow( FALSE );

				pClass->m_pSavesList->SetCurSelVisible(0);
			}
		}
		else
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,iY,-1,-1,iLB,iRB);
			pClass->GetSaveInfo();
		}
	}

	// enable input again
	pClass->m_bIgnoreInput=false;

	return 0;
}

HRESULT CScene_MultiGameJoinLoad::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// 4J-PB - TODO - Don't think we can do this - if a 2nd player signs in here with an offline profile, the signed in LIVE player gets re-logged in, and bMultiplayerAllowed is false briefly
	switch(pTimer->nId)
	{


	case JOIN_LOAD_ONLINE_TIMER_ID:
	{
		XPARTY_USER_LIST partyList;

		if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
		{
			m_bInParty=true;
		}
		else
		{
			m_bInParty=false;
		}

		bool bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);
		if(bMultiplayerAllowed != m_bMultiplayerAllowed)
		{
			if( bMultiplayerAllowed )
			{
// 					m_CheckboxOnline.SetEnable(TRUE);
// 					m_CheckboxPrivate.SetEnable(TRUE);
			}
			else
			{
				m_bInParty = false;
				m_pGamesList->RemoveAllData();
				//m_GamesList.DeleteItems(0, m_GamesList.GetItemCount() );
				m_pGamesList->SetEnable(FALSE);
				//XuiElementSetDisableFocusRecursion( m_pGamesList->m_hObj, TRUE);
				m_NetGamesListTimer.SetShow( TRUE );
				m_LabelNoGames.SetShow( FALSE );
			}

			int iLB = -1;
			if(m_bInParty)
			{
				if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
				else iLB = IDS_TOOLTIPS_PARTY_GAMES;
			}
			int iRB=-1;
			int iY=-1;

			if( DoesGamesListHaveFocus() )
			{
			}
			else if(DoesSavesListHaveFocus())
			{
				if(ProfileManager.IsSignedInLive( m_iPad ))
				{
					iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
				}

				if(StorageManager.GetSaveDisabled())
				{
					iRB=IDS_TOOLTIPS_DELETESAVE;
				}
				else
				{
					if(StorageManager.EnoughSpaceForAMinSaveGame())
					{
						iRB=IDS_TOOLTIPS_SAVEOPTIONS;
					}
					else
					{
						iRB=IDS_TOOLTIPS_DELETESAVE;
					}
				}
			}
			else if(DoesMashUpWorldHaveFocus())
			{
				// If it's a mash-up pack world, give the Hide option
				iRB=IDS_TOOLTIPS_HIDE;
			}

			if(ProfileManager.IsFullVersion()==false )
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1,-1,-1,iLB);
			}
			else if(StorageManager.GetSaveDisabled())
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,-1,-1,-1,iLB,iRB);
			}
			else
			{
				ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGEDEVICE,iY,-1,-1,iLB,iRB);
			}
			m_bMultiplayerAllowed = bMultiplayerAllowed;
		}
	}
	break;
	case JOIN_LOAD_SEARCH_MINIMUM_TIMER_ID:
	{
		XuiKillTimer( m_hObj, JOIN_LOAD_SEARCH_MINIMUM_TIMER_ID );
		m_NetGamesListTimer.SetShow( FALSE );
		m_LabelNoGames.SetShow( TRUE );
	}
	break;
	case JOIN_LOAD_SCROLL_GAME_NAMES_TIMER_ID:
	{
		// This is called by the gameslist callback function, so isn't needed on a timer
		//UpdateGamesListLabels();
	}
	break;
		case CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID:
			{
				// also check for any new texture packs info being available
				// for each item in the mem list, check it's in the data list

				//CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
				// for each iConfig, check if the data is available, and add it to the List, then remove it from the viConfig

				for(int i=0;i<m_iTexturePacksNotInstalled;i++)
				{
					if(m_iConfigA[i]!=-1)
					{
						DWORD dwBytes=0;
						PBYTE pbData=NULL;
						//app.DebugPrintf("Retrieving iConfig %d from TPD\n",m_iConfigA[i]);

						app.GetTPD(m_iConfigA[i],&pbData,&dwBytes);

						if(dwBytes > 0 && pbData)
						{
							//update the games list
							UpdateGamesList();

							m_iConfigA[i]=-1;
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

				if(bAllDone)
				{
					// kill this timer
					XuiKillTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
				}
			}
			break;
			}

	return S_OK;
}

/*
int CScene_MultiGameJoinLoad::LoadSaveDataReturned(void *pParam,bool bContinue)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	if(bContinue==true)
	{
		bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad());

		// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
		DWORD connectedControllers = 0;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
		}

		if(!isClientSide || connectedControllers == 1 || !RenderManager.IsHiDef())
		{
			DWORD dwLocalUsersMask = CGameNetworkManager::GetLocalPlayerMask(ProfileManager.GetPrimaryPad());

			// No guest problems so we don't need to force a sign-in of players here
			StartGameFromSave(pClass, dwLocalUsersMask);
		}
		else
		{
			ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameJoinLoad::StartGame_SignInReturned, pParam,ProfileManager.GetPrimaryPad());
		}
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}
*/

int CScene_MultiGameJoinLoad::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	if(bContinue==true)
	{
		// It's possible that the player has not signed in - they can back out
		if(ProfileManager.IsSignedIn(iPad))
		{
			DWORD dwLocalUsersMask = 0;

			for(unsigned int index = 0; index < XUSER_MAX_COUNT; ++index)
			{
				if(ProfileManager.IsSignedIn(index) )
				{
					dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(index);
				}
			}
			StartGameFromSave(pClass, dwLocalUsersMask);
		}
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

// 4J Stu - Shared functionality that is the same whether we needed a quadrant sign-in or not
void CScene_MultiGameJoinLoad::StartGameFromSave(CScene_MultiGameJoinLoad* pClass, DWORD dwLocalUsersMask)
{
	/*bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_CheckboxOnline.IsChecked() == TRUE;
	//bool isPrivate = pClass->m_CheckboxPrivate.IsChecked() == TRUE;

	SenStatGameEvent(ProfileManager.GetPrimaryPad(),eTelemetryGameEvent_Load,Minecraft::GetInstance()->options->difficulty, isClientSide, ProfileManager.IsFullVersion(), 1,0 );

	g_NetworkManager.HostGame(dwLocalUsersMask,isClientSide,isPrivate,MINECRAFT_NET_MAX_PLAYERS,0);

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = NULL;

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);*/
}

int CScene_MultiGameJoinLoad::DeleteSaveDataReturned(void *pParam,bool bSuccess)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	if(bSuccess==true)
	{
		// need to re-get the saves list and update the display
		// clear the saves list
		pClass->m_pSavesList->RemoveAllData();
		pClass->m_iSaveInfoC=0;
		pClass->GetSaveInfo();
	}

	pClass->m_bIgnoreInput=false;

	return 0;
}

void CScene_MultiGameJoinLoad::LoadLevelGen(LevelGenerationOptions *levelGen)
{
	// Load data from disc
	//File saveFile( L"Tutorial\\Tutorial" );
	//LoadSaveFromDisk(&saveFile);

	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	StorageManager.ResetSaveData();
	// Make our next save default to the name of the level
	StorageManager.SetSaveTitle(levelGen->getDefaultSaveName().c_str());

	bool isClientSide = false;
	bool isPrivate = false;
	int maxPlayers = MINECRAFT_NET_MAX_PLAYERS;

	if( app.GetTutorialMode() )
	{
		isClientSide = false;
		maxPlayers = 4;
	}

	g_NetworkManager.HostGame(0,isClientSide,isPrivate,maxPlayers,0);

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = 0;
	param->saveData = NULL;
	param->settings = app.GetGameHostOption( eGameHostOption_Tutorial );
	param->levelGen = levelGen;

	if(levelGen->requiresTexturePack())
	{
		param->texturePackId = levelGen->getRequiredTexturePackId();

		Minecraft *pMinecraft = Minecraft::GetInstance();
		pMinecraft->skins->selectTexturePackById(param->texturePackId);
		//pMinecraft->skins->updateUI();
	}

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

void CScene_MultiGameJoinLoad::LoadSaveFromDisk(File *saveFile)
{
	// we'll only be coming in here when the tutorial is loaded now

	StorageManager.ResetSaveData();

	// Make our next save default to the name of the level
	StorageManager.SetSaveTitle(saveFile->getName().c_str());

	int64_t fileSize = saveFile->length();
	FileInputStream fis(*saveFile);
	byteArray ba(fileSize);
	fis.read(ba);
	fis.close();

	bool isClientSide = false;
	bool isPrivate = false;
	int maxPlayers = MINECRAFT_NET_MAX_PLAYERS;

	if( app.GetTutorialMode() )
	{
		isClientSide = false;
		maxPlayers = 4;
	}

	app.SetGameHostOption(eGameHostOption_GameType,GameType::CREATIVE->getId());

	g_NetworkManager.HostGame(0,isClientSide,isPrivate,maxPlayers,0);

	LoadSaveDataThreadParam *saveData = new LoadSaveDataThreadParam(ba.data, ba.length, saveFile->getName());

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = 0;
	param->saveData = saveData;
	param->settings = app.GetGameHostOption( eGameHostOption_All );

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

int CScene_MultiGameJoinLoad::DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultDecline)
	{
		if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
		{
			pClass->m_bIgnoreInput=false;
		}
		else
		{
			XCONTENT_DATA XContentData;
			StorageManager.GetSaveCacheFileInfo(pClass->m_iChangingSaveGameInfoIndex-pClass->m_iDefaultButtonsC,XContentData);
			StorageManager.DeleteSaveData(&XContentData,CScene_MultiGameJoinLoad::DeleteSaveDataReturned,pClass);
			pClass->m_SavesListTimer.SetShow( TRUE );
		}
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

int CScene_MultiGameJoinLoad::SaveTransferDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		// upload the save

		// first load the save
		int iIndex=pClass->m_pSavesList->GetData(pClass->m_pSavesList->GetCurSel()).iIndex-pClass->m_iDefaultButtonsC;
		XCONTENT_DATA ContentData;

		// 4J-PB - ensure we've switched to the right title group id for uploading to
		app.TMSPP_SetTitleGroupID(SAVETRANSFER_GROUP_ID);
		StorageManager.GetSaveCacheFileInfo(iIndex,ContentData);
		C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&ContentData,CScene_MultiGameJoinLoad::LoadSaveDataReturned,pClass);

		pClass->m_bIgnoreInput=false;
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

int CScene_MultiGameJoinLoad::UploadSaveForXboxOneThreadProc( LPVOID lpParameter )
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad *) lpParameter;
	Minecraft *pMinecraft = Minecraft::GetInstance();

	pMinecraft->progressRenderer->progressStart(IDS_SAVE_TRANSFER_TITLE);
	pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_UPLOADING );

	// Delete the marker file
	DeleteFile(pClass, "completemarker");
	if(!WaitForTransferComplete(pClass)) return 0;

	// Upload the save data
	{
		unsigned int uiSaveBytes;
		uiSaveBytes=StorageManager.GetSaveSize();
		pClass->m_pbSaveTransferData=new BYTE [uiSaveBytes];

		StorageManager.GetSaveData(pClass->m_pbSaveTransferData,&uiSaveBytes);

		app.DebugPrintf("Uploading save data (%d bytes)\n", uiSaveBytes);
		UploadFile(pClass, "savedata", pClass->m_pbSaveTransferData, uiSaveBytes);
	}

	if(!WaitForTransferComplete(pClass)) return 0;
	if(pClass->m_bTransferFail)
	{
		// something went wrong, user has been informed
		pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_UPLOADFAILED );
		return 0;
	}

	// Upload the metadata and thumbnail
	{
		ByteArrayOutputStream baos;
		DataOutputStream dos(&baos);

		LPCWSTR title = StorageManager.GetSaveTitle();
		dos.writeUTF(title);

		char szUniqueMapName[14];
		StorageManager.GetSaveUniqueFilename(szUniqueMapName);
		dos.writeUTF(convStringToWstring(szUniqueMapName));

		{
			// set the save icon
			PBYTE pbImageData=NULL;
			DWORD dwImageBytes=0;
			XCONTENT_DATA XContentData;
			int iIndex=pClass->m_pSavesList->GetData(pClass->m_pSavesList->GetCurSel()).iIndex-pClass->m_iDefaultButtonsC;
			StorageManager.GetSaveCacheFileInfo(iIndex,XContentData);
			StorageManager.GetSaveCacheFileInfo(iIndex,&pbImageData,&dwImageBytes);

			// if there is no thumbnail, retrieve the default one from the file.
			// Don't delete the image data after creating the xuibrush, since we'll use it in the rename of the save
			if(pbImageData==NULL)
			{
				DWORD dwResult=XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&XContentData,NULL,&dwImageBytes,NULL);
				if(dwResult==ERROR_SUCCESS)
				{
					pClass->m_pbSaveTransferData = new BYTE[dwImageBytes];
					pbImageData = pClass->m_pbSaveTransferData; // Copy pointer so that we can use the same name as the library owned one, but m_pbSaveTransferData will get deleted when done
					XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&XContentData,pbImageData,&dwImageBytes,NULL);
				}
			}

			dos.writeInt(dwImageBytes);

			byteArray ba(pbImageData, dwImageBytes);
			dos.write(ba);
		}

		pClass->m_pbSaveTransferData=new BYTE [baos.size()];
		memcpy(pClass->m_pbSaveTransferData,baos.buf.data,baos.size());

		app.DebugPrintf("Uploading meta data (%d bytes)\n", baos.size());
		UploadFile(pClass, "metadata", pClass->m_pbSaveTransferData, baos.size());
	}

	// Wait for metadata and thumbnail
	if(!WaitForTransferComplete(pClass)) return 0;
	if(pClass->m_bTransferFail)
	{
		// something went wrong, user has been informed
		pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_UPLOADFAILED );
		return 0;
	}

	// Upload the marker file
	{
		char singleByteData[1] = {1};
		app.DebugPrintf("Uploading marker (%d bytes)\n", 1);
		UploadFile(pClass, "completemarker", &singleByteData, 1);
	}

	// Wait for marker
	if(!WaitForTransferComplete(pClass)) return 0;
	if(pClass->m_bTransferFail)
	{
		// something went wrong, user has been informed
		pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_UPLOADFAILED );

		return 0;
	}
	// change text for completion confirmation
	pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_UPLOADCOMPLETE );

	// done
	return 0;
}

void CScene_MultiGameJoinLoad::DeleteFile(CScene_MultiGameJoinLoad *pClass, char *filename)
{
	pClass->m_fProgress=0.0f;
	pClass->m_bTransferComplete=false;

	C4JStorage::ETMSStatus result = StorageManager.TMSPP_DeleteFile(
		ProfileManager.GetPrimaryPad(),
		filename,
		C4JStorage::TMS_FILETYPE_BINARY,
		&CScene_MultiGameJoinLoad::DeleteComplete,
		pClass,
		NULL);

	if(result != C4JStorage::ETMSStatus_DeleteInProgress)
	{
		DeleteComplete(pClass,ProfileManager.GetPrimaryPad(), -1);
	}
}

void CScene_MultiGameJoinLoad::UploadFile(CScene_MultiGameJoinLoad *pClass, char *filename, LPVOID data, DWORD size)
{
	pClass->m_fProgress=0.0f;
	pClass->m_bTransferComplete=false;

	C4JStorage::ETMSStatus result = StorageManager.TMSPP_WriteFileWithProgress(
			ProfileManager.GetPrimaryPad(),
			C4JStorage::eGlobalStorage_TitleUser,
			C4JStorage::TMS_FILETYPE_BINARY,
			C4JStorage::TMS_UGCTYPE_NONE,
			filename,
			(CHAR *)data,
			size,
			&CScene_MultiGameJoinLoad::TransferComplete,pClass, 0,
			&CScene_MultiGameJoinLoad::Progress,pClass);

#ifdef _DEBUG_MENUS_ENABLED
	if(app.GetWriteSavesToFolderEnabled())
	{
		File targetFileDir(L"GAME:\\FakeTMSPP");
		if(!targetFileDir.exists()) targetFileDir.mkdir();
		string path = string( wstringtofilename( targetFileDir.getPath() ) ).append("\\").append(filename);
		HANDLE hSaveFile = CreateFile( path.c_str(), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS, NULL);

		DWORD numberOfBytesWritten = 0;
		WriteFile( hSaveFile,data,size,&numberOfBytesWritten,NULL);
		assert(numberOfBytesWritten == size);

		CloseHandle(hSaveFile);
	}
#endif

	if(result != C4JStorage::ETMSStatus_WriteInProgress)
	{
		TransferComplete(pClass,ProfileManager.GetPrimaryPad(), -1);
	}
}

bool CScene_MultiGameJoinLoad::WaitForTransferComplete( CScene_MultiGameJoinLoad *pClass )
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	// loop until complete
	while(pClass->m_bTransferComplete==false)
	{
		// check for a cancel
		if(pClass->m_bSaveTransferInProgress==false)
		{
			// cancelled
			return false;
		}
		Sleep(50);
		// update the progress
		pMinecraft->progressRenderer->progressStagePercentage((unsigned int)(pClass->m_fProgress*100.0f));
	}

	// was there a transfer error?

	return true;
}

int CScene_MultiGameJoinLoad::SaveOptionsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	// results switched for this dialog
	// EMessage_ResultAccept means cancel
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultThirdOption)
	{
		if(result==C4JStorage::EMessage_ResultDecline) // rename
		{
			ZeroMemory(pClass->m_wchNewName,sizeof(WCHAR)*XCONTENT_MAX_DISPLAYNAME_LENGTH);
			// bring up a keyboard
			InputManager.RequestKeyboard(IDS_RENAME_WORLD_TITLE,L"",IDS_RENAME_WORLD_TEXT,iPad,pClass->m_wchNewName,XCONTENT_MAX_DISPLAYNAME_LENGTH,&CScene_MultiGameJoinLoad::KeyboardReturned,pClass,C_4JInput::EKeyboardMode_Default,app.GetStringTable());
		}
		else // delete
		{
			// delete the save game
			// Have to ask the player if they are sure they want to delete this game
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			StorageManager.RequestMessageBox(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, iPad,&CScene_MultiGameJoinLoad::DeleteSaveDialogReturned,pClass, app.GetStringTable());
			//pClass->m_bIgnoreInput=false;
		}
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

int CScene_MultiGameJoinLoad::LoadSaveDataReturned(void *pParam,bool bContinue)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	if(bContinue==true)
	{
		pClass->m_bSaveTransferInProgress=true;
		LoadingInputParams *loadingParams = new LoadingInputParams();
		loadingParams->func = &CScene_MultiGameJoinLoad::UploadSaveForXboxOneThreadProc;
		loadingParams->lpParam = (LPVOID)pParam;

		UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
		completionData->bShowBackground=TRUE;
		completionData->bShowLogo=TRUE;
		completionData->type = e_ProgressCompletion_NavigateBack;
		completionData->iPad = DEFAULT_XUI_MENU_USER;
		completionData->bRequiresUserAction=TRUE;
		loadingParams->completionData = completionData;

		loadingParams->cancelFunc=&CScene_MultiGameJoinLoad::CancelSaveUploadCallback;
		loadingParams->completeFunc=&CScene_MultiGameJoinLoad::SaveUploadCompleteCallback;
		loadingParams->m_cancelFuncParam=pClass;
		loadingParams->m_completeFuncParam=pClass;
		loadingParams->cancelText=IDS_TOOLTIPS_CANCEL;

		app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
	}
	else
	{
		// switch back to the normal title group id
		app.TMSPP_SetTitleGroupID(GROUP_ID);

		// the save is corrupt!

		pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;

		// give the option to delete the save
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_CANCEL;
		uiIDA[1]=IDS_CONFIRM_OK;
		StorageManager.RequestMessageBox(IDS_CORRUPT_OR_DAMAGED_SAVE_TITLE, IDS_CORRUPT_OR_DAMAGED_SAVE_TEXT, uiIDA, 2,
			pClass->m_iPad,&CScene_MultiGameJoinLoad::DeleteSaveDialogReturned,pClass, app.GetStringTable());

	}

	return 0;
}

int CScene_MultiGameJoinLoad::Progress(void *pParam,float fProgress)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	app.DebugPrintf("Progress - %f\n",fProgress);
	pClass->m_fProgress=fProgress;
	return 0;
}

int CScene_MultiGameJoinLoad::TransferComplete(void *pParam,int iPad, int iResult)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;

	delete [] pClass->m_pbSaveTransferData;
	pClass->m_pbSaveTransferData = NULL;
	if(iResult!=0)
	{
		// There was a transfer fail
		// Display a dialog
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		StorageManager.RequestMessageBox(IDS_SAVE_TRANSFER_TITLE, IDS_SAVE_TRANSFER_UPLOADFAILED, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,NULL,app.GetStringTable());
		pClass->m_bTransferFail=true;
	}
	else
	{
		pClass->m_bTransferFail=false;
	}
	pClass->m_bTransferComplete=true;
	//pClass->m_bSaveTransferInProgress=false;
	return 0;
}

int CScene_MultiGameJoinLoad::DeleteComplete(void *pParam,int iPad, int iResult)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
	pClass->m_bTransferComplete=true;
	return 0;
}

int CScene_MultiGameJoinLoad::KeyboardReturned(void *pParam,bool bSet)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
	HRESULT hr = S_OK;

	// if the user has left the name empty, treat this as backing out
	if((pClass->m_wchNewName[0]!=0) && bSet)
	{
#ifdef _XBOX
		XCONTENT_DATA XContentData;
		StorageManager.GetSaveCacheFileInfo(pClass->m_iChangingSaveGameInfoIndex-pClass->m_iDefaultButtonsC,XContentData);

		C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&XContentData,CScene_MultiGameJoinLoad::LoadSaveDataForRenameReturned,pClass);

		if(eLoadStatus==C4JStorage::ELoadGame_DeviceRemoved)
		{
			// disable saving
			StorageManager.SetSaveDisabled(true);
			StorageManager.SetSaveDeviceSelected(ProfileManager.GetPrimaryPad(),false);
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			StorageManager.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(),&CScene_MultiGameJoinLoad::DeviceRemovedDialogReturned,pClass);
		}
#else
		// rename the save

#endif
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}

	return hr;
}

int CScene_MultiGameJoinLoad::LoadSaveDataForRenameReturned(void *pParam,bool bContinue)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
#ifdef _XBOX
	if(bContinue==true)
	{
		// set the save icon
		PBYTE pbImageData=NULL;
		DWORD dwImageBytes=0;
		HXUIBRUSH hXuiBrush;
		XCONTENT_DATA XContentData;
		StorageManager.GetSaveCacheFileInfo(pClass->m_iChangingSaveGameInfoIndex-pClass->m_iDefaultButtonsC,XContentData);
		StorageManager.GetSaveCacheFileInfo(pClass->m_iChangingSaveGameInfoIndex-pClass->m_iDefaultButtonsC,&pbImageData,&dwImageBytes);

		// if there is no thumbnail, retrieve the default one from the file.
		// Don't delete the image data after creating the xuibrush, since we'll use it in the rename of the save
		if(pbImageData==NULL)
		{
			DWORD dwResult=XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&XContentData,NULL,&dwImageBytes,NULL);
			if(dwResult==ERROR_SUCCESS)
			{
				pbImageData = new BYTE[dwImageBytes];
				XContentGetThumbnail(ProfileManager.GetPrimaryPad(),&XContentData,pbImageData,&dwImageBytes,NULL);
				XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&hXuiBrush);
			}
		}
		else
		{
			XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&hXuiBrush);
		}
		// save the data with this icon
		StorageManager.CopySaveDataToNewSave( pbImageData,dwImageBytes,pClass->m_wchNewName,&CScene_MultiGameJoinLoad::CopySaveReturned,pClass);
	}
	else
#endif
	{
		//pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}

int CScene_MultiGameJoinLoad::CopySaveReturned(void *pParam,bool bResult)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad*)pParam;
#ifdef _XBOX
	if(bResult)
	{
		// and delete the old save
		XCONTENT_DATA XContentData;
		StorageManager.GetSaveCacheFileInfo(pClass->m_iChangingSaveGameInfoIndex-pClass->m_iDefaultButtonsC,XContentData);
		StorageManager.DeleteSaveData(&XContentData,CScene_MultiGameJoinLoad::DeleteSaveDataReturned,pClass);
		pClass->m_SavesListTimer.SetShow( TRUE );
	}
	else
#endif
	{
		//pClass->SetShow( TRUE );
		pClass->m_bIgnoreInput=false;
	}

	return 0;
}

int CScene_MultiGameJoinLoad::TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameJoinLoad *pClass = (CScene_MultiGameJoinLoad *)pParam;

	// Exit with or without saving
	// Decline means install full version of the texture pack in this dialog
	if(result==C4JStorage::EMessage_ResultDecline || result==C4JStorage::EMessage_ResultAccept)
	{
		// we need to enable background downloading for the DLC
		XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

		ULONGLONG ullOfferID_Full;
		ULONGLONG ullIndexA[1];
		app.GetDLCFullOfferIDForPackID(pClass->m_initData->selectedSession->data.texturePackParentId,&ullOfferID_Full);

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
	pClass->m_bIgnoreInput=false;
	return 0;
}

HRESULT CScene_MultiGameJoinLoad::OnCustomMessage_DLCInstalled()
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
		// clear out the saves list and re-fill

		m_pSavesList->RemoveAllData();
		m_SavesListTimer.SetShow( TRUE );
	}
	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}

HRESULT CScene_MultiGameJoinLoad::OnCustomMessage_DLCMountingComplete()
{

	VOID *pObj;
	XuiObjectFromHandle( m_SavesList, &pObj );
	m_pSavesList = (CXuiCtrl4JList *)pObj;

	m_iChangingSaveGameInfoIndex = 0;

	m_generators = app.getLevelGenerators();
	m_iDefaultButtonsC = 0;
	m_iMashUpButtonsC = 0;
	XPARTY_USER_LIST partyList;

	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		m_bInParty=true;
	}
	else
	{
		m_bInParty=false;
	}

	int iLB = -1;

	int iY=-1;
	if(DoesSavesListHaveFocus())
	{
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			iY=IDS_TOOLTIPS_UPLOAD_SAVE_FOR_XBOXONE;
		}
	}
	if(m_bInParty) iLB = IDS_TOOLTIPS_PARTY_GAMES;
	// check if we're in the trial version
	if(ProfileManager.IsFullVersion()==false)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1, -1, -1,iLB);

		AddDefaultButtons();

		m_pSavesList->SetCurSelVisible(0);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		if(StorageManager.GetSaveDeviceSelected(m_iPad))
		{
			// saving is disabled, but we should still be able to load from a selected save device

			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,iY,-1,-1,iLB,IDS_TOOLTIPS_DELETESAVE);

			GetSaveInfo();
		}
		else
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE,iY,-1,-1,iLB);

			AddDefaultButtons();
			m_SavesListTimer.SetShow( FALSE );

			m_pSavesList->SetCurSelVisible(0);
		}
	}
	else
	{
		// 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)
		bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE,iY,-1,-1,-1,bCanRename?IDS_TOOLTIPS_SAVEOPTIONS:IDS_TOOLTIPS_DELETESAVE);

		GetSaveInfo();
	}

	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	return S_OK;
}

/*
void CScene_MultiGameJoinLoad::UpdateTooltips()
{
	int iA=IDS_TOOLTIPS_SELECT;
	int iB=IDS_TOOLTIPS_BACK;
	int iX=-1;
	int iY=-1
	int iLB = -1;
	XPARTY_USER_LIST partyList;

	if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
	{
		m_bInParty=true;
	}
	else
	{
		m_bInParty=false;
	}

	if(m_bInParty) iLB = IDS_TOOLTIPS_PARTY_GAMES;

	if(ProfileManager.IsFullVersion()==false)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK, -1, -1, -1, -1,iLB);
	}
	else if(StorageManager.GetSaveDisabled())
	{
		if(StorageManager.GetSaveDeviceSelected(m_iPad))
		{
			// saving is disabled, but we should still be able to load from a selected save device
			iX=IDS_TOOLTIPS_CHANGEDEVICE;
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
		else
		{
			iX=IDS_TOOLTIPS_SELECTDEVICE;
		}
	}
	else
	{
		// 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)
		bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();

		if(bCanRename)
		{
			iRB=IDS_TOOLTIPS_SAVEOPTIONS;
		}
		else
		{
			iRB=IDS_TOOLTIPS_DELETESAVE;
		}
	}

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, iA,iB, iX, iY, iLT, iRT,iLB, iRB);
}
*/



#ifdef _XBOX
bool CScene_MultiGameJoinLoad::GetSavesInfoCallback(LPVOID pParam,int iTotalSaveInfoC, C4JStorage::CACHEINFOSTRUCT *InfoA, int iPad, HRESULT hResult)
{
	CScene_MultiGameJoinLoad *pClass=(CScene_MultiGameJoinLoad *)pParam;
	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;
	PBYTE pbImageData=(PBYTE)InfoA;
	PBYTE pbCurrentImagePtr;
	HXUIBRUSH hXuiBrush;
	HRESULT hr;

	// move the image data pointer to the right place
	if(iTotalSaveInfoC!=0)
	{
		pbImageData+=sizeof(C4JStorage::CACHEINFOSTRUCT)*iTotalSaveInfoC;
	}

	pClass->m_SavesListTimer.SetShow( FALSE );
	pClass->m_SavesList.SetEnable(TRUE);

	pClass->AddDefaultButtons();

	for(int i=0;i<iTotalSaveInfoC;i++)
	{
		ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));
		// Add these to the save list
		if(!(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled()))
		{
			// if the save is corrupt, display this instead of the title
			if(InfoA[i].dwImageBytes==0)
			{
				ListInfo.pwszText=app.GetString(IDS_CORRUPT_OR_DAMAGED_SAVE_TITLE);
				ListInfo.bIsDamaged=true;
			}
			else
			{
				ListInfo.pwszText=InfoA[i].wchDisplayName;
				ListInfo.bIsDamaged=false;
			}
			ListInfo.fEnabled=TRUE;
			ListInfo.iData = -1;

			pClass->m_pSavesList->AddData(ListInfo,-1);

			// update the graphic on the list item

			// if there is no thumbnail, this is a corrupt file
			if(InfoA[i].dwImageBytes!=0)
			{
				pbCurrentImagePtr=pbImageData+InfoA[i].dwImageOffset;
				hr=XuiCreateTextureBrushFromMemory(pbCurrentImagePtr,InfoA[i].dwImageBytes,&hXuiBrush);
				pClass->m_pSavesList->UpdateGraphic(i+pClass->m_iDefaultButtonsC,hXuiBrush );
			}
			else
			{
				// we could put in a damaged save icon here
				const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string
				WCHAR szResourceLocator[ LOCATOR_SIZE ];
				const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

				swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/Graphics/MinecraftBrokenIcon.png");

				XuiCreateTextureBrush(szResourceLocator,&hXuiBrush);
				pClass->m_pSavesList->UpdateGraphic(i+pClass->m_iDefaultButtonsC,hXuiBrush );
			}
		}
	}

	pClass->m_iSaveInfoC=iTotalSaveInfoC;

	// If there are some saves, then set the focus to be on the most recent one, which will be the first one after the create and tutorial
	if(iTotalSaveInfoC>0)
	{
		pClass->m_pSavesList->SetCurSelVisible(pClass->m_iDefaultButtonsC);
		pClass->m_bReady=true;
	}

	pClass->m_bRetrievingSaveInfo=false;

	// It's possible that the games list is updated but we haven't displayed it yet as we were still waiting on saves list to load
	// This is to fix a bug where joining a game before the saves list has loaded causes a crash when this callback is called
	// as the scene no longer exists
	pClass->UpdateGamesList();

	// Fix for #45154 - Frontend: DLC: Content can only be downloaded from the frontend if you have not joined/exited multiplayer
	XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);

	return false;
}
#else
int CScene_MultiGameJoinLoad::GetSavesInfoCallback(LPVOID lpParam,const bool)
{
	return true;
}
#endif

void CScene_MultiGameJoinLoad::CancelSaveUploadCallback(LPVOID lpParam)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad *) lpParam;

	StorageManager.TMSPP_CancelWriteFileWithProgress(pClass->m_iPad);

	pClass->m_bSaveTransferInProgress=false;

	// change back to the normal title group id
	app.TMSPP_SetTitleGroupID(GROUP_ID);
// 	app.getRemoteStorage()->abort();
// 	pClass->m_eSaveUploadState = eSaveUpload_Idle;

	UINT uiIDA[1] = { IDS_CONFIRM_OK };
	ui.RequestMessageBox(IDS_XBONE_CANCEL_UPLOAD_TITLE, IDS_XBONE_CANCEL_UPLOAD_TEXT, uiIDA, 1, pClass->m_iPad, NULL, NULL, app.GetStringTable());
}

void CScene_MultiGameJoinLoad::SaveUploadCompleteCallback(LPVOID lpParam)
{
	CScene_MultiGameJoinLoad* pClass = (CScene_MultiGameJoinLoad *) lpParam;

	pClass->m_bSaveTransferInProgress=false;
	// change back to the normal title group id
	app.TMSPP_SetTitleGroupID(GROUP_ID);
	// 	app.getRemoteStorage()->abort();
	// 	pClass->m_eSaveUploadState = eSaveUpload_Idle;
}