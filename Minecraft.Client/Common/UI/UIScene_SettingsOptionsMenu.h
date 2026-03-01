#pragma once

#include "UIScene.h"

class UIScene_SettingsOptionsMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_ViewBob,
		eControl_ShowHints,
		eControl_ShowTooltips,
		eControl_InGameGamertags,
		eControl_ShowMashUpWorlds,
		eControl_Autosave,
		eControl_Languages,
		eControl_Difficulty
	};
protected:
	static int m_iDifficultySettingA[4];
	static int m_iDifficultyTitleSettingA[4];

private:
	UIControl_CheckBox m_checkboxViewBob, m_checkboxShowHints, m_checkboxShowTooltips, m_checkboxInGameGamertags, m_checkboxMashupWorlds; // Checkboxes
	UIControl_Slider m_sliderAutosave, m_sliderDifficulty; // Sliders
	UIControl_Label m_labelDifficultyText; //Text
	UIControl_Button m_buttonLanguageSelect;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_checkboxViewBob, "ViewBob")
		UI_MAP_ELEMENT( m_checkboxShowHints, "ShowHints")
		UI_MAP_ELEMENT( m_checkboxShowTooltips, "ShowTooltips")
		UI_MAP_ELEMENT( m_checkboxInGameGamertags, "InGameGamertags")
		UI_MAP_ELEMENT( m_checkboxMashupWorlds, "ShowMashUpWorlds")
		UI_MAP_ELEMENT( m_sliderAutosave, "Autosave")
		UI_MAP_ELEMENT( m_sliderDifficulty, "Difficulty")
		UI_MAP_ELEMENT( m_labelDifficultyText, "DifficultyText")
		UI_MAP_ELEMENT( m_buttonLanguageSelect, "Languages")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	bool m_bNotInGame;
	bool m_bMashUpWorldsUnhideOption;
	bool m_bNavigateToLanguageSelector;

public:
	UIScene_SettingsOptionsMenu(int iPad, void *initData, UILayer *parentLayer);
	virtual ~UIScene_SettingsOptionsMenu();

	virtual EUIScene getSceneType() { return eUIScene_SettingsOptionsMenu;}
	
	virtual void tick();

	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handlePress(F64 controlId, F64 childId);

	virtual void handleReload();

	virtual void handleSliderMove(F64 sliderId, F64 currentValue);

protected:
	void setGameSettings();

};