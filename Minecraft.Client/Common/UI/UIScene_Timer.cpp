#include "stdafx.h"
#include "UI.h"
#include "UIScene_Timer.h"


UIScene_Timer::UIScene_Timer(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	// In normal usage, we want to hide the new background that's used during texture pack reloading
	if(initData == 0)
	{
		m_controlBackground.setVisible(false);
	}
}

wstring UIScene_Timer::getMoviePath()
{
	return L"Timer";
}

void UIScene_Timer::reloadMovie(bool force)
{
	// Never needs reloaded
}

bool UIScene_Timer::needsReloaded()
{
	// Never needs reloaded
	return false;
}