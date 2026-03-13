#pragma once
#include "../Media/xuiscene_ingame_player_options.h"

class CScene_InGamePlayerOptions : public CXuiSceneImpl
{
private:
	enum EControls
	{
		// Checkboxes
		eControl_BuildAndMine,
		eControl_UseDoorsAndSwitches,
		eControl_UseContainers,
		eControl_AttackPlayers,
		eControl_AttackAnimals,
		eControl_Op,
		eControl_CheatTeleport,
		eControl_HostFly,
		eControl_HostHunger,
		eControl_HostInvisible,

		eControl_CHECKBOXES_COUNT,

		// Others
		eControl_Kick = eControl_CHECKBOXES_COUNT,
	};

protected:
	HXUIOBJ m_focusElement; // Only used for the remove control process

	CXuiControl m_Icon;
	CXuiControl m_Gamertag;
	CXuiScene m_TeleportGroup;
	CXuiControl m_buttonKick;
	CXuiCheckbox m_checkboxes[eControl_CHECKBOXES_COUNT];

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_CONTROL_NAVIGATE( OnControlNavigate )
		XUI_ON_XM_TRANSITION_START( OnTransitionStart )
	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_Icon, m_Icon)
		MAP_CONTROL(IDC_Gamertag, m_Gamertag)

		MAP_CONTROL(IDC_CheckboxBuildAndMine, m_checkboxes[eControl_BuildAndMine])
		MAP_CONTROL(IDC_CheckboxAttackPlayers, m_checkboxes[eControl_AttackPlayers])
		MAP_CONTROL(IDC_CheckboxAttackAnimals, m_checkboxes[eControl_AttackAnimals])
		MAP_CONTROL(IDC_CheckboxUseContainers, m_checkboxes[eControl_UseContainers])
		MAP_CONTROL(IDC_CheckboxUseDoorsAndSwitches, m_checkboxes[eControl_UseDoorsAndSwitches])
		MAP_CONTROL(IDC_CheckboxOp, m_checkboxes[eControl_Op])
		MAP_CONTROL(IDC_CheckboxTeleport, m_checkboxes[eControl_CheatTeleport])
		MAP_CONTROL(IDC_CheckboxHostInvisible, m_checkboxes[eControl_HostInvisible])
		MAP_CONTROL(IDC_CheckboxHostFly, m_checkboxes[eControl_HostFly])
		MAP_CONTROL(IDC_CheckboxHostHunger, m_checkboxes[eControl_HostHunger])

		MAP_CONTROL(IDC_ButtonKick, m_buttonKick)
	END_CONTROL_MAP()


	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnDestroy();
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled);
	HRESULT OnControlNavigate(XUIMessageControlNavigate *pControlNavigateData, BOOL& bHandled);
	HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_InGamePlayerOptions, L"CScene_InGamePlayerOptions", XUI_CLASS_SCENE )

	static void OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving);

private:
	bool m_editingSelf;
	int m_iPad;
	BYTE m_networkSmallId;
	unsigned int m_playerPrivileges;
	D3DXVECTOR3 m_OriginalPosition;

	void removeControl(HXUIOBJ hObjToRemove, bool center);

	/** 4J-JEV:
		For enabling/disabling 'Can Fly', 'Can Teleport', 'Can Disable Hunger' etc 
		used after changing the moderator checkbox.
	 */
	void resetCheatCheckboxes();

public:
	static int KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
};
