#pragma once

#include "UIScene.h"
#include "IUIScene_AbstractContainerMenu.h"

class AbstractContainerMenu;

class UIScene_AbstractContainerMenu : public UIScene, public virtual IUIScene_AbstractContainerMenu
{
private:
	ESceneSection m_focusSection;
	bool m_bIgnoreInput;
#ifdef _WINDOWS64
	bool m_bMouseDragSlider;
#endif

protected:
	UIControl m_controlMainPanel;
	UIControl_SlotList m_slotListHotbar, m_slotListInventory;
	UIControl_Cursor m_cursorPath;
	UIControl_Label m_labelInventory, m_labelBrewingStand;
	UIControl m_controlBackgroundPanel;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_controlMainPanel, "MainPanel" )
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_controlBackgroundPanel, "BackgroundPanel" )
			UI_MAP_ELEMENT( m_slotListHotbar, "hotbarList")
			UI_MAP_ELEMENT( m_slotListInventory, "inventoryList")
			UI_MAP_ELEMENT( m_cursorPath, "cursor")
			UI_MAP_ELEMENT( m_labelInventory, "inventoryLabel")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()
	
public:
	UIScene_AbstractContainerMenu(int iPad, UILayer *parentLayer);
	~UIScene_AbstractContainerMenu();

	virtual void handleDestroy();

	int getPad() { return m_iPad; }
	bool getIgnoreInput() { return m_bIgnoreInput; }
	void setIgnoreInput(bool bVal) { m_bIgnoreInput=bVal; }

protected:
	virtual void PlatformInitialize(int iPad, int startIndex);
	virtual void InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex = 0);
	virtual bool doesSectionTreeHaveFocus(ESceneSection eSection) { return false; }
	virtual void setSectionFocus(ESceneSection eSection, int iPad);
	void setFocusToPointer(int iPad);
	void SetPointerText(const wstring &description, vector<wstring> &unformattedStrings, bool newSlot);
	virtual shared_ptr<ItemInstance> getSlotItem(ESceneSection eSection, int iSlot);
	virtual bool isSlotEmpty(ESceneSection eSection, int iSlot);
	virtual void adjustPointerForSafeZone();

	virtual UIControl *getSection(ESceneSection eSection) { return NULL; }

public:
	virtual void tick();
	
	virtual void render(S32 width, S32 height, C4JRender::eViewportType viewpBort);
	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);
};
