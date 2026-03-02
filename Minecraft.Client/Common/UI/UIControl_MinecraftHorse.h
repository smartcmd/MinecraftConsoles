#pragma once

#include "UIControl.h"

class UIControl_MinecraftHorse : public UIControl
{
private:
	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;

public:
	UIControl_MinecraftHorse();

	void render(IggyCustomDrawCallbackRegion *region);
};