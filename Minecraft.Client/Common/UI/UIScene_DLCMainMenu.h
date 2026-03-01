#pragma once

#include "UIScene.h"

class UIScene_DLCMainMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_OffersList,
	};

	UIControl_DynamicButtonList m_buttonListOffers;
	UIControl_Label m_labelOffers, m_labelXboxStore;
	UIControl m_Timer;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonListOffers, "OffersList")
		UI_MAP_ELEMENT( m_labelOffers, "OffersList_Title")
		UI_MAP_ELEMENT( m_Timer, "Timer")
		if(m_loadedResolution == eSceneResolution_1080)
		{
			UI_MAP_ELEMENT( m_labelXboxStore, "XboxLabel" )
		}
	UI_END_MAP_ELEMENTS_AND_NAMES()

	static int ExitDLCMainMenu(void *pParam,int iPad,C4JStorage::EMessageResult result);

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
 	bool m_bCategoriesShown;
#endif

public:
	UIScene_DLCMainMenu(int iPad, void *initData, UILayer *parentLayer);
	~UIScene_DLCMainMenu();
	virtual void handleTimerComplete(int id);
	virtual void handleGainFocus(bool navBack);

	virtual EUIScene getSceneType() { return eUIScene_DLCMainMenu;}
	virtual void tick();
	virtual void updateTooltips();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handlePress(F64 controlId, F64 childId);
};
