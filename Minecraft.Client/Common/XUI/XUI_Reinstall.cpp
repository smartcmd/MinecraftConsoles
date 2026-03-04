// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\XUI\XUI_Reinstall.h"
#include "..\..\..\Minecraft.World\AABB.h"
#include "..\..\..\Minecraft.World\Vec3.h"
#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\..\Minecraft.Client\StatsCounter.h"
#include "..\..\..\Minecraft.World\Entity.h"
#include "..\..\..\Minecraft.World\Level.h"
#include "..\..\..\Minecraft.Client\LocalPlayer.h"
#include "..\..\MinecraftServer.h"
#include "..\..\ProgressRenderer.h"
#include "..\..\..\Minecraft.World\DisconnectPacket.h"
#include "..\..\Minecraft.h"
#include "..\..\Options.h"



//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Reinstall::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	// We'll only be in this menu from the main menu, not in game
	m_iPad = *(int *)pInitData->pvInitData;

	m_bIgnoreInput=false;
	MapChildControls();

	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_THEME],app.GetString(IDS_REINSTALL_THEME));
	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_GAMERPIC1],app.GetString(IDS_REINSTALL_GAMERPIC_1));
	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_GAMERPIC2],app.GetString(IDS_REINSTALL_GAMERPIC_2));
	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_AVATAR1],app.GetString(IDS_REINSTALL_AVATAR_ITEM_1));
	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_AVATAR2],app.GetString(IDS_REINSTALL_AVATAR_ITEM_2));
	XuiControlSetText(m_Buttons[BUTTON_REINSTALL_AVATAR3],app.GetString(IDS_REINSTALL_AVATAR_ITEM_3));

	int iFirstEnabled=-1;

	// we can only come in here if we are the primary player, it's the full version, and we have some content to re-install

	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_mine100Blocks) )
	{
		m_Buttons[BUTTON_REINSTALL_GAMERPIC1].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_GAMERPIC1].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_GAMERPIC1;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_GAMERPIC1].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_GAMERPIC1].EnableInput(FALSE);
	}

	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_kill10Creepers) )
	{
		m_Buttons[BUTTON_REINSTALL_GAMERPIC2].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_GAMERPIC2].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_GAMERPIC2;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_GAMERPIC2].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_GAMERPIC2].EnableInput(FALSE);
	}
	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_eatPorkChop) )
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR1].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_AVATAR1].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_AVATAR1;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR1].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_AVATAR1].EnableInput(FALSE);
	}
	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_play100Days) )
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR2].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_AVATAR2].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_AVATAR2;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR2].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_AVATAR2].EnableInput(FALSE);
	}
	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_arrowKillCreeper) )
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR3].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_AVATAR3].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_AVATAR3;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_AVATAR3].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_AVATAR3].EnableInput(FALSE);
	}
	if(ProfileManager.IsAwardsFlagSet(m_iPad,eAward_socialPost) )
	{
		m_Buttons[BUTTON_REINSTALL_THEME].SetEnable(TRUE);
		m_Buttons[BUTTON_REINSTALL_THEME].EnableInput(TRUE);
		if(iFirstEnabled==-1) iFirstEnabled=BUTTON_REINSTALL_THEME;
	}
	else
	{
		m_Buttons[BUTTON_REINSTALL_THEME].SetEnable(FALSE);
		m_Buttons[BUTTON_REINSTALL_THEME].EnableInput(FALSE);
	}

	// if iFirstEnabled is still -1 then we shouldn't set focus to a button
	if(iFirstEnabled!=-1)
	{
		m_Buttons[iFirstEnabled].InitFocus(m_iPad);
	}
	
	// allow the user to change the storage device
	if(!StorageManager.GetSaveDeviceSelected(m_iPad))
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE);	
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE);
	}
	
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_Reinstall::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	unsigned int uiButtonCounter=0;

	while((uiButtonCounter<BUTTONS_REINSTALL_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;
	
	// Determine which button was pressed,
	// and call the appropriate function.	

	// store the last button pressed, so on a nav back we can set the focus properly
	m_iLastButtonPressed=uiButtonCounter;
	// you do have a storage device don't you?
	if(!StorageManager.GetSaveDeviceSelected(m_iPad) || StorageManager.GetSaveDisabled())
	{
		// Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage 
		// device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
		m_bIgnoreInput=true;

		StorageManager.SetSaveDevice(&CScene_Reinstall::DeviceSelectReturned_AndReinstall,this,true);
		return S_OK;
	}

	switch(uiButtonCounter)
	{
	case BUTTON_REINSTALL_THEME:
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_socialPost, true );
		break;
	case BUTTON_REINSTALL_GAMERPIC1:	
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_mine100Blocks, true);
		break;
	case BUTTON_REINSTALL_GAMERPIC2:	
		ProfileManager.Award( pNotifyPressData->UserIndex,  eAward_kill10Creepers, true);
		break;
	case BUTTON_REINSTALL_AVATAR1:	
		ProfileManager.Award( pNotifyPressData->UserIndex,  eAward_eatPorkChop, true );
		break;
	case BUTTON_REINSTALL_AVATAR2:	
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_play100Days, true );
		break;
	case BUTTON_REINSTALL_AVATAR3:	
		ProfileManager.Award( pNotifyPressData->UserIndex, eAward_arrowKillCreeper, true );
		break;
	}

	return S_OK;
}

HRESULT CScene_Reinstall::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_PAD_START:
	case VK_ESCAPE:
		app.NavigateBack(m_iPad);
		rfHandled = TRUE;

		break;
	case VK_PAD_X:
		// Change device
		// we need a function to deal with the return from this - if it changes, we need to update the tooltips
		// Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage 
		// device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
		m_bIgnoreInput=true;
		StorageManager.SetSaveDevice(&CScene_Reinstall::DeviceSelectReturned,this,true);
		
		rfHandled = TRUE;
		break;
	}

	return S_OK;
}

HRESULT CScene_Reinstall::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	
	return S_OK;
}

HRESULT CScene_Reinstall::OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	pControlNavigateData->hObjDest=XuiControlGetNavigation(pControlNavigateData->hObjSource,pControlNavigateData->nControlNavigate,TRUE,TRUE);
	
	if(pControlNavigateData->hObjDest!=NULL)
	{
		bHandled=TRUE;
	}

	return S_OK;
}

HRESULT CScene_Reinstall::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{

		// 4J-PB - Going to resize buttons if the text is too big to fit on any of them (Br-pt problem with the length of Unlock Full Game)
		XUIRect xuiRect;
		HXUIOBJ visual=NULL;
		HXUIOBJ text;
		float fMaxTextLen=0.0f;
		float fTextVisualLen;
		float fMaxButton;
		float fWidth,fHeight;

		HRESULT hr=XuiControlGetVisual(m_Buttons[0].m_hObj,&visual);
		hr=XuiElementGetChildById(visual,L"text_Label",&text);
		hr=XuiElementGetBounds(text,&fTextVisualLen,&fHeight);
		m_Buttons[0].GetBounds(&fMaxButton,&fHeight);


		for(int i=0;i<BUTTONS_REINSTALL_MAX;i++)
		{
			hr=XuiTextPresenterMeasureText(text, m_Buttons[i].GetText(), &xuiRect);
			if(xuiRect.right>fMaxTextLen) fMaxTextLen=xuiRect.right;
		}

		if(fTextVisualLen<fMaxTextLen)
		{
			D3DXVECTOR3 vec;

			// centre is vec.x+(fWidth/2)
			for(int i=0;i<BUTTONS_REINSTALL_MAX;i++)
			{
				// need to resize and reposition the buttons
				m_Buttons[i].GetPosition(&vec);
				m_Buttons[i].GetBounds(&fWidth,&fHeight);
				vec.x= vec.x+(fWidth/2.0f)-(fMaxTextLen/2.0f); 

				m_Buttons[i].SetPosition(&vec);
				m_Buttons[i].SetBounds(fMaxButton+fMaxTextLen-fTextVisualLen,fHeight);
			}
		}
	}

	return S_OK;
}

int CScene_Reinstall::DeviceSelectReturned(void *pParam,bool bContinue)
{
	CScene_Reinstall* pClass = (CScene_Reinstall*)pParam;

	if(!StorageManager.GetSaveDeviceSelected(pClass->m_iPad))
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE);	
	}
	else
	{
		ui.SetTooltips(DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE);
	}

	pClass->m_bIgnoreInput=false;
	return 0;
}

int CScene_Reinstall::DeviceSelectReturned_AndReinstall(void *pParam,bool bContinue)
{
	CScene_Reinstall* pClass = (CScene_Reinstall*)pParam;

	if(!StorageManager.GetSaveDeviceSelected(pClass->m_iPad))
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_SELECTDEVICE);	
	}
	else
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,IDS_TOOLTIPS_CHANGEDEVICE);
		// reinstall
		switch(pClass->m_iLastButtonPressed)
		{
		case BUTTON_REINSTALL_THEME:
			ProfileManager.Award( pClass->m_iPad, eAward_socialPost, true );
			break;
		case BUTTON_REINSTALL_GAMERPIC1:	
			ProfileManager.Award( pClass->m_iPad, eAward_mine100Blocks, true);
			break;
		case BUTTON_REINSTALL_GAMERPIC2:	
			ProfileManager.Award( pClass->m_iPad,  eAward_kill10Creepers, true);
			break;
		case BUTTON_REINSTALL_AVATAR1:	
			ProfileManager.Award( pClass->m_iPad,  eAward_eatPorkChop, true );
			break;
		case BUTTON_REINSTALL_AVATAR2:	
			ProfileManager.Award( pClass->m_iPad, eAward_play100Days, true );
			break;
		case BUTTON_REINSTALL_AVATAR3:	
			ProfileManager.Award( pClass->m_iPad, eAward_arrowKillCreeper, true );
			break;
		}
	}
	pClass->m_bIgnoreInput=false;
	return 0;
}


