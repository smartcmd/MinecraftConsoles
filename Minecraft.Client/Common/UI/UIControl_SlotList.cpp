#include "stdafx.h"
#include "UI.h"
#include "UIControl_SlotList.h"

UIControl_SlotList::UIControl_SlotList()
{
	m_lastHighlighted = -1;
}

bool UIControl_SlotList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eSlotList);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//SlotList specific initialisers
	m_addSlotFunc = registerFastName(L"addSlot");
	m_setRedBoxFunc = registerFastName(L"SetSlotRedBox");
	m_setHighlightFunc = registerFastName(L"SetSlotHighlight");

	m_lastHighlighted = 0;

	return success;
}

void UIControl_SlotList::ReInit()
{
	UIControl_Base::ReInit();

	m_lastHighlighted = -1;
}

void UIControl_SlotList::addSlot(int id)
{
	IggyDataValue result;
	IggyDataValue value[3];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = id;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = false;
	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = false;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addSlotFunc ,3 , value );
}

void UIControl_SlotList::addSlots(int iStartValue, int iCount)
{
	for(unsigned int i = iStartValue; i < iStartValue + iCount; ++i)
	{
		addSlot(i);
	}
}


void UIControl_SlotList::setHighlightSlot(int index)
{
	if(index != m_lastHighlighted)
	{
		if(m_lastHighlighted != -1)
		{
			setSlotHighlighted(m_lastHighlighted, false);
		}
		setSlotHighlighted(index, true);
		m_lastHighlighted = index;
	}
}

void UIControl_SlotList::setSlotHighlighted(int index, bool highlight)
{
	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = highlight;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_setHighlightFunc , 2 , value );
}

void UIControl_SlotList::showSlotRedBox(int index, bool show)
{
	//app.DebugPrintf("Setting red box at index %d to %s\n", index, show?"on":"off");
	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = show;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_setRedBoxFunc , 2, value );
}

void UIControl_SlotList::setFocus(bool focus)
{
	if(m_lastHighlighted != -1)
	{
		if(focus) setSlotHighlighted(m_lastHighlighted, true);
		else setSlotHighlighted(m_lastHighlighted, false);
	}
}
