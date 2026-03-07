// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "../../Minecraft.h"
#include "../../../Minecraft.World/DisconnectPacket.h"

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_ConnectingProgress::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	ConnectionProgressParams *param = (ConnectionProgressParams *)pInitData->pvInitData;
	m_iPad = param->iPad;
	MapChildControls();

	if( param->stringId >= 0 )
	{
		m_title.SetText( app.GetString( param->stringId ) );
	}
	else
	{		
		m_title.SetText( L"" );
	}

	m_buttonConfirm.SetText( app.GetString( IDS_CONFIRM_OK ) );

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad,false);
	}

	CXuiSceneBase::ShowBackground( m_iPad, TRUE );
	CXuiSceneBase::ShowLogo( m_iPad, TRUE );

	m_showTooltips = param->showTooltips;
	if( param->showTooltips )
		ui.SetTooltips( m_iPad, -1, IDS_TOOLTIPS_CANCEL_JOIN, -1, -1 );
	else
		ui.SetTooltips( m_iPad, -1 );

	m_runFailTimer = param->setFailTimer;
	m_timerTime = param->timerTime;

	return S_OK;
}

// The framework calls this handler when the object is to be destroyed. 
HRESULT CScene_ConnectingProgress::OnDestroy()
{
	return S_OK;
}

//----------------------------------------------------------------------------------
// Updates the UI when the list selection changes.
//----------------------------------------------------------------------------------
HRESULT CScene_ConnectingProgress::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_ConnectingProgress::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed == m_buttonConfirm)
	{
		if( m_iPad != ProfileManager.GetPrimaryPad() && g_NetworkManager.IsInSession() )
		{
			// The connection failed if we see the button, so the temp player should be removed and the viewports updated again
			Minecraft *pMinecraft = Minecraft::GetInstance();
			pMinecraft->removeLocalPlayerIdx(m_iPad);
		}
		else
		{
			app.NavigateToHomeMenu();
			//app.NavigateBack( ProfileManager.GetPrimaryPad() );
		}
	}

	return S_OK;
}

HRESULT CScene_ConnectingProgress::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if( m_showTooltips )
	{
		ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
	
		// Explicitly handle B button presses
		if (pInputData->dwKeyCode == VK_PAD_B)
		{
			// Cancel the join
			Minecraft *pMinecraft = Minecraft::GetInstance();
			pMinecraft->removeLocalPlayerIdx(m_iPad);
			rfHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CScene_ConnectingProgress::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	// This gets called every frame, so use it to update our two text boxes

	return S_OK;
}

HRESULT CScene_ConnectingProgress::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//if(m_runFailTimer) XuiSetTimer(m_hObj,0,m_timerTime);
	if( pTransition->dwTransType == XUI_TRANSITION_FROM )
	{
		XuiKillTimer(m_hObj,0);
	}

	return S_OK;
}

HRESULT CScene_ConnectingProgress::OnTransitionEnd( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	// are we being destroyed? If so, don't do anything 
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) 
	{
		return S_OK;
	}

	if( pTransition->dwTransType == XUI_TRANSITION_TO )
	{
		if(m_runFailTimer) XuiSetTimer(m_hObj,0,m_timerTime);
	}

	return S_OK;
}

HRESULT CScene_ConnectingProgress::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// Check if the connection failed
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->m_connectionFailed[m_iPad] || !g_NetworkManager.IsInSession() )
	{
		app.RemoveBackScene(m_iPad);

		// 4J-PB - timers auto repeat, so kill it
		XuiKillTimer(m_hObj,0);

		int exitReasonStringId;
		switch(pMinecraft->m_connectionFailedReason[m_iPad])
		{
		case DisconnectPacket::eDisconnect_LoginTooLong:
			exitReasonStringId = IDS_DISCONNECTED_LOGIN_TOO_LONG;
			break;
		case DisconnectPacket::eDisconnect_ServerFull:
			exitReasonStringId = IDS_DISCONNECTED_SERVER_FULL;
			break;
		case DisconnectPacket::eDisconnect_Kicked:
			exitReasonStringId = IDS_DISCONNECTED_KICKED;
			break;
		case DisconnectPacket::eDisconnect_NoUGC_AllLocal:
			exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_ALL_LOCAL;
			break;
		case DisconnectPacket::eDisconnect_NoUGC_Single_Local:
			exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
			break;
		case DisconnectPacket::eDisconnect_NoUGC_Remote:
			exitReasonStringId = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_REMOTE;
			break;
		case DisconnectPacket::eDisconnect_NoFlying:
			exitReasonStringId = IDS_DISCONNECTED_FLYING;
			break;
		case DisconnectPacket::eDisconnect_Quitting:
			exitReasonStringId = IDS_DISCONNECTED_SERVER_QUIT;
			break;
		case DisconnectPacket::eDisconnect_OutdatedServer:
			exitReasonStringId = IDS_DISCONNECTED_SERVER_OLD;
			break;
		case DisconnectPacket::eDisconnect_OutdatedClient:
			exitReasonStringId = IDS_DISCONNECTED_CLIENT_OLD;
			break;
		default:
			exitReasonStringId = IDS_CONNECTION_LOST_SERVER;
			break;
		}

		if( m_iPad != ProfileManager.GetPrimaryPad() && g_NetworkManager.IsInSession() )
		{
			m_buttonConfirm.SetShow(TRUE);
			m_buttonConfirm.SetFocus(m_iPad);

			// Set text
			m_title.SetText( app.GetString( IDS_CONNECTION_FAILED ) );
			m_status.SetText( app.GetString( exitReasonStringId ) );
		}
		else
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
#ifdef _XBOX
			StorageManager.RequestMessageBox( IDS_CONNECTION_FAILED, exitReasonStringId, uiIDA,1,ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable());
#endif
			exitReasonStringId = -1;

			//app.NavigateToHomeMenu();
			app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_ExitWorld,(void *)TRUE);
		}
	}

    return S_OK;
}
