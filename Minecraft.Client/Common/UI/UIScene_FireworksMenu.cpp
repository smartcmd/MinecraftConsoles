#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\Minecraft.h"
#include "..\..\LocalPlayer.h"
#include "UIScene_FireworksMenu.h"

UIScene_FireworksMenu::UIScene_FireworksMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	FireworksScreenInput *initData = (FireworksScreenInput *)_initData;

	m_labelFireworks.init(app.GetString(IDS_HOW_TO_PLAY_MENU_FIREWORKS));

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Fireworks_Menu, this);
	}

	FireworksMenu* menu = new FireworksMenu( initData->player->inventory, initData->player->level, initData->x, initData->y, initData->z );

	Initialize( initData->iPad, menu, true, FireworksMenu::INV_SLOT_START, eSectionFireworksUsing, eSectionFireworksMax );

	m_slotListResult.addSlots(FireworksMenu::RESULT_SLOT,1);
	m_slotList3x3.addSlots(FireworksMenu::CRAFT_SLOT_START, 9);
	ShowLargeCraftingGrid(true);

	delete initData;
}

wstring UIScene_FireworksMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"FireworksMenuSplit";
	}
	else
	{
		return L"FireworksMenu";
	}
}

void UIScene_FireworksMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, FireworksMenu::INV_SLOT_START, eSectionFireworksUsing, eSectionFireworksMax );
	
	m_slotListResult.addSlots(FireworksMenu::RESULT_SLOT,1);
	m_slotList3x3.addSlots(FireworksMenu::CRAFT_SLOT_START, 9);
	ShowLargeCraftingGrid(true);
}

int UIScene_FireworksMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		cols = 3;
		break;
	case eSectionFireworksResult:
		cols = 1;
		break;
	case eSectionFireworksInventory:
		cols = 9;
		break;
	case eSectionFireworksUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_FireworksMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		rows = 3;
		break;
	case eSectionFireworksResult:
		rows = 1;
		break;
	case eSectionFireworksInventory:
		rows = 3;
		break;
	case eSectionFireworksUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_FireworksMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		pPosition->x = m_slotList3x3.getXPos();
		pPosition->y = m_slotList3x3.getYPos();
		break;
	case eSectionFireworksResult:
		pPosition->x = m_slotListResult.getXPos();
		pPosition->y = m_slotListResult.getYPos();
		break;
	case eSectionFireworksInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionFireworksUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_FireworksMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		sectionSize.x = m_slotList3x3.getWidth();
		sectionSize.y = m_slotList3x3.getHeight();
		break;
	case eSectionFireworksResult:
		sectionSize.x = m_slotListResult.getWidth();
		sectionSize.y = m_slotListResult.getHeight();
		break;
	case eSectionFireworksInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionFireworksUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
	default:
		assert( false );
		break;
	}

	int rows = getSectionRows(eSection);
	int cols = getSectionColumns(eSection);

	pSize->x = sectionSize.x/cols;
	pSize->y = sectionSize.y/rows;

	int itemCol = iItemIndex % cols;
	int itemRow = iItemIndex/cols;

	pPosition->x = itemCol * pSize->x;
	pPosition->y = itemRow * pSize->y;
}

void UIScene_FireworksMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = NULL;
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		slotList = &m_slotList3x3;
		break;
	case eSectionFireworksResult:
		slotList = &m_slotListResult;
		break;
	case eSectionFireworksInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionFireworksUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	slotList->setHighlightSlot(index);
}

UIControl *UIScene_FireworksMenu::getSection(ESceneSection eSection)
{
	UIControl *control = NULL;
	switch( eSection )
	{
	case eSectionFireworksIngredients:
		control = &m_slotList3x3;
		break;
	case eSectionFireworksResult:
		control = &m_slotListResult;
		break;
	case eSectionFireworksInventory:
		control = &m_slotListInventory;
		break;
	case eSectionFireworksUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}

// bShow == true removes the 2x2 crafting grid and bShow == false removes the 3x3 crafting grid
void UIScene_FireworksMenu::ShowLargeCraftingGrid(boolean bShow)
{
	app.DebugPrintf("ShowLargeCraftingGrid to %d\n", bShow);

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = bShow;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowLargeCraftingGrid , 1 , value );
}