#include "stdafx.h"
#include "UI.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "UIScene_CraftingMenu.h"
#ifdef _WINDOWS64
#include "../../Windows64/Iggy/gdraw/gdraw_d3d11.h"
#endif

#ifdef __PSVITA__
#define GAME_CRAFTING_TOUCHUPDATE_TIMER_ID 0
#define GAME_CRAFTING_TOUCHUPDATE_TIMER_TIME 100
#endif

UIScene_CraftingMenu::UIScene_CraftingMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
#ifdef _WINDOWS64
	m_hSlotBoundsValid = false;
	m_hSlotX0 = m_hSlotY0 = m_hSlotY1 = 0;
	m_hSlotSpacing = 0;
#endif
	m_bIgnoreKeyPresses = false;

	CraftingPanelScreenInput* initData = static_cast<CraftingPanelScreenInput *>(_initData);
	m_iContainerType=initData->iContainerType;
	m_pPlayer=initData->player;
	m_bSplitscreen=initData->bSplitscreen;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	for(unsigned int i = 0; i < 4; ++i)	m_labelIngredientsDesc[i].init(L"");
	m_labelDescription.init(L"");
	m_labelGroupName.init(L"");
	m_labelItemName.init(L"");
	m_labelInventory.init( app.GetString(IDS_INVENTORY) );
	m_labelIngredients.init( app.GetString(IDS_INGREDIENTS) );

	if(m_iContainerType==RECIPE_TYPE_2x2)
	{
		m_menu = m_pPlayer->inventoryMenu;
		m_iMenuInventoryStart = InventoryMenu::INV_SLOT_START;
		m_iMenuHotBarStart = InventoryMenu::USE_ROW_SLOT_START;
	}
	else
	{
		CraftingMenu *menu = new CraftingMenu(m_pPlayer->inventory, m_pPlayer->level, initData->x, initData->y, initData->z);
		Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu = menu;

		m_menu = menu;
		m_iMenuInventoryStart = CraftingMenu::INV_SLOT_START;
		m_iMenuHotBarStart = CraftingMenu::USE_ROW_SLOT_START;
	}
	m_slotListInventory.addSlots(CRAFTING_INVENTORY_SLOT_START,CRAFTING_INVENTORY_SLOT_END - CRAFTING_INVENTORY_SLOT_START);
	m_slotListHotBar.addSlots(CRAFTING_HOTBAR_SLOT_START, CRAFTING_HOTBAR_SLOT_END - CRAFTING_HOTBAR_SLOT_START);

#if TO_BE_IMPLEMENTED
	// if we are in splitscreen, then we need to figure out if we want to move this scene
	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
	}

	XuiElementSetShow(m_hGrid,TRUE);
	XuiElementSetShow(m_hPanel,TRUE);
#endif

	if(m_iContainerType==RECIPE_TYPE_3x3)
	{
		m_iIngredientsMaxSlotC = m_iIngredients3x3SlotC;
		m_pGroupA=(Recipy::_eGroupType *)&m_GroupTypeMapping9GridA;
		m_pGroupTabA=(_eGroupTab *)&m_GroupTabBkgMapping3x3A;
		m_iCraftablesMaxHSlotC=m_iMaxHSlot3x3C;
	}
	else
	{
		m_iIngredientsMaxSlotC = m_iIngredients2x2SlotC;
		m_pGroupA=(Recipy::_eGroupType *)&m_GroupTypeMapping4GridA;
		m_pGroupTabA=(_eGroupTab *)&m_GroupTabBkgMapping2x2A;
		m_iCraftablesMaxHSlotC=m_iMaxHSlot2x2C;
	}

#if TO_BE_IMPLEMENTED


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

#endif

	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_CRAFTING);
	setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));

	// Update the tutorial state
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
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

#ifdef _TO_BE_IMPLEMENTED
	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);
#endif

	for(unsigned int i = 0; i < 4; ++i)
	{
		m_slotListIngredients[i].addSlot(CRAFTING_INGREDIENTS_DESCRIPTION_START + i);
	}
	m_slotListCraftingOutput.addSlot(CRAFTING_OUTPUT_SLOT_START);
	m_slotListIngredientsLayout.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);

	// 3 Slot vertical scroll
	m_slotListCrafting3VSlots[0].addSlot(CRAFTING_V_SLOT_START + 0);
	m_slotListCrafting3VSlots[1].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting3VSlots[2].addSlot(CRAFTING_V_SLOT_START + 2);

	// 2 Slot vertical scroll
	// 2 slot scroll has swapped order
	m_slotListCrafting2VSlots[0].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting2VSlots[1].addSlot(CRAFTING_V_SLOT_START + 0);

	// 1 Slot scroll (for 480 mainly)
	m_slotListCrafting1VSlots.addSlot(CRAFTING_V_SLOT_START);

	m_slotListCraftingHSlots.addSlots(CRAFTING_H_SLOT_START,m_iCraftablesMaxHSlotC);

	// Check which recipes are available with the resources we have
	CheckRecipesAvailable();
	// reset the vertical slots
	iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
	iVSlotIndexA[1]=0;
	iVSlotIndexA[2]=1;
	UpdateVerticalSlots();
	UpdateHighlight();

	if(initData) delete initData;

	// in this scene, we override the press sound with our own for crafting success or fail
	ui.OverrideSFX(m_iPad,ACTION_MENU_A,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,true);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,true);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,true);

	// 4J-PB - Must be after the CanBeMade list has been set up with CheckRecipesAvailable
	UpdateTooltips();

#ifdef __PSVITA__
	// initialise vita touch controls with ids
	for(unsigned int i = 0; i < ETouchInput_Count; ++i)
	{
		m_TouchInput[i].init(i);
	}
	ui.TouchBoxRebuild(this);
#endif
}

void UIScene_CraftingMenu::handleDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr && Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu->containerId == m_menu->containerId)
	{
		Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	}

	ui.OverrideSFX(m_iPad,ACTION_MENU_A,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,false);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,false);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,false);
}

EUIScene UIScene_CraftingMenu::getSceneType()
{
	if(m_iContainerType==RECIPE_TYPE_3x3)
	{
		return eUIScene_Crafting3x3Menu;
	}
	else
	{
		return eUIScene_Crafting2x2Menu;
	}
}

wstring UIScene_CraftingMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		m_bSplitscreen = true;
		if(m_iContainerType==RECIPE_TYPE_3x3)
		{
			return L"Crafting3x3MenuSplit";
		}
		else
		{
			return L"Crafting2x2MenuSplit";
		}
	}
	else
	{
		if(m_iContainerType==RECIPE_TYPE_3x3)
		{
			return L"Crafting3x3Menu";
		}
		else
		{
			return L"Crafting2x2Menu";
		}
	}
}

#if defined(__PSVITA__) || defined(_WINDOWS64)
UIControl* UIScene_CraftingMenu::GetMainPanel()
{
	return &m_controlMainPanel;
}
#endif

#ifdef __PSVITA__
void UIScene_CraftingMenu::handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased)
{
	// perform action on release
	if(bPressed)
	{
		if(iId == ETouchInput_CraftingHSlots)
		{
			m_iCraftingSlotTouchStartY = y;
		}
	}
	else if(bRepeat)
	{
		if(iId == ETouchInput_CraftingHSlots)
		{
			if(y >= m_iCraftingSlotTouchStartY + m_TouchInput[ETouchInput_CraftingHSlots].getHeight())			// scroll list down
			{
				if(iVSlotIndexA[1]==(CanBeMadeA[m_iCurrentSlotHIndex].iCount-1))
				{
					iVSlotIndexA[1]=0;
				}
				else
				{
					iVSlotIndexA[1]++;
				}
				ui.PlayUISFX(eSFX_Focus);

				UpdateVerticalSlots();
				UpdateHighlight();

				m_iCraftingSlotTouchStartY = y;
			}
			else if(y <= m_iCraftingSlotTouchStartY - m_TouchInput[ETouchInput_CraftingHSlots].getHeight())		// scroll list up
			{
				if(iVSlotIndexA[1]==0)
				{
					iVSlotIndexA[1]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
				}
				else
				{
					iVSlotIndexA[1]--;
				}		
				ui.PlayUISFX(eSFX_Focus);

				UpdateVerticalSlots();
				UpdateHighlight();

				m_iCraftingSlotTouchStartY = y;
			}
		}
	}
	else if(bReleased)
	{
		if(iId >= ETouchInput_TouchPanel_0 && iId <= ETouchInput_TouchPanel_6)		// Touch Change Group
		{
			m_iGroupIndex = iId;
			// turn on the new group
			showTabHighlight(m_iGroupIndex,true);

			m_iCurrentSlotHIndex=0;
			m_iCurrentSlotVIndex=1;
			CheckRecipesAvailable();
			// reset the vertical slots
			iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
			iVSlotIndexA[1]=0;
			iVSlotIndexA[2]=1;
			ui.PlayUISFX(eSFX_Focus);
			UpdateVerticalSlots();
			UpdateHighlight();
			setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));
		}
		else if(iId == ETouchInput_CraftingHSlots)									// Touch Change Slot
		{
			int iMaxHSlots = 0;
			if(m_iContainerType==RECIPE_TYPE_3x3)
			{
				iMaxHSlots = m_iMaxHSlot3x3C;
			}
			else
			{
				iMaxHSlots = m_iMaxHSlot2x2C;
			}

			int iNewSlot = (x - m_TouchInput[ETouchInput_CraftingHSlots].getXPos() - m_controlMainPanel.getXPos()) / m_TouchInput[ETouchInput_CraftingHSlots].getHeight();

			int iOldHSlot=m_iCurrentSlotHIndex;

			m_iCurrentSlotHIndex = iNewSlot;
			if(m_iCurrentSlotHIndex>=m_iCraftablesMaxHSlotC) m_iCurrentSlotHIndex=0;
			m_iCurrentSlotVIndex=1;
			// clear the indices
			iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
			iVSlotIndexA[1]=0;
			iVSlotIndexA[2]=1;	

			UpdateVerticalSlots();
			UpdateHighlight();
			// re-enable the old hslot
			if(CanBeMadeA[iOldHSlot].iCount>0)
			{
				setShowCraftHSlot(iOldHSlot,true);
			}
			ui.PlayUISFX(eSFX_Focus);
		}
	}
}

void UIScene_CraftingMenu::handleTouchBoxRebuild()
{
	addTimer(GAME_CRAFTING_TOUCHUPDATE_TIMER_ID,GAME_CRAFTING_TOUCHUPDATE_TIMER_TIME);
}

void UIScene_CraftingMenu::handleTimerComplete(int id)
{
	if(id == GAME_CRAFTING_TOUCHUPDATE_TIMER_ID)
	{
		// we cannot rebuild touch boxes in an iggy callback because it requires further iggy calls
		GetMainPanel()->UpdateControl();
		ui.TouchBoxRebuild(this);
		killTimer(GAME_CRAFTING_TOUCHUPDATE_TIMER_ID);
	}
}
#endif

#ifdef _WINDOWS64
bool UIScene_CraftingMenu::handleMouseClick(F32 x, F32 y)
{
	if (!m_hSlotBoundsValid || m_hSlotSpacing <= 0)
		return false;

	// Tab click — tabs sit directly above the H slot row. We derive their
	// bounds from the H slot positions cached in customDraw, since the Vita
	// TouchPanel controls are full-screen overlays with unusable bounds.
	int maxTabs = (m_iContainerType == RECIPE_TYPE_3x3) ? m_iMaxGroup3x3 : m_iMaxGroup2x2;
	F32 slotHeight = m_hSlotY1 - m_hSlotY0;
	F32 tabRowY0 = (m_hSlotY0 * 0.75f) - slotHeight * 1.55f;
	F32 tabRowY1 = tabRowY0 + (slotHeight * 1.7f);
	F32 tabRowWidth = m_hSlotSpacing * m_iCraftablesMaxHSlotC;
	F32 tabWidth = tabRowWidth / maxTabs;

	if (tabWidth > 0 && x >= m_hSlotX0 && x < m_hSlotX0 + tabRowWidth &&
		y >= tabRowY0 && y < tabRowY1)
	{
		int iTab = (int)((x - m_hSlotX0) / tabWidth);
		if (iTab >= 0 && iTab < maxTabs && iTab != m_iGroupIndex)
		{
			showTabHighlight(m_iGroupIndex, false);
			m_iGroupIndex = iTab;
			showTabHighlight(m_iGroupIndex, true);
			m_iCurrentSlotHIndex = 0;
			m_iCurrentSlotVIndex = 1;
			CheckRecipesAvailable();
			iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
			iVSlotIndexA[1] = 0;
			iVSlotIndexA[2] = 1;
			ui.PlayUISFX(eSFX_Focus);
			UpdateVerticalSlots();
			UpdateHighlight();
			setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));
		}
		return true;
	}

	// H slot click — select or craft
	F32 rowWidth = m_hSlotSpacing * m_iCraftablesMaxHSlotC;
	if (x >= m_hSlotX0 && x < m_hSlotX0 + rowWidth &&
		y >= m_hSlotY0 && y < m_hSlotY1)
	{
		int iNewSlot = (int)((x - m_hSlotX0) / m_hSlotSpacing);
		if (iNewSlot >= 0 && iNewSlot < m_iCraftablesMaxHSlotC)
		{
			// Only interact with populated slots
			if (CanBeMadeA[iNewSlot].iCount == 0)
				return true;

			if (iNewSlot == m_iCurrentSlotHIndex)
			{
				// Click on already-selected slot — craft the item
				handleKeyDown(m_iPad, ACTION_MENU_A, false);
			}
			else
			{
				int iOldHSlot = m_iCurrentSlotHIndex;
				m_iCurrentSlotHIndex = iNewSlot;
				m_iCurrentSlotVIndex = 1;
				iVSlotIndexA[0] = CanBeMadeA[m_iCurrentSlotHIndex].iCount - 1;
				iVSlotIndexA[1] = 0;
				iVSlotIndexA[2] = 1;
				UpdateVerticalSlots();
				UpdateHighlight();
				if (CanBeMadeA[iOldHSlot].iCount > 0)
					setShowCraftHSlot(iOldHSlot, true);
				ui.PlayUISFX(eSFX_Focus);
			}
			return true;
		}
	}
	// Consume all mouse clicks so misses don't generate ACTION_MENU_A
	// and accidentally craft. Only blocks mouse-originated presses.
	return true;
}
#endif

void UIScene_CraftingMenu::handleReload()
{
	m_slotListInventory.addSlots(CRAFTING_INVENTORY_SLOT_START,CRAFTING_INVENTORY_SLOT_END - CRAFTING_INVENTORY_SLOT_START);
	m_slotListHotBar.addSlots(CRAFTING_HOTBAR_SLOT_START, CRAFTING_HOTBAR_SLOT_END - CRAFTING_HOTBAR_SLOT_START);

	for(unsigned int i = 0; i < 4; ++i)
	{
		m_slotListIngredients[i].addSlot(CRAFTING_INGREDIENTS_DESCRIPTION_START + i);
	}
	m_slotListCraftingOutput.addSlot(CRAFTING_OUTPUT_SLOT_START);
	m_slotListIngredientsLayout.addSlots(CRAFTING_INGREDIENTS_LAYOUT_START, m_iIngredientsMaxSlotC);

	// 3 Slot vertical scroll
	m_slotListCrafting3VSlots[0].addSlot(CRAFTING_V_SLOT_START + 0);
	m_slotListCrafting3VSlots[1].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting3VSlots[2].addSlot(CRAFTING_V_SLOT_START + 2);

	// 2 Slot vertical scroll
	// 2 slot scroll has swapped order
	m_slotListCrafting2VSlots[0].addSlot(CRAFTING_V_SLOT_START + 1);
	m_slotListCrafting2VSlots[1].addSlot(CRAFTING_V_SLOT_START + 0);

	// 1 Slot scroll (for 480 mainly)
	m_slotListCrafting1VSlots.addSlot(CRAFTING_V_SLOT_START);

	m_slotListCraftingHSlots.addSlots(CRAFTING_H_SLOT_START,m_iCraftablesMaxHSlotC);

	app.DebugPrintf(app.USER_SR,"Reloading MultiPanel\n");
	int temp = m_iDisplayDescription;
	m_iDisplayDescription = m_iDisplayDescription==0?1:0;
	UpdateMultiPanel();
	m_iDisplayDescription = temp;
	UpdateMultiPanel();

	app.DebugPrintf(app.USER_SR,"Reloading Highlight and scroll\n");

	// reset the vertical slots
	m_iCurrentSlotHIndex = 0;
	m_iCurrentSlotVIndex = 1;
	iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
	iVSlotIndexA[1]=0;
	iVSlotIndexA[2]=1;
	UpdateVerticalSlots();
	UpdateHighlight();

	app.DebugPrintf(app.USER_SR,"Reloading tabs\n");
	showTabHighlight(0,false);
	showTabHighlight(m_iGroupIndex,true);
}

void UIScene_CraftingMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return;

	shared_ptr<ItemInstance> item = nullptr;
	int slotId = -1;
	float alpha = 1.0f;
	bool decorations = true;
	bool inventoryItem = false;
	swscanf(static_cast<wchar_t *>(region->name),L"slot_%d",&slotId);
	if (slotId == -1)
	{
		app.DebugPrintf("This is not the control we are looking for\n");
	}
	else if(slotId >= CRAFTING_INVENTORY_SLOT_START && slotId < CRAFTING_INVENTORY_SLOT_END)
	{
		int iIndex = slotId - CRAFTING_INVENTORY_SLOT_START;
		iIndex += m_iMenuInventoryStart;
		Slot *slot = m_menu->getSlot(iIndex);
		item = slot->getItem();
		inventoryItem = true;
	}
	else if(slotId >= CRAFTING_HOTBAR_SLOT_START && slotId < CRAFTING_HOTBAR_SLOT_END)
	{
		int iIndex = slotId - CRAFTING_HOTBAR_SLOT_START;
		iIndex += m_iMenuHotBarStart;
		Slot *slot = m_menu->getSlot(iIndex);
		item = slot->getItem();
		inventoryItem = true;
	}
	else if(slotId >= CRAFTING_V_SLOT_START && slotId < CRAFTING_V_SLOT_END )
	{
		decorations = false;
		int iIndex = slotId - CRAFTING_V_SLOT_START;
		if(m_vSlotsInfo[iIndex].show)
		{
			item = m_vSlotsInfo[iIndex].item;
			alpha = static_cast<float>(m_vSlotsInfo[iIndex].alpha)/31.0f;
		}
	}
	else if(slotId >= CRAFTING_H_SLOT_START && slotId < (CRAFTING_H_SLOT_START + m_iCraftablesMaxHSlotC) )
	{
		decorations = false;
		int iIndex = slotId - CRAFTING_H_SLOT_START;
#ifdef _WINDOWS64
		// Cache H slot SWF-space positions from the custom draw transform matrix
		if (iIndex == 0 || iIndex == 1)
		{
			F32 mat[16];
			gdraw_D3D11_CalculateCustomDraw_4J(region, mat);
			// Matrix to SWF coords (same formula as setupCustomDrawMatrices)
			F32 sw = (F32)getRenderWidth();
			F32 sh = (F32)getRenderHeight();
			F32 swfX = sw * (1.0f + mat[3]) / 2.0f;
			F32 swfY = sh * (1.0f - mat[7]) / 2.0f;
			if (iIndex == 0)
			{
				m_hSlotX0 = swfX;
				m_hSlotY0 = swfY;
				// Slot visual height from matrix scale and region height
				F32 slotH = sh * (-mat[5]) / 2.0f * region->y1;
				m_hSlotY1 = swfY + slotH;
			}
			else
			{
				m_hSlotSpacing = swfX - m_hSlotX0;
				m_hSlotBoundsValid = (m_hSlotSpacing > 0);
			}
		}
#endif
		if(m_hSlotsInfo[iIndex].show)
		{
			item = m_hSlotsInfo[iIndex].item;
			alpha = static_cast<float>(m_hSlotsInfo[iIndex].alpha)/31.0f;
		}
	}
	else if(slotId >= CRAFTING_INGREDIENTS_LAYOUT_START && slotId < (CRAFTING_INGREDIENTS_LAYOUT_START + m_iIngredientsMaxSlotC) )
	{
		int iIndex = slotId - CRAFTING_INGREDIENTS_LAYOUT_START;
		if(m_ingredientsSlotsInfo[iIndex].show)
		{
			item = m_ingredientsSlotsInfo[iIndex].item;
			alpha = static_cast<float>(m_ingredientsSlotsInfo[iIndex].alpha)/31.0f;
		}
	}
	else if(slotId >= CRAFTING_INGREDIENTS_DESCRIPTION_START && slotId < (CRAFTING_INGREDIENTS_DESCRIPTION_START + 4) )
	{
		int iIndex = slotId - CRAFTING_INGREDIENTS_DESCRIPTION_START;
		if(m_ingredientsInfo[iIndex].show)
		{
			item = m_ingredientsInfo[iIndex].item;
			alpha = static_cast<float>(m_ingredientsInfo[iIndex].alpha)/31.0f;
		}
	}
	else if(slotId == CRAFTING_OUTPUT_SLOT_START )
	{
		if(m_craftingOutputSlotInfo.show)
		{
			item = m_craftingOutputSlotInfo.item;
			alpha = static_cast<float>(m_craftingOutputSlotInfo.alpha)/31.0f;
		}
	}

	if(item != nullptr)
	{
		if(!inventoryItem)
		{
			if( item->id == Item::clock_Id || item->id == Item::compass_Id )
			{
				// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
				// rather than the dynamic one for the player
				item->setAuxValue(0xFF);
			}
			else if( (item->getAuxValue() & 0xFF) == 0xFF)
			{
				// 4J Stu - If the aux value is set to match any
				item->setAuxValue(0);
			}
		}
		customDrawSlotControl(region,m_iPad,item,alpha,item->isFoil(),decorations);
	}
}

int UIScene_CraftingMenu::getPad()
{
	return m_iPad;
}

bool UIScene_CraftingMenu::allowRepeat(int key)
{
	switch(key)
	{
	// X is used to open this menu, so don't let it repeat
	case ACTION_MENU_X:
		return false;
	}
	return true;
}

void UIScene_CraftingMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_InventoryMenu handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_OTHER_STICK_UP:
	case ACTION_MENU_OTHER_STICK_DOWN:
		sendInputToMovie(key,repeat,pressed,released);
		break;
	default:
		if(pressed)
		{
			handled = handleKeyDown(m_iPad, key, repeat);
		}
		break;
	};
}

void UIScene_CraftingMenu::hideAllHSlots()
{
	for(unsigned int iIndex = 0; iIndex < m_iMaxHSlotC; ++iIndex)
	{
		m_hSlotsInfo[iIndex].item = nullptr;
		m_hSlotsInfo[iIndex].alpha = 31;
		m_hSlotsInfo[iIndex].show = false;
	}
}

void UIScene_CraftingMenu::hideAllVSlots()
{
	for(unsigned int iIndex = 0; iIndex < m_iMaxDisplayedVSlotC; ++iIndex)
	{
		m_vSlotsInfo[iIndex].item = nullptr;
		m_vSlotsInfo[iIndex].alpha = 31;
		m_vSlotsInfo[iIndex].show = false;
	}
}

void UIScene_CraftingMenu::hideAllIngredientsSlots()
{
	for(int i=0;i<m_iIngredientsC;i++)
	{
		m_ingredientsInfo[i].item = nullptr;
		m_ingredientsInfo[i].alpha = 31;
		m_ingredientsInfo[i].show = false;

		m_labelIngredientsDesc[i].setLabel(L"");

		IggyDataValue result;
		IggyDataValue value[2];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = i;

		value[1].type = IGGY_DATATYPE_boolean;
		value[1].boolval = false;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowIngredientSlot , 2 , value );
	}
}

void UIScene_CraftingMenu::setCraftHSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_hSlotsInfo[iIndex].item = item;
	m_hSlotsInfo[iIndex].alpha = uiAlpha;
	m_hSlotsInfo[iIndex].show = true;
}

void UIScene_CraftingMenu::setCraftVSlotItem(int iPad, int iIndex, shared_ptr<ItemInstance> item, unsigned int uiAlpha)
{
	m_vSlotsInfo[iIndex].item = item;
	m_vSlotsInfo[iIndex].alpha = uiAlpha;
	m_vSlotsInfo[iIndex].show = true;
}

void UIScene_CraftingMenu::setCraftingOutputSlotItem(int iPad, shared_ptr<ItemInstance> item)
{
	m_craftingOutputSlotInfo.item = item;
	m_craftingOutputSlotInfo.alpha = 31;
	m_craftingOutputSlotInfo.show = item != nullptr;
}

void UIScene_CraftingMenu::setCraftingOutputSlotRedBox(bool show)
{
	m_slotListCraftingOutput.showSlotRedBox(0,show);
}

void UIScene_CraftingMenu::setIngredientSlotItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	m_ingredientsSlotsInfo[index].item = item;
	m_ingredientsSlotsInfo[index].alpha = 31;
	m_ingredientsSlotsInfo[index].show = item != nullptr;
}

void UIScene_CraftingMenu::setIngredientSlotRedBox(int index, bool show)
{
	m_slotListIngredientsLayout.showSlotRedBox(index,show);
}

void UIScene_CraftingMenu::setIngredientDescriptionItem(int iPad, int index, shared_ptr<ItemInstance> item)
{
	m_ingredientsInfo[index].item = item;
	m_ingredientsInfo[index].alpha = 31;
	m_ingredientsInfo[index].show = item != nullptr;

	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = m_ingredientsInfo[index].show;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowIngredientSlot , 2 , value );
}

void UIScene_CraftingMenu::setIngredientDescriptionRedBox(int index, bool show)
{
	m_slotListIngredients[index].showSlotRedBox(0,show);
}

void UIScene_CraftingMenu::setIngredientDescriptionText(int index, LPCWSTR text)
{
	m_labelIngredientsDesc[index].setLabel(text);
}


void UIScene_CraftingMenu::setShowCraftHSlot(int iIndex, bool show)
{
	m_hSlotsInfo[iIndex].show = show;
}

void UIScene_CraftingMenu::showTabHighlight(int iIndex, bool show)
{
	if(show)
	{	
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = iIndex;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcSetActiveTab , 1 , value );
	}
}

void UIScene_CraftingMenu::setGroupText(LPCWSTR text)
{
	m_labelGroupName.setLabel(text);
}

void UIScene_CraftingMenu::setDescriptionText(LPCWSTR text)
{
	m_labelDescription.setLabel(text);
}

void UIScene_CraftingMenu::setItemText(LPCWSTR text)
{
	m_labelItemName.setLabel(text);
}

void UIScene_CraftingMenu::UpdateMultiPanel()
{
	// Call Iggy function to show the current panel
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = m_iDisplayDescription;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcShowPanelDisplay , 1 , value );
}

void UIScene_CraftingMenu::scrollDescriptionUp()
{
	// handled differently
}

void UIScene_CraftingMenu::scrollDescriptionDown()
{
	// handled differently
}

void UIScene_CraftingMenu::updateHighlightAndScrollPositions()
{
	{	
		IggyDataValue result;
		IggyDataValue value[2];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = m_iCurrentSlotHIndex;

		int selectorType = 0;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount  == 2)
		{
			selectorType = 1;
		}
		else if( CanBeMadeA[m_iCurrentSlotHIndex].iCount  > 2)
		{
			selectorType = 2;
		}

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = selectorType;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcMoveSelector , 2 , value );
	}

	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = m_iCurrentSlotVIndex;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcSelectVerticalItem , 1 , value );
	}
}

void UIScene_CraftingMenu::HandleMessage(EUIMessage message, void *data)
{
	switch(message)
	{
	case eUIMessage_InventoryUpdated:
		handleInventoryUpdated(data);
		break;
	};
}

void UIScene_CraftingMenu::handleInventoryUpdated(LPVOID data)
{
	HandleInventoryUpdated();
}

void UIScene_CraftingMenu::updateVSlotPositions(int iSlots, int i)
{
	// Not needed
}
