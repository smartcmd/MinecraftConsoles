#pragma once
#include "../media/xuiscene_hud.h"
#include "XUI_CustomMessages.h"

#define CHAT_LINES_COUNT 10

class CXuiSceneHud : public CXuiSceneImpl
{
private:	
	Random m_random;	
	int m_tickCount;

protected:
	CXuiControl m_hudHolder; // Contains the HUD group to enable moving all elements together
	CXuiControl m_hudGroup; // Contains all the HUD elements except crosshair, in a group that scales
	CXuiControl m_hudScaleGroup; // Contains all the HUD elements except crosshair
	CXuiControl m_hotbarGroup;
	CXuiCtrlSlotItem *m_hotbarIcon[9];
	CXuiProgressBar m_ExperienceProgress;
	CXuiControl m_healthGroup;
	CXuiControl m_healthIcon[10];
	CXuiControl m_armourGroup;
	CXuiControl m_armourIcon[10];
	CXuiControl m_foodGroup;
	CXuiControl m_foodIcon[10];
	CXuiControl m_airGroup;
	CXuiControl m_airIcon[10];
	CXuiControl m_xpLevel;
	
	D3DXVECTOR3 m_OriginalPosition;
	int m_iPad;

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
		XUI_ON_XM_CUSTOMTICKSCENE_MESSAGE(OnCustomMessage_TickScene)
		XUI_ON_XM_DLCINSTALLED_MESSAGE(OnCustomMessage_DLCInstalled)
		XUI_ON_XM_DLCLOADED_MESSAGE(OnCustomMessage_DLCMountingComplete)
	XUI_END_MSG_MAP()
	
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_HudHolder, m_hudHolder)
		BEGIN_MAP_CHILD_CONTROLS(m_hudHolder)
			MAP_CONTROL(IDC_HudGroup, m_hudGroup)
			BEGIN_MAP_CHILD_CONTROLS(m_hudGroup)
				MAP_CONTROL(IDC_HudScaleGroup, m_hudScaleGroup)
				BEGIN_MAP_CHILD_CONTROLS(m_hudScaleGroup)
					MAP_CONTROL(IDC_Hotbar, m_hotbarGroup)
					BEGIN_MAP_CHILD_CONTROLS(m_hotbarGroup)
						MAP_OVERRIDE(IDC_Inventory1, m_hotbarIcon[0])
						MAP_OVERRIDE(IDC_Inventory2, m_hotbarIcon[1])
						MAP_OVERRIDE(IDC_Inventory3, m_hotbarIcon[2])
						MAP_OVERRIDE(IDC_Inventory4, m_hotbarIcon[3])
						MAP_OVERRIDE(IDC_Inventory5, m_hotbarIcon[4])
						MAP_OVERRIDE(IDC_Inventory6, m_hotbarIcon[5])
						MAP_OVERRIDE(IDC_Inventory7, m_hotbarIcon[6])
						MAP_OVERRIDE(IDC_Inventory8, m_hotbarIcon[7])
						MAP_OVERRIDE(IDC_Inventory9, m_hotbarIcon[8])
					END_MAP_CHILD_CONTROLS()
					MAP_CONTROL(IDC_ExperienceProgress, m_ExperienceProgress)
					MAP_CONTROL(IDC_Health, m_healthGroup)
					BEGIN_MAP_CHILD_CONTROLS(m_healthGroup)
						MAP_CONTROL(IDC_Health0, m_healthIcon[0])
						MAP_CONTROL(IDC_Health1, m_healthIcon[1])
						MAP_CONTROL(IDC_Health2, m_healthIcon[2])
						MAP_CONTROL(IDC_Health3, m_healthIcon[3])
						MAP_CONTROL(IDC_Health4, m_healthIcon[4])
						MAP_CONTROL(IDC_Health5, m_healthIcon[5])
						MAP_CONTROL(IDC_Health6, m_healthIcon[6])
						MAP_CONTROL(IDC_Health7, m_healthIcon[7])
						MAP_CONTROL(IDC_Health8, m_healthIcon[8])
						MAP_CONTROL(IDC_Health9, m_healthIcon[9])
					END_MAP_CHILD_CONTROLS()
					MAP_CONTROL(IDC_Armour, m_armourGroup)
					BEGIN_MAP_CHILD_CONTROLS(m_armourGroup)
						MAP_CONTROL(IDC_Armour0, m_armourIcon[0])
						MAP_CONTROL(IDC_Armour1, m_armourIcon[1])
						MAP_CONTROL(IDC_Armour2, m_armourIcon[2])
						MAP_CONTROL(IDC_Armour3, m_armourIcon[3])
						MAP_CONTROL(IDC_Armour4, m_armourIcon[4])
						MAP_CONTROL(IDC_Armour5, m_armourIcon[5])
						MAP_CONTROL(IDC_Armour6, m_armourIcon[6])
						MAP_CONTROL(IDC_Armour7, m_armourIcon[7])
						MAP_CONTROL(IDC_Armour8, m_armourIcon[8])
						MAP_CONTROL(IDC_Armour9, m_armourIcon[9])
					END_MAP_CHILD_CONTROLS()
					MAP_CONTROL(IDC_Food, m_foodGroup)
					BEGIN_MAP_CHILD_CONTROLS(m_foodGroup)
						MAP_CONTROL(IDC_Food0, m_foodIcon[0])
						MAP_CONTROL(IDC_Food1, m_foodIcon[1])
						MAP_CONTROL(IDC_Food2, m_foodIcon[2])
						MAP_CONTROL(IDC_Food3, m_foodIcon[3])
						MAP_CONTROL(IDC_Food4, m_foodIcon[4])
						MAP_CONTROL(IDC_Food5, m_foodIcon[5])
						MAP_CONTROL(IDC_Food6, m_foodIcon[6])
						MAP_CONTROL(IDC_Food7, m_foodIcon[7])
						MAP_CONTROL(IDC_Food8, m_foodIcon[8])
						MAP_CONTROL(IDC_Food9, m_foodIcon[9])
					END_MAP_CHILD_CONTROLS()
					MAP_CONTROL(IDC_Air, m_airGroup)
					BEGIN_MAP_CHILD_CONTROLS(m_airGroup)
						MAP_CONTROL(IDC_Air0, m_airIcon[0])
						MAP_CONTROL(IDC_Air1, m_airIcon[1])
						MAP_CONTROL(IDC_Air2, m_airIcon[2])
						MAP_CONTROL(IDC_Air3, m_airIcon[3])
						MAP_CONTROL(IDC_Air4, m_airIcon[4])
						MAP_CONTROL(IDC_Air5, m_airIcon[5])
						MAP_CONTROL(IDC_Air6, m_airIcon[6])
						MAP_CONTROL(IDC_Air7, m_airIcon[7])
						MAP_CONTROL(IDC_Air8, m_airIcon[8])
						MAP_CONTROL(IDC_Air9, m_airIcon[9])
					END_MAP_CHILD_CONTROLS()
					MAP_CONTROL(IDC_XPLevel, m_xpLevel)
				END_MAP_CHILD_CONTROLS()
			END_MAP_CHILD_CONTROLS()
		END_MAP_CHILD_CONTROLS()
	END_CONTROL_MAP()	

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);
	HRESULT OnCustomMessage_TickScene();
	HRESULT OnCustomMessage_DLCInstalled();
	HRESULT OnCustomMessage_DLCMountingComplete();
public:
	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CXuiSceneHud, L"CXuiSceneHud", XUI_CLASS_SCENE )
};