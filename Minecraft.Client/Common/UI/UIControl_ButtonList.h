#pragma once

#include "UIControl_Base.h"

class UIControl_ButtonList : public UIControl_Base
{
protected:
	IggyName m_addNewItemFunc, m_removeAllItemsFunc, m_funcHighlightItem, m_funcRemoveItem, m_funcSetButtonLabel, m_funcSetTouchFocus, m_funcCanTouchTrigger;

	int m_itemCount;
	int m_iCurrentSelection;

public:
	UIControl_ButtonList();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(int id);
	virtual void ReInit();

	void clearList();

	void addItem(const wstring &label);
	void addItem(const string &label);

	void addItem(const wstring &label, int data);
	void addItem(const string &label, int data);

	void removeItem(int index);

	int getItemCount() { return m_itemCount; }

	void setCurrentSelection(int iSelection);
	int getCurrentSelection();

	void updateChildFocus(int iChild);

	void setButtonLabel(int iButtonId, const wstring &label);

#if defined(__PSVITA__) || defined(_WINDOWS64)
	void SetTouchFocus(S32 iX, S32 iY, bool bRepeat);
	bool CanTouchTrigger(S32 iX, S32 iY);
#endif

};

class UIControl_DynamicButtonList : public UIControl_ButtonList
{
protected:
	vector<UIString> m_labels;

public:
	virtual void tick();

	virtual void addItem(UIString label, int data = -1);

	virtual void removeItem(int index);
};