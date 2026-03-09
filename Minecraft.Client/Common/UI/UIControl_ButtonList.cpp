#include "stdafx.h"
#include "UI.h"
#include "UIControl_ButtonList.h"

UIControl_ButtonList::UIControl_ButtonList()
{
	m_itemCount = 0;
	m_iCurrentSelection = 0;
}

bool UIControl_ButtonList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eButtonList);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//SlotList specific initialisers
	m_addNewItemFunc = registerFastName(L"addNewItem");
	m_removeAllItemsFunc = registerFastName(L"removeAllItems");
	m_funcHighlightItem = registerFastName(L"HighlightItem");
	m_funcRemoveItem = registerFastName(L"RemoveItem");
	m_funcSetButtonLabel = registerFastName(L"SetButtonLabel");
	m_funcSetTouchFocus = registerFastName(L"SetTouchFocus");
	m_funcCanTouchTrigger = registerFastName(L"CanTouchTrigger");

	return success;
}

void UIControl_ButtonList::init(int id)
{
	m_id = id;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = id;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 1 , value );

 #ifdef __PSVITA__
 	// 4J-PB - add this buttonlist to the vita touch box list
 
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

void UIControl_ButtonList::ReInit()
{
	UIControl_Base::ReInit();
	init(m_id);
	m_itemCount = 0;
	m_iCurrentSelection = 0;
}

void UIControl_ButtonList::clearList()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_removeAllItemsFunc , 0 , nullptr );

	m_itemCount = 0;
}

void UIControl_ButtonList::addItem(const string &label)
{
	addItem(label, m_itemCount);
}

void UIControl_ButtonList::addItem(const wstring &label)
{
	addItem(label, m_itemCount);
}

void UIControl_ButtonList::addItem(const string &label, int data)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF8 stringVal;
	stringVal.string = (char*)label.c_str();
	stringVal.length = static_cast<S32>(label.length());
	value[0].type = IGGY_DATATYPE_string_UTF8;
	value[0].string8 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = data;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 2 , value );

	++m_itemCount;
}

void UIControl_ButtonList::addItem(const wstring &label, int data)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = data;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_addNewItemFunc , 2 , value );

	++m_itemCount;
}

void UIControl_ButtonList::removeItem(int index)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcRemoveItem , 1 , value );

	--m_itemCount;
}

void UIControl_ButtonList::setCurrentSelection(int iSelection)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iSelection;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcHighlightItem , 1 , value );
}

int UIControl_ButtonList::getCurrentSelection()
{
	return m_iCurrentSelection;
}

void UIControl_ButtonList::updateChildFocus(int iChild)
{
	m_iCurrentSelection = iChild;
}

void UIControl_ButtonList::setButtonLabel(int iButtonId, const wstring &label)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iButtonId;

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[1].type = IGGY_DATATYPE_string_UTF16;
	value[1].string16 = stringVal;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcSetButtonLabel, 2 , value );
}

#if defined(__PSVITA__) || defined(_WINDOWS64)
void UIControl_ButtonList::SetTouchFocus(S32 iX, S32 iY, bool bRepeat)
{
	IggyDataValue result;
	IggyDataValue value[3];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iX;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iY;
	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = bRepeat;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcSetTouchFocus, 3 , value );
}

bool UIControl_ButtonList::CanTouchTrigger(S32 iX, S32 iY)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iX;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iY;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcCanTouchTrigger, 2 , value );

	S32 bCanTouchTrigger = false;
	if(result.type == IGGY_DATATYPE_boolean)
	{
		bCanTouchTrigger = (bool)result.boolval;
	}
	return bCanTouchTrigger;
}
#endif


void UIControl_DynamicButtonList::tick()
{
	UIControl_ButtonList::tick();

	int buttonIndex = 0;
	vector<UIString>::iterator itr;
	for (itr = m_labels.begin(); itr != m_labels.end(); itr++)
	{
		if ( itr->needsUpdating() )
		{
			setButtonLabel(buttonIndex, itr->getString());
			itr->setUpdated();
		}
		buttonIndex++;
	}
}

void UIControl_DynamicButtonList::addItem(UIString label, int data)
{
	if (data < 0) data = m_itemCount;

	if (data < m_labels.size())
	{
		m_labels[data] = label;
	}
	else
	{
		while (data > m_labels.size()) 
		{
			m_labels.push_back(UIString());
		}
		m_labels.push_back(label);
	}

	UIControl_ButtonList::addItem(label.getString(), data);
}

void UIControl_DynamicButtonList::removeItem(int index)
{
	m_labels.erase( m_labels.begin() + index );
	UIControl_ButtonList::removeItem(index);
}