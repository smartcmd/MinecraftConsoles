#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\Minecraft.h"
#include "UIScene_AnvilMenu.h"

UIScene_AnvilMenu::UIScene_AnvilMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_showingCross = false;
	m_textInputAnvil.init(m_itemName,eControl_TextInput);

	m_labelAnvil.init( app.GetString(IDS_REPAIR_AND_NAME) );

	AnvilScreenInput *initData = (AnvilScreenInput *)_initData;
	m_inventory = initData->inventory;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Anvil_Menu, this);
	}

	m_repairMenu = new RepairMenu( initData->inventory, initData->level, initData->x, initData->y, initData->z, pMinecraft->localplayers[iPad] );
	m_repairMenu->addSlotListener(this);

	Initialize( iPad, m_repairMenu, true, RepairMenu::INV_SLOT_START, eSectionAnvilUsing, eSectionAnvilMax );

	m_slotListItem1.addSlots(RepairMenu::INPUT_SLOT, 1);
	m_slotListItem2.addSlots(RepairMenu::ADDITIONAL_SLOT, 1);
	m_slotListResult.addSlots(RepairMenu::RESULT_SLOT, 1);

	bool expensive = false;
	wstring m_costString = L"";

	if(m_repairMenu->cost > 0)
	{
		if(m_repairMenu->cost >= 40 && !pMinecraft->localplayers[iPad]->abilities.instabuild)
		{
			m_costString = app.GetString(IDS_REPAIR_EXPENSIVE);
			expensive = true;
		}
		else if(!m_repairMenu->getSlot(RepairMenu::RESULT_SLOT)->hasItem())
		{
			// Do nothing
		}
		else
		{
			LPCWSTR costString = app.GetString(IDS_REPAIR_COST);
			wchar_t temp[256];
			swprintf(temp, 256, costString, m_repairMenu->cost);
			m_costString = temp;
			if(!m_repairMenu->getSlot(RepairMenu::RESULT_SLOT)->mayPickup(dynamic_pointer_cast<Player>(m_inventory->player->shared_from_this())))
			{
				expensive = true;
			}
		}
	}
	setCostLabel(m_costString, expensive);

	if(initData) delete initData;

	setIgnoreInput(false);

	app.SetRichPresenceContext(iPad, CONTEXT_GAME_STATE_ANVIL);
}

wstring UIScene_AnvilMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"AnvilMenuSplit";
	}
	else
	{
		return L"AnvilMenu";
	}
}

void UIScene_AnvilMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, RepairMenu::INV_SLOT_START, eSectionAnvilUsing, eSectionAnvilMax  );

	m_slotListItem1.addSlots(RepairMenu::INPUT_SLOT, 1);
	m_slotListItem2.addSlots(RepairMenu::ADDITIONAL_SLOT, 1);
	m_slotListResult.addSlots(RepairMenu::RESULT_SLOT, 1);
}

void UIScene_AnvilMenu::tick()
{
	UIScene_AbstractContainerMenu::tick();

	handleTick();
}

int UIScene_AnvilMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionAnvilItem1:
		cols = 1;
		break;
	case eSectionAnvilItem2:
		cols = 1;
		break;
	case eSectionAnvilResult:
		cols = 1;
		break;
	case eSectionAnvilInventory:
		cols = 9;
		break;
	case eSectionAnvilUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	}
	return cols;
}

int UIScene_AnvilMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionAnvilItem1:
		rows = 1;
		break;
	case eSectionAnvilItem2:
		rows = 1;
		break;
	case eSectionAnvilResult:
		rows = 1;
		break;
	case eSectionAnvilInventory:
		rows = 3;
		break;
	case eSectionAnvilUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	}
	return rows;
}

void UIScene_AnvilMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionAnvilItem1:
		pPosition->x = m_slotListItem1.getXPos();
		pPosition->y = m_slotListItem1.getYPos();
		break;
	case eSectionAnvilItem2:
		pPosition->x = m_slotListItem2.getXPos();
		pPosition->y = m_slotListItem2.getYPos();
		break;
	case eSectionAnvilResult:
		pPosition->x = m_slotListResult.getXPos();
		pPosition->y = m_slotListResult.getYPos();
		break;
	case eSectionAnvilName:
		pPosition->x = m_textInputAnvil.getXPos();
		pPosition->y = m_textInputAnvil.getYPos();
		break;
	case eSectionAnvilInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionAnvilUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	default:
		assert( false );
		break;
	}
}

void UIScene_AnvilMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;

	switch( eSection )
	{
	case eSectionAnvilItem1:
		sectionSize.x = m_slotListItem1.getWidth();
		sectionSize.y = m_slotListItem1.getHeight();
		break;
	case eSectionAnvilItem2:
		sectionSize.x = m_slotListItem2.getWidth();
		sectionSize.y = m_slotListItem2.getHeight();
		break;
	case eSectionAnvilResult:
		sectionSize.x = m_slotListResult.getWidth();
		sectionSize.y = m_slotListResult.getHeight();
		break;
	case eSectionAnvilName:
		sectionSize.x = m_textInputAnvil.getWidth();
		sectionSize.y = m_textInputAnvil.getHeight();
		break;
	case eSectionAnvilInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionAnvilUsing:
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

void UIScene_AnvilMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = NULL;
	switch( eSection )
	{
	case eSectionAnvilItem1:
		slotList = &m_slotListItem1;
		break;
	case eSectionAnvilItem2:
		slotList = &m_slotListItem2;
		break;
	case eSectionAnvilResult:
		slotList = &m_slotListResult;
		break;
	case eSectionAnvilInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionAnvilUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_AnvilMenu::getSection(ESceneSection eSection)
{
	UIControl *control = NULL;
	switch( eSection )
	{
	case eSectionAnvilItem1:
		control = &m_slotListItem1;
		break;
	case eSectionAnvilItem2:
		control = &m_slotListItem2;
		break;
	case eSectionAnvilResult:
		control = &m_slotListResult;
		break;
	case eSectionAnvilName:
		control = &m_textInputAnvil;
		break;
	case eSectionAnvilInventory:
		control = &m_slotListInventory;
		break;
	case eSectionAnvilUsing:
		control = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	}
	return control;
}

int UIScene_AnvilMenu::KeyboardCompleteCallback(LPVOID lpParam,bool bRes)
{
	// 4J HEG - No reason to set value if keyboard was cancelled
	UIScene_AnvilMenu *pClass=(UIScene_AnvilMenu *)lpParam;
	pClass->setIgnoreInput(false);

	if (bRes)
	{
		uint16_t pchText[128];
		ZeroMemory(pchText, 128 * sizeof(uint16_t) );
		InputManager.GetText(pchText);
		pClass->setEditNameValue((wchar_t *)pchText);
		pClass->m_itemName = (wchar_t *)pchText;
		pClass->updateItemName();
	}
	return 0;
}

void UIScene_AnvilMenu::handleEditNamePressed()
{
	setIgnoreInput(true);
#if defined(__PS3__) || defined(__ORBIS__) || defined __PSVITA__
	int language = XGetLanguage();
	switch(language)
	{
	case XC_LANGUAGE_JAPANESE:
	case XC_LANGUAGE_KOREAN:
	case XC_LANGUAGE_TCHINESE:
		InputManager.RequestKeyboard(app.GetString(IDS_TITLE_RENAME),m_textInputAnvil.getLabel(),(DWORD)m_iPad,30,&UIScene_AnvilMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Default);
		break;
	default:
		// 4J Stu - Use a different keyboard for non-asian languages so we don't have prediction on
		InputManager.RequestKeyboard(app.GetString(IDS_TITLE_RENAME),m_textInputAnvil.getLabel(),(DWORD)m_iPad,30,&UIScene_AnvilMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Alphabet_Extended);
		break;
	}
#else
	InputManager.RequestKeyboard(app.GetString(IDS_TITLE_RENAME),m_textInputAnvil.getLabel(),(DWORD)m_iPad,30,&UIScene_AnvilMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Default);
#endif
}

void UIScene_AnvilMenu::setEditNameValue(const wstring &name)
{
	m_textInputAnvil.setLabel(name);
}

void UIScene_AnvilMenu::setEditNameEditable(bool enabled)
{
}

void UIScene_AnvilMenu::setCostLabel(const wstring &label, bool canAfford)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_boolean;
	value[1].boolval = canAfford;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetCostLabel , 2 , value );
}

void UIScene_AnvilMenu::showCross(bool show)
{
	if(m_showingCross != show)
	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowRedCross , 1 , value );

		m_showingCross = show;
	}
}

void UIScene_AnvilMenu::handleDestroy()
{
#ifdef __PSVITA__
	app.DebugPrintf("missing InputManager.DestroyKeyboard on Vita !!!!!!\n");
#endif

	// another player destroyed the anvil, so shut down the keyboard if it is displayed
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO)
	InputManager.DestroyKeyboard();
#endif	
	UIScene_AbstractContainerMenu::handleDestroy();
}
