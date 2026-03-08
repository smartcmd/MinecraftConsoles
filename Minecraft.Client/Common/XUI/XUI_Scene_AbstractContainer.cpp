#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include <assert.h>

#include "../../../Minecraft.World/Player.h"
#include "../../../Minecraft.Client/LocalPlayer.h"
#include "../../../Minecraft.Client/Minecraft.h"
#include "../../../Minecraft.Client/GameMode.h"
#include "../../../Minecraft.World/AbstractContainerMenu.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"
#include "../../../Minecraft.World/Tile.h"
#include "../../../Minecraft.World/FurnaceRecipes.h"
#include "../../../Minecraft.World/Recipy.h"
#include "../../../Minecraft.World/Recipes.h"
#include "../../../Minecraft.World/ArmorRecipes.h"
#include "../../../Minecraft.World/StringHelpers.h"

#include "../../Common/Tutorial/Tutorial.h"
#include "../../Common/Tutorial/TutorialMode.h"

#include "XUI_Ctrl_SlotList.h"
#include "XUI_Ctrl_SlotItem.h"
#include "XUI_Ctrl_SlotItemListItem.h"
#include "XUI_Scene_AbstractContainer.h"
#ifdef _DEBUG_MENUS_ENABLED
#include "XUI_DebugItemEditor.h"
#endif

#define IGNORE_KEYPRESS_TIMERID 11
#define IGNORE_KEYPRESS_TIME 100

void CXuiSceneAbstractContainer::PlatformInitialize(int iPad, int startIndex)
{
	m_bIgnoreKeyPresses=true;
	m_iPad = iPad;

	XuiControlSetText(m_InventoryText,app.GetString(IDS_INVENTORY));

		// Determine min and max extents for pointer, it needs to be able to move off the container to drop items.
	float fPanelWidth, fPanelHeight;
	float fPointerWidth, fPointerHeight;

	// We may have varying depths of controls here, so base off the pointers parent
	HXUIOBJ parent;
	XuiElementGetBounds( m_pointerControl->m_hObj, &fPointerWidth, &fPointerHeight );
	XuiElementGetParent( m_pointerControl->m_hObj, &parent );
	m_pointerControl->SetShow(TRUE);
	XuiElementGetBounds( parent, &fPanelWidth, &fPanelHeight );
	// Get size of pointer
	m_fPointerImageOffsetX = floor(fPointerWidth/2.0f);
	m_fPointerImageOffsetY = floor(fPointerHeight/2.0f);

	m_fPanelMinX = 0.0f;
	m_fPanelMaxX = fPanelWidth;
	m_fPanelMinY = 0.0f;
	m_fPanelMaxY = fPanelHeight;

	// 4J-PB - need to limit this in splitscreen
	if(app.GetLocalPlayerCount()>1)
	{
		// don't let the pointer go into someone's screen
		m_fPointerMinY = floor(fPointerHeight/2.0f);
	}
	else
	{
		m_fPointerMinY = -fPointerHeight;
	}
	m_fPointerMinX = -fPointerWidth;
	m_fPointerMaxX = fPanelWidth + fPointerWidth;
	m_fPointerMaxY = fPanelHeight + (fPointerHeight/2);

// 	m_hPointerText=NULL;
// 	m_hPointerTextBkg=NULL;

	UIVec2D itemPos;
	UIVec2D itemSize;
	GetItemScreenData( m_eCurrSection, 0, &( itemPos ), &( itemSize ) );

	UIVec2D sectionPos;
	GetPositionOfSection( m_eCurrSection, &( sectionPos ) );

	UIVec2D vPointerPos = sectionPos;
	vPointerPos += itemPos;
	vPointerPos.x += ( itemSize.x / 2.0f );
	vPointerPos.y += ( itemSize.y / 2.0f );

	vPointerPos.x -= m_fPointerImageOffsetX;
	vPointerPos.y -= m_fPointerImageOffsetY;

	D3DXVECTOR3 newPointerPos;
	newPointerPos.x = vPointerPos.x;
	newPointerPos.y = vPointerPos.y;
	newPointerPos.z = 0.0f;
	m_pointerControl->SetPosition( &newPointerPos );
	m_pointerPos.x = newPointerPos.x;
	m_pointerPos.y = newPointerPos.y;

#ifdef USE_POINTER_ACCEL
	m_fPointerVelX = 0.0f;
	m_fPointerVelY = 0.0f;
	m_fPointerAccelX = 0.0f;
	m_fPointerAccelY = 0.0f;
#endif

	// Add timer to poll controller stick input at 60Hz
	HRESULT timerResult = SetTimer( POINTER_INPUT_TIMER_ID, ( 1000 / 60 ) );
	assert( timerResult == S_OK );

	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);

	// Disable the default navigation behaviour for all slot lsit items (prevent old style cursor navigation).
	for ( int iSection = m_eFirstSection; iSection < m_eMaxSection; ++iSection )
	{
		ESceneSection eSection = ( ESceneSection )( iSection );

		if(!IsSectionSlotList(eSection)) continue;

		// Get dimensions of this section.
		int iNumRows;
		int iNumColumns;
		int iNumItems = GetSectionDimensions( eSection, &( iNumColumns ), &( iNumRows ) );

		for ( int iItem = 0; iItem < iNumItems; ++iItem )
		{
			CXuiCtrlSlotItemListItem* pCXuiCtrlSlotItem;
			GetSectionSlotList( eSection )->GetCXuiCtrlSlotItem( iItem, &( pCXuiCtrlSlotItem ) );
			pCXuiCtrlSlotItem->SetSkipsDefaultNavigation( TRUE );
		}
	}
}

// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
void CXuiSceneAbstractContainer::InitDataAssociations(int iPad, AbstractContainerMenu *menu, int startIndex /*= 0*/)
{
	// TODO Inventory dimensions need defined as constants
	m_inventoryControl->SetData( iPad, menu, 3, 9, startIndex, startIndex + 3*9 );

	// TODO Inventory dimensions need defined as constants
	m_useRowControl->SetData( iPad, menu, 1, 9, startIndex + 3*9, startIndex + 4*9 );

	m_pointerControl->SetUserIndex(m_pointerControl->m_hObj, iPad);
}

int CXuiSceneAbstractContainer::getSectionColumns(ESceneSection eSection)
{
	return GetSectionSlotList( eSection )->GetColumns();
}

int CXuiSceneAbstractContainer::getSectionRows(ESceneSection eSection)
{
	return GetSectionSlotList( eSection )->GetRows();
}

// Adding this so we can turn off the pointer text background, since it flickers on then off at the start of a scene where a tutorial popup is
HRESULT CXuiSceneAbstractContainer::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{
		// 4J Stu - Added to support auto-save. Need to re-associate on a navigate back
		if(pTransition->dwTransType == XUI_TRANSITION_BACKTO)
		{
			// Add timer to poll controller stick input at 60Hz
			HRESULT timerResult = SetTimer( POINTER_INPUT_TIMER_ID, ( 1000 / 60 ) );
			assert( timerResult == S_OK );

			InitDataAssociations(m_iPad, m_menu);
		}

		HXUIOBJ hObj=NULL;
		HRESULT hr=XuiControlGetVisual(m_pointerControl->m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"text_measurer",&m_hPointerTextMeasurer);
		hr=XuiElementGetChildById(hObj,L"text_name",&m_hPointerText);
		hr=XuiElementGetChildById(hObj,L"text_panel",&m_hPointerTextBkg);
		hr=XuiElementGetChildById(hObj,L"pointer_image",&m_hPointerImg);
		XuiElementSetShow(m_hPointerText,FALSE);
		XuiElementSetShow(m_hPointerTextBkg,FALSE);
	}

	return S_OK;
}


D3DXVECTOR3 CXuiSceneAbstractContainer::GetCursorScreenPosition()
{
	return app.GetElementScreenPosition(m_pointerControl->m_hObj);
}

HRESULT CXuiSceneAbstractContainer::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
{
	if(m_bIgnoreKeyPresses) return S_OK;

	bHandled = handleKeyDown(pInputData->UserIndex, mapVKToAction(pInputData->dwKeyCode), (pInputData->dwFlags & XUI_INPUT_FLAG_REPEAT) != 0);

	return S_OK;
}

int CXuiSceneAbstractContainer::mapVKToAction(int vk)
{
	int action = MINECRAFT_ACTION_MAX;
	switch(vk)
	{
	case VK_PAD_A:
		action = ACTION_MENU_A;
		break;
	case VK_PAD_B:
	case VK_PAD_START:
		action = ACTION_MENU_B;
		break;
	case VK_PAD_X:
		action = ACTION_MENU_X;
		break;
	case VK_PAD_Y:
		action = ACTION_MENU_Y;
		break;
	case VK_PAD_DPAD_LEFT:
		action = ACTION_MENU_LEFT;
		break;
	case VK_PAD_DPAD_RIGHT:
		action = ACTION_MENU_RIGHT;
		break;
	case VK_PAD_DPAD_UP:
		action = ACTION_MENU_UP;
		break;
	case VK_PAD_DPAD_DOWN:
		action = ACTION_MENU_DOWN;
		break;
	case VK_PAD_LTRIGGER:
		action = ACTION_MENU_PAGEUP;
		break;
	case VK_PAD_RTRIGGER:
		action = ACTION_MENU_PAGEDOWN;
		break;
	case VK_PAD_LSHOULDER:
		action = ACTION_MENU_LEFT_SCROLL;
		break;
	case VK_PAD_RSHOULDER:
		action = ACTION_MENU_RIGHT_SCROLL;
		break;
	case VK_PAD_RTHUMB_UP:
		action =  ACTION_MENU_OTHER_STICK_UP;
		break;
	case VK_PAD_RTHUMB_DOWN:
		action =  ACTION_MENU_OTHER_STICK_DOWN;
		break;
	case VK_PAD_RTHUMB_RIGHT:
		action =  ACTION_MENU_OTHER_STICK_RIGHT;
		break;
	case VK_PAD_RTHUMB_LEFT:
		action =  ACTION_MENU_OTHER_STICK_LEFT;
		break;
	};

	return action;
}

void CXuiSceneAbstractContainer::handleSectionClick(ESceneSection eSection)
{
	CXuiCtrlSlotList *slotList = GetSectionSlotList( eSection );
	slotList->Clicked();
}

HRESULT CXuiSceneAbstractContainer::handleCustomTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	return S_OK;
}

// Returns XUI position of given scene section.
void CXuiSceneAbstractContainer::GetPositionOfSection( ESceneSection eSection, UIVec2D* pPosition )
{
	D3DXVECTOR3 xuiPos;
	GetSectionControl( eSection )->GetPosition( &xuiPos );
	pPosition->x = xuiPos.x;
	pPosition->y = xuiPos.y;
}

void CXuiSceneAbstractContainer::GetItemScreenData( ESceneSection eSection, int iItemIndex, UIVec2D* pPosition, UIVec2D* pSize )
{
	D3DXVECTOR3 xuiPos;
	if(IsSectionSlotList(eSection))
	{
		// Get slot item.
		CXuiCtrlSlotItemListItem* pCXuiCtrlSlotItem;
		GetSectionSlotList( eSection )->GetCXuiCtrlSlotItem( iItemIndex, &( pCXuiCtrlSlotItem ) );

		// Get size of slot.
		pCXuiCtrlSlotItem->GetBounds( &( pSize->x ), &( pSize->y ) );

		// Get position of slot.
		pCXuiCtrlSlotItem->GetPosition( &xuiPos );
		pPosition->x = xuiPos.x;
		pPosition->y = xuiPos.y;
	}
	else
	{
		// Get size of control
		GetSectionControl( eSection )->GetBounds( &( pSize->x ), &( pSize->y ) );

		// Get position of control
		GetSectionControl( eSection )->GetPosition( &xuiPos );
		pPosition->x = xuiPos.x;
		pPosition->y = xuiPos.y;
	}
}

shared_ptr<ItemInstance> CXuiSceneAbstractContainer::getSlotItem(ESceneSection eSection, int iSlot)
{
	CXuiCtrlSlotItemListItem* pCXuiCtrlSlotItem;
	GetSectionSlotList( eSection )->GetCXuiCtrlSlotItem( iSlot, &( pCXuiCtrlSlotItem ) );
	return pCXuiCtrlSlotItem->getItemInstance( pCXuiCtrlSlotItem->m_hObj );
}

bool CXuiSceneAbstractContainer::isSlotEmpty(ESceneSection eSection, int iSlot)
{
	CXuiCtrlSlotItemListItem* pCXuiCtrlSlotItem;
	GetSectionSlotList( eSection )->GetCXuiCtrlSlotItem( iSlot, &( pCXuiCtrlSlotItem ) );
	return pCXuiCtrlSlotItem->isEmpty( pCXuiCtrlSlotItem->m_hObj );
}

HRESULT CXuiSceneAbstractContainer::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	HRESULT hr = S_OK;

	// Update pointer from stick input on timer.
	if ( pTimer->nId == POINTER_INPUT_TIMER_ID )
    {

		onMouseTick();
		D3DXVECTOR3 pointerPos;
		pointerPos.x = m_pointerPos.x;
		pointerPos.y = m_pointerPos.y;
		pointerPos.z = 0.0f;
		m_pointerControl->SetPosition( &pointerPos );
		// This message has been dealt with, don't pass it on further.
		bHandled = TRUE;
	}
	else if ( pTimer->nId == IGNORE_KEYPRESS_TIMERID)
	{
		KillTimer(IGNORE_KEYPRESS_TIMERID);
		m_bIgnoreKeyPresses=false;
	}
	else
	{
		// Some scenes may have their own timers for other events, so handle them here
		hr = handleCustomTimer( pTimer, bHandled );
	}
	return hr;
}

HRESULT CXuiSceneAbstractContainer::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

bool CXuiSceneAbstractContainer::doesSectionTreeHaveFocus(ESceneSection eSection)
{
	return GetSectionControl( eSection )->TreeHasFocus();
}

void CXuiSceneAbstractContainer::setSectionFocus(ESceneSection eSection, int iPad)
{
	HRESULT focusResult = GetSectionControl( eSection )->SetFocus( iPad );
	assert( focusResult == S_OK );
}

void CXuiSceneAbstractContainer::setSectionSelectedSlot(ESceneSection eSection, int x, int y)
{
	GetSectionSlotList( eSection )->SetCurrentSlot(y,x);
}

void CXuiSceneAbstractContainer::setFocusToPointer(int iPad)
{
	m_pointerControl->SetFocus( iPad );
}

void CXuiSceneAbstractContainer::SetPointerText(const wstring &description, vector<wstring> &unformattedStrings, bool newSlot)
{
	if(description.empty())
	{
		m_pointerControl->SetText(L"");
		XuiElementSetShow(m_hPointerText,FALSE);
		XuiElementSetShow(m_hPointerTextBkg,FALSE);
		return;
	}

	bool smallPointer = m_bSplitscreen || (!RenderManager.IsHiDef() && !RenderManager.IsWidescreen());
	wstring desc = L"<font size=\"" + std::to_wstring(smallPointer ? 12 :14) + L"\">" + description + L"</font>";

	XUIRect tempXuiRect, xuiRect;
	HRESULT hr;
	xuiRect.right = 0;

    for (auto& it : unformattedStrings )
    {
		XuiTextPresenterMeasureText(m_hPointerTextMeasurer, parseXMLSpecials(it).c_str(), &tempXuiRect);
		if(tempXuiRect.right > xuiRect.right) xuiRect = tempXuiRect;
	}

	// Set size with the new width so that the HTML height check is correct
	XuiElementSetBounds(m_hPointerTextBkg,xuiRect.right+4.0f+4.0f+4.0f,xuiRect.bottom); // edge graphics are 8 pixels, with 4 for the border, extra 4 for the background is fudge
	XuiElementSetBounds(m_hPointerText,xuiRect.right+4.0f+4.0f,xuiRect.bottom); // edge graphics are 8 pixels, text is centred

	XuiHtmlSetText(m_hPointerText, desc.c_str() );

	// Check if we need to resize the box
	XUIContentDims contentDims;
	XuiHtmlGetContentDims(m_hPointerText,&contentDims);
	xuiRect.bottom = contentDims.nContentHeight;

	// get the size of the button, and apply the change in size due to the text to the whole button
	float fImgWidth,fImgHeight;
	XuiElementGetBounds(m_hPointerImg,&fImgWidth, &fImgHeight);
	// 4J-PB - changing to calculate values
	D3DXVECTOR3 vPosText, vPosTextBkg,vPosImg;

	XuiElementGetPosition(m_hPointerImg,&vPosImg);
	XuiElementGetPosition(m_hPointerText,&vPosText);
	XuiElementGetPosition(m_hPointerTextBkg,&vPosTextBkg);

	// Set the new height
	XuiElementSetBounds(m_hPointerTextBkg,xuiRect.right+4.0f+4.0f+4.0f,xuiRect.bottom+4.0f+4.0f); // edge graphics are 8 pixels, with 4 for the border, extra 4 for the background is fudge
	XuiElementSetBounds(m_hPointerText,xuiRect.right+4.0f+4.0f,xuiRect.bottom+4.0f+4.0f); // edge graphics are 8 pixels, text is centred

	// position the text and panel relative to the pointer image
	vPosTextBkg.x=vPosImg.x+fImgWidth*0.6f;
	vPosText.x=vPosTextBkg.x + 4;
	vPosTextBkg.y=vPosImg.y-(xuiRect.bottom+4.0f+4.0f)+fImgWidth*0.4f;
	vPosText.y=vPosTextBkg.y + 4;

	XuiElementSetPosition(m_hPointerText,&vPosText);
	XuiElementSetPosition(m_hPointerTextBkg,&vPosTextBkg);

	XuiElementSetShow(m_hPointerTextBkg,TRUE);
	XuiElementSetShow(m_hPointerText,TRUE);
}

void CXuiSceneAbstractContainer::adjustPointerForSafeZone()
{
	D3DXVECTOR2 baseSceneOrigin;
	float baseWidth, baseHeight;
	if(CXuiSceneBase::GetBaseSceneSafeZone( m_iPad, baseSceneOrigin, baseWidth, baseHeight))
	{
		D3DXMATRIX pointerBackgroundMatrix;
		XuiElementGetFullXForm( m_hPointerTextBkg, &pointerBackgroundMatrix);

		float width, height;
		XuiElementGetBounds( m_hPointerTextBkg, &width, &height );

		if( ( (pointerBackgroundMatrix._41 / pointerBackgroundMatrix._11) + width) > (baseSceneOrigin.x + baseWidth) )
		{
			//app.DebugPrintf("Pointer is outside the safe zone for this base scene\n");

			// get the size of the button, and apply the change in size due to the text to the whole button
			float fImgWidth,fImgHeight;
			XuiElementGetBounds(m_hPointerImg,&fImgWidth, &fImgHeight);
			// 4J-PB - changing to calculate values
			D3DXVECTOR3 vPosText, vPosTextBkg,vPosImg;

			XuiElementGetPosition(m_hPointerImg,&vPosImg);
			XuiElementGetPosition(m_hPointerText,&vPosText);
			XuiElementGetPosition(m_hPointerTextBkg,&vPosTextBkg);

			// position the text and panel relative to the pointer image
			vPosTextBkg.x= (vPosImg.x+fImgWidth*0.4f) - width;
			vPosText.x=vPosTextBkg.x + 4;

			XuiElementSetPosition(m_hPointerText,&vPosText);
			XuiElementSetPosition(m_hPointerTextBkg,&vPosTextBkg);
		}
	}
}