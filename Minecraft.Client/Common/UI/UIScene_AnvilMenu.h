#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "IUIScene_AnvilMenu.h"
#include "..\Minecraft.World\MerchantMenu.h"

class InventoryMenu;

class UIScene_AnvilMenu : public UIScene_AbstractContainerMenu, public IUIScene_AnvilMenu
{
private:
	bool m_showingCross;

	enum EControls
	{
		eControl_TextInput,
	};

public:
	UIScene_AnvilMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_AnvilMenu;}

protected:
	UIControl_SlotList m_slotListItem1, m_slotListItem2, m_slotListResult;
	UIControl_Label m_labelAnvil;
	UIControl_TextInput m_textInputAnvil;

	IggyName m_funcShowRedCross, m_funcSetCostLabel;

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			UI_MAP_ELEMENT( m_slotListItem1, "Ingredient")
			UI_MAP_ELEMENT( m_slotListItem2, "Ingredient2")
			UI_MAP_ELEMENT( m_slotListResult, "Result")
			UI_MAP_ELEMENT( m_labelAnvil, "AnvilText")
			UI_MAP_ELEMENT( m_textInputAnvil, "AnvilTextInput")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME(m_funcShowRedCross, L"ShowRedCross")
		UI_MAP_NAME(m_funcSetCostLabel, L"SetCostLabel")
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

#ifdef _WINDOWS64
	virtual void getDirectEditInputs(vector<UIControl_TextInput*> &inputs);
	virtual void onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result);
#endif
	static int KeyboardCompleteCallback(LPVOID lpParam,bool bRes);
	virtual void handleEditNamePressed();
	virtual void setEditNameValue(const wstring &name);
	virtual void setEditNameEditable(bool enabled);
	virtual void handleDestroy();

	void setCostLabel(const wstring &label, bool canAfford);
	void showCross(bool show);
};