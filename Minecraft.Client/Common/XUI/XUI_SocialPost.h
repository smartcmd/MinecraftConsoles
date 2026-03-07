#pragma once

#include "../media/xuiscene_socialpost.h"
#include "XUI_Ctrl_4JEdit.h"
#include "XUI_CustomMessages.h"


class CScene_SocialPost : public CXuiSceneImpl
{
	protected:
	// Control and Element wrapper objects.
	CXuiControl m_OK;
	CXuiCtrl4JEdit m_EditCaption;
	CXuiCtrl4JEdit m_EditDesc;
	CXuiControl m_text, m_LabelCaption, m_LabelDescription;
	wstring m_wTitle;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED(OnNotifyValueChanged)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)

	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		//MAP_CONTROL(IDC_XuiEditTitle, m_EditTitle)
		MAP_CONTROL(IDC_XuiLabelText, m_text)
		MAP_CONTROL(IDC_XuiLabelCaption, m_LabelCaption)
		MAP_CONTROL(IDC_XuiLabelDescription, m_LabelDescription)
		MAP_CONTROL(IDC_XuiEditCaption, m_EditCaption)
		MAP_CONTROL(IDC_XuiEditDescription, m_EditDesc)
		MAP_CONTROL(IDC_XuiOK, m_OK)
		
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged (HXUIOBJ hObjSource, XUINotifyValueChanged* pValueChangedData, BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);

	D3DXVECTOR3 m_OriginalPosition;
	int m_iPad;

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_SocialPost, L"CScene_SocialPost", XUI_CLASS_SCENE )



};
