#include "stdafx.h"
#include "UI.h"
#include "UIScene_SettingsGraphicsMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"
#include "..\..\GameRenderer.h"

namespace
{
	const int FOV_MIN = 70;
	const int FOV_MAX = 110;
	const int FOV_SLIDER_MAX = 100;

	const int fpsCaps[] = {30, 60, 120, 0};

	int clampFov(int value)
	{
		if (value < FOV_MIN) return FOV_MIN;
		if (value > FOV_MAX) return FOV_MAX;
		return value;
	}

	int fovToSliderValue(float fov)
	{
		int clampedFov = clampFov((int)(fov + 0.5f));
		return ((clampedFov - FOV_MIN) * FOV_SLIDER_MAX) / (FOV_MAX - FOV_MIN);
	}

	int sliderValueToFov(int sliderValue)
	{
		if (sliderValue < 0) sliderValue = 0;
		if (sliderValue > FOV_SLIDER_MAX) sliderValue = FOV_SLIDER_MAX;
		return FOV_MIN + ((sliderValue * (FOV_MAX - FOV_MIN)) / FOV_SLIDER_MAX);
	}

	void formatFpsCapLabel(WCHAR* buf, int idx)
	{
		if (fpsCaps[idx] == 0)
			swprintf(buf, 256, L"Max Framerate: Unlimited");
		else
			swprintf(buf, 256, L"Max Framerate: %d", fpsCaps[idx]);
	}
}

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
	initialiseMovie();
	Minecraft* pMinecraft = Minecraft::GetInstance();
	
	m_bNotInGame = (Minecraft::GetInstance()->level == NULL);

	m_checkboxClouds.init(app.GetString(IDS_CHECKBOX_RENDER_CLOUDS), eControl_Clouds, (app.GetGameSettings(m_iPad, eGameSetting_Clouds) != 0));
	m_checkboxBedrockFog.init(app.GetString(IDS_CHECKBOX_RENDER_BEDROCKFOG), eControl_BedrockFog, (app.GetGameSettings(m_iPad, eGameSetting_BedrockFog) != 0));
	m_checkboxCustomSkinAnim.init(app.GetString(IDS_CHECKBOX_CUSTOM_SKIN_ANIM), eControl_CustomSkinAnim, (app.GetGameSettings(m_iPad, eGameSetting_CustomSkinAnim) != 0));
	m_checkboxVSync.init(L"VSync", eControl_VSync, (app.GetGameSettings(m_iPad, eGameSetting_VSync) != 0));

	WCHAR TempString[256];

	swprintf(TempString, 256, L"Render Distance: %d", (int)app.GetGameSettings(m_iPad, eGameSetting_RenderDistance));
	m_sliderRenderDistance.init(TempString, eControl_RenderDistance, 0, 5, DistanceToLevel((int)app.GetGameSettings(m_iPad, eGameSetting_RenderDistance)));

	swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_GAMMA), (int)app.GetGameSettings(m_iPad, eGameSetting_Gamma));
	m_sliderGamma.init(TempString, eControl_Gamma, 0, 100, (int)app.GetGameSettings(m_iPad, eGameSetting_Gamma));

	int initialFovSlider = (int)app.GetGameSettings(m_iPad, eGameSetting_FOV);
	swprintf(TempString, 256, L"FOV: %d", sliderValueToFov(initialFovSlider));
	m_sliderFOV.init(TempString, eControl_FOV, 0, FOV_SLIDER_MAX, initialFovSlider);

	swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_INTERFACEOPACITY), (int)app.GetGameSettings(m_iPad, eGameSetting_InterfaceOpacity));
	m_sliderInterfaceOpacity.init(TempString, eControl_InterfaceOpacity, 0, 100, (int)app.GetGameSettings(m_iPad, eGameSetting_InterfaceOpacity));

	int fpsCapIndex = (int)app.GetGameSettings(m_iPad, eGameSetting_FpsCap);
	if (fpsCapIndex < 0 || fpsCapIndex > 3) fpsCapIndex = 1;
	formatFpsCapLabel(TempString, fpsCapIndex);
	m_sliderFpsCap.init(TempString, eControl_FpsCap, 0, 3, fpsCapIndex);

	doHorizontalResizeCheck();
	
	bool bInGame = (Minecraft::GetInstance()->level != NULL);
	bool bIsPrimaryPad = (ProfileManager.GetPrimaryPad() == m_iPad);

	if(bInGame)
	{
		if(bIsPrimaryPad)
		{	
			if(!g_NetworkManager.IsHost())
			{
				removeControl(&m_checkboxBedrockFog, true);
			}
		}
		else
		{
			removeControl(&m_checkboxBedrockFog, true);
			removeControl(&m_checkboxCustomSkinAnim, true);
		}
	}

	if(app.GetLocalPlayerCount() > 1)
	{
#if TO_BE_IMPLEMENTED
		app.AdjustSplitscreenScene(m_hObj, &m_OriginalPosition, m_iPad);
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
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);
}

void UIScene_SettingsGraphicsMenu::updateComponents()
{
	bool bNotInGame = (Minecraft::GetInstance()->level == NULL);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, true);
		m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad, eUIComponent_Panorama, false);
	
		if(app.GetLocalPlayerCount() == 1) m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, true);
		else m_parentLayer->showComponent(m_iPad, eUIComponent_Logo, false);
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
			app.SetGameSettings(m_iPad, eGameSetting_Clouds, m_checkboxClouds.IsChecked() ? 1 : 0);
			app.SetGameSettings(m_iPad, eGameSetting_BedrockFog, m_checkboxBedrockFog.IsChecked() ? 1 : 0);
			app.SetGameSettings(m_iPad, eGameSetting_CustomSkinAnim, m_checkboxCustomSkinAnim.IsChecked() ? 1 : 0);
			app.SetGameSettings(m_iPad, eGameSetting_VSync, m_checkboxVSync.IsChecked() ? 1 : 0);

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
	int value = (int)currentValue;
	switch((int)sliderId)
	{
	case eControl_RenderDistance:
		{
			m_sliderRenderDistance.handleSliderMove(value);

			int dist = LevelToDistance(value);
			app.SetGameSettings(m_iPad, eGameSetting_RenderDistance, dist);

			Minecraft* mc = Minecraft::GetInstance();
			mc->options->viewDistance = 3 - value;
			swprintf(TempString, 256, L"Render Distance: %d", dist);
			m_sliderRenderDistance.setLabel(TempString);
		}
		break;

	case eControl_Gamma:
		m_sliderGamma.handleSliderMove(value);
		app.SetGameSettings(m_iPad, eGameSetting_Gamma, value);
		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_GAMMA), value);
		m_sliderGamma.setLabel(TempString);
		break;

	case eControl_FOV:
		{
			m_sliderFOV.handleSliderMove(value);
			Minecraft* pMinecraft = Minecraft::GetInstance();
			int fovValue = sliderValueToFov(value);
			pMinecraft->gameRenderer->SetFovVal((float)fovValue);
			app.SetGameSettings(m_iPad, eGameSetting_FOV, value);
			swprintf(TempString, 256, L"FOV: %d", fovValue);
			m_sliderFOV.setLabel(TempString);
		}
		break;

	case eControl_InterfaceOpacity:
		m_sliderInterfaceOpacity.handleSliderMove(value);
		app.SetGameSettings(m_iPad, eGameSetting_InterfaceOpacity, value);
		swprintf(TempString, 256, L"%ls: %d%%", app.GetString(IDS_SLIDER_INTERFACEOPACITY), value);
		m_sliderInterfaceOpacity.setLabel(TempString);
		break;

	case eControl_FpsCap:
		{
			int idx = value;
			if (idx < 0) idx = 0;
			if (idx > 3) idx = 3;
			m_sliderFpsCap.handleSliderMove(idx);
			app.SetGameSettings(m_iPad, eGameSetting_FpsCap, idx);
			formatFpsCapLabel(TempString, idx);
			m_sliderFpsCap.setLabel(TempString);
		}
		break;
	}
}
