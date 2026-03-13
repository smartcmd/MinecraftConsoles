#include "stdafx.h"
#include "XUI_TrialExitUpsell.h"

// wchImages[TRIAL_EXIT_UPSELL_IMAGE_COUNT]
WCHAR *CScene_TrialExitUpsell::wchImages[]=
{
	L"Graphics/UpsellScreenshots/Screenshot1.png",
	L"Graphics/UpsellScreenshots/Screenshot2.png",
	L"Graphics/UpsellScreenshots/Screenshot3.png",
	L"Graphics/UpsellScreenshots/Screenshot4.png",
	L"Graphics/UpsellScreenshots/Screenshot5.png",
	L"Graphics/UpsellScreenshots/Screenshot6.png",
	L"Graphics/UpsellScreenshots/Screenshot7.png",
	L"Graphics/UpsellScreenshots/Screenshot8.png"
};

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_TrialExitUpsell::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad=*static_cast<int *>(pInitData->pvInitData);

	MapChildControls();

	m_bCanExit = true;
	m_bFadeStarted = false;
	m_bShowingImage1 = true;
	m_imagesShown = 0;

	m_image1.SetImagePath( wchImages[m_imagesShown] );
	SetTimer( 0, TRIAL_EXIT_UPSELL_IMAGE_DISPLAY_TIME);

	CXuiSceneBase::ShowBackground(DEFAULT_XUI_MENU_USER , FALSE);
	CXuiSceneBase::ShowLogo(DEFAULT_XUI_MENU_USER , FALSE);

	if( m_bCanExit )
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_EXIT_GAME,IDS_TOOLTIPS_BACK, IDS_UNLOCK_TITLE);
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1,IDS_TOOLTIPS_BACK, IDS_UNLOCK_TITLE);
	}
	
	return S_OK;
}

HRESULT CScene_TrialExitUpsell::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	// ignore repeats
	if(pInputData->dwFlags&XUI_INPUT_FLAG_REPEAT) return S_OK;
	
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr=S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_A:
#ifdef _XBOX
		if( m_bCanExit )
		{
			XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, 0);
		}
#endif
		break;
	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	case VK_PAD_X:
		if(ProfileManager.IsSignedIn(pInputData->UserIndex))
		{
			TelemetryManager->RecordUpsellPresented(pInputData->UserIndex, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);
			ProfileManager.DisplayFullVersionPurchase(false,pInputData->UserIndex,eSen_UpsellID_Full_Version_Of_Game);
		}
		break;
	}

	return hr;
}

HRESULT CScene_TrialExitUpsell::OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled)
{
	if(m_bFadeStarted)
	{
		m_bFadeStarted = false;
		m_bShowingImage1 = !m_bShowingImage1;
		// We start a timer so we know when this image has been displayed for the required time
		return SetTimer( 0, TRIAL_EXIT_UPSELL_IMAGE_DISPLAY_TIME);
	}
	else
	{
		return S_OK;
	}
}

HRESULT CScene_TrialExitUpsell::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	int nStart, nEnd;

	KillTimer(0);

	++m_imagesShown;
	if( m_imagesShown >= TRIAL_EXIT_UPSELL_IMAGE_COUNT )
	{
		m_imagesShown = 0; //Loop round to the start
		m_bCanExit = true;
		
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_EXIT_GAME,IDS_TOOLTIPS_BACK, IDS_UNLOCK_TITLE);
	}

	if( m_bShowingImage1 )
	{
		m_image2.SetImagePath( wchImages[m_imagesShown] );

		FindNamedFrame( L"Fade1to2", &nStart );
		FindNamedFrame( L"EndFade1to2", &nEnd );
		PlayTimeline( nStart, nStart, nEnd, FALSE, TRUE );
		m_bFadeStarted = true;
	}
	else
	{
		m_image1.SetImagePath( wchImages[m_imagesShown] );

		FindNamedFrame( L"Fade2to1", &nStart );
		FindNamedFrame( L"EndFade2to1", &nEnd );
		PlayTimeline( nStart, nStart, nEnd, FALSE, TRUE );
		m_bFadeStarted = true;
	}

	return S_OK;
}