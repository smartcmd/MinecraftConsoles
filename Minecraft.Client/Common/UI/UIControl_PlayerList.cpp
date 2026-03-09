#include "stdafx.h"
#include "UI.h"
#include "UIControl_PlayerList.h"

bool UIControl_PlayerList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::ePlayerList);
	bool success = UIControl_ButtonList::setupControl(scene,parent,controlName);

	//SlotList specific initialisers
	m_funcSetPlayerIcon = registerFastName(L"SetPlayerIcon");
	m_funcSetVOIPIcon = registerFastName(L"SetVOIPIcon");

	return success;
}

void UIControl_PlayerList::addItem(const wstring &label, int iPlayerIcon, int iVOIPIcon)
{
	IggyDataValue result;
	IggyDataValue value[4];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = m_itemCount;

	value[2].type = IGGY_DATATYPE_number;
	value[2].number = iPlayerIcon + 1;

	value[3].type = IGGY_DATATYPE_number;
	value[3].number = iVOIPIcon + 1;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 4 , value );

	++m_itemCount;
}

void UIControl_PlayerList::setPlayerIcon(int iId, int iPlayerIcon)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iId;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iPlayerIcon + 1;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcSetPlayerIcon , 2 , value );
}

void UIControl_PlayerList::setVOIPIcon(int iId, int iVOIPIcon)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iId;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iVOIPIcon + 1;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcSetVOIPIcon , 2 , value );
}
