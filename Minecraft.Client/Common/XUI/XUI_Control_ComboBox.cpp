#include "stdafx.h"
#include "XUI_Control_ComboBox.h"
#include "../Xbox_App.h"

HRESULT CXuiControl4JComboBox::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	m_ListData.nItems=0;
	m_ListData.pItems=NULL;

	return S_OK;
}

void CXuiControl4JComboBox::SetData(LIST_ITEM_INFO *pItems,int iCount)
{
	CXuiControl4JComboBox *pThis;
    HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);

	// copy the data in
	pThis->m_ListData.pItems= new LIST_ITEM_INFO [iCount] ;
	memcpy(pThis->m_ListData.pItems,pItems,sizeof(LIST_ITEM_INFO)*iCount);
	pThis->m_ListData.nItems=iCount;

	//InsertItems( 0, iCount );
}

int CXuiControl4JComboBox::GetSelectedIndex()
{
	return XuiListGetCurSel(GetListObject(),NULL);
}

// Gets called every frame
HRESULT CXuiControl4JComboBox::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) ) 
	{
		pGetSourceTextData->szText = 
			m_ListData.pItems[pGetSourceTextData->iItem].pwszText;
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CXuiControl4JComboBox::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData,BOOL& bHandled)
{
	pGetItemCountData->cItems = m_ListData.nItems;
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXuiControl4JComboBox::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{
	return S_OK;

	//if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) ) 
	//{
	//	// Check for a brush

	//	if(m_ListData.pItems[pGetSourceImageData->iItem].hXuiBrush!=NULL)
	//	{
	//		pGetSourceImageData->hBrush=m_ListData.pItems[pGetSourceImageData->iItem].hXuiBrush;
	//	}
	//	else
	//	{
	//		pGetSourceImageData->szPath = 
	//			m_ListData.pItems[pGetSourceImageData->iItem].pwszImage;
	//	}
	//	bHandled = TRUE;
	//}
	//return S_OK;
}

HRESULT CXuiControl4JComboBox::OnGetItemEnable(XUIMessageGetItemEnable *pGetItemEnableData,BOOL& bHandled)
{
	if(m_ListData.pItems!=NULL && m_ListData.nItems!=0)
	{
		pGetItemEnableData->bEnabled = 
			m_ListData.pItems[pGetItemEnableData->iItem].fEnabled;
	}
	bHandled = TRUE;
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CXuiControl4JComboBox::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	CScene_Base::HandleKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);
	
	if(hObjPressed==GetValueObject())
	{
		XuiElementSetShow(GetListObject(),TRUE);
		XuiElementSetFocus(GetListObject());
		rfHandled = TRUE;
	}
	return S_OK;
}