#include "stdafx.h"
#include "UI.h"
#include "UIControl_CheckBox.h"

UIControl_CheckBox::UIControl_CheckBox()
{
}

bool UIControl_CheckBox::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eCheckBox);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//CheckBox specific initialisers
	m_checkedProp = registerFastName(L"Checked");
	m_funcEnable = registerFastName(L"EnableCheckBox");
	m_funcSetCheckBox = registerFastName(L"SetCheckBox");

	m_bEnabled = true;

	return success;
}

void UIControl_CheckBox::init(UIString label, int id, bool checked)
{
	m_label = label;
	m_id = id;
	m_bChecked = checked;

	IggyDataValue result;
	IggyDataValue value[3];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (int)id;

	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = checked;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 3 , value );

#ifdef __PSVITA__
	// 4J-TomK - add checkbox to the vita touch box list

	switch(m_parentScene->GetParentLayer()->m_iLayer)
	{
	case eUILayer_Fullscreen:
	case eUILayer_Scene:
	case eUILayer_HUD:
		ui.TouchBoxAdd(this,m_parentScene);
		break;
}
#endif
}

bool UIControl_CheckBox::IsChecked()
{
	rrbool checked = false;
	IggyResult result = IggyValueGetBooleanRS ( &m_iggyPath , m_checkedProp, nullptr, &checked );
	m_bChecked = checked;
	return checked;
}

bool UIControl_CheckBox::IsEnabled()
{
	return m_bEnabled;
}

void UIControl_CheckBox::SetEnable(bool enable)
{
	m_bEnabled = enable;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = enable;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcEnable , 1 , value );
}

// 4J HEG - this is only ever used when required, most of this should happen in the flash
void UIControl_CheckBox::setChecked(bool checked)
{
 	IggyDataValue result;
 	IggyDataValue value[1];
 	value[0].type = IGGY_DATATYPE_boolean;
 	value[0].boolval = checked;
 	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetCheckBox , 1 , value );
}

// 4J-TomK we need to trigger this one via function instead of key down event because of how it works
void UIControl_CheckBox::TouchSetCheckbox(bool checked)
{
 	IggyDataValue result;
 	IggyDataValue value[1];
 	value[0].type = IGGY_DATATYPE_boolean;
 	value[0].boolval = checked;
 	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetCheckBox , 1 , value );
}

void UIControl_CheckBox::ReInit()
{
	UIControl_Base::ReInit();

	init(m_label, m_id, m_bChecked);
}
