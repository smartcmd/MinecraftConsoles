#pragma once

#include <string>
#include <xuiapp.h>

using namespace std;

class TileRenderer;
class ItemRenderer;
class CXuiSceneInventory;

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftPlayer class
//-----------------------------------------------------------------------------
class CXuiCtrlMinecraftPlayer : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlMinecraftPlayer, L"CXuiCtrlMinecraftPlayer", XUI_CLASS_LABEL)

	CXuiCtrlMinecraftPlayer();
	virtual ~CXuiCtrlMinecraftPlayer() { };

protected:

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_RENDER(OnRender)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnRender(XUIMessageRender *pRenderData, BOOL &rfHandled);

private:
	BOOL m_bDirty;
	float m_fScale,m_fAlpha;
	int	m_iPad;
	CXuiSceneInventory *m_containerScene;

	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;

};