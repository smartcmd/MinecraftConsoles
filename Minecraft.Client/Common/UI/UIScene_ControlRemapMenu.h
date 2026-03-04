#pragma once

#include "UIScene.h"
#include "UIScene_InputPrompt.h"

class Options;

class UIScene_ControlRemapMenu : public UIScene
{
private:
	enum EControls
	{
		eControl_ResetDefaults = 0,
		eControl_ActionButtons,
	};

	enum ELabels
	{
		eLabel_Action = 0,
		eLabel_Keyboard,
		eLabel_Controller,

		eLabel_COUNT
	};

	// JoinMenu SWF has: JoinGame button, GamePlayers ButtonList, Label0-8 / Value0-8
	UIControl_Button m_buttonReset;
	UIControl_ButtonList m_buttonListActions;

	UIControl_Label m_labelLabels[9];
	UIControl_Label m_labelValues[9];

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_buttonReset, "JoinGame")
		UI_MAP_ELEMENT( m_buttonListActions, "GamePlayers")

		UI_MAP_ELEMENT( m_labelLabels[0], "Label0")
		UI_MAP_ELEMENT( m_labelLabels[1], "Label1")
		UI_MAP_ELEMENT( m_labelLabels[2], "Label2")
		UI_MAP_ELEMENT( m_labelLabels[3], "Label3")
		UI_MAP_ELEMENT( m_labelLabels[4], "Label4")
		UI_MAP_ELEMENT( m_labelLabels[5], "Label5")
		UI_MAP_ELEMENT( m_labelLabels[6], "Label6")
		UI_MAP_ELEMENT( m_labelLabels[7], "Label7")
		UI_MAP_ELEMENT( m_labelLabels[8], "Label8")

		UI_MAP_ELEMENT( m_labelValues[0], "Value0")
		UI_MAP_ELEMENT( m_labelValues[1], "Value1")
		UI_MAP_ELEMENT( m_labelValues[2], "Value2")
		UI_MAP_ELEMENT( m_labelValues[3], "Value3")
		UI_MAP_ELEMENT( m_labelValues[4], "Value4")
		UI_MAP_ELEMENT( m_labelValues[5], "Value5")
		UI_MAP_ELEMENT( m_labelValues[6], "Value6")
		UI_MAP_ELEMENT( m_labelValues[7], "Value7")
		UI_MAP_ELEMENT( m_labelValues[8], "Value8")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	int m_currentAction;
	Options *m_options;
	InputPromptInfo m_promptInfo;

	void updateActionDisplay();
	static wstring getActionFriendlyName(int action);
	static wstring getKeyDisplayName(Options *options, int action);

	static void KeyboardPromptCallback(void *param, int detectedValue);
	static void ControllerPromptCallback(void *param, int detectedValue);
	static int ResetDefaultsDialogReturned(void *pParam, int iPad, C4JStorage::EMessageResult result);

public:
	UIScene_ControlRemapMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_ControlRemapMenu; }

	virtual void updateTooltips();
	virtual void updateComponents();

protected:
	virtual wstring getMoviePath();

public:
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
	virtual void handleFocusChange(F64 controlId, F64 childId);

protected:
	void handlePress(F64 controlId, F64 childId);
};
