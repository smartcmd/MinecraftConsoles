#include "stdafx.h"
#include "UIGroup.h"

UIGroup::UIGroup(EUIGroup group, int iPad)
{
	m_group = group;
	m_iPad = iPad;
	m_bMenuDisplayed = false;
	m_bPauseMenuDisplayed = false;
	m_bContainerMenuDisplayed = false;
	m_bIgnoreAutosaveMenuDisplayed = false;
	m_bIgnorePlayerJoinMenuDisplayed = false;

	m_updateFocusStateCountdown = 0;

	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i] = new UILayer(this);
#ifdef __PSVITA__
		m_layers[i]->m_iLayer=(EUILayer)i;
#endif
	}

	m_tooltips = (UIComponent_Tooltips *)m_layers[(int)eUILayer_Tooltips]->addComponent(0, eUIComponent_Tooltips);

	m_tutorialPopup = NULL;
	m_hud = NULL;
	m_pressStartToPlay = NULL;
	if(m_group != eUIGroup_Fullscreen)
	{
		m_tutorialPopup = (UIComponent_TutorialPopup *)m_layers[(int)eUILayer_Popup]->addComponent(m_iPad, eUIComponent_TutorialPopup);

		m_hud = (UIScene_HUD *)m_layers[(int)eUILayer_HUD]->addComponent(m_iPad, eUIScene_HUD);

		//m_layers[(int)eUILayer_Chat]->addComponent(m_iPad, eUIComponent_Chat);
	}
	else
	{
		m_pressStartToPlay = (UIComponent_PressStartToPlay *)m_layers[(int)eUILayer_Tooltips]->addComponent(0, eUIComponent_PressStartToPlay);
	}

	m_viewportType = C4JRender::VIEWPORT_TYPE_FULLSCREEN;

	// 4J Stu - Pre-allocate this for cached rendering in scenes. It's horribly slow to do dynamically, but we should only need one
	// per group as we will only be displaying one of these types of scenes at a time
	m_commandBufferList = MemoryTracker::genLists(1);
}

void UIGroup::DestroyAll()
{
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->DestroyAll();
	}
}

void UIGroup::ReloadAll()
{
	// We only need to reload things when they are likely to be rendered
	int highestRenderable = 0;
	for(; highestRenderable < eUILayer_COUNT; ++highestRenderable)
	{
		if(m_layers[highestRenderable]->hidesLowerScenes()) break;
	}
	if(highestRenderable < eUILayer_Fullscreen) highestRenderable = eUILayer_Fullscreen;
	for(; highestRenderable >= 0; --highestRenderable)
	{
		if(highestRenderable < eUILayer_COUNT) m_layers[highestRenderable]->ReloadAll(highestRenderable != (int)eUILayer_Fullscreen);
	}
}

void UIGroup::tick()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->tick();

		// TODO: May wish to ignore ticking other layers here based on current layer
	}

	// Handle deferred update focus
	if (m_updateFocusStateCountdown > 0) 
	{
		m_updateFocusStateCountdown--;
		if (m_updateFocusStateCountdown == 0)_UpdateFocusState();
}
}

void UIGroup::render()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	S32 width = 0;
	S32 height = 0;
	ui.getRenderDimensions(m_viewportType, width, height);
	int highestRenderable = 0;
	for(; highestRenderable < eUILayer_COUNT; ++highestRenderable)
	{
		if(m_layers[highestRenderable]->hidesLowerScenes()) break;
	}
	for(; highestRenderable >= 0; --highestRenderable)
	{
		if(highestRenderable < eUILayer_COUNT) m_layers[highestRenderable]->render(width, height,m_viewportType);
	}
}

bool UIGroup::hidesLowerScenes()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return false;
	bool hidesScenes = false;
	for(int i = eUILayer_COUNT - 1; i >= 0; --i)
	{
		hidesScenes = m_layers[i]->hidesLowerScenes();
		if(hidesScenes) break;
	}
	return hidesScenes;
}

void UIGroup::getRenderDimensions(S32 &width, S32 &height)
{
	ui.getRenderDimensions(m_viewportType, width, height);
}

// NAVIGATION
bool UIGroup::NavigateToScene(int iPad, EUIScene scene, void *initData, EUILayer layer)
{
	bool succeeded = m_layers[(int)layer]->NavigateToScene(iPad, scene, initData);
	updateStackStates();
	return succeeded;
}

bool UIGroup::NavigateBack(int iPad, EUIScene eScene, EUILayer eLayer)
{
	// Keep navigating back on every layer until we hit the target scene
	bool foundTarget = false;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		if(eLayer < eUILayer_COUNT && eLayer != i) continue;
		foundTarget = m_layers[i]->NavigateBack(iPad, eScene);
		if(foundTarget) break;
	}
	updateStackStates();
	return foundTarget;
}

void UIGroup::closeAllScenes()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( m_iPad >= 0 )
	{
		if(pMinecraft != NULL && pMinecraft->localgameModes[m_iPad] != NULL )
		{
			TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];

			// This just allows it to be shown
			gameMode->getTutorial()->showTutorialPopup(true);
		}
	}

	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		// Ignore the error layer
		if(i != (int)eUILayer_Error) m_layers[i]->closeAllScenes();
	}
	updateStackStates();
}

UIScene *UIGroup::GetTopScene(EUILayer layer)
{
	return m_layers[(int)layer]->GetTopScene();
}

bool UIGroup::GetMenuDisplayed()
{
	return m_bMenuDisplayed;
}

bool UIGroup::IsSceneInStack(EUIScene scene)
{
	bool found = false;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		found = m_layers[i]->IsSceneInStack(scene);
		if(found) break;
	}
	return found;
}

bool UIGroup::HasFocus(int iPad)
{
	bool hasFocus = false;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		if( m_layers[i]->m_hasFocus)
		{
			if(m_layers[i]->HasFocus(iPad))
			{
				hasFocus = true;
			}
			break;
		}
	}
	return hasFocus;
}

#ifdef __PSVITA__
UIScene *UIGroup::getCurrentScene()
{
	UIScene *pScene;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		pScene=m_layers[i]->getCurrentScene();

		if(pScene!=NULL) return pScene;
	}

	return NULL;
}
#endif

// INPUT
void UIGroup::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->handleInput(iPad, key, repeat, pressed, released, handled);
		if(handled) break;
	}
}

// FOCUS 

// Check that a layer may recieve focus, specifically that there is no infocus layer above
bool UIGroup::RequestFocus(UILayer* layerPtr)
{
	// Find the layer
	unsigned int layerIndex = GetLayerIndex(layerPtr);

	// Top layer is always allowed focus
	if (layerIndex == 0) return true;

	// Check layers above to see if any of them have focus
	for (int i = layerIndex-1; i >= 0; i--)
	{
		if (m_layers[i]->m_hasFocus) return false;
	}

	return true;
}

void UIGroup::showComponent(int iPad, EUIScene scene, EUILayer layer, bool show)
{
	m_layers[layer]->showComponent(iPad, scene, show);
}

UIScene *UIGroup::addComponent(int iPad, EUIScene scene, EUILayer layer)
{
	return m_layers[layer]->addComponent(iPad, scene);
}

void UIGroup::removeComponent(EUIScene scene, EUILayer layer)
{
	m_layers[layer]->removeComponent(scene);
}

void UIGroup::SetViewportType(C4JRender::eViewportType type)
{
	if(m_viewportType != type)
	{
		m_viewportType = type;
		for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
		{
			m_layers[i]->ReloadAll(true);
		}
	}
}

C4JRender::eViewportType UIGroup::GetViewportType()
{
	return m_viewportType;
}

void UIGroup::HandleDLCMountingComplete()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		app.DebugPrintf("UIGroup::HandleDLCMountingComplete - m_layers[%d]\n",i);
		m_layers[i]->HandleDLCMountingComplete();
	}
}

void UIGroup::HandleDLCInstalled()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->HandleDLCInstalled();
	}
}

#ifdef _XBOX_ONE
void UIGroup::HandleDLCLicenseChange()
{
	// Ignore this group if the player isn't signed in
	if(m_iPad >= 0 && !ProfileManager.IsSignedIn(m_iPad)) return;
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->HandleDLCLicenseChange();
	}
}
#endif

bool UIGroup::IsFullscreenGroup()
{
	return m_group == eUIGroup_Fullscreen;
}


void UIGroup::handleUnlockFullVersion()
{
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_layers[i]->handleUnlockFullVersion();
	}
}

void UIGroup::updateStackStates()
{
	m_bMenuDisplayed = false;
	m_bPauseMenuDisplayed = false;
	m_bContainerMenuDisplayed = false;
	m_bIgnoreAutosaveMenuDisplayed = false;
	m_bIgnorePlayerJoinMenuDisplayed = false;

	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		m_bMenuDisplayed = m_bMenuDisplayed || m_layers[i]->m_bMenuDisplayed;
		m_bPauseMenuDisplayed = m_bPauseMenuDisplayed || m_layers[i]->m_bPauseMenuDisplayed;
		m_bContainerMenuDisplayed = m_bContainerMenuDisplayed || m_layers[i]->m_bContainerMenuDisplayed;
		m_bIgnoreAutosaveMenuDisplayed = m_bIgnoreAutosaveMenuDisplayed || m_layers[i]->m_bIgnoreAutosaveMenuDisplayed;
		m_bIgnorePlayerJoinMenuDisplayed = m_bIgnorePlayerJoinMenuDisplayed || m_layers[i]->m_bIgnorePlayerJoinMenuDisplayed;
	}
}

// Defer update focus till for 10 UI ticks
void UIGroup::UpdateFocusState()
{
	m_updateFocusStateCountdown = 10;
}

// Pass focus to uppermost layer that accepts focus
void UIGroup::_UpdateFocusState()
{
	bool groupFocusSet = false;

	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		groupFocusSet = m_layers[i]->updateFocusState(true);
		if (groupFocusSet) break;
	}
}

// Get the index of the layer
unsigned int UIGroup::GetLayerIndex(UILayer* layerPtr)
{
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		if (m_layers[i] == layerPtr) return i;
	}

	// can't get here...
	return 0;
}
	
void UIGroup::PrintTotalMemoryUsage(__int64 &totalStatic, __int64 &totalDynamic)
{
	__int64 groupStatic = 0;
	__int64 groupDynamic = 0;
	app.DebugPrintf(app.USER_SR, "-- BEGIN GROUP %d\n",m_group);
	for(unsigned int i = 0; i < eUILayer_COUNT; ++i)
	{
		app.DebugPrintf(app.USER_SR, "  \\- BEGIN LAYER %d\n",i);
		m_layers[i]->PrintTotalMemoryUsage(groupStatic, groupDynamic);		
		app.DebugPrintf(app.USER_SR, "  \\- END LAYER %d\n",i);
	}
	app.DebugPrintf(app.USER_SR, "-- Group static: %d, Group dynamic: %d\n", groupStatic, groupDynamic);
	totalStatic += groupStatic;
	totalDynamic += groupDynamic;
	app.DebugPrintf(app.USER_SR, "-- END GROUP %d\n",m_group);
}

int UIGroup::getCommandBufferList()
{
	return m_commandBufferList;
}

// Returns the first scene of given type if it exists, NULL otherwise 
UIScene *UIGroup::FindScene(EUIScene sceneType)
{
	UIScene *pScene = NULL;

	for (int i = 0; i < eUILayer_COUNT; i++)
	{
		pScene = m_layers[i]->FindScene(sceneType);
#ifdef __PS3__
		if (pScene != NULL) return pScene;
#else
		if (pScene != nullptr) return pScene;
#endif
	}

	return pScene;
}
