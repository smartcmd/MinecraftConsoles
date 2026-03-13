#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.item.alchemy.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../../Minecraft.h"
#include "UIScene_BrewingStandMenu.h"

UIScene_BrewingStandMenu::UIScene_BrewingStandMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_progressBrewingArrow.init(L"",0,0,PotionBrewing::BREWING_TIME_SECONDS * SharedConstants::TICKS_PER_SECOND,0);
	m_progressBrewingBubbles.init(L"",0,0,30,0);

	BrewingScreenInput *initData = static_cast<BrewingScreenInput *>(_initData);
	m_brewingStand = initData->brewingStand;

	m_labelBrewingStand.init( m_brewingStand->getName() );

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[initData->iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Brewing_Menu, this);
	}

	BrewingStandMenu* menu = new BrewingStandMenu( initData->inventory, initData->brewingStand );

	Initialize( initData->iPad, menu, true, BrewingStandMenu::INV_SLOT_START, eSectionBrewingUsing, eSectionBrewingMax );

	m_slotListIngredient.addSlots(BrewingStandMenu::INGREDIENT_SLOT, 1);

	for(unsigned int i = 0; i < 3; ++i)
	{
		m_slotListBottles[i].addSlots(BrewingStandMenu::BOTTLE_SLOT_START + i, 1);
	}

	if(initData) delete initData;

	app.SetRichPresenceContext(iPad, CONTEXT_GAME_STATE_BREWING);
}

wstring UIScene_BrewingStandMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"BrewingStandMenuSplit";
	}
	else
	{
		return L"BrewingStandMenu";
	}
}

void UIScene_BrewingStandMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, BrewingStandMenu::INV_SLOT_START, eSectionBrewingUsing, eSectionBrewingMax  );

	m_slotListIngredient.addSlots(BrewingStandMenu::INGREDIENT_SLOT, 1);

	for(unsigned int i = 0; i < 3; ++i)
	{
		m_slotListBottles[i].addSlots(BrewingStandMenu::BOTTLE_SLOT_START + i, 1);
	}
}

void UIScene_BrewingStandMenu::tick()
{
	m_progressBrewingArrow.setProgress( m_brewingStand->getBrewTime() );

	int value = 0;
	int bubbleStep = (m_brewingStand->getBrewTime() / 2) % 7;
	switch (bubbleStep)
	{
	case 0:
		value = 0;
		break;
	case 6:
		value = 5;
		break;
	case 5:
		value = 10;
		break;
	case 4:
		value = 15;
		break;
	case 3:
		value = 20;
		break;
	case 2:
		value = 25;
		break;
	case 1:
		value = 30;
		break;
	}
	m_progressBrewingBubbles.setProgress( value);
	UIScene_AbstractContainerMenu::tick();
}

int UIScene_BrewingStandMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		cols = 1;
		break;
	case eSectionBrewingBottle2:
		cols = 1;
		break;
	case eSectionBrewingBottle3:
		cols = 1;
		break;
	case eSectionBrewingIngredient:
		cols = 1;
		break;
	case eSectionBrewingInventory:
		cols = 9;
		break;
	case eSectionBrewingUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_BrewingStandMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		rows = 1;
		break;
	case eSectionBrewingBottle2:
		rows = 1;
		break;
	case eSectionBrewingBottle3:
		rows = 1;
		break;
	case eSectionBrewingIngredient:
		rows = 1;
		break;
	case eSectionBrewingInventory:
		rows = 3;
		break;
	case eSectionBrewingUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_BrewingStandMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		pPosition->x = m_slotListBottles[0].getXPos();
		pPosition->y = m_slotListBottles[0].getYPos();
		break;
	case eSectionBrewingBottle2:
		pPosition->x = m_slotListBottles[1].getXPos();
		pPosition->y = m_slotListBottles[1].getYPos();
		break;
	case eSectionBrewingBottle3:
		pPosition->x = m_slotListBottles[2].getXPos();
		pPosition->y = m_slotListBottles[2].getYPos();
		break;
	case eSectionBrewingIngredient:
		pPosition->x = m_slotListIngredient.getXPos();
		pPosition->y = m_slotListIngredient.getYPos();
		break;
	case eSectionBrewingInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionBrewingUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_BrewingStandMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionBrewingBottle1:
		sectionSize.x = m_slotListBottles[0].getWidth();
		sectionSize.y = m_slotListBottles[0].getHeight();
		break;
	case eSectionBrewingBottle2:
		sectionSize.x = m_slotListBottles[1].getWidth();
		sectionSize.y = m_slotListBottles[1].getHeight();
		break;
	case eSectionBrewingBottle3:
		sectionSize.x = m_slotListBottles[2].getWidth();
		sectionSize.y = m_slotListBottles[2].getHeight();
		break;
	case eSectionBrewingIngredient:
		sectionSize.x = m_slotListIngredient.getWidth();
		sectionSize.y = m_slotListIngredient.getHeight();
		break;
	case eSectionBrewingInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionBrewingUsing:
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

void UIScene_BrewingStandMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		slotList = &m_slotListBottles[0];
		break;
	case eSectionBrewingBottle2:
		slotList = &m_slotListBottles[1];
		break;
	case eSectionBrewingBottle3:
		slotList = &m_slotListBottles[2];
		break;
	case eSectionBrewingIngredient:
		slotList = &m_slotListIngredient;
		break;
	case eSectionBrewingInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionBrewingUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_BrewingStandMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionBrewingBottle1:
		control = &m_slotListBottles[0];
		break;
	case eSectionBrewingBottle2:
		control = &m_slotListBottles[1];
		break;
	case eSectionBrewingBottle3:
		control = &m_slotListBottles[2];
		break;
	case eSectionBrewingIngredient:
		control = &m_slotListIngredient;
		break;
	case eSectionBrewingInventory:
		control = &m_slotListInventory;
		break;
	case eSectionBrewingUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}
