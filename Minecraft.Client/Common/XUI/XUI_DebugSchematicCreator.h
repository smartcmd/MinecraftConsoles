#pragma once
#include "..\Media\xuiscene_debug_schematic_create.h"
#include "XUI_Ctrl_4JEdit.h"
#include "..\..\Common\GameRules\ConsoleSchematicFile.h"

class CScene_DebugSchematicCreator : public CXuiSceneImpl
{
#ifndef _CONTENT_PACKAGE
private:
	CXuiControl m_createButton;
	CXuiCtrl4JEdit m_name, m_startX, m_startY, m_startZ, m_endX, m_endY, m_endZ;
	CXuiCheckbox m_saveMobs, m_useXboxCompr;

	ConsoleSchematicFile::XboxSchematicInitParam *m_data;

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
		MAP_CONTROL(IDC_CreateButton, m_createButton)
		MAP_CONTROL(IDC_Name, m_name)
		MAP_CONTROL(IDC_StartX, m_startX)
		MAP_CONTROL(IDC_StartY, m_startY)
		MAP_CONTROL(IDC_StartZ, m_startZ)
		MAP_CONTROL(IDC_EndX, m_endX)
		MAP_CONTROL(IDC_EndY, m_endY)
		MAP_CONTROL(IDC_EndZ, m_endZ)
		MAP_CONTROL(IDC_SaveMobs, m_saveMobs)
		MAP_CONTROL(IDC_UseXboxCompression, m_useXboxCompr)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled);
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DebugSchematicCreator, L"CScene_DebugSchematicCreator", XUI_CLASS_SCENE )
#endif
};