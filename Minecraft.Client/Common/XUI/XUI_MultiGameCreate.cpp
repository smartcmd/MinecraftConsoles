#include "stdafx.h"
#include "..\..\..\Minecraft.World\Random.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "XUI_MultiGameCreate.h"
#include "XUI_Controls.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\..\Minecraft.World\LevelSettings.h"
#include "XUI_MultiGameLaunchMoreOptions.h"
#include "..\..\..\Minecraft.World\BiomeSource.h"
#include "..\..\..\Minecraft.World\IntCache.h"
#include "..\..\..\Minecraft.World\LevelType.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\DLC\DLCLocalisationFile.h"
#include "..\..\StringTable.h"
#include "..\..\DLCTexturePack.h"

#define GAME_CREATE_ONLINE_TIMER_ID 0
#define GAME_CREATE_ONLINE_TIMER_TIME 100
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID 1
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME 100

int CScene_MultiGameCreate::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameCreate::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_bSetup = false;
	m_texturePackDescDisplayed = false;
	m_iConfigA=nullptr;

	WCHAR TempString[256];
	MapChildControls();

	XuiControlSetText(m_EditWorldName,app.GetString(IDS_DEFAULT_WORLD_NAME));
	XuiControlSetText(m_MoreOptions,app.GetString(IDS_MORE_OPTIONS));
	XuiControlSetText(m_NewWorld,app.GetString(IDS_CREATE_NEW_WORLD));

	XuiControlSetText(m_labelWorldName,app.GetString(IDS_WORLD_NAME));
	XuiControlSetText(m_labelSeed,app.GetString(IDS_CREATE_NEW_WORLD_SEED));
	XuiControlSetText(m_labelRandomSeed,app.GetString(IDS_CREATE_NEW_WORLD_RANDOM_SEED));
	XuiControlSetText(m_pTexturePacksList->m_hObj,app.GetString(IDS_DLC_MENU_TEXTUREPACKS));

	CreateWorldMenuInitData *params = static_cast<CreateWorldMenuInitData *>(pInitData->pvInitData);

	m_MoreOptionsParams.bGenerateOptions=TRUE;
	m_MoreOptionsParams.bStructures=TRUE;
	m_MoreOptionsParams.bFlatWorld=FALSE;
	m_MoreOptionsParams.bBonusChest=FALSE;
	m_MoreOptionsParams.bPVP = TRUE;
	m_MoreOptionsParams.bTrust = TRUE;
	m_MoreOptionsParams.bFireSpreads = TRUE;
	m_MoreOptionsParams.bHostPrivileges = FALSE;
	m_MoreOptionsParams.bTNT = TRUE;
	m_MoreOptionsParams.iPad = params->iPad;
	m_iPad=params->iPad;
	delete params;

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

	m_ButtonGameMode.SetText(app.GetString(IDS_GAMEMODE_SURVIVAL));
	m_bGameModeSurvival=true;

	m_CurrentDifficulty=app.GetGameSettings(m_iPad,eGameSetting_Difficulty);
	m_SliderDifficulty.SetValue(m_CurrentDifficulty);
	swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[m_CurrentDifficulty]));
	m_SliderDifficulty.SetText(TempString);

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );

	// restrict the keyboard - don't want languages that are not supported, like cyrillic, etc.
	switch(XGetLanguage())
	{
	case XC_LANGUAGE_ENGLISH:
	case XC_LANGUAGE_GERMAN:
	case XC_LANGUAGE_FRENCH:
	case XC_LANGUAGE_SPANISH:
	case XC_LANGUAGE_ITALIAN:
	case XC_LANGUAGE_PORTUGUESE:
	case XC_LANGUAGE_JAPANESE:
	case XC_LANGUAGE_TCHINESE:
	case XC_LANGUAGE_KOREAN:
		m_EditWorldName.SetKeyboardType(C_4JInput::EKeyboardMode_Default);
		m_EditWorldName.SetKeyboardType(C_4JInput::EKeyboardMode_Default);
		break;
	default:
		m_EditWorldName.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
		m_EditWorldName.SetKeyboardType(C_4JInput::EKeyboardMode_Full);
		break;
	}

	m_NewWorld.SetEnable(true);

	m_EditWorldName.SetTextLimit(XCONTENT_MAX_DISPLAYNAME_LENGTH);

	wstring wWorldName = m_EditWorldName.GetText();

	// set the caret to the end of the default text
	m_EditWorldName.SetCaretPosition(static_cast<int>(wWorldName.length()));
	// In the dashboard, there's room for about 30 W characters on two lines before they go over the top of things
	m_EditWorldName.SetTextLimit(25);

	m_EditWorldName.SetTitleAndText(IDS_NAME_WORLD,IDS_NAME_WORLD_TEXT);
	m_EditSeed.SetTitleAndText(IDS_CREATE_NEW_WORLD,IDS_CREATE_NEW_WORLD_SEEDTEXT);

	XuiSetTimer(m_hObj,GAME_CREATE_ONLINE_TIMER_ID,GAME_CREATE_ONLINE_TIMER_TIME);
	XuiSetTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_CreateWorldMenu, 0);

	// 4J-PB - Load up any texture pack data we have locally in the XZP
	for(int i=0;i<TMS_COUNT;i++)
	{
		if(app.TMSFileA[i].eTMSType==eTMSFileType_TexturePack)
		{
			app.LoadLocalTMSFile(app.TMSFileA[i].wchFilename,app.TMSFileA[i].eEXT);
			app.AddMemoryTPDFile(app.TMSFileA[i].iConfig, app.TMSFileA[i].pbData,app.TMSFileA[i].uiSize);
		}
	}

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
				ListInfo.fEnabled = true;
				DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tp);
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


		// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this

		DLC_INFO *pDLCInfo=nullptr;

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

		m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
		UpdateTexturePackDescription(m_currentTexturePackIndex);

		m_bSetup = true;
	}
	return S_OK;
}


HRESULT CScene_MultiGameCreate::OnDestroy()
{
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

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameCreate::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_NewWorld)
	{
		// Check if we need to upsell the texture pack
		if(m_MoreOptionsParams.dwTexturePack!=0)
		{
			// texture pack hasn't been set yet, so check what it will be
			TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);

			if(pTexturePack==nullptr)
			{
				// They've selected a texture pack they don't have yet
				// upsell
				CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
				// get the current index of the list, and then get the data
				ListItem=m_pTexturePacksList->GetData(m_currentTexturePackIndex);


				// upsell the texture pack
				// tell sentient about the upsell of the full version of the skin pack
				ULONGLONG ullOfferID_Full;
				app.GetDLCFullOfferIDForPackID(ListItem.iData,&ullOfferID_Full);

				// DLC might have been corrupt
				if(ullOfferID_Full!=0LL)
				{
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
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&CScene_MultiGameCreate::TexturePackDialogReturned,this,app.GetStringTable());
					}
					else
					{
						uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
						uiIDA[1]=IDS_CONFIRM_CANCEL;
						// Give the player a warning about the texture pack missing
						StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CScene_MultiGameCreate::TexturePackDialogReturned,this,app.GetStringTable());
					}

					return S_OK;
				}
			}
		}

		m_bIgnoreInput = true;
		SetShow( FALSE );
		bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && m_MoreOptionsParams.bOnlineGame;

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

		// Check that we have the rights to use a texture pack we have selected.
		if(m_MoreOptionsParams.dwTexturePack!=0)
		{
			// texture pack hasn't been set yet, so check what it will be
			TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);

			if(pTexturePack== nullptr)
			{
				// corrupt DLC so set it to the default textures
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

						if(pDLCInfo!=nullptr)
						{
							ullOfferID_Full=pDLCInfo->ullOfferID_Full;
						}
						else
						{
							ullOfferID_Full=pTexturePack->getDLCPack()->getPurchaseOfferId();
						}

						// tell sentient about the upsell of the full version of the skin pack
						TelemetryManager->RecordUpsellPresented(pNotifyPressData->UserIndex, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);

						UINT uiIDA[1];
						uiIDA[0]=IDS_CONFIRM_OK;

						// Give the player a warning about the trial version of the texture pack
						StorageManager.RequestMessageBox(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 1, pNotifyPressData->UserIndex,&CScene_MultiGameCreate::WarningTrialTexturePackReturned,this,app.GetStringTable());
						return S_OK;
					}
				}
			}
		}

		if(m_bGameModeSurvival != true || m_MoreOptionsParams.bHostPrivileges == TRUE)
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			if(m_bGameModeSurvival != true)
			{
				StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_CREATIVE, uiIDA, 2, m_iPad,&CScene_MultiGameCreate::ConfirmCreateReturned,this,app.GetStringTable());
			}
			else
			{
				StorageManager.RequestMessageBox(IDS_TITLE_START_GAME, IDS_CONFIRM_START_HOST_PRIVILEGES, uiIDA, 2, m_iPad,&CScene_MultiGameCreate::ConfirmCreateReturned,this,app.GetStringTable());
			}
		}
		else
		{
			// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
			DWORD connectedControllers = 0;
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
			}

			if(isClientSide && connectedControllers > 1 && RenderManager.IsHiDef())
			{
				ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameCreate::StartGame_SignInReturned, this,ProfileManager.GetPrimaryPad());
			}
			else
			{
				// Check if user-created content is allowed, as we cannot play multiplayer if it's not
				//bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && m_MoreOptionsParams.bOnlineGame;
				bool noUGC = false;
				BOOL pccAllowed = TRUE;
				BOOL pccFriendsAllowed = TRUE;
				ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
				if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

				if(isClientSide && noUGC )
				{
					m_bIgnoreInput = false;
					SetShow( TRUE );
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
				}
				else
				{
					CreateGame(this, 0);
				}
			}
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


int CScene_MultiGameCreate::UnlockTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameCreate* pScene = static_cast<CScene_MultiGameCreate *>(pParam);
#ifdef _XBOX
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{
			ULONGLONG ullIndexA[1];
			DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(pScene->m_pDLCPack->getPurchaseOfferId());

			if(pDLCInfo!=nullptr)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Full;
			}
			else
			{
				ullIndexA[0]=pScene->m_pDLCPack->getPurchaseOfferId();
			}

			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);

			// the license change coming in when the offer has been installed will cause this scene to refresh
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Texture_DLC, ( pScene->m_pDLCPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}
#endif
	pScene->m_bIgnoreInput = false;
	pScene->SetShow( TRUE );

	return 0;
}

int CScene_MultiGameCreate::WarningTrialTexturePackReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameCreate* pScene = static_cast<CScene_MultiGameCreate *>(pParam);
	pScene->m_bIgnoreInput = false;
	pScene->SetShow( TRUE );
	bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pScene->m_MoreOptionsParams.bOnlineGame;


	// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
	DWORD connectedControllers = 0;
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
	}

	if(isClientSide && connectedControllers > 1 && RenderManager.IsHiDef())
	{
		ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameCreate::StartGame_SignInReturned, pScene,ProfileManager.GetPrimaryPad());
	}
	else
	{
		// Check if user-created content is allowed, as we cannot play multiplayer if it's not
		bool noUGC = false;
		BOOL pccAllowed = TRUE;
		BOOL pccFriendsAllowed = TRUE;
		ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
		if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

		if(isClientSide && noUGC )
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
		}
		else
		{
			// This is called from a storage manager thread... need to set up thread storage for IntCache as CreateGame requires this to search for a suitable seed if we haven't set a seed.
			IntCache::CreateNewThreadStorage();
			CreateGame(pScene, 0);
			IntCache::ReleaseThreadStorage();
		}
	}

	return 0;
}

HRESULT CScene_MultiGameCreate::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}
	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled)
{
	WCHAR TempString[256];

	if(hObjSource == m_EditWorldName)
	{
		//  Enable the done button when we have all of the necessary information
		wstring wWorldName = m_EditWorldName.GetText();
		BOOL bHasWorldName = ( wWorldName.length()!=0);
		m_NewWorld.SetEnable(bHasWorldName);
	}
	else if(hObjSource==m_SliderDifficulty.GetSlider() )
	{
		app.SetGameSettings(m_iPad,eGameSetting_Difficulty,pValueChangedData->nValue);
		swprintf( static_cast<WCHAR *>(TempString), 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[pValueChangedData->nValue]));
		m_SliderDifficulty.SetText(TempString);
	}

	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);

	if(pControlNavigateData->hObjDest==nullptr)
	{
		pControlNavigateData->hObjDest=pControlNavigateData->hObjSource;
	}

	bHandled=TRUE;
	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
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
					PBYTE pbData=nullptr;
					//app.DebugPrintf("Retrieving iConfig %d from TPD\n",m_iConfigA[i]);

					app.GetTPD(m_iConfigA[i],&pbData,&dwBytes);

					ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));
					if(dwBytes > 0 && pbData)
					{
						DWORD dwImageBytes=0;
						PBYTE pbImageData=nullptr;

						app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbImageData,&dwImageBytes );
						ListInfo.fEnabled = TRUE;
						ListInfo.iData = m_iConfigA[i];
						HRESULT hr=XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&ListInfo.hXuiBrush);
						app.DebugPrintf("Adding texturepack  %d from TPD\n",m_iConfigA[i]);

						ListInfo.iSortIndex=m_iConfigA[i];
						m_pTexturePacksList->AddData(ListInfo,0,CXuiCtrl4JList::eSortList_Index);

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

int CScene_MultiGameCreate::ConfirmCreateReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameCreate* pClass = static_cast<CScene_MultiGameCreate *>(pParam);

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;

		// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
		DWORD connectedControllers = 0;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
		}

		if(isClientSide && connectedControllers > 1 && RenderManager.IsHiDef())
		{
			ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameCreate::StartGame_SignInReturned, pClass,ProfileManager.GetPrimaryPad());
		}
		else
		{
			// Check if user-created content is allowed, as we cannot play multiplayer if it's not
			bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
			bool noUGC = false;
			BOOL pccAllowed = TRUE;
			BOOL pccFriendsAllowed = TRUE;
			ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

			if(isClientSide && noUGC )
			{
				pClass->m_bIgnoreInput = false;
				pClass->SetShow( TRUE );
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
			}
			else
			{
				// This is called from a storage manager thread... need to set up thread storage for IntCache as CreateGame requires this to search for a suitable seed if we haven't set a seed.
				IntCache::CreateNewThreadStorage();
				CreateGame(pClass, 0);
				IntCache::ReleaseThreadStorage();
			}
		}
	}
	else
	{
		pClass->m_bIgnoreInput = false;
		pClass->SetShow( TRUE );
	}
	return 0;
}

int CScene_MultiGameCreate::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_MultiGameCreate* pClass = static_cast<CScene_MultiGameCreate *>(pParam);

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
			ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

			if(isClientSide && (noPrivileges || noUGC) )
			{
				if( noUGC )
				{
					pClass->m_bIgnoreInput = false;
					pClass->SetShow( TRUE );
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
				}
				else
				{
					pClass->m_bIgnoreInput = false;
					pClass->SetShow( TRUE );
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					StorageManager.RequestMessageBox( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_HOST_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad(),nullptr,nullptr, app.GetStringTable());
				}
			}
			else
			{
				// This is NOT called from a storage manager thread, and is in fact called from the main thread in the Profile library tick. Therefore we use the main threads IntCache.
				CreateGame(pClass, dwLocalUsersMask);
			}
		}
	}
	else
	{
		pClass->m_bIgnoreInput = false;
		pClass->SetShow( TRUE );
	}
	return 0;
}

// 4J Stu - Shared functionality that is the same whether we needed a quadrant sign-in or not
void CScene_MultiGameCreate::CreateGame(CScene_MultiGameCreate* pClass, DWORD dwLocalUsersMask)
{
	// stop the timer running that causes a check for new texture packs in TMS but not installed, since this will run all through the create game, and will crash if it tries to create an hbrush
	XuiKillTimer(pClass->m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);

	bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
	bool isPrivate = pClass->m_MoreOptionsParams.bInviteOnly?true:false;

	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	// create the world and launch
	wstring wWorldName = pClass->m_EditWorldName.GetText();

	StorageManager.ResetSaveData();
	// Make our next save default to the name of the level
	StorageManager.SetSaveTitle((wchar_t *)wWorldName.c_str());

	BOOL bHasSeed = (pClass->m_EditSeed.GetText() != nullptr);

	wstring wSeed;
	if(bHasSeed)
	{
		wSeed=pClass->m_EditSeed.GetText();
	}
	else
	{
		// random
		wSeed=L"";
	}

	// start the game
	bool isFlat = (pClass->m_MoreOptionsParams.bFlatWorld==TRUE);
	int64_t seedValue = 0; //BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);	// 4J - was (new Random())->nextLong() - now trying to actually find a seed to suit our requirements

	if (wSeed.length() != 0)
	{
		int64_t value = 0;
		const unsigned int len = static_cast<unsigned int>(wSeed.length());

		//Check if the input string contains a numerical value
		bool isNumber = true;
		for( unsigned int i = 0 ; i < len ; ++i )
			if( wSeed.at(i) < L'0' || wSeed.at(i) > L'9' )
				if( !(i==0 && wSeed.at(i) == L'-' ) )
				{
					isNumber = false;
					break;
				}

		//If the input string is a numerical value, convert it to a number
		if( isNumber )
			value = _fromString<int64_t>(wSeed);

		//If the value is not 0 use it, otherwise use the algorithm from the java String.hashCode() function to hash it
		if( value != 0 )
			seedValue = value;
		else
		{
			int hashValue = 0;
			for( unsigned int i = 0 ; i < len ; ++i )
				hashValue = 31 * hashValue + wSeed.at(i);
			seedValue = hashValue;
		}

	}
	else
	{
		seedValue = BiomeSource::findSeed(isFlat?LevelType::lvl_flat:LevelType::lvl_normal);	// 4J - was (new Random())->nextLong() - now trying to actually find a seed to suit our requirements
	}

	g_NetworkManager.HostGame(dwLocalUsersMask,isClientSide,isPrivate,MINECRAFT_NET_MAX_PLAYERS,0);

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = seedValue;
	param->saveData = nullptr;
	param->texturePackId = pClass->m_MoreOptionsParams.dwTexturePack;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->skins->selectTexturePackById(pClass->m_MoreOptionsParams.dwTexturePack);
	//pMinecraft->skins->updateUI();

	app.SetGameHostOption(eGameHostOption_Difficulty,Minecraft::GetInstance()->options->difficulty);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,pClass->m_MoreOptionsParams.bAllowFriendsOfFriends);
	app.SetGameHostOption(eGameHostOption_Gamertags,app.GetGameSettings(pClass->m_iPad,eGameSetting_GamertagsVisible)?1:0);

	app.SetGameHostOption(eGameHostOption_BedrockFog,app.GetGameSettings(pClass->m_iPad,eGameSetting_BedrockFog)?1:0);

// 	CXuiList listObject;
// 	listObject.Attach( pClass->m_GameMode.GetListObject() );
	app.SetGameHostOption(eGameHostOption_GameType,pClass->m_bGameModeSurvival?GameType::SURVIVAL->getId():GameType::CREATIVE->getId());
	app.SetGameHostOption(eGameHostOption_LevelType,pClass->m_MoreOptionsParams.bFlatWorld );
	app.SetGameHostOption(eGameHostOption_Structures,pClass->m_MoreOptionsParams.bStructures );
	app.SetGameHostOption(eGameHostOption_BonusChest,pClass->m_MoreOptionsParams.bBonusChest );

	app.SetGameHostOption(eGameHostOption_PvP,pClass->m_MoreOptionsParams.bPVP);
	app.SetGameHostOption(eGameHostOption_TrustPlayers,pClass->m_MoreOptionsParams.bTrust );
	app.SetGameHostOption(eGameHostOption_FireSpreads,pClass->m_MoreOptionsParams.bFireSpreads );
	app.SetGameHostOption(eGameHostOption_TNT,pClass->m_MoreOptionsParams.bTNT );
	app.SetGameHostOption(eGameHostOption_HostCanFly,pClass->m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanChangeHunger,pClass->m_MoreOptionsParams.bHostPrivileges);
	app.SetGameHostOption(eGameHostOption_HostCanBeInvisible,pClass->m_MoreOptionsParams.bHostPrivileges );


	param->settings = app.GetGameHostOption( eGameHostOption_All );

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = static_cast<LPVOID>(param);

	// Reset the autosave time
	app.SetAutosaveTimerTime();

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

HRESULT CScene_MultiGameCreate::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		m_SliderDifficulty.SetValueDisplay(FALSE);
	}

	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled )
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
				m_pTexturePacksList->RemoveAllData();
			}
		}

	}

	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if(hObjSource == m_pTexturePacksList->m_hObj)
	{
		UpdateTexturePackDescription(pNotifySelChangedData->iItem);

		// 4J-JEV: Removed expand description check, taken care of elsewhere.
	}

	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
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
		// 4J-JEV: Shouldn't we always do this?
		//int texturePacksCount = Minecraft::GetInstance()->skins->getTexturePackCount();
		//if(texturePacksCount == 1)
		//{
			XUITimeline *timeline;
			XUINamedFrame *startFrame, *endFrame;
			GetTimeline( &timeline );
			startFrame = timeline->FindNamedFrame( L"SlideOut" );
			endFrame = timeline->FindNamedFrame( L"SlideOutEnd" );
			timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, endFrame->m_dwFrame, FALSE, FALSE );
			m_texturePackDescDisplayed = true;
		//}
	}

	return S_OK;
}

void CScene_MultiGameCreate::UpdateTexturePackDescription(int index)
{
	int iTexPackId=m_pTexturePacksList->GetData(index).iData;
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(iTexPackId);

	if(tp==nullptr)
	{
		// this is probably a texture pack icon added from TMS

		DWORD dwBytes=0,dwFileBytes=0;
		PBYTE pbData=nullptr,pbFileData=nullptr;

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
			m_texturePackComparison->UseBrush(nullptr);
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

		pbImageData = tp->getPackComparison(dwImageBytes);

		if(dwImageBytes > 0 && pbImageData)
		{
			XuiCreateTextureBrushFromMemory(pbImageData,dwImageBytes,&m_hTexturePackComparisonBrush);
			m_texturePackComparison->UseBrush(m_hTexturePackComparisonBrush);
		}
		else
		{
			m_texturePackComparison->UseBrush(nullptr);
		}
	}
}

void CScene_MultiGameCreate::UpdateCurrentTexturePack()
{
	m_currentTexturePackIndex = m_pTexturePacksList->GetCurSel();
	int iTexPackId=m_pTexturePacksList->GetData(m_currentTexturePackIndex).iData;
	TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(iTexPackId);

	// if the texture pack is null, you don't have it yet
	if(tp==nullptr)
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
			StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 3, ProfileManager.GetPrimaryPad(),&CScene_MultiGameCreate::TexturePackDialogReturned,this,app.GetStringTable());
		}
		else
		{
			uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			// Give the player a warning about the texture pack missing
			StorageManager.RequestMessageBox(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&CScene_MultiGameCreate::TexturePackDialogReturned,this,app.GetStringTable());
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

int CScene_MultiGameCreate::TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_MultiGameCreate *pClass = static_cast<CScene_MultiGameCreate *>(pParam);
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
			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);

		}
		else // trial version
		{
			// if there is no trial version, this is a Cancel
			DLC_INFO *pDLCInfo=app.GetDLCInfoForFullOfferID(ullOfferID_Full);
			if(pDLCInfo->ullOfferID_Trial!=0LL)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Trial;
				StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);
			}
		}
	}
	pClass->m_bIgnoreInput=false;
	return 0;
}

HRESULT CScene_MultiGameCreate::OnCustomMessage_DLCInstalled()
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
		// clear out the texture pack list
		m_pTexturePacksList->RemoveAllData();
		ClearTexturePackDescription();
	}
	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}

HRESULT CScene_MultiGameCreate::OnCustomMessage_DLCMountingComplete()
{
	// refill the texture pack list
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

			DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tp);
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

	m_iTexturePacksNotInstalled=0;

	// 4J-PB - there may be texture packs we don't have, so use the info from TMS for this
	// REMOVE UNTIL WORKING
	DLC_INFO *pDLCInfo=nullptr;

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
		if(m_iConfigA!=nullptr)
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

	m_currentTexturePackIndex = pMinecraft->skins->getTexturePackIndex(0);
	UpdateTexturePackDescription(m_currentTexturePackIndex);

	m_bSetup = true;
	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	return S_OK;
}

void CScene_MultiGameCreate::ClearTexturePackDescription()
{
	m_texturePackTitle.SetText(L" ");
	m_texturePackDescription.SetText(L" ");
	m_texturePackComparison->UseBrush(nullptr);
	m_texturePackIcon->UseBrush(nullptr);
}