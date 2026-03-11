#include "stdafx.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

#include "..\..\..\Minecraft.World\Container.h"
#include "..\..\..\Minecraft.World\Slot.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"

#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\Common\Tutorial\TutorialMode.h"
#include "..\..\Minecraft.h"

#include "XUI_Scene_AbstractContainer.h"
#include "XUI_Ctrl_SlotItem.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Ctrl_SlotItemListItem.h"

#include "..\..\Common\Potion_macros.h"

//--------------------------------------------------------------------------------------
// Name: CXuiSceneInventoryCreative::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneInventoryCreative::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	D3DXVECTOR3 vec;
	MapChildControls();

	Minecraft *pMinecraft = Minecraft::GetInstance();

	InventoryScreenInput *initData = static_cast<InventoryScreenInput *>(pInitData->pvInitData);
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;

	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		if(m_bSplitscreen)
		{
			app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
		}
	}

#ifdef _XBOX
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Creative_Inventory_Menu, this);
	}
#endif

	// 4J JEV - Does this still count as opening the inventory?
	initData->player->awardStat(GenericStats::openInventory(), GenericStats::param_noArgs());

	// 4J JEV - Item Picker Menu
	shared_ptr<SimpleContainer> creativeContainer = std::make_shared<SimpleContainer>(0, TabSpec::MAX_SIZE + 9);
	itemPickerMenu = new ItemPickerMenu(creativeContainer, initData->player->inventory);
	
	// 4J JEV - InitDataAssociations.
	m_containerControl->SetData( initData->iPad, itemPickerMenu,  TabSpec::rows, TabSpec::columns,    0, TabSpec::MAX_SIZE );
	m_useRowControl->SetData(    initData->iPad, itemPickerMenu,  1, 9,  TabSpec::MAX_SIZE, TabSpec::MAX_SIZE + 9 );	
	m_pointerControl->SetUserIndex(m_pointerControl->m_hObj, initData->iPad);

	// Initialize superclass.
	CXuiSceneAbstractContainer::Initialize( initData->iPad, itemPickerMenu, false, -1, eSectionInventoryCreativeUsing, eSectionInventoryCreativeMax, initData->bNavigateBack );

	delete initData;

	// Change the point at which the cursor stops so we can't move the pointer over the tabs
	D3DXVECTOR3 containerPos;
	m_containerControl->GetPosition(&containerPos);
	m_fPointerMinY += containerPos.y;

	// 4J JEV - Settup Tabs
	for (int i = 0; i < eCreativeInventoryTab_COUNT; i++) 
	{
		m_hTabGroupA[i].SetShow(FALSE);
	}

	m_curTab = eCreativeInventoryTab_COUNT;
	switchTab(eCreativeInventoryTab_BuildingBlocks);

	return S_OK;
}

HRESULT CXuiSceneInventoryCreative::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

#ifdef _XBOX
	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}
#endif

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
//	OnTransitionEnd
//
//////////////////////////////////////////////////////////////////////////
HRESULT CXuiSceneInventoryCreative::OnTransitionEnd( XUIMessageTransition *pTransData, BOOL& bHandled )
{
	// are we being destroyed? If so, don't do anything 
	if(pTransData->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) 
	{
		return S_OK;
	}

	// Fix for issue caused by autosave while crafting is up
	if(pTransData->dwTransType == XUI_TRANSITION_TO || pTransData->dwTransType == XUI_TRANSITION_BACKTO)
	{
		for(int i=0;i<eCreativeInventoryTab_COUNT;i++)
		{
			m_hGroupIconA[i].PlayVisualRange(specs[i]->m_icon,nullptr,specs[i]->m_icon);
			XuiElementSetShow(m_hGroupIconA[i].m_hObj,TRUE);
		}
	}

	return S_OK;
}

CXuiControl* CXuiSceneInventoryCreative::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionInventoryCreativeUsing:
		return static_cast<CXuiControl *>(m_useRowControl);
		break;
	case eSectionInventoryCreativeSelector:
		return static_cast<CXuiControl *>(m_containerControl);
		break;
	default:
		assert( false );
		break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneInventoryCreative::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
	case eSectionInventoryCreativeUsing:
		return m_useRowControl;
		break;
	case eSectionInventoryCreativeSelector:
		return m_containerControl;
		break;
	default:
		assert( false );
		break;
	}
	return nullptr;
}

void CXuiSceneInventoryCreative::updateTabHighlightAndText(ECreativeInventoryTabs tab)
{
	if (m_curTab < eCreativeInventoryTab_COUNT)
	{
		m_hTabGroupA[m_curTab].SetShow(FALSE);
	}

	m_hTabGroupA[tab].SetShow(TRUE);
	wstring wsText=app.GetString(specs[tab]->m_descriptionId);
	m_GroupDescription.SetText(wsText.c_str());
	m_GroupDescription.SetShow(TRUE);
}

void CXuiSceneInventoryCreative::updateScrollCurrentPage(int currentPage, int pageCount)
{
	m_pageSlider.SetEnable(pageCount > 1);

	if(pageCount == 1)
	{
		m_pageSlider.SetRange(0,1);
		m_pageSlider.SetValue(0);
	}
	else
	{
		m_pageSlider.SetRange(0,pageCount - 1);
		m_pageSlider.SetValue(currentPage - 1);
	}

	m_scrollUp.SetShow(currentPage > 1);
	m_scrollUp.PlayOptionalVisual(L"ScrollMore",L"EndScrollMore");

	
	m_scrollDown.SetShow(currentPage < pageCount);
	m_scrollDown.PlayOptionalVisual(L"ScrollMore",L"EndScrollMore");

	//wchar_t pageNum[10];
	//swprintf(pageNum,10,L"%d/%d",currentPage,pageCount);
	//m_pageNumber.SetText(pageNum);
}