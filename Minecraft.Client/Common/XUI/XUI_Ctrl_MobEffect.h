#pragma once
using namespace std;

#include "../../../Minecraft.World/MobEffect.h"

class CXuiCtrlMobEffect : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrlMobEffect, L"CXuiCtrlMobEffect", XUI_CLASS_CONTROL)

protected:
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
	XUI_END_MSG_MAP()

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);

public:
	void setIcon(MobEffect::EMobEffectIcon icon);
	void setName(const wstring &name);
	void setDuration(const wstring &duration);

private:
	MobEffect::EMobEffectIcon m_icon;
	wstring m_name;
	wstring m_duration;

	static LPCWSTR iconFrameNames[MobEffect::e_MobEffectIcon_COUNT];
};