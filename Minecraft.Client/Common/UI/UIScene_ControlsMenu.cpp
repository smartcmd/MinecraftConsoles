#include "stdafx.h"
#include "UI.h"
#include "UIScene_ControlsMenu.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"

UIScene_ControlsMenu::UIScene_ControlsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
#if defined(_XBOX) || defined(_WIN64)
	value[0].number = (F64)0;
#elif defined(_DURANGO)
	value[0].number = (F64)1;
#elif defined(__PS3__)
	value[0].number = (F64)2;
#elif defined(__ORBIS__)
	value[0].number = (F64)3;
#elif defined(__PSVITA__)
	value[0].number = (F64)4;
#endif
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlatform , 1 , value );

	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	if(bNotInGame)
	{
		LPWSTR layoutString = new wchar_t[ 128 ];
		swprintf( layoutString, 128, L"%ls", VER_PRODUCTVERSION_STR_W);	
		m_labelVersion.init(layoutString);
		delete [] layoutString;
	}
	// 4J-PB - stop the label showing in the in-game controls menu
	else
	{
		m_labelVersion.init(L" ");
	}
	m_bCreativeMode = !bNotInGame && Minecraft::GetInstance()->localplayers[m_iPad] && Minecraft::GetInstance()->localplayers[m_iPad]->abilities.mayfly;

#ifndef __PSVITA__
#ifdef __ORBIS__
	// no buttons to initialise if we're running this on PS4 remote play
	if(!InputManager.UsingRemoteVita())
#endif
	{
		m_buttonLayouts[0].init(L"1", eControl_Button0);
		m_buttonLayouts[1].init(L"2", eControl_Button1);
		m_buttonLayouts[2].init(L"3", eControl_Button2);
	}
#endif

	m_checkboxInvert.init(app.GetString(IDS_INVERT_LOOK), eControl_InvertLook, app.GetGameSettings(m_iPad,eGameSetting_ControlInvertLook));
	m_checkboxSouthpaw.init(app.GetString(IDS_SOUTHPAW), eControl_Southpaw, app.GetGameSettings(m_iPad,eGameSetting_ControlSouthPaw));

	m_iSchemeTextA[0]=IDS_CONTROLS_SCHEME0;
	m_iSchemeTextA[1]=IDS_CONTROLS_SCHEME1;
	m_iSchemeTextA[2]=IDS_CONTROLS_SCHEME2;

	int iSelected=app.GetGameSettings(m_iPad,eGameSetting_ControlScheme);

#ifndef __PSVITA__
	LPWSTR layoutString = new wchar_t[ 128 ];
	swprintf( layoutString, 128, L"%ls : %ls", app.GetString( IDS_CURRENT_LAYOUT ),app.GetString(m_iSchemeTextA[iSelected]));
#ifdef __ORBIS__
	if (!InputManager.UsingRemoteVita())
#endif
	{
		m_labelCurrentLayout.init(layoutString);
	}
#endif

	m_iCurrentNavigatedControlsLayout = iSelected;


#ifdef __ORBIS__
	// don't set controller layout if we're entering the PS4 remote play scene
	if(!InputManager.UsingRemoteVita())
#endif
	{
		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = (F64)m_iCurrentNavigatedControlsLayout;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetControllerLayout , 1 , value );
	}

#ifdef __ORBIS__
	// Set mapping to Vita mapping
	if (InputManager.UsingRemoteVita()) m_iCurrentNavigatedControlsLayout = 3;
#elif defined __PSVITA__
	// Set mapping to Vita mapping
	if (InputManager.IsVitaTV()) m_iCurrentNavigatedControlsLayout = 1;
#endif

	for(unsigned int i = 0; i < e_PadCOUNT; ++i)
	{
		m_labelsPad[i].init(L"");
		m_controlLines[i].setVisible(false);
	}
	m_bLayoutChanged = false;


	PositionAllText(m_iPad);
}

wstring UIScene_ControlsMenu::getMoviePath()
{
#ifdef __ORBIS__
	if(InputManager.UsingRemoteVita())
	{
		return L"ControlsRemotePlay";
	}
	else
#endif
#ifdef __PSVITA__
	if(InputManager.IsVitaTV())
	{
		return L"ControlsTV";
	}
	else
#endif
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"ControlsSplit";
	}
	else
	{
		return L"Controls";
	}
}

void UIScene_ControlsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_ControlsMenu::tick()
{
	if(m_bLayoutChanged) PositionAllText(m_iPad);
	UIScene::tick();
}

void UIScene_ControlsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			app.CheckGameSettingsChanged(true,iPad);
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if( pressed )
		{
			//CD - Added for audio
			ui.PlayUISFX(eSFX_Press);
		}
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_ControlsMenu::handleCheckboxToggled(F64 controlId, bool selected)
{
	switch((int)controlId)
	{
	case eControl_InvertLook:
		app.SetGameSettings(m_iPad,eGameSetting_ControlInvertLook,(unsigned char)( selected ) );
		break;
	case eControl_Southpaw:
		app.SetGameSettings(m_iPad,eGameSetting_ControlSouthPaw,(unsigned char)( selected ) );
		PositionAllText(m_iPad);
		break;
	};
}

void UIScene_ControlsMenu::handlePress(F64 controlId, F64 childId)
{
	int control = (int)controlId;
	switch(control)
	{
	case eControl_Button0:
	case eControl_Button1:
	case eControl_Button2:
		app.SetGameSettings(m_iPad,eGameSetting_ControlScheme,(unsigned char)control);
		LPWSTR layoutString = new wchar_t[ 128 ];
		swprintf( layoutString, 128, L"%ls : %ls", app.GetString( IDS_CURRENT_LAYOUT ),app.GetString(m_iSchemeTextA[control]));	
#ifdef __ORBIS__
		if (!InputManager.UsingRemoteVita())
#endif
		{
			m_labelCurrentLayout.setLabel(layoutString);
		}

		break;
	};
}

void UIScene_ControlsMenu::handleFocusChange(F64 controlId, F64 childId)
{
	int control = (int)controlId;
	switch(control)
	{
	case eControl_Button0:
	case eControl_Button1:
	case eControl_Button2:
		m_iCurrentNavigatedControlsLayout=control;
		m_bLayoutChanged = true;
		break;
	};
}

void UIScene_ControlsMenu::PositionAllText(int iPad)
{
	for(unsigned int i = 0; i < e_PadCOUNT; ++i)
	{
		m_labelsPad[i].setLabel(L"");
		m_controlLines[i].setVisible(false);
	}

	if(m_bCreativeMode)
	{
		PositionText(iPad,IDS_CONTROLS_JUMPFLY,MINECRAFT_ACTION_JUMP);
	}
	else
	{
		PositionText(iPad,IDS_CONTROLS_JUMP,MINECRAFT_ACTION_JUMP);
	}
	PositionText(iPad,IDS_CONTROLS_INVENTORY,MINECRAFT_ACTION_INVENTORY);
	PositionText(iPad,IDS_CONTROLS_PAUSE,MINECRAFT_ACTION_PAUSEMENU);
	if(m_bCreativeMode)
	{
		PositionText(iPad,IDS_CONTROLS_SNEAKFLY,MINECRAFT_ACTION_SNEAK_TOGGLE);
	}
	else
	{
		PositionText(iPad,IDS_CONTROLS_SNEAK,MINECRAFT_ACTION_SNEAK_TOGGLE);
	}
	PositionText(iPad,IDS_CONTROLS_USE,MINECRAFT_ACTION_USE);
	PositionText(iPad,IDS_CONTROLS_ACTION,MINECRAFT_ACTION_ACTION);
	PositionText(iPad,IDS_CONTROLS_HELDITEM,MINECRAFT_ACTION_RIGHT_SCROLL);
	PositionText(iPad,IDS_CONTROLS_HELDITEM,MINECRAFT_ACTION_LEFT_SCROLL);
	PositionText(iPad,IDS_CONTROLS_DROP,MINECRAFT_ACTION_DROP);
	PositionText(iPad,IDS_CONTROLS_CRAFTING,MINECRAFT_ACTION_CRAFTING);
	PositionText(iPad,IDS_CONTROLS_THIRDPERSON,MINECRAFT_ACTION_RENDER_THIRD_PERSON);
	PositionText(iPad,IDS_CONTROLS_PLAYERS,MINECRAFT_ACTION_GAME_INFO);

	// Swap for southpaw.
	if ( app.GetGameSettings(m_iPad,eGameSetting_ControlSouthPaw) )
	{
		// Move
		PositionText(iPad,IDS_CONTROLS_LOOK,MINECRAFT_ACTION_RIGHT);
		// Look
		PositionText(iPad,IDS_CONTROLS_MOVE,MINECRAFT_ACTION_LOOK_RIGHT);
	}
	else // Normal right handed.
	{
		// Move
		PositionText(iPad,IDS_CONTROLS_MOVE,MINECRAFT_ACTION_RIGHT);
		// Look
		PositionText(iPad,IDS_CONTROLS_LOOK,MINECRAFT_ACTION_LOOK_RIGHT);
	}

	bool layoutHasDpadFly;
#ifdef __PSVITA__
	layoutHasDpadFly = m_iCurrentNavigatedControlsLayout == 1;
#else
	layoutHasDpadFly = m_iCurrentNavigatedControlsLayout == 0;
#endif

	// If we're in controls mode 1, and creative mode show the dpad for Creative Mode
	if(m_bCreativeMode && layoutHasDpadFly)
	{
		PositionText(iPad,IDS_CONTROLS_DPAD,MINECRAFT_ACTION_DPAD_LEFT);
	}
	m_bLayoutChanged = false;
}

void UIScene_ControlsMenu::PositionText(int iPad,int iTextID, unsigned char ucAction)
{
	unsigned int uiVal = InputManager.GetGameJoypadMaps(m_iCurrentNavigatedControlsLayout, ucAction);

	if (uiVal & _360_JOY_BUTTON_A) PositionTextDirect(iPad, iTextID, e_PadA, true);
	if (uiVal & _360_JOY_BUTTON_B) PositionTextDirect(iPad, iTextID, e_PadB, true);
	if (uiVal & _360_JOY_BUTTON_X) PositionTextDirect(iPad, iTextID, e_PadX, true);
	if (uiVal & _360_JOY_BUTTON_Y) PositionTextDirect(iPad, iTextID, e_PadY, true);
	if (uiVal & _360_JOY_BUTTON_BACK)
	{
#ifdef __ORBIS__
		PositionTextDirect(iPad, iTextID, (InputManager.UsingRemoteVita() ? e_PadTouch : e_PadBack), true);
#else
		PositionTextDirect(iPad, iTextID, e_PadBack, true);
#endif
	}
	if (uiVal & _360_JOY_BUTTON_START) PositionTextDirect(iPad, iTextID, e_PadStart, true);
	if (uiVal & _360_JOY_BUTTON_RB) PositionTextDirect(iPad, iTextID, e_PadRB, true);
	if (uiVal & _360_JOY_BUTTON_LB) PositionTextDirect(iPad, iTextID, e_PadLB, true);
	if (uiVal & _360_JOY_BUTTON_RTHUMB) PositionTextDirect(iPad, iTextID, e_PadRS_1, true);
	if (uiVal & _360_JOY_BUTTON_LTHUMB) PositionTextDirect(iPad, iTextID, e_PadLS_1, true);
		// Look
	if (uiVal & _360_JOY_BUTTON_RSTICK_RIGHT) PositionTextDirect(iPad, iTextID, e_PadRS_2, true);
		// Move
	if (uiVal & _360_JOY_BUTTON_LSTICK_RIGHT) PositionTextDirect(iPad, iTextID, e_PadLS_2, true);
	if (uiVal & _360_JOY_BUTTON_RT) PositionTextDirect(iPad, iTextID, e_PadRT, true);
	if (uiVal & _360_JOY_BUTTON_LT) PositionTextDirect(iPad, iTextID, e_PadLT, true);
	if (uiVal & _360_JOY_BUTTON_DPAD_RIGHT) PositionTextDirect(iPad, iTextID, e_PadDPadRight, true);
	if (uiVal & _360_JOY_BUTTON_DPAD_LEFT) PositionTextDirect(iPad, iTextID, e_PadDPadLeft, true);
	if (uiVal & _360_JOY_BUTTON_DPAD_UP) PositionTextDirect(iPad, iTextID, e_PadDPadUp, true);
	if (uiVal & _360_JOY_BUTTON_DPAD_DOWN) PositionTextDirect(iPad, iTextID, e_PadDPadDown, true);
	}

void UIScene_ControlsMenu::PositionTextDirect(int iPad,int iTextID, int iControlDetailsIndex, bool bShow)
{
	LPCWSTR text = app.GetString(iTextID);

	m_labelsPad[iControlDetailsIndex].setLabel(text);
	m_controlLines[iControlDetailsIndex].setVisible(bShow);
}