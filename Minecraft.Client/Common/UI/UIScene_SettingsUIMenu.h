#pragma once

#include "UIScene.h"

class UIScene_SettingsUIMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_DisplayHUD,
		eControl_DisplayHand,
		eControl_DisplayDeathMessages,
		eControl_DisplayAnimatedCharacter,
		eControl_Splitscreen,
		eControl_ShowSplitscreenGamertags,
		eControl_OldSwingAnimation,
		eControl_UISize,
		eControl_UISizeSplitscreen
	};

	UIControl_CheckBox m_checkboxDisplayHUD, m_checkboxDisplayHand, m_checkboxDisplayDeathMessages, m_checkboxDisplayAnimatedCharacter, m_checkboxSplitscreen, m_checkboxShowSplitscreenGamertags, m_checkboxOldSwingAnimation; // Checkboxes
	UIControl_Slider m_sliderUISize, m_sliderUISizeSplitscreen; // Sliders
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_checkboxDisplayHUD, "DisplayHUD")
		UI_MAP_ELEMENT( m_checkboxDisplayHand, "DisplayHand")
		UI_MAP_ELEMENT( m_checkboxDisplayDeathMessages, "DisplayDeathMessages")
		UI_MAP_ELEMENT( m_checkboxDisplayAnimatedCharacter, "DisplayAnimatedCharacter")
		UI_MAP_ELEMENT( m_checkboxSplitscreen, "Splitscreen")
		UI_MAP_ELEMENT( m_checkboxShowSplitscreenGamertags, "ShowSplitscreenGamertags")
		UI_MAP_ELEMENT( m_checkboxOldSwingAnimation, "OldSwingAnimation")

		UI_MAP_ELEMENT( m_sliderUISize, "UISize")
		UI_MAP_ELEMENT( m_sliderUISizeSplitscreen, "UISizeSplitscreen")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	bool m_bNotInGame;
public:
	UIScene_SettingsUIMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsUIMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsUIMenu;}
	
	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleSliderMove(F64 sliderId, F64 currentValue);
};