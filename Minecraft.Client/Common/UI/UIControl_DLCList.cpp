#include "stdafx.h"
#include "UI.h"
#include "UIControl_DLCList.h"

bool UIControl_DLCList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eDLCList);
	bool success = UIControl_ButtonList::setupControl(scene,parent,controlName);

	//SlotList specific initialisers
	m_funcShowTick = registerFastName(L"ShowTick");

	return success;
}

void UIControl_DLCList::addItem(const string &label, bool showTick, int iId)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF8 stringVal;
	stringVal.string = (char*)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF8;
	value[0].string8 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iId;

	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = showTick;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 3 , value );

	++m_itemCount;
}

void UIControl_DLCList::addItem(const wstring &label, bool showTick, int iId)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16 *)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iId;

	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = showTick;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 3 , value );

	++m_itemCount;
}

void UIControl_DLCList::showTick(int iId, bool showTick)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iId;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = showTick;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcShowTick , 2 , value );
}
