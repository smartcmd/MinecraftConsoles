#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "IUIScene_FireworksMenu.h"

class InventoryMenu;

class UIScene_FireworksMenu : public UIScene_AbstractContainerMenu, public IUIScene_FireworksMenu
{
public:
	UIScene_FireworksMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_FireworksMenu;}

protected:
	UIControl_SlotList m_slotListResult, m_slotList3x3, m_slotList2x2;
	UIControl_Label m_labelFireworks;
	IggyName m_funcShowLargeCraftingGrid;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListResult, "Result")
			UI_MAP_ELEMENT( m_slotList3x3, "Fireworks3x3")
			UI_MAP_ELEMENT( m_slotList2x2, "Fireworks2x2")
			UI_MAP_ELEMENT( m_labelFireworks, "FireworksLabel")

			UI_MAP_NAME( m_funcShowLargeCraftingGrid, L"ShowLargeCraftingGrid")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();

	virtual int getSectionColumns(ESceneSection eSection);
	virtual int getSectionRows(ESceneSection eSection);
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition );
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize );
	virtual void handleSectionClick(ESceneSection eSection) {}
	virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y);

	virtual UIControl *getSection(ESceneSection eSection);

	void ShowLargeCraftingGrid(boolean bShow);
};