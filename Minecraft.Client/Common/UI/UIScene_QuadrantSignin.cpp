#include "stdafx.h"
#include "UI.h"
#include "UIScene_QuadrantSignin.h"
#include "..\..\Minecraft.h"
#if defined(__ORBIS__)
#include "Common\Network\Sony\SonyHttp.h"
#endif

UIScene_QuadrantSignin::UIScene_QuadrantSignin(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_signInInfo = *static_cast<SignInInfo *>(_initData);

	m_bIgnoreInput = false;
	
	m_lastRequestedAvatar = -1;
	
	_initQuadrants();

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(InputManager.IsCircleCrossSwapped())
	{
		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = true;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetABSwap , 1 , value );
	}
#endif

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);
}

UIScene_QuadrantSignin::~UIScene_QuadrantSignin()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

wstring UIScene_QuadrantSignin::getMoviePath()
{
	return L"QuadrantSignin";
}

void UIScene_QuadrantSignin::updateTooltips()
{
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_CONTINUE, IDS_TOOLTIPS_CANCEL);
}

// Returns true if this scene has focus for the pad passed in
bool UIScene_QuadrantSignin::hasFocus(int iPad)
{
	// Allow input from any controller
	return bHasFocus;
}

bool UIScene_QuadrantSignin::hidesLowerScenes()
{
	// This is a Modal dialog, so don't need to hide the scene behind
	return false;
}

void UIScene_QuadrantSignin::tick()
{
	if(!getMovie()) return;

	UIScene::tick();

	updateState();
}

void UIScene_QuadrantSignin::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	app.DebugPrintf("UIScene_QuadrantSignin handling input for pad %d, key %d, repeat- %s, pressed- %s, released- %s\n", iPad, key, repeat?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");

	if(!m_bIgnoreInput)
	{
		ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

		switch(key)
		{
		case ACTION_MENU_CANCEL:
			{
				if(pressed)
				{
#ifdef _XBOX_ONE
					if(InputManager.IsPadLocked(iPad))
					{
						if(iPad != ProfileManager.GetPrimaryPad())
						{
							ProfileManager.RemoveGamepadFromGame(iPad);
						}
						else
#endif
						{
							m_bIgnoreInput = true;
							m_signInInfo.Func(m_signInInfo.lpParam,false,iPad);
							ProfileManager.CancelProfileAvatarRequest();

							navigateBack();
						}
					}
#ifdef _XBOX_ONE
				}
#endif
			}
			break;
		case ACTION_MENU_OK:
#ifdef __ORBIS__
		case ACTION_MENU_TOUCHPAD_PRESS:
#endif
			if(pressed)
			{
				m_bIgnoreInput = true;
#ifdef _XBOX_ONE
				if(ProfileManager.IsSignedIn(iPad)&&InputManager.IsPadLocked(iPad))
#else
				if(ProfileManager.IsSignedIn(iPad))
#endif
				{
					app.DebugPrintf("Signed in pad pressed\n");
					ProfileManager.CancelProfileAvatarRequest();

#ifdef _XBOX_ONE
					// On Durango, if we don't navigate forward here, then when we are on the main menu, it (re)gains focus & that causes our users to get cleared
					ui.NavigateToScene(m_iPad, eUIScene_Timer);
#endif
					navigateBack();
					m_signInInfo.Func(m_signInInfo.lpParam,true,m_iPad);
				}
				else
				{
#ifdef _XBOX_ONE
					if(ProfileManager.IsSignedIn(0)&&!InputManager.IsPadLocked(0))
					{
						app.DebugPrintf("Signed in pad with no controller bound pressed\n");
						ProfileManager.RequestSignInUI(false, false, false, true, false,&UIScene_QuadrantSignin::SignInReturned, this, iPad);
					}
					else
#endif
					{
						app.DebugPrintf("Non-signed in pad pressed\n");
						ProfileManager.RequestSignInUI(false, false, false, true, true,&UIScene_QuadrantSignin::SignInReturned, this, iPad);
					}
				}
			}
			break;
		case ACTION_MENU_UP:
		case ACTION_MENU_DOWN:
			if(pressed)
			{
				sendInputToMovie(key, repeat, pressed, released);
			}
			break;
		}
	}

	handled = true;
}

#ifdef _XBOX_ONE
int UIScene_QuadrantSignin::SignInReturned(void *pParam,bool bContinue, int iPad, int iController)
#else
int UIScene_QuadrantSignin::SignInReturned(void *pParam,bool bContinue, int iPad)
#endif
{
	app.DebugPrintf("SignInReturned for pad %d\n", iPad);

	UIScene_QuadrantSignin *pClass = static_cast<UIScene_QuadrantSignin *>(pParam);

#ifdef _XBOX_ONE
	if(bContinue && pClass->m_signInInfo.requireOnline && ProfileManager.IsSignedIn(iPad))
	{
		if( !InputManager.IsPadLocked(iPad) )
		{
			ProfileManager.ForcePrimaryPadController(iController);
		}
		ProfileManager.CheckMultiplayerPrivileges(iPad, true, &checkAllPrivilegesCallback, pClass);
	}
	else
#endif
	{
		pClass->m_bIgnoreInput = false;
		pClass->updateState();
	}

	return 0;
}

#ifdef _XBOX_ONE
void UIScene_QuadrantSignin::checkAllPrivilegesCallback(LPVOID lpParam, bool hasPrivileges, int iPad)
{
	UIScene_QuadrantSignin* pClass = (UIScene_QuadrantSignin*)lpParam;

	if(!hasPrivileges)
	{
		ProfileManager.RemoveGamepadFromGame(iPad);
	}
	pClass->m_bIgnoreInput = false;
	pClass->updateState();
}
#endif

void UIScene_QuadrantSignin::updateState()
{
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if(ProfileManager.IsSignedIn(i) && InputManager.IsPadConnected(i))
		{
			//app.DebugPrintf("Index %d is signed in, display name - '%s'\n", i, ProfileManager.GetDisplayName(i).data());

#ifdef _XBOX_ONE
			if(!InputManager.IsPadLocked(i))
			{
				setControllerState(i, eControllerStatus_PressToJoin_LoggedIn);
			}
			else
#endif
			{
				setControllerState(i, eControllerStatus_PlayerDetails);
			}

			m_labelDisplayName[i].setLabel(ProfileManager.GetDisplayName(i));
			//m_buttonControllers[i].setLabel(app.GetString(IDS_TOOLTIPS_CONTINUE),i);

			if(!m_iconRequested[i])
			{
				app.DebugPrintf(app.USER_SR, "Requesting avatar for %d\n", i);
				if(ProfileManager.GetProfileAvatar(i, &UIScene_QuadrantSignin::AvatarReturned, this))
				{
					m_iconRequested[i] = true;
					m_lastRequestedAvatar = i;
				}
			}
		}
		else if(InputManager.IsPadConnected(i))
		{
			//app.DebugPrintf("Index %d is not signed in\n", i);

			setControllerState(i, eControllerStatus_PressToJoin);
			m_labelDisplayName[i].setLabel(L"");
			m_iconRequested[i] = false;
		}
		else
		{
			//app.DebugPrintf("Index %d is not connected\n", i);

			setControllerState(i, eControllerStatus_ConnectController);
			m_iconRequested[i] = false;
		}
	}
}

void UIScene_QuadrantSignin::setControllerState(int iPad, EControllerStatus state)
{
	if(m_controllerStatus[iPad] != state)
	{
		m_controllerStatus[iPad] = state;

		IggyDataValue result;
		IggyDataValue value[2];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = iPad;

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = static_cast<int>(state);

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetControllerStatus , 2 , value );
	}
}

int UIScene_QuadrantSignin::AvatarReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes)
{
	UIScene_QuadrantSignin *pClass = static_cast<UIScene_QuadrantSignin *>(lpParam);
	app.DebugPrintf(app.USER_SR,"AvatarReturned callback\n");
	if(pbThumbnail != nullptr)
	{
		// 4J-JEV - Added to ensure each new texture gets a unique name.
		static unsigned int quadrantImageCount = 0;

		wchar_t iconName[32];
		swprintf(iconName,32,L"quadrantImage%05d",quadrantImageCount++);
		
		pClass->registerSubstitutionTexture(iconName,pbThumbnail,dwThumbnailBytes,true);
		pClass->m_bitmapIcon[pClass->m_lastRequestedAvatar].setTextureName(iconName);
	}

	pClass->m_lastRequestedAvatar = -1;

	return 0;
}

void UIScene_QuadrantSignin::_initQuadrants()
{
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		m_iconRequested[i] = false;

		m_labelPressToJoin[i].init(IDS_MUST_SIGN_IN_TITLE);
		m_labelConnectController[i].init(L"");
		m_labelAccountType[i].init(L"");

		m_controllerStatus[i] = eControllerStatus_ConnectController;

		if(ProfileManager.IsSignedIn(i))
		{
			app.DebugPrintf("Index %d is signed in\n", i);

#ifdef _XBOX_ONE
			if(!InputManager.IsPadLocked(i))
			{
				setControllerState(i, eControllerStatus_PressToJoin_LoggedIn);
			}
			else
#endif
			{
				setControllerState(i, eControllerStatus_PlayerDetails);
			}

			m_labelDisplayName[i].init(ProfileManager.GetDisplayName(i));
		}
		else if(InputManager.IsPadConnected(i))
		{
			app.DebugPrintf("Index %d is not signed in\n", i);

			setControllerState(i, eControllerStatus_PressToJoin);
			m_labelDisplayName[i].init(L"");
		}
		else
		{
			app.DebugPrintf("Index %d is not connected\n", i);

			setControllerState(i, eControllerStatus_ConnectController);
		}
	}
}

void UIScene_QuadrantSignin::handleReload()
{
	_initQuadrants();
}