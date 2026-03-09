#include "stdafx.h"
#include "XUI_Ctrl_SliderWrapper.h"

#define NO_SOUND_TIMER 0

HRESULT CXuiCtrlSliderWrapper::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	VOID *pObj;
	HXUIOBJ hObjChild;

	XuiElementGetChildById(m_hObj,L"FocusSink",&hObjChild);
	XuiObjectFromHandle( hObjChild, &pObj );
	m_pFocusSink = static_cast<CXuiControl *>(pObj);

	XuiElementGetChildById(m_hObj,L"XuiSlider",&hObjChild);
	XuiObjectFromHandle( hObjChild, &pObj );
	m_pSlider = static_cast<CXuiSlider *>(pObj);

	m_sliderActive = false;
	m_bDisplayVal=true;
	m_bPlaySound=false; // make this false to avoid a sound being played in the first setting of the slider value in a scene
	XuiSetTimer( m_hObj,NO_SOUND_TIMER,50);
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXuiCtrlSliderWrapper::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

// 	switch(pInputData->dwKeyCode)
// 	{

// 	case VK_PAD_A:
// 		// 4J-PB - IGNORE !
// 		if(m_sliderActive)
// 		{
// 			m_pFocusSink->SetFocus(pInputData->UserIndex);
// 			m_sliderActive = false;
// 		}
// 		else
// 		{
// 			m_pSlider->SetFocus(pInputData->UserIndex);
// 			m_sliderActive = true;
// 		}
// 		rfHandled = TRUE;
// 
// 		break;
// 	default:
// 		m_pSlider->SetFocus(pInputData->UserIndex);
// 		m_sliderActive = false;
// 		break;
// 
// 	}
// 
	return S_OK;
}

HRESULT CXuiCtrlSliderWrapper::OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled)
{
	XUIMessage Message;
	XUINotify Notify;
	XUINotifyValueChanged MsgValueChanged;
	HRESULT hr;
	HXUIOBJ hObj;
	
	if(m_bPlaySound)
	{
		m_bPlaySound=false;
		CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		XuiSetTimer( m_hObj,NO_SOUND_TIMER,150);
	}

	//app.DebugPrintf("Slider val changed - %d\n",pValueChangedData->nValue);

	XuiNotifyValueChanged(&Message,&Notify,&MsgValueChanged,hObjSource,pValueChangedData->nValue);

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &Message);
		rfHandled = TRUE;
	}
	return S_OK;
}


HRESULT CXuiCtrlSliderWrapper::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==NO_SOUND_TIMER)
	{
		XuiKillTimer(m_hObj,NO_SOUND_TIMER);
		m_bPlaySound=true;
	}

	return S_OK;
}

HRESULT CXuiCtrlSliderWrapper::SetValue( int nValue )
{
	CXuiCtrlSliderWrapper *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;

	pThis->m_pSlider->SetValue(nValue);
	return S_OK;
}

HRESULT CXuiCtrlSliderWrapper::SetValueDisplay( BOOL bShow )
{
	CXuiCtrlSliderWrapper *pThis;
	HXUIOBJ hVisual,hText;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;

	hr=XuiControlGetVisual(pThis->m_pSlider->m_hObj,&hVisual);
	hr=XuiElementGetChildById(hVisual,L"Text_Value",&hText);

	if(hText!=nullptr)
	{
		XuiElementSetShow(hText,bShow);
	}

	return S_OK;
}

LPCWSTR CXuiCtrlSliderWrapper::GetText(  )
{
	CXuiCtrlSliderWrapper *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return nullptr;
	return pThis->m_pSlider->GetText();
	//return S_OK;
}

HRESULT CXuiCtrlSliderWrapper::SetText(LPCWSTR text  , int iDataAssoc)
{
	CXuiCtrlSliderWrapper *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;

	// if there's a data assoc value, find the right control for it
	if(iDataAssoc!=0)
	{

	}

	pThis->m_pSlider->SetText(text);
	return hr;
}

HXUIOBJ CXuiCtrlSliderWrapper::GetSlider()
{
	CXuiCtrlSliderWrapper *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return nullptr;
	return pThis->m_pSlider->m_hObj;
}

HRESULT CXuiCtrlSliderWrapper::SetRange( int nRangeMin, int nRangeMax)
{
	CXuiCtrlSliderWrapper *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	pThis->m_pSlider->SetRange(nRangeMin, nRangeMax);
	return S_OK;
}