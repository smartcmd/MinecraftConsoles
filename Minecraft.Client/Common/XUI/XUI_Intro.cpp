// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "../XUI/XUI_Intro.h"

#define TIMELINE_NORMAL		0
#define TIMELINE_ESRBFADE	1
#define TIMELINE_LOGOSFADE	2

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Intro::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();

	// We may need to display a ratings image for a while at the start...
	m_bWantsToSkip=false;
	m_iTimeline=TIMELINE_NORMAL;

	// 4J-PB - We can't check to see if the version is a trial or full game until after 5 seconds...
	// The reason that this is a requirement is that there is a problem that occasionally happens *only* in the production 
	// environment (not partnernet or cert), where if you don’t wait 5 seconds, you can run into an issue where the timing 
	// of the call fails and the game is always identified as being the trial version even if you have upgraded to the full version.
	// -Joe Dunavant

	// start a timer for the required 5 seconds, plus an extra bit to allow the lib timer to enable the xcontent license check call
#ifdef _CONTENT_PACKAGE
	m_bSkippable=false;
	XuiSetTimer( m_hObj,0,5200);
#else
	m_bSkippable=true;
#endif
	
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Intro::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);


	return S_OK;
}

HRESULT CScene_Intro::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	static bool bPressed=false;

	if(bPressed==false)
	{
		if(m_bSkippable)
		{
			// stop the animation
			XuiElementStopTimeline(m_hObj,TRUE);
			app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_SaveMessage);
			app.SetIntroRunning(false);		
		}
		else
		{
			m_bWantsToSkip=true;
		}
		
		bPressed=true;
	}

	return S_OK;
}

HRESULT CScene_Intro::OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled)
{
	int nStart, nEnd;

	if(m_bSkippable && m_bWantsToSkip)
	{
		// straight to the game
		app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_SaveMessage);
		app.SetIntroRunning(false);
	}
	else
	{
		switch(m_iTimeline)
		{
		case TIMELINE_NORMAL:
			{	
				// 4J-PB - lots of discussions over this because Brazil is in the NA region. This is what I have been advised to do...
				//if(ProfileManager.RegionIsNorthAmerica())
				if(ProfileManager.LocaleIsUSorCanada())
				{
					m_iTimeline=TIMELINE_ESRBFADE;
					XuiElementFindNamedFrame( m_hObj, L"ESRBFade", &nStart );
					XuiElementFindNamedFrame( m_hObj, L"ESRBFadeEnd", &nEnd );
					XuiElementPlayTimeline( m_hObj, nStart, nStart, nEnd, FALSE, TRUE ); 
				}
				else
				{
					m_iTimeline=TIMELINE_LOGOSFADE;
					XuiElementFindNamedFrame( m_hObj, L"StartFade", &nStart );
					XuiElementFindNamedFrame( m_hObj, L"EndFade", &nEnd );
					XuiElementPlayTimeline( m_hObj, nStart, nStart, nEnd, FALSE, TRUE ); 
				}			
			}
			break;

		case TIMELINE_ESRBFADE:
			if(m_bWantsToSkip && m_bSkippable)
			{
				app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_SaveMessage);
				app.SetIntroRunning(false);	
			}
			else
			{			
				m_iTimeline=TIMELINE_LOGOSFADE;				
				XuiElementFindNamedFrame( m_hObj, L"StartFade", &nStart );
				XuiElementFindNamedFrame( m_hObj, L"EndFade", &nEnd );
				XuiElementPlayTimeline( m_hObj, nStart, nStart, nEnd, FALSE, TRUE ); 
			}
			break;
		case TIMELINE_LOGOSFADE:
			app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_SaveMessage);
			app.SetIntroRunning(false);		
			break;
		}
	}

	return S_OK;
}


HRESULT CScene_Intro::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	HRESULT hr=XuiKillTimer(m_hObj,0);
	m_bSkippable=true;

	if(m_bWantsToSkip)
	{
		// stop the animation
		XuiElementStopTimeline(m_hObj,TRUE);
		app.NavigateToScene(XUSER_INDEX_ANY,eUIScene_SaveMessage);
		app.SetIntroRunning(false);		
	}

	return hr;
}