#include "stdafx.h"
#include "UI.h"
#include "UIControl.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\JavaMath.h"

UIControl::UIControl()
{
	m_parentScene = NULL;
	m_lastOpacity = 1.0f;
	m_controlName = "";
	m_isVisible = true;
	m_bHidden = false;
	m_eControlType = eNoControl;
	m_id = -1;
	m_pParentPanel = NULL;
}

bool UIControl::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	m_parentScene = scene;
	m_controlName = controlName;

	rrbool res = IggyValuePathMakeNameRef ( &m_iggyPath , parent , controlName.c_str() );

	m_nameXPos = registerFastName(L"x");
	m_nameYPos = registerFastName(L"y");
	m_nameWidth = registerFastName(L"width");
	m_nameHeight = registerFastName(L"height");
	m_funcSetAlpha = registerFastName(L"SetControlAlpha");
	m_nameVisible = registerFastName(L"visible");

	F64 fx, fy, fwidth, fheight;
	IggyValueGetF64RS( getIggyValuePath() , m_nameXPos , NULL , &fx ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameYPos , NULL , &fy ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameWidth , NULL , &fwidth ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameHeight , NULL , &fheight );

	m_x = (S32)fx;
	m_y = (S32)fy;
	m_width = (S32)Math::round(fwidth);
	m_height = (S32)Math::round(fheight);

	return res;
}

void UIControl::UpdateControl()
{
	F64 fx, fy, fwidth, fheight;
	IggyValueGetF64RS( getIggyValuePath() , m_nameXPos , NULL , &fx ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameYPos , NULL , &fy ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameWidth , NULL , &fwidth ); 
	IggyValueGetF64RS( getIggyValuePath() , m_nameHeight , NULL , &fheight );
	m_x = (S32)fx;
	m_y = (S32)fy;
	m_width = (S32)Math::round(fwidth);
	m_height = (S32)Math::round(fheight);
}

void UIControl::ReInit()
{
	if(m_lastOpacity != 1.0f)
	{
		IggyDataValue result;
		IggyDataValue value[2];
		IggyStringUTF8 stringVal;

		stringVal.string = (char *)m_controlName.c_str();
		stringVal.length = m_controlName.length();
		value[0].type = IGGY_DATATYPE_string_UTF8;
		value[0].string8 = stringVal;

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = m_lastOpacity;

		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, m_parentScene->m_rootPath , m_funcSetAlpha , 2 , value );
	}

	IggyValueSetBooleanRS( getIggyValuePath(), m_nameVisible, NULL, m_isVisible );
}

IggyValuePath *UIControl::getIggyValuePath()
{
	return &m_iggyPath;
}

S32 UIControl::getXPos()
{
	return m_x;
}

S32 UIControl::getYPos()
{
	return m_y;
}

S32 UIControl::getWidth()
{
	return m_width;
}

S32 UIControl::getHeight()
{
	return m_height;
}

void UIControl::setOpacity(float percent)
{
	if(percent != m_lastOpacity)
	{
		m_lastOpacity = percent;

		IggyDataValue result;
		IggyDataValue value[2];
		IggyStringUTF8 stringVal;

		stringVal.string = (char *)m_controlName.c_str();
		stringVal.length = m_controlName.length();
		value[0].type = IGGY_DATATYPE_string_UTF8;
		value[0].string8 = stringVal;

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = m_lastOpacity;

		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, m_parentScene->m_rootPath , m_funcSetAlpha , 2 , value );
	}
}

void UIControl::setVisible(bool visible)
{
	if(visible != m_isVisible)
	{
		rrbool succ = IggyValueSetBooleanRS( getIggyValuePath(), m_nameVisible, NULL, visible );
		if(succ) m_isVisible = visible;
		else app.DebugPrintf("Failed to set visibility for control\n");
	}
}

bool UIControl::getVisible()
{
	rrbool bVisible = false;
	
	IggyResult result = IggyValueGetBooleanRS ( getIggyValuePath() , m_nameVisible, NULL, &bVisible );
	
	m_isVisible = bVisible;
	
	return bVisible;
}

IggyName UIControl::registerFastName(const wstring &name)
{
	return m_parentScene->registerFastName(name);
}
