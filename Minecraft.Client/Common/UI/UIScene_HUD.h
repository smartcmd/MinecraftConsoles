#pragma once

#include "UIScene.h"
#include "IUIScene_HUD.h"

#define CHAT_LINES_COUNT 10

class UIScene_HUD : public UIScene, public IUIScene_HUD
{
private:
	bool m_bSplitscreen;

protected:
	UIControl_Label m_labelChatText[CHAT_LINES_COUNT];
	UIControl_Label m_labelJukebox;
	UIControl m_controlLabelBackground[CHAT_LINES_COUNT];
	UIControl_Label m_labelDisplayName;

	IggyName m_funcLoadHud, m_funcSetExpBarProgress, m_funcSetPlayerLevel, m_funcSetActiveSlot;
	IggyName m_funcSetHealth, m_funcSetFood, m_funcSetAir, m_funcSetArmour;
	IggyName m_funcShowHealth, m_funcShowHorseHealth, m_funcShowFood, m_funcShowAir, m_funcShowArmour, m_funcShowExpbar;
	IggyName m_funcSetRegenerationEffect, m_funcSetFoodSaturationLevel;
	IggyName m_funcSetDragonHealth, m_funcSetDragonLabel, m_funcShowDragonHealth;
	IggyName m_funcSetSelectedLabel, m_funcHideSelectedLabel;
	IggyName m_funcRepositionHud, m_funcSetDisplayName, m_funcSetTooltipsEnabled;
	IggyName m_funcSetRidingHorse, m_funcSetHorseHealth, m_funcSetHorseJumpBarProgress;
	IggyName m_funcSetHealthAbsorb;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT(m_labelChatText[0],"Label1")
		UI_MAP_ELEMENT(m_labelChatText[1],"Label2")
		UI_MAP_ELEMENT(m_labelChatText[2],"Label3")
		UI_MAP_ELEMENT(m_labelChatText[3],"Label4")
		UI_MAP_ELEMENT(m_labelChatText[4],"Label5")
		UI_MAP_ELEMENT(m_labelChatText[5],"Label6")
		UI_MAP_ELEMENT(m_labelChatText[6],"Label7")
		UI_MAP_ELEMENT(m_labelChatText[7],"Label8")
		UI_MAP_ELEMENT(m_labelChatText[8],"Label9")
		UI_MAP_ELEMENT(m_labelChatText[9],"Label10")

		UI_MAP_ELEMENT(m_controlLabelBackground[0],"Label1Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[1],"Label2Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[2],"Label3Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[3],"Label4Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[4],"Label5Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[5],"Label6Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[6],"Label7Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[7],"Label8Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[8],"Label9Background")
		UI_MAP_ELEMENT(m_controlLabelBackground[9],"Label10Background")

		UI_MAP_ELEMENT(m_labelJukebox,"Jukebox")

		UI_MAP_ELEMENT(m_labelDisplayName,"LabelGamertag")

		UI_MAP_NAME(m_funcLoadHud, L"LoadHud")
		UI_MAP_NAME(m_funcSetExpBarProgress, L"SetExpBarProgress")
		UI_MAP_NAME(m_funcSetPlayerLevel, L"SetPlayerLevel")
		UI_MAP_NAME(m_funcSetActiveSlot, L"SetActiveSlot")
		
		UI_MAP_NAME(m_funcSetHealth, L"SetHealth")
		UI_MAP_NAME(m_funcSetFood, L"SetFood")
		UI_MAP_NAME(m_funcSetAir, L"SetAir")
		UI_MAP_NAME(m_funcSetArmour, L"SetArmour")
		
		UI_MAP_NAME(m_funcShowHealth, L"ShowHealth")
		UI_MAP_NAME(m_funcShowHorseHealth, L"ShowHorseHealth")
		UI_MAP_NAME(m_funcShowFood, L"ShowFood")
		UI_MAP_NAME(m_funcShowAir, L"ShowAir")
		UI_MAP_NAME(m_funcShowArmour, L"ShowArmour")
		UI_MAP_NAME(m_funcShowExpbar, L"ShowExpBar")
		
		UI_MAP_NAME(m_funcSetRegenerationEffect, L"SetRegenerationEffect")
		UI_MAP_NAME(m_funcSetFoodSaturationLevel, L"SetFoodSaturationLevel")

		UI_MAP_NAME(m_funcSetDragonHealth, L"SetDragonHealth")
		UI_MAP_NAME(m_funcSetDragonLabel, L"SetDragonLabel")
		UI_MAP_NAME(m_funcShowDragonHealth, L"ShowDragonHealthBar")
		
		UI_MAP_NAME(m_funcSetSelectedLabel, L"SetSelectedLabel")
		UI_MAP_NAME(m_funcHideSelectedLabel, L"HideSelectedLabel")

		UI_MAP_NAME(m_funcRepositionHud, L"RepositionHud")
		UI_MAP_NAME(m_funcSetDisplayName, L"SetGamertag")

		UI_MAP_NAME(m_funcSetTooltipsEnabled, L"SetTooltipsEnabled")
		
		UI_MAP_NAME(m_funcSetRidingHorse, L"SetRidingHorse")
		UI_MAP_NAME(m_funcSetHorseHealth, L"SetHorseHealth")
		UI_MAP_NAME(m_funcSetHorseJumpBarProgress, L"SetHorseJumpBarProgress")
		
		UI_MAP_NAME(m_funcSetHealthAbsorb, L"SetHealthAbsorb")
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_HUD(int iPad, void *initData, UILayer *parentLayer);

	virtual void tick();

	virtual void updateSafeZone();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	virtual EUIScene getSceneType() { return eUIScene_HUD;}

	// Returns true if this scene handles input
	virtual bool stealsFocus() { return false; }

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return false; }

	// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
	virtual bool hidesLowerScenes() { return false; }

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

	virtual void handleReload();

private:
	virtual int getPad();
	virtual void SetOpacity(float opacity);
	virtual void SetVisible(bool visible);

	void SetHudSize(int scale);
	void SetExpBarProgress(float progress, int xpNeededForNextLevel);
	void SetExpLevel(int level);
	void SetActiveSlot(int slot);

	void SetHealth(int iHealth, int iLastHealth, bool bBlink, bool bPoison, bool bWither);
	void SetFood(int iFood, int iLastFood, bool bPoison);
	void SetAir(int iAir, int extra);
	void SetArmour(int iArmour);

	void ShowHealth(bool show);
	void ShowHorseHealth(bool show);
	void ShowFood(bool show);
	void ShowAir(bool show);
	void ShowArmour(bool show);
	void ShowExpBar(bool show);

	void SetRegenerationEffect(bool bEnabled);
	void SetFoodSaturationLevel(int iSaturation);

	void SetDragonHealth(float health);
	void SetDragonLabel(const wstring &label);
	void ShowDragonHealth(bool show);

	void HideSelectedLabel();

	void SetDisplayName(const wstring &displayName);

	void SetTooltipsEnabled(bool bEnabled);

	void SetRidingHorse(bool ridingHorse, bool bIsJumpable, int maxHorseHealth);
	void SetHorseHealth(int health, bool blink = false);
	void SetHorseJumpBarProgress(float progress);

	void SetHealthAbsorb(int healthAbsorb);

public:
	void SetSelectedLabel(const wstring &label);
	void ShowDisplayName(bool show);

	void handleGameTick();

	// RENDERING
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewport);

protected:
	void handleTimerComplete(int id);

#ifdef _DURANGO	
	virtual long long getDefaultGtcButtons() { return _360_GTC_PAUSE | _360_GTC_MENU | _360_GTC_VIEW; }
#endif

private:
	void repositionHud(S32 tileWidth, S32 tileHeight, F32 scale);
};
