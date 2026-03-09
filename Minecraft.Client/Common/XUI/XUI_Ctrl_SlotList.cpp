#include "stdafx.h"

#include "..\..\..\Minecraft.World\AbstractContainerMenu.h"

#include "XUI_Ctrl_SlotItemListItem.h"
#include "XUI_Ctrl_SlotList.h"


//--------------------------------------------------------------------------------------
// Name: CXuiCtrlSlotList::OnInit
// Desc: Message handler for XM_INIT
//--------------------------------------------------------------------------------------
HRESULT CXuiCtrlSlotList::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	slotCount = 0;

	return S_OK;
}

HRESULT CXuiCtrlSlotList::OnDestroy()
{
	return S_OK;
}

HRESULT CXuiCtrlSlotList::OnKeyDown(XUIMessageInput *pInputData, BOOL& bHandled)
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
		hr = XuiElementGetParent( m_hObj, &parent );

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

void CXuiCtrlSlotList::SetData(int m_iPad, AbstractContainerMenu* menu, int rows, int columns, int startIndex /*= 0*/, int endIndex /*= 0*/)
{
	assert( startIndex >= 0 && startIndex < menu->getSize() );
	assert( endIndex <= menu->getSize() );

	if( startIndex < 0 )
	{
		startIndex = 0;
	}
	else if( startIndex > menu->getSize() )
	{
		startIndex = menu->getSize();
	}
	
	if( endIndex == 0 )
	{
		endIndex = startIndex + (rows * columns);
	}

	if( endIndex > menu->getSize() )
	{
		endIndex = menu->getSize();
	}

	if( startIndex > endIndex )
	{
		endIndex = startIndex;
	}

	assert( (rows * columns) == (endIndex - startIndex) );

	this->rows = rows;
	this->columns = columns;

	this->startIndex = startIndex;

	this->slotCount = rows * columns;

	InsertItems( 0, slotCount );
	
	for(int i = 0; i < slotCount; i++)
	{		
		CXuiCtrlSlotItemListItem* slotControl;
		GetCXuiCtrlSlotItem(i, &slotControl);

		slotControl->SetSlot( slotControl->m_hObj, menu->getSlot( i + startIndex ) );

		slotControl->SetUserIndex( slotControl->m_hObj,  m_iPad );

		slotControl = nullptr;
	}
}

HRESULT CXuiCtrlSlotList::OnGetItemCountAll( XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled )
{
	// We don't need to look at the type of request. The message map
	// has already filtered out a request to retrieve all items.
	pGetItemCountData->cItems = slotCount;
	bHandled = TRUE;

	return( S_OK );
}

HRESULT CXuiCtrlSlotList::OnGetItemCountMaxLines( XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled )
{
	// We don't need to look at the type of request. The message map
	// has already filtered out a request to retrieve max lines.
	pGetItemCountData->cItems = rows;
	bHandled = TRUE;

	return( S_OK );
}

HRESULT CXuiCtrlSlotList::OnGetItemCountMaxPerLine( XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled )
{
	// We don't need to look at the type of request. The message map
	// has already filtered out a request to retrieve max per line.
	pGetItemCountData->cItems = columns;
	bHandled = TRUE;

	return( S_OK );
}

int CXuiCtrlSlotList::GetCurrentColumn()
{
	int currentItemIndex = GetCurSel();

	return currentItemIndex % columns;
}

int CXuiCtrlSlotList::GetCurrentRow()
{
	int currentItemIndex = GetCurSel();

	return (currentItemIndex/columns) % rows;
}

// Return the index in the menu object
int CXuiCtrlSlotList::GetCurrentIndex()
{
	int currentSelected = GetCurSel();
	return currentSelected + this->startIndex;
}

void CXuiCtrlSlotList::SetCurrentSlot(int row, int column)
{
	if( row >= rows )
	{
		row = rows - 1;
	}
	else if ( row < 0 )
	{
		row = 0;
	}
	if( column >= columns )
	{
		column = columns - 1;
	}
	else if ( column < 0 )
	{
		column = 0;
	}
	int newSlot = ( row * columns ) + column;
	SetCurSel( newSlot );
}

void CXuiCtrlSlotList::SetEntrySlot(int row, int column, XUI_CONTROL_NAVIGATE direction)
{
	// The direction is the direction in which we are leaving the previous control to get to here
	// So a Navigate up means we want to start at the bottom of ourself
	switch( direction )
	{
	case XUI_CONTROL_NAVIGATE_UP:
		{
			row = rows - 1;
			break;
		}
	case XUI_CONTROL_NAVIGATE_DOWN:
		{
			row = 0;
			break;
		}
	case XUI_CONTROL_NAVIGATE_LEFT:
	case XUI_CONTROL_NAVIGATE_TABBACKWARD:
		{
			column = columns - 1;
			break;
		}
	case XUI_CONTROL_NAVIGATE_RIGHT:
	case XUI_CONTROL_NAVIGATE_TABFORWARD:
		{
			column = 0;
			break;
		}
	}
	SetCurrentSlot( row, column );
}

void CXuiCtrlSlotList::Clicked()
{
	CXuiCtrlSlotItemListItem* slot;
	GetCXuiCtrlSlotItem( GetCurSel() , &slot);

	// To get the press animation
	slot->Press();
}

void CXuiCtrlSlotList::GetCXuiCtrlSlotItem(int itemIndex, CXuiCtrlSlotItemListItem** CXuiCtrlSlotItem)
{
	HXUIOBJ itemControl = this->GetItemControl(itemIndex);
	VOID *pObj;
	XuiObjectFromHandle( itemControl, &pObj );
	*CXuiCtrlSlotItem = static_cast<CXuiCtrlSlotItemListItem *>(pObj);
}

