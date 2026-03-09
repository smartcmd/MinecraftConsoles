#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsGraphicsMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\GameRenderer.h"

int UIScene_SettingsGraphicsMenu::LevelToDistance(int level)
{
	static const int table[6] = {2,4,8,16,32,64};
	if(level < 0) level = 0;
	if(level > 5) level = 5;
	return table[level];
}

int UIScene_SettingsGraphicsMenu::DistanceToLevel(int dist)
{
    static const int table[6] = {2,4,8,16,32,64};
    for(int i = 0; i < 6; i++){
        if(table[i] == dist)
            return i;
    }
    return 3;
}

UIScene_SettingsGraphicsMenu::UIScene_SettingsGraphicsMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	Minecraft* pMinecraft = Minecraft::GetInstance();
	
	m_bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	m_checkboxClouds.init(app.GetString(IDS_CHECKBOX_RENDER_CLOUDS),eControl_Clouds,(app.GetGameSettings(m_iPad,eGameSetting_Clouds)!=0));
	m_checkboxBedrockFog.init(app.GetString(IDS_CHECKBOX_RENDER_BEDROCKFOG),eControl_BedrockFog,(app.GetGameSettings(m_iPad,eGameSetting_BedrockFog)!=0));
	m_checkboxCustomSkinAnim.init(app.GetString(IDS_CHECKBOX_CUSTOM_SKIN_ANIM),eControl_CustomSkinAnim,(app.GetGameSettings(m_iPad,eGameSetting_CustomSkinAnim)!=0));


	WCHAR TempString[256];

	swprintf(TempString, 256, L"Render Distance: %d",app.GetGameSettings(m_iPad,eGameSetting_RenderDistance));	
	m_sliderRenderDistance.init(TempString,eControl_RenderDistance,0,5,DistanceToLevel(app.GetGameSettings(m_iPad,eGameSetting_RenderDistance)));
	
	swprintf( TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_GAMMA ),app.GetGameSettings(m_iPad,eGameSetting_Gamma));	
	m_sliderGamma.init(TempString,eControl_Gamma,0,100,app.GetGameSettings(m_iPad,eGameSetting_Gamma));

	if ((int)(30.0f + (app.GetGameSettings(m_iPad, eGameSetting_FOV) - 101) * 80.0f / 100.0f) == 70)
	{
		swprintf((WCHAR*)TempString, 256, L"FOV: Default");
	}
	else
	{
		swprintf((WCHAR*)TempString, 256, L"FOV: %d", (int)(30.0f + (app.GetGameSettings(m_iPad, eGameSetting_FOV) - 101) * 80.0f / 100.0f));
	}
	m_sliderFOV.init(TempString, eControl_FOV, 0, 80, ((app.GetGameSettings(m_iPad, eGameSetting_FOV)) - 101) * 80.0f / 100.0f);
	
	swprintf( TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_INTERFACEOPACITY ),app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity));	
	m_sliderInterfaceOpacity.init(TempString,eControl_InterfaceOpacity,0,100,app.GetGameSettings(m_iPad,eGameSetting_InterfaceOpacity));

	doHorizontalResizeCheck();

	const bool bInGame=(Minecraft::GetInstance()->level!=nullptr);
	const bool bIsPrimaryPad=(ProfileManager.GetPrimaryPad()==m_iPad);
	// if we're not in the game, we need to use basescene 0 
	if(bInGame)
	{
		// If the game has started, then you need to be the host to change the in-game gamertags
		if(bIsPrimaryPad)
		{	
			// we are the primary player on this machine, but not the game host
			// are we the game host? If not, we need to remove the bedrockfog setting
			if(!g_NetworkManager.IsHost())
			{
				// hide the in-game bedrock fog setting
				removeControl(&m_checkboxBedrockFog, true);
			}
		}
		else
		{
			// We shouldn't have the bedrock fog option, or the m_CustomSkinAnim option
			removeControl(&m_checkboxBedrockFog, true);
			removeControl(&m_checkboxCustomSkinAnim, true);
		}
	}

	if(app.GetLocalPlayerCount()>1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
#endif
	}
}

UIScene_SettingsGraphicsMenu::~UIScene_SettingsGraphicsMenu()
{
}

wstring UIScene_SettingsGraphicsMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SettingsGraphicsMenuSplit";
	}
	else
	{
		return L"SettingsGraphicsMenu";
	}
}

void UIScene_SettingsGraphicsMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsGraphicsMenu::updateComponents()
{
	const bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
	
		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);

	}
}

void UIScene_SettingsGraphicsMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			// check the checkboxes
			app.SetGameSettings(m_iPad,eGameSetting_Clouds,m_checkboxClouds.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_BedrockFog,m_checkboxBedrockFog.IsChecked()?1:0);
			app.SetGameSettings(m_iPad,eGameSetting_CustomSkinAnim,m_checkboxCustomSkinAnim.IsChecked()?1:0);

			navigateBack();
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
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

void UIScene_SettingsGraphicsMenu::handleSliderMove(F64 sliderId, F64 currentValue)
{
	WCHAR TempString[256];
	const int value = static_cast<int>(currentValue);
	switch(static_cast<int>(sliderId))
	{
	case eControl_RenderDistance:
		{
			m_sliderRenderDistance.handleSliderMove(value);

			const int dist = LevelToDistance(value);

			app.SetGameSettings(m_iPad,eGameSetting_RenderDistance,dist);

			const Minecraft* mc = Minecraft::GetInstance();
			mc->options->viewDistance = 3 - value;
			swprintf(TempString,256,L"Render Distance: %d",dist);
			m_sliderRenderDistance.setLabel(TempString);
		}
		break;

	case eControl_Gamma:
		m_sliderGamma.handleSliderMove(value);
		
		app.SetGameSettings(m_iPad,eGameSetting_Gamma,value);
		swprintf( TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_GAMMA ),value);
		m_sliderGamma.setLabel(TempString);

		break;

	case eControl_FOV:
		{ 

			// jvnpr -- code in Consoles_App.cpp should always reflect the same calculations as here so that controller and mouse inputs work the same.
			int v = (int)currentValue;
			m_sliderFOV.handleSliderMove(v);
			Minecraft *pMinecraft = Minecraft::GetInstance();
			if (v < 0) v = 0;
			if (v > 80) v = 80;
			int displayFOV = v + 30; // jvnpr -- convert 0-80 to 30-110

			pMinecraft->options->fov = (v / 40.0f) - 1; // jvnpr -- use FOV option instead of hooking into gamerenderer (range from -1 to 1 for 30-110)
			WCHAR TempString[256];
			
			if (v + 30 == 70) {
				swprintf((WCHAR*)TempString, 256, L"FOV: Default", displayFOV); // when fov is 70 display "FOV: Default (90)"
			}
			else {
				swprintf((WCHAR*)TempString, 256, L"FOV: %d", displayFOV);
			}
			
			m_sliderFOV.setLabel(TempString);
			
			app.SetGameSettings(m_iPad, eGameSetting_FOV, (v / ( 80.0f / 100.0f )) + 101); // offset from storing as 0-100 to 101-201 to detect whether we use old system and convert
		}
		break;

	case eControl_InterfaceOpacity:
		m_sliderInterfaceOpacity.handleSliderMove(value);
		
		app.SetGameSettings(m_iPad,eGameSetting_InterfaceOpacity,value);
		swprintf( TempString, 256, L"%ls: %d%%", app.GetString( IDS_SLIDER_INTERFACEOPACITY ),value);	
		m_sliderInterfaceOpacity.setLabel(TempString);

		break;
	}
}
