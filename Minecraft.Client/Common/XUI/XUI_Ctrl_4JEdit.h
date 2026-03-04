#pragma once

#include <xuiapp.h>

#define XUI_4JEDIT_MAX_CHARS 61

class CXuiCtrl4JEdit : public CXuiControlImpl
{
public:
	XUI_IMPLEMENT_CLASS(CXuiCtrl4JEdit, L"CXuiCtrl4JEdit", XUI_CLASS_EDIT)

protected:


	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT(OnInit)
		XUI_ON_XM_CHAR(OnChar)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_END_MSG_MAP()        

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& rfHandled);
	HRESULT OnChar(XUIMessageChar* pInputData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);	
public:
	HRESULT SetReadOnly(bool bReadOnly);
// 	HRESULT SetIPMode(bool bIPMode);
// 	HRESULT SetExtendedMode(bool bExtendedMode);
	HRESULT SetKeyboardType(C_4JInput::EKeyboardMode eKeyboardMode);
	HRESULT SetTextLimit(int iLimit);
	HRESULT SetCaretPosition(int iPos);
	HRESULT SetTitleAndText(unsigned int uiTitle, unsigned int uiText);

	void RequestKeyboard(int iPad);
protected:
	bool m_bReadOnly;
	C_4JInput::EKeyboardMode m_eKeyboardMode;
	unsigned int m_uiTitle,m_uiText;

private:    
	static int KeyboardReturned(void *pParam,bool bSet);
	HRESULT SendNotifyValueChanged(int);
	WCHAR wchText[XUI_4JEDIT_MAX_CHARS];
	unsigned int m_uTextLimit;
};
