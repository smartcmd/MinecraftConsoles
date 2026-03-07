#include "stdafx.h"

#include <assert.h>
#include "../XUI/XUI_HowToPlayMenu.h"
#include "../XUI/XUI_HelpHowToPlay.h"

// strings for buttons in the list				  
unsigned int CScene_HowToPlayMenu::m_uiHTPButtonNameA[]=
{
	IDS_HOW_TO_PLAY_MENU_WHATSNEW,		  // eHTPButton_WhatsNew
	IDS_HOW_TO_PLAY_MENU_BASICS,		  // eHTPButton_Basics, 
	IDS_HOW_TO_PLAY_MENU_MULTIPLAYER,	  // eHTPButton_Multiplayer
	IDS_HOW_TO_PLAY_MENU_HUD,			  // eHTPButton_Hud,
	IDS_HOW_TO_PLAY_MENU_CREATIVE,		  // eHTPButton_Creative,
	IDS_HOW_TO_PLAY_MENU_INVENTORY,		  // eHTPButton_Inventory,
	IDS_HOW_TO_PLAY_MENU_CHESTS,		  // eHTPButton_Chest,
	IDS_HOW_TO_PLAY_MENU_CRAFTING,		  // eHTPButton_Crafting,
	IDS_HOW_TO_PLAY_MENU_FURNACE,		  // eHTPButton_Furnace,
	IDS_HOW_TO_PLAY_MENU_DISPENSER,		  // eHTPButton_Dispenser,

	IDS_HOW_TO_PLAY_MENU_BREWING,		  // eHTPButton_Brewing,
	IDS_HOW_TO_PLAY_MENU_ENCHANTMENT,	  // eHTPButton_Enchantment,
	IDS_HOW_TO_PLAY_MENU_ANVIL,
	IDS_HOW_TO_PLAY_MENU_FARMANIMALS,		  // eHTPButton_Breeding,
	IDS_HOW_TO_PLAY_MENU_BREEDANIMALS,		  // eHTPButton_Breeding,
	IDS_HOW_TO_PLAY_MENU_TRADING,

	IDS_HOW_TO_PLAY_MENU_NETHERPORTAL,		// eHTPButton_NetherPortal,
	IDS_HOW_TO_PLAY_MENU_THEEND,			// eHTPButton_TheEnd,
	IDS_HOW_TO_PLAY_MENU_SOCIALMEDIA,		// eHTPButton_SocialMedia,
	IDS_HOW_TO_PLAY_MENU_BANLIST,			// eHTPButton_BanningLevels,
	IDS_HOW_TO_PLAY_MENU_HOSTOPTIONS,		// eHTPButton_HostOptions,
};

// mapping the buttons to a scene value
unsigned int CScene_HowToPlayMenu::m_uiHTPSceneA[]=
{
	eHowToPlay_WhatsNew,
	eHowToPlay_Basics,
	eHowToPlay_Multiplayer,
	eHowToPlay_HUD,
	eHowToPlay_Creative,
	eHowToPlay_Inventory,
	eHowToPlay_Chest,
	eHowToPlay_InventoryCrafting,
	eHowToPlay_Furnace,
	eHowToPlay_Dispenser,

	eHowToPlay_Brewing,
	eHowToPlay_Enchantment,
	eHowToPlay_Anvil,
	eHowToPlay_FarmingAnimals,
	eHowToPlay_Breeding,
	eHowToPlay_Trading,

	eHowToPlay_NetherPortal,
	eHowToPlay_TheEnd,
	eHowToPlay_SocialMedia,
	eHowToPlay_BanList,
	eHowToPlay_HostOptions,
};

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_HowToPlayMenu::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *(int *)pInitData->pvInitData;
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	bool bSplitscreen= app.GetLocalPlayerCount()>1;
	m_ButtonList=NULL;

	//MapChildControls();

	//m_iButtons=0;
	if(bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad, false);
	}

	// 4J-PB - changing all versions to use a list of buttons, since we're adding some
		// We're going to use a list of buttons here
	CXuiElement e = m_hObj;
	HRESULT hr = e.GetChildById(L"HowToListButtons", &m_ButtonList);
	m_iButtons=eHTPButton_Max;
	for(int i=0;i<eHTPButton_Max;i++)
	{
		//m_Buttons[i].SetShow(FALSE);
		//m_Buttons[i].SetEnable(FALSE);
		m_ButtonList.InsertItems( m_ButtonList.GetItemCount(), 1 );
	}

	// set the focus to the list
	m_ButtonList.SetFocus(m_iPad);

	if((!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()) || bSplitscreen)
	{
		if(bNotInGame)
		{
			CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, FALSE );
		}
	}	
	else if(bNotInGame)
	{
		CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{
		CXuiSceneBase::ShowLogo( m_iPad, TRUE );
	}
	// Display the tooltips
	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}
	else
	{		
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
	}

	return S_OK;
}

HRESULT CScene_HowToPlayMenu::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	if( pGetSourceTextData->bItemData )
	{
		if( pGetSourceTextData->iItem < (int)eHTPButton_Max )
		{
			pGetSourceTextData->szText = app.GetString(m_uiHTPButtonNameA[pGetSourceTextData->iItem]);//m_Buttons[pGetSourceTextData->iItem].GetText();
			pGetSourceTextData->bDisplay = TRUE;

			bHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CScene_HowToPlayMenu::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = m_iButtons;
	bHandled = TRUE;
	return S_OK;
}


HRESULT CScene_HowToPlayMenu::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{
	// In a list, we need to play the 'focus' sound ourselves
	if((pNotifySelChangedData->iOldItem!=-1) && m_ButtonList && (hObjSource==m_ButtonList.m_hObj))
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}

	return S_OK;
}
//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_HowToPlayMenu::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);
	unsigned int uiInitData;
	unsigned int uiButtonCounter=0;

	// 4J-PB - now using a list for all resolutions
	//if((!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()) || app.GetLocalPlayerCount()>1)
	{
		if(hObjPressed==m_ButtonList && m_ButtonList.TreeHasFocus() && (m_ButtonList.GetItemCount() > 0) && (m_ButtonList.GetCurSel() < (int)eHTPButton_Max) )
		{
			uiButtonCounter=m_ButtonList.GetCurSel();
		}
	}
	/*else
	{
		while((uiButtonCounter<BUTTONS_HTP_MAX) && (m_Buttons[uiButtonCounter]!=hObjPressed)) uiButtonCounter++;
	}*/

	// Determine which button was pressed,
	// and call the appropriate function.

	uiInitData = ( ( 1 <<  31 )  | ( m_uiHTPSceneA[uiButtonCounter]  << 16 ) | ( short )( m_iPad ) );
	if(app.GetLocalPlayerCount()>1)
	{
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_HowToPlay, ( void* )( uiInitData ) );
	}
	else
	{
		app.NavigateToScene(pNotifyPressData->UserIndex,eUIScene_HowToPlay, ( void* )( uiInitData ) );
	}
	
	rfHandled=TRUE;
	return S_OK;
}

HRESULT CScene_HowToPlayMenu::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_ESCAPE:
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;

		break;
	}

	return S_OK;
}

HRESULT CScene_HowToPlayMenu::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	return S_OK;
}

HRESULT CScene_HowToPlayMenu::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,false);
}
