#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "UIControl_SlotList.h"
#include "IUIScene_BeaconMenu.h"

class UIScene_BeaconMenu : public UIScene_AbstractContainerMenu, public IUIScene_BeaconMenu
{
private:
	enum EControls
	{
		eControl_Primary1,
		eControl_Primary2,
		eControl_Primary3,
		eControl_Primary4,
		eControl_Primary5,
		eControl_Secondary1,
		eControl_Secondary2,

		eControl_EFFECT_COUNT,
	};
public:
	UIScene_BeaconMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_BeaconMenu;}

protected:
	UIControl_SlotList m_slotListActivator;
	UIControl_SlotList m_slotListActivatorIcons;
	UIControl_Label m_labelPrimary, m_labelSecondary;
	UIControl_BeaconEffectButton m_buttonsPowers[eControl_EFFECT_COUNT];
	UIControl_BeaconEffectButton m_buttonConfirm;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListActivator, "ActivatorSlot")
			UI_MAP_ELEMENT( m_slotListActivatorIcons, "ActivatorList")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Primary1], "Primary_Slot_01")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Primary2], "Primary_Slot_02")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Primary3], "Primary_Slot_03")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Primary4], "Primary_Slot_04")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Primary5], "Primary_Slot_05")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Secondary1], "Secondary_Slot_01")
			UI_MAP_ELEMENT( m_buttonsPowers[eControl_Secondary2], "Secondary_Slot_02")
			UI_MAP_ELEMENT( m_buttonConfirm, "ConfirmButton")
			UI_MAP_ELEMENT( m_labelPrimary, "PrimaryPowerLabel")
			UI_MAP_ELEMENT( m_labelSecondary, "SecondaryPowerLabel")
		UI_END_MAP_CHILD_ELEMENTS()
	UI_END_MAP_ELEMENTS_AND_NAMES()

	virtual wstring getMoviePath();
	virtual void handleReload();
	virtual void tick();
	virtual int GetBaseSlotCount() { return 4; }

	virtual int getSectionColumns(ESceneSection eSection);
	virtual int getSectionRows(ESceneSection eSection);
	virtual void GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition );
	virtual void GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize );
	virtual void handleSectionClick(ESceneSection eSection) {}
	virtual void setSectionSelectedSlot(ESceneSection eSection, int x, int y);

	virtual UIControl *getSection(ESceneSection eSection);
	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

	virtual void SetConfirmButtonEnabled(bool enabled);
	virtual void AddPowerButton(int id, int icon, int tier, int count, bool active, bool selected);
	virtual int GetPowerButtonId(ESceneSection eSection);
	virtual bool IsPowerButtonSelected(ESceneSection eSection);
	virtual void SetPowerButtonSelected(ESceneSection eSection);
};