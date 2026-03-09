#include "stdafx.h"
#include "UI.h"
#include "UIControl_DynamicLabel.h"

UIControl_DynamicLabel::UIControl_DynamicLabel()
{
}

bool UIControl_DynamicLabel::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eDynamicLabel);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Label specific initialisers
	m_funcAddText = registerFastName(L"AddText");
	m_funcTouchScroll = registerFastName(L"TouchScroll");
	m_funcGetRealWidth = registerFastName(L"GetRealWidth");
	m_funcGetRealHeight = registerFastName(L"GetRealHeight");

	return success;
}

void UIControl_DynamicLabel::addText(const wstring &text, bool bLastEntry)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)text.c_str();
	stringVal.length = text.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = bLastEntry;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcAddText , 2 , value );
}

void UIControl_DynamicLabel::ReInit()
{
	UIControl_Base::ReInit();
}

void UIControl_DynamicLabel::SetupTouch()
{
 #ifdef __PSVITA__
 	// 4J-TomK - add this dynamic label to the vita touch box list
 
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

void UIControl_DynamicLabel::TouchScroll(S32 iY, bool bActive)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iY;
	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = bActive;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcTouchScroll, 2 , value );
}

S32 UIControl_DynamicLabel::GetRealWidth()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcGetRealWidth, 0 , nullptr );

	S32 iRealWidth = m_width;
	if(result.type == IGGY_DATATYPE_number)
	{
		iRealWidth = static_cast<S32>(result.number);
	}
	return iRealWidth;
}

S32 UIControl_DynamicLabel::GetRealHeight()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcGetRealHeight, 0 , nullptr );

	S32 iRealHeight = m_height;
	if(result.type == IGGY_DATATYPE_number)
	{
		iRealHeight = static_cast<S32>(result.number);
	}
	return iRealHeight;
}