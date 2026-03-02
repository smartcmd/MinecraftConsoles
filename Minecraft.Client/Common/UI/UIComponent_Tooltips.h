#pragma once

#include "UIScene.h"

class UIComponent_Tooltips : public UIScene
{
private:
	bool m_bSplitscreen;

protected:
	typedef struct _TooltipValues
	{
		bool show;	
		int iString;

		UIString label;

		_TooltipValues()
		{
			show = false;
			iString = -1;
		}
	} TooltipValues;

	TooltipValues m_tooltipValues[eToolTipNumButtons];

	IggyName m_funcSetTooltip, m_funcSetOpacity, m_funcSetABSwap, m_funcUpdateLayout;
	
#ifdef __PSVITA__
	enum ETouchInput
	{
		ETouchInput_Touch_A,
		ETouchInput_Touch_B,
		ETouchInput_Touch_X,
		ETouchInput_Touch_Y,
		ETouchInput_Touch_LT,
		ETouchInput_Touch_RightTrigger,
		ETouchInput_Touch_LeftBumper,
		ETouchInput_Touch_RightBumper,
		ETouchInput_Touch_LeftStick,
		ETouchInput_Touch_RightStick,
		ETouchInput_Touch_Select,
		
		ETouchInput_Count,
	};
	UIControl_Touch m_TouchController[ETouchInput_Count];
#endif

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
#ifdef __PSVITA__
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_A], "Touch_A")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_B], "Touch_B")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_X], "Touch_X")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_Y], "Touch_Y")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_LT], "Touch_LT")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_RightTrigger], "Touch_RightTrigger")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_LeftBumper], "Touch_LeftBumper")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_RightBumper], "Touch_RightBumper")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_LeftStick], "Touch_LeftStick")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_RightStick], "Touch_RightStick")
		UI_MAP_ELEMENT( m_TouchController[ETouchInput_Touch_Select], "Touch_Select")
#endif
		UI_MAP_NAME( m_funcSetTooltip, L"SetToolTip")
		UI_MAP_NAME( m_funcSetOpacity, L"SetOpacity")
		UI_MAP_NAME( m_funcSetABSwap, L"SetABSwap")
		UI_MAP_NAME( m_funcUpdateLayout, L"UpdateLayout")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	
	virtual F64 getSafeZoneHalfWidth();

public:
	UIComponent_Tooltips(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIComponent_Tooltips;}

	// Returns true if this scene handles input
	virtual bool stealsFocus() { return false; }

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return false; }

	// Returns true if lower scenes in this scenes layer, or in any layer below this scenes layers should be hidden
	virtual bool hidesLowerScenes() { return false; }

	virtual void updateSafeZone();

	virtual void tick();

	// RENDERING
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewport);

	virtual void SetTooltipText( unsigned int tooltip, int iTextID );
	virtual void SetEnableTooltips( bool bVal );
	virtual void ShowTooltip( unsigned int tooltip, bool show );
	virtual void SetTooltips( int iA, int iB=-1, int iX=-1, int iY=-1 , int iLT=-1, int iRT=-1, int iLB=-1, int iRB=-1, int iLS=-1, int iRS=-1, int iBack=-1, bool forceUpdate = false);
	virtual void EnableTooltip( unsigned int tooltip, bool enable );

	virtual void handleReload();
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	void overrideSFX(int iPad, int key, bool bVal);


private:
	bool _SetTooltip(unsigned int iToolTip, int iTextID);
	void _SetTooltip(unsigned int iToolTipId, UIString label, bool show, bool force = false);
	void _Relayout();

	bool m_overrideSFX[XUSER_MAX_COUNT][ACTION_MAX_MENU];

#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
#endif
};