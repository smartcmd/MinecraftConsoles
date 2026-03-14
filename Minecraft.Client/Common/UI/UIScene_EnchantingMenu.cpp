#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../Minecraft.h"
#include "UIScene_EnchantingMenu.h"

UIScene_EnchantingMenu::UIScene_EnchantingMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene_AbstractContainerMenu(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_enchantButton[0].init(0);
	m_enchantButton[1].init(1);
	m_enchantButton[2].init(2);

	EnchantingScreenInput *initData = static_cast<EnchantingScreenInput *>(_initData);
	
	m_labelEnchant.init( initData->name.empty() ? app.GetString(IDS_ENCHANT) : initData->name );

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[initData->iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[initData->iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Enchanting_Menu, this);
	}
	
	EnchantmentMenu *menu = new EnchantmentMenu(initData->inventory, initData->level, initData->x, initData->y, initData->z);

	Initialize( initData->iPad, menu, true, EnchantmentMenu::INV_SLOT_START, eSectionEnchantUsing, eSectionEnchantMax );

	m_slotListIngredient.addSlots(EnchantmentMenu::INGREDIENT_SLOT, 1);

	app.SetRichPresenceContext(m_iPad,CONTEXT_GAME_STATE_ENCHANTING);

	delete initData;
}

wstring UIScene_EnchantingMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"EnchantingMenuSplit";
	}
	else
	{
		return L"EnchantingMenu";
	}
}

void UIScene_EnchantingMenu::handleReload()
{
	Initialize( m_iPad, m_menu, true, EnchantmentMenu::INV_SLOT_START, eSectionEnchantUsing, eSectionEnchantMax );

	m_slotListIngredient.addSlots(EnchantmentMenu::INGREDIENT_SLOT, 1);
}

int UIScene_EnchantingMenu::getSectionColumns(ESceneSection eSection)
{
	int cols = 0;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		cols = 1;
		break;
	case eSectionEnchantInventory:
		cols = 9;
		break;
	case eSectionEnchantUsing:
		cols = 9;
		break;
	default:
		assert( false );
		break;
	};
	return cols;
}

int UIScene_EnchantingMenu::getSectionRows(ESceneSection eSection)
{
	int rows = 0;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		rows = 1;
		break;
	case eSectionEnchantInventory:
		rows = 3;
		break;
	case eSectionEnchantUsing:
		rows = 1;
		break;
	default:
		assert( false );
		break;
	};
	return rows;
}

void UIScene_EnchantingMenu::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	switch( eSection )
	{
	case eSectionEnchantSlot:
		pPosition->x = m_slotListIngredient.getXPos();
		pPosition->y = m_slotListIngredient.getYPos();
		break;
	case eSectionEnchantInventory:
		pPosition->x = m_slotListInventory.getXPos();
		pPosition->y = m_slotListInventory.getYPos();
		break;
	case eSectionEnchantUsing:
		pPosition->x = m_slotListHotbar.getXPos();
		pPosition->y = m_slotListHotbar.getYPos();
		break;
	case eSectionEnchantButton1:
		pPosition->x = m_enchantButton[0].getXPos();
		pPosition->y = m_enchantButton[0].getYPos();
		break;
	case eSectionEnchantButton2:
		pPosition->x = m_enchantButton[1].getXPos();
		pPosition->y = m_enchantButton[1].getYPos();
		break;
	case eSectionEnchantButton3:
		pPosition->x = m_enchantButton[2].getXPos();
		pPosition->y = m_enchantButton[2].getYPos();
		break;
	default:
		assert( false );
		break;
	};
}

void UIScene_EnchantingMenu::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	UIVec2D sectionSize;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		sectionSize.x = m_slotListIngredient.getWidth();
		sectionSize.y = m_slotListIngredient.getHeight();
		break;
	case eSectionEnchantInventory:
		sectionSize.x = m_slotListInventory.getWidth();
		sectionSize.y = m_slotListInventory.getHeight();
		break;
	case eSectionEnchantUsing:
		sectionSize.x = m_slotListHotbar.getWidth();
		sectionSize.y = m_slotListHotbar.getHeight();
		break;
	case eSectionEnchantButton1:
		sectionSize.x = m_enchantButton[0].getWidth();
		sectionSize.y = m_enchantButton[0].getHeight();
		break;
	case eSectionEnchantButton2:
		sectionSize.x = m_enchantButton[1].getWidth();
		sectionSize.y = m_enchantButton[1].getHeight();
		break;
	case eSectionEnchantButton3:
		sectionSize.x = m_enchantButton[2].getWidth();
		sectionSize.y = m_enchantButton[2].getHeight();
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

void UIScene_EnchantingMenu::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	int cols = getSectionColumns(eSection);

	int index = (y * cols) + x;

	UIControl_SlotList *slotList = nullptr;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		slotList = &m_slotListIngredient;
		break;
	case eSectionEnchantInventory:
		slotList = &m_slotListInventory;
		break;
	case eSectionEnchantUsing:
		slotList = &m_slotListHotbar;
		break;
	default:
		assert( false );
		break;
	};

	slotList->setHighlightSlot(index);
}

UIControl *UIScene_EnchantingMenu::getSection(ESceneSection eSection)
{
	UIControl *control = nullptr;
	switch( eSection )
	{
	case eSectionEnchantSlot:
		control = &m_slotListIngredient;
		break;
	case eSectionEnchantInventory:
		control = &m_slotListInventory;
		break;
	case eSectionEnchantUsing:
		control = &m_slotListHotbar;
		break;
	case eSectionEnchantButton1:
		control = &m_enchantButton[0];
		break;
	case eSectionEnchantButton2:
		control = &m_enchantButton[1];
		break;
	case eSectionEnchantButton3:
		control = &m_enchantButton[2];
		break;
	default:
		assert( false );
		break;
	};
	return control;
}

void UIScene_EnchantingMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == nullptr || pMinecraft->localgameModes[m_iPad] == nullptr) return;
	

	if(wcscmp((wchar_t *)region->name,L"EnchantmentBook")==0)
	{
		// Setup GDraw, normal game render states and matrices
		CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);
		delete customDrawRegion;

		m_enchantBook.render(region);

		// Finish GDraw and anything else that needs to be finalised
		ui.endCustomDraw(region);
	}
	else
	{
		int slotId = -1;
		swscanf(static_cast<wchar_t *>(region->name),L"slot_Button%d",&slotId);
		if(slotId >= 0)
		{
			// Setup GDraw, normal game render states and matrices
			CustomDrawData *customDrawRegion = ui.setupCustomDraw(this,region);
			delete customDrawRegion;

			m_enchantButton[slotId-1].render(region);

			// Finish GDraw and anything else that needs to be finalised
			ui.endCustomDraw(region);
		}
		else
		{
			UIScene_AbstractContainerMenu::customDraw(region);
		}
	}
}
