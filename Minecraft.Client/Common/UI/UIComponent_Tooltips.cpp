#include "stdafx.h"
#include "UI.h"
#include "UIComponent_Tooltips.h"
#include "UISplitScreenHelpers.h"

UIComponent_Tooltips::UIComponent_Tooltips(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		for(int j=0;j<ACTION_MAX_MENU;j++)
		{
			m_overrideSFX[i][j]=false;
		}
	}	
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

#ifdef __PSVITA__
	// initialise vita touch controls with ids
	for(unsigned int i = 0; i < ETouchInput_Count; ++i)
	{
		m_TouchController[i].init(i);
	}
#endif

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(InputManager.IsCircleCrossSwapped())
	{
		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = true;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetABSwap , 1 , value );
	}
#endif
}

wstring UIComponent_Tooltips::getMoviePath()
{
	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		m_bSplitscreen = true;
		return L"ToolTipsSplit";
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		m_bSplitscreen = false;
		return L"ToolTips";
		break;
	}
}

F64 UIComponent_Tooltips::getSafeZoneHalfWidth()
{
	float width = ui.getScreenWidth();

	float safeWidth = 0.0f;

#ifndef __PSVITA__
	// 85% safezone for tooltips in either SD mode
	if( !RenderManager.IsHiDef() )
	{
		// 85% safezone
		safeWidth = m_movieWidth * (0.15f / 2);
	}
	else
	{
		// 90% safezone
		safeWidth = width * (0.1f / 2);
	}
#endif
	return safeWidth;
}

void UIComponent_Tooltips::updateSafeZone()
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
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		safeLeft = getSafeZoneHalfWidth();
		safeBottom = getSafeZoneHalfHeight();
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		safeRight = getSafeZoneHalfWidth();
		safeBottom = getSafeZoneHalfHeight();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		safeTop = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		safeTop = getSafeZoneHalfHeight();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		safeBottom = getSafeZoneHalfHeight();
		safeRight = getSafeZoneHalfWidth();
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		safeTop = getSafeZoneHalfHeight();
		safeBottom = getSafeZoneHalfHeight();
		safeLeft = getSafeZoneHalfWidth();
		safeRight = getSafeZoneHalfWidth();
		break;
	}
	setSafeZone(safeTop, safeBottom, safeLeft, safeRight);
}

void UIComponent_Tooltips::tick()
{
	UIScene::tick();

	// set the opacity of the tooltip items
	unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
	float fVal;

	if(ucAlpha<80)
	{
		// if we are in a menu, set the minimum opacity for tooltips to 15%
		if(ui.GetMenuDisplayed(m_iPad) && (ucAlpha<15))
		{
			ucAlpha=15;
		}

		// check if we have the timer running for the opacity
		unsigned int uiOpacityTimer=app.GetOpacityTimer(m_iPad);
		if(uiOpacityTimer!=0)
		{
			if(uiOpacityTimer<10)
			{
				float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
				fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
			}
			else
			{
				fVal=0.01f*80.0f;
			}
		}
		else
		{
			fVal=0.01f*static_cast<float>(ucAlpha);
		}
	}
	else
	{
		// if we are in a menu, set the minimum opacity for tooltips to 15%
		if(ui.GetMenuDisplayed(m_iPad) && (ucAlpha<15))
		{
			ucAlpha=15;
		}
		fVal=0.01f*static_cast<float>(ucAlpha);
	}
	setOpacity(fVal);

	bool layoutChanges = false;
	for (int i = 0; i < eToolTipNumButtons; i++)
	{
		if ( !ui.IsReloadingSkin() && m_tooltipValues[i].show && m_tooltipValues[i].label.needsUpdating() )
		{
			layoutChanges = true;
			_SetTooltip(i, m_tooltipValues[i].label, m_tooltipValues[i].show, true);	
			m_tooltipValues[i].label.setUpdated();
		}
	}
	if (layoutChanges) _Relayout();
}

void UIComponent_Tooltips::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	if((ProfileManager.GetLockedProfile()!=-1) && !ui.GetMenuDisplayed(m_iPad) && (app.GetGameSettings(m_iPad,eGameSetting_Tooltips)==0 || app.GetGameSettings(m_iPad,eGameSetting_DisplayHUD)==0))
	{
		return;
	}

	if(m_bSplitscreen)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			yPos = static_cast<S32>(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			xPos = static_cast<S32>(ui.getScreenWidth() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			xPos = static_cast<S32>(ui.getScreenWidth() / 2);
			yPos = static_cast<S32>(ui.getScreenHeight() / 2);
			break;
		}
		ui.setupRenderPosition(xPos, yPos);

		S32 tileXStart = 0;
		S32 tileYStart = 0;
		S32 tileWidth = width;
		S32 tileHeight = height;

		bool needsYTile = false;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			tileHeight = static_cast<S32>(ui.getScreenHeight());
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			tileWidth = static_cast<S32>(ui.getScreenWidth());
			needsYTile = true;
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			needsYTile = true;
			break;
		}

		F32 scale;
		ComputeTileScale(tileWidth, tileHeight, m_movieWidth, m_movieHeight, needsYTile, scale, tileYStart);

		// For vertical split, scale down to fit the full SWF height when the
		// window is shorter than the movie (same fix as HUD).
		if(!needsYTile && m_movieHeight > 0)
		{
			F32 scaleH = (F32)tileHeight / (F32)m_movieHeight;
			if(scaleH < scale)
				scale = scaleH;
		}

		IggyPlayerSetDisplaySize( getMovie(), (S32)(m_movieWidth * scale), (S32)(m_movieHeight * scale) );

		IggyPlayerDrawTilesStart ( getMovie() );

		m_renderWidth = tileWidth;
		m_renderHeight = tileHeight;
		IggyPlayerDrawTile ( getMovie() ,
			tileXStart ,
			tileYStart ,
			tileXStart + tileWidth ,
			tileYStart + tileHeight ,
			0 );
		IggyPlayerDrawTilesEnd ( getMovie() );
	}
	else
	{
		UIScene::render(width, height, viewport);
	}
}

void UIComponent_Tooltips::SetTooltipText( unsigned int tooltip, int iTextID )
{
	if( _SetTooltip(tooltip, iTextID) )	_Relayout();
}

void UIComponent_Tooltips::SetEnableTooltips( bool bVal )
{
}

void UIComponent_Tooltips::ShowTooltip( unsigned int tooltip, bool show )
{
	if(show != m_tooltipValues[tooltip].show)
	{
		_SetTooltip(tooltip, L"", show);
		_Relayout();
	}
}

void UIComponent_Tooltips::SetTooltips( int iA, int iB, int iX, int iY , int iLT, int iRT, int iLB, int iRB, int iLS, int iRS, int iBack, bool forceUpdate)
{
	bool needsRelayout = false;
	needsRelayout = _SetTooltip( eToolTipButtonA, iA ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonB, iB ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonX, iX ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonY, iY ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonLT, iLT ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonRT, iRT ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonLB, iLB ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonRB, iRB ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonLS, iLS ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonRS, iRS ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonRS, iRS ) || needsRelayout;
	needsRelayout = _SetTooltip( eToolTipButtonBack, iBack ) || needsRelayout;
	if (needsRelayout) _Relayout();
}

void UIComponent_Tooltips::EnableTooltip( unsigned int tooltip, bool enable )
{
}

bool UIComponent_Tooltips::_SetTooltip(unsigned int iToolTip, int iTextID)
{
	bool changed = false;
	if(iTextID != m_tooltipValues[iToolTip].iString || (iTextID > -1 && !m_tooltipValues[iToolTip].show))
	{
		m_tooltipValues[iToolTip].iString = iTextID;
		changed = true;
		if(iTextID > -1)		_SetTooltip(iToolTip, iTextID, true);
		else if(iTextID == -2)	_SetTooltip(iToolTip, L"", true);
		else					_SetTooltip(iToolTip, L"", false);
	}
	return changed;
}

void UIComponent_Tooltips::_SetTooltip(unsigned int iToolTipId, UIString label, bool show, bool force)
{
	if(!force && !show && !m_tooltipValues[iToolTipId].show)
	{
		return;
	}
	m_tooltipValues[iToolTipId].show = show;
	m_tooltipValues[iToolTipId].label = label;

	IggyDataValue result;
	IggyDataValue value[3];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iToolTipId;

	value[1].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[1].string16 = stringVal;

	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = show;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetTooltip , 3 , value );

	//app.DebugPrintf("Actual tooltip update!\n");
}

void UIComponent_Tooltips::_Relayout()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcUpdateLayout, 0 , nullptr );

#ifdef __PSVITA__
	// rebuild touchboxes
	ui.TouchBoxRebuild(this);
#endif
}

#ifdef __PSVITA__
void UIComponent_Tooltips::handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased)
{
	//app.DebugPrintf("ToolTip Touch ID = %i\n", iId);
	bool handled = false;

	// 4J - TomK no tooltips no touch!
	if((!ui.GetMenuDisplayed(ProfileManager.GetPrimaryPad())) && (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) == 0))
		return;

	// perform action on release
	if(bReleased)
	{
		switch(iId)
		{
			case ETouchInput_Touch_A:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_X\n", iId);
				if(InputManager.IsCircleCrossSwapped())
					InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_O);
				else
					InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_X);
				break;
			case ETouchInput_Touch_B:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_O\n", iId);
				if(InputManager.IsCircleCrossSwapped())
					InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_X);
				else
					InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_O);
				break;
			case ETouchInput_Touch_X:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_SQUARE\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_SQUARE);
				break;
			case ETouchInput_Touch_Y:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_TRIANGLE\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_TRIANGLE);
				break;
			case ETouchInput_Touch_LT:
				/* not in use on vita */
				app.DebugPrintf("ToolTip no action\n", iId);
				break;
			case ETouchInput_Touch_RightTrigger:
				app.DebugPrintf("ToolTip no action\n", iId);
				/* no action */
				break;
			case ETouchInput_Touch_LeftBumper:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_L1\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_L1);
				break;
			case ETouchInput_Touch_RightBumper:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_R1\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_R1);
				break;
			case ETouchInput_Touch_LeftStick:
				app.DebugPrintf("ToolTip no action\n", iId);
				/* no action */
				break;
			case ETouchInput_Touch_RightStick:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_DPAD_DOWN\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_DPAD_DOWN);
				break;
			case ETouchInput_Touch_Select:
				app.DebugPrintf("ToolTip Map Touch to _PSV_JOY_BUTTON_SELECT\n", iId);
				InputManager.MapTouchInput(iPad, _PSV_JOY_BUTTON_SELECT);
				break;
		}
	}
}
#endif

void UIComponent_Tooltips::handleReload()
{
	app.DebugPrintf("UIComponent_Tooltips::handleReload\n");

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
	if(InputManager.IsCircleCrossSwapped())
	{
		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = true;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetABSwap , 1 , value );
	}
#endif

	for(unsigned int i = 0; i < eToolTipNumButtons; ++i)
	{
		_SetTooltip(i, m_tooltipValues[i].iString, m_tooltipValues[i].show, true);
	}
	_Relayout();
}

void UIComponent_Tooltips::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if( (0 <= iPad) && (iPad <= 3) && m_overrideSFX[iPad][key] )
	{
		// don't play a sound for this action
		switch(key)
		{
		case ACTION_MENU_A:
		case ACTION_MENU_OK:
		case ACTION_MENU_PAGEUP:
		case ACTION_MENU_PAGEDOWN:
		case ACTION_MENU_X:
		case ACTION_MENU_Y:
		case ACTION_MENU_B:
		case ACTION_MENU_CANCEL:
		case ACTION_MENU_LEFT_SCROLL:
		case ACTION_MENU_RIGHT_SCROLL:
		case ACTION_MENU_LEFT:
		case ACTION_MENU_RIGHT:
		case ACTION_MENU_UP:
		case ACTION_MENU_DOWN:
			sendInputToMovie(key, repeat, pressed, released);
			break;
		}
	}
	else
	{
		switch(key)
		{
		case ACTION_MENU_OK:
		case ACTION_MENU_CANCEL:
			// 4J-PB - We get both A and OK, and B and Cancel, so only play a sound on one of them.
			sendInputToMovie(key, repeat, pressed, released);
			break;
		case ACTION_MENU_A:
		case ACTION_MENU_X:
		case ACTION_MENU_Y:
			// 4J-PB - play a Press sound
			//CD - Removed, causes a sound on all presses
			/*if(pressed)
			{
				ui.PlayUISFX(eSFX_Press);
			}*/
			sendInputToMovie(key, repeat, pressed, released);
			break;

		case ACTION_MENU_B:
			// 4J-PB - play a Press sound
			//CD - Removed, causes a sound on all presses
			/*if(pressed)
			{
				ui.PlayUISFX(eSFX_Back);
			}*/		
			sendInputToMovie(key, repeat, pressed, released);
			break;

		case ACTION_MENU_LEFT_SCROLL:
		case ACTION_MENU_RIGHT_SCROLL:
			//CD - Removed, causes a sound on all presses
			/*if(pressed)
			{
				ui.PlayUISFX(eSFX_Scroll);
			}*/
			sendInputToMovie(key, repeat, pressed, released);
			break;
		case ACTION_MENU_PAGEUP:
		case ACTION_MENU_PAGEDOWN:
		case ACTION_MENU_LEFT:
		case ACTION_MENU_RIGHT:
		case ACTION_MENU_UP:
		case ACTION_MENU_DOWN:
			sendInputToMovie(key, repeat, pressed, released);
			break;
		}
	}
}

void UIComponent_Tooltips::overrideSFX(int iPad, int key, bool bVal)
{
	m_overrideSFX[iPad][key]=bVal;
}
