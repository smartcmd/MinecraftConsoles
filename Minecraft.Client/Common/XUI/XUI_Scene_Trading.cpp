#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.item.trading.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_Trading.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/JavaMath.h"

//--------------------------------------------------------------------------------------
// Name: CXuiSceneTrading::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneTrading::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();

	//XuiControlSetText(m_villagerText,app.GetString(IDS_VILLAGER));
	XuiControlSetText(m_inventoryLabel,app.GetString(IDS_INVENTORY));
	XuiControlSetText(m_requiredLabel,app.GetString(IDS_REQUIRED_ITEMS_FOR_TRADE));


	Minecraft *pMinecraft = Minecraft::GetInstance();

	TradingScreenInput* initData = static_cast<TradingScreenInput *>(pInitData->pvInitData);
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;
	m_merchant = initData->trader;

	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Trading_Menu, this);
	}

	m_menu = new MerchantMenu( initData->inventory, initData->trader, initData->level );
	Minecraft::GetInstance()->localplayers[m_iPad]->containerMenu = m_menu;

	// TODO Inventory dimensions need defined as constants
	m_inventoryControl->SetData( m_iPad, m_menu, 3, 9, MerchantMenu::INV_SLOT_START, MerchantMenu::INV_SLOT_END );

	// TODO Inventory dimensions need defined as constants
	m_useRowControl->SetData( m_iPad, m_menu, 1, 9, MerchantMenu::USE_ROW_SLOT_START, MerchantMenu::USE_ROW_SLOT_END );

	delete initData;

	D3DXVECTOR3 vec;
	// store the slot 0 position
	m_tradeHSlots[0]->GetPosition(&m_vTradeSlot0Pos);
	m_tradeHSlots[1]->GetPosition(&vec);
	m_fSlotSize=vec.x-m_vTradeSlot0Pos.x;

	// store the slot 0 highlight position
	m_tradingSelector.GetPosition(&m_vSelectorInitialPos);

	//app.SetRichPresenceContextValue(m_iPad,CONTEXT_GAME_STATE_FORGING);

	XuiSetTimer(m_hObj,TRADING_UPDATE_TIMER_ID,TRADING_UPDATE_TIMER_TIME);

	ui.SetTooltips(m_iPad, -1, IDS_TOOLTIPS_EXIT);

	return S_OK;
}

HRESULT CXuiSceneTrading::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

HRESULT CXuiSceneTrading::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		HXUIOBJ hObj=nullptr;
		HRESULT hr=XuiControlGetVisual(m_offerInfoControl.m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"text_measurer",&m_hOfferInfoTextMeasurer);
		hr=XuiElementGetChildById(hObj,L"text_name",&m_hOfferInfoText);
		hr=XuiElementGetChildById(hObj,L"text_panel",&m_hOfferInfoTextBkg);
	}

	return S_OK;
}

HRESULT CXuiSceneTrading::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	bHandled = handleKeyDown(pInputData->UserIndex, mapVKToAction(pInputData->dwKeyCode), (pInputData->dwFlags & XUI_INPUT_FLAG_REPEAT) != 0);

	return S_OK;
}

HRESULT CXuiSceneTrading::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

HRESULT CXuiSceneTrading::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	if(pTimer->nId == TRADING_UPDATE_TIMER_ID)
	{
		handleTick();
		bHandled = TRUE;
	}
	return S_OK;
}

int CXuiSceneTrading::mapVKToAction(int vk)
{
	int action = MINECRAFT_ACTION_MAX;
	switch(vk)
	{
	case VK_PAD_A:
		action = ACTION_MENU_A;
		break;
	case VK_PAD_B:
	case VK_PAD_START:
		action = ACTION_MENU_B;
		break;
	case VK_PAD_X:
		action = ACTION_MENU_X;
		break;
	case VK_PAD_Y:
		action = ACTION_MENU_Y;
		break;
	case VK_PAD_DPAD_LEFT:
	case VK_PAD_LTHUMB_LEFT:
		action = ACTION_MENU_LEFT;
		break;
	case VK_PAD_DPAD_RIGHT:
	case VK_PAD_LTHUMB_RIGHT:
		action = ACTION_MENU_RIGHT;
		break;
	case VK_PAD_LTHUMB_UP:
	case VK_PAD_DPAD_UP:
		action = ACTION_MENU_UP;
		break;
	case VK_PAD_LTHUMB_DOWN:
	case VK_PAD_DPAD_DOWN:
		action = ACTION_MENU_DOWN;
		break;
	case VK_PAD_LTRIGGER:
		action = ACTION_MENU_PAGEUP;
		break;
	case VK_PAD_RTRIGGER:
		action = ACTION_MENU_PAGEDOWN;
		break;
	case VK_PAD_LSHOULDER:
		action = ACTION_MENU_LEFT_SCROLL;
		break;
	case VK_PAD_RSHOULDER:
		action = ACTION_MENU_RIGHT_SCROLL;
		break;
	case VK_PAD_RTHUMB_UP:
		action =  ACTION_MENU_OTHER_STICK_UP;
		break;
	case VK_PAD_RTHUMB_DOWN:
		action =  ACTION_MENU_OTHER_STICK_DOWN;
		break;
	case VK_PAD_RTHUMB_RIGHT:
		action =  ACTION_MENU_OTHER_STICK_RIGHT;
		break;
	case VK_PAD_RTHUMB_LEFT:
		action =  ACTION_MENU_OTHER_STICK_LEFT;
		break;
	};

	return action;
}

void CXuiSceneTrading::showScrollRightArrow(bool show)
{
	m_scrollRight.SetShow(show?TRUE:FALSE);
	m_scrollRight.PlayOptionalVisual(L"ScrollMore",L"EndScrollMore");
}

void CXuiSceneTrading::showScrollLeftArrow(bool show)
{
	m_scrollLeft.SetShow(show?TRUE:FALSE);
	m_scrollLeft.PlayOptionalVisual(L"ScrollMore",L"EndScrollMore");
}

void CXuiSceneTrading::moveSelector(bool right)
{
	D3DXVECTOR3 vec;

	vec.z=0.0f;
	vec.x=m_vSelectorInitialPos.x + (m_selectedSlot*m_fSlotSize);
	vec.y=m_vSelectorInitialPos.y;
	m_tradingSelector.SetPosition(&vec);
}

void CXuiSceneTrading::setTitle(const wstring &name)
{
	XuiControlSetText(m_villagerText,name.c_str());
}

void CXuiSceneTrading::setRequest1Name(const wstring &name)
{
	m_request1Label.SetText(name.c_str());
}

void CXuiSceneTrading::setRequest2Name(const wstring &name)
{
	m_request2Label.SetText(name.c_str());
}

void CXuiSceneTrading::setRequest1RedBox(bool show)
{
	m_request1Control->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneTrading::setRequest2RedBox(bool show)
{
	m_request2Control->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneTrading::setTradeRedBox(int index, bool show)
{
	m_tradeHSlots[index]->SetRedBox(show?TRUE:FALSE);
}

void CXuiSceneTrading::setRequest1Item(shared_ptr<ItemInstance> item)
{
	m_request1Control->SetIcon(getPad(), item, 12, 31, true);
}

void CXuiSceneTrading::setRequest2Item(shared_ptr<ItemInstance> item)
{
	m_request2Control->SetIcon(getPad(), item, 12, 31, true);
}

void CXuiSceneTrading::setTradeItem(int index, shared_ptr<ItemInstance> item)
{
	m_tradeHSlots[index]->SetIcon(getPad(), item, 12, 31, true);
}

void CXuiSceneTrading::setOfferDescription(const wstring &name, vector<wstring> &unformattedStrings)
{
	if(name.empty())
	{
		m_offerInfoControl.SetText(L"");
		m_offerInfoControl.SetShow(FALSE);
		return;
	}

	bool smallPointer = m_bSplitscreen || (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen());
	wstring desc = L"<font size=\"" + std::to_wstring(smallPointer ? 12 :14) + L"\">" + name + L"</font>";

	XUIRect tempXuiRect, xuiRect;
	HRESULT hr;
	xuiRect.right = 0;

    for (auto& it : unformattedStrings )
    {
		XuiTextPresenterMeasureText(m_hOfferInfoTextMeasurer, it.c_str(), &tempXuiRect);
		if(tempXuiRect.right > xuiRect.right) xuiRect = tempXuiRect;
	}

	// Set size with the new width so that the HTML height check is correct
	XuiElementSetBounds(m_hOfferInfoTextBkg,xuiRect.right+4.0f+4.0f+4.0f,xuiRect.bottom); // edge graphics are 8 pixels, with 4 for the border, extra 4 for the background is fudge
	XuiElementSetBounds(m_hOfferInfoText,xuiRect.right+4.0f+4.0f,xuiRect.bottom); // edge graphics are 8 pixels, text is centred

	XuiHtmlSetText(m_hOfferInfoText, desc.c_str() );

	// Check if we need to resize the box
	XUIContentDims contentDims;
	XuiHtmlGetContentDims(m_hOfferInfoText,&contentDims);
	xuiRect.bottom = contentDims.nContentHeight;

	// Set the new height
	float backgroundWidth = xuiRect.right+4.0f+4.0f+4.0f;
	XuiElementSetBounds(m_hOfferInfoTextBkg,backgroundWidth,xuiRect.bottom+4.0f+4.0f); // edge graphics are 8 pixels, with 4 for the border, extra 4 for the background is fudge
	XuiElementSetBounds(m_hOfferInfoText,xuiRect.right+4.0f+4.0f,xuiRect.bottom+4.0f+4.0f); // edge graphics are 8 pixels, text is centred

	m_offerInfoControl.SetShow(TRUE);

	D3DXVECTOR3 highlightPos, offerInfoPos;
	float highlightWidth, highlightHeight;
	m_tradingSelector.GetPosition(&highlightPos);
	m_tradingSelector.GetBounds(&highlightWidth,&highlightHeight);
	m_offerInfoControl.GetPosition(&offerInfoPos);

	if(m_selectedSlot < DISPLAY_TRADES_COUNT/2)
	{
		// Display on the right
		offerInfoPos.x = Math::round(highlightPos.x + highlightWidth * 1.1);
	}
	else
	{
		// Display on the left
		offerInfoPos.x = Math::round(highlightPos.x - backgroundWidth - highlightWidth * 0.1);
	}
	m_offerInfoControl.SetPosition(&offerInfoPos);
}