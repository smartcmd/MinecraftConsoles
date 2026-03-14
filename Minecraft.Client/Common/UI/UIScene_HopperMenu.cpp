#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../Minecraft.h"
#include "UIScene_HopperMenu.h"

UIScene_HopperMenu::UIScene_HopperMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	HopperScreenInput *initData = static_cast<HopperScreenInput *>(_initData);

	m_labelDispenser.init(initData->hopper->getName());

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[initData->iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Hopper_Menu, this);
	}

	HopperMenu* menu = new HopperMenu( initData->inventory, initData->hopper );

	m_containerSize = initData->hopper->getContainerSize();
	Initialize( initData->iPad, menu, true, m_containerSize, eSectionHopperUsing, eSectionHopperMax );

	m_slotListTrap.addSlots(0, 9);

	delete initData;
}

wstring UIScene_HopperMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"HopperMenuSplit";
	}
	else
	{
		return L"HopperMenu";
	}
}

void UIScene_HopperMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, m_containerSize, eSectionHopperUsing, eSectionHopperMax );

	m_slotListTrap.addSlots(0, 9);
}

int UIScene_HopperMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionHopperContents:
		cols = 5;
		break;
	case eSectionHopperInventory:
		cols = 9;
		break;
	case eSectionHopperUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_HopperMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionHopperContents:
		rows = 1;
		break;
	case eSectionHopperInventory:
		rows = 3;
		break;
	case eSectionHopperUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_HopperMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionHopperContents:
		pPosition->x = m_slotListTrap.getXPos();
		pPosition->y = m_slotListTrap.getYPos();
		break;
	case eSectionHopperInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionHopperUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_HopperMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionHopperContents:
		sectionSize.x = m_slotListTrap.getWidth();
		sectionSize.y = m_slotListTrap.getHeight();
		break;
	case eSectionHopperInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionHopperUsing:
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

void UIScene_HopperMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionHopperContents:
		slotList = &m_slotListTrap;
		break;
	case eSectionHopperInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionHopperUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	slotList->setHighlightSlot(index);
}

UIControl *UIScene_HopperMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionHopperContents:
		control = &m_slotListTrap;
		break;
	case eSectionHopperInventory:
		control = &m_slotListInventory;
		break;
	case eSectionHopperUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}