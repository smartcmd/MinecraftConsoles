#include "stdafx.h"
#include "Xbox_UIController.h"
#include "..\Common\XUI\XUI_Scene_Base.h"
#include "..\Common\XUI\XUI_Scene_Win.h"

ConsoleUIController ui;

void ConsoleUIController::init()
{
}

void ConsoleUIController::tick()
{
}

void ConsoleUIController::render()
{
}

void ConsoleUIController::StartReloadSkinThread()
{
}

bool ConsoleUIController::IsReloadingSkin()
{
	return false;
}

void ConsoleUIController::CleanUpSkinReload()
{
}

bool ConsoleUIController::NavigateToScene(int iPad, EUIScene scene, void *initData, EUILayer layer, EUIGroup group)
{
	// 4J Stu - The majority of calls to this function use the default parameters
	return app.NavigateToScene(iPad, scene, initData) == S_OK;
}

bool ConsoleUIController::NavigateBack(int iPad, bool forceUsePad, EUIScene eScene, EUILayer eLayer)
{
	return app.NavigateBack(iPad, forceUsePad, eScene) == S_OK;
}

void ConsoleUIController::NavigateToHomeMenu()
{
	app.NavigateToHomeMenu();
}

void ConsoleUIController::CloseUIScenes(int iPad, bool forceIPad)
{
	app.CloseXuiScenes(iPad,forceIPad);
}

void ConsoleUIController::CloseAllPlayersScenes()
{
	app.CloseAllPlayersXuiScenes();
}

bool ConsoleUIController::IsPauseMenuDisplayed(int iPad)
{
	return app.IsPauseMenuDisplayed(iPad);
}

bool ConsoleUIController::IsContainerMenuDisplayed(int iPad)
{
	return app.IsContainerMenuDisplayed(iPad);
}

bool ConsoleUIController::IsIgnorePlayerJoinMenuDisplayed(int iPad)
{
	return app.IsIgnorePlayerJoinMenuDisplayed(iPad);
}

bool ConsoleUIController::IsIgnoreAutosaveMenuDisplayed(int iPad)
{
	return app.IsIgnoreAutosaveMenuDisplayed(iPad);
}

void ConsoleUIController::SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed)
{
	app.SetIgnoreAutosaveMenuDisplayed(iPad, displayed);
}

bool ConsoleUIController::IsSceneInStack(int iPad, EUIScene eScene)
{
	return app.IsSceneInStack(iPad, eScene);
}

bool ConsoleUIController::GetMenuDisplayed(int iPad)
{
	return app.GetMenuDisplayed(iPad);
}

void ConsoleUIController::CheckMenuDisplayed()
{
	app.CheckMenuDisplayed();
}

void ConsoleUIController::AnimateKeyPress(int iPad, int dwKeyCode)
{
	CXuiSceneBase::AnimateKeyPress(iPad, dwKeyCode);
}

void ConsoleUIController::SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID )
{
	CXuiSceneBase::SetTooltipText(iPad,tooltip,iTextID);
}

void ConsoleUIController::SetEnableTooltips( unsigned int iPad, BOOL bVal )
{
	CXuiSceneBase::SetEnableTooltips(iPad,bVal);
}

void ConsoleUIController::ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show )
{
	CXuiSceneBase::ShowTooltip(iPad,tooltip,show);
}

void ConsoleUIController::SetTooltips( unsigned int iPad, int iA, int iB, int iX, int iY, int iLT, int iRT, int iLB, int iRB, int iLS, bool forceUpdate)
{
	CXuiSceneBase::SetTooltips(iPad,iA,iB,iX,iY,iLT,iRT,iLB,iRB,iLS,forceUpdate);
}

void ConsoleUIController::EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable )
{
	CXuiSceneBase::EnableTooltip(iPad,tooltip,enable);
}

void ConsoleUIController::RefreshTooltips(unsigned int iPad)
{
	CXuiSceneBase::RefreshTooltips(iPad);
}

void ConsoleUIController::PlayUISFX(ESoundEffect eSound)
{
	CXuiSceneBase::PlayUISFX(eSound);
}

void ConsoleUIController::DisplayGamertag(unsigned int iPad, bool show)
{
	CXuiSceneBase::DisplayGamertag(iPad, show?TRUE:FALSE);
}

void ConsoleUIController::SetSelectedItem(unsigned int iPad, const wstring &name)
{
	CXuiSceneBase::SetSelectedItem(iPad, name);
}

void ConsoleUIController::UpdateSelectedItemPos(unsigned int iPad)
{
	CXuiSceneBase::UpdateSelectedItemPos(iPad);
}

void ConsoleUIController::HandleDLCMountingComplete()
{
	// tell the xui scenes the DLC has mounted
	XUIMessage xuiMsg;
	CustomMessage_DLCMountingComplete( &xuiMsg );

	// send the message
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(i), &xuiMsg );
	}
}

void ConsoleUIController::HandleDLCInstalled(int iPad)
{
	XUIMessage xuiMsg;
	CustomMessage_DLCInstalled( &xuiMsg );

	// The DLC message should only happen in the main menus, so it should go to the default xui scenes
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	if(bNotInGame)
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(DEFAULT_XUI_MENU_USER), &xuiMsg );
	}
	else
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(iPad), &xuiMsg );
	}
}

void ConsoleUIController::HandleTMSDLCFileRetrieved(int iPad)
{
	// send a message to the scene to say we've retrieved the DLC file
	XUIMessage xuiMsg;
	CustomMessage_TMS_DLCFileRetrieved( &xuiMsg );

	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	if(bNotInGame)
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(DEFAULT_XUI_MENU_USER), &xuiMsg );
	}
	else
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(iPad), &xuiMsg );
	}
}

void ConsoleUIController::HandleTMSBanFileRetrieved(int iPad)
{
	XUIMessage xuiMsg;
	CustomMessage_TMS_BanFileRetrieved( &xuiMsg );
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);

	if(bNotInGame)
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(DEFAULT_XUI_MENU_USER), &xuiMsg );
	}
	else
	{
		XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(iPad), &xuiMsg );
	}
}

void ConsoleUIController::HandleInventoryUpdated(int iPad)
{
	XUIMessage xuiMsg;
	CustomMessage_InventoryUpdated( &xuiMsg );
	XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(iPad), &xuiMsg );
}

void ConsoleUIController::HandleGameTick()
{
	CXuiSceneBase::TickAllBaseScenes();
}

void ConsoleUIController::SetTutorialDescription(int iPad, TutorialPopupInfo *info)
{
	CScene_TutorialPopup::SetDescription(iPad, info);
}

void ConsoleUIController::SetTutorialVisible(int iPad, bool visible)
{
	CScene_TutorialPopup::SetSceneVisible( iPad, visible );
}

bool ConsoleUIController::IsTutorialVisible(int iPad)
{
	return CScene_TutorialPopup::IsSceneVisible(iPad);
}

void ConsoleUIController::UpdatePlayerBasePositions()
{
	CXuiSceneBase::UpdatePlayerBasePositions();
}

void ConsoleUIController::SetEmptyQuadrantLogo(int iSection)
{
	CXuiSceneBase::SetEmptyQuadrantLogo(iSection);
}

void ConsoleUIController::HideAllGameUIElements()
{
	CXuiSceneBase::HideAllGameUIElements();
}

void ConsoleUIController::ShowOtherPlayersBaseScene(unsigned int iPad, bool show)
{
	CXuiSceneBase::ShowOtherPlayersBaseScene(iPad, show);
}

void ConsoleUIController::ShowTrialTimer(bool show)
{
	CXuiSceneBase::ShowTrialTimer(show?TRUE:FALSE);
}

void ConsoleUIController::SetTrialTimerLimitSecs(unsigned int uiSeconds)
{
	CXuiSceneBase::m_dwTrialTimerLimitSecs = uiSeconds;
}

void ConsoleUIController::UpdateTrialTimer(unsigned int iPad)
{
	CXuiSceneBase::UpdateTrialTimer(iPad);
}

void ConsoleUIController::ReduceTrialTimerValue()
{
	CXuiSceneBase::ReduceTrialTimerValue();
}

void ConsoleUIController::ShowAutosaveCountdownTimer(bool show)
{
	CXuiSceneBase::ShowAutosaveCountdownTimer(show?TRUE:FALSE);
}

void ConsoleUIController::UpdateAutosaveCountdownTimer(unsigned int uiSeconds)
{
	CXuiSceneBase::UpdateAutosaveCountdownTimer(uiSeconds);
}

void ConsoleUIController::ShowSavingMessage(unsigned int iPad, C4JStorage::ESavingMessage eVal)
{
	CXuiSceneBase::ShowSavingMessage(iPad, eVal);
}

bool ConsoleUIController::PressStartPlaying(unsigned int iPad)
{
	return CXuiSceneBase::PressStartPlaying(iPad);
}

void ConsoleUIController::ShowPressStart(unsigned int iPad)
{
	CXuiSceneBase::ShowPressStart(iPad);
}

void ConsoleUIController::SetWinUserIndex(unsigned int iPad)
{
	CScene_Win::setWinUserIndex( iPad );
}

C4JStorage::EMessageResult ConsoleUIController::RequestMessageBox(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad,
						int( *Func)(LPVOID,int,const C4JStorage::EMessageResult),LPVOID lpParam, CXuiStringTable *pStringTable, WCHAR *pwchFormatString,DWORD dwFocusButton,bool bIsError)
{
	return StorageManager.RequestMessageBox(uiTitle, uiText, uiOptionA, uiOptionC, dwPad, Func, lpParam, pStringTable, pwchFormatString, dwFocusButton);
}

C4JStorage::EMessageResult ConsoleUIController::RequestUGCMessageBox(UINT title/* = -1 */, UINT message/* = -1 */, int iPad/* = -1*/, int( *Func)(LPVOID,int,const C4JStorage::EMessageResult)/* = NULL*/, LPVOID lpParam/* = NULL*/)
{
	// Default title / messages
	if (title == -1)
	{
		title = IDS_CONNECTION_FAILED;
	}

	if (message == -1)
	{
		message = IDS_NO_USER_CREATED_CONTENT_PRIVILEGE_SINGLE_LOCAL;
	}

	// Default pad to primary player
	if (iPad == -1) iPad = ProfileManager.GetPrimaryPad();

	UINT uiIDA[1];
	uiIDA[0]=IDS_CONFIRM_OK;
	return ui.RequestMessageBox(title, message, uiIDA, 1, iPad, Func, lpParam, app.GetStringTable(), NULL, 0, false);
}