#include "stdafx.h"
#include "XUI_Ctrl_MobEffect.h"

LPCWSTR CXuiCtrlMobEffect::iconFrameNames[MobEffect::e_MobEffectIcon_COUNT]=
{
	L"Normal",
	L"Blindness",
	L"Fire_Resistance",
	L"Haste",
	L"Hunger",
	L"Invisibility",
	L"Jump_Boost",
	L"Mining_Fatigue",
	L"Nausea",
	L"Night_Vision",
	L"Poison",
	L"Regeneration",
	L"Resistance",
	L"Slowness",
	L"Speed",
	L"Strength",
	L"Water_Breathing",
	L"Weakness",
};

HRESULT CXuiCtrlMobEffect::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	m_icon = MobEffect::e_MobEffectIcon_None;
	m_name = L"";
	m_duration = L"";
	return S_OK;
}

HRESULT CXuiCtrlMobEffect::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled)
{
	if( pGetSourceTextData->iData == 1 )
	{
		pGetSourceTextData->szText = m_name.c_str();
		pGetSourceTextData->bDisplay = TRUE;

		if(FAILED(PlayVisualRange(iconFrameNames[m_icon],nullptr,iconFrameNames[m_icon])))
		{
			PlayVisualRange(L"Normal",nullptr,L"Normal");
		}

		bHandled = TRUE;
	}
	else if( pGetSourceTextData->iData == 2 )
	{
		pGetSourceTextData->szText = m_duration.c_str();
		pGetSourceTextData->bDisplay = TRUE;

		bHandled = TRUE;
	}
	return S_OK;
}

void CXuiCtrlMobEffect::setIcon(MobEffect::EMobEffectIcon icon)
{
	m_icon = icon;
}

void CXuiCtrlMobEffect::setName(const wstring &name)
{
	m_name = name;
}

void CXuiCtrlMobEffect::setDuration(const wstring &duration)
{
	m_duration = duration;
}