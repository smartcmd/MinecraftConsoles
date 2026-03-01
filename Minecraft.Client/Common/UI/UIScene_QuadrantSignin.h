#pragma once

#include "UIScene.h"

class UIScene_QuadrantSignin : public UIScene
{
private:
	enum EControllerStatus
	{
		eControllerStatus_ConnectController,
		eControllerStatus_PressToJoin,
		eControllerStatus_PlayerDetails,
		eControllerStatus_PressToJoin_LoggedIn,
		eControllerStatus_PressToJoin_NoController,
	};

	bool m_bIgnoreInput;
	SignInInfo m_signInInfo;

	EControllerStatus m_controllerStatus[4];
	bool m_iconRequested[4];

	int m_lastRequestedAvatar;

	UIControl m_controlPanels[4];
	UIControl_Label m_labelPressToJoin[4], m_labelDisplayName[4], m_labelAccountType[4], m_labelPlayerNumber[4], m_labelConnectController[4];
	UIControl_BitmapIcon m_bitmapIcon[4];
	IggyName m_funcJoinButtonPressed, m_funcSetControllerStatus, m_funcSetABSwap;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT(m_controlPanels[0],"Controller1")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlPanels[0])
			UI_MAP_ELEMENT(m_labelPressToJoin[0], "PressLabel")
			
			UI_MAP_ELEMENT(m_labelDisplayName[0], "GamerTag")
			UI_MAP_ELEMENT(m_labelAccountType[0], "AccountType")
			UI_MAP_ELEMENT(m_labelPlayerNumber[0], "PlayerNumber")
			UI_MAP_ELEMENT(m_bitmapIcon[0], "PlayerPic")

			UI_MAP_ELEMENT(m_labelConnectController[0], "ConnectControllerLabel")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_ELEMENT(m_controlPanels[1],"Controller2")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlPanels[1])
			UI_MAP_ELEMENT(m_labelPressToJoin[1], "PressLabel")
			
			UI_MAP_ELEMENT(m_labelDisplayName[1], "GamerTag")
			UI_MAP_ELEMENT(m_labelAccountType[1], "AccountType")
			UI_MAP_ELEMENT(m_labelPlayerNumber[1], "PlayerNumber")
			UI_MAP_ELEMENT(m_bitmapIcon[1], "PlayerPic")

			UI_MAP_ELEMENT(m_labelConnectController[1], "ConnectControllerLabel")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_ELEMENT(m_controlPanels[2],"Controller3")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlPanels[2])
			UI_MAP_ELEMENT(m_labelPressToJoin[2], "PressLabel")
			
			UI_MAP_ELEMENT(m_labelDisplayName[2], "GamerTag")
			UI_MAP_ELEMENT(m_labelAccountType[2], "AccountType")
			UI_MAP_ELEMENT(m_labelPlayerNumber[2], "PlayerNumber")
			UI_MAP_ELEMENT(m_bitmapIcon[2], "PlayerPic")

			UI_MAP_ELEMENT(m_labelConnectController[2], "ConnectControllerLabel")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_ELEMENT(m_controlPanels[3],"Controller4")
		UI_BEGIN_MAP_CHILD_ELEMENTS(m_controlPanels[3])
			UI_MAP_ELEMENT(m_labelPressToJoin[3], "PressLabel")
			
			UI_MAP_ELEMENT(m_labelDisplayName[3], "GamerTag")
			UI_MAP_ELEMENT(m_labelAccountType[3], "AccountType")
			UI_MAP_ELEMENT(m_labelPlayerNumber[3], "PlayerNumber")
			UI_MAP_ELEMENT(m_bitmapIcon[3], "PlayerPic")

			UI_MAP_ELEMENT(m_labelConnectController[3], "ConnectControllerLabel")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME(m_funcJoinButtonPressed, L"JoinButtonPressed")
		UI_MAP_NAME(m_funcSetControllerStatus, L"SetControllerStatus")
		UI_MAP_NAME(m_funcSetABSwap, L"SetABSwap")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_QuadrantSignin(int iPad, void *initData, UILayer *parentLayer);
	~UIScene_QuadrantSignin();

	virtual EUIScene getSceneType() { return eUIScene_QuadrantSignin;}
	virtual void updateTooltips();

	virtual bool hasFocus(int iPad);
	virtual bool hidesLowerScenes();

	void tick();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

private:
#ifdef _XBOX_ONE
	static int SignInReturned(void *pParam,bool bContinue, int iPad, int iController);
#else
	static int SignInReturned(void *pParam,bool bContinue, int iPad);
#endif
	static int AvatarReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes);

	void updateState();
	void setControllerState(int iPad, EControllerStatus state);

#ifdef _DURANGO
	static void checkAllPrivilegesCallback(LPVOID lpParam, bool hasPrivileges, int iPad);
#endif

protected:
	void _initQuadrants();

	virtual void handleReload();
};
