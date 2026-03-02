#pragma once

#include "UIScene.h"

#define TUTORIAL_POPUP_FADE_TIMER_ID 0
#define TUTORIAL_POPUP_MOVE_SCENE_TIMER_ID 1
#define TUTORIAL_POPUP_MOVE_SCENE_TIME 500


class UIComponent_TutorialPopup : public UIScene
{
private:
	// A scene that may be displayed behind the popup that the player is using, that will need shifted so we can see it clearly.
	UIScene *m_interactScene, *m_lastInteractSceneMoved;
	bool m_lastSceneMovedLeft;
	bool m_bAllowFade;
	Tutorial *m_tutorial;
	shared_ptr<ItemInstance> m_iconItem;
	bool m_iconIsFoil;
	//int m_iLocalPlayerC;

	bool m_bContainerMenuVisible;
	bool m_bSplitscreenGamertagVisible;

	// Maps to values in AS
	enum EIcons
	{
		e_ICON_TYPE_IGGY = 0,
		e_ICON_TYPE_ARMOUR = 1,
		e_ICON_TYPE_BREWING = 2,
		e_ICON_TYPE_DECORATION = 3,
		e_ICON_TYPE_FOOD = 4,
		e_ICON_TYPE_MATERIALS = 5,
		e_ICON_TYPE_MECHANISMS = 6,
		e_ICON_TYPE_MISC = 7,
		e_ICON_TYPE_REDSTONE_AND_TRANSPORT = 8,
		e_ICON_TYPE_STRUCTURES = 9,
		e_ICON_TYPE_TOOLS = 10,
		e_ICON_TYPE_TRANSPORT = 11,
	};

	EIcons m_iconType;

public:
	UIComponent_TutorialPopup(int iPad, void *initData, UILayer *parentLayer);

protected:
	UIControl_Label m_labelDescription, m_labelTitle;
	UIControl m_controlIconHolder;
	UIControl m_controlExitScreenshot;
	IggyName m_funcAdjustLayout, m_funcSetupIconHolder;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_labelTitle, "Title")
		UI_MAP_ELEMENT( m_labelDescription, "Description")
		UI_MAP_ELEMENT( m_controlIconHolder, "IconHolder")
		UI_MAP_ELEMENT( m_controlExitScreenshot, "ExitScreenShot")

		UI_MAP_NAME( m_funcAdjustLayout, L"AdjustLayout")
		UI_MAP_NAME( m_funcSetupIconHolder, L"SetupIconHolder")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();

public:
	virtual EUIScene getSceneType() { return eUIComponent_TutorialPopup;}

	// Returns true if this scene handles input
	virtual bool stealsFocus() { return false; }

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return false; }

	// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
	virtual bool hidesLowerScenes() { return false; }

	virtual void handleReload();

	void SetContainerMenuVisible(bool bContainerMenuVisible) { m_bContainerMenuVisible = bContainerMenuVisible; }
	void UpdateTutorialPopup();

	void SetTutorial( Tutorial *tutorial ) { m_tutorial = tutorial; }
	void SetTutorialDescription(TutorialPopupInfo *info);
	void RemoveInteractSceneReference(UIScene *scene);
	void SetVisible(bool visible);
	bool IsVisible();

	// RENDERING
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewport);

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

protected:
	void handleTimerComplete(int id);

private:
	void _SetDescription(UIScene *interactScene, const wstring &desc, const wstring &title, bool allowFade, bool isReminder);
	wstring _SetIcon(int icon, int iAuxVal, bool isFoil, LPCWSTR desc);
	wstring _SetImage(wstring &desc);
	wstring ParseDescription(int iPad, wstring &text);
	void UpdateInteractScenePosition(bool visible);

	void setupIconHolder(EIcons icon);
};
