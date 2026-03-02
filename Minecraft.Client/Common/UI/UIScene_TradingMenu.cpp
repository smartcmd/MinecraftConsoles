#include "stdafx.h"
#include "UI.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.trading.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\Minecraft.h"
#include "UIScene_TradingMenu.h"

UIScene_TradingMenu::UIScene_TradingMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	
	m_showingLeftArrow = true;
	m_showingRightArrow = true;

	// 4J-PB - "Villager" appears for a short time on opening the trading menu
	//m_labelTrading.init( app.GetString(IDS_VILLAGER) );
	m_labelTrading.init( L"" );
	m_labelInventory.init( app.GetString(IDS_INVENTORY) );
	m_labelRequired.init( app.GetString(IDS_REQUIRED_ITEMS_FOR_TRADE) );

	m_labelRequest1.init(L"");
	m_labelRequest2.init(L"");

	TradingScreenInput *initData = (TradingScreenInput *)_initData;
	m_merchant = initData->trader;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[iPad];
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Trading_Menu, this);
	}

	m_menu = new MerchantMenu( initData->inventory, initData->trader, initData->level );
	
	Minecraft::GetInstance()->localplayers[iPad]->containerMenu = m_menu;

	m_slotListRequest1.addSlots(BUY_A,1);
	m_slotListRequest2.addSlots(BUY_B,1);

	m_slotListTrades.addSlots(TRADES_START,DISPLAY_TRADES_COUNT);

	m_slotListInventory.addSlots(MerchantMenu::INV_SLOT_START, 27);
	m_slotListHotbar.addSlots(MerchantMenu::USE_ROW_SLOT_START, 9);

	if(initData) delete initData;

	// in this scene, we override the press sound with our own for crafting success or fail
	ui.OverrideSFX(m_iPad,ACTION_MENU_A,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,true);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,true);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,true);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,true);

	app.SetRichPresenceContext(iPad, CONTEXT_GAME_STATE_TRADING);
}

wstring UIScene_TradingMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"TradingMenuSplit";
	}
	else
	{
		return L"TradingMenu";
	}
}

void UIScene_TradingMenu::updateTooltips()
{
	ui.SetTooltips(m_iPad, IDS_TOOLTIPS_TRADE, IDS_TOOLTIPS_BACK);
}

void UIScene_TradingMenu::handleDestroy()
{
	app.DebugPrintf("UIScene_TradingMenu::handleDestroy\n");
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if( pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(gameMode != NULL) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(pMinecraft->localplayers[m_iPad] != NULL) pMinecraft->localplayers[m_iPad]->closeContainer();

	ui.OverrideSFX(m_iPad,ACTION_MENU_A,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_OK,false);
#ifdef __ORBIS__
	ui.OverrideSFX(m_iPad,ACTION_MENU_TOUCHPAD_PRESS,false);
#endif
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT_SCROLL,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_LEFT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_RIGHT,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_UP,false);
	ui.OverrideSFX(m_iPad,ACTION_MENU_DOWN,false);
}

void UIScene_TradingMenu::handleReload()
{
	m_slotListRequest1.addSlots(BUY_A,1);
	m_slotListRequest2.addSlots(BUY_B,1);

	m_slotListTrades.addSlots(TRADES_START,DISPLAY_TRADES_COUNT);

	m_slotListInventory.addSlots(MerchantMenu::INV_SLOT_START, 27);
	m_slotListHotbar.addSlots(MerchantMenu::USE_ROW_SLOT_START, 9);
}

void UIScene_TradingMenu::tick()
{
	UIScene::tick();
	handleTick();
}

void UIScene_TradingMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_InventoryMenu handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	default:
		if(pressed)
		{
			handled = handleKeyDown(m_iPad, key, repeat);
		}
		break;
	};
}

void UIScene_TradingMenu::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL) return;

	shared_ptr<ItemInstance> item = nullptr;
	int slotId = -1;
	swscanf((wchar_t*)region->name,L"slot_%d",&slotId);

	if(slotId < MerchantMenu::USE_ROW_SLOT_END)
	{			
		Slot *slot = m_menu->getSlot(slotId);
		item = slot->getItem();
	}
	else if(slotId >= TRADES_START)
	{
		int tradeId = (slotId - TRADES_START) + m_offersStartIndex;
		if(tradeId < m_activeOffers.size())
		{
			item = m_activeOffers.at(tradeId).first->getSellItem();
		}
	}
	else
	{
		int tradeId = m_selectedSlot + m_offersStartIndex;
		if( tradeId < m_activeOffers.size() )
		{
			switch(slotId)
			{
			case BUY_A:
				item = m_activeOffers.at(tradeId).first->getBuyAItem();
				break;
			case BUY_B:
				item = m_activeOffers.at(tradeId).first->getBuyBItem();
				break;
			};
		}
	}
	if(item != NULL) customDrawSlotControl(region,m_iPad,item,1.0f,item->isFoil(),true);
}

void UIScene_TradingMenu::showScrollRightArrow(bool show)
{
	if(m_showingRightArrow != show)
	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowScrollRightArrow , 1 , value );

		m_showingRightArrow = show;
	}
}

void UIScene_TradingMenu::showScrollLeftArrow(bool show)
{
	if(m_showingLeftArrow != show)
	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = show;
		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowScrollLeftArrow , 1 , value );

		m_showingLeftArrow = show;
	}
}

void UIScene_TradingMenu::moveSelector(bool right)
{
	IggyDataValue result;
	IggyDataValue value[1];

	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = right;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcMoveSelector , 1 , value );
}

void UIScene_TradingMenu::setTitle(const wstring &name)
{
	m_labelTrading.setLabel(name);
}

void UIScene_TradingMenu::setRequest1Name(const wstring &name)
{
	m_labelRequest1.setLabel(name);
}

void UIScene_TradingMenu::setRequest2Name(const wstring &name)
{
	m_labelRequest2.setLabel(name);
}

void UIScene_TradingMenu::setRequest1RedBox(bool show)
{
	m_slotListRequest1.showSlotRedBox(0,show);
}

void UIScene_TradingMenu::setRequest2RedBox(bool show)
{
	m_slotListRequest2.showSlotRedBox(0,show);
}

void UIScene_TradingMenu::setTradeRedBox(int index, bool show)
{
	m_slotListTrades.showSlotRedBox(index,show);
}

void UIScene_TradingMenu::setOfferDescription(const wstring &name, vector<wstring> &unformattedStrings)
{
	IggyDataValue result;
	IggyDataValue value[1];

	IggyStringUTF16 stringVal;
	stringVal.string = (IggyUTF16*)name.c_str();
	stringVal.length = name.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal;

	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetOfferDescription , 1 , value );
}
