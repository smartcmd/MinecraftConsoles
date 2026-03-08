#include "stdafx.h"
#include "UI.h"
#include "UIControl.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/JavaMath.h"

UIControl_Base::UIControl_Base()
{
	m_bLabelChanged = false;
	m_label;
	m_id = 0;
}

bool UIControl_Base::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	bool success = UIControl::setupControl(scene,parent,controlName);

	m_setLabelFunc = registerFastName(L"SetLabel");
	m_initFunc = registerFastName(L"Init");
	m_funcGetLabel = registerFastName(L"GetLabel");
	m_funcCheckLabelWidths = registerFastName(L"CheckLabelWidths");

	return success;
}

void UIControl_Base::tick()
{
	UIControl::tick();

	if ( m_label.needsUpdating() || m_bLabelChanged )
	{
		//app.DebugPrintf("Calling SetLabel - '%ls'\n", m_label.c_str());
		m_bLabelChanged = false;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_string_UTF16;
		IggyStringUTF16 stringVal;

		stringVal.string = (IggyUTF16*) m_label.c_str();
		stringVal.length = m_label.length();
		value[0].string16 = stringVal;

		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setLabelFunc , 1 , value );

		m_label.setUpdated();
	}
}

void UIControl_Base::setLabel(UIString label, bool instant, bool force)
{
	if( force || ((!m_label.empty() || !label.empty()) && m_label.compare(label) != 0) ) m_bLabelChanged = true;
	m_label = label;

	if(m_bLabelChanged && instant)
	{
		m_bLabelChanged = false;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_string_UTF16;
		IggyStringUTF16 stringVal;

		stringVal.string = (IggyUTF16*)m_label.c_str();
		stringVal.length = m_label.length();
		value[0].string16 = stringVal;

		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_setLabelFunc , 1 , value );
	}
}

const wchar_t* UIControl_Base::getLabel()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS(m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcGetLabel, 0, NULL);

	if(result.type == IGGY_DATATYPE_string_UTF16)
	{
		m_label = wstring((wchar_t *)result.string16.string, result.string16.length);
	}

	return m_label.c_str();
}

void UIControl_Base::setAllPossibleLabels(int labelCount, wchar_t labels[][256])
{
	IggyDataValue result;
	IggyDataValue *value = new IggyDataValue[labelCount];
	IggyStringUTF16 * stringVal = new IggyStringUTF16[labelCount];

	for(unsigned int i = 0; i < labelCount; ++i)
	{
		stringVal[i].string = (IggyUTF16 *)labels[i];
		stringVal[i].length = wcslen(labels[i]);
		value[i].type = IGGY_DATATYPE_string_UTF16;
		value[i].string16 = stringVal[i];
	}

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcCheckLabelWidths , labelCount , value );

	delete [] value;
	delete [] stringVal;
}

bool UIControl_Base::hasFocus()
{
	return m_parentScene->controlHasFocus( this );
}
