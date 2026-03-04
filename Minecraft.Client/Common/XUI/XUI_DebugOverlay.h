#pragma once
using namespace std;
#include "../media/xuiscene_debugoverlay.h"

#define DEBUG_OVERLAY_UPDATE_TIME_PERIOD 10000

class RegionFile;
class DataOutputStream;
class ConsoleSaveFile;
#include "..\..\..\Minecraft.World\File.h"
#include "..\..\..\Minecraft.World\Entity.h"

class CScene_DebugOverlay : public CXuiSceneImpl
{
#ifdef _DEBUG_MENUS_ENABLED	
private:
	CXuiList m_items, m_mobs, m_enchantments;
	CXuiControl m_resetTutorial, m_createSchematic, m_toggleRain, m_toggleThunder, m_setCamera;
	CXuiControl m_setDay, m_setNight;
	CXuiSlider m_chunkRadius, m_setTime,m_setFov;
	vector<int> m_itemIds;
	vector<eINSTANCEOF> m_mobFactories;
	vector<int> m_enchantmentIds;

protected:
	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_NOTIFY_PRESS_EX( OnNotifyPressEx )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_VALUE_CHANGED( OnNotifyValueChanged )
		XUI_ON_XM_TIMER( OnTimer )
	XUI_END_MSG_MAP()

	// Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_ChunkRadius, m_chunkRadius)
		MAP_CONTROL(IDC_ResetTutorial, m_resetTutorial)
		MAP_CONTROL(IDC_CreateSchematic, m_createSchematic)
		MAP_CONTROL(IDC_ToggleRain, m_toggleRain)
		MAP_CONTROL(IDC_ToggleThunder, m_toggleThunder)
		MAP_CONTROL(IDC_SetDay, m_setDay)
		MAP_CONTROL(IDC_SetNight, m_setNight)
		MAP_CONTROL(IDC_SliderTime, m_setTime)
		MAP_CONTROL(IDC_SliderFov, m_setFov)
		MAP_CONTROL(IDC_MobList, m_mobs)
		MAP_CONTROL(IDC_EnchantmentsList, m_enchantments)
		MAP_CONTROL(IDC_ItemsList, m_items)
		MAP_CONTROL(IDC_SetCamera, m_setCamera)
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled);
	HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_DebugOverlay, L"CScene_DebugOverlay", XUI_CLASS_SCENE )

private:
	void SetSpawnToPlayerPos();
#ifndef _CONTENT_PACKAGE
	void SaveLimitedFile(int chunkRadius);
#endif	
	RegionFile *getRegionFile(unordered_map<File, RegionFile *, FileKeyHash, FileKeyEq> &newFileCache, ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ);

	DataOutputStream *getChunkDataOutputStream(unordered_map<File, RegionFile *, FileKeyHash, FileKeyEq> &newFileCache, ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ);
#endif
};