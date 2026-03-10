#pragma once

#include "UIControl_Base.h"

class UIControl_SpaceIndicatorBar : public UIControl_Base
{
private:
	IggyName m_setSaveSizeFunc, m_setTotalSizeFunc, m_setSaveGameOffsetFunc;
	int64_t m_min;
	int64_t m_max;
	int64_t m_currentSave, m_currentTotal;
	float m_currentOffset;

	vector<pair<int64_t,float> > m_sizeAndOffsets;

public:
	UIControl_SpaceIndicatorBar();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(UIString label, int id, int64_t min, int64_t max);
	virtual void ReInit();
	void reset();

	void addSave(int64_t size);
	void selectSave(int index);


private:
	void setSaveSize(int64_t size);
	void setTotalSize(int64_t totalSize);
	void setSaveGameOffset(float offset);
};