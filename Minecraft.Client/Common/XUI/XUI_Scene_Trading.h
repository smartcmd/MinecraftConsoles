#pragma once
using namespace std;
#include "../Media/xuiscene_trading.h"
#include "../UI/IUIScene_TradingMenu.h"

#define TRADING_UPDATE_TIMER_ID (10)
#define TRADING_UPDATE_TIMER_TIME (50)

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneTrading : public CXuiSceneImpl, public IUIScene_TradingMenu
{
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneTrading, L"CXuiSceneTrading", XUI_CLASS_SCENE )

private:
	CXuiCtrlCraftIngredientSlot *m_request1Control;
	CXuiCtrlCraftIngredientSlot *m_request2Control;

	CXuiCtrlCraftIngredientSlot *m_tradeHSlots[7];

	CXuiControl m_request1Label, m_request2Label;
	CXuiControl m_inventoryLabel, m_requiredLabel;

	CXuiControl m_villagerText;
	CXuiControl m_scrollLeft, m_scrollRight, m_tradingSelector;

	CXuiControl m_sceneGroup;
	CXuiElement m_hGridInventory;

	CXuiCtrlSlotList* m_inventoryControl;
	CXuiCtrlSlotList* m_useRowControl;

	CXuiControl m_offerInfoControl;
	HXUIOBJ m_hOfferInfoTextMeasurer;
	HXUIOBJ m_hOfferInfoText;
	HXUIOBJ m_hOfferInfoTextBkg;

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_TIMER( OnTimer )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_sceneGroup )
			MAP_OVERRIDE(IDC_Request1, m_request1Control)
			MAP_OVERRIDE(IDC_Request2, m_request2Control)

			MAP_OVERRIDE(IDC_TradingBar0, m_tradeHSlots[0])
			MAP_OVERRIDE(IDC_TradingBar1, m_tradeHSlots[1])
			MAP_OVERRIDE(IDC_TradingBar2, m_tradeHSlots[2])
			MAP_OVERRIDE(IDC_TradingBar3, m_tradeHSlots[3])
			MAP_OVERRIDE(IDC_TradingBar4, m_tradeHSlots[4])
			MAP_OVERRIDE(IDC_TradingBar5, m_tradeHSlots[5])
			MAP_OVERRIDE(IDC_TradingBar6, m_tradeHSlots[6])

			MAP_CONTROL(IDC_Offer1Label, m_request1Label)
			MAP_CONTROL(IDC_Offer2Label, m_request2Label)

			MAP_CONTROL(IDC_VillagerText,m_villagerText)
			MAP_CONTROL(IDC_InventoryLabel,m_inventoryLabel)
			MAP_CONTROL(IDC_RequiredLabel,m_requiredLabel)

			MAP_CONTROL(IDC_ScrollLeftArrow,m_scrollLeft)
			MAP_CONTROL(IDC_ScrollRightArrow,m_scrollRight)
			MAP_CONTROL(IDC_TradingSelector,m_tradingSelector)

			MAP_CONTROL(IDC_HtmlTextPanel,m_offerInfoControl)

			MAP_CONTROL(IDC_InventoryGrid, m_hGridInventory)
			BEGIN_MAP_CHILD_CONTROLS(m_hGridInventory)
				MAP_OVERRIDE(IDC_Inventory, m_inventoryControl)
				MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()		
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnDestroy();
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );

protected:
	int	m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	bool m_bSplitscreen;

public:
	int getPad() { return m_iPad; }

private:

	float m_fSlotSize;
	D3DXVECTOR3  m_vTradeSlot0Pos;
	D3DXVECTOR3  m_vSelectorInitialPos;

	int mapVKToAction(int vk);
protected:
	virtual void showScrollRightArrow(bool show);
	virtual void showScrollLeftArrow(bool show);
	virtual void moveSelector(bool right);
	virtual void setRequest1Name(const wstring &name);
	virtual void setRequest2Name(const wstring &name);
	virtual void setTitle(const wstring &name);

	virtual void setRequest1RedBox(bool show);
	virtual void setRequest2RedBox(bool show);
	virtual void setTradeRedBox(int index, bool show);

	virtual void setRequest1Item(shared_ptr<ItemInstance> item);
	virtual void setRequest2Item(shared_ptr<ItemInstance> item);
	virtual void setTradeItem(int index, shared_ptr<ItemInstance> item);
	
	virtual void setOfferDescription(const wstring &name, vector<wstring> &unformattedStrings);
};