#pragma once

#include <string>
#include <XuiApp.h>

using namespace std;

class TileRenderer;
class ItemRenderer;

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftSlot class
//-----------------------------------------------------------------------------
class CXuiCtrlMinecraftSlot : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlMinecraftSlot, L"CXuiCtrlMinecraftSlot", XUI_CLASS_LABEL)

	VOID SetPassThroughDataAssociation(unsigned int iID, unsigned int iData);
	CXuiCtrlMinecraftSlot();
	virtual ~CXuiCtrlMinecraftSlot();

	void renderGuiItem(Font *font, Textures *textures,ItemInstance *item, int x, int y);
	void RenderItem();
	void SetIcon(int iPad, int iId,int iAuxVal, int iCount, int iScale, unsigned int uiAlpha,bool bDecorations,BOOL bShow, bool isFoil);
	void SetIcon(int iPad, shared_ptr<ItemInstance> item, int iScale, unsigned int uiAlpha,bool bDecorations, BOOL bShow=TRUE);

protected:

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceImage)
		XUI_ON_XM_RENDER(OnRender)
		XUI_END_MSG_MAP()

	HRESULT OnGetSourceImage(XUIMessageGetSourceImage* pData, BOOL& rfHandled);
	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnRender(XUIMessageRender *pRenderData, BOOL &rfHandled);

private:
	shared_ptr<ItemInstance> m_item;
	BOOL m_bDirty;
	INT m_iPassThroughDataAssociation;
	INT m_iPassThroughIdAssociation;
	float m_fScale,m_fAlpha;
	int m_iPad;
	int m_iID;
	int m_iCount;
	int m_iAuxVal;
	bool m_bDecorations;
	bool m_isFoil;
	int m_popTime;

	bool m_bScreenWidthSetup;
	float m_fScreenWidth,m_fScreenHeight;
	ItemRenderer *m_pItemRenderer;

	static LPCWSTR xzpIcons[15];
};