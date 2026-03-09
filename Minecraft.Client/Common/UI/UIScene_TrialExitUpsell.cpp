#include "stdafx.h"
#include "UI.h"
#include "UIScene_TrialExitUpsell.h"


UIScene_TrialExitUpsell::UIScene_TrialExitUpsell(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
}

wstring UIScene_TrialExitUpsell::getMoviePath()
{
	return L"TrialExitUpsell";
}

void UIScene_TrialExitUpsell::updateTooltips()
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_EXIT_GAME,IDS_TOOLTIPS_BACK, IDS_UNLOCK_TITLE);
}

void UIScene_TrialExitUpsell::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		navigateBack();
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		if(pressed)
		{
			//CD - Added for audio
			ui.PlayUISFX(eSFX_Press);
			app.ExitGame();
		}
		break;
	case ACTION_MENU_X:
		if(ProfileManager.IsSignedIn(iPad))
		{
			//CD - Added for audio
			ui.PlayUISFX(eSFX_Press);

			// 4J-PB - need to check this user can access the store
#if defined(__PS3__) || defined(__PSVITA__)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,nullptr,&bContentRestricted,nullptr);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad());
			}
			else
#endif
			{
				TelemetryManager->RecordUpsellPresented(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID);
				ProfileManager.DisplayFullVersionPurchase(false,iPad,eSen_UpsellID_Full_Version_Of_Game);
			}
		}
		break;
	}
}

void UIScene_TrialExitUpsell::handleAnimationEnd()
{
	//ui.NavigateToHomeMenu();
	ui.NavigateToScene(0,eUIScene_SaveMessage);
}
