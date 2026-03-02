#include "stdafx.h"
#include "UI.h"
#include "UIControl_Slider.h"

UIControl_Slider::UIControl_Slider()
{
	m_id = 0;
	m_min = 0;
	m_max = 100;
	m_current = 0;
}

bool UIControl_Slider::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eSlider);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Slider specific initialisers
	m_funcSetRelativeSliderPos = registerFastName(L"SetRelativeSliderPos");
	m_funcGetRealWidth = registerFastName(L"GetRealWidth");

	return success;
}

void UIControl_Slider::init(UIString label, int id, int min, int max, int current)
{
	m_label = label;
	m_id = id;
	m_min = min;
	m_max = max;
	m_current = current;

	IggyDataValue result;
	IggyDataValue value[5];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (int)id;

	value[2].type = IGGY_DATATYPE_number;
	value[2].number = (int)min;

	value[3].type = IGGY_DATATYPE_number;
	value[3].number = (int)max;

	value[4].type = IGGY_DATATYPE_number;
	value[4].number = (int)current;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 5 , value );

#ifdef __PSVITA__
	// 4J-TomK - add slider to the vita touch box list

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

void UIControl_Slider::handleSliderMove(int newValue)
{
	if (m_current!=newValue)
	{
		ui.PlayUISFX(eSFX_Scroll);
		m_current = newValue;

		if(newValue < m_allPossibleLabels.size())
		{
			setLabel(m_allPossibleLabels[newValue]);
		}
	}
}

void UIControl_Slider::SetSliderTouchPos(float fTouchPos)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = fTouchPos;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetRelativeSliderPos , 1 , value );
	}

S32 UIControl_Slider::GetRealWidth()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcGetRealWidth , 0 , NULL );

	S32 iRealWidth = m_width;
	if(result.type == IGGY_DATATYPE_number)
	{
		iRealWidth = (S32)result.number;
	}
	return iRealWidth;
}

void UIControl_Slider::setAllPossibleLabels(int labelCount, wchar_t labels[][256])
{
	m_allPossibleLabels.clear();
	for(unsigned int i = 0; i < labelCount; ++i)
	{
		m_allPossibleLabels.push_back(labels[i]);
	}
	UIControl_Base::setAllPossibleLabels(labelCount, labels);
}

void UIControl_Slider::ReInit()
{
	UIControl_Base::ReInit();

	init(m_label, m_id, m_min, m_max, m_current);
}
