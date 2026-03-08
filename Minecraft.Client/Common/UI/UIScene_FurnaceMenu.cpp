#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../Minecraft.h"
#include "UIScene_FurnaceMenu.h"

UIScene_FurnaceMenu::UIScene_FurnaceMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	FurnaceScreenInput *initData = (FurnaceScreenInput *)_initData;
	m_furnace = initData->furnace;

	m_labelFurnace.init(m_furnace->getName());
	m_labelIngredient.init(app.GetString(IDS_INGREDIENT));
	m_labelFuel.init(app.GetString(IDS_FUEL));

	m_progressFurnaceFire.init(L"",0,0,12,0);
	m_progressFurnaceArrow.init(L"",0,0,24,0);

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Furnace_Menu, this);
	}

	FurnaceMenu* menu = new FurnaceMenu( initData->inventory, initData->furnace );

	Initialize( initData->iPad, menu, true, FurnaceMenu::INV_SLOT_START, eSectionFurnaceUsing, eSectionFurnaceMax );

	m_slotListFuel.addSlots(FurnaceMenu::FUEL_SLOT, 1);
	m_slotListIngredient.addSlots(FurnaceMenu::INGREDIENT_SLOT, 1);
	m_slotListResult.addSlots(FurnaceMenu::RESULT_SLOT, 1);

	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_FORGING);

	delete initData;
}

wstring UIScene_FurnaceMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"FurnaceMenuSplit";
	}
	else
	{
		return L"FurnaceMenu";
	}
}

void UIScene_FurnaceMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, FurnaceMenu::INV_SLOT_START, eSectionFurnaceUsing, eSectionFurnaceMax );

	m_slotListFuel.addSlots(FurnaceMenu::FUEL_SLOT, 1);
	m_slotListIngredient.addSlots(FurnaceMenu::INGREDIENT_SLOT, 1);
	m_slotListResult.addSlots(FurnaceMenu::RESULT_SLOT, 1);
}

void UIScene_FurnaceMenu::tick()
{
	m_progressFurnaceFire.setProgress( m_furnace->getLitProgress( 12 ) );
	m_progressFurnaceArrow.setProgress( m_furnace->getBurnProgress( 24 ) );
	UIScene_AbstractContainerMenu::tick();
}

int UIScene_FurnaceMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionFurnaceResult:
		cols = 1;
		break;
	case eSectionFurnaceFuel:
		cols = 1;
		break;
	case eSectionFurnaceIngredient:
		cols = 1;
		break;
	case eSectionFurnaceInventory:
		cols = 9;
		break;
	case eSectionFurnaceUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_FurnaceMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionFurnaceResult:
		rows = 1;
		break;
	case eSectionFurnaceFuel:
		rows = 1;
		break;
	case eSectionFurnaceIngredient:
		rows = 1;
		break;
	case eSectionFurnaceInventory:
		rows = 3;
		break;
	case eSectionFurnaceUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_FurnaceMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionFurnaceResult:
		pPosition->x = m_slotListResult.getXPos();
		pPosition->y = m_slotListResult.getYPos();
		break;
	case eSectionFurnaceFuel:
		pPosition->x = m_slotListFuel.getXPos();
		pPosition->y = m_slotListFuel.getYPos();
		break;
	case eSectionFurnaceIngredient:
		pPosition->x = m_slotListIngredient.getXPos();
		pPosition->y = m_slotListIngredient.getYPos();
		break;
	case eSectionFurnaceInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionFurnaceUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_FurnaceMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionFurnaceResult:
		sectionSize.x = m_slotListResult.getWidth();
		sectionSize.y = m_slotListResult.getHeight();
		break;
	case eSectionFurnaceFuel:
		sectionSize.x = m_slotListFuel.getWidth();
		sectionSize.y = m_slotListFuel.getHeight();
		break;
	case eSectionFurnaceIngredient:
		sectionSize.x = m_slotListIngredient.getWidth();
		sectionSize.y = m_slotListIngredient.getHeight();
		break;
	case eSectionFurnaceInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionFurnaceUsing:
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

void UIScene_FurnaceMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = NULL;
	switch( eSection )
	{
	case eSectionFurnaceResult:
		slotList = &m_slotListResult;
		break;
	case eSectionFurnaceFuel:
		slotList = &m_slotListFuel;
		break;
	case eSectionFurnaceIngredient:
		slotList = &m_slotListIngredient;
		break;
	case eSectionFurnaceInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionFurnaceUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_FurnaceMenu::getSection(ESceneSection eSection)
{
	UIControl *control = NULL;
	switch( eSection )
	{
	case eSectionFurnaceResult:
		control = &m_slotListResult;
		break;
	case eSectionFurnaceFuel:
		control = &m_slotListFuel;
		break;
	case eSectionFurnaceIngredient:
		control = &m_slotListIngredient;
		break;
	case eSectionFurnaceInventory:
		control = &m_slotListInventory;
		break;
	case eSectionFurnaceUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}
