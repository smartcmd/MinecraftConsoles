#include "stdafx.h"
#include "UI.h"
#include "UIControl_Button.h"

UIControl_Button::UIControl_Button()
{
}

bool UIControl_Button::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eButton);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Button specific initialisers
	m_funcEnableButton = registerFastName(L"EnableButton");

	return success;
}

void UIControl_Button::init(UIString label, int id)
{
	m_label = label;
	m_id = id;

	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = id;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 2 , value );

#ifdef __PSVITA__
	// 4J-PB - add this button to the vita touch box list

	switch(m_parentScene->GetParentLayer()->m_iLayer)
	{
	case eUILayer_Error:
	case eUILayer_Fullscreen:
	case eUILayer_Scene:
	case eUILayer_HUD:
		ui.TouchBoxAdd(this,m_parentScene);
		break;
	}
#endif
}

void UIControl_Button::ReInit()
{
	UIControl_Base::ReInit();

	init(m_label, m_id);
}

void UIControl_Button::setEnable(bool enable)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = enable;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcEnableButton , 1 , value );
}
