#pragma once

class IUIScene_HUD
{
protected:
	int m_lastActiveSlot;
	int m_iGuiScale;
	bool m_bToolTipsVisible;
	float m_lastExpProgress;
	int m_lastExpLevel;
	int m_iCurrentHealth;
	int m_lastMaxHealth;
	bool m_lastHealthBlink, m_lastHealthPoison, m_lastHealthWither;
	int m_iCurrentFood;
	bool m_lastFoodPoison;
	int m_lastAir, m_currentExtraAir;
	int m_lastArmour;
	float m_lastDragonHealth;
	bool m_showDragonHealth;
	int m_ticksWithNoBoss;
	bool m_lastShowDisplayName;
	int m_horseHealth;
	int m_iCurrentHealthAbsorb;
	float m_horseJumpProgress;
	int m_iHeartOffsetIndex;
	bool m_bHealthAbsorbActive;
	int m_iHorseMaxHealth;

	bool m_showHealth, m_showHorseHealth, m_showFood, m_showAir, m_showArmour, m_showExpBar, m_bRidingHorse, m_bIsJumpable;
	bool m_bRegenEffectEnabled;
	int m_iFoodSaturation;

	unsigned int m_uiSelectedItemOpacityCountDown;

	wstring m_displayName;

	IUIScene_HUD();

	virtual int getPad() = 0;
	virtual void SetOpacity(float opacity) = 0;
	virtual void SetVisible(bool visible) = 0;

	virtual void SetHudSize(int scale) = 0;
	virtual void SetExpBarProgress(float progress, int xpNeededForNextLevel) = 0;
	virtual void SetExpLevel(int level) = 0;
	virtual void SetActiveSlot(int slot) = 0;

	virtual void SetHealth(int iHealth, int iLastHealth, bool bBlink, bool bPoison, bool bWither) = 0;
	virtual void SetFood(int iFood, int iLastFood, bool bPoison) = 0;
	virtual void SetAir(int iAir, int extra) = 0;
	virtual void SetArmour(int iArmour) = 0;

	virtual void ShowHealth(bool show) = 0;
	virtual void ShowHorseHealth(bool show) = 0;
	virtual void ShowFood(bool show) = 0;
	virtual void ShowAir(bool show) = 0;
	virtual void ShowArmour(bool show) = 0;
	virtual void ShowExpBar(bool show) = 0;

	virtual void SetRegenerationEffect(bool bEnabled) = 0;
	virtual void SetFoodSaturationLevel(int iSaturation) = 0;

	virtual void SetDragonHealth(float health) = 0;
	virtual void SetDragonLabel(const wstring &label) = 0;
	virtual void ShowDragonHealth(bool show) = 0;

	virtual void HideSelectedLabel() = 0;

	virtual void SetDisplayName(const wstring &displayName) = 0;

	virtual void SetTooltipsEnabled(bool bEnabled) = 0;

	virtual void SetRidingHorse(bool ridingHorse, bool bIsJumpable, int maxHorseHealth) = 0;
	virtual void SetHorseHealth(int health, bool blink = false) = 0;
	virtual void SetHorseJumpBarProgress(float progress) = 0;

	virtual void SetHealthAbsorb(int healthAbsorb) = 0;

	virtual void SetSelectedLabel(const wstring &label) = 0;
	virtual void ShowDisplayName(bool show) = 0;

public:
	void updateFrameTick();
	void renderPlayerHealth();
};