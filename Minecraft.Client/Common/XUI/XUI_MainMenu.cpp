// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\XUI\XUI_MainMenu.h"
#include "..\..\..\Minecraft.Client\SurvivalMode.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileIO.h"
#include "..\..\LocalPlayer.h"
#include "..\..\..\Minecraft.World\AABB.h"
#include "..\..\..\Minecraft.World\Vec3.h"
#include "..\..\User.h"
//#include "XUI_CreateLoad.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\Random.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\Font.h"
#include "..\..\Common\GameRules\ConsoleGameRules.h"

#define DLC_INSTALLED_TIMER_ID 1
#define DLC_INSTALLED_TIMER_TIME 100
#define TMS_TIMER_ID 2
#define TMS_TIMER_TIME 100
Random *CScene_Main::random = new Random();

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Main::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();

	XuiControlSetText(m_Buttons[BUTTON_PLAYGAME],app.GetString(IDS_PLAY_GAME));
	XuiControlSetText(m_Buttons[BUTTON_LEADERBOARDS],app.GetString(IDS_LEADERBOARDS));
	XuiControlSetText(m_Buttons[BUTTON_ACHIEVEMENTS],app.GetString(IDS_ACHIEVEMENTS));
	XuiControlSetText(m_Buttons[BUTTON_HELPANDOPTIONS],app.GetString(IDS_HELP_AND_OPTIONS));
	XuiControlSetText(m_Buttons[BUTTON_UNLOCKFULLGAME],app.GetString(IDS_UNLOCK_FULL_GAME));
	XuiControlSetText(m_Buttons[BUTTON_EXITGAME],app.GetString(IDS_EXIT_GAME));

	m_Timer.SetShow(FALSE);
	m_eAction=eAction_None;

	// Display the tooltips
	HRESULT hr = S_OK; 
	ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);

	// can't set presence until someone is signed in and playing

	// Need to check which menu items to display

	// Are we the trial version?
	// store the exitgame position
	m_Buttons[BUTTON_EXITGAME].GetPosition(&m_vPosExitGame);

	if(ProfileManager.IsFullVersion())
	{
		// Replace the Unlock Full Game with Downloadable Content
		m_Buttons[BUTTON_UNLOCKFULLGAME].SetText(app.GetString(IDS_DOWNLOADABLECONTENT));
		XuiElementSetShow(m_Buttons[BUTTON_UNLOCKFULLGAME],TRUE);
	}

	// Do we have downloadable content? - We need to have this in for a Pre-Cert test, whether or not we have DLC at the time.


	CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );

	const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
	WCHAR szResourceLocator[ LOCATOR_SIZE ];

	// load from the .xzp file
	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(nullptr);
	swprintf(szResourceLocator, LOCATOR_SIZE ,L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/splashes.txt");

	BYTE *splashesData;
	UINT splashesSize;
	hr = XuiResourceLoadAllNoLoc(szResourceLocator, &splashesData, &splashesSize);

	if( HRESULT_SUCCEEDED( hr ) )
	{
		//BufferedReader *br = new BufferedReader(new InputStreamReader(InputStream::getResourceAsStream(L"res\\title\\splashes.txt"))); //, Charset.forName("UTF-8")
		byteArray splashesArray(splashesSize);
		memcpy(splashesArray.data, splashesData, splashesSize);
		ByteArrayInputStream *bais= new ByteArrayInputStream(splashesArray);
		InputStreamReader *isr = new InputStreamReader( bais );
		BufferedReader *br = new BufferedReader( isr );

		wstring line = L"";
		while ( !(line = br->readLine()).empty() )
		{
			line = trimString( line );
			if (line.length() > 0)
			{
				m_splashes.push_back(line);
			}
		}

		XuiFree(splashesData);	// Frees copy returned from XuiResourceLoadAllNoLoc

		br->close();
		delete br;
		delete isr;
		delete bais;			// Frees copy made in splashesArray
	}

	XuiElementGetBounds(m_Subtitle,&m_fSubtitleWidth, &m_fSubtitleHeight);

#if 1
	XuiElementSetShow(m_Subtitle, FALSE);
	XuiElementSetShow(m_SubtitleMCFont, TRUE);
#else
	XuiElementSetShow(m_Subtitle, TRUE);
	XuiElementSetShow(m_SubtitleMCFont, FALSE);
#endif

	m_bIgnorePress=false;

	// 4J Stu - Clear out any loaded game rules
	app.setLevelGenerationOptions(nullptr);

	// Fix for #45154 - Frontend: DLC: Content can only be downloaded from the frontend if you have not joined/exited multiplayer
	XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);

	return S_OK;
}

HRESULT CScene_Main::OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	// If we sign out when in the saves list, we get a notifysetfocus in the saves list after the init of the main menu
	ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);

	return S_OK;
}


//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Main::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// should we ignore the button press? This will be set if we're waiting for a callback from a function launched from a button press
	if(m_bIgnorePress) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;
	//Minecraft *pMinecraft=Minecraft::GetInstance();

	while((uiButtonCounter<BUTTONS_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;

	ProfileManager.SetPrimaryPad(pNotifyPressData->UserIndex);
	ProfileManager.SetLockedProfile(-1);
	
	// Determine which button was pressed,
	// and call the appropriate function.	
	switch(uiButtonCounter)
	{
	case BUTTON_PLAYGAME:
		// Move to the new/load game screen
		// need a signed in user here

		ProfileManager.SetCurrentGameActivity(pNotifyPressData->UserIndex,CONTEXT_PRESENCE_MENUS,true);
		
		m_eAction=eAction_RunGame;
		if(ProfileManager.IsSignedIn(pNotifyPressData->UserIndex))
		{
			RunPlayGame(pNotifyPressData->UserIndex);
		}
		else
		{
			// get them to sign in
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			StorageManager.RequestMessageBox(IDS_MUST_SIGN_IN_TITLE, IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_Main::MustSignInReturned,this, app.GetStringTable());
		}
	
		break;

	case BUTTON_LEADERBOARDS:
		m_eAction=eAction_RunLeaderboards;
		if(ProfileManager.IsSignedIn(pNotifyPressData->UserIndex))
		{				
			RunLeaderboards(pNotifyPressData->UserIndex);
		}
		else
		{
			// get them to sign in
			//ProfileManager.RequestSignInUI(false, false, true,false,true, &CScene_Main::Leaderboards_SignInReturned, this);
			// get them to sign in
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			StorageManager.RequestMessageBox(IDS_MUST_SIGN_IN_TITLE, IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_Main::MustSignInReturned,this, app.GetStringTable());

		}
		break;
	case BUTTON_ACHIEVEMENTS:
		m_eAction=eAction_RunAchievements;
		if(ProfileManager.IsSignedIn(pNotifyPressData->UserIndex))
		{
			RunAchievements(pNotifyPressData->UserIndex);
		}
		else
		{
			// get them to sign in
			//ProfileManager.RequestSignInUI(false, false, true,false,true,&CScene_Main::Achievements_SignInReturned,this );
			// get them to sign in
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			StorageManager.RequestMessageBox(IDS_MUST_SIGN_IN_TITLE, IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_Main::MustSignInReturned,this, app.GetStringTable());

		}

		break;
	case BUTTON_HELPANDOPTIONS:
		// need a signed in user here, so we have a profile to write to
		ProfileManager.SetLockedProfile(pNotifyPressData->UserIndex);

		m_eAction=eAction_RunHelpAndOptions;
		if(ProfileManager.IsSignedIn(pNotifyPressData->UserIndex))
		{
			RunHelpAndOptions(pNotifyPressData->UserIndex);
		}
		else
		{
			// get them to sign in
			//ProfileManager.RequestSignInUI(false, false, true,false,true,&CScene_Main::HelpAndOptions_SignInReturned,this );
			// get them to sign in
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			StorageManager.RequestMessageBox(IDS_MUST_SIGN_IN_TITLE, IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_Main::MustSignInReturned,this, app.GetStringTable());

		}
		break;
		// 	case BUTTON_RETURNTOARCADE:
		// 		break;
	case BUTTON_UNLOCKFULLGAME:
		{
			// need a signed in user here
			ProfileManager.SetLockedProfile(pNotifyPressData->UserIndex);

			m_eAction=eAction_RunUnlockOrDLC;
			if(ProfileManager.IsSignedIn(pNotifyPressData->UserIndex))
			{
				RunUnlockOrDLC(pNotifyPressData->UserIndex);
			}
			else
			{		
				// get them to sign in
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_OK;
				uiIDA[1]=IDS_CONFIRM_CANCEL;
				StorageManager.RequestMessageBox(IDS_MUST_SIGN_IN_TITLE, IDS_MUST_SIGN_IN_TEXT, uiIDA, 2, pNotifyPressData->UserIndex,&CScene_Main::MustSignInReturned,this, app.GetStringTable());
			}
		}
		break;
	case BUTTON_EXITGAME:
		if( ProfileManager.IsFullVersion() )
		{				
			UINT uiIDA[2];
			uiIDA[0]=IDS_CANCEL;
			uiIDA[1]=IDS_OK;
			StorageManager.RequestMessageBox(IDS_WARNING_ARCADE_TITLE, IDS_WARNING_ARCADE_TEXT, uiIDA, 2, XUSER_INDEX_ANY,&CScene_Main::ExitGameReturned,this);
		}
		else
		{
			app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_TrialExitUpsell);
		}
		break;

	default:
		break;
	}



	return S_OK;
}

HRESULT CScene_Main::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT);
	CXuiSceneBase::ShowLogo(DEFAULT_XUI_MENU_USER,TRUE);	
	CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	// unlock the locked profile - anyone can navigate the main menu
	ProfileManager.SetLockedProfile(-1);

	// incase the debug trial version has been set
	// Are we the trial version?

	m_Buttons[BUTTON_EXITGAME].GetPosition(&m_vPosExitGame);

	for(int i=0;i<BUTTONS_MAX;i++)
	{
		m_Buttons[i].SetShow(TRUE);
	}


	if(ProfileManager.IsFullVersion())
	{
		// Replace the Unlock Full Game with Downloadable Content
		m_Buttons[BUTTON_UNLOCKFULLGAME].SetText(app.GetString(IDS_DOWNLOADABLECONTENT));
		XuiElementSetShow(m_Buttons[BUTTON_UNLOCKFULLGAME],TRUE);
	}

	// Fix for #45154 - Frontend: DLC: Content can only be downloaded from the frontend if you have not joined/exited multiplayer
	XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
	m_bIgnorePress=false;
	m_Timer.SetShow(FALSE);

	return S_OK;
}

HRESULT CScene_Main::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		// 4J-PB - remove the "hobo humping" message legal (Sony) say we can't have - pretty sure Microsoft would say the same if they noticed it.
		int splashIndex = eSplashRandomStart + 1 + random->nextInt( static_cast<int>(m_splashes.size()) - (eSplashRandomStart + 1) );

		// Override splash text on certain dates
		SYSTEMTIME LocalSysTime;
		GetLocalTime( &LocalSysTime );
		if (LocalSysTime.wMonth == 11 && LocalSysTime.wDay == 9)
		{
			splashIndex = eSplashHappyBirthdayEx;
        }
		else if (LocalSysTime.wMonth == 6 && LocalSysTime.wDay == 1)
		{
			splashIndex = eSplashHappyBirthdayNotch;
        }
		else if (LocalSysTime.wMonth == 12 && LocalSysTime.wDay == 24) // the Java game shows this on Christmas Eve, so we will too
		{
			splashIndex = eSplashMerryXmas;
        }
		else if (LocalSysTime.wMonth == 1 && LocalSysTime.wDay == 1)
		{
			splashIndex = eSplashHappyNewYear;
        }
		//splashIndex = 47; // Very short string
		//splashIndex = 197; // Very long string
		//splashIndex = 296; // Coloured
		//splashIndex = 297; // Noise
		wstring splash = m_splashes.at( splashIndex );
		m_Subtitle.SetText(splash.c_str());
		m_SubtitleMCFont.SetText(splash.c_str());

#ifndef OVERRIDE_XUI_FONT_RENDERER
		XUIRect xuiRect;
		HRESULT hr=S_OK;
		float fWidth,fHeight;

		HXUIOBJ visual=nullptr;
		HXUIOBJ pulser, subtitle, text;
		hr=XuiControlGetVisual(m_Subtitle.m_hObj,&visual);
		hr=XuiElementGetChildById(visual,L"Pulser",&pulser);
		hr=XuiElementGetChildById(pulser,L"SubTitle",&subtitle);
		hr=XuiElementGetChildById(subtitle,L"Text_String",&text);

		memset(&xuiRect, 0, sizeof(xuiRect));
		// Start with a base size
		XuiElementSetBounds(m_Subtitle,m_fSubtitleWidth, m_fSubtitleHeight);
		hr=XuiTextPresenterMeasureText(text, splash.c_str(), &xuiRect);
		XuiElementGetBounds(text,&fWidth, &fHeight);

		float diff = fWidth / (xuiRect.right+5);

		diff = min(diff,MAIN_MENU_MAX_TEXT_SCALE);

		// Resize
		XuiElementGetBounds(m_Subtitle,&fWidth, &fHeight);
		XuiElementSetBounds(m_Subtitle,fWidth/diff, fHeight);

		// Scale
		D3DXVECTOR3 vScale(diff,diff,0);
		XuiElementSetScale(m_Subtitle,&vScale);

		//Adjust pivot for animation
		D3DXVECTOR3 vPivot;
		XuiElementGetPivot(subtitle,&vPivot);
		vPivot.x = vPivot.x + ( ( (fWidth/diff) - fWidth) / 2 );
		XuiElementSetPivot(subtitle,&vPivot);

		// 4J-PB - Going to resize buttons if the text is too big to fit on any of them (Br-pt problem with the length of Unlock Full Game)
		
		float fMaxTextLen=0.0f;
		float fTextVisualLen;
		float fMaxButton;

		hr=XuiControlGetVisual(m_Buttons[0].m_hObj,&visual);
		hr=XuiElementGetChildById(visual,L"text_Label",&text);
		hr=XuiElementGetBounds(text,&fTextVisualLen,&fHeight);
		m_Buttons[0].GetBounds(&fMaxButton,&fHeight);


		for(int i=0;i<BUTTONS_MAX;i++)
		{
			hr=XuiTextPresenterMeasureText(text, m_Buttons[i].GetText(), &xuiRect);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
		}

		if(fTextVisualLen<fMaxTextLen)
		{
			D3DXVECTOR3 vec;

			// centre is vec.x+(fWidth/2)
			for(int i=0;i<BUTTONS_MAX;i++)
			{
				// need to resize and reposition the buttons
				m_Buttons[i].GetPosition(&vec);
				m_Buttons[i].GetBounds(&fWidth,&fHeight);
				vec.x= vec.x+(fWidth/2.0f)-(fMaxTextLen/2.0f); 

				m_Buttons[i].SetPosition(&vec);
				m_Buttons[i].SetBounds(fMaxButton+fMaxTextLen-fTextVisualLen,fHeight);
			}
		}
#endif
	}

	return S_OK;
}

HRESULT CScene_Main::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
#ifdef _MINECON
	// added so we can skip greyed out items for Minecon
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);
	
	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}
#endif
	return S_OK;
}

HRESULT CScene_Main::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Don't set handled to true.

	return S_OK;
}

/////////////////////////////////////////////////////////////

int CScene_Main::SignInReturned(void *pParam,bool bContinue)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(bContinue==true)
	{
		StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,pClass);
	}


	return 0;
}

int CScene_Main::DeviceSelectReturned(void *pParam,bool bContinue)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);
	//HRESULT hr;

	if(bContinue==true)
	{
		// change the minecraft player name
		Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));
		// ensure we've applied this player's settings
		app.ApplyGameSettingsChanged(ProfileManager.GetPrimaryPad());
		// check for DLC
		// start timer to track DLC check finished
		pClass->m_Timer.SetShow(TRUE);
		XuiSetTimer(pClass->m_hObj,DLC_INSTALLED_TIMER_ID,DLC_INSTALLED_TIMER_TIME);

		//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		ProfileManager.SetPrimaryPad(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}
	}

	return 0;
}

int CScene_Main::CreateLoad_OfflineProfileReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(bContinue==true)
	{
		// accepted offline profiles, so go on to select a device
		ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());

		// change the minecraft player name
		Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

		if(ProfileManager.IsFullVersion())
		{
			if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,pClass))
			{
				// save device already selected
				// ensure we've applied this player's settings
				app.ApplyGameSettingsChanged(ProfileManager.GetPrimaryPad());

				// check for DLC
				// start timer to track DLC check finished
				pClass->m_Timer.SetShow(TRUE);
				XuiSetTimer(pClass->m_hObj,DLC_INSTALLED_TIMER_ID,DLC_INSTALLED_TIMER_TIME);

				//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
			}
		}
		else
		{
			// 4J-PB - if this is the trial game, we can't have any networking
			// Can't apply the player's settings here - they haven't come back from the QuerySignInStatud call above yet.
			// Need to let them action in the main loop when they come in
			// ensure we've applied this player's settings
			//app.ApplyGameSettingsChanged(iPad);
			// go straight in to the trial level
			LoadTrial();
		}
	}
	else
	{
		// force a sign-in - they were offline, and they want to be online, so don't let it display offline players
		// set the bAddUser to false to allow offline to go online by selecting the already signed in player again
		ProfileManager.RequestSignInUI(false, false, true,false,false,&CScene_Main::CreateLoad_SignInReturned,pClass, ProfileManager.GetPrimaryPad() );
	}

	return 0;
}

int CScene_Main::CreateLoad_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(bContinue==true)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_OK;

		if(ProfileManager.IsGuest(ProfileManager.GetPrimaryPad()))
		{
			StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
		}
		else
		{
			ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());


			// change the minecraft player name
			Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

			if(ProfileManager.IsFullVersion())
			{
				// Check if we're signed in to LIVE
				if(ProfileManager.IsSignedInLive(iPad))
				{
					// 4J-PB - Need to check for installed DLC
					if(!app.DLCInstallProcessCompleted()) app.StartInstallDLCProcess(iPad);

					if(ProfileManager.IsGuest(iPad))
					{
						StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
					}
					else
					{
						// check if all the TMS files are loaded
						if(app.GetTMSDLCInfoRead() && app.GetTMSXUIDsFileRead() && app.GetBanListRead(iPad))
						{
							if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,pClass)==true)
							{
								// save device already selected

								// ensure we've applied this player's settings
								app.ApplyGameSettingsChanged(ProfileManager.GetPrimaryPad());
								// check for DLC
								// start timer to track DLC check finished
								pClass->m_Timer.SetShow(TRUE);
								XuiSetTimer(pClass->m_hObj,DLC_INSTALLED_TIMER_ID,DLC_INSTALLED_TIMER_TIME);
								//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_MultiGameJoinLoad);
							}
						}
						else
						{
							// Changing to async TMS calls
							app.SetTMSAction(iPad,eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);

							// block all input
							pClass->m_bIgnorePress=true;
							// We want to hide everything in this scene and display a timer until we get a completion for the TMS files
							for(int i=0;i<BUTTONS_MAX;i++)
							{
								pClass->m_Buttons[i].SetShow(FALSE);
							}

							// turn off tooltips
							ui.SetTooltips(DEFAULT_XUI_MENU_USER, -1);

							pClass->m_Timer.SetShow(TRUE);
						}
					}
				}
				else
				{
					// offline
					ProfileManager.DisplayOfflineProfile(&CScene_Main::CreateLoad_OfflineProfileReturned,pClass, ProfileManager.GetPrimaryPad() );
				}
			}
			else
			{
				// 4J-PB - if this is the trial game, we can't have any networking
				// Can't apply the player's settings here - they haven't come back from the QuerySignInStatud call above yet.
				// Need to let them action in the main loop when they come in
				// ensure we've applied this player's settings
				//app.ApplyGameSettingsChanged(iPad);

				// go straight in to the trial level
				LoadTrial();
			}
		}
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}

	}	
	return 0;
}

int CScene_Main::MustSignInReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(result==C4JStorage::EMessage_ResultAccept) 
	{
		// we need to specify local game here to display local and LIVE profiles in the list
		switch(pClass->m_eAction)
		{
		case eAction_RunGame:
			ProfileManager.RequestSignInUI(false, true, false,false,true,&CScene_Main::CreateLoad_SignInReturned,pClass ,iPad);
			break;
		case eAction_RunLeaderboards:
			ProfileManager.RequestSignInUI(false, false, true,false,true, &CScene_Main::Leaderboards_SignInReturned, pClass,iPad);
			break;
		case eAction_RunAchievements:
			ProfileManager.RequestSignInUI(false, false, true,false,true,&CScene_Main::Achievements_SignInReturned,pClass,iPad );
			break;
		case eAction_RunHelpAndOptions:
			ProfileManager.RequestSignInUI(false, false, true,false,true,&CScene_Main::HelpAndOptions_SignInReturned,pClass,iPad );
			break;
		case eAction_RunUnlockOrDLC:
			ProfileManager.RequestSignInUI(false, false, true,false,true,&CScene_Main::UnlockFullGame_SignInReturned,pClass,iPad );
			break;

		}
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}
	}

	return 0;
}




int CScene_Main::AchievementsDeviceSelectReturned(void *pParam,bool bContinue)
{
	//CScene_Main* pClass = (CScene_Main*)pParam;
	//HRESULT hr;

	if(bContinue==true)
	{
		XShowAchievementsUI( ProfileManager.GetLockedProfile() );
	}

	return 0;
}


int CScene_Main::Leaderboards_SignInReturned(void *pParam,bool bContinue,int iPad)
{
	//CScene_Main* pClass = (CScene_Main*)pParam;

	if(bContinue==true)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_OK;
		// guests can't look at leaderboards
		if(ProfileManager.IsGuest(ProfileManager.GetPrimaryPad()))
		{
			StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
		}
		else if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
		{
			StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
		}
		else
		{
			ProfileManager.SetLockedProfile(ProfileManager.GetPrimaryPad());
			app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LeaderboardsMenu);
		}
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}

	}	
	return 0;
}

int CScene_Main::Achievements_SignInReturned(void *pParam,bool bContinue,int iPad)
{
	//CScene_Main* pClass = (CScene_Main*)pParam;

	if(bContinue==true)
	{
		XShowAchievementsUI( ProfileManager.GetPrimaryPad() );
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}

	}	
	return 0;
}
int CScene_Main::HelpAndOptions_SignInReturned(void *pParam,bool bContinue,int iPad)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(bContinue==true)
	{
		// 4J-PB - You can be offline and still can go into help and options
		if(app.GetTMSDLCInfoRead() || !ProfileManager.IsSignedInLive(iPad))
		{
			app.NavigateToScene(iPad,eUIScene_HelpAndOptionsMenu);
		}
		else
		{
			// Changing to async TMS calls
			app.SetTMSAction(iPad,eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions);

			// block all input
			pClass->m_bIgnorePress=true;
			// We want to hide everything in this scene and display a timer until we get a completion for the TMS files
			for(int i=0;i<BUTTONS_MAX;i++)
			{
				pClass->m_Buttons[i].SetShow(FALSE);
			}

			// turn off tooltips
			ui.SetTooltips(DEFAULT_XUI_MENU_USER, -1);

			pClass->m_Timer.SetShow(TRUE);
		}
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}
	}	

	return 0;
}

int CScene_Main::UnlockFullGame_SignInReturned(void *pParam,bool bContinue,int iPad)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	if(bContinue==true)
	{
		pClass->RunUnlockOrDLC(iPad);
	}
	else
	{
		// unlock the profile
		ProfileManager.SetLockedProfile(-1);
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			// if the user is valid, we should set the presence
			if(ProfileManager.IsSignedIn(i))
			{
				ProfileManager.SetCurrentGameActivity(i,CONTEXT_PRESENCE_MENUS,false);
			}
		}

	}	



	return 0;
}
int CScene_Main::SaveGameReturned(void *pParam,bool bContinue)
{
	//CScene_Main* pClass = (CScene_Main*)pParam;

	// display a saving complete message
	ProfileManager.SetLockedProfile(-1);
	return 0;
}

int CScene_Main::ExitGameReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	//CScene_Main* pClass = (CScene_Main*)pParam;

	// buttons reversed on this
	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		//XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, 0);
		app.ExitGame();
	}

	return 0;
}

void CScene_Main::LoadTrial(void)
{		
	app.SetTutorialMode( true );

	// clear out the app's terrain features list
	app.ClearTerrainFeaturePosition();

	StorageManager.ResetSaveData();

	// Need to set the mode as trial
	ProfileManager.StartTrialGame();

	// No saving in the trial
	StorageManager.SetSaveDisabled(true);

	StorageManager.SetSaveTitle(L"Tutorial");

	// Reset the autosave time
	app.SetAutosaveTimerTime();

	// not online for the trial game
	g_NetworkManager.HostGame(0,false,true,MINECRAFT_NET_MAX_PLAYERS,0);

	NetworkGameInitData *param = new NetworkGameInitData();
	param->seed = 0;
	param->saveData = nullptr;
	param->settings = app.GetGameHostOption( eGameHostOption_Tutorial );

	vector<LevelGenerationOptions *> *generators = app.getLevelGenerators();
	param->levelGen = generators->at(0);

	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
	loadingParams->lpParam = static_cast<LPVOID>(param);

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
	completionData->iPad = ProfileManager.GetPrimaryPad();
	loadingParams->completionData = completionData;

	CXuiSceneBase::ShowTrialTimer(TRUE);
	
	app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}



void CScene_Main::RunPlayGame(int iPad)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	app.ReleaseSaveThumbnail();

	if(ProfileManager.IsGuest(iPad))
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_OK;

		StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
	}
	else
	{
		ProfileManager.SetLockedProfile(iPad);

		// If the player was signed in before selecting play, we'll not have read the profile yet, so query the sign-in status to get this to happen
		ProfileManager.QuerySigninStatus();
		// 4J-PB - Need to check for installed DLC
		if(!app.DLCInstallProcessCompleted()) app.StartInstallDLCProcess(iPad);

		if(ProfileManager.IsFullVersion())
		{
			// are we offline?
			if(!ProfileManager.IsSignedInLive(iPad))
			{

				ProfileManager.DisplayOfflineProfile(&CScene_Main::CreateLoad_OfflineProfileReturned,this,iPad );
			}
			else
			{
				// Check if there is any new DLC
				app.ClearNewDLCAvailable();
				StorageManager.GetAvailableDLCCount(iPad);

				// check if all the TMS files are loaded
				if(app.GetTMSDLCInfoRead() && app.GetTMSXUIDsFileRead() && app.GetBanListRead(iPad))
				{
					if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,this)==true)
					{
						// change the minecraft player name
						pMinecraft->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));
						// save device already selected

						// ensure we've applied this player's settings
						app.ApplyGameSettingsChanged(iPad);
						// check for DLC
						// start timer to track DLC check finished
						m_Timer.SetShow(TRUE);
						XuiSetTimer(m_hObj,DLC_INSTALLED_TIMER_ID,DLC_INSTALLED_TIMER_TIME);
						//app.NavigateToScene(iPad,eUIScene_MultiGameJoinLoad);
					}
				}
				else
				{
					// Changing to async TMS calls

					// flag the timer to start the TMS calls when there is nothing happening with TMS.
					// fix for X360 - 162325 - TCR 001: BAS Game Stability: TU17: The game goes into an infinite loading upon entering the Play Game menu shortly after visiting the Minecraft Store
					if(app.GetTMSAction(iPad)!=eTMSAction_Idle)
					{
						XuiSetTimer(m_hObj,TMS_TIMER_ID,TMS_TIMER_TIME);
					}
					else
					{
						app.SetTMSAction(iPad,eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);
					}

					// block all input
					m_bIgnorePress=true;
					// We want to hide everything in this scene and display a timer until we get a completion for the TMS files
					for(int i=0;i<BUTTONS_MAX;i++)
					{
						m_Buttons[i].SetShow(FALSE);
					}

					// turn off tooltips
					ui.SetTooltips(DEFAULT_XUI_MENU_USER, -1);

					m_Timer.SetShow(TRUE);
				}
			}
		}
		else
		{
			// 4J-PB - if this is the trial game, we can't have any networking
			// go straight in to the trial level
			// change the minecraft player name
			Minecraft::GetInstance()->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));

			// Can't apply the player's settings here - they haven't come back from the QuerySignInStatud call above yet.
			// Need to let them action in the main loop when they come in
			// ensure we've applied this player's settings
			//app.ApplyGameSettingsChanged(iPad);
			LoadTrial();
		}
	}
}

HRESULT CScene_Main::OnTMSBanFileRetrieved()
{
	Minecraft *pMinecraft=Minecraft::GetInstance();
	int iPad=ProfileManager.GetLockedProfile();

	if(StorageManager.SetSaveDevice(&CScene_Main::DeviceSelectReturned,this)==true)
	{
		// change the minecraft player name
		pMinecraft->user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()));
		// save device already selected

		// ensure we've applied this player's settings
		app.ApplyGameSettingsChanged(iPad);
		// check for DLC
		// start timer to track DLC check finished
		m_Timer.SetShow(TRUE);
		XuiSetTimer(m_hObj,DLC_INSTALLED_TIMER_ID,DLC_INSTALLED_TIMER_TIME);
		//app.NavigateToScene(iPad,eUIScene_MultiGameJoinLoad);
	}
	return S_OK;
}

void CScene_Main::RunLeaderboards(int iPad)
{
	UINT uiIDA[1];
	uiIDA[0]=IDS_OK;

	// guests can't look at leaderboards
	if(ProfileManager.IsGuest(iPad))
	{
		StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
	}
	else if(!ProfileManager.IsSignedInLive(iPad))
	{
		StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
	}
	else
	{	
		ProfileManager.SetLockedProfile(iPad);
		// If the player was signed in before selecting play, we'll not have read the profile yet, so query the sign-in status to get this to happen
		ProfileManager.QuerySigninStatus();

		app.NavigateToScene(iPad, eUIScene_LeaderboardsMenu);
	}
}
void CScene_Main::RunAchievements(int iPad)
{
	UINT uiIDA[1];
	uiIDA[0]=IDS_OK;

	// guests can't look at achievements
	if(ProfileManager.IsGuest(iPad))
	{
		StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
	}
	else
	{
		XShowAchievementsUI( iPad );
	}
}
void CScene_Main::RunHelpAndOptions(int iPad)
{
	if(ProfileManager.IsGuest(iPad))
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_OK;
		StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
	}
	else
	{
		// If the player was signed in before selecting play, we'll not have read the profile yet, so query the sign-in status to get this to happen
		ProfileManager.QuerySigninStatus();

		// 4J-PB - You can be offline and still can go into help and options
		if(app.GetTMSDLCInfoRead() || !ProfileManager.IsSignedInLive(iPad))
		{
			app.NavigateToScene(iPad,eUIScene_HelpAndOptionsMenu);
		}
		else
		{
			// Changing to async TMS calls
			app.SetTMSAction(iPad,eTMSAction_TMSPP_RetrieveFiles_HelpAndOptions);

			// block all input
			m_bIgnorePress=true;
			// We want to hide everything in this scene and display a timer until we get a completion for the TMS files
			for(int i=0;i<BUTTONS_MAX;i++)
			{
				m_Buttons[i].SetShow(FALSE);
			}

			// turn off tooltips
			ui.SetTooltips(DEFAULT_XUI_MENU_USER, -1);

			m_Timer.SetShow(TRUE);
		}
	}
}

HRESULT CScene_Main::OnTMSDLCFileRetrieved( )
{
	m_Timer.SetShow(FALSE);
	switch(m_eAction)
	{
	case eAction_RunHelpAndOptions:
		app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_HelpAndOptionsMenu);
		break;
	case eAction_RunUnlockOrDLC:
		app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DLCMainMenu);
		break;
	}
	//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DLCMainMenu);

	return S_OK;
}

void CScene_Main::RunUnlockOrDLC(int iPad)
{
	UINT uiIDA[1];
	uiIDA[0]=IDS_OK;

	// Check if this means downloadable content
	if(ProfileManager.IsFullVersion())
	{
		// downloadable content
		if(ProfileManager.IsSignedInLive(iPad))
		{
			if(ProfileManager.IsGuest(iPad))
			{
				StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
			}
			else
			{
				// If the player was signed in before selecting play, we'll not have read the profile yet, so query the sign-in status to get this to happen
				ProfileManager.QuerySigninStatus();

				if(app.GetTMSDLCInfoRead())
				{
					app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DLCMainMenu);
				}
				else
				{
					// Changing to async TMS calls
					app.SetTMSAction(iPad,eTMSAction_TMSPP_RetrieveFiles_DLCMain);

					// block all input
					m_bIgnorePress=true;
					// We want to hide everything in this scene and display a timer until we get a completion for the TMS files
					for(int i=0;i<BUTTONS_MAX;i++)
					{
						m_Buttons[i].SetShow(FALSE);
					}

					// turn off tooltips
					ui.SetTooltips(DEFAULT_XUI_MENU_USER, -1);

					m_Timer.SetShow(TRUE);
				}

				// read the DLC info from TMS
				/*app.ReadDLCFileFromTMS(iPad);*/

				// We want to navigate to the DLC scene, but block input until we get the DLC file in from TMS
				// Don't navigate - we might have an uplink disconnect
				//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DLCMainMenu);

			}
		}
		else
		{
			StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
		}
	}
	else
	{
		// guests can't buy the game
		if(ProfileManager.IsGuest(iPad))
		{
			StorageManager.RequestMessageBox(IDS_UNLOCK_TITLE, IDS_UNLOCK_GUEST_TEXT, uiIDA, 1,iPad);
		}
		else if(!ProfileManager.IsSignedInLive(iPad))
		{
			StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
		}
		else
		{
			// If the player was signed in before selecting play, we'll not have read the profile yet, so query the sign-in status to get this to happen
			ProfileManager.QuerySigninStatus();

			TelemetryManager->RecordUpsellPresented(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);
			ProfileManager.DisplayFullVersionPurchase(false,iPad,eSen_UpsellID_Full_Version_Of_Game);
		}
	}
}

int CScene_Main::TMSReadFileListReturned(void *pParam,int iPad,C4JStorage::PTMSPP_FILE_LIST pTmsFileList)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	// push the file details in to a unordered map if they are not already in there
// 	for(int i=0;i<pTmsFileList->iCount;i++)
// 	{
// 		app.PutTMSPP_FileSize(filenametowstring(pTmsFileList->FileDetailsA[i].szFilename),pTmsFileList->FileDetailsA[i].iFileSize);
// 	}
	return 0;
}

int CScene_Main::TMSFileWriteReturned(void *pParam,int iPad,int iResult)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	// push the file details in to a unordered map if they are not already in there
	// 	for(int i=0;i<pTmsFileList->iCount;i++)
	// 	{
	// 		app.PutTMSPP_FileSize(filenametowstring(pTmsFileList->FileDetailsA[i].szFilename),pTmsFileList->FileDetailsA[i].iFileSize);
	// 	}
	return 0;
}

int CScene_Main::TMSFileReadReturned(void *pParam,int iPad,C4JStorage::PTMSPP_FILEDATA pData)
{
	CScene_Main* pClass = static_cast<CScene_Main *>(pParam);

	// push the file details in to a unordered map if they are not already in there
	// 	for(int i=0;i<pTmsFileList->iCount;i++)
	// 	{
	// 		app.PutTMSPP_FileSize(filenametowstring(pTmsFileList->FileDetailsA[i].szFilename),pTmsFileList->FileDetailsA[i].iFileSize);
	// 	}
	return 0;
}

HRESULT CScene_Main::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// 4J-PB - TODO - Don't think we can do this - if a 2nd player signs in here with an offline profile, the signed in LIVE player gets re-logged in, and bMultiplayerAllowed is false briefly 
	if( pTimer->nId == DLC_INSTALLED_TIMER_ID)
	{
		if(!app.DLCInstallPending())
		{
			XuiKillTimer(m_hObj,DLC_INSTALLED_TIMER_ID);
			m_Timer.SetShow(FALSE);
			app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LoadOrJoinMenu);
		}
	}

	else if( pTimer->nId == TMS_TIMER_ID)
	{
		if(app.GetTMSAction(ProfileManager.GetPrimaryPad())==eTMSAction_Idle)
		{
			app.SetTMSAction(ProfileManager.GetPrimaryPad(),eTMSAction_TMSPP_RetrieveFiles_RunPlayGame);
			XuiKillTimer(m_hObj,TMS_TIMER_ID);
		}
	}

	return S_OK;
}