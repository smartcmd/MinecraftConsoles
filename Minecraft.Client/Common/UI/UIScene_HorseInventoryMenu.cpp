#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\Minecraft.h"
#include "UIScene_HorseInventoryMenu.h"

UIScene_HorseInventoryMenu::UIScene_HorseInventoryMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	HorseScreenInput *initData = static_cast<HorseScreenInput *>(_initData);

	m_labelHorse.init( initData->container->getName() );
	m_inventory = initData->inventory;
	m_horse = initData->horse;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Horse_Menu, this);
	}

	HorseInventoryMenu *horseMenu = new HorseInventoryMenu(initData->inventory, initData->container, initData->horse);

	int startSlot = EntityHorse::INV_BASE_COUNT;
	if(m_horse->isChestedHorse())
	{
		startSlot += EntityHorse::INV_DONKEY_CHEST_COUNT;
	}
	Initialize( iPad, horseMenu, true, startSlot, eSectionHorseUsing, eSectionHorseMax );

	m_slotSaddle.addSlots(EntityHorse::INV_SLOT_SADDLE,1);
	m_slotArmor.addSlots(EntityHorse::INV_SLOT_ARMOR,1);

	if(m_horse->isChestedHorse())
	{
		// also starts at one, because a donkey can't wear armor!
		m_slotListChest.addSlots(EntityHorse::INV_BASE_COUNT, EntityHorse::INV_DONKEY_CHEST_COUNT);
	}

	// remove horse inventory
	if(!m_horse->isChestedHorse())
		SetHasInventory(false);

	// cannot wear armor? remove armor slot!
	if(!m_horse->canWearArmor())
		SetIsDonkey(true);

	if(initData) delete initData;

	setIgnoreInput(false);

	//app.SetRichPresenceContext(iPad, CONTEXT_GAME_STATE_HORSE);
}

wstring UIScene_HorseInventoryMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"HorseInventoryMenuSplit";
	}
	else
	{
		return L"HorseInventoryMenu";
	}
}

void UIScene_HorseInventoryMenu::handleReload()
{
	int startSlot = EntityHorse::INV_BASE_COUNT;
	if(m_horse->isChestedHorse())
	{
		startSlot += EntityHorse::INV_DONKEY_CHEST_COUNT;
	}
	Initialize( m_iPad, m_menu, true, startSlot, eSectionHorseUsing, eSectionHorseMax  );

	m_slotSaddle.addSlots(EntityHorse::INV_SLOT_SADDLE,1);
	m_slotArmor.addSlots(EntityHorse::INV_SLOT_ARMOR,1);

	if(m_horse->isChestedHorse())
	{
		// also starts at one, because a donkey can't wear armor!
		m_slotListChest.addSlots(EntityHorse::INV_BASE_COUNT, EntityHorse::INV_DONKEY_CHEST_COUNT);
	}

	// remove horse inventory
	if(!m_horse->isChestedHorse())
		SetHasInventory(false);

	// cannot wear armor? remove armor slot!
	if(!m_horse->canWearArmor())
		SetIsDonkey(true);
}

int UIScene_HorseInventoryMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionHorseArmor:
		cols = 1;
		break;
	case eSectionHorseSaddle:
		cols = 1;
		break;
	case eSectionHorseChest:
		cols = 5;
		break;
	case eSectionHorseInventory:
		cols = 9;
		break;
	case eSectionHorseUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_HorseInventoryMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionHorseArmor:
		rows = 1;
		break;
	case eSectionHorseSaddle:
		rows = 1;
		break;
	case eSectionHorseChest:
		rows = 3;
		break;
	case eSectionHorseInventory:
		rows = 3;
		break;
	case eSectionHorseUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_HorseInventoryMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionHorseArmor:
		pPosition->x = m_slotArmor.getXPos();
		pPosition->y = m_slotArmor.getYPos();
		break;
	case eSectionHorseSaddle:
		pPosition->x = m_slotSaddle.getXPos();
		pPosition->y = m_slotSaddle.getYPos();
		break;
	case eSectionHorseChest:
		pPosition->x = m_slotListChest.getXPos();
		pPosition->y = m_slotListChest.getYPos();
		break;
	case eSectionHorseInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionHorseUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_HorseInventoryMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionHorseArmor:
		sectionSize.x = m_slotArmor.getWidth();
		sectionSize.y = m_slotArmor.getHeight();
		break;
	case eSectionHorseSaddle:
		sectionSize.x = m_slotSaddle.getWidth();
		sectionSize.y = m_slotSaddle.getHeight();
		break;
	case eSectionHorseChest:
		sectionSize.x = m_slotListChest.getWidth();
		sectionSize.y = m_slotListChest.getHeight();
		break;
	case eSectionHorseInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionHorseUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
	default:
		assert( false );
		break;
	}

	if(IsSectionSlotList(eSection))
	{
		int rows = getSectionRows(eSection);
		int cols = getSectionColumns(eSection);

		pSize->x = sectionSize.x/cols;
		pSize->y = sectionSize.y/rows;

		int itemCol = iItemIndex % cols;
		int itemRow = iItemIndex/cols;

		pPosition->x = itemCol * pSize->x;
		pPosition->y = itemRow * pSize->y;
	}
	else
	{
		GetPositionOfSection(eSection, pPosition);
		pSize->x = sectionSize.x;
		pSize->y = sectionSize.y;
	}
}

void UIScene_HorseInventoryMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionHorseArmor:
		slotList = &m_slotArmor;
		break;
	case eSectionHorseSaddle:
		slotList = &m_slotSaddle;
		break;
	case eSectionHorseChest:
		slotList = &m_slotListChest;
		break;
	case eSectionHorseInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionHorseUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_HorseInventoryMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionHorseArmor:
		control = &m_slotArmor;
		break;
	case eSectionHorseSaddle:
		control = &m_slotSaddle;
		break;
	case eSectionHorseChest:
		control = &m_slotListChest;
		break;
	case eSectionHorseInventory:
		control = &m_slotListInventory;
		break;
	case eSectionHorseUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}

void UIScene_HorseInventoryMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return;

	if(wcscmp((wchar_t *)region->name,L"horse")==0)
	{
		// Setup GDraw, normal game render states and matrices
		CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);
		delete customDrawRegion;

		m_horsePreview.render(region);

		// Finish GDraw and anything else that needs to be finalised
		ui.endCustomDraw(region);
	}
	else
	{
		UIScene_AbstractContainerMenu::customDraw(region);
	}
}

void UIScene_HorseInventoryMenu::SetHasInventory(bool bHasInventory)
{
	app.DebugPrintf("SetHasInventory to %d\n", bHasInventory);

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = bHasInventory;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetHasInventory , 1 , value );
}

void UIScene_HorseInventoryMenu::SetIsDonkey(bool bSetIsDonkey)
{
	app.DebugPrintf("SetIsDonkey to %d\n", bSetIsDonkey);

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = bSetIsDonkey;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetIsDonkey , 1 , value );
}