#pragma once

#include "UIControl_Base.h"

class UIControl_Label : public UIControl_Base
{
private:
	bool m_reinitEnabled;

public:
	UIControl_Label();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(UIString label);
	virtual void ReInit();

	void disableReinitialisation() { m_reinitEnabled = false; }
};