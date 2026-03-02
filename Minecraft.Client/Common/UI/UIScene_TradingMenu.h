#pragma once

#include "IUIScene_TradingMenu.h"

class InventoryMenu;

class UIScene_TradingMenu : public UIScene, public IUIScene_TradingMenu
{
private:
	bool m_showingRightArrow, m_showingLeftArrow;

public:
	UIScene_TradingMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_TradingMenu;}

protected:
	UIControl m_controlMainPanel;
	UIControl_SlotList m_slotListTrades;
	UIControl_SlotList m_slotListRequest1, m_slotListRequest2;
	UIControl_SlotList m_slotListHotbar, m_slotListInventory;
	UIControl_Label m_labelInventory;
	UIControl_Label m_labelTrading, m_labelRequired;
	UIControl_Label m_labelRequest1, m_labelRequest2;

	IggyName m_funcMoveSelector, m_funcShowScrollRightArrow, m_funcShowScrollLeftArrow, m_funcSetOfferDescription, m_funcSetActiveSlot;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListTrades, "TradingBar")
			UI_MAP_ELEMENT( m_slotListRequest1, "Request1")
			UI_MAP_ELEMENT( m_slotListRequest2, "Request2")

			UI_MAP_ELEMENT( m_labelTrading, "VillagerText")
			UI_MAP_ELEMENT( m_labelRequired, "RequiredLabel")

			UI_MAP_ELEMENT( m_labelRequest1, "Request1Label")
			UI_MAP_ELEMENT( m_labelRequest2, "Request2Label")

			UI_MAP_ELEMENT( m_slotListHotbar, "HotBar")
			UI_MAP_ELEMENT( m_slotListInventory, "Inventory")
			UI_MAP_ELEMENT( m_labelInventory, "InventoryLabel")

		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME(m_funcMoveSelector, L"MoveSelector")
		UI_MAP_NAME(m_funcShowScrollRightArrow, L"ShowScrollRightArrow")
		UI_MAP_NAME(m_funcShowScrollLeftArrow, L"ShowScrollLeftArrow")
		UI_MAP_NAME(m_funcSetOfferDescription, L"SetOfferDescription")
		UI_MAP_NAME(m_funcSetActiveSlot, L"SetSelectorSlot")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void updateTooltips();
	virtual void handleDestroy();
	virtual void handleReload();

	virtual void tick();
	
	void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	void customDraw(IggyCustomDrawCallbackRegion *region);

	virtual void showScrollRightArrow(bool show);
	virtual void showScrollLeftArrow(bool show);
	virtual void moveSelector(bool right);
	virtual void setTitle(const wstring &name);
	virtual void setRequest1Name(const wstring &name);
	virtual void setRequest2Name(const wstring &name);

	virtual void setRequest1RedBox(bool show);
	virtual void setRequest2RedBox(bool show);
	virtual void setTradeRedBox(int index, bool show);

	virtual void setOfferDescription(vector<HtmlString> *description);
	
	virtual void HandleMessage(EUIMessage message, void *data);
	void handleInventoryUpdated(LPVOID data);

	int getPad() { return m_iPad; }
};