#include "stdafx.h"

#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "../../../Minecraft.World/net.minecraft.world.item.h"

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftSlot class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CXuiCtrlCraftIngredientSlot::CXuiCtrlCraftIngredientSlot() 
{
	m_iID=0;
	m_Desc=nullptr;
	m_isFoil = false;
	m_isDirty = false;
	m_item = nullptr;
}



//-----------------------------------------------------------------------------
HRESULT CXuiCtrlCraftIngredientSlot::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;
	
	return hr;
}
//-----------------------------------------------------------------------------
HRESULT CXuiCtrlCraftIngredientSlot::OnCustomMessage_GetSlotItem(CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled)
{
	if( m_iID != 0 || m_item != nullptr )
	{
		pData->item = m_item;
		pData->iItemBitField = MAKE_SLOTDISPLAY_ITEM_BITMASK(m_iID,m_iAuxVal,m_isFoil);
		pData->iDataBitField = MAKE_SLOTDISPLAY_DATA_BITMASK(m_iPad, m_uiAlpha,m_bDecorations,m_iCount,m_iScale,0);
	}
	else
	{
		pData->iDataBitField = 0;
		pData->szPath = L"";
	}
	pData->bDirty = m_isDirty ? TRUE : FALSE;
	m_isDirty = false;

	bHandled = TRUE;
	return S_OK;
}

HRESULT CXuiCtrlCraftIngredientSlot::OnGetSourceText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{	
	pGetSourceTextData->szText=m_Desc;
	bHandled = TRUE;
	
	return S_OK;
}

void CXuiCtrlCraftIngredientSlot::SetRedBox(BOOL bVal)
{
	HRESULT hr=S_OK;

	HXUIOBJ hObj,hObjChild;
	hr=GetVisual(&hObj);
	XuiElementGetChildById(hObj,L"BoxRed",&hObjChild);
	XuiElementSetShow(hObjChild,bVal);
	XuiElementGetChildById(hObj,L"Exclaim",&hObjChild);
	XuiElementSetShow(hObjChild,bVal);
}

void CXuiCtrlCraftIngredientSlot::SetIcon(int iPad, int iId,int iAuxVal, int iCount, int iScale, unsigned int uiAlpha,bool bDecorations,bool isFoil, BOOL bShow)
{
	m_item = nullptr;
	m_iID=iId;
	m_iAuxVal=iAuxVal;
	
	// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
	// rather than the dynamic one for the player
	// not right... auxvals for diggables are damage values, can be a lot higher
	if( (m_iAuxVal & 0xFF) == 0xFF && !( iId == Item::clock_Id || iId == Item::compass_Id ) ) // 4J Stu - If the aux value is set to match any
		m_iAuxVal = 0;

	// if the count comes in as 0, make it 1
	m_iCount=iCount==0?1:iCount;
	m_iScale=iScale;
	m_uiAlpha=uiAlpha;
	m_bDecorations=bDecorations;
	m_isFoil = isFoil;

	m_iPad = iPad;
	m_isDirty = true;

	XuiElementSetShow(m_hObj,bShow);
}

void CXuiCtrlCraftIngredientSlot::SetIcon(int iPad, shared_ptr<ItemInstance> item, int iScale, unsigned int uiAlpha,bool bDecorations, BOOL bShow)
{
	if(item == nullptr) SetIcon(iPad, 0,0,0,0,0,false,false,bShow);
	else
	{
		m_item = item;
		m_iID = item->id;
		m_iScale = iScale;
		m_uiAlpha = uiAlpha;
		m_bDecorations = bDecorations;

		m_iPad = iPad;
		m_isDirty = true;

		XuiElementSetShow(m_hObj,bShow);
	}
}

void CXuiCtrlCraftIngredientSlot::SetDescription(LPCWSTR Desc)
{
	HRESULT hr=S_OK;

	HXUIOBJ hObj,hObjChild;
	hr=GetVisual(&hObj);
	XuiElementGetChildById(hObj,L"text_name",&hObjChild);
	XuiControlSetText(hObjChild,Desc);
	XuiElementSetShow(hObjChild,Desc==nullptr?FALSE:TRUE);
	m_Desc=Desc;
}