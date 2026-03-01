#pragma once

#include "UIScene_AbstractContainerMenu.h"
#include "IUIScene_CreativeMenu.h"

class UIScene_CreativeMenu : public UIScene_AbstractContainerMenu, public IUIScene_CreativeMenu
{
public:
	UIScene_CreativeMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_CreativeMenu;}

protected:
	UIControl_SlotList m_slotListContainer;
	IggyName m_funcSetActiveTab, m_funcSetScrollBar;

	enum ETouchInput
	{
		ETouchInput_TouchPanel_0,
		ETouchInput_TouchPanel_1,
		ETouchInput_TouchPanel_2,
		ETouchInput_TouchPanel_3,
		ETouchInput_TouchPanel_4,
		ETouchInput_TouchPanel_5,
		ETouchInput_TouchPanel_6,
		ETouchInput_TouchPanel_7,
		ETouchInput_TouchSlider,
		
		ETouchInput_Count,
	};

#ifdef __PSVITA__
	// 4J - TomK - this only needs to be a touch component on vita!
	UIControl_Touch m_TouchInput[ETouchInput_Count];
#else
	UIControl_Base m_TouchInput[ETouchInput_Count];
#endif

	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene_AbstractContainerMenu)
		UI_BEGIN_MAP_CHILD_ELEMENTS( m_controlMainPanel )
			
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_0], "TouchPanel_0" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_1], "TouchPanel_1" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_2], "TouchPanel_2" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_3], "TouchPanel_3" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_4], "TouchPanel_4" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_5], "TouchPanel_5" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_6], "TouchPanel_6" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchPanel_7], "TouchPanel_7" )
			UI_MAP_ELEMENT( m_TouchInput[ETouchInput_TouchSlider], "TouchPanel_Slider" )

			UI_MAP_ELEMENT( m_slotListContainer, "containerList")
		UI_END_MAP_CHILD_ELEMENTS()

		UI_MAP_NAME(m_funcSetActiveTab, L"SetActiveTab")
		UI_MAP_NAME(m_funcSetScrollBar, L"SetScrollBar")
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

	virtual void handleOtherClicked(int iPad, ESceneSection eSection, int buttonNum, bool quickKey);

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

#ifdef __PSVITA__
	virtual void handleTouchInput(unsigned int iPad, S32 x, S32 y, int iId, bool bPressed, bool bRepeat, bool bReleased);
	virtual UIControl* GetMainPanel();
	virtual void handleTouchBoxRebuild();
	virtual void handleTimerComplete(int id);
#endif

private:
	// IUIScene_CreativeMenu
	void updateTabHighlightAndText(ECreativeInventoryTabs tab);
	void updateScrollCurrentPage(int currentPage, int pageCount);
	bool m_bFirstCall;
};