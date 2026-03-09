#include "stdafx.h"
#include "UI.h"
#include "UIControl_HTMLLabel.h"

UIControl_HTMLLabel::UIControl_HTMLLabel()
{
}

bool UIControl_HTMLLabel::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eHTMLLabel);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Label specific initialisers
	m_funcStartAutoScroll = registerFastName(L"StartAutoScroll");
	m_funcTouchScroll = registerFastName(L"TouchScroll");
	m_funcGetRealWidth = registerFastName(L"GetRealWidth");
	m_funcGetRealHeight = registerFastName(L"GetRealHeight");

	return success;
}

void UIControl_HTMLLabel::startAutoScroll()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcStartAutoScroll , 0 , nullptr );
}

void UIControl_HTMLLabel::ReInit()
{
	UIControl_Base::ReInit();
	// Don't set the label, HTML sizes will have changed. Let the scene update us.
	init(L"");
}

void UIControl_HTMLLabel::setLabel(const string &label)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_string_UTF8;
	IggyStringUTF8 stringVal;

	stringVal.string = (char *) label.c_str();
	stringVal.length = label.length();
	value[0].string8 = stringVal;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setLabelFunc , 1 , value );
}

void UIControl_HTMLLabel::SetupTouch()
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

void UIControl_HTMLLabel::TouchScroll(S32 iY, bool bActive)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iY;
	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = bActive;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcTouchScroll, 2 , value );
}

S32 UIControl_HTMLLabel::GetRealWidth()
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

S32 UIControl_HTMLLabel::GetRealHeight()
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