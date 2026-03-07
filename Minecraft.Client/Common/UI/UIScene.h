#pragma once
// 4J-PB - remove the inherits via dominance warnings
#pragma warning( disable : 4250 )
using namespace std;
// A scene map directly to an Iggy movie (or more accurately a collection of different sized movies)

#include "UIEnums.h"
#include "UIControl_Base.h"
#include "UIControl_TextInput.h"

class ItemRenderer;
class UILayer;

// 4J Stu - Setup some defines for quickly mapping elements in the scene

#define UI_BEGIN_MAP_ELEMENTS_AND_NAMES(parentClass) \
	virtual bool mapElementsAndNames() \
	{ \
		parentClass::mapElementsAndNames(); \
		IggyValuePath *currentRoot = IggyPlayerRootPath ( getMovie() ); \
		UIControl *_mapPanel = NULL;

#define UI_END_MAP_ELEMENTS_AND_NAMES() \
		return true; \
	}

#define UI_MAP_ELEMENT( var, name) \
	{ var.setupControl(this, currentRoot , name ); var.m_pParentPanel = _mapPanel; m_controls.push_back(&var); }

#define UI_BEGIN_MAP_CHILD_ELEMENTS( parent ) \
	{ \
		IggyValuePath *lastRoot = currentRoot; \
		UIControl *_lastPanel = _mapPanel; \
		currentRoot = parent.getIggyValuePath(); \
		_mapPanel = &parent;

#define UI_END_MAP_CHILD_ELEMENTS() \
		currentRoot = lastRoot; \
		_mapPanel = _lastPanel; \
	}

#define UI_MAP_NAME( var, name ) \
	{ var = registerFastName(name); }

class UIScene
{
	friend class UILayer;
public:
	IggyValuePath *m_rootPath;

private:
	Iggy *swf;
	IggyName m_funcRemoveObject, m_funcSlideLeft, m_funcSlideRight, m_funcSetSafeZone, m_funcSetFocus, m_funcHorizontalResizeCheck;
	IggyName m_funcSetAlpha;

	ItemRenderer *m_pItemRenderer;
	unordered_map<wstring, IggyName> m_fastNames;
	unordered_map<wstring, bool> m_registeredTextures;

	typedef struct _TimerInfo
	{
		int duration;
		int targetTime;
		bool running;
	} TimerInfo;
	unordered_map<int,TimerInfo> m_timers;

	int m_iFocusControl, m_iFocusChild;
	float m_lastOpacity;
	bool m_bUpdateOpacity;
	bool m_bVisible;
	bool m_bCanHandleInput;
	UIScene *m_backScene;

	size_t m_callbackUniqueId;

public:
	enum ESceneResolution
	{
		eSceneResolution_1080,
		eSceneResolution_720,
		eSceneResolution_480,
		eSceneResolution_Vita,
	};

protected:
	ESceneResolution m_loadedResolution;

	bool m_bIsReloading;
	bool m_bFocussedOnce;

	int m_movieWidth, m_movieHeight;
	int m_renderWidth, m_renderHeight;
	vector<UIControl *> m_controls;

protected:
	UILayer *m_parentLayer;
	bool bHasFocus;
	int m_iPad;
	bool m_hasTickedOnce;

public:
	virtual Iggy *getMovie() { return swf; }

	void destroyMovie();
	virtual void reloadMovie(bool force = false);
	virtual bool needsReloaded();
	virtual bool hasMovie();
	virtual void updateSafeZone();

	int getRenderWidth() { return m_renderWidth; }
	int getRenderHeight() { return m_renderHeight; }

#ifdef __PSVITA__
	EUIGroup GetParentLayerGroup() {return m_parentLayer->m_parentGroup->GetGroup();}
#endif
#if defined(__PSVITA__) || defined(_WINDOWS64)
	UILayer *GetParentLayer() {return m_parentLayer;}
	vector<UIControl *> *GetControls() {return &m_controls;}
#endif

protected:
	virtual F64 getSafeZoneHalfHeight();
	virtual F64 getSafeZoneHalfWidth();
	void setSafeZone(S32 top, S32 bottom, S32 left, S32 right);
	void doHorizontalResizeCheck();
	virtual wstring getMoviePath() = 0;

	virtual bool mapElementsAndNames();
	void initialiseMovie();
	void loadMovie();

private:
	void getDebugMemoryUseRecursive(const wstring &moviePath, IggyMemoryUseInfo &memoryInfo);

public:
	void PrintTotalMemoryUsage(int64_t &totalStatic, int64_t &totalDynamic);

public:
	UIScene(int iPad, UILayer *parentLayer);
	virtual ~UIScene();

	virtual EUIScene getSceneType() = 0;
	ESceneResolution getSceneResolution() { return m_loadedResolution; }

	virtual void tick();

	IggyName registerFastName(const wstring &name);
#if defined(__PSVITA__) || defined(_WINDOWS64)
	void SetFocusToElement(int iID);
#endif
#ifdef __PSVITA__
	void UpdateSceneControls();
#endif
protected:
	void addTimer(int id, int ms);
	void killTimer(int id);
	void tickTimers();
	TimerInfo* getTimer(int id) { return &m_timers[id]; }
	virtual void handleTimerComplete(int id) {}

public:
	// FOCUS
	// Returns true if this scene handles input
	virtual bool stealsFocus() { return true; }

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return bHasFocus && iPad == m_iPad; }

	void gainFocus();
	void loseFocus();

	virtual void updateTooltips();
	virtual void updateComponents() {}
	virtual void handleGainFocus(bool navBack);
	virtual void handleLoseFocus() {}

	// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
	virtual bool hidesLowerScenes() { return m_hasTickedOnce; }

	// Returns true if this scene should block input to lower scenes (works like hidesLowerScenes but doesn't interfere with rendering)
	virtual bool blocksInput() { return false; }

	// returns main panel if controls are not living in the root
	virtual UIControl* GetMainPanel();

#ifdef _WINDOWS64
	// Direct edit support: scenes override to register their text inputs.
	// Base class handles tickDirectEdit in tick(), click-outside-to-deselect
	// in handleMouseClick(), and provides isDirectEditBlocking() for guards.
	virtual void getDirectEditInputs(vector<UIControl_TextInput*> &inputs) {}
	virtual void onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result) {}
	bool isDirectEditBlocking();

	// Mouse click dispatch. Hit-tests C++ controls and picks the smallest-area
	// match, then calls handlePress. Override for custom behaviour (e.g. crafting).
	virtual bool handleMouseClick(F32 x, F32 y);
#endif

	void removeControl( UIControl_Base *control, bool centreScene);
	void slideLeft();
	void slideRight();

	// RENDERING
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewpBort);

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

	void setOpacity(float percent);
	void setVisible(bool visible);
	bool isVisible() { return m_bVisible; }

protected:
	//void customDrawSlotControl(IggyCustomDrawCallbackRegion *region, int iPad, int iID, int iCount, int iAuxVal, float fAlpha, bool isFoil, bool bDecorations);
	void customDrawSlotControl(IggyCustomDrawCallbackRegion *region, int iPad, shared_ptr<ItemInstance> item, float fAlpha, bool isFoil, bool bDecorations);

	bool m_cacheSlotRenders;
	bool m_needsCacheRendered;
	int m_expectedCachedSlotCount;
private:
	typedef struct _CachedSlotDrawData
	{
		CustomDrawData *customDrawRegion;
		shared_ptr<ItemInstance> item;
		float fAlpha;
		bool isFoil;
		bool bDecorations;
	} CachedSlotDrawData;
	vector<CachedSlotDrawData *> m_cachedSlotDraw;

	void _customDrawSlotControl(CustomDrawData *region, int iPad, shared_ptr<ItemInstance> item, float fAlpha, bool isFoil, bool bDecorations, bool usingCommandBuffer);

public:
	// INPUT
	bool canHandleInput() { return m_bCanHandleInput; }
	virtual bool allowRepeat(int key);
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled) {}
	void externalCallback(IggyExternalFunctionCallUTF16 * call);

	virtual void handleDestroy() {}
protected:
	void sendInputToMovie(int key, bool repeat, bool pressed, bool released);
	virtual void handlePreReload() {}
	virtual void handleReload() {}
	virtual void handlePress(F64 controlId, F64 childId) {}
	virtual void handleFocusChange(F64 controlId, F64 childId) {}
	virtual void handleInitFocus(F64 controlId, F64 childId) {}
	virtual void handleCheckboxToggled(F64 controlId, bool selected) {}
	virtual void handleSliderMove(F64 sliderId, F64 currentValue) {}
	virtual void handleAnimationEnd() {}
	virtual void handleSelectionChanged(F64 selectedId) {}
	virtual void handleRequestMoreData(F64 startIndex, bool up) {}
	virtual void handleTouchBoxRebuild() {}
private:
	void _handleFocusChange(F64 controlId, F64 childId);
	void _handleInitFocus(F64 controlId, F64 childId);

	int convertGameActionToIggyKeycode(int action);

public:
	bool controlHasFocus(int iControlId);
	bool controlHasFocus(UIControl_Base *control);
	int getControlFocus();
	int getControlChildFocus();

	// NAVIGATION
protected:
	//void navigateForward(int iPad, EUIScene scene, void *initData = NULL);
	void navigateBack();

public:
	void setBackScene(UIScene *scene);
	UIScene *getBackScene();
	virtual void HandleDLCMountingComplete() {}
	virtual void HandleDLCInstalled() {}
#ifdef _XBOX_ONE
	virtual void HandleDLCLicenseChange() {}
#endif

	virtual void HandleMessage(EUIMessage message, void *data);

	void registerSubstitutionTexture(const wstring &textureName, PBYTE pbData, DWORD dwLength, bool deleteData = false);
	bool hasRegisteredSubstitutionTexture(const wstring &textureName);

	virtual void handleUnlockFullVersion() {}

	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased) {}


protected:

#ifdef _DURANGO
	virtual long long getDefaultGtcButtons() { return _360_GTC_BACK; }
#endif

	size_t GetCallbackUniqueId();

	virtual bool isReadyToDelete();
};
