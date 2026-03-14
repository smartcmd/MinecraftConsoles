#include "stdafx.h"
#include "XUI_Ctrl_PassthroughList.h"

HRESULT CXuiCtrlPassThroughList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	return S_OK;
}

HRESULT CXuiCtrlPassThroughList::OnKeyDown(XUIMessageInput* pInputData, BOOL& bHandled)
{
	XUIMessage message;
	XUIMessageInput messageInput;
	HRESULT hr;
	HXUIOBJ hObj;
	
	XuiMessageInput( &message, &messageInput, XUI_KEYDOWN, pInputData->dwKeyCode, pInputData->wch, pInputData->dwFlags, pInputData->UserIndex );

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &message);

		if (message.bHandled)
		{
			bHandled = TRUE;
		}
	}

	return S_OK;
}

// Gets called every frame
HRESULT CXuiCtrlPassThroughList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{	
	XUIMessage Message;
	XUIMessageGetSourceText MsgGetText;
	HRESULT hr;
	HXUIOBJ hObj;
	

	XuiMessageGetSourceText(&Message, &MsgGetText, pGetSourceTextData->iItem, pGetSourceTextData->iData, pGetSourceTextData->bItemData);

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &Message);

		if (Message.bHandled)
		{
			pGetSourceTextData->szText = MsgGetText.szText;
			bHandled = TRUE;
		}
	}
	return S_OK;
}

// Gets called every frame
HRESULT CXuiCtrlPassThroughList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{	
	XUIMessage Message;
	XUIMessageGetSourceImage MsgGetImage;
	HRESULT hr;
	HXUIOBJ hObj;
	

	XuiMessageGetSourceImage(&Message, &MsgGetImage, pGetSourceImageData->iItem, pGetSourceImageData->iData, pGetSourceImageData->bItemData);

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &Message);

		if (Message.bHandled)
		{
			pGetSourceImageData->szPath	= MsgGetImage.szPath;
			bHandled = TRUE;
		}
	}
	return S_OK;
}

HRESULT CXuiCtrlPassThroughList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData,BOOL& bHandled)
{
	XUIMessage Message;
	XUIMessageGetItemCount MsgGetItemCountAll;
	HRESULT hr;
	HXUIOBJ hObj;
	

	XuiMessageGetItemCount(&Message, &MsgGetItemCountAll, XUI_ITEMCOUNT_ALL);

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &Message);

		if (Message.bHandled)
		{
			pGetItemCountData->cItems = MsgGetItemCountAll.cItems;
			bHandled = TRUE;
		}
	}
	bHandled = TRUE;
	return S_OK;
}