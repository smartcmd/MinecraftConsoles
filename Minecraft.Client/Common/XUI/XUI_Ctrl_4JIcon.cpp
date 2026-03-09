#include "stdafx.h"
#include "XUI_Ctrl_4JIcon.h"

HRESULT CXuiCtrl4JIcon::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	m_hBrush=nullptr;
	return S_OK;
}

HRESULT CXuiCtrl4JIcon::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{	
	XUIMessage Message;
	XUIMessageGetSourceImage MsgGetImage;
	HRESULT hr;
	HXUIOBJ hObj;

	if(m_hBrush)
	{
		pGetSourceImageData->hBrush	= m_hBrush;
		bHandled = TRUE;
	}
	else
	{
		XuiMessageGetSourceImage(&Message, &MsgGetImage, pGetSourceImageData->iItem, pGetSourceImageData->iData, TRUE);

		hr = GetParent(&hObj);

		if (HRESULT_SUCCEEDED(hr))
		{
			hr = XuiBubbleMessage(hObj, &Message);

			if (Message.bHandled)
			{
				pGetSourceImageData->hBrush	= MsgGetImage.hBrush;
				bHandled = TRUE;
			}
		}
	}
	return S_OK;
}

HRESULT CXuiCtrl4JIcon::UseBrush(HXUIBRUSH hBrush)
{
	if( m_hBrush )
	{
		XuiDestroyBrush( m_hBrush );
	}
	m_hBrush = hBrush;
	return XuiControlSetImageBrush(m_hObj,hBrush);
}

HRESULT CXuiCtrl4JIcon::OnDestroy()
{

	if( m_hBrush )
	{
		XuiDestroyBrush( m_hBrush );
	}

	return S_OK;
}