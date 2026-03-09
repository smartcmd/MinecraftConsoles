#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\Minecraft.h"
#include "UIScene_BeaconMenu.h"

UIScene_BeaconMenu::UIScene_BeaconMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_labelPrimary.init(IDS_CONTAINER_BEACON_PRIMARY_POWER);
	m_labelSecondary.init(IDS_CONTAINER_BEACON_SECONDARY_POWER);

	m_buttonsPowers[eControl_Primary1].setVisible(false);
	m_buttonsPowers[eControl_Primary2].setVisible(false);
	m_buttonsPowers[eControl_Primary3].setVisible(false);
	m_buttonsPowers[eControl_Primary4].setVisible(false);
	m_buttonsPowers[eControl_Primary5].setVisible(false);
	m_buttonsPowers[eControl_Secondary1].setVisible(false);
	m_buttonsPowers[eControl_Secondary2].setVisible(false);

	BeaconScreenInput *initData = static_cast<BeaconScreenInput *>(_initData);

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[initData->iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Beacon_Menu, this);
	}

	m_beacon = initData->beacon;
	
	BeaconMenu *menu = new BeaconMenu(initData->inventory, initData->beacon);

	Initialize( initData->iPad, menu, true, BeaconMenu::INV_SLOT_START, eSectionBeaconUsing, eSectionBeaconMax );

	m_slotListActivator.addSlots(BeaconMenu::PAYMENT_SLOT, 1);

	m_slotListActivatorIcons.addSlots(m_menu->getSize(),4);

	//app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_BEACON);

	delete initData;
}

wstring UIScene_BeaconMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"BeaconMenuSplit";
	}
	else
	{
		return L"BeaconMenu";
	}
}

void UIScene_BeaconMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, BeaconMenu::INV_SLOT_START, eSectionBeaconUsing, eSectionBeaconMax );

	m_slotListActivator.addSlots(BeaconMenu::PAYMENT_SLOT, 1);

	m_slotListActivatorIcons.addSlots(m_menu->getSize(),4);
}

void UIScene_BeaconMenu::tick()
{
	UIScene_AbstractContainerMenu::tick();

	handleTick();
}

int UIScene_BeaconMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionBeaconItem:
		cols = 1;
		break;
	case eSectionBeaconInventory:
		cols = 9;
		break;
	case eSectionBeaconUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	};
	return cols;
}

int UIScene_BeaconMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionBeaconItem:
		rows = 1;
		break;
	case eSectionBeaconInventory:
		rows = 3;
		break;
	case eSectionBeaconUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	};
	return rows;
}

void UIScene_BeaconMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionBeaconItem:
		pPosition->x = m_slotListActivator.getXPos();
		pPosition->y = m_slotListActivator.getYPos();
		break;
	case eSectionBeaconInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionBeaconUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;

	case eSectionBeaconPrimaryTierOneOne:
		pPosition->x = m_buttonsPowers[eControl_Primary1].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Primary1].getYPos();
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		pPosition->x = m_buttonsPowers[eControl_Primary2].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Primary2].getYPos();
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		pPosition->x = m_buttonsPowers[eControl_Primary3].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Primary3].getYPos();
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		pPosition->x = m_buttonsPowers[eControl_Primary4].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Primary4].getYPos();
		break;
	case eSectionBeaconPrimaryTierThree:
		pPosition->x = m_buttonsPowers[eControl_Primary5].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Primary5].getYPos();
		break;
	case eSectionBeaconSecondaryOne:
		pPosition->x = m_buttonsPowers[eControl_Secondary1].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Secondary1].getYPos();
		break;
	case eSectionBeaconSecondaryTwo:
		pPosition->x = m_buttonsPowers[eControl_Secondary2].getXPos();
		pPosition->y = m_buttonsPowers[eControl_Secondary2].getYPos();
		break;
	case eSectionBeaconConfirm:
		pPosition->x = m_buttonConfirm.getXPos();
		pPosition->y = m_buttonConfirm.getYPos();
		break;
	default:
		assert( false );
		break;
	};
}

void UIScene_BeaconMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionBeaconItem:
		sectionSize.x = m_slotListActivator.getWidth();
		sectionSize.y = m_slotListActivator.getHeight();
		break;
	case eSectionBeaconInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionBeaconUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
		
	case eSectionBeaconPrimaryTierOneOne:
		sectionSize.x = m_buttonsPowers[eControl_Primary1].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Primary1].getHeight();
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		sectionSize.x = m_buttonsPowers[eControl_Primary2].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Primary2].getHeight();
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		sectionSize.x = m_buttonsPowers[eControl_Primary3].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Primary3].getHeight();
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		sectionSize.x = m_buttonsPowers[eControl_Primary4].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Primary4].getHeight();
		break;
	case eSectionBeaconPrimaryTierThree:
		sectionSize.x = m_buttonsPowers[eControl_Primary5].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Primary5].getHeight();
		break;
	case eSectionBeaconSecondaryOne:
		sectionSize.x = m_buttonsPowers[eControl_Secondary1].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Secondary1].getHeight();
		break;
	case eSectionBeaconSecondaryTwo:
		sectionSize.x = m_buttonsPowers[eControl_Secondary2].getWidth();
		sectionSize.y = m_buttonsPowers[eControl_Secondary2].getHeight();
		break;
	case eSectionBeaconConfirm:
		sectionSize.x = m_buttonConfirm.getWidth();
		sectionSize.y = m_buttonConfirm.getHeight();
		break;
	default:
		assert( false );
		break;
	};

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

void UIScene_BeaconMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionBeaconItem:
		slotList = &m_slotListActivator;
		break;
	case eSectionBeaconInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionBeaconUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	};

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_BeaconMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionBeaconItem:
		control = &m_slotListActivator;
		break;
	case eSectionBeaconInventory:
		control = &m_slotListInventory;
		break;
	case eSectionBeaconUsing:
		control = &m_slotListHotbar;
		break;
		
	case eSectionBeaconPrimaryTierOneOne:
		control = &m_buttonsPowers[eControl_Primary1];
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		control = &m_buttonsPowers[eControl_Primary2];
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		control = &m_buttonsPowers[eControl_Primary3];
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		control = &m_buttonsPowers[eControl_Primary4];
		break;
	case eSectionBeaconPrimaryTierThree:
		control = &m_buttonsPowers[eControl_Primary5];
		break;
	case eSectionBeaconSecondaryOne:
		control = &m_buttonsPowers[eControl_Secondary1];
		break;
	case eSectionBeaconSecondaryTwo:
		control = &m_buttonsPowers[eControl_Secondary2];
		break;
	case eSectionBeaconConfirm:
		control = &m_buttonConfirm;
		break;

	default:
		assert( false );
		break;
	};
	return control;
}

void UIScene_BeaconMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return;

	shared_ptr<ItemInstance> item = nullptr;
	int slotId = -1;
	swscanf(static_cast<wchar_t *>(region->name),L"slot_%d",&slotId);

	if(slotId >= 0 && slotId >= m_menu->getSize() )
	{			
		int icon = slotId - m_menu->getSize();
		switch(icon)
		{
		case 0:
			item = std::make_shared<ItemInstance>(Item::emerald);
			break;
		case 1:
			item = std::make_shared<ItemInstance>(Item::diamond);
			break;
		case 2:
			item = std::make_shared<ItemInstance>(Item::goldIngot);
			break;
		case 3:
			item = std::make_shared<ItemInstance>(Item::ironIngot);
			break;
		default:
			assert(false);
			break;
		};		
		if(item != nullptr) customDrawSlotControl(region,m_iPad,item,1.0f,item->isFoil(),true);
	}
	else
	{
		UIScene_AbstractContainerMenu::customDraw(region);
	}
}

void UIScene_BeaconMenu::SetConfirmButtonEnabled(bool enabled)
{
	m_buttonConfirm.SetButtonActive(enabled);
}

void UIScene_BeaconMenu::AddPowerButton(int id, int icon, int tier, int count, bool active, bool selected)
{
	switch(tier)
	{
	case 0:
		if(count == 0)
		{
			m_buttonsPowers[eControl_Primary1].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Primary1].setVisible(true);
		}
		else
		{
			m_buttonsPowers[eControl_Primary2].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Primary2].setVisible(true);
		}
		break;
	case 1:
		if(count == 0)
		{
			m_buttonsPowers[eControl_Primary3].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Primary3].setVisible(true);
		}
		else
		{
			m_buttonsPowers[eControl_Primary4].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Primary4].setVisible(true);
		}
		break;
	case 2:
		m_buttonsPowers[eControl_Primary5].SetData(id, icon,active,selected);
		m_buttonsPowers[eControl_Primary5].setVisible(true);
		break;
	case 3:
		if(count == 0)
		{
			m_buttonsPowers[eControl_Secondary1].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Secondary1].setVisible(true);
		}
		else
		{
			m_buttonsPowers[eControl_Secondary2].SetData(id, icon,active,selected);
			m_buttonsPowers[eControl_Secondary2].setVisible(true);
		}
		break;
	};
}

int UIScene_BeaconMenu::GetPowerButtonId(ESceneSection eSection)
{
	switch(eSection)
	{
	case eSectionBeaconPrimaryTierOneOne:
		return m_buttonsPowers[eControl_Primary1].GetData();
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		return m_buttonsPowers[eControl_Primary2].GetData();
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		return m_buttonsPowers[eControl_Primary3].GetData();
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		return m_buttonsPowers[eControl_Primary4].GetData();
		break;
	case eSectionBeaconPrimaryTierThree:
		return m_buttonsPowers[eControl_Primary5].GetData();
		break;
	case eSectionBeaconSecondaryOne:
		return m_buttonsPowers[eControl_Secondary1].GetData();
		break;
	case eSectionBeaconSecondaryTwo:
		return m_buttonsPowers[eControl_Secondary2].GetData();
		break;
	};
	return 0;
}

bool UIScene_BeaconMenu::IsPowerButtonSelected(ESceneSection eSection)
{
	switch(eSection)
	{
	case eSectionBeaconPrimaryTierOneOne:
		return m_buttonsPowers[eControl_Primary1].IsButtonSelected();
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		return m_buttonsPowers[eControl_Primary2].IsButtonSelected();
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		return m_buttonsPowers[eControl_Primary3].IsButtonSelected();
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		return m_buttonsPowers[eControl_Primary4].IsButtonSelected();
		break;
	case eSectionBeaconPrimaryTierThree:
		return m_buttonsPowers[eControl_Primary5].IsButtonSelected();
		break;
	case eSectionBeaconSecondaryOne:
		return m_buttonsPowers[eControl_Secondary1].IsButtonSelected();
		break;
	case eSectionBeaconSecondaryTwo:
		return m_buttonsPowers[eControl_Secondary2].IsButtonSelected();
		break;
	};
	return false;
}

void UIScene_BeaconMenu::SetPowerButtonSelected(ESceneSection eSection)
{
	switch(eSection)
	{
	case eSectionBeaconPrimaryTierOneOne:
	case eSectionBeaconPrimaryTierOneTwo:
	case eSectionBeaconPrimaryTierTwoOne:
	case eSectionBeaconPrimaryTierTwoTwo:
	case eSectionBeaconPrimaryTierThree:
		m_buttonsPowers[eControl_Primary1].SetButtonSelected(false);
		m_buttonsPowers[eControl_Primary2].SetButtonSelected(false);
		m_buttonsPowers[eControl_Primary3].SetButtonSelected(false);
		m_buttonsPowers[eControl_Primary4].SetButtonSelected(false);
		m_buttonsPowers[eControl_Primary5].SetButtonSelected(false);
		break;
	case eSectionBeaconSecondaryOne:
	case eSectionBeaconSecondaryTwo:
		m_buttonsPowers[eControl_Secondary1].SetButtonSelected(false);
		m_buttonsPowers[eControl_Secondary2].SetButtonSelected(false);
		break;
	};


	switch(eSection)
	{
	case eSectionBeaconPrimaryTierOneOne:
		return m_buttonsPowers[eControl_Primary1].SetButtonSelected(true);
		break;
	case eSectionBeaconPrimaryTierOneTwo:
		return m_buttonsPowers[eControl_Primary2].SetButtonSelected(true);
		break;
	case eSectionBeaconPrimaryTierTwoOne:
		return m_buttonsPowers[eControl_Primary3].SetButtonSelected(true);
		break;
	case eSectionBeaconPrimaryTierTwoTwo:
		return m_buttonsPowers[eControl_Primary4].SetButtonSelected(true);
		break;
	case eSectionBeaconPrimaryTierThree:
		return m_buttonsPowers[eControl_Primary5].SetButtonSelected(true);
		break;
	case eSectionBeaconSecondaryOne:
		return m_buttonsPowers[eControl_Secondary1].SetButtonSelected(true);
		break;
	case eSectionBeaconSecondaryTwo:
		return m_buttonsPowers[eControl_Secondary2].SetButtonSelected(true);
		break;
	};
}