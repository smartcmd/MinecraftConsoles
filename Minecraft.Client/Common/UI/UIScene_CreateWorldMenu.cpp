#include "stdafx.h"
#include "UI.h"
#include "UIScene_CreateWorldMenu.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\..\..\Minecraft.World\LevelSettings.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\BiomeSource.h"
#include "..\..\..\Minecraft.World\IntCache.h"
#include "..\..\..\Minecraft.World\LevelType.h"
#include "..\..\DLCTexturePack.h"

#ifdef __PSVITA__
#include "PSVita\Network\SQRNetworkManager_AdHoc_Vita.h"
#endif

#ifdef  _WINDOWS64

#include <windows.h>
#include "Xbox\Resource.h"
#endif

#define GAME_CREATE_ONLINE_TIMER_ID 0
#define GAME_CREATE_ONLINE_TIMER_TIME 100

int UIScene_CreateWorldMenu::m_iDifficultyTitleSettingA[4]=
{
	IDS_DIFFICULTY_TITLE_PEACEFUL,
	IDS_DIFFICULTY_TITLE_EASY,
	IDS_DIFFICULTY_TITLE_NORMAL,
	IDS_DIFFICULTY_TITLE_HARD
};

UIScene_CreateWorldMenu::UIScene_CreateWorldMenu(int iPad, void *initData, UILayer *parentLayer) : IUIScene_StartGame(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_worldName = app.GetString(IDS_DEFAULT_WORLD_NAME);
	m_seed = L"";

	m_iPad=iPad;

	m_labelWorldName.init(app.GetString(IDS_WORLD_NAME));

	m_editWorldName.init(m_worldName, eControl_EditWorldName);

	m_buttonGamemode.init(app.GetString(IDS_GAMEMODE_SURVIVAL),eControl_GameModeToggle);
	m_buttonMoreOptions.init(app.GetString(IDS_MORE_OPTIONS),eControl_MoreOptions);
	m_buttonCreateWorld.init(app.GetString(IDS_CREATE_NEW_WORLD),eControl_NewWorld);

	m_texturePackList.init(app.GetString(IDS_DLC_MENU_TEXTUREPACKS), eControl_TexturePackList);

	m_labelTexturePackName.init(L"");
	m_labelTexturePackDescription.init(L"");

	WCHAR TempString[256];
	swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[app.GetGameSettings(m_iPad,eGameSetting_Difficulty)]));	
	m_sliderDifficulty.init(TempString,eControl_Difficulty,0,3,app.GetGameSettings(m_iPad,eGameSetting_Difficulty));

	m_MoreOptionsParams.bGenerateOptions=TRUE;
	m_MoreOptionsParams.bStructures=TRUE;	
	m_MoreOptionsParams.bFlatWorld=FALSE;
	m_MoreOptionsParams.bBonusChest=FALSE;
	m_MoreOptionsParams.bPVP = TRUE;
	m_MoreOptionsParams.bTrust = TRUE;
	m_MoreOptionsParams.bFireSpreads = TRUE;
	m_MoreOptionsParams.bHostPrivileges = FALSE;
	m_MoreOptionsParams.bTNT = TRUE;
	m_MoreOptionsParams.iPad = iPad;

	m_MoreOptionsParams.bMobGriefing = true;
	m_MoreOptionsParams.bKeepInventory = false;
	m_MoreOptionsParams.bDoMobSpawning = true;
	m_MoreOptionsParams.bDoMobLoot = true;
	m_MoreOptionsParams.bDoTileDrops = true;
	m_MoreOptionsParams.bNaturalRegeneration = true;
	m_MoreOptionsParams.bDoDaylightCycle = true;

	m_bGameModeCreative = false;
	m_iGameModeId = GameType::SURVIVAL->getId();
	m_pDLCPack = NULL;
	m_bRebuildTouchBoxes = false;

	m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);
	// 4J-PB - read the settings for the online flag. We'll only save this setting if the user changed it.
	bool bGameSetting_Online=(app.GetGameSettings(m_iPad,eGameSetting_Online)!=0);
	m_MoreOptionsParams.bOnlineSettingChangedBySystem=false;

	// 4J-PB - Removing this so that we can attempt to create an online game on PS3 when we are a restricted child account
	// It'll fail when we choose create, but this matches the behaviour of load game, and lets the player know why they can't play online, 
	// instead of just greying out the online setting in the More Options
	// #ifdef __PS3__
	// 	if(ProfileManager.IsSignedInLive( m_iPad ))
	// 	{
	// 		ProfileManager.GetChatAndContentRestrictions(m_iPad,true,&bChatRestricted,&bContentRestricted,NULL);
	// 	}
	// #endif

	// Set the text for friends of friends, and default to on
	if( m_bMultiplayerAllowed )
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
	
	// Set up online game checkbox
	bool bOnlineGame = m_MoreOptionsParams.bOnlineGame;
	m_checkboxOnline.SetEnable(true);

	// 4J-PB - to stop an offline game being able to select the online flag
	if(ProfileManager.IsSignedInLive(m_iPad) == false)
	{
		m_checkboxOnline.SetEnable(false);
	}

	if(m_MoreOptionsParams.bOnlineSettingChangedBySystem)
	{
		m_checkboxOnline.SetEnable(false);
		bOnlineGame = false;
	}

	m_checkboxOnline.init(app.GetString(IDS_ONLINE_GAME), eControl_OnlineGame, bOnlineGame);

	addTimer( GAME_CREATE_ONLINE_TIMER_ID,GAME_CREATE_ONLINE_TIMER_TIME );
#if TO_BE_IMPLEMENTED
	XuiSetTimer(m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);
#endif

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_CreateWorldMenu, 0);

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
		int texturePacksCount = pMinecraft->skins->getTexturePackCount();
		for(unsigned int i = 0; i < texturePacksCount; ++i)
		{
			TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);

			DWORD dwImageBytes;
			PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

			if(dwImageBytes > 0 && pbImageData)
			{
				wchar_t imageName[64];
				swprintf(imageName,64,L"tpack%08x",tp->getId());
				registerSubstitutionTexture(imageName, pbImageData, dwImageBytes);
				m_texturePackList.addPack(i,imageName);
				app.DebugPrintf("Adding texture pack %ls at %d\n",imageName,i);
			}
		}

#if TO_BE_IMPLEMENTED
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
#endif

		UpdateTexturePackDescription(m_currentTexturePackIndex);


		m_texturePackList.selectSlot(m_currentTexturePackIndex);
	}
}

UIScene_CreateWorldMenu::~UIScene_CreateWorldMenu()
{
}

void UIScene_CreateWorldMenu::updateTooltips()
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_CreateWorldMenu::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

wstring UIScene_CreateWorldMenu::getMoviePath()
{
	return L"CreateWorldMenu";
}

UIControl* UIScene_CreateWorldMenu::GetMainPanel()
{
	return &m_controlMainPanel;
}

void UIScene_CreateWorldMenu::handleDestroy()
{
#ifdef __PSVITA__
	app.DebugPrintf("missing InputManager.DestroyKeyboard on Vita !!!!!!\n");
#endif

	// shut down the keyboard if it is displayed
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO)
	InputManager.DestroyKeyboard();
#endif
}

void UIScene_CreateWorldMenu::tick()
{
	UIScene::tick();


	if(m_iSetTexturePackDescription >= 0 )
	{
		UpdateTexturePackDescription( m_iSetTexturePackDescription );
		m_iSetTexturePackDescription = -1;
	}
	if(m_bShowTexturePackDescription)
	{
		slideLeft();
		m_texturePackDescDisplayed = true;

		m_bShowTexturePackDescription = false;
	}

#ifdef __ORBIS__
	// check the status of the PSPlus common dialog
	switch (sceNpCommerceDialogUpdateStatus())
	{
	case SCE_COMMON_DIALOG_STATUS_FINISHED:
		{
			SceNpCommerceDialogResult Result;
			sceNpCommerceDialogGetResult(&Result);
			sceNpCommerceDialogTerminate();

			if(Result.authorized)
			{
				ProfileManager.PsPlusUpdate(ProfileManager.GetPrimaryPad(), &Result);
				// they just became a PSPlus member
				checkStateAndStartGame();
			}
			else
			{
				// continue offline?
				UINT uiIDA[1];
				uiIDA[0]=IDS_PRO_NOTONLINE_DECLINE;

				// Give the player a warning about the texture pack missing
				ui.RequestAlertMessage(IDS_PLAY_OFFLINE,IDS_NO_PLAYSTATIONPLUS, uiIDA, 1, ProfileManager.GetPrimaryPad(),&UIScene_CreateWorldMenu::ContinueOffline,dynamic_cast<UIScene_CreateWorldMenu*>(this));
			}
		}
		break;
	default:
		break;
	}
#endif
}

#ifdef __ORBIS__
int UIScene_CreateWorldMenu::ContinueOffline(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)pParam;

	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultAccept) 
	{
		pClass->m_MoreOptionsParams.bOnlineGame=false;
		pClass->checkStateAndStartGame();
	}
	return 0;
}

#endif

#ifdef _WINDOWS64
void UIScene_CreateWorldMenu::getDirectEditInputs(vector<UIControl_TextInput*> &inputs)
{
	inputs.push_back(&m_editWorldName);
}

void UIScene_CreateWorldMenu::onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result)
{
	m_worldName = input->getEditBuffer();
	m_buttonCreateWorld.setEnable(!m_worldName.empty());
}
#endif

void UIScene_CreateWorldMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) { handled = true; return; }
#endif

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif

		// 4J-JEV: Inform user why their game must be offline.
#if defined _XBOX_ONE
		if ( pressed && controlHasFocus(m_checkboxOnline.getId()) && !m_checkboxOnline.IsEnabled() )
		{
			UINT uiIDA[1] = { IDS_CONFIRM_OK };
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad); 
		}
#endif

	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
	case ACTION_MENU_OTHER_STICK_UP:
	case ACTION_MENU_OTHER_STICK_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		
		bool bOnlineGame = m_checkboxOnline.IsChecked();
		if (m_MoreOptionsParams.bOnlineGame != bOnlineGame)
		{
			m_MoreOptionsParams.bOnlineGame = bOnlineGame;

			if (!m_MoreOptionsParams.bOnlineGame)
			{
				m_MoreOptionsParams.bInviteOnly = false;
				m_MoreOptionsParams.bAllowFriendsOfFriends = false;
			}
		}

		handled = true;
		break;
	}
}

void UIScene_CreateWorldMenu::handlePress(F64 controlId, F64 childId)
{
	if(m_bIgnoreInput) return;
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) return;
#endif

	//CD - Added for audio
	ui.PlayUISFX(eSFX_Press);

	switch((int)controlId)
	{
	case eControl_EditWorldName:
		{
			m_bIgnoreInput=true;
#ifdef _WINDOWS64
			if (!g_KBMInput.IsKBMActive())
			{
				UIKeyboardInitData kbData;
				kbData.title       = app.GetString(IDS_CREATE_NEW_WORLD);
				kbData.defaultText = m_editWorldName.getLabel();
				kbData.maxChars    = 25;
				kbData.callback    = &UIScene_CreateWorldMenu::KeyboardCompleteWorldNameCallback;
				kbData.lpParam     = this;
				ui.NavigateToScene(m_iPad, eUIScene_Keyboard, &kbData);
			}
			else
			{
				m_bIgnoreInput = false;
				m_editWorldName.beginDirectEdit(25);
			}
#else
			InputManager.RequestKeyboard(app.GetString(IDS_CREATE_NEW_WORLD),m_editWorldName.getLabel(),(DWORD)0,25,&UIScene_CreateWorldMenu::KeyboardCompleteWorldNameCallback,this,C_4JInput::EKeyboardMode_Default);
#endif
		}
		break;
	case eControl_GameModeToggle:
		switch(m_iGameModeId)
		{
		case 0: // Survival
			m_buttonGamemode.setLabel(app.GetString(IDS_GAMEMODE_CREATIVE));
			m_iGameModeId = GameType::CREATIVE->getId();
			m_bGameModeCreative = true;
			break;
		case 1: // Creative
			m_buttonGamemode.setLabel(app.GetString(IDS_GAMEMODE_SURVIVAL));
			m_iGameModeId = GameType::SURVIVAL->getId();
			m_bGameModeCreative = false;
			break;
		};
		break;
	case eControl_MoreOptions:
		ui.NavigateToScene(m_iPad, eUIScene_LaunchMoreOptionsMenu, &m_MoreOptionsParams);
		break;
	case eControl_TexturePackList:
		{
			UpdateCurrentTexturePack((int)childId);
		}
		break;
	case eControl_NewWorld:
		{
#ifdef _DURANGO
			if(m_MoreOptionsParams.bOnlineGame)
			{
				m_bIgnoreInput = true;
				ProfileManager.CheckMultiplayerPrivileges(m_iPad, true, &checkPrivilegeCallback, this);
			}
			else
#endif
			{
				StartSharedLaunchFlow();
			}
			break;
		}
	}
}

#ifdef _DURANGO
void UIScene_CreateWorldMenu::checkPrivilegeCallback(LPVOID lpParam, bool hasPrivilege, int iPad)
{
	UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)lpParam;

	if(hasPrivilege)
	{
		pClass->StartSharedLaunchFlow();
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}
}
#endif

void UIScene_CreateWorldMenu::StartSharedLaunchFlow()
{
	Minecraft *pMinecraft=Minecraft::GetInstance();
	// Check if we need to upsell the texture pack
	if(m_MoreOptionsParams.dwTexturePack!=0)
	{
		// texture pack hasn't been set yet, so check what it will be
		TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);

		if(pTexturePack==NULL)
		{
#if TO_BE_IMPLEMENTED
			// They've selected a texture pack they don't have yet
			// upsell
			CXuiCtrl4JList::LIST_ITEM_INFO ListItem;
			// get the current index of the list, and then get the data
			ListItem=m_pTexturePacksList->GetData(m_currentTexturePackIndex);


			// upsell the texture pack
			// tell sentient about the upsell of the full version of the skin pack
			ULONGLONG ullOfferID_Full;
			app.GetDLCFullOfferIDForPackID(m_MoreOptionsParams.dwTexturePack,&ullOfferID_Full);

			TelemetryManager->RecordUpsellPresented(ProfileManager.GetPrimaryPad(), eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

			UINT uiIDA[2];

			uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
			//uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			// Give the player a warning about the texture pack missing
			ui.RequestAlertMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&TexturePackDialogReturned,this);
			return;
		}
	}
	m_bIgnoreInput = true;

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

	app.CheckGameSettingsChanged(true,m_iPad);

	// Check that we have the rights to use a texture pack we have selected.
	if(m_MoreOptionsParams.dwTexturePack!=0)
	{
		// texture pack hasn't been set yet, so check what it will be
		TexturePack *pTexturePack = pMinecraft->skins->getTexturePackById(m_MoreOptionsParams.dwTexturePack);		
		DLCTexturePack *pDLCTexPack=(DLCTexturePack *)pTexturePack;
		m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();

		// do we have a license?
		if(m_pDLCPack && !m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{
			// no

			// We need to allow people to use a trial texture pack if they are offline - we only need them online if they want to buy it.

			/*
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;

			if(!ProfileManager.IsSignedInLive(m_iPad))
			{
			// need to be signed in to live
			ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
			m_bIgnoreInput = false;
			return;
			}
			else */
			{
				// upsell
#ifdef _XBOX
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
				TelemetryManager->RecordUpsellPresented(m_iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

#if defined(_DURANGO) || defined(_WINDOWS64)
				// trial pack warning
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_USING_TRIAL_TEXUREPACK_WARNING, uiIDA, 1, m_iPad,&TrialTexturePackWarningReturned,this);
#elif defined __PS3__ || defined __ORBIS__ || defined(__PSVITA__)
				// trial pack warning
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_OK;
				uiIDA[1]=IDS_CONFIRM_CANCEL;
				ui.RequestAlertMessage(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_USING_TRIAL_TEXUREPACK_WARNING, uiIDA, 2, m_iPad,&TrialTexturePackWarningReturned,this);	
#endif

#if defined _XBOX_ONE || defined __ORBIS__
				StorageManager.SetSaveDisabled(true);
#endif
				return;
			}
		}
	}
#if defined _XBOX_ONE || defined __ORBIS__
	app.SetGameHostOption(eGameHostOption_DisableSaving, m_MoreOptionsParams.bDisableSaving?1:0);
	StorageManager.SetSaveDisabled(m_MoreOptionsParams.bDisableSaving);
#endif
	checkStateAndStartGame();
}

void UIScene_CreateWorldMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	WCHAR TempString[256];
	int value = (int)currentValue;
	switch((int)sliderId)
	{
	case eControl_Difficulty:
		m_sliderDifficulty.handleSliderMove(value);

		app.SetGameSettings(m_iPad,eGameSetting_Difficulty,value);
		swprintf( (WCHAR *)TempString, 256, L"%ls: %ls", app.GetString( IDS_SLIDER_DIFFICULTY ),app.GetString(m_iDifficultyTitleSettingA[value]));		
		m_sliderDifficulty.setLabel(TempString);
		break;
	}
}

void UIScene_CreateWorldMenu::handleTimerComplete(int id)
{
#ifdef __PSVITA__
	// we cannot rebuild touch boxes in an iggy callback because it requires further iggy calls
	if(m_bRebuildTouchBoxes)
	{
		GetMainPanel()->UpdateControl();
		ui.TouchBoxRebuild(this);
		m_bRebuildTouchBoxes = false;
	}
#endif

	switch(id)
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
				
				m_checkboxOnline.SetEnable(bMultiplayerAllowed);
				m_checkboxOnline.setChecked(m_MoreOptionsParams.bOnlineGame);

				m_bMultiplayerAllowed = bMultiplayerAllowed;
			}
		}
		break;
		// 4J-PB - Only Xbox will not have trial DLC patched into the game
#ifdef _XBOX
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
					//app.DebugPrintf("Retrieving iConfig %d from TPD\n",m_iConfigA[i]);

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
						app.DebugPrintf("Adding texturepack  %d from TPD\n",m_iConfigA[i]);

						m_pTexturePacksList->AddData(ListInfo);

						m_iConfigA[i]=-1;
					}
				}
			}
		}
		break;
#endif
	};
}

void UIScene_CreateWorldMenu::handleGainFocus(bool navBack)
{
	if(navBack)
	{
		m_checkboxOnline.setChecked(m_MoreOptionsParams.bOnlineGame);
	}
}

int UIScene_CreateWorldMenu::KeyboardCompleteWorldNameCallback(LPVOID lpParam,bool bRes)
{
	UIScene_CreateWorldMenu *pClass=(UIScene_CreateWorldMenu *)lpParam;
	pClass->m_bIgnoreInput=false;
	// 4J HEG - No reason to set value if keyboard was cancelled
	if (bRes)
	{
		uint16_t pchText[128];
		ZeroMemory(pchText, 128 * sizeof(uint16_t) );
#ifdef _WINDOWS64
		Win64_GetKeyboardText(pchText, 128);
#else
		InputManager.GetText(pchText);
#endif

		if(pchText[0]!=0)
		{
			pClass->m_editWorldName.setLabel((wchar_t *)pchText);
			pClass->m_worldName = (wchar_t *)pchText;
		}

		pClass->m_buttonCreateWorld.setEnable( !pClass->m_worldName.empty() );
	}
	return 0;
}

void UIScene_CreateWorldMenu::checkStateAndStartGame()
{
	int primaryPad = ProfileManager.GetPrimaryPad();
	bool isSignedInLive = true;
	bool isOnlineGame = m_MoreOptionsParams.bOnlineGame;
	int iPadNotSignedInLive = -1;
	bool isLocalMultiplayerAvailable = app.IsLocalMultiplayerAvailable();
		
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		if (ProfileManager.IsSignedIn(i) && (i == primaryPad || isLocalMultiplayerAvailable))
		{
			if (isSignedInLive && !ProfileManager.IsSignedInLive(i))
			{
				// Record the first non signed in live pad
				iPadNotSignedInLive = i;
			}

			isSignedInLive = isSignedInLive && ProfileManager.IsSignedInLive(i);
		}
	}

	// If this is an online game but not all players are signed in to Live, stop!
	if (isOnlineGame && !isSignedInLive)
	{
#ifdef __ORBIS__
		assert(iPadNotSignedInLive != -1);

		// Check if PSN is unavailable because of age restriction
		int npAvailability = ProfileManager.getNPAvailability(iPadNotSignedInLive);
		if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
		{
			m_bIgnoreInput = false;
			// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive if the npAvailability isn't SCE_OK
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPadNotSignedInLive);
		}
		else
		{
			m_bIgnoreInput = true;
			UINT uiIDA[2];
			uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1] = IDS_CANCEL;
			ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, iPadNotSignedInLive, &UIScene_CreateWorldMenu::MustSignInReturnedPSN, this);
		}
		return;
/* 4J-PB - Add this after release
#elif defined __PSVITA__
		m_bIgnoreInput=false;
		// Determine why they're not "signed in live"
		if (ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()))
		{
			// Signed in to PSN but not connected (no internet access)
			UINT uiIDA[1];
			uiIDA[0] = IDS_CONFIRM_OK;
			ui.RequestErrorMessage(IDS_PRO_CURRENTLY_NOT_ONLINE_TITLE, IDS_PRO_PSNOFFLINE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
		}
		else
		{		
			// Not signed in to PSN
			UINT uiIDA[1];
			uiIDA[0] = IDS_CONFIRM_OK;
			ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad());
			return;
		}*/
#else
		m_bIgnoreInput=false;
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
		return;
#endif
	}

#ifdef __ORBIS__

	bool bPlayStationPlus = true;
	int iPadWithNoPlaystationPlus=0;
	if(isOnlineGame && isSignedInLive)
	{
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if(ProfileManager.IsSignedIn(i) && (i == primaryPad || isLocalMultiplayerAvailable))
			{
				if(ProfileManager.HasPlayStationPlus(i)==false)
				{
					bPlayStationPlus=false;
					iPadWithNoPlaystationPlus=i;
					break;
				}
			}
		}

		if(bPlayStationPlus==false)
		{
			m_bIgnoreInput=false;

			if(ProfileManager.RequestingPlaystationPlus(iPadWithNoPlaystationPlus))
			{
				// MGH -  added this so we don't try and upsell when we don't know if the player has PS Plus yet (if it can't connect to the PS Plus server).
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestAlertMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, ProfileManager.GetPrimaryPad(), NULL, NULL);
				return;
			}


			// 4J-PB - we're not allowed to show the text Playstation Plus - have to call the upsell all the time!
			// upsell psplus
			int32_t iResult=sceNpCommerceDialogInitialize();

			SceNpCommerceDialogParam param;
			sceNpCommerceDialogParamInitialize(&param);
			param.mode=SCE_NP_COMMERCE_DIALOG_MODE_PLUS;
			param.features = SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY; 
			param.userId = ProfileManager.getUserID(iPadWithNoPlaystationPlus);

			iResult=sceNpCommerceDialogOpen(&param);

// 			UINT uiIDA[2];
// 			uiIDA[0]=IDS_PLAY_OFFLINE;
// 			uiIDA[1]=IDS_PLAYSTATIONPLUS_SIGNUP;
// 			ui.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_PLAYSTATIONPLUS, uiIDA,2,ProfileManager.GetPrimaryPad(),&UIScene_CreateWorldMenu::PSPlusReturned,this, app.GetStringTable(),NULL,0,false);
			return;
		}
	}
#endif

	if(m_bGameModeCreative == true || m_MoreOptionsParams.bHostPrivileges == TRUE)
	{			
		UINT uiIDA[2];
		uiIDA[0]=IDS_CONFIRM_OK;
		uiIDA[1]=IDS_CONFIRM_CANCEL;
		if(m_bGameModeCreative == true)
		{
			ui.RequestAlertMessage(IDS_TITLE_START_GAME, IDS_CONFIRM_START_CREATIVE, uiIDA, 2, m_iPad,&UIScene_CreateWorldMenu::ConfirmCreateReturned,this);
		}
		else
		{
			ui.RequestAlertMessage(IDS_TITLE_START_GAME, IDS_CONFIRM_START_HOST_PRIVILEGES, uiIDA, 2, m_iPad,&UIScene_CreateWorldMenu::ConfirmCreateReturned,this);
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

		// Check if user-created content is allowed, as we cannot play multiplayer if it's not
		//bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && m_MoreOptionsParams.bOnlineGame;
		bool noUGC = false;
		BOOL pccAllowed = TRUE;
		BOOL pccFriendsAllowed = TRUE;
		bool bContentRestricted = false;

		ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
#if defined(__PS3__) || defined(__PSVITA__)
		if(isOnlineGame && isSignedInLive)
		{
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,NULL,&bContentRestricted,NULL);
		}
#endif

		noUGC = !pccAllowed && !pccFriendsAllowed;

		if(isOnlineGame && isSignedInLive && app.IsLocalMultiplayerAvailable())
		{
			// 4J-PB not sure why we aren't checking the content restriction for the main player here when multiple controllers are connected - adding now
			if(noUGC )
			{
				m_bIgnoreInput=false;
				ui.RequestUGCMessageBox();
			}
			else if(bContentRestricted )
			{
				m_bIgnoreInput=false;
				ui.RequestContentRestrictedMessageBox();
			}
#ifdef __ORBIS__
			else if(bPlayStationPlus==false)
			{
				m_bIgnoreInput=false;

				if(ProfileManager.RequestingPlaystationPlus(iPadWithNoPlaystationPlus))
				{
					// MGH -  added this so we don't try and upsell when we don't know if the player has PS Plus yet (if it can't connect to the PS Plus server).
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestAlertMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, ProfileManager.GetPrimaryPad(), NULL, NULL);
					return;
				}

				// 4J-PB - we're not allowed to show the text Playstation Plus - have to call the upsell all the time!
				// upsell psplus
				int32_t iResult=sceNpCommerceDialogInitialize();

				SceNpCommerceDialogParam param;
				sceNpCommerceDialogParamInitialize(&param);
				param.mode=SCE_NP_COMMERCE_DIALOG_MODE_PLUS;
				param.features = SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY; 
				param.userId = ProfileManager.getUserID(iPadWithNoPlaystationPlus);

				iResult=sceNpCommerceDialogOpen(&param);
// 				UINT uiIDA[2];
// 				uiIDA[0]=IDS_PLAY_OFFLINE;
// 				uiIDA[1]=IDS_PLAYSTATIONPLUS_SIGNUP;
// 				ui.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_PLAYSTATIONPLUS, uiIDA,2,ProfileManager.GetPrimaryPad(),&UIScene_CreateWorldMenu::PSPlusReturned,this, app.GetStringTable(),NULL,0,false);
			}

#endif
			else
			{
#ifdef _WINDOWS64
				// On Windows64, Xbox Live is unavailable. Skip QuadrantSignin and start directly.
				CreateGame(this, 0);
#else
				//ProfileManager.RequestSignInUI(false, false, false, true, false,&CScene_MultiGameCreate::StartGame_SignInReturned, this,ProfileManager.GetPrimaryPad());
				SignInInfo info;
				info.Func = &UIScene_CreateWorldMenu::StartGame_SignInReturned;
				info.lpParam = this;
				info.requireOnline = m_MoreOptionsParams.bOnlineGame;
				ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_QuadrantSignin,&info);
#endif
			}
		}
		else
		{
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

			if(isOnlineGame && isSignedInLive && noUGC )
			{
				m_bIgnoreInput=false;
				ui.RequestUGCMessageBox();
			}
			else if(isOnlineGame && isSignedInLive && bContentRestricted )
			{
				m_bIgnoreInput=false;
				ui.RequestContentRestrictedMessageBox();
			}
#ifdef __ORBIS__
			else if(isOnlineGame && isSignedInLive && (bPlayStationPlus==false))
			{
				m_bIgnoreInput=false;
				if(ProfileManager.RequestingPlaystationPlus(iPadWithNoPlaystationPlus))
				{
					// MGH -  added this so we don't try and upsell when we don't know if the player has PS Plus yet (if it can't connect to the PS Plus server).
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestAlertMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, ProfileManager.GetPrimaryPad(), NULL, NULL);
					return;
				}


				setVisible( true );

				// 4J-PB - we're not allowed to show the text Playstation Plus - have to call the upsell all the time!
				// upsell psplus
				int32_t iResult=sceNpCommerceDialogInitialize();

				SceNpCommerceDialogParam param;
				sceNpCommerceDialogParamInitialize(&param);
				param.mode=SCE_NP_COMMERCE_DIALOG_MODE_PLUS;
				param.features = SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY; 
				param.userId = ProfileManager.getUserID(iPadWithNoPlaystationPlus); 

				iResult=sceNpCommerceDialogOpen(&param);

// 				UINT uiIDA[2];
// 				uiIDA[0]=IDS_PLAY_OFFLINE;
// 				uiIDA[1]=IDS_PLAYSTATIONPLUS_SIGNUP;
// 				ui.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_PLAYSTATIONPLUS, uiIDA,2,ProfileManager.GetPrimaryPad(),&UIScene_CreateWorldMenu::PSPlusReturned,this, app.GetStringTable(),NULL,0,false);
			}

#endif
			else
			{
#if defined(__ORBIS__) || defined(__PSVITA__)
				if(isOnlineGame)
				{
					bool chatRestricted = false;
					ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,&chatRestricted,NULL,NULL);
					if(chatRestricted)
					{
						ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, ProfileManager.GetPrimaryPad() );
					}
				}
#endif
				CreateGame(this, 0);
			}
		}
	}

}

// 4J Stu - Shared functionality that is the same whether we needed a quadrant sign-in or not
void UIScene_CreateWorldMenu::CreateGame(UIScene_CreateWorldMenu* pClass, DWORD dwLocalUsersMask)
{
#if TO_BE_IMPLEMENTED
	// stop the timer running that causes a check for new texture packs in TMS but not installed, since this will run all through the create game, and will crash if it tries to create an hbrush
	XuiKillTimer(pClass->m_hObj,CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
#endif

	bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
#ifdef __PSVITA__
	if(CGameNetworkManager::usingAdhocMode())
	{
		if(SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())// && pClass->m_MoreOptionsParams.bOnlineGame)
			isClientSide = true;
	}
#endif // __PSVITA__

	bool isPrivate = pClass->m_MoreOptionsParams.bInviteOnly?true:false;

	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	// create the world and launch
	wstring wWorldName = pClass->m_worldName;

	StorageManager.ResetSaveData();
	// Make our next save default to the name of the level
	StorageManager.SetSaveTitle((wchar_t *)wWorldName.c_str());

	wstring wSeed;
	if(!pClass->m_MoreOptionsParams.seed.empty() )
	{
		wSeed=pClass->m_MoreOptionsParams.seed;
	}
	else
	{
		// random
		wSeed=L"";
	}

	// start the game
	bool isFlat = (pClass->m_MoreOptionsParams.bFlatWorld==TRUE);
	__int64 seedValue = 0;

	NetworkGameInitData *param = new NetworkGameInitData();
	param->levelName = wWorldName;

	if (wSeed.length() != 0)
	{
		__int64 value = 0;
		unsigned int len = (unsigned int)wSeed.length();

		//Check if the input string contains a numerical value
		bool isNumber = true;
		for( unsigned int i = 0 ; i < len ; ++i )
		{
			if( wSeed.at(i) < L'0' || wSeed.at(i) > L'9' )
			{
				if( !(i==0 && wSeed.at(i) == L'-' ) )
				{
					isNumber = false;
					break;
				}
			}
		}

		//If the input string is a numerical value, convert it to a number
		if( isNumber )
			value = _fromString<__int64>(wSeed);

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
		param->findSeed = true;	// 4J - java code sets the seed to was (new Random())->nextLong() here - we used to at this point find a suitable seed, but now just set a flag so this is performed in Minecraft::Server::initServer.
	}


	param->seed = seedValue;
	param->saveData = NULL;
	param->texturePackId = pClass->m_MoreOptionsParams.dwTexturePack;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	pMinecraft->skins->selectTexturePackById(pClass->m_MoreOptionsParams.dwTexturePack);

	app.SetGameHostOption(eGameHostOption_Difficulty,Minecraft::GetInstance()->options->difficulty);
	app.SetGameHostOption(eGameHostOption_FriendsOfFriends,pClass->m_MoreOptionsParams.bAllowFriendsOfFriends);
	app.SetGameHostOption(eGameHostOption_Gamertags,app.GetGameSettings(pClass->m_iPad,eGameSetting_GamertagsVisible)?1:0);

	app.SetGameHostOption(eGameHostOption_BedrockFog,app.GetGameSettings(pClass->m_iPad,eGameSetting_BedrockFog)?1:0);

	app.SetGameHostOption(eGameHostOption_GameType,pClass->m_iGameModeId );
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
	
	app.SetGameHostOption(eGameHostOption_MobGriefing, pClass->m_MoreOptionsParams.bMobGriefing);
	app.SetGameHostOption(eGameHostOption_KeepInventory, pClass->m_MoreOptionsParams.bKeepInventory);
	app.SetGameHostOption(eGameHostOption_DoMobSpawning, pClass->m_MoreOptionsParams.bDoMobSpawning);
	app.SetGameHostOption(eGameHostOption_DoMobLoot, pClass->m_MoreOptionsParams.bDoMobLoot);
	app.SetGameHostOption(eGameHostOption_DoTileDrops, pClass->m_MoreOptionsParams.bDoTileDrops);
	app.SetGameHostOption(eGameHostOption_NaturalRegeneration, pClass->m_MoreOptionsParams.bNaturalRegeneration);
	app.SetGameHostOption(eGameHostOption_DoDaylightCycle, pClass->m_MoreOptionsParams.bDoDaylightCycle);
	
	app.SetGameHostOption(eGameHostOption_WasntSaveOwner, false); 
#ifdef _LARGE_WORLDS
	app.SetGameHostOption(eGameHostOption_WorldSize, pClass->m_MoreOptionsParams.worldSize+1 );  // 0 is GAME_HOST_OPTION_WORLDSIZE_UNKNOWN
	pClass->m_MoreOptionsParams.currentWorldSize = (EGameHostOptionWorldSize)(pClass->m_MoreOptionsParams.worldSize+1);
	pClass->m_MoreOptionsParams.newWorldSize = (EGameHostOptionWorldSize)(pClass->m_MoreOptionsParams.worldSize+1);
#endif

	g_NetworkManager.HostGame(dwLocalUsersMask,isClientSide,isPrivate,MINECRAFT_NET_MAX_PLAYERS,0);

	param->settings = app.GetGameHostOption( eGameHostOption_All );

#ifdef _LARGE_WORLDS
	switch(pClass->m_MoreOptionsParams.worldSize)
	{
	case 0:
		// Classic
		param->xzSize = LEVEL_WIDTH_CLASSIC;
		param->hellScale = HELL_LEVEL_SCALE_CLASSIC;  // hellsize = 54/3 = 18
		break;
	case 1:
		// Small
		param->xzSize = LEVEL_WIDTH_SMALL;
		param->hellScale = HELL_LEVEL_SCALE_SMALL;	// hellsize = ceil(64/3) = 22
		break;
	case 2:
		// Medium
		param->xzSize = LEVEL_WIDTH_MEDIUM;
		param->hellScale = HELL_LEVEL_SCALE_MEDIUM;	// hellsize= ceil(3*64/6) = 32
		break;
	case 3:
		// Large
		param->xzSize = LEVEL_WIDTH_LARGE;
		param->hellScale = HELL_LEVEL_SCALE_LARGE; // hellsize = ceil(5*64/8) = 40
		break;
	};
#else
	param->xzSize = LEVEL_MAX_WIDTH;
	param->hellScale = HELL_LEVEL_MAX_SCALE;
#endif

#ifndef _XBOX
	g_NetworkManager.FakeLocalPlayerJoined();
#endif

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = (LPVOID)param;

	// Reset the autosave time
	app.SetAutosaveTimerTime();

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = DEFAULT_XUI_MENU_USER;
	loadingParams->completionData = completionData;

	ui.NavigateToScene(pClass->m_iPad,eUIScene_FullscreenProgress, loadingParams);
}


int UIScene_CreateWorldMenu::StartGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)pParam;

	if(bContinue==true)
	{
		// It's possible that the player has not signed in - they can back out
		if(ProfileManager.IsSignedIn(pClass->m_iPad))
		{
			bool isOnlineGame = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
			// bool isOnlineGame = pClass->m_MoreOptionsParams.bOnlineGame;
			int primaryPad = ProfileManager.GetPrimaryPad();
			bool noPrivileges = false;
			DWORD dwLocalUsersMask = 0;
			bool isSignedInLive = ProfileManager.IsSignedInLive(primaryPad);
			int iPadNotSignedInLive = -1;
			bool isLocalMultiplayerAvailable = app.IsLocalMultiplayerAvailable();

			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if (ProfileManager.IsSignedIn(i) && ((i == primaryPad) || isLocalMultiplayerAvailable))
				{
					if (isSignedInLive && !ProfileManager.IsSignedInLive(i))
					{
						// Record the first non signed in live pad
						iPadNotSignedInLive = i;
					}

					if( !ProfileManager.AllowedToPlayMultiplayer(i) ) noPrivileges = true;
					dwLocalUsersMask |= CGameNetworkManager::GetLocalPlayerMask(i);
					isSignedInLive = isSignedInLive && ProfileManager.IsSignedInLive(i);
				}
			}

			// If this is an online game but not all players are signed in to Live, stop!
			if (isOnlineGame && !isSignedInLive)
			{
#ifdef __ORBIS__
				assert(iPadNotSignedInLive != -1);

				// Check if PSN is unavailable because of age restriction
				int npAvailability = ProfileManager.getNPAvailability(iPadNotSignedInLive);
				if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					pClass->m_bIgnoreInput = false;
					// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive if the npAvailability isn't SCE_OK
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestAlertMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPadNotSignedInLive);
				}
				else
				{
					pClass->m_bIgnoreInput=true;
					UINT uiIDA[2];
					uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
					uiIDA[1] = IDS_CANCEL;
					ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, iPadNotSignedInLive, &UIScene_CreateWorldMenu::MustSignInReturnedPSN, pClass);
				}
				return 0;
#else
				pClass->m_bIgnoreInput=false;
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
				return 0;
#endif
			}

			// Check if user-created content is allowed, as we cannot play multiplayer if it's not
			bool noUGC = false;
			BOOL pccAllowed = TRUE;
			BOOL pccFriendsAllowed = TRUE;

			ProfileManager.AllowedPlayerCreatedContent(ProfileManager.GetPrimaryPad(),false,&pccAllowed,&pccFriendsAllowed);
			if(!pccAllowed && !pccFriendsAllowed) noUGC = true;

			if(isOnlineGame && (noPrivileges || noUGC) )
			{
				if( noUGC )
				{
					pClass->m_bIgnoreInput = false;
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad());
				}
				else
				{
					pClass->m_bIgnoreInput = false;
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_HOST_TEXT, uiIDA,1,ProfileManager.GetPrimaryPad());
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
	}
	return 0;
}


int UIScene_CreateWorldMenu::ConfirmCreateReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept) 
	{
		bool isClientSide = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;

		// 4J Stu - If we only have one controller connected, then don't show the sign-in UI again
		DWORD connectedControllers = 0;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) ) ++connectedControllers;
		}

		if(isClientSide && app.IsLocalMultiplayerAvailable())
		{
#ifdef _WINDOWS64
			// On Windows64, Xbox Live is unavailable. Skip QuadrantSignin and start directly.
			CreateGame(pClass, 0);
			return 0;
#else
			//ProfileManager.RequestSignInUI(false, false, false, true, false,&UIScene_CreateWorldMenu::StartGame_SignInReturned, pClass,ProfileManager.GetPrimaryPad());
			SignInInfo info;
			info.Func = &UIScene_CreateWorldMenu::StartGame_SignInReturned;
			info.lpParam = pClass;
			info.requireOnline = pClass->m_MoreOptionsParams.bOnlineGame;
			ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_QuadrantSignin,&info);
#endif
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
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestAlertMessage( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_CREATE, uiIDA,1,ProfileManager.GetPrimaryPad());
			}
			else
			{	
#if defined( __ORBIS__) || defined(__PSVITA__)
				bool isOnlineGame = ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && pClass->m_MoreOptionsParams.bOnlineGame;
				if(isOnlineGame)
				{
					bool chatRestricted = false;
					ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),false,&chatRestricted,NULL,NULL);
					if(chatRestricted)
					{
						ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, ProfileManager.GetPrimaryPad() );
					}
				}
#endif
				CreateGame(pClass, 0);
			}
		}
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}
	return 0;
}

#ifdef __ORBIS__
int UIScene_CreateWorldMenu::MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_CreateWorldMenu* pClass = (UIScene_CreateWorldMenu *)pParam;
	pClass->m_bIgnoreInput = false;

    if(result==C4JStorage::EMessage_ResultAccept) 
    {
        SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_CreateWorldMenu::StartGame_SignInReturned, pClass, false, iPad);
    }

    return 0;
}

// int UIScene_CreateWorldMenu::PSPlusReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
// {
// 	int32_t iResult;
// 	UIScene_CreateWorldMenu *pClass = (UIScene_CreateWorldMenu *)pParam;
// 
// 	// continue offline, or upsell PS Plus?
// 	if(result==C4JStorage::EMessage_ResultDecline) 
// 	{
// 		// upsell psplus
// 		int32_t iResult=sceNpCommerceDialogInitialize();
// 
// 		SceNpCommerceDialogParam param;
// 		sceNpCommerceDialogParamInitialize(&param);
// 		param.mode=SCE_NP_COMMERCE_DIALOG_MODE_PLUS;
// 		param.features = SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY; 
// 		param.userId = ProfileManager.getUserID(pClass->m_iPad);
// 
// 		iResult=sceNpCommerceDialogOpen(&param);
// 	}
// 	else if(result==C4JStorage::EMessage_ResultAccept) 
// 	{
// 		// continue offline
// 		pClass->m_MoreOptionsParams.bOnlineGame=false;
// 		pClass->checkStateAndStartGame();
// 	}
// 
// 	pClass->m_bIgnoreInput=false;
// 	return 0;
// }
#endif


void UIScene_CreateWorldMenu::handleTouchBoxRebuild()
{
	m_bRebuildTouchBoxes = true;
}
