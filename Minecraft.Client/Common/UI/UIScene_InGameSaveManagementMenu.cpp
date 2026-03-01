#include "stdafx.h"
#include "UI.h"
#include "UIScene_InGameSaveManagementMenu.h"

#if defined(__ORBIS__) || defined(__PSVITA__)
#include <ces.h>
#endif

int UIScene_InGameSaveManagementMenu::LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes)
{
	UIScene_InGameSaveManagementMenu *pClass= (UIScene_InGameSaveManagementMenu *)lpParam;

	app.DebugPrintf("Received data for save thumbnail\n");

	if(pbThumbnail && dwThumbnailBytes)
	{
		pClass->m_saveDetails[pClass->m_iRequestingThumbnailId].pbThumbnailData = new BYTE[dwThumbnailBytes];
		memcpy(pClass->m_saveDetails[pClass->m_iRequestingThumbnailId].pbThumbnailData, pbThumbnail, dwThumbnailBytes);
		pClass->m_saveDetails[pClass->m_iRequestingThumbnailId].dwThumbnailSize = dwThumbnailBytes;
	}
	else
	{
		pClass->m_saveDetails[pClass->m_iRequestingThumbnailId].pbThumbnailData = NULL;
		pClass->m_saveDetails[pClass->m_iRequestingThumbnailId].dwThumbnailSize = 0;
		app.DebugPrintf("Save thumbnail data is NULL, or has size 0\n");
	}
	pClass->m_bSaveThumbnailReady = true;

	return 0;
}

UIScene_InGameSaveManagementMenu::UIScene_InGameSaveManagementMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_iRequestingThumbnailId = 0;
	m_iSaveInfoC=0;
	m_bIgnoreInput = false;
	m_iState=e_SavesIdle;
	//m_bRetrievingSaveInfo=false;

	m_buttonListSaves.init(eControl_SavesList);

	m_labelSavesListTitle.init( app.GetString(IDS_SAVE_INCOMPLETE_DELETE_SAVES) );
	m_controlSavesTimer.setVisible( true );


#if defined(_XBOX_ONE) || defined(__ORBIS__)
	m_spaceIndicatorSaves.init(L"",eControl_SpaceIndicator,0, (4LL *1024LL * 1024LL * 1024LL) );
#endif
	m_bUpdateSaveSize = false;

	m_bAllLoaded = false;
	m_bRetrievingSaveThumbnails = false;
	m_bSaveThumbnailReady = false;
	m_bExitScene=false;
	m_pSaveDetails=NULL;
	m_bSavesDisplayed=false;
	m_saveDetails = NULL;
	m_iSaveDetailsCount = 0;

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__) || defined(_DURANGO)
	// Always clear the saves when we enter this menu
	StorageManager.ClearSavesInfo();
#endif

	// block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
	if(app.StartInstallDLCProcess(m_iPad)==true || app.DLCInstallPending())
	{
		// if we're waiting for DLC to mount, don't fill the save list. The custom message on end of dlc mounting will do that
		m_bIgnoreInput = true;
	}
	else
	{
		Initialise();
	}

#ifdef __PSVITA__
	if(CGameNetworkManager::usingAdhocMode() && SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())
	{
		g_NetworkManager.startAdhocMatching();			// create the client matching context and clear out the friends list
	}

#endif

	// If we're not ignoring input, then we aren't still waiting for the DLC to mount, and can now check for corrupt dlc. Otherwise this will happen when the dlc has finished mounting.
	if( !m_bIgnoreInput)
	{
		app.m_dlcManager.checkForCorruptDLCAndAlert();
	}

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);
}


UIScene_InGameSaveManagementMenu::~UIScene_InGameSaveManagementMenu()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);

	if(m_saveDetails)
	{
		for(int i = 0; i < m_iSaveDetailsCount; ++i)
		{
			delete m_saveDetails[i].pbThumbnailData;
		}
		delete [] m_saveDetails;
	}
	app.LeaveSaveNotificationSection();
	StorageManager.SetSaveDisabled(false);
	StorageManager.ContinueIncompleteOperation();
}

void UIScene_InGameSaveManagementMenu::updateTooltips()
{
	int iA = -1;
	if( m_bSavesDisplayed && m_iSaveDetailsCount > 0)
	{
		iA = IDS_TOOLTIPS_DELETESAVE;
	}
	ui.SetTooltips( m_parentLayer->IsFullscreenGroup()?XUSER_INDEX_ANY:m_iPad, iA, IDS_SAVE_INCOMPLETE_RETRY_SAVING);
}

// 
void UIScene_InGameSaveManagementMenu::Initialise()
{
	m_iSaveListIndex = 0;

	// Check if we're in the trial version
	if(ProfileManager.IsFullVersion()==false)
	{
	}
	else if(StorageManager.GetSaveDisabled())
	{
		GetSaveInfo();
	}
	else
	{
		// 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)
		bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();

		GetSaveInfo();
	}

	m_bIgnoreInput=false;
}

void UIScene_InGameSaveManagementMenu::handleReload()
{
	m_bIgnoreInput = false;
	m_iRequestingThumbnailId = 0;
	m_bAllLoaded=false;
	m_bRetrievingSaveThumbnails=false;
	m_bSavesDisplayed=false;
	m_iSaveInfoC=0;
}

void UIScene_InGameSaveManagementMenu::handleGainFocus(bool navBack)
{
	UIScene::handleGainFocus(navBack);

	updateTooltips();

	if(navBack)
	{
		// re-enable button presses
		m_bIgnoreInput=false;
	}
}

wstring UIScene_InGameSaveManagementMenu::getMoviePath()
{
	return L"SaveMenu";
}

void UIScene_InGameSaveManagementMenu::tick()
{
	UIScene::tick();

	if(m_bExitScene) // navigate forward or back
	{
		if(!m_bRetrievingSaveThumbnails)
		{
			// need to wait for any callback retrieving thumbnail to complete
			navigateBack();
		}
	}
	// Stop loading thumbnails if we navigate forwards
	if(hasFocus(m_iPad))
	{
		if(m_bUpdateSaveSize)
		{
			m_spaceIndicatorSaves.selectSave(m_iSaveListIndex);
			m_bUpdateSaveSize = false;
		}

		// Display the saves if we have them
		if(!m_bSavesDisplayed)
		{
			m_pSaveDetails=StorageManager.ReturnSavesInfo();
			if(m_pSaveDetails!=NULL)
			{
				m_spaceIndicatorSaves.reset();

				m_bSavesDisplayed=true;

				if(m_saveDetails!=NULL)
				{
					for(unsigned int i = 0; i < m_pSaveDetails->iSaveC; ++i)
					{
						if(m_saveDetails[i].pbThumbnailData!=NULL)
						{
							delete m_saveDetails[i].pbThumbnailData;
						}
					}
					delete m_saveDetails;
				}
				m_saveDetails = new SaveListDetails[m_pSaveDetails->iSaveC];

				m_iSaveDetailsCount = m_pSaveDetails->iSaveC;
				for(unsigned int i = 0; i < m_pSaveDetails->iSaveC; ++i)
				{
#if defined(_XBOX_ONE)
					m_spaceIndicatorSaves.addSave( m_pSaveDetails->SaveInfoA[i].totalSize );
#elif defined(__ORBIS__)
					m_spaceIndicatorSaves.addSave( m_pSaveDetails->SaveInfoA[i].blocksUsed * (32 * 1024) );
#endif
#ifdef _DURANGO
					m_buttonListSaves.addItem(m_pSaveDetails->SaveInfoA[i].UTF16SaveTitle, L"");

					m_saveDetails[i].saveId = i;
					memcpy(m_saveDetails[i].UTF16SaveName, m_pSaveDetails->SaveInfoA[i].UTF16SaveTitle, 128);
					memcpy(m_saveDetails[i].UTF16SaveFilename, m_pSaveDetails->SaveInfoA[i].UTF16SaveFilename, MAX_SAVEFILENAME_LENGTH);
#else
					m_buttonListSaves.addItem(m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, L"");

					m_saveDetails[i].saveId = i;
					memcpy(m_saveDetails[i].UTF8SaveName, m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, 128);
					memcpy(m_saveDetails[i].UTF8SaveFilename, m_pSaveDetails->SaveInfoA[i].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH);
#endif
				}
				m_controlSavesTimer.setVisible( false );

				// set focus on the first button

			}
		}

		if(!m_bExitScene && m_bSavesDisplayed && !m_bRetrievingSaveThumbnails && !m_bAllLoaded)
		{
			if( m_iRequestingThumbnailId < (m_buttonListSaves.getItemCount() ))
			{
				m_bRetrievingSaveThumbnails = true;
				app.DebugPrintf("Requesting the first thumbnail\n");
				// set the save to load
				PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
				C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[(int)m_iRequestingThumbnailId],&LoadSaveDataThumbnailReturned,this);

				if(eLoadStatus!=C4JStorage::ESaveGame_GetSaveThumbnail)
				{
					// something went wrong
					m_bRetrievingSaveThumbnails=false;
					m_bAllLoaded = true;
				}
			}
		}
		else if (m_bSavesDisplayed && m_bSaveThumbnailReady)
		{
			m_bSaveThumbnailReady = false;

			// check we're not waiting to exit the scene
			if(!m_bExitScene)
			{
				// convert to utf16
				uint16_t u16Message[MAX_SAVEFILENAME_LENGTH];
#ifdef _DURANGO
				// Already utf16 on durango
				memcpy(u16Message, m_saveDetails[m_iRequestingThumbnailId].UTF16SaveFilename, MAX_SAVEFILENAME_LENGTH);
#elif defined(_WINDOWS64)
				int result = ::MultiByteToWideChar(
					CP_UTF8,                // convert from UTF-8
					MB_ERR_INVALID_CHARS,   // error on invalid chars
					m_saveDetails[m_iRequestingThumbnailId].UTF8SaveFilename,            // source UTF-8 string
					MAX_SAVEFILENAME_LENGTH,                 // total length of source UTF-8 string,
					// in CHAR's (= bytes), including end-of-string \0
					(wchar_t *)u16Message,               // destination buffer
					MAX_SAVEFILENAME_LENGTH                // size of destination buffer, in WCHAR's
					);
#else
#ifdef __PS3
				size_t srcmax,dstmax;
#else
				uint32_t srcmax,dstmax;
				uint32_t srclen,dstlen;
#endif
				srcmax=MAX_SAVEFILENAME_LENGTH;
				dstmax=MAX_SAVEFILENAME_LENGTH;

#if defined(__PS3__)
				L10nResult lres= UTF8stoUTF16s((uint8_t *)m_saveDetails[m_iRequestingThumbnailId].UTF8SaveFilename,&srcmax,u16Message,&dstmax);
#else
				SceCesUcsContext context;
				sceCesUcsContextInit(&context);

				sceCesUtf8StrToUtf16Str(&context, (uint8_t *)m_saveDetails[m_iRequestingThumbnailId].UTF8SaveFilename,srcmax,&srclen,u16Message,dstmax,&dstlen);
#endif
#endif
				if( m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData )
				{
					registerSubstitutionTexture((wchar_t *)u16Message,m_saveDetails[m_iRequestingThumbnailId].pbThumbnailData,m_saveDetails[m_iRequestingThumbnailId].dwThumbnailSize);
				}
				m_buttonListSaves.setTextureName(m_iRequestingThumbnailId, (wchar_t *)u16Message);

				++m_iRequestingThumbnailId;
				if( m_iRequestingThumbnailId < (m_buttonListSaves.getItemCount() ))
				{
					app.DebugPrintf("Requesting another thumbnail\n");
					// set the save to load
					PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
					C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[(int)m_iRequestingThumbnailId],&LoadSaveDataThumbnailReturned,this);
					if(eLoadStatus!=C4JStorage::ESaveGame_GetSaveThumbnail)
					{
						// something went wrong
						m_bRetrievingSaveThumbnails=false;
						m_bAllLoaded = true;
					}
				}
				else
				{
					m_bRetrievingSaveThumbnails = false;
					m_bAllLoaded = true;
				}
			}
			else
			{
				// stop retrieving thumbnails, and exit
				m_bRetrievingSaveThumbnails = false;
			}
		}
	}

	switch(m_iState)
	{
	case e_SavesIdle:
		break;
	case e_SavesRepopulateAfterDelete:
		m_bIgnoreInput = false;
		m_iRequestingThumbnailId = 0;
		m_bAllLoaded=false;
		m_bRetrievingSaveThumbnails=false;
		m_bSavesDisplayed=false;
		m_iSaveInfoC=0;
		m_buttonListSaves.clearList();
		//StorageManager.ClearSavesInfo();
		//GetSaveInfo();
		m_iState=e_SavesIdle;
		break;
	}
}

void UIScene_InGameSaveManagementMenu::GetSaveInfo(  )
{
	unsigned int uiSaveC=0;

	// This will return with the number retrieved in uiSaveC

	// clear the saves list
	m_bSavesDisplayed = false; // we're blocking the exit from this scene until complete
	m_buttonListSaves.clearList();
	m_iSaveInfoC=0;
	m_controlSavesTimer.setVisible(true);

	m_pSaveDetails=StorageManager.ReturnSavesInfo();
	if(m_pSaveDetails==NULL)
	{
		C4JStorage::ESaveGameState eSGIStatus= StorageManager.GetSavesInfo(m_iPad,NULL,this,"save"); 
	}


	return;
}

void UIScene_InGameSaveManagementMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

	// if we're retrieving save info, ignore key presses
	if(!m_bSavesDisplayed) return;

	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
			m_bExitScene=true;
#else
			navigateBack();
#endif
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

void UIScene_InGameSaveManagementMenu::handleInitFocus(F64 controlId, F64 childId)
{
	app.DebugPrintf(app.USER_SR, "UIScene_InGameSaveManagementMenu::handleInitFocus - %d , %d\n", (int)controlId, (int)childId);
}

void UIScene_InGameSaveManagementMenu::handleFocusChange(F64 controlId, F64 childId) 
{
	app.DebugPrintf(app.USER_SR, "UIScene_InGameSaveManagementMenu::handleFocusChange - %d , %d\n", (int)controlId, (int)childId);
	m_iSaveListIndex = childId;
	if(m_bSavesDisplayed) m_bUpdateSaveSize = true;
	updateTooltips();
}

void UIScene_InGameSaveManagementMenu::handlePress(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_SavesList:
		{
			m_bIgnoreInput = true;

			// delete the save game
			// Have to ask the player if they are sure they want to delete this game
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestErrorMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2,m_iPad,&UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned,this);

			ui.PlayUISFX(eSFX_Press);
			break;
		}
	}
}

int UIScene_InGameSaveManagementMenu::DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_InGameSaveManagementMenu* pClass = (UIScene_InGameSaveManagementMenu*)pParam;
	// results switched for this dialog

	if(result==C4JStorage::EMessage_ResultDecline) 
	{
		if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
		{
			pClass->m_bIgnoreInput=false;
		}
		else
		{
			StorageManager.DeleteSaveData(&pClass->m_pSaveDetails->SaveInfoA[pClass->m_iSaveListIndex],UIScene_InGameSaveManagementMenu::DeleteSaveDataReturned,pClass);
			pClass->m_controlSavesTimer.setVisible( true );
		}
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}

	return 0;
}

int UIScene_InGameSaveManagementMenu::DeleteSaveDataReturned(LPVOID lpParam,bool bRes)
{
	UIScene_InGameSaveManagementMenu* pClass = (UIScene_InGameSaveManagementMenu*)lpParam;

	if(bRes)
	{
		// wipe the list and repopulate it
		pClass->m_iState=e_SavesRepopulateAfterDelete;
	}
	else pClass->m_bIgnoreInput=false;

	pClass->updateTooltips();

	return 0;
}

bool UIScene_InGameSaveManagementMenu::hasFocus(int iPad)
{
	return bHasFocus && (iPad == m_iPad || m_iPad == XUSER_INDEX_ANY);
}