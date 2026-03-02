#pragma once

#include "UIScene.h"

class UIScene_HowToPlayMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_Buttons,
	};

	enum eHTPButton
	{
		eHTPButton_WhatsNew = 0,
		eHTPButton_Basics,
		eHTPButton_Multiplayer,
		eHTPButton_Hud,
		eHTPButton_Creative,
		eHTPButton_Inventory,
		eHTPButton_Chest,
		eHTPButton_Crafting,
		eHTPButton_Furnace,
		eHTPButton_Dispenser,
		eHTPButton_Brewing,
		eHTPButton_Enchantment,
		eHTPButton_Anvil,
		eHTPButton_FarmingAnimals,
		eHTPButton_Breeding,
		eHTPButton_Trading,
		eHTPButton_Horses,
		eHTPButton_Beacons,
		eHTPButton_Fireworks,
		eHTPButton_Hoppers,
		eHTPButton_Droppers,
		eHTPButton_NetherPortal,
		eHTPButton_TheEnd,
#ifdef _XBOX
		eHTPButton_SocialMedia,
		eHTPButton_BanningLevels,
#endif
		eHTPButton_HostOptions,
		eHTPButton_Max,
	};
	
	static unsigned int m_uiHTPButtonNameA[eHTPButton_Max];
	static unsigned int m_uiHTPSceneA[eHTPButton_Max];

	UIControl_ButtonList m_buttonListHowTo;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonListHowTo, "HowToList")
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	UIScene_HowToPlayMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_HowToPlayMenu;}
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual void handleReload();
protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

protected:
	void handlePress(F64 controlId, F64 childId);
};