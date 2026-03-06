#pragma once

#include "UIScene.h"

class UIScene_LaunchMoreOptionsMenu : public UIScene
{
private:
	static const int TAB_WORLD_OPTIONS = 0;
	static const int TAB_GAME_OPTIONS = 1;

	enum EControls
	{
		// Add all checkboxes at the start as they also index into a checkboxes array
		eLaunchCheckbox_Online,
		eLaunchCheckbox_InviteOnly,
		eLaunchCheckbox_AllowFoF,
		eLaunchCheckbox_PVP,
		eLaunchCheckbox_TrustSystem,
		eLaunchCheckbox_FireSpreads,
		eLaunchCheckbox_TNT,
		eLaunchCheckbox_HostPrivileges,
		eLaunchCheckbox_ResetNether,
		eLaunchCheckbox_Structures,
		eLaunchCheckbox_FlatWorld,
		eLaunchCheckbox_BonusChest,
		eLaunchCheckbox_DisableSaving,
		eLaunchCheckbox_WorldResizeType,
		eLaunchCheckbox_KeepInventory,
		eLaunchCheckbox_MobSpawning,
		eLaunchCheckbox_MobLoot,
		eLaunchCheckbox_MobGriefing,
		eLaunchCheckbox_TileDrops,
		eLaunchCheckbox_NaturalRegeneration,
		eLaunchCheckbox_DayLightCycle,

		eLaunchCheckboxes_Count,

		eControl_EditSeed,
		eControl_WorldSize,
		eControl_WorldResize,

		eControl_Count
	};

#ifdef __PSVITA__
	enum ETouchInput
	{
		ETouchInput_TabWorld = eControl_Count,
		ETouchInput_TabGame,
		
		ETouchInput_Count
	};
	UIControl_Touch m_TouchTabWorld, m_TouchTabGame;
	UIControl m_controlWorldPanel, m_controlGamePanel;
#endif
	UIControl m_gameOptions, m_worldOptions;
	UIControl_CheckBox m_checkboxes[eLaunchCheckboxes_Count];
	UIControl_Label m_labelWorldOptions, m_labelGameOptions, m_labelDescription;
	UIControl_HTMLLabel m_labelDescription_GameOptions, m_labelDescription_WorldOptions;
	UIControl_Label m_labelSeed, m_labelRandomSeed, m_labelWorldSize, m_labelWorldResize;
	UIControl_TextInput m_editSeed;
	UIControl_Slider m_sliderWorldSize;
	UIControl_Slider m_sliderWorldResize;
	IggyName m_funcSetMenuType, m_funcChangeTab, m_funcSetDescription;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_labelGameOptions, "LabelGame")
		UI_MAP_ELEMENT( m_labelWorldOptions, "LabelWorld")

		UI_MAP_ELEMENT( m_gameOptions, "GameOptions")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_gameOptions)
#ifdef __PSVITA__
			UI_MAP_ELEMENT( m_TouchTabGame, "TouchTabGame" )
#endif
			UI_MAP_ELEMENT( m_labelDescription_GameOptions, "Description_GameOptions")

			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_Online], "CheckboxOnline")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_InviteOnly], "CheckboxInviteOnly")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_AllowFoF], "CheckboxAllowFoF")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_PVP], "CheckboxPVP")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_HostPrivileges], "CheckboxHostPrivileges")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_DayLightCycle], "CheckboxDayLightCycle")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_KeepInventory], "CheckboxKeepInventory")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_MobSpawning], "CheckboxMobSpawning")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_MobGriefing], "CheckboxMobGriefing")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_MobLoot], "CheckboxMobLoot")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_TileDrops], "CheckboxTileDrops")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_NaturalRegeneration], "CheckboxNaturalRegeneration")
		UI_END_MAP_CHILD_ELEMENTS()
			
		UI_MAP_ELEMENT(m_worldOptions, "WorldOptions")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_worldOptions)
#ifdef __PSVITA__
			UI_MAP_ELEMENT( m_TouchTabWorld, "TouchTabWorld" )
#endif
			UI_MAP_ELEMENT( m_labelDescription_WorldOptions, "Description_WorldOptions")

			UI_MAP_ELEMENT( m_labelSeed, "Seed")
			UI_MAP_ELEMENT( m_editSeed, "EditSeed")
			UI_MAP_ELEMENT( m_labelRandomSeed, "RandomSeed")
			UI_MAP_ELEMENT( m_labelWorldSize, "WorldSize")
			UI_MAP_ELEMENT( m_sliderWorldSize, "WorldSizeSlider")

			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_Structures], "CheckboxStructures")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_BonusChest], "CheckboxBonusChest")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_FlatWorld], "CheckboxFlatWorld")

			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_ResetNether], "CheckboxResetNether")

			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_DisableSaving], "CheckboxDisableSaving")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_TrustSystem], "CheckboxTrustSystem")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_FireSpreads], "CheckboxFireSpreads")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_TNT], "CheckboxTNT")

			UI_MAP_ELEMENT( m_labelWorldResize, "ResizeLabel")
			UI_MAP_ELEMENT( m_sliderWorldResize, "ChangeWorldSizeSlider")
			UI_MAP_ELEMENT( m_checkboxes[eLaunchCheckbox_WorldResizeType], "CheckboxResizeType")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME( m_funcChangeTab, L"ChangeTab")
		UI_MAP_NAME( m_funcSetMenuType, L"SetMenuType")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	LaunchMoreOptionsMenuInitData *m_params;
	bool m_bMultiplayerAllowed;
	bool m_bIgnoreInput;
	int m_tabIndex;

public:
	UIScene_LaunchMoreOptionsMenu(int iPad, void *initData, UILayer *parentLayer);
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_LaunchMoreOptionsMenu;}

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	virtual void tick();
	virtual void handleDestroy();
#ifdef _WINDOWS64
	virtual void getDirectEditInputs(vector<UIControl_TextInput*> &inputs);
	virtual void onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result);
#endif
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handleFocusChange(F64 controlId, F64 childId);
	virtual void handleTimerComplete(int id);
	static int KeyboardCompleteSeedCallback(LPVOID lpParam,const bool bRes);
	virtual void handlePress(F64 controlId, F64 childId);
	virtual void handleSliderMove(F64 sliderId, F64 currentValue);

protected:
	void handleCheckboxToggled(F64 controlId, bool selected);

private:
	bool m_bUpdateCheats; // If true, update cheats on next tick
	void UpdateCheats();

	bool m_bUpdateOnline; // If true, update online settings on next tick
	void UpdateOnline();

#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
#endif //__PSVITA__
#if defined(__PSVITA__) || defined(_WINDOWS64)
	virtual UIControl* GetMainPanel();
#endif
};
