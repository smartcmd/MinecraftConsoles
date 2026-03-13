#include "stdafx.h"
#include "UI.h"
#include "UIScene_ContainerMenu.h"

#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../LocalPlayer.h"
#include "../../Minecraft.h"
#include "../Tutorial/Tutorial.h"
#include "../Tutorial/TutorialMode.h"
#include "../Tutorial/TutorialEnum.h"

UIScene_ContainerMenu::UIScene_ContainerMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	ContainerScreenInput *initData = static_cast<ContainerScreenInput *>(_initData);
	m_bLargeChest = (initData->container->getContainerSize() > 3*9)?true:false;

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_labelChest.init(initData->container->getName());

	ContainerMenu* menu = new ContainerMenu( initData->inventory, initData->container );

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[initData->iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Container_Menu, this);
	}

	int containerSize = menu->getSize() - (27 + 9);

	Initialize( initData->iPad, menu, true, containerSize, eSectionContainerUsing, eSectionContainerMax);
	
	m_slotListContainer.addSlots(0, containerSize);

	if(initData) delete initData;
}

wstring UIScene_ContainerMenu::getMoviePath()
{
	if(m_bLargeChest)
	{
		if(app.GetLocalPlayerCount() > 1)
		{
			return L"ChestLargeMenuSplit";
		}
		else
		{
			return L"ChestLargeMenu";
		}
	}
	else
	{
		if(app.GetLocalPlayerCount() > 1)
		{
			return L"ChestMenuSplit";
		}
		else
		{
			return L"ChestMenu";
		}
	}
}

void UIScene_ContainerMenu::handleReload()
{
	int containerSize = m_menu->getSize() - (27 + 9);

	Initialize( m_iPad, m_menu, true, containerSize, eSectionContainerUsing, eSectionContainerMax );	

	m_slotListContainer.addSlots(0, containerSize);
}

int UIScene_ContainerMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
		case eSectionContainerChest:
			cols = 9;
			break;
		case eSectionContainerInventory:
			cols = 9;
			break;
		case eSectionContainerUsing:
			cols = 9;
			break;
		default:
			assert( false );
			break;
	}
	return cols;
}

int UIScene_ContainerMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
		case eSectionContainerChest:
			rows = (m_menu->getSize() - (27 + 9)) / 9;
			break;
		case eSectionContainerInventory:
			rows = 3;
			break;
		case eSectionContainerUsing:
			rows = 1;
			break;
		default:
			assert( false );
			break;
	}
	return rows;
}

void UIScene_ContainerMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionContainerChest:
		pPosition->x = m_slotListContainer.getXPos();
		pPosition->y = m_slotListContainer.getYPos();
		break;
	case eSectionContainerInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionContainerUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_ContainerMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionContainerChest:
		sectionSize.x = m_slotListContainer.getWidth();
		sectionSize.y = m_slotListContainer.getHeight();
		break;
	case eSectionContainerInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionContainerUsing:
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

void UIScene_ContainerMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionContainerChest:
		slotList = &m_slotListContainer;
		break;
	case eSectionContainerInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionContainerUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_ContainerMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionContainerChest:
		control = &m_slotListContainer;
		break;
	case eSectionContainerInventory:
		control = &m_slotListInventory;
		break;
	case eSectionContainerUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}