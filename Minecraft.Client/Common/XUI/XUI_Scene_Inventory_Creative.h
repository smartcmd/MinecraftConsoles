#pragma once
using namespace std;

#include "../Media/xuiscene_inventory_creative.h"
#include "XUI_Scene_AbstractContainer.h"
#include "XUI_CustomMessages.h"
#include "../../../Minecraft.World/AbstractContainerMenu.h"
#include "../../../Minecraft.World/SimpleContainer.h"

#include "../UI/IUIScene_CreativeMenu.h"

#include <vector>

//--------------------------------------------------------------------------------------
// Scene implementation class.
//--------------------------------------------------------------------------------------
class CXuiSceneInventoryCreative : public CXuiSceneAbstractContainer, public IUIScene_CreativeMenu
{
public:
    // Define the class. The class name must match the ClassOverride property
    // set for the scene in the UI Authoring tool.
    XUI_IMPLEMENT_CLASS( CXuiSceneInventoryCreative, L"CXuiSceneInventoryCreative", XUI_CLASS_SCENE )

protected:

	CXuiControl m_hTabGroupA[eCreativeInventoryTab_COUNT];
	CXuiControl m_hGroupIconA[eCreativeInventoryTab_COUNT];
	CXuiElement m_Group;
	CXuiControl m_GroupDescription;

	CXuiSlider m_pageSlider;
	CXuiControl m_scrollUp, m_scrollDown;//, m_pageNumber;

	XUI_BEGIN_MSG_MAP()
        XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_TIMER( OnTimer )			// Poll stick input on a timer.
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_TRANSITION_END( OnTransitionEnd)
   XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Group, m_sceneGroup)
		BEGIN_MAP_CHILD_CONTROLS( m_sceneGroup )
			// Common to all abstract container scenes
			MAP_OVERRIDE(IDC_UseRow, m_useRowControl)
			MAP_OVERRIDE(IDC_Pointer, m_pointerControl)
			MAP_OVERRIDE(IDC_Container, m_containerControl)
			
			MAP_CONTROL(IDC_XuiSlider,m_pageSlider)
			//MAP_CONTROL(IDC_PageNumber,m_pageNumber)
			MAP_CONTROL(IDC_ScrollDown,m_scrollDown)
			MAP_CONTROL(IDC_ScrollUp,m_scrollUp)

			MAP_CONTROL(IDC_InventoryText,m_GroupDescription)

			// 4J JEV - Tabs
			MAP_CONTROL(IDC_Group_Tab_Icons, m_Group)
			BEGIN_MAP_CHILD_CONTROLS(m_Group)
				MAP_CONTROL(IDC_Icon_1,m_hGroupIconA[0])
				MAP_CONTROL(IDC_Icon_2,m_hGroupIconA[1])
				MAP_CONTROL(IDC_Icon_3,m_hGroupIconA[2])
				MAP_CONTROL(IDC_Icon_4,m_hGroupIconA[3])
				MAP_CONTROL(IDC_Icon_5,m_hGroupIconA[4])
				MAP_CONTROL(IDC_Icon_6,m_hGroupIconA[5])
				MAP_CONTROL(IDC_Icon_7,m_hGroupIconA[6])
				MAP_CONTROL(IDC_Icon_8,m_hGroupIconA[7])
			END_MAP_CHILD_CONTROLS()
			MAP_CONTROL(IDC_Group_Tab_Images, m_Group)
			BEGIN_MAP_CHILD_CONTROLS(m_Group)
				MAP_CONTROL(IDC_TabImage1,m_hTabGroupA[0])
				MAP_CONTROL(IDC_TabImage2,m_hTabGroupA[1])
				MAP_CONTROL(IDC_TabImage3,m_hTabGroupA[2])
				MAP_CONTROL(IDC_TabImage4,m_hTabGroupA[3])
				MAP_CONTROL(IDC_TabImage5,m_hTabGroupA[4])
				MAP_CONTROL(IDC_TabImage6,m_hTabGroupA[5])
				MAP_CONTROL(IDC_TabImage7,m_hTabGroupA[6])
				MAP_CONTROL(IDC_TabImage8,m_hTabGroupA[7])
			END_MAP_CHILD_CONTROLS()

		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnTransitionEnd( XUIMessageTransition *pTransData, BOOL& bHandled);
	//HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled) {return S_OK;}

private:
	CXuiControl m_sceneGroup;

	virtual CXuiControl*	GetSectionControl( ESceneSection eSection );
	virtual CXuiCtrlSlotList*	GetSectionSlotList( ESceneSection eSection );

	CXuiCtrlSlotList* m_containerControl;

private:	
	// IUIScene_CreativeMenu
	void updateTabHighlightAndText(ECreativeInventoryTabs tab);
	void updateScrollCurrentPage(int currentPage, int pageCount);
};