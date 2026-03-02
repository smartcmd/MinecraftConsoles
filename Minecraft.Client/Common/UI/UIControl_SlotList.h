#pragma once

#include "UIControl_Base.h"

class UIControl_SlotList : public UIControl_Base
{
private:
	//IggyName m_addSlotFunc, m_getSlotFunc, m_setRedBoxFunc, m_setHighlightFunc;
	IggyName m_addSlotFunc, m_setRedBoxFunc, m_setHighlightFunc;

	int m_lastHighlighted;

public:
	UIControl_SlotList();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);
	
	virtual void ReInit();

	void addSlot(int id);
	void addSlots(int iStartValue, int iCount);

	void setHighlightSlot(int index);
	void showSlotRedBox(int index, bool show);

	virtual void setFocus(bool focus);

private:
	void setSlotHighlighted(int index, bool highlight);
};
