#pragma once

#include <string>
#include <XuiApp.h>


//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftSlot class
//-----------------------------------------------------------------------------
class CXuiCtrlCraftIngredientSlot : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlCraftIngredientSlot, L"CXuiCtrlCraftIngredientSlot", XUI_CLASS_LABEL)

	CXuiCtrlCraftIngredientSlot();
	virtual ~CXuiCtrlCraftIngredientSlot() { };
	void SetRedBox(BOOL bVal);
	void SetIcon(int iPad, int iId,int iAuxVal, int iCount, int iScale, unsigned int uiAlpha, bool bDecorations, bool isFoil = false, BOOL bShow=TRUE);
	void SetIcon(int iPad, shared_ptr<ItemInstance> item, int iScale, unsigned int uiAlpha,bool bDecorations, BOOL bShow=TRUE);
	void SetDescription(LPCWSTR Desc);
protected:

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GETSLOTITEM_MESSAGE(OnCustomMessage_GetSlotItem)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceText)
	XUI_END_MSG_MAP()

	HRESULT OnCustomMessage_GetSlotItem(CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled);
	HRESULT OnGetSourceText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);

private:
	shared_ptr<ItemInstance> m_item;
	int m_iID;
	int m_iAuxVal;
	int m_iCount;
	int m_iScale;
	unsigned int m_uiAlpha;
	int m_iPad;
	bool m_bDecorations;
	bool m_isFoil;
	LPCWSTR m_Desc;
	bool m_isDirty;
};