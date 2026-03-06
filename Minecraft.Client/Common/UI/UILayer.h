#pragma once
#include "UIEnums.h"
using namespace std;
class UIScene;
class UIGroup;

// A layer include a collection of scenes and other components
class UILayer
{
private:
	vector<UIScene *> m_sceneStack; // Operates as a stack mainly, but we may wish to iterate over all elements
	vector<UIScene *> m_components; // Other componenents in this scene that to do not conform the the user nav stack, and cannot take focus
	vector<UIScene *> m_scenesToDelete; // A list of scenes to delete
	vector<UIScene *> m_scenesToDestroy; // A list of scenes where we want to dump the swf

#ifdef __ORBIS__
	unordered_map<EUIScene,std::pair<int,bool>,std::hash<int>> m_componentRefCount;
#else
	unordered_map<EUIScene,pair<int,bool> > m_componentRefCount;
#endif

public:
	bool m_hasFocus; // True if the layer "has focus", should be the only layer in the group
	bool m_bMenuDisplayed;
	bool m_bPauseMenuDisplayed;
	bool m_bContainerMenuDisplayed;
	bool m_bIgnoreAutosaveMenuDisplayed;
	bool m_bIgnorePlayerJoinMenuDisplayed;

#ifdef __PSVITA__
	EUILayer m_iLayer;
#endif

	UIGroup *m_parentGroup;
public:
	UILayer(UIGroup *parent);

	void tick();
	void render(S32 width, S32 height, C4JRender::eViewportType viewport);
	void getRenderDimensions(S32 &width, S32 &height);

	void DestroyAll();
	void ReloadAll(bool force = false);

	// NAVIGATION
	bool NavigateToScene(int iPad, EUIScene scene, void *initData);
	bool NavigateBack(int iPad, EUIScene eScene);
	void removeScene(UIScene *scene);
	void closeAllScenes();
	UIScene *GetTopScene();

	bool GetMenuDisplayed();
	bool IsPauseMenuDisplayed() { return m_bPauseMenuDisplayed; }

	bool IsSceneInStack(EUIScene scene);
	bool HasFocus(int iPad);

	bool hidesLowerScenes();

	// A component is an element on a layer that displays BELOW other scenes in this layer, but does not engage in any navigation
	// E.g. you can keep a component active while performing navigation with other scenes on this layer
	void showComponent(int iPad, EUIScene scene, bool show);
	bool isComponentVisible(EUIScene scene);
	UIScene *addComponent(int iPad, EUIScene scene, void *initData = NULL);
	void removeComponent(EUIScene scene);

	// INPUT
	void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
#ifdef __PSVITA__
	// Current active scene
	UIScene *getCurrentScene();
#endif
	// FOCUS

	bool updateFocusState(bool allowedFocus = false);

public:
	bool IsFullscreenGroup();
	C4JRender::eViewportType getViewport();

	virtual void HandleDLCMountingComplete();
	virtual void HandleDLCInstalled();
#ifdef _XBOX_ONE
	virtual void HandleDLCLicenseChange();
#endif
	virtual void HandleMessage(EUIMessage message, void *data);

	void handleUnlockFullVersion();
	UIScene *FindScene(EUIScene sceneType);

	void PrintTotalMemoryUsage(int64_t &totalStatic, int64_t &totalDynamic);

};
