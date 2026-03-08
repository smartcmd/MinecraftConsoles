#pragma once
#include "../Media/xuiscene_text_entry.h"
#include "XUI_Ctrl_4JEdit.h"


class CScene_TextEntry : public CXuiSceneImpl
{
public:
	typedef struct _XuiTextInputParams
	{
		int iPad;
		WCHAR wch;
	} 
	XuiTextInputParams;

	typedef struct _CommamndParams
	{
		WCHAR wchCommand[40];
		WCHAR wchFormat[40];
	} 
	CommandParams;

	enum 
	{
		eCommand_Teleport=0,
		eCommand_Give,
		eCommand_MAX
	}
	eCommands;

protected:
	CXuiCtrl4JEdit m_EditText;


	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
		XUI_ON_XM_KEYDOWN(OnKeyDown)

	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiEditText, m_EditText)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);

	HRESULT InterpretString(wstring &wsText);
	HRESULT Init_Commands();
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_TextEntry, L"CScene_TextEntry", XUI_CLASS_SCENE )

private:
	int m_iPad;
	WCHAR m_wchInitialChar;
	static CommandParams CommandA[eCommand_MAX];
	unordered_map<wstring, int> m_CommandSet;
};