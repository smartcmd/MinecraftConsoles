#pragma once

#include <string>
#include <xuiapp.h>

using namespace std;

//-----------------------------------------------------------------------------
//  CXuiCtrlSplashPulser class
//-----------------------------------------------------------------------------
class CXuiCtrlSplashPulser : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlSplashPulser, L"CXuiCtrlSplashPulser", XUI_CLASS_LABEL)

	CXuiCtrlSplashPulser();
	virtual ~CXuiCtrlSplashPulser() { };

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

	float m_fScreenWidth,m_fScreenHeight;
	float m_fRawWidth,m_fRawHeight;

};