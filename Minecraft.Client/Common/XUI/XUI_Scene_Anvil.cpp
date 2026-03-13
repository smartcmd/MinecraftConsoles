#include "stdafx.h"

#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Common/Tutorial/TutorialEnum.h"
#include "../../Minecraft.h"
#include "XUI_Ctrl_SlotList.h"
#include "XUI_Scene_Anvil.h"


//--------------------------------------------------------------------------------------
// Name: CXuiSceneAnvil::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiSceneAnvil::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(m_anvilText,app.GetString(IDS_REPAIR_AND_NAME));
	
	m_cross.SetShow(FALSE);
	m_editName.SetText(L"");
	m_editName.SetTextLimit(30);
	m_editName.SetTitleAndText(IDS_TITLE_RENAME,IDS_TITLE_RENAME);

	Minecraft *pMinecraft = Minecraft::GetInstance();

	AnvilScreenInput* initData = static_cast<AnvilScreenInput *>(pInitData->pvInitData);
	m_iPad=initData->iPad;
	m_bSplitscreen=initData->bSplitscreen;
	m_inventory = initData->inventory;

	// if we are in splitscreen, then we need to figure out if we want to move this scene

	if(m_bSplitscreen)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);	
	}

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		m_previousTutorialState = gameMode->getTutorial()->getCurrentState();
		gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Anvil_Menu, this);
	}

	m_repairMenu = new RepairMenu( initData->inventory, initData->level, initData->x, initData->y, initData->z, pMinecraft->localplayers[m_iPad] );	
	m_repairMenu->addSlotListener(this);

	InitDataAssociations(m_iPad, m_repairMenu);

	delete initData;

	CXuiSceneAbstractContainer::Initialize( m_iPad, m_repairMenu, true, RepairMenu::INV_SLOT_START, eSectionAnvilUsing, eSectionAnvilMax );
	
	//ProfileManager.SetRichPresenceContextValue(m_iPad,CONTEXT_GAME_STATE,CONTEXT_GAME_STATE_FORGING);
	
	XuiSetTimer(m_hObj,ANVIL_UPDATE_TIMER_ID,ANVIL_UPDATE_TIMER_TIME);

	return S_OK;
}

HRESULT CXuiSceneAnvil::OnDestroy()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[m_iPad] != nullptr )
	{
		TutorialMode *gameMode = static_cast<TutorialMode *>(pMinecraft->localgameModes[m_iPad]);
		if(gameMode != nullptr) gameMode->getTutorial()->changeTutorialState(m_previousTutorialState);
	}

	// 4J Stu - Fix for #11302 - TCR 001: Network Connectivity: Host crashed after being killed by the client while accessing a chest during burst packet loss.
	// We need to make sure that we call closeContainer() anytime this menu is closed, even if it is forced to close by some other reason (like the player dying)	
	if(Minecraft::GetInstance()->localplayers[m_iPad] != nullptr) Minecraft::GetInstance()->localplayers[m_iPad]->closeContainer();
	return S_OK;
}

HRESULT CXuiSceneAnvil::OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled)
{
	if(hObjSource == m_editName)
	{
		wstring newValue = m_editName.GetText();
		LPCWSTR szText=newValue.c_str();
		stripWhitespaceForHtml(newValue);

		// strip leading spaces
		wstring b;
		size_t start = newValue.find_first_not_of(L" ");
		size_t end = newValue.find_last_not_of(L" ");

		if( start == wstring::npos )
		{
			// the string is all space
			newValue=L"";
		}
		else
		{
			if( end == wstring::npos ) end = newValue.size() - 1;
			b = newValue.substr(start,(end-start)+1);
			newValue=b;
		}

		// 4J-PB - This was stopping the player deleting the last character in the name using the chatpad
		//if(!newValue.empty() && !(newValue.size() == 1 && newValue.at(0) == L' '))
		{
			newValue = escapeXML(newValue);
			m_itemName = newValue;
			updateItemName();   
		}
// 		else
// 		{
// 			LPCWSTR szText=m_itemName.c_str();
// 			m_editName.SetText(szText);
// 			m_editName.SetCaretPosition(m_itemName.length());
// 		}
	}

	return S_OK;
}

HRESULT CXuiSceneAnvil::handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	if(pTimer->nId == ANVIL_UPDATE_TIMER_ID)
	{
		handleTick();
		bHandled = TRUE;
	}
	return S_OK;
}

CXuiControl* CXuiSceneAnvil::GetSectionControl( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionAnvilItem1:
			return (CXuiControl *)m_ingredient1Control;
			break;
		case eSectionAnvilItem2:
			return (CXuiControl *)m_ingredient2Control;
			break;
		case eSectionAnvilResult:
			return (CXuiControl *)m_resultControl;
			break;
		case eSectionAnvilName:
			return (CXuiControl *)&m_editName;
			break;
		case eSectionAnvilInventory:
			return (CXuiControl *)m_inventoryControl;
			break;
		case eSectionAnvilUsing:
			return (CXuiControl *)m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

CXuiCtrlSlotList* CXuiSceneAnvil::GetSectionSlotList( ESceneSection eSection )
{
	switch( eSection )
	{
		case eSectionAnvilItem1:
			return m_ingredient1Control;
			break;
		case eSectionAnvilItem2:
			return m_ingredient2Control;
			break;
		case eSectionAnvilResult:
			return m_resultControl;
			break;
		case eSectionAnvilInventory:
			return m_inventoryControl;
			break;
		case eSectionAnvilUsing:
			return m_useRowControl;
			break;
		default:
			assert( false );
			break;
	}
	return nullptr;
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneAnvil::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	// TODO Inventory dimensions need defined as constants
	m_resultControl->SetData( iPad, menu, 1, 1, RepairMenu::RESULT_SLOT );
	
	m_ingredient1Control->SetData( iPad, menu, 1, 1, RepairMenu::INPUT_SLOT );
	m_ingredient2Control->SetData( iPad, menu, 1, 1, RepairMenu::ADDITIONAL_SLOT);

	//m_litProgressControl->SetUserData( initData->furnace.get() );
	
	//m_burnProgress->SetUserData( initData->furnace.get() );

	CXuiSceneAbstractContainer::InitDataAssociations(iPad, menu, RepairMenu::INV_SLOT_START);
}

void CXuiSceneAnvil::handleEditNamePressed()
{
	// 4J Stu - The edit control already handles A/Start presses, so ignore anything else
	//m_editName.RequestKeyboard(m_iPad);
}

void CXuiSceneAnvil::setEditNameValue(const wstring &name)
{
	wstring parsedName = parseXMLSpecials(name);
	m_editName.SetText(parsedName.c_str());
	m_editName.SetCaretPosition(parsedName.length());
}

void CXuiSceneAnvil::setEditNameEditable(bool enabled)
{
}

void CXuiSceneAnvil::setCostLabel(const wstring &label, bool canAfford)
{
	if(canAfford)
	{
		m_affordableText.SetText(label.c_str());
		m_affordableText.SetShow(TRUE);
		m_expensiveText.SetShow(FALSE);
	}
	else
	{
		m_expensiveText.SetText(label.c_str());
		m_affordableText.SetShow(FALSE);
		m_expensiveText.SetShow(TRUE);
	}
}

void CXuiSceneAnvil::showCross(bool show)
{
	m_cross.SetShow(show?TRUE:FALSE);
}