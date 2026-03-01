#pragma once

#include "UIControl_Base.h"

class UIControl_TextInput : public UIControl_Base
{
private:
	IggyName m_textName, m_funcChangeState, m_funcSetCharLimit;
	bool m_bHasFocus;

public:
	UIControl_TextInput();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(UIString label, int id);
	void ReInit();

	virtual void setFocus(bool focus);

	void SetCharLimit(int iLimit);
};