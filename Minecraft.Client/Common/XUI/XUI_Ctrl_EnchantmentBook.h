#pragma once
#include "..\..\..\Minecraft.World\Random.h"

using namespace std;

class CXuiSceneEnchant;
class BookModel;

//-----------------------------------------------------------------------------
//  CXuiCtrlEnchantPanel class
//-----------------------------------------------------------------------------
class CXuiCtrlEnchantmentBook : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlEnchantmentBook, L"CXuiCtrlEnchantmentBook", XUI_CLASS_LABEL)

	CXuiCtrlEnchantmentBook();
	virtual ~CXuiCtrlEnchantmentBook();

	void setChanged();
	void setOpen(bool);

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_RENDER(OnRender)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnRender(XUIMessageRender *pRenderData, BOOL &rfHandled);

private:
	BookModel *model;
	Random random;

	// 4J JEV: Book animation variables.
	int time;
    float flip, oFlip, flipT, flipA;
    float open, oOpen;

	BOOL m_bDirty;
	float m_fScale,m_fAlpha;
	int	m_iPad;
	CXuiSceneEnchant *m_containerScene;
	shared_ptr<ItemInstance> last;

	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;

	void tickBook();
};