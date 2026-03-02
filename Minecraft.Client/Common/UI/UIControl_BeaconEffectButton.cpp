#include "stdafx.h"
#include "UI.h"
#include "UIControl_BeaconEffectButton.h"

UIControl_BeaconEffectButton::UIControl_BeaconEffectButton()
{
	m_data = 0;
	m_icon = 0;
	m_selected = false;
	m_active = false;
	m_focus = false;
}

bool UIControl_BeaconEffectButton::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	bool success = UIControl::setupControl(scene,parent,controlName);

	m_funcChangeState = registerFastName(L"ChangeState");
	m_funcSetIcon = registerFastName(L"SetIcon");

	return success;
}

void UIControl_BeaconEffectButton::SetData(int data, int icon, bool active, bool selected)
{
	m_data = data;
	m_active = active;
	m_selected = selected;

	SetIcon(icon);
	UpdateButtonState();
}

int UIControl_BeaconEffectButton::GetData()
{
	return m_data;
}

void UIControl_BeaconEffectButton::SetButtonSelected(bool selected)
{
	if(selected != m_selected)
	{
		m_selected = selected;

		UpdateButtonState();
	}
}

bool UIControl_BeaconEffectButton::IsButtonSelected()
{
	return m_selected;
}

void UIControl_BeaconEffectButton::SetButtonActive(bool active)
{
	if(m_active != active)
	{
		m_active = active;

		UpdateButtonState();
	}
}

void UIControl_BeaconEffectButton::setFocus(bool focus)
{
	if(m_focus != focus)
	{
		m_focus = focus;

		UpdateButtonState();
	}
}

void UIControl_BeaconEffectButton::SetIcon(int icon)
{
	if(icon != m_icon)
	{
		m_icon = icon;

		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = m_icon;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetIcon , 1 , value );
	}
}

void UIControl_BeaconEffectButton::UpdateButtonState()
{
	EState state = eState_Disabled;

	if(!m_active)
	{
		state = eState_Disabled;
	}
	else if(m_selected)
	{
		state = eState_Pressed;
	}
	else if(m_focus)
	{
		state = eState_Enabled_Selected;
	}
	else
	{
		state = eState_Enabled_Unselected;
	}

	if(state != m_lastState)
	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = state;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcChangeState , 1 , value );

		if(out == IGGY_RESULT_SUCCESS) m_lastState = state;
	}
}