#pragma once
#include "Common\UI\IUIScene_AbstractContainerMenu.h"

class BeaconTileEntity;

class IUIScene_BeaconMenu : public virtual IUIScene_AbstractContainerMenu
{
public:
	IUIScene_BeaconMenu();

	virtual ESceneSection GetSectionAndSlotInDirection(ESceneSection eSection, ETapState eTapDirection, int *piTargetX, int *piTargetY);
	int getSectionStartOffset(ESceneSection eSection);
	virtual void handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey);
	virtual bool IsSectionSlotList( ESceneSection eSection );
	virtual vector<HtmlString> *GetSectionHoverText(ESceneSection eSection);
	bool IsVisible( ESceneSection eSection );

protected:
	void handleTick();
	int GetId(int tier, int effectId);

	virtual void SetConfirmButtonEnabled(bool enabled) = 0;
	virtual void AddPowerButton(int id, int icon, int tier, int count, bool active, bool selected) = 0;
	virtual int GetPowerButtonId(ESceneSection eSection) = 0;
	virtual bool IsPowerButtonSelected(ESceneSection eSection) = 0;
	virtual void SetPowerButtonSelected(ESceneSection eSection) = 0;

	shared_ptr<BeaconTileEntity> m_beacon;
	bool m_initPowerButtons;
};

