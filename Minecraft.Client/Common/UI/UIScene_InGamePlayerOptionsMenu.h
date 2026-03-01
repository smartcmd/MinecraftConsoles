#pragma once

#include "UIScene.h"

class UIScene_InGamePlayerOptionsMenu : public UIScene
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

	bool m_bShouldNavBack;
	bool m_editingSelf;
	BYTE m_networkSmallId;
	unsigned int m_playerPrivileges;
	
	UIControl_Label m_labelGamertag;
	UIControl_CheckBox m_checkboxes[eControl_CHECKBOXES_COUNT];
	UIControl_Button m_buttonKick;
	IggyName m_funcSetPlayerIcon;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_checkboxes[eControl_BuildAndMine], "CheckboxBuildAndMine")
		UI_MAP_ELEMENT( m_checkboxes[eControl_UseDoorsAndSwitches], "CheckboxUseDoorsAndSwitches")
		UI_MAP_ELEMENT( m_checkboxes[eControl_UseContainers], "CheckboxUseContainers")
		UI_MAP_ELEMENT( m_checkboxes[eControl_AttackPlayers], "CheckboxAttackPlayers")
		UI_MAP_ELEMENT( m_checkboxes[eControl_AttackAnimals], "CheckboxAttackAnimals")
		UI_MAP_ELEMENT( m_checkboxes[eControl_Op], "CheckboxOp")
		UI_MAP_ELEMENT( m_checkboxes[eControl_CheatTeleport], "CheckboxTeleport")
		UI_MAP_ELEMENT( m_checkboxes[eControl_HostFly], "CheckboxHostFly")
		UI_MAP_ELEMENT( m_checkboxes[eControl_HostHunger], "CheckboxHostHunger")
		UI_MAP_ELEMENT( m_checkboxes[eControl_HostInvisible], "CheckboxHostInvisible")

		UI_MAP_ELEMENT( m_buttonKick, "ButtonKick")

		UI_MAP_ELEMENT( m_labelGamertag, "Gamertag")

		UI_MAP_NAME( m_funcSetPlayerIcon, L"SetPlayerIcon" );
	UI_END_MAP_ELEMENTS_AND_NAMES()

		bool m_bModeratorState;


public:
	UIScene_InGamePlayerOptionsMenu(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_InGamePlayerOptionsMenu;}
	virtual void updateTooltips();

	virtual void handleReload();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();
	virtual void handleCheckboxToggled(F64 controlId, bool selected);
	virtual void handleTimerComplete(int id);

public:
	virtual void tick();

	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleDestroy();
	virtual void handlePress(F64 controlId, F64 childId);


	static int KickPlayerReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static void OnPlayerChanged(void *callbackParam, INetworkPlayer *pPlayer, bool leaving);

private:
		/** 4J-JEV:
		For enabling/disabling 'Can Fly', 'Can Teleport', 'Can Disable Hunger' etc 
		used after changing the moderator checkbox.
	 */
	void resetCheatCheckboxes();
};