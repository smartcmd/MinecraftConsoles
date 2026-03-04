#include "stdafx.h"
#include "UI.h"
#if defined(__PS3__) || defined(__ORBIS__)
#include "Common\Network\Sony\SonyCommerce.h"
#endif
#include "UIScene_DLCMainMenu.h"

#define PLAYER_ONLINE_TIMER_ID 0
#define PLAYER_ONLINE_TIMER_TIME 100

UIScene_DLCMainMenu::UIScene_DLCMainMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	// Alert the app the we want to be informed of ethernet connections
	app.SetLiveLinkRequired( true );

	m_labelOffers.init(IDS_DOWNLOADABLE_CONTENT_OFFERS);
	m_buttonListOffers.init(eControl_OffersList);

#if defined _XBOX_ONE || defined __ORBIS__
	// load any local DLC images
	app.LoadLocalDLCImages();
#endif

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
	// show a timer on this menu
	m_Timer.setVisible(true);

 	m_bCategoriesShown=false;
#endif

	if(m_loadedResolution == eSceneResolution_1080)
	{
#ifdef _DURANGO
		m_labelXboxStore.init(IDS_XBOX_STORE);
#else
		m_labelXboxStore.init( L"" );
#endif
	}

#if defined(_DURANGO)
	m_Timer.setVisible(false);

	m_buttonListOffers.addItem(IDS_DLC_MENU_SKINPACKS,e_DLC_SkinPack);
	m_buttonListOffers.addItem(IDS_DLC_MENU_TEXTUREPACKS,e_DLC_TexturePacks);
	m_buttonListOffers.addItem(IDS_DLC_MENU_MASHUPPACKS,e_DLC_MashupPacks);

	app.AddDLCRequest(e_Marketplace_Content); // content is skin packs, texture packs and mash-up packs
	// we also need to mount the local DLC so we can tell what's been purchased
	app.StartInstallDLCProcess(iPad);
#endif
	
	TelemetryManager->RecordMenuShown(iPad, eUIScene_DLCMainMenu, 0);

#if defined __ORBIS__ || defined __PSVITA__
	app.GetCommerce()->ShowPsStoreIcon();
#endif

#if ( defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ )
	addTimer( PLAYER_ONLINE_TIMER_ID, PLAYER_ONLINE_TIMER_TIME );
#endif
}

UIScene_DLCMainMenu::~UIScene_DLCMainMenu()
{
	// Alert the app the we no longer want to be informed of ethernet connections
	app.SetLiveLinkRequired( false );
#if defined _XBOX_ONE || defined __ORBIS__
	app.FreeLocalDLCImages();
#endif

#ifdef _XBOX_ONE
	// 4J-JEV: Have to switch back to user preferred languge now.
	setLanguageOverride(true);
#endif
}

wstring UIScene_DLCMainMenu::getMoviePath()
{
	return L"DLCMainMenu";
}

void UIScene_DLCMainMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK );
}

void UIScene_DLCMainMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
#if defined __ORBIS__ || defined __PSVITA__
			app.GetCommerce()->HidePsStoreIcon();
#endif
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_DLCMainMenu::handlePress(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_OffersList:
		{
			int iIndex = (int)childId;
			DLCOffersParam *param = new DLCOffersParam();
			param->iPad = m_iPad;

			param->iType = iIndex;
			// promote the DLC content request type 

			// Xbox One will have requested the marketplace content - there is only that type
#ifndef _XBOX_ONE
			app.AddDLCRequest((eDLCMarketplaceType)iIndex, true);
#endif
			killTimer(PLAYER_ONLINE_TIMER_ID);
			ui.NavigateToScene(m_iPad, eUIScene_DLCOffersMenu, param);
			break;
		}
	};
}

void UIScene_DLCMainMenu::handleTimerComplete(int id)
{
#if ( defined __PS3__ || defined __ORBIS__  || defined __PSVITA__)
	switch(id)
	{
	case PLAYER_ONLINE_TIMER_ID:
#ifndef _WINDOWS64
		if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad())==false)
		{
			// check the player hasn't gone offline
			// If they have, bring up the PSN warning and exit from the leaderboards
			unsigned int uiIDA[1];
			uiIDA[0]=IDS_OK;
			C4JStorage::EMessageResult result = ui.RequestErrorMessage( IDS_CONNECTION_LOST, g_NetworkManager.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT), uiIDA,1,ProfileManager.GetPrimaryPad(),UIScene_DLCMainMenu::ExitDLCMainMenu,this);
		}
#endif
		break;
	}
#endif
}

int UIScene_DLCMainMenu::ExitDLCMainMenu(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_DLCMainMenu* pClass = (UIScene_DLCMainMenu*)pParam;

#if defined __ORBIS__ || defined __PSVITA__
	app.GetCommerce()->HidePsStoreIcon();
#endif
	pClass->navigateBack();

	return 0;
}

void UIScene_DLCMainMenu::handleGainFocus(bool navBack)
{
	UIScene::handleGainFocus(navBack);

	updateTooltips();

	if(navBack)
	{
		// add the timer back in
#if ( defined __PS3__ || defined __ORBIS__ || defined __PSVITA__ )
		addTimer( PLAYER_ONLINE_TIMER_ID, PLAYER_ONLINE_TIMER_TIME );
#endif
	}
}

void UIScene_DLCMainMenu::tick()
{
	UIScene::tick();

#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
	if((m_bCategoriesShown==false) && (app.GetCommerceCategoriesRetrieved()))
	{
		// disable the timer display on this menu
		m_Timer.setVisible(false);
		m_bCategoriesShown=true;

		// add the categories to the list box
		SonyCommerce::CategoryInfo *pCategories=app.GetCategoryInfo();
		std::list<SonyCommerce::CategoryInfoSub>::iterator iter = pCategories->subCategories.begin();
		SonyCommerce::CategoryInfoSub category;
		for(int i=0;i<pCategories->countOfSubCategories;i++)
		{
			// add a button in with the subcategory
			category = (SonyCommerce::CategoryInfoSub)(*iter);

			string teststring=category.categoryName;
			m_buttonListOffers.addItem(teststring,i);
			
			iter++;
		}

		// set the focus to the first thing in the categories if there are any
		if(pCategories->countOfSubCategories>0)
		{
			m_buttonListOffers.setFocus(true);
		}
		else
		{
#if defined __ORBIS__ || defined __PSVITA__ || defined __PS3__
			app.CheckForEmptyStore(ProfileManager.GetPrimaryPad());
#endif
			// need to display text to say no downloadable content available yet
			m_labelOffers.setLabel(app.GetString(IDS_NO_DLCCATEGORIES));

#ifdef __ORBIS__
			// 4J-JEV: TRC Requirement (R4055), need to display this system message.
			ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE, ProfileManager.GetPrimaryPad() );
#endif
		}

	}
#endif
}

