#pragma once

#include "UIScene.h"

class UIScene_InGameHostOptionsMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_FireSpreads,
		eControl_TNT,
		eControl_MobGriefing,
		eControl_KeepInventory,
		eControl_DoMobSpawning,
		eControl_DoMobLoot,
		eControl_DoTileDrops,
		eControl_NaturalRegeneration,
		eControl_DoDaylightCycle,
		eControl_TeleportToPlayer,
		eControl_TeleportToMe,
	};
	
	UIControl_CheckBox m_checkboxFireSpreads, m_checkboxTNT, m_checkboxMobGriefing, m_checkboxKeepInventory, m_checkboxDoMobSpawning, m_checkboxDoMobLoot, m_checkboxDoTileDrops, m_checkboxNaturalRegeneration, m_checkboxDoDaylightCycle;
	UIControl_Button m_buttonTeleportToPlayer, m_buttonTeleportToMe;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_checkboxFireSpreads, "CheckboxFireSpreads")
		UI_MAP_ELEMENT( m_checkboxTNT, "CheckboxTNT")
		UI_MAP_ELEMENT( m_checkboxMobGriefing, "CheckboxMobGriefing")
		UI_MAP_ELEMENT( m_checkboxKeepInventory, "CheckboxKeepInventory")
		UI_MAP_ELEMENT( m_checkboxDoMobSpawning, "CheckboxMobSpawning")
		UI_MAP_ELEMENT( m_checkboxDoMobLoot, "CheckboxMobLoot")
		UI_MAP_ELEMENT( m_checkboxDoTileDrops, "CheckboxTileDrops")
		UI_MAP_ELEMENT( m_checkboxNaturalRegeneration, "CheckboxNaturalRegeneration")
		UI_MAP_ELEMENT( m_checkboxDoDaylightCycle, "CheckboxDayLightCycle")
		UI_MAP_ELEMENT( m_buttonTeleportToPlayer, "TeleportToPlayer")
		UI_MAP_ELEMENT( m_buttonTeleportToMe, "TeleportPlayerToMe")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_InGameHostOptionsMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_InGameHostOptionsMenu;}
	virtual void updateTooltips();

	virtual void handleReload();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handlePress(F64 controlId, F64 childId);
};