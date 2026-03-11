#include "stdafx.h"
#include "XUI_MultiGameCreate.h"
#include "XUI_MultiGameLaunchMoreOptions.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\Minecraft.h"

#define GAME_CREATE_ONLINE_TIMER_ID 0
#define GAME_CREATE_ONLINE_TIMER_TIME 100

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameLaunchMoreOptions::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(m_CheckboxOnline,app.GetString(IDS_ONLINE_GAME));
	XuiControlSetText(m_CheckboxInviteOnly,app.GetString(IDS_INVITE_ONLY));
	XuiControlSetText(m_CheckboxAllowFoF,app.GetString(IDS_ALLOWFRIENDSOFFRIENDS));
	XuiControlSetText(m_CheckboxPVP,app.GetString(IDS_PLAYER_VS_PLAYER));
	XuiControlSetText(m_CheckboxTrustPlayers,app.GetString(IDS_TRUST_PLAYERS));
	XuiControlSetText(m_CheckboxFireSpreads,app.GetString(IDS_FIRE_SPREADS));
	XuiControlSetText(m_CheckboxTNTExplodes,app.GetString(IDS_TNT_EXPLODES));
	XuiControlSetText(m_CheckboxHostPrivileges,app.GetString(IDS_HOST_PRIVILEGES));
	XuiControlSetText(m_CheckboxResetNether,app.GetString(IDS_RESET_NETHER));
	XuiControlSetText(m_LabelWorldOptions,app.GetString(IDS_WORLD_OPTIONS));
	XuiControlSetText(m_CheckboxStructures,app.GetString(IDS_GENERATE_STRUCTURES));
	XuiControlSetText(m_CheckboxFlatWorld,app.GetString(IDS_SUPERFLAT_WORLD));
	XuiControlSetText(m_CheckboxBonusChest,app.GetString(IDS_BONUS_CHEST));

	m_params = static_cast<LaunchMoreOptionsMenuInitData *>(pInitData->pvInitData);

	if(m_params->bGenerateOptions)
	{
		m_CheckboxStructures.SetEnable(TRUE);
		m_CheckboxFlatWorld.SetEnable(TRUE);
		m_CheckboxBonusChest.SetEnable(TRUE);
		m_CheckboxStructures.SetCheck(m_params->bStructures);
		m_CheckboxFlatWorld.SetCheck (m_params->bFlatWorld);
		m_CheckboxBonusChest.SetCheck(m_params->bBonusChest);

		// This is a create world, so don't need Reset Nether		
		float fHeight, fCheckboxHeight, fWidth;
		m_CheckboxResetNether.GetBounds(&fWidth, &fCheckboxHeight);
		
		m_HostOptionGroup.GetBounds(&fWidth, &fHeight);
		m_HostOptionGroup.SetBounds(fWidth, fHeight - fCheckboxHeight);

		GetBounds(&fWidth,&fHeight);
		SetBounds(fWidth, fHeight - fCheckboxHeight);

		D3DXVECTOR3 pos;
		GetPosition(&pos);
		pos.y += (fCheckboxHeight/2);
		SetPosition(&pos);
		
		m_GenerationGroup.GetPosition(&pos);
		pos.y -= fCheckboxHeight;
		m_GenerationGroup.SetPosition(&pos);

		m_CheckboxResetNether.SetShow(FALSE);
	}
	else
	{
		float fHeight, fGroupHeight, fWidth;
		m_GenerationGroup.GetBounds(&fWidth, &fGroupHeight);
		m_GenerationGroup.SetShow(FALSE);
		m_GenerationGroup.SetEnable(FALSE);
		m_CheckboxStructures.SetShow(FALSE);
		m_CheckboxFlatWorld.SetShow(FALSE);
		m_CheckboxBonusChest.SetShow(FALSE);

		GetBounds(&fWidth,&fHeight);
		SetBounds(fWidth, fHeight +10 - fGroupHeight);

		D3DXVECTOR3 pos;
		GetPosition(&pos);
		pos.y += (fGroupHeight/2);
		SetPosition(&pos);

		m_CheckboxResetNether.SetEnable(TRUE);
		m_CheckboxResetNether.SetCheck(m_params->bResetNether);
	}

	m_CheckboxPVP.SetEnable(TRUE);
	m_CheckboxTrustPlayers.SetEnable(TRUE);
	m_CheckboxFireSpreads.SetEnable(TRUE);
	m_CheckboxTNTExplodes.SetEnable(TRUE);
	m_CheckboxHostPrivileges.SetEnable(TRUE);

	m_CheckboxPVP.SetCheck(m_params->bPVP);
	m_CheckboxTrustPlayers.SetCheck (m_params->bTrust);
	m_CheckboxFireSpreads.SetCheck(m_params->bFireSpreads);
	m_CheckboxTNTExplodes.SetCheck(m_params->bTNT);
	m_CheckboxHostPrivileges.SetCheck(m_params->bHostPrivileges);

	
	m_CheckboxOnline.SetCheck(m_params->bOnlineGame);
	m_CheckboxInviteOnly.SetCheck(m_params->bInviteOnly);
	m_CheckboxAllowFoF.SetCheck(m_params->bAllowFriendsOfFriends);	

	m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_params->iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_params->iPad);
	if(m_params->bOnlineSettingChangedBySystem)
	{
		m_CheckboxOnline.SetCheck(FALSE);
		m_CheckboxOnline.SetEnable(FALSE);
		m_CheckboxInviteOnly.SetCheck(FALSE);
		m_CheckboxInviteOnly.SetEnable(FALSE);
		m_CheckboxAllowFoF.SetCheck(FALSE);	
		m_CheckboxAllowFoF.SetEnable(FALSE);
	}
	else if(!m_params->bOnlineGame)
	{
		m_CheckboxInviteOnly.SetEnable(FALSE);
		m_CheckboxAllowFoF.SetEnable(FALSE);
	}

	XuiSetTimer(m_hObj,GAME_CREATE_ONLINE_TIMER_ID,GAME_CREATE_ONLINE_TIMER_TIME);

	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );

	
	//SentientManager.RecordMenuShown(m_params->iPad, eUIScene_CreateWorldMenu, 0);

	return S_OK;
}


HRESULT CScene_MultiGameLaunchMoreOptions::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
		m_params->bOnlineGame		= m_CheckboxOnline.IsChecked();
		m_params->bInviteOnly		= m_CheckboxInviteOnly.IsChecked();
		m_params->bAllowFriendsOfFriends = m_CheckboxAllowFoF.IsChecked();

		m_params->bStructures		= m_CheckboxStructures.IsChecked();
		m_params->bFlatWorld		= m_CheckboxFlatWorld.IsChecked();
		m_params->bBonusChest		= m_CheckboxBonusChest.IsChecked();
		m_params->bPVP				= m_CheckboxPVP.IsChecked();
		m_params->bTrust			= m_CheckboxTrustPlayers.IsChecked();
		m_params->bFireSpreads		= m_CheckboxFireSpreads.IsChecked();
		m_params->bTNT				= m_CheckboxTNTExplodes.IsChecked();

		m_params->bHostPrivileges	= m_CheckboxHostPrivileges.IsChecked();

		m_params->bResetNether	= m_CheckboxResetNether.IsChecked();

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
	}
	return S_OK;
}


HRESULT CScene_MultiGameLaunchMoreOptions::OnNotifySetFocus( HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled )
{
	int stringId = 0;
	if(hObjSource == m_CheckboxStructures)
	{
		stringId = IDS_GAMEOPTION_STRUCTURES;
	}
	else if(hObjSource == m_CheckboxOnline)
	{
		stringId = IDS_GAMEOPTION_ONLINE;
	}
	else if(hObjSource == m_CheckboxInviteOnly)
	{
		stringId = IDS_GAMEOPTION_INVITEONLY;
	}
	else if(hObjSource == m_CheckboxAllowFoF)
	{
		stringId = IDS_GAMEOPTION_ALLOWFOF;
	}
	else if(hObjSource == m_CheckboxFlatWorld)
	{
		stringId = IDS_GAMEOPTION_SUPERFLAT;
	}
	else if(hObjSource == m_CheckboxBonusChest)
	{
		stringId = IDS_GAMEOPTION_BONUS_CHEST;
	}
	else if(hObjSource == m_CheckboxPVP)
	{
		stringId = IDS_GAMEOPTION_PVP;
	}
	else if(hObjSource == m_CheckboxTrustPlayers)
	{
		stringId = IDS_GAMEOPTION_TRUST;
	}
	else if(hObjSource == m_CheckboxFireSpreads)
	{
		stringId = IDS_GAMEOPTION_FIRE_SPREADS;
	}
	else if(hObjSource == m_CheckboxTNTExplodes)
	{
		stringId = IDS_GAMEOPTION_TNT_EXPLODES;
	}
	else if(hObjSource == m_CheckboxHostPrivileges)
	{
		stringId = IDS_GAMEOPTION_HOST_PRIVILEGES;
	}
	else if(hObjSource == m_CheckboxResetNether)
	{
		stringId = IDS_GAMEOPTION_RESET_NETHER;
	}

	wstring wsText=app.GetString(stringId);
	int size = 14;
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		size = 12;
	}
	wchar_t startTags[64];
	swprintf(startTags,64,L"<font color=\"#%08x\" size=%d>",app.GetHTMLColour(eHTMLColor_White), size);
	wsText= startTags + wsText;

	m_Description.SetText(wsText.c_str());
	return S_OK;
}

HRESULT CScene_MultiGameLaunchMoreOptions::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);
	
	if(pControlNavigateData->hObjDest!=nullptr)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_MultiGameLaunchMoreOptions::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_CheckboxOnline)
	{
		if(m_CheckboxOnline.IsChecked())
		{
			m_CheckboxInviteOnly.SetEnable(TRUE);
			m_CheckboxAllowFoF.SetEnable(TRUE);
		}
		else
		{
			m_CheckboxInviteOnly.SetCheck(FALSE);
			m_CheckboxInviteOnly.SetEnable(FALSE);
			m_CheckboxAllowFoF.SetCheck(FALSE);	
			m_CheckboxAllowFoF.SetEnable(FALSE);
		}	
	}
	return S_OK;
}

HRESULT CScene_MultiGameLaunchMoreOptions::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	// 4J-PB - TODO - Don't think we can do this - if a 2nd player signs in here with an offline profile, the signed in LIVE player gets re-logged in, and bMultiplayerAllowed is false briefly 
	if( pTimer->nId == GAME_CREATE_ONLINE_TIMER_ID)
	{
		bool bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_params->iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_params->iPad);

		if(bMultiplayerAllowed != m_bMultiplayerAllowed)
		{
			if( bMultiplayerAllowed )
			{
				bool bGameSetting_Online=(app.GetGameSettings(m_params->iPad,eGameSetting_Online)!=0);
				m_CheckboxOnline.SetCheck(bGameSetting_Online?TRUE:FALSE);
				if(bGameSetting_Online)
				{
					m_CheckboxInviteOnly.SetCheck((app.GetGameSettings(m_params->iPad,eGameSetting_InviteOnly)!=0)?TRUE:FALSE);
					m_CheckboxAllowFoF.SetCheck((app.GetGameSettings(m_params->iPad,eGameSetting_FriendsOfFriends)!=0)?TRUE:FALSE);	
				}
				else
				{
					m_CheckboxInviteOnly.SetCheck(FALSE);
					m_CheckboxAllowFoF.SetCheck(FALSE);	
				}
			}
			else
			{
				m_CheckboxOnline.SetCheck(FALSE);
				m_CheckboxOnline.SetEnable(FALSE);
				m_CheckboxInviteOnly.SetCheck(FALSE);
				m_CheckboxInviteOnly.SetEnable(FALSE);
				m_CheckboxAllowFoF.SetCheck(FALSE);	
			}

			m_bMultiplayerAllowed = bMultiplayerAllowed;
		}
	}

	return S_OK;
}