#pragma once

#include "UIControl_Base.h"

class UIControl_SpaceIndicatorBar : public UIControl_Base
{
private:
	IggyName m_setSaveSizeFunc, m_setTotalSizeFunc, m_setSaveGameOffsetFunc;
	__int64 m_min;
	__int64 m_max;
	__int64 m_currentSave, m_currentTotal;
	float m_currentOffset;

	vector<pair<__int64,float> > m_sizeAndOffsets;

public:
	UIControl_SpaceIndicatorBar();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(const wstring &label, int id, __int64 min, __int64 max);
	virtual void ReInit();
	void reset();

	void addSave(__int64 size);
	void selectSave(int index);


private:
	void setSaveSize(__int64 size);
	void setTotalSize(__int64 totalSize);
	void setSaveGameOffset(float offset);
};