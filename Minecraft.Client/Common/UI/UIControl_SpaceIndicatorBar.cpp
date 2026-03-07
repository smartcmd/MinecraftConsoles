#include "stdafx.h"
#include "UI.h"
#include "UIControl_SpaceIndicatorBar.h"

UIControl_SpaceIndicatorBar::UIControl_SpaceIndicatorBar()
{
	m_min = 0;
	m_max = 100;
	m_currentSave = 0;
	m_currentTotal = 0;
	m_currentOffset = 0.0f;
}

bool UIControl_SpaceIndicatorBar::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eProgress);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//Progress specific initialisers
	m_setSaveSizeFunc = registerFastName(L"setSaveGameSize");
	m_setTotalSizeFunc = registerFastName(L"setTotalSize");
	m_setSaveGameOffsetFunc = registerFastName(L"setSaveGameOffset");

	return success;
}

void UIControl_SpaceIndicatorBar::init(UIString label, int id, int64_t min, int64_t max)
{
	m_label = label;
	m_id = id;
	m_min = min;
	m_max = max;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 1 , value );
}

void UIControl_SpaceIndicatorBar::ReInit()
{
	UIControl_Base::ReInit();
	init(m_label, m_id, m_min, m_max);
	setSaveSize(m_currentSave);
	setTotalSize(m_currentTotal);
	setSaveGameOffset(m_currentOffset);
}

void UIControl_SpaceIndicatorBar::reset()
{
	m_sizeAndOffsets.clear();
	m_currentTotal = 0;
	setTotalSize(0);
	setSaveSize(0);
	setSaveGameOffset(0.0f);
}

void UIControl_SpaceIndicatorBar::addSave(int64_t size)
{
	float startPercent = (float)((m_currentTotal-m_min))/(m_max-m_min);

	m_sizeAndOffsets.push_back( pair<int64_t, float>(size, startPercent) );

	m_currentTotal += size;
	setTotalSize(m_currentTotal);
}

void UIControl_SpaceIndicatorBar::selectSave(int index)
{
	if(index >= 0 && index < m_sizeAndOffsets.size())
	{
		pair<int64_t,float> values = m_sizeAndOffsets[index];
		setSaveSize(values.first);
		setSaveGameOffset(values.second);
	}
	else
	{
		setSaveSize(0);
		setSaveGameOffset(0);
	}
}

void UIControl_SpaceIndicatorBar::setSaveSize(int64_t size)
{
	m_currentSave = size;

	float percent = (float)((m_currentSave-m_min))/(m_max-m_min);

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = percent;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setSaveSizeFunc , 1 , value );
}

void UIControl_SpaceIndicatorBar::setTotalSize(int64_t size)
{
	float percent = (float)((m_currentTotal-m_min))/(m_max-m_min);

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = percent;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setTotalSizeFunc , 1 , value );
}

void UIControl_SpaceIndicatorBar::setSaveGameOffset(float offset)
{
	m_currentOffset = offset;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = m_currentOffset;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setSaveGameOffsetFunc , 1 , value );
}