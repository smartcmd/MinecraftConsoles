#include "stdafx.h"
#include "UI.h"
#include "UIScene_CreativeMenu.h"

#include "..\Minecraft.World\JavaMath.h"
#include "..\..\LocalPlayer.h"
#include "..\Tutorial\Tutorial.h"
#include "..\Tutorial\TutorialMode.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"

#ifdef __PSVITA__
#define GAME_CREATIVE_TOUCHUPDATE_TIMER_ID 0
#define GAME_CREATIVE_TOUCHUPDATE_TIMER_TIME 100
#endif

UIScene_CreativeMenu::UIScene_CreativeMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	InventoryScreenInput *initData = (InventoryScreenInput *)_initData;

	shared_ptr<SimpleContainer> creativeContainer = shared_ptr<SimpleContainer>(new SimpleContainer( 0, L"", false, TabSpec::MAX_SIZE ));
	itemPickerMenu = new ItemPickerMenu(creativeContainer, initData->player->inventory);

	Initialize( initData->iPad, itemPickerMenu, false, -1, eSectionInventoryCreativeUsing, eSectionInventoryCreativeMax, initData->bNavigateBack);

	m_labelInventory.setLabel( L"" );
	m_bFirstCall=true;

	//m_slotListContainer.addSlots(0,TabSpec::MAX_SIZE);
	//m_slotListHotbar.addSlots(TabSpec::MAX_SIZE,TabSpec::MAX_SIZE + 9);
	for(unsigned int i = 0; i < TabSpec::MAX_SIZE; ++i)
	{
		m_slotListContainer.addSlot(i);
	}

	for(unsigned int i = TabSpec::MAX_SIZE; i < TabSpec::MAX_SIZE + 9; ++i)
	{
		m_slotListHotbar.addSlot(i);
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[initData->iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Creative_Inventory_Menu, this);
	}

	if(initData) delete initData;

	m_curTab = eCreativeInventoryTab_COUNT;
	switchTab(eCreativeInventoryTab_BuildingBlocks);

#ifdef __PSVITA__
	// initialise vita touch controls with ids
	for(unsigned int i = 0; i < ETouchInput_Count; ++i)
	{
		m_TouchInput[i].init(i);
	}
#endif
}

wstring UIScene_CreativeMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"CreativeMenuSplit";
	}
	else
	{
		return L"CreativeMenu";
	}
}

#ifdef __PSVITA__
UIControl* UIScene_CreativeMenu::GetMainPanel()
{
	return &m_controlMainPanel;
}

void UIScene_CreativeMenu::handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased)
{
	// perform action on release
	if(bReleased)
	{
		if(iId >= eCreativeInventoryTab_BuildingBlocks && iId <= eCreativeInventoryTab_Misc)
		{
			switchTab((ECreativeInventoryTabs)iId);
			ui.PlayUISFX(eSFX_Focus);
		}
	}
	if(bRepeat && iId == ETouchInput_TouchSlider && specs[m_curTab]->getPageCount() > 1)
	{
		// calculate relative touch position on slider
		float fPosition = ((float)y - (float)m_TouchInput[ETouchInput_TouchSlider].getYPos() - m_controlMainPanel.getYPos()) /  (float)m_TouchInput[ETouchInput_TouchSlider].getHeight();
		
		// clamp
		if(fPosition > 1)
			fPosition = 1.0f;
		else if(fPosition < 0)
			fPosition = 0.0f;

		// calculate page position according to page count
		int iCurrentPage = Math::round(fPosition * (specs[m_curTab]->getPageCount() - 1));
		
		// set tab page
		m_tabPage[m_curTab] = iCurrentPage;

		// update tab
		switchTab(m_curTab);
	}
}

void UIScene_CreativeMenu::handleTouchBoxRebuild()
{
	addTimer(GAME_CREATIVE_TOUCHUPDATE_TIMER_ID,GAME_CREATIVE_TOUCHUPDATE_TIMER_TIME);
}

void UIScene_CreativeMenu::handleTimerComplete(int id)
{
	if(id == GAME_CREATIVE_TOUCHUPDATE_TIMER_ID)
	{
		// we cannot rebuild touch boxes in an iggy callback because it requires further iggy calls
		GetMainPanel()->UpdateControl();
		ui.TouchBoxRebuild(this);
		killTimer(GAME_CREATIVE_TOUCHUPDATE_TIMER_ID);
	}
}
#endif

void UIScene_CreativeMenu::handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey)
{
	switch(eSection)
	{
	case eSectionInventoryCreativeTab_0:
	case eSectionInventoryCreativeTab_1:
	case eSectionInventoryCreativeTab_2:
	case eSectionInventoryCreativeTab_3:
	case eSectionInventoryCreativeTab_4:
	case eSectionInventoryCreativeTab_5:
	case eSectionInventoryCreativeTab_6:
	case eSectionInventoryCreativeTab_7:
		{
			ECreativeInventoryTabs tab = (ECreativeInventoryTabs)((int)eCreativeInventoryTab_BuildingBlocks + (int)eSection - (int)eSectionInventoryCreativeTab_0);
			if(tab != m_curTab)
			{
				switchTab(tab);
				ui.PlayUISFX(eSFX_Focus);
			}
		}
		break;
	case eSectionInventoryCreativeSlider:
		ScrollBar(this->m_pointerPos);
		break;
	}
}

void UIScene_CreativeMenu::handleReload()
{
	Initialize( m_iPad, m_menu, false, -1, eSectionInventoryCreativeUsing, eSectionInventoryCreativeMax, m_bNavigateBack );

	for(unsigned int i = 0; i < TabSpec::MAX_SIZE; ++i)
	{
		m_slotListContainer.addSlot(i);
	}

	for(unsigned int i = TabSpec::MAX_SIZE; i < TabSpec::MAX_SIZE + 9; ++i)
	{
		m_slotListHotbar.addSlot(i);
	}
	
	ECreativeInventoryTabs lastTab = m_curTab;
	m_curTab = eCreativeInventoryTab_COUNT;
	switchTab(lastTab);
}

void UIScene_CreativeMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	// 4J-PB - going to ignore repeats on this scene
	if(repeat) return;

	//app.DebugPrintf("UIScene_CreativeMenu handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	int dir = 1;
	switch(key)
	{
	case VK_PAD_LSHOULDER:
		dir = -1;
		// Fall through intentional
	case VK_PAD_RSHOULDER:
		{		
			ECreativeInventoryTabs tab = (ECreativeInventoryTabs)(m_curTab + dir);
			if (tab < 0) tab = (ECreativeInventoryTabs)(eCreativeInventoryTab_COUNT - 1);
			if (tab >= eCreativeInventoryTab_COUNT) tab = eCreativeInventoryTab_BuildingBlocks;
			switchTab(tab);
			ui.PlayUISFX(eSFX_Focus);
		}
		break;
	case VK_PAD_LTRIGGER:
		// change the potion strength
		{
			++m_tabDynamicPos[m_curTab];
			if(m_tabDynamicPos[m_curTab] >= specs[m_curTab]->m_dynamicGroupsCount) m_tabDynamicPos[m_curTab] = 0;
			switchTab(m_curTab);
		}
		break;
	default:
		UIScene_AbstractContainerMenu::handleInput(iPad,key,repeat,pressed,released,handled);
		break;
	}
}

void UIScene_CreativeMenu::updateTabHighlightAndText(ECreativeInventoryTabs tab)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (F64)tab;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcSetActiveTab , 1 , value );

	m_labelInventory.setLabel(app.GetString(specs[tab]->m_descriptionId));
}

int UIScene_CreativeMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		cols = 10;
		break;
	case eSectionInventoryCreativeUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_CreativeMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		rows = 5;
		break;
	case eSectionInventoryCreativeUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_CreativeMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		pPosition->x = m_slotListContainer.getXPos();
		pPosition->y = m_slotListContainer.getYPos();
		break;
	case eSectionInventoryCreativeUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	case eSectionInventoryCreativeTab_0:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_0].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_0].getYPos();
		break;
	case eSectionInventoryCreativeTab_1:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_1].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_1].getYPos();
		break;
	case eSectionInventoryCreativeTab_2:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_2].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_2].getYPos();
		break;
	case eSectionInventoryCreativeTab_3:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_3].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_3].getYPos();
		break;
	case eSectionInventoryCreativeTab_4:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_4].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_4].getYPos();
		break;
	case eSectionInventoryCreativeTab_5:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_5].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_5].getYPos();
		break;
	case eSectionInventoryCreativeTab_6:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_6].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_6].getYPos();
		break;
	case eSectionInventoryCreativeTab_7:
		pPosition->x = m_TouchInput[ETouchInput_TouchPanel_7].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchPanel_7].getYPos();
		break;
	case eSectionInventoryCreativeSlider:
		pPosition->x = m_TouchInput[ETouchInput_TouchSlider].getXPos();
		pPosition->y = m_TouchInput[ETouchInput_TouchSlider].getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_CreativeMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		sectionSize.x = m_slotListContainer.getWidth();
		sectionSize.y = m_slotListContainer.getHeight();
		break;
	case eSectionInventoryCreativeUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
	case eSectionInventoryCreativeTab_0:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_0].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_0].getHeight();
		break;
	case eSectionInventoryCreativeTab_1:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_1].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_1].getHeight();
		break;
	case eSectionInventoryCreativeTab_2:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_2].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_2].getHeight();
		break;
	case eSectionInventoryCreativeTab_3:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_3].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_3].getHeight();
		break;
	case eSectionInventoryCreativeTab_4:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_4].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_4].getHeight();
		break;
	case eSectionInventoryCreativeTab_5:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_5].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_5].getHeight();
		break;
	case eSectionInventoryCreativeTab_6:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_6].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_6].getHeight();
		break;
	case eSectionInventoryCreativeTab_7:
		sectionSize.x = m_TouchInput[ETouchInput_TouchPanel_7].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchPanel_7].getHeight();
		break;
	case eSectionInventoryCreativeSlider:
		sectionSize.x = m_TouchInput[ETouchInput_TouchSlider].getWidth();
		sectionSize.y = m_TouchInput[ETouchInput_TouchSlider].getHeight();
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

void UIScene_CreativeMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = NULL;
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		slotList = &m_slotListContainer;
		break;
	case eSectionInventoryCreativeUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_CreativeMenu::getSection(ESceneSection eSection)
{
	UIControl *control = NULL;
	switch( eSection )
	{
	case eSectionInventoryCreativeSelector:
		control = &m_slotListContainer;
		break;
	case eSectionInventoryCreativeUsing:
		control = &m_slotListHotbar;
		break;
	case eSectionInventoryCreativeTab_0:
		control = &m_TouchInput[ETouchInput_TouchPanel_0];
		break;
	case eSectionInventoryCreativeTab_1:
		control = &m_TouchInput[ETouchInput_TouchPanel_1];
		break;
	case eSectionInventoryCreativeTab_2:
		control = &m_TouchInput[ETouchInput_TouchPanel_2];
		break;
	case eSectionInventoryCreativeTab_3:
		control = &m_TouchInput[ETouchInput_TouchPanel_3];
		break;
	case eSectionInventoryCreativeTab_4:
		control = &m_TouchInput[ETouchInput_TouchPanel_4];
		break;
	case eSectionInventoryCreativeTab_5:
		control = &m_TouchInput[ETouchInput_TouchPanel_5];
		break;
	case eSectionInventoryCreativeTab_6:
		control = &m_TouchInput[ETouchInput_TouchPanel_6];
		break;
	case eSectionInventoryCreativeTab_7:
		control = &m_TouchInput[ETouchInput_TouchPanel_7];
		break;
	case eSectionInventoryCreativeSlider:
		control = &m_TouchInput[ETouchInput_TouchSlider];
		break;
	default:
		assert( false );
		break;
	}
	return control;
}

void UIScene_CreativeMenu::updateScrollCurrentPage(int currentPage, int pageCount)
{
	IggyDataValue result;
	IggyDataValue value[2];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = (F64)pageCount;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (F64)currentPage - 1;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ) , m_funcSetScrollBar , 2 , value );
}