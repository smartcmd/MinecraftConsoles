// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <assert.h>
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.crafting.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\Tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\Common\Tutorial\TutorialEnum.h"
#include "..\..\Minecraft.h"
#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "XUI_Ctrl_SlotList.h"

#define IGNORE_KEYPRESS_TIMERID 0
#define IGNORE_KEYPRESS_TIME 100

//////////////////////////////////////////////////////////////////////////
//
//	
//
//////////////////////////////////////////////////////////////////////////
CXuiSceneCraftingPanel::CXuiSceneCraftingPanel()
{
}

//////////////////////////////////////////////////////////////////////////
//
//	OnInit
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneCraftingPanel::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_bIgnoreKeyPresses=true;

	D3DXVECTOR3 vec;
	VOID *pObj;
	CraftingPanelScreenInput* pCraftingPanelData = static_cast<CraftingPanelScreenInput *>(pInitData->pvInitData);
	m_iContainerType=pCraftingPanelData->iContainerType;
	m_pPlayer=pCraftingPanelData->player;
	m_iPad=pCraftingPanelData->iPad;
	m_bSplitscreen=pCraftingPanelData->bSplitscreen;

	HRESULT hr = S_OK; 

	MapChildControls();
	
	if(m_iContainerType==RECIPE_TYPE_2x2)
	{
		// TODO Inventory dimensions need defined as constants
		m_inventoryControl->SetData( m_iPad, m_pPlayer->inventoryMenu, 3, 9, InventoryMenu::INV_SLOT_START, InventoryMenu::INV_SLOT_END );

		// TODO Inventory dimensions need defined as constants
		m_useRowControl->SetData( m_iPad, m_pPlayer->inventoryMenu, 1, 9, InventoryMenu::USE_ROW_SLOT_START, InventoryMenu::USE_ROW_SLOT_END );
	}
	else
	{
		CraftingMenu *menu = new CraftingMenu(m_pPlayer->inventory, m_pPlayer->level, pCraftingPanelData->x, pCraftingPanelData->y, pCraftingPanelData->z);
		Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu = menu;
		// TODO Inventory dimensions need defined as constants
		m_inventoryControl->SetData( m_iPad, menu, 3, 9, CraftingMenu::INV_SLOT_START, CraftingMenu::INV_SLOT_END );

		// TODO Inventory dimensions need defined as constants
		m_useRowControl->SetData( m_iPad, menu, 1, 9, CraftingMenu::USE_ROW_SLOT_START, CraftingMenu::USE_ROW_SLOT_END );
	}

	delete pCraftingPanelData;

	// if we are in splitscreen, then we need to figure out if we want to move this scene
	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
	}

	XuiElementSetShow(m_hGrid,TRUE);
	XuiElementSetShow(m_hPanel,TRUE);

	// Set up the CXuiCtrlCraftIngredientSlots
	if(m_iContainerType==RECIPE_TYPE_3x3)
	{
		m_pCursors=m_pHSlotsCraftingTableCursors;
		
		m_iIngredientsMaxSlotC = m_iIngredients3x3SlotC;
		for(int i=0;i<m_iIngredients3x3SlotC;i++)
		{
			XuiObjectFromHandle( m_hCraftIngredientA[i], &pObj );
			m_pCraftingIngredientA[i] = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		}
		XuiObjectFromHandle( m_hCraftOutput, &pObj );
		m_pCraftingOutput = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		m_pGroupA=static_cast<Recipy::_eGroupType *>(&m_GroupTypeMapping9GridA);
		m_pGroupTabA=static_cast<CXuiSceneCraftingPanel::_eGroupTab *>(&m_GroupTabBkgMapping3x3A);

		m_iCraftablesMaxHSlotC=m_iMaxHSlot3x3C;

		// set up the ingredients descriptions
		for(int i=0;i<4;i++)
		{
			XuiObjectFromHandle( m_hCraftIngredientDescA[i], &pObj );
			m_pCraftIngredientDescA[i] = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		}
	}
	else
	{
		m_pCursors=m_pHSlotsCraftingCursors;
		
		m_iIngredientsMaxSlotC = m_iIngredients2x2SlotC;
		for(int i=0;i<m_iIngredients2x2SlotC;i++)
		{
			XuiObjectFromHandle( m_hCraftIngredientA[i], &pObj );
			m_pCraftingIngredientA[i] = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		}

		XuiObjectFromHandle( m_hCraftOutput, &pObj );
		m_pCraftingOutput = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		m_pGroupA=static_cast<Recipy::_eGroupType *>(&m_GroupTypeMapping4GridA);
		m_pGroupTabA=static_cast<CXuiSceneCraftingPanel::_eGroupTab *>(&m_GroupTabBkgMapping2x2A);

		m_iCraftablesMaxHSlotC=m_iMaxHSlot2x2C;

		// set up the ingredients descriptions
		for(int i=0;i<4;i++)
		{
			XuiObjectFromHandle( m_hCraftIngredientDescA[i], &pObj );
			m_pCraftIngredientDescA[i] = static_cast<CXuiCtrlCraftIngredientSlot *>(pObj);
		}
	}

	// display the first group tab 
	m_hTabGroupA[m_iGroupIndex].SetShow(TRUE);

	// store the slot 0 position
	m_pHSlotsBrushImageControl[0]->GetPosition(&m_vSlot0Pos);
	m_pHSlotsBrushImageControl[1]->GetPosition(&vec);
	m_fSlotSize=vec.x-m_vSlot0Pos.x;

	// store the slot 0 highlight position
	m_hHighlight.GetPosition(&m_vSlot0HighlightPos);
	// Store the V slot position
	m_hScrollBar2.GetPosition(&m_vSlot0V2ScrollPos);
	m_hScrollBar3.GetPosition(&m_vSlot0V3ScrollPos);

	// get the position of the slot from the xui, and apply any offset needed
	for(int i=0;i<m_iCraftablesMaxHSlotC;i++)
	{
		m_pHSlotsBrushImageControl[i]->SetShow(FALSE);
	}

	XuiElementSetShow(m_hGridInventory,FALSE);

	m_hScrollBar2.SetShow(FALSE);
	m_hScrollBar3.SetShow(FALSE);

	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_CRAFTING);
	m_GroupName.SetText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));

	UpdateTooltips();

	// Update the tutorial state
	Minecraft *pMinecraft = Minecraft::GetInstance();

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		if(m_iContainerType==RECIPE_TYPE_2x2)
		{
			gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_2x2Crafting_Menu, this);
		}
		else
		{
			gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_3x3Crafting_Menu, this);
		}
	}
#endif

	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
//	OnTransitionEnd
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneCraftingPanel::OnTransitionEnd( XUIMessageTransition *pTransData, BOOL& bHandled )
{
	// are we being destroyed? If so, don't do anything 
	if(pTransData->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) 
	{
		return S_OK;
	}

	// Fix for issue caused by autosave while crafting is up
	if(pTransData->dwTransType == XUI_TRANSITION_TO || pTransData->dwTransType == XUI_TRANSITION_BACKTO)
	{
		// Update the tab icons
		if(m_iContainerType==RECIPE_TYPE_3x3)
		{
			for(int i=0;i<m_iMaxGroup3x3;i++)
			{
				m_hGroupIconA[i].PlayVisualRange(m_GroupIconNameA[m_pGroupA[i]],nullptr,m_GroupIconNameA[m_pGroupA[i]]);
				XuiElementSetShow(m_hGroupIconA[i].m_hObj,TRUE);
			}
		}
		else
		{
			for(int i=0;i<m_iMaxGroup2x2;i++)
			{
				m_hGroupIconA[i].PlayVisualRange(m_GroupIconNameA[m_pGroupA[i]],nullptr,m_GroupIconNameA[m_pGroupA[i]]);
				XuiElementSetShow(m_hGroupIconA[i].m_hObj,TRUE);
			}
		}


		// Display the tooltips
		ui.SetTooltips(m_iPad, IDS_TOOLTIPS_CREATE,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INVENTORY,-1,-1,-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
		CXuiSceneBase::EnableTooltip(m_iPad, BUTTON_TOOLTIP_A, FALSE );	

		// Check which recipes are available with the resources we have
		CheckRecipesAvailable();
		// reset the vertical slots
		iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
		iVSlotIndexA[1]=0;
		iVSlotIndexA[2]=1;
		UpdateVerticalSlots();
		UpdateHighlight();
	}

	return S_OK;
}

HRESULT CXuiSceneCraftingPanel::OnCustomMessage_InventoryUpdated()
{
	// Display the tooltips
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_CREATE,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INVENTORY,-1,-1,-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
	CXuiSceneBase::EnableTooltip(m_iPad, BUTTON_TOOLTIP_A, FALSE );	

	// Check which recipes are available with the resources we have
	CheckRecipesAvailable();
	UpdateVerticalSlots();
	UpdateHighlight();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
//	OnKeyDown
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneCraftingPanel::OnKeyDown(XUIMessageInput* pInputData, BOOL& bHandled)
{
	bHandled = handleKeyDown(pInputData->UserIndex, mapVKToAction(pInputData->dwKeyCode), (pInputData->dwFlags & XUI_INPUT_FLAG_REPEAT) != 0);

	return S_OK;
}

int CXuiSceneCraftingPanel::mapVKToAction(int vk)
{
	int action = MINECRAFT_ACTION_MAX;
	switch(vk)
	{
	case VK_PAD_A:
		action = ACTION_MENU_A;
		break;
	case VK_PAD_B:
	case VK_PAD_START:
		action = ACTION_MENU_B;
		break;
	case VK_PAD_X:
		action = ACTION_MENU_X;
		break;
	case VK_PAD_Y:
		action = ACTION_MENU_Y;
		break;
	case VK_PAD_DPAD_LEFT:
	case VK_PAD_LTHUMB_LEFT:
		action = ACTION_MENU_LEFT;
		break;
	case VK_PAD_DPAD_RIGHT:
	case VK_PAD_LTHUMB_RIGHT:
		action = ACTION_MENU_RIGHT;
		break;
	case VK_PAD_LTHUMB_UP:
	case VK_PAD_DPAD_UP:
		action = ACTION_MENU_UP;
		break;
	case VK_PAD_LTHUMB_DOWN:
	case VK_PAD_DPAD_DOWN:
		action = ACTION_MENU_DOWN;
		break;
	case VK_PAD_LTRIGGER:
		action = ACTION_MENU_PAGEUP;
		break;
	case VK_PAD_RTRIGGER:
		action = ACTION_MENU_PAGEDOWN;
		break;
	case VK_PAD_LSHOULDER:
		action = ACTION_MENU_LEFT_SCROLL;
		break;
	case VK_PAD_RSHOULDER:
		action = ACTION_MENU_RIGHT_SCROLL;
		break;
	case VK_PAD_RTHUMB_UP:
		action =  ACTION_MENU_OTHER_STICK_UP;
		break;
	case VK_PAD_RTHUMB_DOWN:
		action =  ACTION_MENU_OTHER_STICK_DOWN;
		break;
	case VK_PAD_RTHUMB_RIGHT:
		action =  ACTION_MENU_OTHER_STICK_RIGHT;
		break;
	case VK_PAD_RTHUMB_LEFT:
		action =  ACTION_MENU_OTHER_STICK_LEFT;
		break;
	};

	return action;
}

//////////////////////////////////////////////////////////////////////////
//
//	OnGetSourceImage
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneCraftingPanel::OnGetSourceImage(XUIMessageGetSourceImage* pData, BOOL& rfHandled)
{
	HRESULT hr = S_OK;
	//int iId=pData->iItem;
	int iId=(pData->iData>>22)&0x1FF;
	pData->szPath = nullptr;
	pData->bDirty=true;
	rfHandled = TRUE;
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//
//	OnDestroy
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneCraftingPanel::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}
#endif

	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();

	return S_OK;
}

HRESULT CXuiSceneCraftingPanel::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

HRESULT CXuiSceneCraftingPanel::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==IGNORE_KEYPRESS_TIMERID)
	{
		XuiKillTimer(m_hObj,IGNORE_KEYPRESS_TIMERID);
		m_bIgnoreKeyPresses=false;
	}

	return S_OK;
}

HRESULT CXuiSceneCraftingPanel::OnKillFocus(HXUIOBJ hObjGettingFocus, BOOL& bHandled)
{
	return S_OK;
}

int CXuiSceneCraftingPanel::getPad()
{
	return m_iPad;
}

void CXuiSceneCraftingPanel::hideAllHSlots()
{
	for(int i=0;i<m_iMaxHSlotC;i++)
	{
		m_pHSlotsBrushImageControl[i]->SetShow(FALSE);
	}
}

void CXuiSceneCraftingPanel::hideAllVSlots()
{
	for(int i=0;i<m_iMaxDisplayedVSlotC;i++)
	{
		m_pVSlotsBrushImageControl[i]->SetShow(FALSE);
	}
}


void CXuiSceneCraftingPanel::hideAllIngredientsSlots()
{
	for(int i=0;i<m_iIngredientsC;i++)
	{
		m_pCraftIngredientDescA[i]->SetShow(FALSE);
	}
}

void CXuiSceneCraftingPanel::setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_pHSlotsBrushImageControl[iIndex]->SetIcon(iPad, item, 9, uiAlpha, false);
	//m_pHSlotsBrushImageControl[iIndex]->SetPassThroughDataAssociation(MAKE_SLOTDISPLAY_ITEM_BITMASK(item->id,item->getAuxValue(),item->isFoil()),MAKE_SLOTDISPLAY_DATA_BITMASK (iPad, uiAlpha, false, item->GetCount(), 9,0) );
	//m_pHSlotsBrushImageControl[iIndex]->SetShow(TRUE);
}

void CXuiSceneCraftingPanel::setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_pVSlotsBrushImageControl[iIndex]->SetIcon(iPad, item, 9, uiAlpha, false);
	//m_pVSlotsBrushImageControl[iIndex]->SetPassThroughDataAssociation(MAKE_SLOTDISPLAY_ITEM_BITMASK(item->id,item->getAuxValue(),item->isFoil()),MAKE_SLOTDISPLAY_DATA_BITMASK (iPad, uiAlpha, false, item->GetCount(), 9,0) );
	//m_pVSlotsBrushImageControl[iIndex]->SetShow(TRUE);
}

void CXuiSceneCraftingPanel::setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item)
{
	if(item == nullptr)
	{
		m_pCraftingOutput->SetIcon(iPad, 0,0,0,0,0,false);
	}
	else
	{
		m_pCraftingOutput->SetIcon(iPad, item->id,item->getAuxValue(),item->GetCount(),12,31,true,item->isFoil());
	}
}

void CXuiSceneCraftingPanel::setCraftingOutputSlotRedBox(bool show)
{
	m_pCraftingOutput->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneCraftingPanel::setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	if(item == nullptr)
	{
		m_pCraftingIngredientA[index]->SetIcon(iPad, 0,0,0,0,0,false);
	}
	else
	{
		m_pCraftingIngredientA[index]->SetIcon(m_iPad, item->id,item->getAuxValue(),0,8,31,false);
	}
}

void CXuiSceneCraftingPanel::setIngredientSlotRedBox(int index, bool show)
{
	m_pCraftingIngredientA[index]->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneCraftingPanel::setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	m_pCraftIngredientDescA[index]->SetIcon(iPad, item->id,item->getAuxValue(),item->GetCount(),8,31,TRUE,item->isFoil(),FALSE);
}

void CXuiSceneCraftingPanel::setIngredientDescriptionRedBox(int index, bool show)
{
	m_pCraftIngredientDescA[index]->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneCraftingPanel::setIngredientDescriptionText(int index, LPCWSTR text)
{
	m_pCraftIngredientDescA[index]->SetDescription(text);
}

void CXuiSceneCraftingPanel::setShowCraftHSlot(int iIndex, bool show)
{
	m_pHSlotsBrushImageControl[iIndex]->SetShow(show?TRUE:FALSE);
}

void CXuiSceneCraftingPanel::showTabHighlight(int iIndex, bool show)
{
	m_hTabGroupA[iIndex].SetShow(show?TRUE:FALSE);
}

void CXuiSceneCraftingPanel::setGroupText(LPCWSTR text)
{
	m_GroupName.SetText(text);
}

void CXuiSceneCraftingPanel::setDescriptionText(LPCWSTR text)
{
	m_DescriptionText.SetText(text);
}

void CXuiSceneCraftingPanel::setItemText(LPCWSTR text)
{
	m_ItemName.SetText(text);
}

void CXuiSceneCraftingPanel::UpdateMultiPanel()
{
	switch(m_iDisplayDescription)
	{
	case DISPLAY_INVENTORY:
		// turn off all the ingredients display
		for(int i=0;i<4;i++)
		{
			m_pCraftIngredientDescA[i]->SetShow(FALSE);
		}

		XuiElementSetShow(m_hGridInventory,TRUE);
		XuiControlSetText(m_InventoryText,app.GetString(IDS_INVENTORY));
		XuiElementSetShow(m_InventoryText,TRUE);
		break;
	case DISPLAY_DESCRIPTION:
		// turn off the inventory
		XuiElementSetShow(m_hGridInventory,FALSE);
		XuiElementSetShow(m_DescriptionText,TRUE);
		XuiElementSetShow(m_InventoryText,FALSE);		
		break;
	case DISPLAY_INGREDIENTS:
		// turn off all the descriptions
		XuiElementSetShow(m_DescriptionText,FALSE);

		// display the ingredients
		for(int i=0;i<m_iIngredientsC;i++)
		{
			m_pCraftIngredientDescA[i]->SetShow(TRUE);
		}

		if(m_iIngredientsC==0)
		{
			XuiElementSetShow(m_InventoryText,FALSE);
		}
		else
		{
			XuiControlSetText(m_InventoryText,app.GetString(IDS_INGREDIENTS));
			XuiElementSetShow(m_InventoryText,TRUE);
		}		
		break;
	}
}

void CXuiSceneCraftingPanel::scrollDescriptionUp()
{
		XUIHtmlScrollInfo ScrollInfo;

		XuiHtmlControlGetVScrollInfo(m_DescriptionText.m_hObj,&ScrollInfo);
		if(!ScrollInfo.bScrolling)
		{
			XuiHtmlControlVScrollBy(m_DescriptionText.m_hObj,-1);
		}
}

void CXuiSceneCraftingPanel::scrollDescriptionDown()
{
		XUIHtmlScrollInfo ScrollInfo;

		XuiHtmlControlGetVScrollInfo(m_DescriptionText.m_hObj,&ScrollInfo);
		if(!ScrollInfo.bScrolling)
		{
			XuiHtmlControlVScrollBy(m_DescriptionText.m_hObj,1);
		}
}

void CXuiSceneCraftingPanel::updateHighlightAndScrollPositions()
{
	D3DXVECTOR3 vec;

	vec.z=0.0f;
	vec.x=m_vSlot0HighlightPos.x + (m_iCurrentSlotHIndex*m_fSlotSize);
	vec.y=m_vSlot0HighlightPos.y + ((m_iCurrentSlotVIndex-1)*m_fSlotSize); // vslot 1 is the centred one
	m_hHighlight.SetPosition(&vec);

	// Update the scroll icons for all h boxes
	for(int i=0;i<m_iCraftablesMaxHSlotC;i++)
	{
		if((i!=m_iCurrentSlotHIndex) &&(CanBeMadeA[i].iCount>1))
		{
			m_pCursors[i].SetShow(TRUE);
		}
		else
		{
			m_pCursors[i].SetShow(FALSE);
		}
	}

	if(CanBeMadeA[m_iCurrentSlotHIndex].iCount<2)
	{
		m_hScrollBar2.SetShow(FALSE);
		m_hScrollBar3.SetShow(FALSE);
	}
	else if(CanBeMadeA[m_iCurrentSlotHIndex].iCount<3)
	{
		// 2 slot
		vec.x=m_vSlot0V2ScrollPos.x + (m_iCurrentSlotHIndex*m_fSlotSize);
		vec.y=m_vSlot0V2ScrollPos.y;
		m_hScrollBar2.SetPosition(&vec);
		m_hScrollBar2.SetShow(TRUE);
		m_hScrollBar3.SetShow(FALSE);
	}
	else
	{
		// 3 slot
		vec.x=m_vSlot0V3ScrollPos.x + (m_iCurrentSlotHIndex*m_fSlotSize);
		vec.y=m_vSlot0V3ScrollPos.y;//+m_fSlotSize;

		m_hScrollBar3.SetPosition(&vec);
		m_hScrollBar2.SetShow(FALSE);
		m_hScrollBar3.SetShow(TRUE);
	}
}

void CXuiSceneCraftingPanel::updateVSlotPositions(int iSlots, int i)
{
	D3DXVECTOR3 vec;
	if(iSlots==2)
	{
		vec.y=m_vSlot0Pos.y + (1-i)*m_fSlotSize;
	}
	else
	{
		vec.y=m_vSlot0Pos.y + (i-1)*m_fSlotSize;
	}
	vec.x=m_vSlot0Pos.x + m_iCurrentSlotHIndex*m_fSlotSize;
	vec.z=0.0f;
	m_pVSlotsBrushImageControl[i]->SetPosition(&vec);
}