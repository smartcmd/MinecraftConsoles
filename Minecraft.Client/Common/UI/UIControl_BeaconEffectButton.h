#pragma once

#include "UIControl.h"

class UIControl_BeaconEffectButton : public UIControl
{
private:
	static const int BUTTON_DISABLED = 0;
	static const int BUTTON_ENABLED_UNSELECTED = 1;
	static const int BUTTON_ENABLED_SELECTED = 2;
	static const int BUTTON_PRESSED = 3;

	enum EState
	{
		eState_Disabled,
		eState_Enabled_Unselected,
		eState_Enabled_Selected,
		eState_Pressed
	};
	EState m_lastState;

	int	m_data;
	int m_icon;
	bool m_selected;
	bool m_active;
	bool m_focus;

	IggyName m_funcChangeState, m_funcSetIcon;

public:
	UIControl_BeaconEffectButton();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void SetData(int data, int icon, bool active, bool selected);
	int GetData();

	void SetButtonSelected(bool selected);
	bool IsButtonSelected();

	void SetButtonActive(bool active);

	virtual void setFocus(bool focus);

	void SetIcon(int icon);

private:
	void UpdateButtonState();
};