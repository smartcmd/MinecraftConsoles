#include "stdafx.h"
#include "UI.h"
#include "UIControl_TextInput.h"

UIControl_TextInput::UIControl_TextInput()
{
	m_bHasFocus = false;
}

bool UIControl_TextInput::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eTextInput);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//TextInput specific initialisers
	m_textName = registerFastName(L"text");
	m_funcChangeState = registerFastName(L"ChangeState");
	m_funcSetCharLimit = registerFastName(L"SetCharLimit");

	return success;
}

void UIControl_TextInput::init(UIString label, int id)
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
 	// 4J-TomK - add this buttonlist to the vita touch box list
 
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

void UIControl_TextInput::ReInit()
{
	UIControl_Base::ReInit();

	init(m_label, m_id);
}

void UIControl_TextInput::setFocus(bool focus)
{
	if(m_bHasFocus != focus)
	{
		m_bHasFocus = focus;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = focus?0:1;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcChangeState , 1 , value );
	}
}

void UIControl_TextInput::SetCharLimit(int iLimit)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iLimit;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetCharLimit , 1 , value );
}
