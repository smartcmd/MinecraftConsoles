#include  "stdafx.h"
#include "XUI_Ctrl_4JEdit.h"



HRESULT CXuiCtrl4JEdit::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;

	// set a limit for the text box
	m_uTextLimit=XUI_4JEDIT_MAX_CHARS-1;
	XuiEditSetTextLimit(m_hObj,m_uTextLimit);
	//  Find the text limit. (Add one for nullptr terminator)
	//m_uTextLimit = min( XuiEditGetTextLimit(m_hObj) + 1, XUI_4JEDIT_MAX_CHARS);

	ZeroMemory( wchText , sizeof(WCHAR)*(m_uTextLimit+1) );

	m_bReadOnly = false;
	m_uiTitle = 0;
	m_uiText =0;
	m_eKeyboardMode=C_4JInput::EKeyboardMode_Default;
	return hr;
}


HRESULT CXuiCtrl4JEdit::SetTextLimit(int iLimit)
{  
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	if(iLimit<XUI_4JEDIT_MAX_CHARS)
	{
		pThis->m_uTextLimit=iLimit;
		XuiEditSetTextLimit(pThis->m_hObj,iLimit);
		ZeroMemory( pThis->wchText , sizeof(WCHAR)*XUI_4JEDIT_MAX_CHARS );
	}
	return S_OK;
}
HRESULT CXuiCtrl4JEdit::SetCaretPosition(int iPos)
{  
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	XuiEditSetCaretPosition(pThis->m_hObj,iPos);
	return S_OK;
}

HRESULT CXuiCtrl4JEdit::SetTitleAndText(unsigned int uiTitle, unsigned int uiText)
{   
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	pThis->m_uiTitle=uiTitle;
	pThis->m_uiText=uiText;

	return S_OK;
}

HRESULT CXuiCtrl4JEdit::SetReadOnly(bool bReadOnly)
{       
	// Attempt to make the change on the actual original version of this object that XUI made itself, rather
	// than the copy we make ourselves and then map to it, which shares the same handle
	CXuiCtrl4JEdit *pThis;
    HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
    if (FAILED(hr))
        return hr;
	pThis->m_bReadOnly = bReadOnly;

	return S_OK;
}

HRESULT CXuiCtrl4JEdit::SetKeyboardType(C_4JInput::EKeyboardMode eKeyboardMode)
{
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	pThis->m_eKeyboardMode= eKeyboardMode;
	return S_OK;
}

// HRESULT CXuiCtrl4JEdit::SetIPMode(bool bIPMode)
// {       
// 	// Attempt to make the change on the actual original version of this object that XUI made itself, rather
// 	// than the copy we make ourselves and then map to it, which shares the same handle
// 	CXuiCtrl4JEdit *pThis;
//     HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
//     if (FAILED(hr))
//         return hr;
// 	pThis->m_bIPMode= bIPMode;
// 
// 	return S_OK;
// }

// HRESULT CXuiCtrl4JEdit::SetExtendedMode(bool bExtendedMode)
// {       
// 	// Attempt to make the change on the actual original version of this object that XUI made itself, rather
// 	// than the copy we make ourselves and then map to it, which shares the same handle
// 	CXuiCtrl4JEdit *pThis;
//     HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
//     if (FAILED(hr))
//         return hr;
// 	pThis->m_bExtendedMode= bExtendedMode;
// 
// 	return S_OK;
// }

HRESULT CXuiCtrl4JEdit::OnChar(XUIMessageChar* pInputData, BOOL& rfHandled)
{
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;

	HXUIOBJ hBaseObj;

	// need to send the key down to the base object, so that when we notify the parent, the edit control has been updated with the right text
	hr=XuiGetBaseObject(pThis->m_hObj,&hBaseObj);

	XUIMessage xuiMsg;
	XUIMessageChar xuiMsgChar;
	XuiMessageChar( &xuiMsg, &xuiMsgChar, pInputData->wch, pInputData->dwFlags, pInputData->UserIndex );
	// Send the XM_CHAR message.
	XuiSendMessage( hBaseObj, &xuiMsg );

 	rfHandled = TRUE;
 	SendNotifyValueChanged(static_cast<int>(pInputData->wch));

	return hr;
}    

HRESULT CXuiCtrl4JEdit::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	//HRESULT hr = S_OK;

	if( pThis->m_bReadOnly ) return hr;

	//  Find the text limit. (Add one for nullptr terminator)
	//m_uTextLimit = min( XuiEditGetTextLimit(m_hObj) + 1, XUI_4JEDIT_MAX_CHARS);

	if((((pInputData->dwKeyCode == VK_PAD_A) && (pInputData->wch == 0)) || (pInputData->dwKeyCode == VK_PAD_START)) && !(pInputData->dwFlags & XUI_INPUT_FLAG_REPEAT))
	{
		pThis->RequestKeyboard(pInputData->UserIndex);
		rfHandled = TRUE;
	}

	return hr;
}

void CXuiCtrl4JEdit::RequestKeyboard(int iPad)
{
	InputManager.RequestKeyboard(m_uiTitle,GetText(),m_uiText,iPad,wchText,m_uTextLimit+1,&CXuiCtrl4JEdit::KeyboardReturned,this,m_eKeyboardMode,app.GetStringTable());
}


//-----------------------------------------------------------------------------
HRESULT CXuiCtrl4JEdit::SendNotifyValueChanged(int iValue)
{
	CXuiCtrl4JEdit *pThis;
	HRESULT hr = XuiObjectFromHandle(m_hObj, (void **) &pThis);
	if (FAILED(hr))
		return hr;
	HXUIOBJ hParent;
	//HRESULT hr=S_OK;
	XUIMessage msg;
	XUINotify msgNotify;
	XUINotifyValueChanged msgNotifyValueChanged;

	XuiElementGetParent(pThis->m_hObj, &hParent);
	XuiNotifyValueChanged(&msg, &msgNotify, &msgNotifyValueChanged, XuiGetOuter(pThis->m_hObj), iValue);        
	XuiBubbleMessage(XuiGetOuter(hParent), &msg);

	return hr;
}        

int CXuiCtrl4JEdit::KeyboardReturned(void *pParam,bool bSet)
{
	CXuiCtrl4JEdit* pClass = static_cast<CXuiCtrl4JEdit *>(pParam);
	HRESULT hr = S_OK;

	if(bSet)
	{
		pClass->SetText(pClass->wchText);
		// need to move the caret to the end of the newly set text
		XuiEditSetCaretPosition(pClass->m_hObj, static_cast<int>(wcsnlen(pClass->wchText, 50)));
		pClass->SendNotifyValueChanged(10); // 10 for a return
	}

	return hr;
}
