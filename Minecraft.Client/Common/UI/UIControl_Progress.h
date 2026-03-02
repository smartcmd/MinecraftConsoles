#pragma once

#include "UIControl_Base.h"

class UIControl_Progress : public UIControl_Base
{
private:
	IggyName m_setProgressFunc, m_showBarFunc;
	int m_min;
	int m_max;
	int m_current;
	float m_lastPercent;
	bool m_showingBar;

public:
	UIControl_Progress();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(UIString label, int id, int min, int max, int current);
	virtual void ReInit();

	void setProgress(int current);
	void showBar(bool show);
};