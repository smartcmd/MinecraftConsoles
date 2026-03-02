#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\Minecraft.h"
#include "UIScene_DispenserMenu.h"

UIScene_DispenserMenu::UIScene_DispenserMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	TrapScreenInput *initData = (TrapScreenInput *)_initData;

	m_labelDispenser.init(initData->trap->getName());

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Trap_Menu, this);
	}

	TrapMenu* menu = new TrapMenu( initData->inventory, initData->trap );

	m_containerSize = initData->trap->getContainerSize();
	Initialize( initData->iPad, menu, true, m_containerSize, eSectionTrapUsing, eSectionTrapMax );

	m_slotListTrap.addSlots(0, 9);

	delete initData;
}

wstring UIScene_DispenserMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"DispenserMenuSplit";
	}
	else
	{
		return L"DispenserMenu";
	}
}

void UIScene_DispenserMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, m_containerSize, eSectionTrapUsing, eSectionTrapMax );

	m_slotListTrap.addSlots(0, 9);
}

int UIScene_DispenserMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionTrapTrap:
		cols = 3;
		break;
	case eSectionTrapInventory:
		cols = 9;
		break;
	case eSectionTrapUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_DispenserMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionTrapTrap:
		rows = 3;
		break;
	case eSectionTrapInventory:
		rows = 3;
		break;
	case eSectionTrapUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_DispenserMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionTrapTrap:
		pPosition->x = m_slotListTrap.getXPos();
		pPosition->y = m_slotListTrap.getYPos();
		break;
	case eSectionTrapInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionTrapUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_DispenserMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionTrapTrap:
		sectionSize.x = m_slotListTrap.getWidth();
		sectionSize.y = m_slotListTrap.getHeight();
		break;
	case eSectionTrapInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionTrapUsing:
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

void UIScene_DispenserMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = NULL;
	switch( eSection )
	{
	case eSectionTrapTrap:
		slotList = &m_slotListTrap;
		break;
	case eSectionTrapInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionTrapUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	slotList->setHighlightSlot(index);
}

UIControl *UIScene_DispenserMenu::getSection(ESceneSection eSection)
{
	UIControl *control = NULL;
	switch( eSection )
	{
	case eSectionTrapTrap:
		control = &m_slotListTrap;
		break;
	case eSectionTrapInventory:
		control = &m_slotListInventory;
		break;
	case eSectionTrapUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}