#include "stdafx.h"
#include "UI.h"
#include "UIControl_Progress.h"

UIControl_Progress::UIControl_Progress()
{
	m_min = 0;
	m_max = 100;
	m_current = 0;
	m_lastPercent = 0.0f;
	m_showingBar = true;
}

bool UIControl_Progress::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eProgress);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Progress specific initialisers
	m_setProgressFunc = registerFastName(L"setProgress");
	m_showBarFunc = registerFastName(L"ShowBar");

	return success;
}

void UIControl_Progress::init(UIString label, int id, int min, int max, int current)
{
	m_label = label;
	m_id = id;
	m_min = min;
	m_max = max;
	m_current = current;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 1 , value );
}

void UIControl_Progress::ReInit()
{
	UIControl_Base::ReInit();
	init(m_label, m_id, m_min, m_max, m_current);
}

void UIControl_Progress::setProgress(int current)
{
	m_current = current;

	float percent = (float)((m_current-m_min))/(m_max-m_min);

	if(percent != m_lastPercent)
	{
		m_lastPercent = percent;
		//app.DebugPrintf("Setting progress value to %d/%f\n", m_current, percent);

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = percent;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setProgressFunc , 1 , value );
	}
}

void UIControl_Progress::showBar(bool show)
{
	if(show != m_showingBar)
	{
		m_showingBar = show;
		//app.DebugPrintf("Setting progress value to %d/%f\n", m_current, percent);

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_showBarFunc , 1 , value );
	}
}