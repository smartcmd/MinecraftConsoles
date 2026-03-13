#include "stdafx.h"
#include "UI.h"
#include "UIScene.h"
#include "UISplitScreenHelpers.h"

#include "../../Lighting.h"
#include "../../LocalPlayer.h"
#include "../../ItemRenderer.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"

UIScene::UIScene(int iPad, UILayer *parentLayer)
{
	m_parentLayer = parentLayer;
	m_iPad = iPad;
	swf = nullptr;
	m_pItemRenderer = nullptr;

	bHasFocus = false;
	m_hasTickedOnce = false;
	m_bFocussedOnce = false;
	m_bVisible = true;
	m_bCanHandleInput = false;
	m_bIsReloading = false;

	m_iFocusControl = -1;
	m_iFocusChild = 0;
	m_lastOpacity = 1.0f;
	m_bUpdateOpacity = false;

	m_backScene = nullptr;

	m_cacheSlotRenders = false;
	m_needsCacheRendered = true;
	m_expectedCachedSlotCount = 0;
	m_callbackUniqueId = 0;
}

UIScene::~UIScene()
{
	/* Destroy the Iggy player. */
	IggyPlayerDestroy( swf );

	for(auto & it : m_registeredTextures)
	{
		ui.unregisterSubstitutionTexture( it.first, it.second );
	}

	if(m_callbackUniqueId != 0)
	{
		ui.UnregisterCallbackId(m_callbackUniqueId);
	}

	if(m_pItemRenderer != nullptr) delete m_pItemRenderer;
}

void UIScene::destroyMovie()
{
	/* Destroy the Iggy player. */
	IggyPlayerDestroy( swf );
	swf = nullptr;

	// Clear out the controls collection (doesn't delete the controls, and they get re-setup later)
	m_controls.clear();

	// Clear out all the fast names for the current movie
	m_fastNames.clear();
}

void UIScene::reloadMovie(bool force)
{
	if(!force && (stealsFocus() && (getSceneType() != eUIScene_FullscreenProgress && !bHasFocus))) return;

	m_bIsReloading = true;
	if(swf)
	{
		/* Destroy the Iggy player. */
		IggyPlayerDestroy( swf );

		// Clear out the controls collection (doesn't delete the controls, and they get re-setup later)
		m_controls.clear();

		// Clear out all the fast names for the current movie
		m_fastNames.clear();
	}

	// Reload everything
	initialiseMovie();

	handlePreReload();

	// Reload controls
	for(auto & it : m_controls)
	{
		it->ReInit();
	}

	updateComponents();
	handleReload();

	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = m_iFocusControl;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetFocus , 1 , value );

	m_needsCacheRendered = true;
	m_bIsReloading = false;
}

bool UIScene::needsReloaded()
{
	return !swf && (!stealsFocus() || bHasFocus);
}

bool UIScene::hasMovie()
{
	return swf != nullptr;
}

F64 UIScene::getSafeZoneHalfHeight()
{
	float height = ui.getScreenHeight();

	float safeHeight = 0.0f;

#ifndef __PSVITA__
	if( !RenderManager.IsHiDef() && RenderManager.IsWidescreen() )
	{
		// 90% safezone
		safeHeight = height * (0.15f / 2);
	}
	else
	{
		// 90% safezone
		safeHeight = height * (0.1f / 2);
	}
#endif
	return safeHeight;
}

F64 UIScene::getSafeZoneHalfWidth()
{
	float width = ui.getScreenWidth();

	float safeWidth = 0.0f;
#ifndef __PSVITA__
	if( !RenderManager.IsHiDef() && RenderManager.IsWidescreen() )
	{
		// 85% safezone
		safeWidth = width * (0.15f / 2);
	}
	else
	{
		// 90% safezone
		safeWidth = width * (0.1f / 2);
	}
#endif
	return safeWidth;
}

void UIScene::updateSafeZone()
{
	// Distance from edge
	F64 safeTop = 0.0;
	F64 safeBottom = 0.0;
	F64 safeLeft = 0.0;
	F64 safeRight = 0.0;

	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();

		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		// safeTop mirrors SPLIT_TOP for visual symmetry. safeBottom omitted.
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();

		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		safeTop = getSafeZoneHalfHeight();

		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		safeTop = getSafeZoneHalfHeight();

		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		safeTop = getSafeZoneHalfHeight();

		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		safeTop = getSafeZoneHalfHeight();
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();

		break;
	}
	setSafeZone(safeTop, safeBottom, safeLeft, safeRight);
}

void UIScene::setSafeZone(S32 safeTop, S32 safeBottom, S32 safeLeft, S32 safeRight)
{
	IggyDataValue result;
	IggyDataValue value[4];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = safeTop;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = safeBottom;
	value[2].type = IGGY_DATATYPE_number;
	value[2].number = safeLeft;
	value[3].type = IGGY_DATATYPE_number;
	value[3].number = safeRight;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetSafeZone , 4 , value );
}

void UIScene::initialiseMovie()
{
	loadMovie();
	mapElementsAndNames();

	updateSafeZone();

	m_bUpdateOpacity = true;
}

#if defined(__PSVITA__) || defined(_WINDOWS64)
void UIScene::SetFocusToElement(int iID)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iID;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetFocus , 1 , value );

	// also trigger handle focus change (just in case if anything else in relation needs updating!)
	_handleFocusChange(iID, 0);
}
#endif

bool UIScene::mapElementsAndNames()
{
	m_rootPath = IggyPlayerRootPath( swf );

	m_funcRemoveObject = registerFastName( L"RemoveObject" );
	m_funcSlideLeft = registerFastName( L"SlideLeft" );
	m_funcSlideRight = registerFastName( L"SlideRight" );
	m_funcSetSafeZone = registerFastName( L"SetSafeZone" );
	m_funcSetAlpha = registerFastName( L"SetAlpha" );
	m_funcSetFocus = registerFastName( L"SetFocus" );
	m_funcHorizontalResizeCheck = registerFastName( L"DoHorizontalResizeCheck");
	return true;
}

extern CRITICAL_SECTION s_loadSkinCS;
void UIScene::loadMovie()
{
	EnterCriticalSection(&UIController::ms_reloadSkinCS);		// MGH - added to prevent crash loading Iggy movies while the skins were being reloaded
	wstring moviePath = getMoviePath();

#ifdef __PS3__
	if(RenderManager.IsWidescreen())
	{
		moviePath.append(L"720.swf");
		m_loadedResolution = eSceneResolution_720;
	}
	else
	{
		moviePath.append(L"480.swf");
		m_loadedResolution = eSceneResolution_480;
	}
#elif defined __PSVITA__
	moviePath.append(L"Vita.swf");
	m_loadedResolution = eSceneResolution_Vita;
#elif defined _WINDOWS64
	if(ui.getScreenHeight() > 720.0f)
	{
		moviePath.append(L"1080.swf");
		m_loadedResolution = eSceneResolution_1080;
	}
	else
	{
		moviePath.append(L"720.swf");
		m_loadedResolution = eSceneResolution_720;
	}
#else
	moviePath.append(L"1080.swf");
	m_loadedResolution = eSceneResolution_1080;
#endif

	if(!app.hasArchiveFile(moviePath))
	{
		app.DebugPrintf("WARNING: Could not find iggy movie %ls, falling back on 720\n", moviePath.c_str());

		moviePath = getMoviePath();
		moviePath.append(L"720.swf");
		m_loadedResolution = eSceneResolution_720;

		if(!app.hasArchiveFile(moviePath))
		{
			app.DebugPrintf("ERROR: Could not find any iggy movie for %ls!\n", moviePath.c_str());
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			app.FatalLoadError();
		}
	}

	byteArray baFile = ui.getMovieData(moviePath.c_str());
	int64_t beforeLoad = ui.iggyAllocCount;
	swf = IggyPlayerCreateFromMemory ( baFile.data , baFile.length, nullptr);
	int64_t afterLoad = ui.iggyAllocCount;

	if(!swf)
	{
		app.DebugPrintf("ERROR: Failed to load iggy scene!\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		app.FatalLoadError();
	}

	// Read movie dimensions from the SWF header (available immediately after
	// CreateFromMemory, no init tick needed).
	IggyProperties *properties = IggyPlayerProperties ( swf );
	m_movieHeight = properties->movie_height_in_pixels;
	m_movieWidth = properties->movie_width_in_pixels;
	m_renderWidth = m_movieWidth;
	m_renderHeight = m_movieHeight;

	// Set display size BEFORE the init tick to match what render() will use.
	// InitializeAndTickRS runs ActionScript that creates text fields. If the
	// display size here differs from what render() passes to SetDisplaySize,
	// Iggy can cache glyph rasterizations at one scale during init and then
	// reuse them at a different scale during draw, producing mixed glyph sizes.
#ifdef _WINDOWS64
	{
		S32 fitW, fitH, fitOffX, fitOffY;
		Fit16x9(ui.getScreenWidth(), ui.getScreenHeight(), fitW, fitH, fitOffX, fitOffY);
		IggyPlayerSetDisplaySize( swf, fitW, fitH );
	}
#else
	IggyPlayerSetDisplaySize( swf, m_movieWidth, m_movieHeight );
#endif

	IggyPlayerInitializeAndTickRS ( swf );
	int64_t afterTick = ui.iggyAllocCount;

#ifdef _WINDOWS64
	// Flush Iggy's internal font caches so all glyphs get rasterized fresh
	// at the current display scale on the first Draw. Without this, stale
	// cache entries from a previous scene (loaded at a different display size)
	// cause mixed glyph sizes. ResizeD3D already calls this, which is why
	// fonts look correct after a resize but break when a scene reloads
	// without one.
	IggyFlushInstalledFonts();
#endif

	app.DebugPrintf( app.USER_SR, "Loaded iggy movie %ls\n", moviePath.c_str() );

	IggyPlayerSetUserdata(swf,this);

//#ifdef _DEBUG
#if 0
	IggyMemoryUseInfo memoryInfo;
	rrbool res;
	int iteration = 0;
	int64_t totalStatic = 0;
	int64_t totalDynamic = 0;
	while(res = IggyDebugGetMemoryUseInfo ( swf ,
		nullptr ,
		0 ,
		0 ,
		iteration ,
		&memoryInfo ))
	{
		totalStatic += memoryInfo.static_allocation_bytes;
		totalDynamic += memoryInfo.dynamic_allocation_bytes;
		app.DebugPrintf(app.USER_SR, "%ls - %.*s static: %d ( %d ) dynamic: %d ( %d )\n", moviePath.c_str(), memoryInfo.subcategory_stringlen, memoryInfo.subcategory,
			memoryInfo.static_allocation_bytes, memoryInfo.static_allocation_count, memoryInfo.dynamic_allocation_bytes, memoryInfo.dynamic_allocation_count);
		++iteration;
		//if(memoryInfo.static_allocation_bytes > 0) getDebugMemoryUseRecursive(moviePath, memoryInfo);

	}

	app.DebugPrintf(app.USER_SR, "%ls - Total: %d, Expected: %d, Diff: %d\n", moviePath.c_str(), totalStatic + totalDynamic, afterTick - beforeLoad, (afterTick - beforeLoad) - (totalStatic + totalDynamic));

#endif
	LeaveCriticalSection(&UIController::ms_reloadSkinCS);

}

void UIScene::getDebugMemoryUseRecursive(const wstring &moviePath, IggyMemoryUseInfo &memoryInfo)
{
    rrbool res;
    IggyMemoryUseInfo internalMemoryInfo;
    int internalIteration = 0;
    while (res = IggyDebugGetMemoryUseInfo(swf,
                                           0,
                                           memoryInfo.subcategory,
                                           memoryInfo.subcategory_stringlen,
                                           internalIteration,
                                           &internalMemoryInfo))
    {
        app.DebugPrintf(app.USER_SR, "%ls - %.*s static: %d ( %d ) dynamic: %d ( %d )\n", moviePath.c_str(), internalMemoryInfo.subcategory_stringlen, internalMemoryInfo.subcategory,
                        internalMemoryInfo.static_allocation_bytes, internalMemoryInfo.static_allocation_count, internalMemoryInfo.dynamic_allocation_bytes, internalMemoryInfo.dynamic_allocation_count);
        ++internalIteration;
        if (internalMemoryInfo.subcategory_stringlen > memoryInfo.subcategory_stringlen)
            getDebugMemoryUseRecursive(moviePath, internalMemoryInfo);
    }
}

void UIScene::PrintTotalMemoryUsage(int64_t &totalStatic, int64_t &totalDynamic)
{
	if(!swf) return;

	IggyMemoryUseInfo memoryInfo;
	rrbool res;
	int iteration = 0;
	int64_t sceneStatic = 0;
	int64_t sceneDynamic = 0;
	while(res = IggyDebugGetMemoryUseInfo ( swf ,
		0 ,
		"" ,
		0 ,
		iteration ,
		&memoryInfo ))
	{
		sceneStatic += memoryInfo.static_allocation_bytes;
		sceneDynamic += memoryInfo.dynamic_allocation_bytes;
		totalStatic += memoryInfo.static_allocation_bytes;
		totalDynamic += memoryInfo.dynamic_allocation_bytes;
		++iteration;

	}

	app.DebugPrintf(app.USER_SR, "    \\- Scene static: %d , Scene dynamic: %d , Total: %d - %ls\n", sceneStatic, sceneDynamic, sceneStatic + sceneDynamic, getMoviePath().c_str());
}

void UIScene::tick()
{
	if(m_bIsReloading) return;
	if(m_hasTickedOnce) m_bCanHandleInput = true;
	while(IggyPlayerReadyToTick( swf ))
	{
		tickTimers();
		for(auto & it : m_controls)
		{
			it->tick();
		}
		IggyPlayerTickRS( swf );
		m_hasTickedOnce = true;
	}

#ifdef _WINDOWS64
	{
		vector<UIControl_TextInput*> inputs;
		getDirectEditInputs(inputs);
		for (size_t i = 0; i < inputs.size(); i++)
		{
			UIControl_TextInput::EDirectEditResult result = inputs[i]->tickDirectEdit();
			if (result != UIControl_TextInput::eDirectEdit_Continue)
				onDirectEditFinished(inputs[i], result);
		}
	}
#endif
}

UIControl* UIScene::GetMainPanel()
{
	return nullptr;
}

#ifdef _WINDOWS64
bool UIScene::isDirectEditBlocking()
{
	vector<UIControl_TextInput*> inputs;
	getDirectEditInputs(inputs);
	for (size_t i = 0; i < inputs.size(); i++)
	{
		if (inputs[i]->isDirectEditing() || inputs[i]->getDirectEditCooldown() > 0)
			return true;
	}
	return false;
}

bool UIScene::handleMouseClick(F32 x, F32 y)
{
	S32 panelOffsetX = 0, panelOffsetY = 0;
	UIControl *pMainPanel = GetMainPanel();
	if (pMainPanel)
	{
		pMainPanel->UpdateControl();
		panelOffsetX = pMainPanel->getXPos();
		panelOffsetY = pMainPanel->getYPos();
	}

	// Click-outside-to-deselect: confirm any active direct edit if
	// the click landed outside the editing text input.
	{
		vector<UIControl_TextInput*> deInputs;
		getDirectEditInputs(deInputs);
		for (size_t i = 0; i < deInputs.size(); i++)
		{
			if (!deInputs[i]->isDirectEditing())
				continue;
			deInputs[i]->UpdateControl();
			S32 cx = deInputs[i]->getXPos() + panelOffsetX;
			S32 cy = deInputs[i]->getYPos() + panelOffsetY;
			S32 cw = deInputs[i]->getWidth();
			S32 ch = deInputs[i]->getHeight();
			if (!(cw > 0 && ch > 0 && x >= cx && x <= cx + cw && y >= cy && y <= cy + ch))
			{
				deInputs[i]->confirmDirectEdit();
				onDirectEditFinished(deInputs[i], UIControl_TextInput::eDirectEdit_Confirmed);
			}
		}
	}

	vector<UIControl *> *controls = GetControls();
	if (!controls) return false;

	// Hit-test controls and pick the smallest-area match to handle
	// overlapping Flash bounds correctly without sacrificing precision.
	int bestId = -1;
	S32 bestArea = INT_MAX;
	UIControl *bestCtrl = NULL;

	for (size_t i = 0; i < controls->size(); ++i)
	{
		UIControl *ctrl = (*controls)[i];
		if (!ctrl || ctrl->getHidden() || !ctrl->getVisible() || ctrl->getId() < 0)
			continue;

		UIControl::eUIControlType type = ctrl->getControlType();
		if (type != UIControl::eButton && type != UIControl::eTextInput &&
			type != UIControl::eCheckBox)
			continue;

		if (pMainPanel && ctrl->getParentPanel() != pMainPanel)
			continue;

		ctrl->UpdateControl();
		S32 cx = ctrl->getXPos() + panelOffsetX;
		S32 cy = ctrl->getYPos() + panelOffsetY;
		S32 cw = ctrl->getWidth();
		S32 ch = ctrl->getHeight();
		if (cw <= 0 || ch <= 0)
			continue;

		if (x >= cx && x <= cx + cw && y >= cy && y <= cy + ch)
		{
			S32 area = cw * ch;
			if (area < bestArea)
			{
				bestArea = area;
				bestId = ctrl->getId();
				bestCtrl = ctrl;
			}
		}
	}

	if (bestId >= 0 && bestCtrl)
	{
		if (bestCtrl->getControlType() == UIControl::eCheckBox)
		{
			UIControl_CheckBox *cb = static_cast<UIControl_CheckBox*>(bestCtrl);
			if (cb->IsEnabled())
			{
				bool newState = !cb->IsChecked();
				cb->setChecked(newState);
				handleCheckboxToggled((F64)bestId, newState);
			}
		}
		else
		{
			handlePress((F64)bestId, 0);
		}
		return true;
	}
	return false;
}
#endif

void UIScene::addTimer(int id, int ms)
{
	int currentTime = System::currentTimeMillis();

	TimerInfo info;
	info.running = true;
	info.duration = ms;
	info.targetTime = currentTime + ms;
	m_timers[id] = info;
}

void UIScene::killTimer(int id)
{
    auto it = m_timers.find(id);
    if(it != m_timers.end())
	{
		it->second.running = false;
	}
}

void UIScene::tickTimers()
{
	int currentTime = System::currentTimeMillis();
    for (auto it = m_timers.begin(); it != m_timers.end();)
    {
		if(!it->second.running)
		{
			it = m_timers.erase(it);
		}
		else
		{
			if(currentTime > it->second.targetTime)
			{
				handleTimerComplete(it->first);

				// Auto-restart
				it->second.targetTime = it->second.duration + currentTime;
			}
			++it;
		}
	}
}

IggyName UIScene::registerFastName(const wstring &name)
{
	IggyName var;
    auto it = m_fastNames.find(name);
    if(it != m_fastNames.end())
	{
		var = it->second;
	}
	else
	{
		var = IggyPlayerCreateFastName ( getMovie() , (IggyUTF16 *)name.c_str() , -1 );
		m_fastNames[name] = var;
	}
	return var;
}

void UIScene::removeControl( UIControl_Base *control, bool centreScene)
{
	IggyDataValue result;
	IggyDataValue value[2];

	string name = control->getControlName();
	IggyStringUTF8 stringVal;
	stringVal.string = (char*)name.c_str();
	stringVal.length = name.length();
	value[0].type = IGGY_DATATYPE_string_UTF8;
	value[0].string8 = stringVal;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = centreScene;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcRemoveObject , 2 , value );

#ifdef __PSVITA__
	// update the button positions since they may have changed
	UpdateSceneControls();

	// remove it from the touchboxes
	ui.TouchBoxRebuild(control->getParentScene());
#endif

	// mark the button as removed so hover/touch hit-tests skip it
	control->setHidden(true);

}

void UIScene::slideLeft()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSlideLeft , 0 , nullptr );
}

void UIScene::slideRight()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSlideRight , 0 , nullptr );
}

void UIScene::doHorizontalResizeCheck()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcHorizontalResizeCheck , 0 , nullptr );
}

void UIScene::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	if(m_bIsReloading) return;
	if(!m_hasTickedOnce || !swf) return;

	if(viewport != C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		F32 originX, originY, viewW, viewH;
		GetViewportRect(ui.getScreenWidth(), ui.getScreenHeight(), viewport, originX, originY, viewW, viewH);
		S32 fitW, fitH, offsetX, offsetY;
		Fit16x9(viewW, viewH, fitW, fitH, offsetX, offsetY);
		ui.setupRenderPosition(static_cast<S32>(originX) + offsetX, static_cast<S32>(originY) + offsetY);
		IggyPlayerSetDisplaySize( swf, fitW, fitH );
		IggyPlayerDraw( swf );
	}
	else
	{
		ui.setupRenderPosition(viewport);
		IggyPlayerSetDisplaySize( swf, width, height );
		IggyPlayerDraw( swf );
	}
}

void UIScene::setOpacity(float percent)
{
	if(percent != m_lastOpacity || (m_bUpdateOpacity && getMovie()))
	{
		m_lastOpacity = percent;

		// 4J-TomK once a scene has been freshly loaded or re-loaded we force update opacity via initialiseMovie
		if(m_bUpdateOpacity)
			m_bUpdateOpacity = false;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = percent;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetAlpha , 1 , value );
	}
}

void UIScene::setVisible(bool visible)
{
	m_bVisible = visible;
}

void UIScene::customDraw(IggyCustomDrawCallbackRegion *region)
{
	app.DebugPrintf("Handling custom draw for scene with no override!\n");
}

void UIScene::customDrawSlotControl(IggyCustomDrawCallbackRegion *region, int iPad, shared_ptr<ItemInstance> item, float fAlpha, bool isFoil, bool bDecorations)
{
	if (item!= nullptr)
	{
		if(m_cacheSlotRenders)
		{
			if( (m_cachedSlotDraw.size() + 1) == m_expectedCachedSlotCount)
			{
				//Make sure that pMinecraft->player is the correct player so that player specific rendering
				// eg clock and compass, are rendered correctly
				Minecraft *pMinecraft=Minecraft::GetInstance();
				shared_ptr<MultiplayerLocalPlayer> oldPlayer = pMinecraft->player;
				if( iPad >= 0 && iPad < XUSER_MAX_COUNT ) pMinecraft->player = pMinecraft->localplayers[iPad];

				// Setup GDraw, normal game render states and matrices
				//CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);
				PIXBeginNamedEvent(0,"Starting Iggy custom draw\n");
				CustomDrawData *customDrawRegion = ui.calculateCustomDraw(region);
				ui.beginIggyCustomDraw4J(region, customDrawRegion);
				ui.setupCustomDrawGameState();

				int list = m_parentLayer->m_parentGroup->getCommandBufferList();

				bool useCommandBuffers = false;
#ifdef _XBOX_ONE
				useCommandBuffers = true;

				// 4J Stu - Temporary until we fix the glint animation which needs updated if we are just replaying a command buffer
				m_needsCacheRendered = true;
#endif

				if(!useCommandBuffers || m_needsCacheRendered)
				{
#if (!defined __PS3__) && (!defined __PSVITA__)
					if(useCommandBuffers) RenderManager.CBuffStart(list, true);
#endif
					PIXBeginNamedEvent(0,"Draw uncached");
					ui.setupCustomDrawMatrices(this, customDrawRegion);
					_customDrawSlotControl(customDrawRegion, iPad, item, fAlpha, isFoil, bDecorations, useCommandBuffers);
					delete customDrawRegion;
					PIXEndNamedEvent();

					PIXBeginNamedEvent(0,"Draw all cache");
					// Draw all the cached slots
					for(auto& drawData : m_cachedSlotDraw)
					{
						ui.setupCustomDrawMatrices(this, drawData->customDrawRegion);
						_customDrawSlotControl(drawData->customDrawRegion, iPad, drawData->item, drawData->fAlpha, drawData->isFoil, drawData->bDecorations, useCommandBuffers);
						delete drawData->customDrawRegion;
						delete drawData;
					}
					PIXEndNamedEvent();
#ifndef __PS3__
					if(useCommandBuffers) RenderManager.CBuffEnd();
#endif
				}
				m_cachedSlotDraw.clear();

#ifndef __PS3__
				if(useCommandBuffers) RenderManager.CBuffCall(list);
#endif

				// Finish GDraw and anything else that needs to be finalised
				ui.endCustomDraw(region);

				pMinecraft->player = oldPlayer;
			}
			else
			{
				PIXBeginNamedEvent(0,"Caching region");
				CachedSlotDrawData *drawData = new CachedSlotDrawData();
				drawData->item = item;
				drawData->fAlpha = fAlpha;
				drawData->isFoil = isFoil;
				drawData->bDecorations = bDecorations;
				drawData->customDrawRegion = ui.calculateCustomDraw(region);

				m_cachedSlotDraw.push_back(drawData);
				PIXEndNamedEvent();
			}
		}
		else
		{
			// Setup GDraw, normal game render states and matrices
			CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);

			Minecraft *pMinecraft=Minecraft::GetInstance();

			//Make sure that pMinecraft->player is the correct player so that player specific rendering
			// eg clock and compass, are rendered correctly
			shared_ptr<MultiplayerLocalPlayer> oldPlayer = pMinecraft->player;
			if( iPad >= 0 && iPad < XUSER_MAX_COUNT ) pMinecraft->player = pMinecraft->localplayers[iPad];

			_customDrawSlotControl(customDrawRegion, iPad, item, fAlpha, isFoil, bDecorations, false);
			delete customDrawRegion;
			pMinecraft->player = oldPlayer;

			// Finish GDraw and anything else that needs to be finalised
			ui.endCustomDraw(region);
		}
	}
}

void UIScene::_customDrawSlotControl(CustomDrawData *region, int iPad, shared_ptr<ItemInstance> item, float fAlpha, bool isFoil, bool bDecorations, bool usingCommandBuffer)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	float bwidth,bheight;
	bwidth = region->x1 - region->x0;
	bheight = region->y1 - region->y0;

	float x = region->x0;
	float y = region->y0;

	// Base scale on height of this control, compared to height of what the item renderer normally renders (16 pixels high). Potentially
	// we might want separate x & y scales here

	float scaleX = bwidth / 16.0f;
	float scaleY = bheight / 16.0f;

	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120, 1, 0, 0);
	Lighting::turnOn();
	glPopMatrix();

	float pop = item->popTime;
	if (pop > 0)
	{
		glPushMatrix();
		float squeeze = 1 + pop / static_cast<float>(Inventory::POP_TIME_DURATION);
		float sx = x;
		float sy = y;
		float sxoffs = 8 * scaleX;
		float syoffs = 12 * scaleY;
		glTranslatef((float)(sx + sxoffs), (float)(sy + syoffs), 0);
		glScalef(1 / squeeze, (squeeze + 1) / 2, 1);
		glTranslatef((float)-(sx + sxoffs), (float)-(sy + syoffs), 0);
	}

	PIXBeginNamedEvent(0,"Render and decorate");
	if(m_pItemRenderer == nullptr) m_pItemRenderer = new ItemRenderer();
	RenderManager.StateSetBlendEnable(true);
	RenderManager.StateSetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderManager.StateSetBlendFactor(0xffffffff);
	m_pItemRenderer->renderAndDecorateItem(pMinecraft->font, pMinecraft->textures, item, x, y,scaleX,scaleY,fAlpha,isFoil,false, !usingCommandBuffer);
	PIXEndNamedEvent();

	if (pop > 0)
	{
		glPopMatrix();
	}

	if(bDecorations)
	{
		if((scaleX!=1.0f) ||(scaleY!=1.0f))
		{
			glPushMatrix();
			glScalef(scaleX, scaleY, 1.0f);
			int iX= static_cast<int>(0.5f + ((float)x) / scaleX);
			int iY= static_cast<int>(0.5f + ((float)y) / scaleY);

			m_pItemRenderer->renderGuiItemDecorations(pMinecraft->font, pMinecraft->textures, item, iX, iY, fAlpha);
			glPopMatrix();
		}
		else
		{
			m_pItemRenderer->renderGuiItemDecorations(pMinecraft->font, pMinecraft->textures, item, static_cast<int>(x), static_cast<int>(y), fAlpha);
		}
	}

	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}

// 4J Stu - Not threadsafe
//void UIScene::navigateForward(int iPad, EUIScene scene, void *initData)
//{
//	if(m_parentLayer == nullptr)
//	{
//		app.DebugPrintf("A scene is trying to navigate forwards, but it's parent layer is nullptr!\n");
//#ifndef _CONTENT_PACKAGE
//		__debugbreak();
//#endif
//	}
//	else
//	{
//		m_parentLayer->NavigateToScene(iPad,scene,initData);
//	}
//}

void UIScene::navigateBack()
{
	//CD - Added for audio
	ui.PlayUISFX(eSFX_Back);

	ui.NavigateBack(m_iPad);

	if(m_parentLayer == nullptr)
	{
//		app.DebugPrintf("A scene is trying to navigate back, but it's parent layer is nullptr!\n");
#ifndef _CONTENT_PACKAGE
//		__debugbreak();
#endif
	}
	else
	{
//		m_parentLayer->removeScene(this);

#ifdef _DURANGO
		if (ui.GetTopScene(0))
			InputManager.SetEnabledGtcButtons( ui.GetTopScene(0)->getDefaultGtcButtons() );
#endif
	}

}

void UIScene::gainFocus()
{
	if( !bHasFocus && stealsFocus() )
	{
		// 4J Stu - Don't do this
		/*
		IggyEvent event;
		IggyMakeEventFocusGained( &event , 0);

		IggyEventResult result;
		IggyPlayerDispatchEventRS( getMovie() , &event , &result );

		app.DebugPrintf("Sent gain focus event to scene\n");
		*/
		bHasFocus = true;
		if(needsReloaded())
		{
			reloadMovie();
		}

		updateTooltips();
		updateComponents();

		if(!m_bFocussedOnce)
		{
			IggyDataValue result;
			IggyDataValue value[1];

			value[0].type = IGGY_DATATYPE_number;
			value[0].number = -1;

			IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetFocus , 1 , value );
		}

		handleGainFocus(m_bFocussedOnce);
		if(bHasFocus) m_bFocussedOnce = true;
	}
	else if(bHasFocus && stealsFocus())
	{
		updateTooltips();
	}
}

void UIScene::loseFocus()
{
	if(bHasFocus)
	{
		// 4J Stu - Don't do this
		/*
		IggyEvent event;
		IggyMakeEventFocusLost( &event );
		IggyEventResult result;
		IggyPlayerDispatchEventRS ( getMovie() , &event , &result );
		*/

		app.DebugPrintf("Sent lose focus event to scene\n");
		bHasFocus = false;
		handleLoseFocus();
	}
}

void UIScene::handleGainFocus(bool navBack)
{
#ifdef _DURANGO
	InputManager.SetEnabledGtcButtons( this->getDefaultGtcButtons() );
#endif
}

void UIScene::updateTooltips()
{
	if(!ui.IsReloadingSkin())
		ui.SetTooltips(m_iPad, -1);
}

void UIScene::sendInputToMovie(int key, bool repeat, bool pressed, bool released)
{
	if(!swf) return;

	int iggyKeyCode = convertGameActionToIggyKeycode(key);

	if(iggyKeyCode < 0)
	{
		app.DebugPrintf("UI WARNING: Ignoring input as game action does not translate to an Iggy keycode\n");
		return;
	}

#ifdef _WINDOWS64
	// If a navigation key is pressed with no focused element, focus the first
	// available one so arrow keys work even when the mouse is over empty space.
	if(pressed && (iggyKeyCode == IGGY_KEYCODE_UP || iggyKeyCode == IGGY_KEYCODE_DOWN ||
	               iggyKeyCode == IGGY_KEYCODE_LEFT || iggyKeyCode == IGGY_KEYCODE_RIGHT))
	{
		IggyFocusHandle currentFocus = IGGY_FOCUS_NULL;
		IggyFocusableObject focusables[64];
		S32 numFocusables = 0;
		IggyPlayerGetFocusableObjects(swf, &currentFocus, focusables, 64, &numFocusables);
		if(currentFocus == IGGY_FOCUS_NULL && numFocusables > 0)
		{
			IggyPlayerSetFocusRS(swf, focusables[0].object, 0);
			return;
		}
	}
#endif

	IggyEvent keyEvent;
	// 4J Stu - Keyloc is always standard as we don't care about shift/alt
	IggyMakeEventKey( &keyEvent, pressed?IGGY_KEYEVENT_Down:IGGY_KEYEVENT_Up, static_cast<IggyKeycode>(iggyKeyCode), IGGY_KEYLOC_Standard );

	IggyEventResult result;
	IggyPlayerDispatchEventRS ( swf , &keyEvent , &result );
}

int UIScene::convertGameActionToIggyKeycode(int action)
{
	// TODO: This action to key mapping should probably use the control mapping
	int keycode = -1;
	switch(action)
	{
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_A:
		keycode = IGGY_KEYCODE_ENTER;
		break;
	case ACTION_MENU_B:
		keycode = IGGY_KEYCODE_ESCAPE;
		break;
	case ACTION_MENU_X:
		keycode = IGGY_KEYCODE_F1;
		break;
	case ACTION_MENU_Y:
		keycode = IGGY_KEYCODE_F2;
		break;
	case ACTION_MENU_OK:
		keycode = IGGY_KEYCODE_ENTER;
		break;
	case ACTION_MENU_CANCEL:
		keycode = IGGY_KEYCODE_ESCAPE;
		break;
	case ACTION_MENU_UP:
		keycode = IGGY_KEYCODE_UP;
		break;
	case ACTION_MENU_DOWN:
		keycode = IGGY_KEYCODE_DOWN;
		break;
	case ACTION_MENU_RIGHT:
		keycode = IGGY_KEYCODE_RIGHT;
		break;
	case ACTION_MENU_LEFT:
		keycode = IGGY_KEYCODE_LEFT;
		break;
	case ACTION_MENU_PAGEUP:
		keycode = IGGY_KEYCODE_PAGE_UP;
		break;
	case ACTION_MENU_PAGEDOWN:
#ifdef __PSVITA__
		if (!InputManager.IsVitaTV())
		{
			keycode = IGGY_KEYCODE_F6;
		}
		else
#endif
		{
			keycode = IGGY_KEYCODE_PAGE_DOWN;
		}
		break;
	case ACTION_MENU_RIGHT_SCROLL:
		keycode = IGGY_KEYCODE_F3;
		break;
	case ACTION_MENU_LEFT_SCROLL:
		keycode = IGGY_KEYCODE_F4;
		break;
	case ACTION_MENU_STICK_PRESS:
		break;
	case ACTION_MENU_OTHER_STICK_PRESS:
		keycode = IGGY_KEYCODE_F5;
		break;
	case ACTION_MENU_OTHER_STICK_UP:
		keycode = IGGY_KEYCODE_F11;
		break;
	case ACTION_MENU_OTHER_STICK_DOWN:
		keycode = IGGY_KEYCODE_F12;
		break;
	case ACTION_MENU_OTHER_STICK_LEFT:
		break;
	case ACTION_MENU_OTHER_STICK_RIGHT:
		break;
	};

	return keycode;
}

bool UIScene::allowRepeat(int key)
{
	// 4J-PB - ignore repeats of action ABXY buttons
	// fix for PS3 213 - [MAIN MENU] Holding down buttons will continue to activate every prompt.
	switch(key)
	{
	case ACTION_MENU_OK:
	case ACTION_MENU_CANCEL:
	case ACTION_MENU_A:
	case ACTION_MENU_B:
	case ACTION_MENU_X:
	case ACTION_MENU_Y:
		return false;
	}
	return true;
}

void UIScene::externalCallback(IggyExternalFunctionCallUTF16 * call)
{
	if(wcscmp((wchar_t *)call->function_name.string,L"handlePress")==0)
	{
		if(call->num_arguments != 2)
		{
			app.DebugPrintf("Callback for handlePress did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_number)
		{
			app.DebugPrintf("Arguments for handlePress were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		handlePress(call->arguments[0].number, call->arguments[1].number);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleFocusChange")==0)
	{
		if(call->num_arguments != 2)
		{
			app.DebugPrintf("Callback for handleFocusChange did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_number)
		{
			app.DebugPrintf("Arguments for handleFocusChange were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		_handleFocusChange(call->arguments[0].number, call->arguments[1].number);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleInitFocus")==0)
	{
		if(call->num_arguments != 2)
		{
			app.DebugPrintf("Callback for handleInitFocus did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_number)
		{
			app.DebugPrintf("Arguments for handleInitFocus were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		_handleInitFocus(call->arguments[0].number, call->arguments[1].number);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleCheckboxToggled")==0)
	{
		if(call->num_arguments != 2)
		{
			app.DebugPrintf("Callback for handleCheckboxToggled did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_boolean)
		{
			app.DebugPrintf("Arguments for handleCheckboxToggled were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		handleCheckboxToggled(call->arguments[0].number, call->arguments[1].boolval);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleSliderMove")==0)
	{
		if(call->num_arguments != 2)
		{
			app.DebugPrintf("Callback for handleSliderMove did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_number)
		{
			app.DebugPrintf("Arguments for handleSliderMove were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		handleSliderMove(call->arguments[0].number, call->arguments[1].number);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleAnimationEnd")==0)
	{
		if(call->num_arguments != 0)
		{
			app.DebugPrintf("Callback for handleAnimationEnd did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		handleAnimationEnd();
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleSelectionChanged")==0)
	{
		if(call->num_arguments != 1)
		{
			app.DebugPrintf("Callback for handleSelectionChanged did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		if(call->arguments[0].type != IGGY_DATATYPE_number)
		{
			app.DebugPrintf("Arguments for handleSelectionChanged were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			return;
		}
		handleSelectionChanged(call->arguments[0].number);
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleRequestMoreData")==0)
	{
		if(call->num_arguments == 0)
		{
			handleRequestMoreData(0,false);
		}
		else
		{
			if(call->num_arguments != 2)
			{
				app.DebugPrintf("Callback for handleRequestMoreData did not have the correct number of arguments\n");
#ifndef _CONTENT_PACKAGE
				__debugbreak();
#endif
				return;
			}
			if(call->arguments[0].type != IGGY_DATATYPE_number || call->arguments[1].type != IGGY_DATATYPE_boolean)
			{
				app.DebugPrintf("Arguments for handleRequestMoreData were not of the correct type\n");
#ifndef _CONTENT_PACKAGE
				__debugbreak();
#endif
				return;
			}
			handleRequestMoreData(call->arguments[0].number, call->arguments[1].boolval);
		}
	}
	else if(wcscmp((wchar_t *)call->function_name.string,L"handleTouchBoxRebuild")==0)
	{
		handleTouchBoxRebuild();
	}
	else
	{
		app.DebugPrintf("Unhandled callback: %s\n", call->function_name.string);
	}
}

void UIScene::registerSubstitutionTexture(const wstring &textureName, PBYTE pbData, DWORD dwLength, bool deleteData)
{
	m_registeredTextures[textureName] = deleteData;;
	ui.registerSubstitutionTexture(textureName, pbData, dwLength);
}

bool UIScene::hasRegisteredSubstitutionTexture(const wstring &textureName)
{
    auto it = m_registeredTextures.find(textureName);

    return  it != m_registeredTextures.end();
}

void UIScene::_handleFocusChange(F64 controlId, F64 childId)
{
	int newControl = static_cast<int>(controlId);
	int newChild = static_cast<int>(childId);
	if (newControl != m_iFocusControl || newChild != m_iFocusChild)
	{
		m_iFocusControl = newControl;
		m_iFocusChild = newChild;

		handleFocusChange(controlId, childId);
		ui.PlayUISFX(eSFX_Focus);
	}
}

void UIScene::_handleInitFocus(F64 controlId, F64 childId)
{
	m_iFocusControl = static_cast<int>(controlId);
	m_iFocusChild = static_cast<int>(childId);

	//handleInitFocus(controlId, childId);
	handleFocusChange(controlId, childId);
}

bool UIScene::controlHasFocus(int iControlId)
{
	return m_iFocusControl == iControlId;
}

bool UIScene::controlHasFocus(UIControl_Base *control)
{
	return controlHasFocus( control->getId() );
}

int UIScene::getControlChildFocus()
{
	return m_iFocusChild;
}

int UIScene::getControlFocus()
{
	return m_iFocusControl;
}

void UIScene::setBackScene(UIScene *scene)
{
	m_backScene = scene;
}

UIScene *UIScene::getBackScene()
{
	return m_backScene;
}
#ifdef __PSVITA__
void UIScene::UpdateSceneControls()
{
	for ( UIControl *control : *GetControls() )
	{
		control->UpdateControl();
	}
}
#endif

void UIScene::HandleMessage(EUIMessage message, void *data)
{
}

size_t UIScene::GetCallbackUniqueId()
{
	if( m_callbackUniqueId == 0)
	{
		m_callbackUniqueId = ui.RegisterForCallbackId(this);
	}
	return m_callbackUniqueId;
}

bool UIScene::isReadyToDelete()
{
	return true;
}