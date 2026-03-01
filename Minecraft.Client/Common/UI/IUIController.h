#pragma once

#include "UIEnums.h"

// 4J Stu - An interface class that defines all the public functions that we use within the game code. This allows us to build the Xbox 360 version without
// using the base UIController class used by the other platforms
class IUIController
{
public:
	virtual void tick() = 0;
	virtual void render() = 0;
	virtual void StartReloadSkinThread() = 0;
	virtual bool IsReloadingSkin() = 0;
	virtual void CleanUpSkinReload() = 0;
	virtual bool NavigateToScene(int iPad, EUIScene scene, void *initData = NULL, EUILayer layer = eUILayer_Scene, EUIGroup group = eUIGroup_PAD) = 0;
	virtual bool NavigateBack(int iPad, bool forceUsePad = false, EUIScene eScene = eUIScene_COUNT, EUILayer eLayer = eUILayer_COUNT) = 0;
	virtual void CloseUIScenes(int iPad, bool forceIPad = false) = 0;
	virtual void CloseAllPlayersScenes() = 0;

	virtual bool IsPauseMenuDisplayed(int iPad)	= 0;
	virtual bool IsContainerMenuDisplayed(int iPad) = 0;
	virtual bool IsIgnorePlayerJoinMenuDisplayed(int iPad) = 0;
	virtual bool IsIgnoreAutosaveMenuDisplayed(int iPad) = 0;
	virtual void SetIgnoreAutosaveMenuDisplayed(int iPad, bool displayed) = 0;
	virtual bool IsSceneInStack(int iPad, EUIScene eScene) = 0;
	virtual bool GetMenuDisplayed(int iPad) = 0;
	virtual void CheckMenuDisplayed() = 0;

	virtual void SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID ) = 0;
	virtual void SetEnableTooltips( unsigned int iPad, BOOL bVal ) = 0;
	virtual void ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show ) = 0;
	virtual void SetTooltips( unsigned int iPad, int iA, int iB=-1, int iX=-1, int iY=-1 , int iLT=-1, int iRT=-1, int iLB=-1, int iRB=-1, int iLS=-1, int iRS=-1, int iBack=-1, bool forceUpdate = false) = 0;
	virtual void EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable ) = 0;
	virtual void RefreshTooltips(unsigned int iPad) = 0;

	virtual void PlayUISFX(ESoundEffect eSound) = 0;

	virtual void ShowUIDebugConsole(bool show) {}
	virtual void ShowUIDebugMarketingGuide(bool show) {}

	virtual void DisplayGamertag(unsigned int iPad, bool show) = 0;
	virtual void SetSelectedItem(unsigned int iPad, const wstring &name) = 0;
	virtual void UpdateSelectedItemPos(unsigned int iPad) = 0;

	virtual void HandleDLCMountingComplete() = 0;
	virtual void HandleDLCInstalled(int iPad) = 0;
#ifdef _XBOX_ONE
	virtual void HandleDLCLicenseChange() = 0;
#endif
	virtual void HandleTMSDLCFileRetrieved(int iPad) = 0;
	virtual void HandleTMSBanFileRetrieved(int iPad) = 0;
	virtual void HandleInventoryUpdated(int iPad) = 0;
	virtual void HandleGameTick() = 0;

	virtual void SetTutorialDescription(int iPad, TutorialPopupInfo *info) = 0;
	virtual void SetTutorialVisible(int iPad, bool visible) = 0;
	virtual bool IsTutorialVisible(int iPad) = 0;

	virtual void UpdatePlayerBasePositions() = 0;
	virtual void SetEmptyQuadrantLogo(int iSection) = 0;
	virtual void HideAllGameUIElements() = 0;
	virtual void ShowOtherPlayersBaseScene(unsigned int iPad, bool show) = 0;

	virtual void ShowTrialTimer(bool show) = 0;
	virtual void SetTrialTimerLimitSecs(unsigned int uiSeconds) = 0;
	virtual void UpdateTrialTimer(unsigned int iPad) = 0;
	virtual void ReduceTrialTimerValue() = 0;

	virtual void ShowAutosaveCountdownTimer(bool show) = 0;
	virtual void UpdateAutosaveCountdownTimer(unsigned int uiSeconds) = 0;
	virtual void ShowSavingMessage(unsigned int iPad, C4JStorage::ESavingMessage eVal) = 0;

	virtual bool PressStartPlaying(unsigned int iPad) = 0;
	virtual void ShowPressStart(unsigned int iPad) = 0;

	virtual void SetWinUserIndex(unsigned int iPad) = 0;
};
