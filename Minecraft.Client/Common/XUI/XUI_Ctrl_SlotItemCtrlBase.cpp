#include "stdafx.h"

#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/Slot.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../Minecraft.h"

#include "XUI_Ctrl_SlotItemCtrlBase.h"

HRESULT CXuiCtrlSlotItemCtrlBase::OnInit( HXUIOBJ hObj, XUIMessageInit* pInitData, BOOL& bHandled )
{
	HRESULT hr = S_OK;
	SlotControlUserDataContainer* pvUserData = new SlotControlUserDataContainer();
	hr = XuiElementSetUserData(hObj, (void *)pvUserData );

	// 4J WESTY : Pointer Prototype : Added to support prototype only.
	m_bSkipDefaultNavigation = false;

	return hr;
}

HRESULT CXuiCtrlSlotItemCtrlBase::OnDestroy( HXUIOBJ hObj )
{
	HRESULT hr = S_OK;
	void* pvUserData;
	hr = XuiElementGetUserData( hObj, &pvUserData );

	if( pvUserData != NULL )
	{
		delete pvUserData;
	}

	return hr;
}

HRESULT CXuiCtrlSlotItemCtrlBase::OnCustomMessage_GetSlotItem(HXUIOBJ hObj, CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled)
{
	shared_ptr<ItemInstance> item = shared_ptr<ItemInstance>();

	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );

	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	if( pUserDataContainer->slot != NULL )
	{
		item = pUserDataContainer->slot->getItem();
	}
	else if(pUserDataContainer->m_iPad >= 0 && pUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad] );
		if(player != NULL) item = player->inventory->getCarried();
	}

	if( item != NULL )
	{
		pData->item = item;
		pData->iItemBitField = MAKE_SLOTDISPLAY_ITEM_BITMASK(item->id,item->getAuxValue(),item->isFoil());
		//int iAuxVal=item->getAuxValue();
		//int iCount = item->GetCount();
		//  8 bits - alpha
		//  1 bit - decorations on
		// 11 bits - auxval
		//  6 bits - count
		//  6 bits - scale
		pData->iDataBitField = MAKE_SLOTDISPLAY_DATA_BITMASK(pUserDataContainer->m_iPad, (int)(31*pUserDataContainer->m_fAlpha),true,item->GetCount(),7,item->popTime);
	}
	else
	{
		//pGetSourceImageData->iData = 0;
		pData->szPath = L"";
	}

	bHandled = TRUE;

	return S_OK;
}

void CXuiCtrlSlotItemCtrlBase::SetSlot( HXUIOBJ hObj, Slot* slot )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );

	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	pUserDataContainer->slot = slot;
}

void CXuiCtrlSlotItemCtrlBase::SetUserIndex(  HXUIOBJ hObj, int iPad )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );

	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	pUserDataContainer->m_iPad = iPad;
}

void CXuiCtrlSlotItemCtrlBase::SetAlpha(  HXUIOBJ hObj, float fAlpha )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );

	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	pUserDataContainer->m_fAlpha = fAlpha;
}

bool CXuiCtrlSlotItemCtrlBase::isEmpty( HXUIOBJ hObj )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	if(pUserDataContainer->slot != NULL)
	{
		return !pUserDataContainer->slot->hasItem();
	}
	else if(pUserDataContainer->m_iPad >= 0 && pUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad] );
		if(player != NULL) return player->inventory->getCarried() == NULL;

	}
	return true;
}

wstring CXuiCtrlSlotItemCtrlBase::GetItemDescription( HXUIOBJ hObj, vector<wstring> &unformattedStrings )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	if(pUserDataContainer->slot != NULL)
	{
		wstring desc = L"";
		vector<wstring> *strings = pUserDataContainer->slot->getItem()->getHoverText(Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad], false, unformattedStrings);
		bool firstLine = true;
        for ( wstring& thisString : *strings )
        {
			if(!firstLine)
			{
				desc.append( L"<br />" );
			}
			else
			{
				firstLine = false;
				wchar_t formatted[256];
				eMinecraftColour rarityColour = pUserDataContainer->slot->getItem()->getRarity()->color;
				int colour = app.GetHTMLColour(rarityColour);

				if(pUserDataContainer->slot->getItem()->hasCustomHoverName())
				{
					colour = app.GetHTMLColour(eTextColor_RenamedItemTitle);
				}

				swprintf(formatted, 256, L"<font color=\"#%08x\">%s</font>",colour,thisString.c_str());
				thisString = formatted;
			}
			desc.append( thisString );
		}
		strings->clear();
		delete strings;
		return desc;//app.GetString( pUserDataContainer->slot->getItem()->getDescriptionId() );
	}
	else if(pUserDataContainer->m_iPad >= 0 && pUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad] );
		if(player != NULL)
		{
			shared_ptr<ItemInstance> item = player->inventory->getCarried();
			if(item != NULL) return app.GetString( item->getDescriptionId() );
		}

	}
	return L"";
}

shared_ptr<ItemInstance> CXuiCtrlSlotItemCtrlBase::getItemInstance( HXUIOBJ hObj )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	if(pUserDataContainer->slot != NULL)
	{
		return pUserDataContainer->slot->getItem();
	}
	else if(pUserDataContainer->m_iPad >= 0 && pUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad] );
		if(player != NULL) return player->inventory->getCarried();

	}
	return nullptr;
}

Slot *CXuiCtrlSlotItemCtrlBase::getSlot( HXUIOBJ hObj )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	return pUserDataContainer->slot;
}

HRESULT CXuiCtrlSlotItemCtrlBase::OnKeyDown(HXUIOBJ hObj, XUIMessageInput *pInputData, BOOL& bHandled)
{
	if( pInputData->dwKeyCode == VK_PAD_DPAD_LEFT ||
		pInputData->dwKeyCode == VK_PAD_DPAD_RIGHT ||
		pInputData->dwKeyCode == VK_PAD_DPAD_UP ||
		pInputData->dwKeyCode == VK_PAD_DPAD_DOWN ||
		pInputData->dwKeyCode == VK_PAD_LTRIGGER ||
		pInputData->dwKeyCode == VK_PAD_RTRIGGER)
	{
		HXUIOBJ parent;
		HRESULT hr;
		hr = XuiElementGetParent( hObj, &parent );

		XUIMessage message;
		XUIMessageInput messageInput;

		XuiMessageInput( &message, &messageInput, XUI_KEYDOWN, pInputData->dwKeyCode, pInputData->wch, pInputData->dwFlags, pInputData->UserIndex );

		if (HRESULT_SUCCEEDED(hr))
		{
			hr = XuiBubbleMessage(parent, &message);

			if (message.bHandled)
			{
				bHandled = TRUE;
			}
		}
	}

	return S_OK;
}

// 4J WESTY : Pointer Prototype : Added to support prototype only.
HRESULT CXuiCtrlSlotItemCtrlBase::OnControlNavigate( HXUIOBJ hObj, XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled)
{
	// Skip default navigation behaviour when navigation is by pointer.
	if ( m_bSkipDefaultNavigation )
	{
		pControlNavigateData->bSkipNavigate = TRUE;
		bHandled = TRUE;
	}
	return S_OK;
}

// 4J WESTY : Pointer Prototype : Added to support prototype only.
int CXuiCtrlSlotItemCtrlBase::GetObjectCount( HXUIOBJ hObj )
{
	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	int iCount = 0;

	if(pUserDataContainer->slot != NULL)
	{
		if ( pUserDataContainer->slot->hasItem() )
		{
			iCount = pUserDataContainer->slot->getItem()->GetCount();
		}
	}
	else if(pUserDataContainer->m_iPad >= 0 && pUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pUserDataContainer->m_iPad] );
		if(player != NULL && player->inventory->getCarried() != NULL)
		{
			iCount = player->inventory->getCarried()->count;
		}

	}
	return iCount;
}


// 4J WESTY : Pointer Prototype : Added to support prototype only.
bool CXuiCtrlSlotItemCtrlBase::IsSameItemAs( HXUIOBJ hThisObj, HXUIOBJ hOtherObj )
{
	bool bThisItemExists = false;
	int iThisID = 0;
	int iThisAux = 0;

	bool bOtherItemExists = false;
	int iOtherID = 0;
	int iOtherAux = 0;

	bool bStackedByData = false;

	// Get the info on this item.
	void* pvThisUserData;
	XuiElementGetUserData( hThisObj, &pvThisUserData );
	SlotControlUserDataContainer* pThisUserDataContainer = (SlotControlUserDataContainer*)pvThisUserData;

	if(pThisUserDataContainer->slot != NULL)
	{
		if ( pThisUserDataContainer->slot->hasItem() )
		{
			iThisID = pThisUserDataContainer->slot->getItem()->id;
			iThisAux = pThisUserDataContainer->slot->getItem()->getAuxValue();
			bThisItemExists = true;
			bStackedByData = pThisUserDataContainer->slot->getItem()->isStackedByData();
		}
	}
	else if(pThisUserDataContainer->m_iPad >= 0 && pThisUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pThisUserDataContainer->m_iPad] );
		if(player != NULL && player->inventory->getCarried() != NULL)
		{
			iThisID = player->inventory->getCarried()->id;
			iThisAux = player->inventory->getCarried()->getAuxValue();
			bThisItemExists = true;
			bStackedByData = player->inventory->getCarried()->isStackedByData();
		}

	}

	// Get the info on other item.
	void* pvOtherUserData;
	XuiElementGetUserData( hOtherObj, &pvOtherUserData );
	SlotControlUserDataContainer* pOtherUserDataContainer = (SlotControlUserDataContainer*)pvOtherUserData;

	if(pOtherUserDataContainer->slot != NULL)
	{
		if ( pOtherUserDataContainer->slot->hasItem() )
		{
			iOtherID = pOtherUserDataContainer->slot->getItem()->id;
			iOtherAux = pOtherUserDataContainer->slot->getItem()->getAuxValue();
			bOtherItemExists = true;
		}
	}
	else if(pOtherUserDataContainer->m_iPad >= 0 && pOtherUserDataContainer->m_iPad < XUSER_MAX_COUNT)
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( Minecraft::GetInstance()->localplayers[pOtherUserDataContainer->m_iPad] );
		if(player != NULL && player->inventory->getCarried() != NULL)
		{
			iOtherID = player->inventory->getCarried()->id;
			iOtherAux = player->inventory->getCarried()->getAuxValue();
			bOtherItemExists = true;
		}

	}

	if ( bThisItemExists && bOtherItemExists )
	{
		return ( ( iThisID == iOtherID ) && ( (bStackedByData && iThisAux == iOtherAux) || !bStackedByData ) );
	}
	else
	{
		return false;
	}
}

// 4J WESTY : Pointer Prototype : Added to support prototype only.
// Returns number of items that can still be stacked into this slot.
int	CXuiCtrlSlotItemCtrlBase::GetEmptyStackSpace( HXUIOBJ hObj )
{
	int iResult = 0;

	void* pvUserData;
	XuiElementGetUserData( hObj, &pvUserData );
	SlotControlUserDataContainer* pUserDataContainer = (SlotControlUserDataContainer*)pvUserData;

	int iCount = 0;
	int iMaxStackSize = 0;
	bool bStackable = false;

	if(pUserDataContainer->slot != NULL)
	{
		if ( pUserDataContainer->slot->hasItem() )
		{
			bStackable = pUserDataContainer->slot->getItem()->isStackable();
			if ( bStackable )
			{
				iCount = pUserDataContainer->slot->getItem()->GetCount();
				iMaxStackSize = min(pUserDataContainer->slot->getItem()->getMaxStackSize(), pUserDataContainer->slot->getMaxStackSize() );

				iResult = iMaxStackSize - iCount;

				if(iResult < 0 ) iResult = 0;
			}
		}
	}

	return iResult;
}

