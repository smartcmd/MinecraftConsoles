#pragma once
#include "../Media/xuiscene_debug_set_camera.h"
#include "XUI_Ctrl_4JEdit.h"
#include "../../Common/GameRules/ConsoleSchematicFile.h"

class CScene_DebugSetCamera : public CXuiSceneImpl
{
public:
	typedef struct _FreezePlayerParam
	{
		int player;
		bool freeze;
	} FreezePlayerParam;

#ifndef _CONTENT_PACKAGE
private:
	CXuiCtrl4JEdit m_camX, m_camY, m_camZ, m_yRot, m_elevation;
	CXuiCheckbox m_lockPlayer;
	CXuiControl m_teleport;

	DebugSetCameraPosition *currentPosition;
	FreezePlayerParam *fpp;

protected:
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX( OnNotifyPressEx )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_CamX, m_camX)
		MAP_CONTROL(IDC_CamY, m_camY)
		MAP_CONTROL(IDC_CamZ, m_camZ)
		MAP_CONTROL(IDC_YRot, m_yRot)
		MAP_CONTROL(IDC_Elevation, m_elevation)
		MAP_CONTROL(IDC_LockPlayer, m_lockPlayer)
		MAP_CONTROL(IDC_Teleport, m_teleport)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DebugSetCamera, L"CScene_DebugSetCamera", XUI_CLASS_SCENE )
#endif
};