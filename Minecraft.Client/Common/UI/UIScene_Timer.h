#pragma once

#include "UIScene.h"

class UIScene_Timer : public UIScene
{
private:
	UIControl m_controlBackground;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT(m_controlBackground,"Background")
	UI_END_MAP_ELEMENTS_AND_NAMES()

public:
	using UIScene::reloadMovie;

	UIScene_Timer(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_Timer;}

	// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
	virtual bool hidesLowerScenes() { return true; }
	virtual void reloadMovie(bool force);
	virtual bool needsReloaded();

protected:
	virtual wstring getMoviePath();
};
