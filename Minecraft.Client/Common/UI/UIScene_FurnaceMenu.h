#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "IUIScene_FurnaceMenu.h"

class InventoryMenu;

class UIScene_FurnaceMenu : public UIScene_AbstractContainerMenu, public IUIScene_FurnaceMenu
{
private:
	shared_ptr<FurnaceTileEntity> m_furnace;

public:
	UIScene_FurnaceMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_FurnaceMenu;}

protected:
	UIControl_SlotList m_slotListFuel, m_slotListIngredient, m_slotListResult;
	UIControl_Label m_labelFurnace, m_labelIngredient, m_labelFuel;
	UIControl_Progress m_progressFurnaceFire, m_progressFurnaceArrow;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListIngredient, "Ingredient")
			UI_MAP_ELEMENT( m_slotListFuel, "Fuel")
			UI_MAP_ELEMENT( m_slotListResult, "Result")
			UI_MAP_ELEMENT( m_labelFurnace, "Furnace_text")
			UI_MAP_ELEMENT( m_labelIngredient, "Ingredient_Label")
			UI_MAP_ELEMENT( m_labelFuel, "Fuel_Label")

			UI_MAP_ELEMENT( m_progressFurnaceFire, "FurnaceFire")
			UI_MAP_ELEMENT( m_progressFurnaceArrow, "FurnaceArrow")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();

	virtual void tick();

	virtual int getSectionColumns(ESceneSection eSection);
	virtual int getSectionRows(ESceneSection eSection);
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition );
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize );
	virtual void handleSectionClick(ESceneSection eSection) {}
	virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y);

	virtual UIControl *getSection(ESceneSection eSection);
};