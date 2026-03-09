#include "stdafx.h"
#include "UI.h"
#include "UIControl_SaveList.h"

bool UIControl_SaveList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eSaveList);
	bool success = UIControl_ButtonList::setupControl(scene,parent,controlName);

	//SlotList specific initialisers
	m_funcSetTextureName = registerFastName(L"SetTextureName");

	return success;
}

void UIControl_SaveList::addItem(const wstring &label)
{
	addItem(label, L"");
}

void UIControl_SaveList::addItem(const string &label)
{
	addItem(label, L"");
}

void UIControl_SaveList::addItem(const wstring &label, int data)
{
	addItem(label, L"", data);
}

void UIControl_SaveList::addItem(const string &label, int data)
{
	addItem(label, L"", data);
}

void UIControl_SaveList::addItem(const string &label, const wstring &iconName)
{
	addItem(label, iconName, m_itemCount);
	++m_itemCount;
}

void UIControl_SaveList::addItem(const wstring &label, const wstring &iconName)
{
	addItem(label, iconName, m_itemCount);
	++m_itemCount;
}

void UIControl_SaveList::addItem(const string &label, const wstring &iconName, int data)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF8 stringVal;
	stringVal.string = (char*)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF8;
	value[0].string8 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = m_itemCount;

	IggyStringUTF16 stringVal2;
	stringVal2.string = (IggyUTF16*)iconName.c_str();
	stringVal2.length = iconName.length();
	value[2].type = IGGY_DATATYPE_string_UTF16;
	value[2].string16 = stringVal2;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 3 , value );
}

void UIControl_SaveList::addItem(const wstring &label, const wstring &iconName, int data)
{
	IggyDataValue result;
	IggyDataValue value[3];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = m_itemCount;

	IggyStringUTF16 stringVal2;
	stringVal2.string = (IggyUTF16*)iconName.c_str();
	stringVal2.length = iconName.length();
	value[2].type = IGGY_DATATYPE_string_UTF16;
	value[2].string16 = stringVal2;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 3 , value );
}

void UIControl_SaveList::setTextureName(int iId, const wstring &iconName)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iId;

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)iconName.c_str();
	stringVal.length = iconName.length();
	value[1].type = IGGY_DATATYPE_string_UTF16;
	value[1].string16 = stringVal;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcSetTextureName , 2 , value );
}
