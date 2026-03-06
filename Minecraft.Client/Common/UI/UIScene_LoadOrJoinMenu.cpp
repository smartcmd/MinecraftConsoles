#include "stdafx.h"
#include "UI.h"
#include "UIScene_LoadOrJoinMenu.h"

#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.chunk.storage.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFile.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileOriginal.h"
#include "..\..\ProgressRenderer.h"
#include "..\..\MinecraftServer.h"
#include "..\..\TexturePackRepository.h"
#include "..\..\TexturePack.h"
#include "..\Network\SessionInfo.h"
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
#include "Common\Network\Sony\SonyHttp.h"
#include "Common\Network\Sony\SonyRemoteStorage.h"
#include "DLCTexturePack.h"
#endif
#if defined(__ORBIS__) || defined(__PSVITA__)
#include <ces.h>
#endif
#ifdef __PSVITA__
#include "message_dialog.h"
#endif

#ifdef _WINDOWS64
#include "..\..\..\Minecraft.World\NbtIo.h"
#include "..\..\..\Minecraft.World\compression.h"

static wstring ReadLevelNameFromSaveFile(const wstring& filePath)
{
    // Check for a worldname.txt sidecar written by the rename feature first
    size_t slashPos = filePath.rfind(L'\\');
    if (slashPos != wstring::npos)
    {
        wstring sidecarPath = filePath.substr(0, slashPos + 1) + L"worldname.txt";
        FILE *fr = NULL;
        if (_wfopen_s(&fr, sidecarPath.c_str(), L"r") == 0 && fr)
        {
            char buf[128] = {};
            if (fgets(buf, sizeof(buf), fr))
            {
                int len = (int)strlen(buf);
                while (len > 0 && (buf[len-1] == '\n' || buf[len-1] == '\r' || buf[len-1] == ' '))
                    buf[--len] = '\0';
                fclose(fr);
                if (len > 0)
                {
                    wchar_t wbuf[128] = {};
                    mbstowcs(wbuf, buf, 127);
                    return wstring(wbuf);
                }
            }
            else fclose(fr);
        }
    }

    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return L"";

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize < 12 || fileSize == INVALID_FILE_SIZE) { CloseHandle(hFile); return L""; }

    unsigned char *rawData = new unsigned char[fileSize];
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, rawData, fileSize, &bytesRead, NULL) || bytesRead != fileSize)
    {
        CloseHandle(hFile);
        delete[] rawData;
        return L"";
    }
    CloseHandle(hFile);

    unsigned char *saveData = NULL;
    unsigned int saveSize   = 0;
    bool freeSaveData = false;

    if (*(unsigned int*)rawData == 0)
    {
        // Compressed format: bytes 0-3=0, bytes 4-7=decompressed size, bytes 8+=compressed data
        unsigned int decompSize = *(unsigned int*)(rawData + 4);
        if (decompSize == 0 || decompSize > 128 * 1024 * 1024)
        {
            delete[] rawData;
            return L"";
        }
        saveData = new unsigned char[decompSize];
        Compression::getCompression()->Decompress(saveData, &decompSize, rawData + 8, fileSize - 8);
        saveSize     = decompSize;
        freeSaveData = true;
    }
    else
    {
        saveData = rawData;
        saveSize = fileSize;
    }

    wstring result = L"";
    if (saveSize >= 12)
    {
        unsigned int headerOffset = *(unsigned int*)saveData;
        unsigned int numEntries   = *(unsigned int*)(saveData + 4);
        const unsigned int entrySize = sizeof(FileEntrySaveData);

        if (headerOffset < saveSize && numEntries > 0 && numEntries < 10000 &&
            headerOffset + numEntries * entrySize <= saveSize)
        {
            FileEntrySaveData *table = (FileEntrySaveData *)(saveData + headerOffset);
            for (unsigned int i = 0; i < numEntries; i++)
            {
                if (wcscmp(table[i].filename, L"level.dat") == 0)
                {
                    unsigned int off = table[i].startOffset;
                    unsigned int len = table[i].length;
                    if (off >= 12 && off + len <= saveSize && len > 0 && len < 4 * 1024 * 1024)
                    {
                        byteArray ba;
                        ba.data   = (byte*)(saveData + off);
                        ba.length = len;
                        CompoundTag *root = NbtIo::decompress(ba);
                        if (root != NULL)
                        {
                            CompoundTag *dataTag = root->getCompound(L"Data");
                            if (dataTag != NULL)
                                result = dataTag->getString(L"LevelName");
                            delete root;
                        }
                    }
                    break;
                }
            }
        }
    }

    if (freeSaveData) delete[] saveData;
    delete[] rawData;
    // "world" is the engine default - it means no real name was ever set,
    // so return empty to let the caller fall back to the save filename (timestamp).
    if (result == L"world") result = L"";
    return result;
}
#endif


#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
unsigned long UIScene_LoadOrJoinMenu::m_ulFileSize=0L;
wstring UIScene_LoadOrJoinMenu::m_wstrStageText=L"";
bool UIScene_LoadOrJoinMenu::m_bSaveTransferRunning = false;
#endif


#define JOIN_LOAD_ONLINE_TIMER_ID 0
#define JOIN_LOAD_ONLINE_TIMER_TIME 100

#ifdef _XBOX
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID 3
#define CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME 50
#endif

#ifdef _XBOX_ONE
UIScene_LoadOrJoinMenu::ESaveTransferFiles UIScene_LoadOrJoinMenu::s_eSaveTransferFile;
unsigned long UIScene_LoadOrJoinMenu::s_ulFileSize=0L;
byteArray UIScene_LoadOrJoinMenu::s_transferData = byteArray();
wstring UIScene_LoadOrJoinMenu::m_wstrStageText=L"";

#ifdef _DEBUG_MENUS_ENABLED
C4JStorage::SAVETRANSFER_FILE_DETAILS UIScene_LoadOrJoinMenu::m_debugTransferDetails;
#endif
#endif

int UIScene_LoadOrJoinMenu::LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes)
{
    UIScene_LoadOrJoinMenu *pClass= (UIScene_LoadOrJoinMenu *)lpParam;

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

int UIScene_LoadOrJoinMenu::LoadSaveCallback(LPVOID lpParam,bool bRes)
{
    //UIScene_LoadOrJoinMenu *pClass= (UIScene_LoadOrJoinMenu *)lpParam;
    // Get the save data now
    if(bRes)
    {
        app.DebugPrintf("Loaded save OK\n");
    }
    return 0;
}

UIScene_LoadOrJoinMenu::UIScene_LoadOrJoinMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
    // Setup all the Iggy references we need for this scene
    initialiseMovie();
    app.SetLiveLinkRequired( true );

    m_iRequestingThumbnailId = 0;
    m_iSaveInfoC=0;
    m_bIgnoreInput = false;
    m_bShowingPartyGamesOnly = false;
    m_bInParty = false;
    m_currentSessions = NULL;
    m_iState=e_SavesIdle;
    //m_bRetrievingSaveInfo=false;

    m_buttonListSaves.init(eControl_SavesList);
    m_buttonListGames.init(eControl_GamesList);

    m_labelSavesListTitle.init( IDS_START_GAME );
    m_labelJoinListTitle.init( IDS_JOIN_GAME );
    m_labelNoGames.init( IDS_NO_GAMES_FOUND );
    m_labelNoGames.setVisible( false );
    m_controlSavesTimer.setVisible( true );
    m_controlJoinTimer.setVisible( true );


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
    m_iTexturePacksNotInstalled = 0;
	m_bCopying = false;
	m_bCopyingCancelled = false;

#ifndef _XBOX_ONE
    m_bSaveTransferCancelled=false;
    m_bSaveTransferInProgress=false;
#endif
	m_eAction = eAction_None;

    m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);

#ifdef _XBOX_ONE
	// 4J-PB - in order to buy the skin packs & texture packs, we need the signed offer ids for them, which we get in the availability info
	// we need to retrieve this info though, so do it here
	app.AddDLCRequest(e_Marketplace_Content); // content is skin packs, texture packs and mash-up packs
#endif


    int iLB = -1;

#ifdef _XBOX
    XPARTY_USER_LIST partyList;

    if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
    {
        m_bInParty=true;
    }
    else
    {
        m_bInParty=false;
    }
#endif

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__) || defined(_DURANGO) || defined(_WINDOWS64)
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

    UpdateGamesList();

    g_NetworkManager.SetSessionsUpdatedCallback( &UpdateGamesListCallback, this );

    m_initData= new JoinMenuInitData();

    // 4J Stu - Fix for #12530 -TCR 001 BAS Game Stability: Title will crash if the player disconnects while starting a new world and then opts to play the tutorial once they have been returned to the Main Menu.
    MinecraftServer::resetFlags();

    // If we're not ignoring input, then we aren't still waiting for the DLC to mount, and can now check for corrupt dlc. Otherwise this will happen when the dlc has finished mounting.
    if( !m_bIgnoreInput)
    {
        app.m_dlcManager.checkForCorruptDLCAndAlert();
    }

    // 4J-PB - Only Xbox will not have trial DLC patched into the game
#ifdef _XBOX
    // 4J-PB - there may be texture packs we don't have, so use the info from TMS for this

    DLC_INFO *pDLCInfo=NULL;

    // first pass - look to see if there are any that are not in the list
    bool bTexturePackAlreadyListed;
    bool bNeedToGetTPD=false;
    Minecraft *pMinecraft = Minecraft::GetInstance();
    int texturePacksCount = pMinecraft->skins->getTexturePackCount();

    for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
    {
        bTexturePackAlreadyListed=false;
#if defined(__PS3__) || defined(__ORBIS__)
        char *pchDLCName=app.GetDLCInfoTextures(i);
        pDLCInfo=app.GetDLCInfo(pchDLCName);
#else
        ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
        pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
#endif
        for(unsigned int i = 0; i < texturePacksCount; ++i)
        {
            TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
            if(pDLCInfo && pDLCInfo->iConfig==tp->getDLCParentPackId())
            {
                bTexturePackAlreadyListed=true;
            }
        }
        if(bTexturePackAlreadyListed==false)
        {
            // some missing
            bNeedToGetTPD=true;

            m_iTexturePacksNotInstalled++;
        }
    }

    if(bNeedToGetTPD==true)
    {
        // add a TMS request for them
        app.DebugPrintf("+++ Adding TMSPP request for texture pack data\n");
        app.AddTMSPPFileTypeRequest(e_DLC_TexturePackData);
        m_iConfigA= new int [m_iTexturePacksNotInstalled];
        m_iTexturePacksNotInstalled=0;

        for(unsigned int i = 0; i < app.GetDLCInfoTexturesOffersCount(); ++i)
        {
            bTexturePackAlreadyListed=false;
#if defined(__PS3__) || defined(__ORBIS__)
            char *pchDLCName=app.GetDLCInfoTextures(i);
            pDLCInfo=app.GetDLCInfo(pchDLCName);
#else
            ULONGLONG ull=app.GetDLCInfoTexturesFullOffer(i);
            pDLCInfo=app.GetDLCInfoForFullOfferID(ull);
#endif
            for(unsigned int i = 0; i < texturePacksCount; ++i)
            {
                TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(i);
                if(pDLCInfo->iConfig==tp->getDLCParentPackId())
                {
                    bTexturePackAlreadyListed=true;
                }
            }
            if(bTexturePackAlreadyListed==false)
            {
                m_iConfigA[m_iTexturePacksNotInstalled++]=pDLCInfo->iConfig;
            }
        }
    }

    addTimer(CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID,CHECKFORAVAILABLETEXTUREPACKS_TIMER_TIME);
#endif

#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
    m_eSaveTransferState = eSaveTransfer_Idle;
#endif
}


UIScene_LoadOrJoinMenu::~UIScene_LoadOrJoinMenu()
{
    g_NetworkManager.SetSessionsUpdatedCallback( NULL, NULL );
    app.SetLiveLinkRequired( false );

    delete m_currentSessions;
    m_currentSessions = NULL;

#if TO_BE_IMPLEMENTED
    // Reset the background downloading, in case we changed it by attempting to download a texture pack
    XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
#endif

    if(m_saveDetails)
    {
        for(int i = 0; i < m_iSaveDetailsCount; ++i)
        {
            delete m_saveDetails[i].pbThumbnailData;
        }
        delete [] m_saveDetails;
    }
}

void UIScene_LoadOrJoinMenu::updateTooltips()
{
#if defined __PS3__ || defined __ORBIS__ || defined __PSVITA__
	if(m_eSaveTransferState!=eSaveTransfer_Idle)
	{
		// we're in a full screen progress for the save download here, so don't change the tooltips
		return;
	}
#endif

    // update the tooltips
    // if the saves list has focus, then we should show the Delete Save tooltip
    // if the games list has focus, then we should the the View Gamercard tooltip
    int iRB=-1;
    int iY = -1;
    int iLB = -1;
    int iX=-1;
    if (DoesGamesListHaveFocus() && m_buttonListGames.getItemCount() > 0)
    {
        iY = IDS_TOOLTIPS_VIEW_GAMERCARD;
    }
    else if (DoesSavesListHaveFocus())
    {
        if((m_iDefaultButtonsC > 0) && (m_iSaveListIndex >= m_iDefaultButtonsC))
        {
            if(StorageManager.GetSaveDisabled())
            {
                iRB=IDS_TOOLTIPS_DELETESAVE;
            }
            else
            {
                if(StorageManager.EnoughSpaceForAMinSaveGame())
                {
                    iRB=IDS_TOOLTIPS_SAVEOPTIONS;
                }
                else
                {
                    iRB=IDS_TOOLTIPS_DELETESAVE;
                }
            }
        }
    }
	else if(DoesMashUpWorldHaveFocus())
	{
		// If it's a mash-up pack world, give the Hide option
		iRB=IDS_TOOLTIPS_HIDE;
	}

    if(m_bInParty)
    {
        if( m_bShowingPartyGamesOnly ) iLB = IDS_TOOLTIPS_ALL_GAMES;
        else iLB = IDS_TOOLTIPS_PARTY_GAMES;
    }

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
    if(m_iPad == ProfileManager.GetPrimaryPad() ) iY = IDS_TOOLTIPS_GAME_INVITES;
#endif

    if(ProfileManager.IsFullVersion()==false )
    {
        iRB = -1;
    }
    else if(StorageManager.GetSaveDisabled())
    {
#ifdef _XBOX
        iX = IDS_TOOLTIPS_SELECTDEVICE;
#endif
    }
    else
    {
#if defined _XBOX_ONE
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			// Is there a save from 360 on TMS?
			iX=IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD;
		}
#elif defined SONY_REMOTE_STORAGE_DOWNLOAD
        // Is there a save from PS3 or PSVita available?
		// Sony asked that this be displayed at all times so users are aware of the functionality. We'll display some text when there's no save available
        //if(app.getRemoteStorage()->saveIsAvailable())
		{
			bool bSignedInLive = ProfileManager.IsSignedInLive(m_iPad);
			if(bSignedInLive)
			{
				iX=IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD;
			}
		}
#else
        iX = IDS_TOOLTIPS_CHANGEDEVICE;
#endif
    }

    ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, iX, iY,-1,-1,iLB,iRB);
}

//
void UIScene_LoadOrJoinMenu::Initialise()
{
    m_iSaveListIndex = 0;
	m_iGameListIndex = 0;

    m_iDefaultButtonsC = 0;
	m_iMashUpButtonsC=0;

    // Check if we're in the trial version
    if(ProfileManager.IsFullVersion()==false)
    {


        AddDefaultButtons();

#if TO_BE_IMPLEMENTED
        m_pSavesList->SetCurSelVisible(0);
#endif
    }
    else if(StorageManager.GetSaveDisabled())
    {
#if defined(__PS3__) || defined(__ORBIS__) || defined (__PSVITA__)
        GetSaveInfo();
#else

#if TO_BE_IMPLEMENTED
        if(StorageManager.GetSaveDeviceSelected(m_iPad))
#endif
        {
            // saving is disabled, but we should still be able to load from a selected save device



            GetSaveInfo();
        }
#if TO_BE_IMPLEMENTED
        else
        {
            AddDefaultButtons();
            m_controlSavesTimer.setVisible( false );
        }
#endif
#endif // __PS3__ || __ORBIS
    }
    else
    {
        // 4J-PB - we need to check that there is enough space left to create a copy of the save (for a rename)
        bool bCanRename = StorageManager.EnoughSpaceForAMinSaveGame();

        GetSaveInfo();
    }

    m_bIgnoreInput=false;
    app.m_dlcManager.checkForCorruptDLCAndAlert();
}

void UIScene_LoadOrJoinMenu::updateComponents()
{
    m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
    m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
}

void UIScene_LoadOrJoinMenu::handleDestroy()
{
#ifdef __PSVITA__
	app.DebugPrintf("missing InputManager.DestroyKeyboard on Vita !!!!!!\n");
#endif

	// shut down the keyboard if it is displayed
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO)
	InputManager.DestroyKeyboard();
#endif
}

void UIScene_LoadOrJoinMenu::handleGainFocus(bool navBack)
{
    UIScene::handleGainFocus(navBack);

    updateTooltips();

    // Add load online timer
    addTimer(JOIN_LOAD_ONLINE_TIMER_ID,JOIN_LOAD_ONLINE_TIMER_TIME);

    if(navBack)
    {
        app.SetLiveLinkRequired( true );

        m_bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);

        // re-enable button presses
        m_bIgnoreInput=false;

        // block input if we're waiting for DLC to install, and wipe the saves list. The end of dlc mounting custom message will fill the list again
        if(app.StartInstallDLCProcess(m_iPad)==false)
        {
            // not doing a mount, so re-enable input
            m_bIgnoreInput=false;
        }
        else
        {
            m_bIgnoreInput=true;
            m_buttonListSaves.clearList();
            m_controlSavesTimer.setVisible(true);
        }

        if( m_bMultiplayerAllowed )
        {
#if TO_BE_IMPLEMENTED
            HXUICLASS hClassFullscreenProgress = XuiFindClass( L"CScene_FullscreenProgress" );
            HXUICLASS hClassConnectingProgress = XuiFindClass( L"CScene_ConnectingProgress" );

            // If we are navigating back from a full screen progress scene, then that means a connection attempt failed
            if( XuiIsInstanceOf( hSceneFrom, hClassFullscreenProgress ) || XuiIsInstanceOf( hSceneFrom, hClassConnectingProgress ) )
            {
                UpdateGamesList();
            }
#endif
        }
        else
        {
            m_buttonListGames.clearList();
            m_controlJoinTimer.setVisible(true);
            m_labelNoGames.setVisible(false);
#if TO_BE_IMPLEMENTED
            m_SavesList.InitFocus(m_iPad);
#endif
        }

        // are we back here because of a delete of a corrupt save?

        if(app.GetCorruptSaveDeleted())
        {
            // wipe the list and repopulate it
            m_iState=e_SavesRepopulateAfterDelete;
            app.SetCorruptSaveDeleted(false);
        }
    }
}

void UIScene_LoadOrJoinMenu::handleLoseFocus()
{
    // Kill load online timer
    killTimer(JOIN_LOAD_ONLINE_TIMER_ID);
}

wstring UIScene_LoadOrJoinMenu::getMoviePath()
{
    return L"LoadOrJoinMenu";
}

void UIScene_LoadOrJoinMenu::tick()
{
    UIScene::tick();

#if (defined  __PS3__  || defined __ORBIS__ || defined _DURANGO || defined _WINDOWS64 || defined __PSVITA__)
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
#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
		// if the loadOrJoin menu has focus again, we can clear the saveTransfer flag now. Added so we can delay the ehternet disconnect till it's cleaned up
		if(m_eSaveTransferState == eSaveTransfer_Idle)
			m_bSaveTransferRunning = false;
#endif
#if defined(_XBOX_ONE) || defined(__ORBIS__)
        if(m_bUpdateSaveSize)
        {
            if((m_iDefaultButtonsC > 0) && (m_iSaveListIndex >= m_iDefaultButtonsC))
            {
                m_spaceIndicatorSaves.selectSave(m_iSaveListIndex-m_iDefaultButtonsC);
            }
            else
            {
                m_spaceIndicatorSaves.selectSave(-1);
            }
            m_bUpdateSaveSize = false;
        }
#endif
        // Display the saves if we have them
        if(!m_bSavesDisplayed)
        {
            m_pSaveDetails=StorageManager.ReturnSavesInfo();
            if(m_pSaveDetails!=NULL)
            {
                //CD - Fix - Adding define for ORBIS/XBOXONE
#if defined(_XBOX_ONE) || defined(__ORBIS__)
                m_spaceIndicatorSaves.reset();
#endif

                AddDefaultButtons();
                m_bSavesDisplayed=true;
                UpdateGamesList();

                if(m_saveDetails!=NULL)
                {
                    for(unsigned int i = 0; i < m_iSaveDetailsCount; ++i)
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
#ifdef _WINDOWS64
                // Build sorted index array (newest-first by filename timestamp YYYYMMDDHHMMSS)
                int *sortedIdx = new int[m_pSaveDetails->iSaveC];
                for (int si = 0; si < (int)m_pSaveDetails->iSaveC; ++si) sortedIdx[si] = si;
                for (int si = 1; si < (int)m_pSaveDetails->iSaveC; ++si)
                {
                    int key = sortedIdx[si];
                    int sj = si - 1;
                    while (sj >= 0 && strcmp(m_pSaveDetails->SaveInfoA[sortedIdx[sj]].UTF8SaveFilename, m_pSaveDetails->SaveInfoA[key].UTF8SaveFilename) < 0)
                    {
                        sortedIdx[sj + 1] = sortedIdx[sj];
                        --sj;
                    }
                    sortedIdx[sj + 1] = key;
                }
#endif
                for(unsigned int i = 0; i < m_pSaveDetails->iSaveC; ++i)
                {
#if defined(_XBOX_ONE)
                    m_spaceIndicatorSaves.addSave(m_pSaveDetails->SaveInfoA[i].totalSize);
#elif defined(__ORBIS__)
                    m_spaceIndicatorSaves.addSave(m_pSaveDetails->SaveInfoA[i].blocksUsed * (32 * 1024) );
#endif
#ifdef _DURANGO
                    m_buttonListSaves.addItem(m_pSaveDetails->SaveInfoA[i].UTF16SaveTitle, L"");

                    m_saveDetails[i].saveId = i;
                    memcpy(m_saveDetails[i].UTF16SaveName, m_pSaveDetails->SaveInfoA[i].UTF16SaveTitle, 128);
                    memcpy(m_saveDetails[i].UTF16SaveFilename, m_pSaveDetails->SaveInfoA[i].UTF16SaveFilename, MAX_SAVEFILENAME_LENGTH);
#else
#ifdef _WINDOWS64
                    {
                        int origIdx = sortedIdx[i];
                        wchar_t wFilename[MAX_SAVEFILENAME_LENGTH];
                        ZeroMemory(wFilename, sizeof(wFilename));
                        mbstowcs(wFilename, m_pSaveDetails->SaveInfoA[origIdx].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH - 1);
                        wstring filePath = wstring(L"Windows64\\GameHDD\\") + wstring(wFilename) + wstring(L"\\saveData.ms");
                        wstring levelName = ReadLevelNameFromSaveFile(filePath);
                        if (!levelName.empty())
                        {
                            m_buttonListSaves.addItem(levelName, wstring(L""));
                            wcstombs(m_saveDetails[i].UTF8SaveName, levelName.c_str(), 127);
                            m_saveDetails[i].UTF8SaveName[127] = '\0';
                        }
                        else
                        {
                            m_buttonListSaves.addItem(m_pSaveDetails->SaveInfoA[origIdx].UTF8SaveTitle, L"");
                            memcpy(m_saveDetails[i].UTF8SaveName, m_pSaveDetails->SaveInfoA[origIdx].UTF8SaveTitle, 128);
                        }
                        m_saveDetails[i].saveId = origIdx;
                        memcpy(m_saveDetails[i].UTF8SaveFilename, m_pSaveDetails->SaveInfoA[origIdx].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH);
                    }
#else
                    m_buttonListSaves.addItem(m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, L"");
                    memcpy(m_saveDetails[i].UTF8SaveName, m_pSaveDetails->SaveInfoA[i].UTF8SaveTitle, 128);
                    m_saveDetails[i].saveId = i;
                    memcpy(m_saveDetails[i].UTF8SaveFilename, m_pSaveDetails->SaveInfoA[i].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH);
#endif
#endif
                }
#ifdef _WINDOWS64
                delete[] sortedIdx;
#endif
                m_controlSavesTimer.setVisible( false );

                // set focus on the first button

			}
        }

        if(!m_bExitScene && m_bSavesDisplayed && !m_bRetrievingSaveThumbnails && !m_bAllLoaded)
        {
            if( m_iRequestingThumbnailId < (m_buttonListSaves.getItemCount() - m_iDefaultButtonsC ))
            {
                m_bRetrievingSaveThumbnails = true;
                app.DebugPrintf("Requesting the first thumbnail\n");
                // set the save to load
                PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
#ifdef _WINDOWS64
                C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[m_saveDetails[m_iRequestingThumbnailId].saveId],&LoadSaveDataThumbnailReturned,this);
#else
                C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[(int)m_iRequestingThumbnailId],&LoadSaveDataThumbnailReturned,this);
#endif

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
                m_buttonListSaves.setTextureName(m_iRequestingThumbnailId + m_iDefaultButtonsC, (wchar_t *)u16Message);

                ++m_iRequestingThumbnailId;
                if( m_iRequestingThumbnailId < (m_buttonListSaves.getItemCount() - m_iDefaultButtonsC ))
                {
                    app.DebugPrintf("Requesting another thumbnail\n");
                    // set the save to load
                    PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
#ifdef _WINDOWS64
                    C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[m_saveDetails[m_iRequestingThumbnailId].saveId],&LoadSaveDataThumbnailReturned,this);
#else
                    C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(&pSaveDetails->SaveInfoA[(int)m_iRequestingThumbnailId],&LoadSaveDataThumbnailReturned,this);
#endif
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
    case e_SavesRepopulate:
        m_bIgnoreInput = false;
        m_iState=e_SavesIdle;
        m_bAllLoaded=false;
        m_bRetrievingSaveThumbnails=false;
        m_iRequestingThumbnailId = 0;
        GetSaveInfo();
        break;
	case e_SavesRepopulateAfterMashupHide:
        m_bIgnoreInput = false;
        m_iRequestingThumbnailId = 0;
        m_bAllLoaded=false;
        m_bRetrievingSaveThumbnails=false;
        m_bSavesDisplayed=false;
        m_iSaveInfoC=0;
        m_buttonListSaves.clearList();
        GetSaveInfo();
        m_iState=e_SavesIdle;
		break;
    case e_SavesRepopulateAfterDelete:
	case e_SavesRepopulateAfterTransferDownload:
        m_bIgnoreInput = false;
        m_iRequestingThumbnailId = 0;
        m_bAllLoaded=false;
        m_bRetrievingSaveThumbnails=false;
        m_bSavesDisplayed=false;
        m_iSaveInfoC=0;
        m_buttonListSaves.clearList();
        StorageManager.ClearSavesInfo();
        GetSaveInfo();
        m_iState=e_SavesIdle;
        break;
    }
#else
    if(!m_bSavesDisplayed)
    {
        AddDefaultButtons();
        m_bSavesDisplayed=true;
        m_controlSavesTimer.setVisible( false );
    }
#endif

#ifdef _XBOX_ONE
	if(g_NetworkManager.ShouldMessageForFullSession())
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;
		ui.RequestErrorMessage( IDS_CONNECTION_FAILED, IDS_IN_PARTY_SESSION_FULL, uiIDA,1,ProfileManager.GetPrimaryPad());
	}
#endif

    // SAVE TRANSFERS
#ifdef __ORBIS__
	// check the status of the PSPlus common dialog
	switch (sceNpCommerceDialogUpdateStatus())
	{
	case SCE_COMMON_DIALOG_STATUS_FINISHED:
		{
			SceNpCommerceDialogResult Result;
			sceNpCommerceDialogGetResult(&Result);
			sceNpCommerceDialogTerminate();

			if(Result.authorized)
			{
				// they just became a PSPlus member
				ProfileManager.PsPlusUpdate(ProfileManager.GetPrimaryPad(), &Result);

			}
			else
			{

			}

			// 4J-JEV: Fix for PS4 #5148 - [ONLINE] If the user attempts to join a game when they do not have Playstation Plus, the title will lose all functionality.
			m_bIgnoreInput = false;
		}
		break;
	default:
		break;
	}
#endif

}

void UIScene_LoadOrJoinMenu::GetSaveInfo()
{
    unsigned int uiSaveC=0;

    // This will return with the number retrieved in uiSaveC

    if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
    {
#ifdef __ORBIS__
		// We need to make sure this is non-null so that we have an idea of free space
        m_pSaveDetails=StorageManager.ReturnSavesInfo();
        if(m_pSaveDetails==NULL)
        {
            C4JStorage::ESaveGameState eSGIStatus= StorageManager.GetSavesInfo(m_iPad,NULL,this,"save");
        }
#endif

        uiSaveC = 0;
#ifdef _XBOX
        File savesDir(L"GAME:\\Saves");
#else
        File savesDir(L"Saves");
#endif
        if( savesDir.exists() )
        {
            m_saves = savesDir.listFiles();
            uiSaveC = (unsigned int)m_saves->size();
        }
        // add the New Game and Tutorial after the saves list is retrieved, if there are any saves

        // Add two for New Game and Tutorial
        unsigned int listItems = uiSaveC;

        AddDefaultButtons();

        for(unsigned int i=0;i<listItems;i++)
        {

            wstring wName = m_saves->at(i)->getName();
            wchar_t *name = new wchar_t[wName.size()+1];
            for(unsigned int j = 0; j < wName.size(); ++j)
            {
                name[j] = wName[j];
            }
            name[wName.size()] = 0;
            m_buttonListSaves.addItem(name,L"");
        }
        m_bSavesDisplayed = true;
        m_bAllLoaded = true;
        m_bIgnoreInput = false;
    }
    else
    {
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

#if TO_BE_IMPLEMENTED
        if(eSGIStatus==C4JStorage::ESGIStatus_NoSaves)
        {
            uiSaveC=0;
            m_controlSavesTimer.setVisible( false );
            m_SavesList.SetEnable(TRUE);
        }
#endif
    }

    return;
}

void UIScene_LoadOrJoinMenu::AddDefaultButtons()
{
    m_iDefaultButtonsC = 0;
	m_iMashUpButtonsC=0;
	m_generators.clear();

	m_buttonListSaves.addItem(app.GetString(IDS_CREATE_NEW_WORLD));
    m_iDefaultButtonsC++;

    int i = 0;

    for ( LevelGenerationOptions *levelGen : *app.getLevelGenerators() )
    {
        // retrieve the save icon from the texture pack, if there is one
        unsigned int uiTexturePackID=levelGen->getRequiredTexturePackId();

		if(uiTexturePackID!=0)
		{
			unsigned int uiMashUpWorldsBitmask=app.GetMashupPackWorlds(m_iPad);

			if((uiMashUpWorldsBitmask & (1<<(uiTexturePackID-1024)))==0)
			{
				// this world is hidden, so skip
				continue;
			}
		}

		// 4J-JEV: For debug. Ignore worlds with no name.
		LPCWSTR wstr = levelGen->getWorldName();
		m_buttonListSaves.addItem( wstr );
		m_generators.push_back(levelGen);

        if(uiTexturePackID!=0)
        {
			// increment the count of the mash-up pack worlds in the save list
			m_iMashUpButtonsC++;
            TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackById(levelGen->getRequiredTexturePackId());
            DWORD dwImageBytes;
            PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

            if(dwImageBytes > 0 && pbImageData)
            {
                wchar_t imageName[64];
                swprintf(imageName,64,L"tpack%08x",tp->getId());
                registerSubstitutionTexture(imageName, pbImageData, dwImageBytes);
                m_buttonListSaves.setTextureName( m_buttonListSaves.getItemCount() - 1, imageName );
            }
        }

        ++i;
    }
    m_iDefaultButtonsC += i;
}

void UIScene_LoadOrJoinMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
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
    case ACTION_MENU_X:
#if TO_BE_IMPLEMENTED
        // Change device
        // Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage
        // device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
        m_bIgnoreInput=true;
        StorageManager.SetSaveDevice(&CScene_MultiGameJoinLoad::DeviceSelectReturned,this,true);
        ui.PlayUISFX(eSFX_Press);
#endif
        // Save Transfer
#ifdef _XBOX_ONE
		if(ProfileManager.IsSignedInLive( m_iPad ))
		{
			UIScene_LoadOrJoinMenu::s_ulFileSize=0;
			LaunchSaveTransfer();
		}
#endif
#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
		{
			bool bSignedInLive = ProfileManager.IsSignedInLive(iPad);
			if(bSignedInLive)
			{
				LaunchSaveTransfer();
			}
		}
#endif
        break;
    case ACTION_MENU_Y:
#if defined(__PS3__) || defined(__PSVITA__) || defined(__ORBIS__)
		m_eAction = eAction_ViewInvites;
        if(pressed && iPad == ProfileManager.GetPrimaryPad())
        {
#ifdef __ORBIS__
			// Check if PSN is unavailable because of age restriction
			int npAvailability = ProfileManager.getNPAvailability(iPad);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				UINT uiIDA[1];
				uiIDA[0] = IDS_OK;
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad);

				break;
			}
#endif

            // are we offline?
            if(!ProfileManager.IsSignedInLive(iPad))
            {
                // get them to sign in to online
                UINT uiIDA[2];
                uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
                uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
                ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(), &UIScene_LoadOrJoinMenu::MustSignInReturnedPSN, this);
            }
            else
            {
#ifdef __ORBIS__
                SQRNetworkManager_Orbis::RecvInviteGUI();
#elif defined __PSVITA__
                SQRNetworkManager_Vita::RecvInviteGUI();
#else
                int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
                app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )\n", ret, ret);
#endif
            }
        }
#elif defined(_DURANGO)
        if(getControlFocus() == eControl_GamesList && m_buttonListGames.getItemCount() > 0)
        {
            DWORD nIndex = m_buttonListGames.getCurrentSelection();
            FriendSessionInfo *pSelectedSession = m_currentSessions->at( nIndex );

            PlayerUID uid = pSelectedSession->searchResult.m_playerXuids[0];
            if( uid != INVALID_XUID ) ProfileManager.ShowProfileCard(ProfileManager.GetLockedProfile(),uid);
            ui.PlayUISFX(eSFX_Press);
        }
#endif // __PS3__ || __ORBIS__
        break;

    case ACTION_MENU_RIGHT_SCROLL:
        if(DoesSavesListHaveFocus())
        {
            // 4J-PB - check we are on a valid save
            if((m_iDefaultButtonsC != 0) && (m_iSaveListIndex >= m_iDefaultButtonsC))
            {
                m_bIgnoreInput = true;

                // Could be delete save or Save Options
                if(StorageManager.GetSaveDisabled())
                {
                    // delete the save game
                    // Have to ask the player if they are sure they want to delete this game
                    UINT uiIDA[2];
                    uiIDA[0]=IDS_CONFIRM_CANCEL;
                    uiIDA[1]=IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, iPad,&UIScene_LoadOrJoinMenu::DeleteSaveDialogReturned,this);
                }
                else
                {
                    if(StorageManager.EnoughSpaceForAMinSaveGame())
                    {
                        UINT uiIDA[4];
                        uiIDA[0]=IDS_CONFIRM_CANCEL;
                        uiIDA[1]=IDS_TITLE_RENAMESAVE;
                        uiIDA[2]=IDS_TOOLTIPS_DELETESAVE;
                        int numOptions = 3;
#ifdef SONY_REMOTE_STORAGE_UPLOAD
                        if(ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
                        {
                            numOptions = 4;
                            uiIDA[3]=IDS_TOOLTIPS_SAVETRANSFER_UPLOAD;
                        }
#endif
#if defined _XBOX_ONE || defined __ORBIS__
                            numOptions = 4;
                            uiIDA[3]=IDS_COPYSAVE;
#endif
                        ui.RequestAlertMessage(IDS_TOOLTIPS_SAVEOPTIONS, IDS_TEXT_SAVEOPTIONS, uiIDA, numOptions, iPad,&UIScene_LoadOrJoinMenu::SaveOptionsDialogReturned,this);
                    }
                    else
                    {
                        // delete the save game
                        // Have to ask the player if they are sure they want to delete this game
                        UINT uiIDA[2];
                        uiIDA[0]=IDS_CONFIRM_CANCEL;
                        uiIDA[1]=IDS_CONFIRM_OK;
                        ui.RequestAlertMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2,iPad,&UIScene_LoadOrJoinMenu::DeleteSaveDialogReturned,this);
                    }
                }
                ui.PlayUISFX(eSFX_Press);
            }
        }
		else if(DoesMashUpWorldHaveFocus())
		{
			// hiding a mash-up world
			if((m_iSaveListIndex != JOIN_LOAD_CREATE_BUTTON_INDEX))
			{
				LevelGenerationOptions *levelGen = m_generators.at(m_iSaveListIndex - 1);

				if(!levelGen->isTutorial())
				{
					if(levelGen->requiresTexturePack())
					{
						unsigned int uiPackID=levelGen->getRequiredTexturePackId();

						m_bIgnoreInput = true;
						app.HideMashupPackWorld(m_iPad,uiPackID);

						// update the saves list
						m_iState = e_SavesRepopulateAfterMashupHide;
					}
				}
			}
			ui.PlayUISFX(eSFX_Press);

		}
        break;
    case ACTION_MENU_LEFT_SCROLL:
#ifdef _XBOX
        if( m_bInParty )
        {
            m_bShowingPartyGamesOnly = !m_bShowingPartyGamesOnly;
            UpdateGamesList();
            CXuiSceneBase::PlayUISFX(eSFX_Press);
        }
#endif
        break;
    case ACTION_MENU_LEFT:
    case ACTION_MENU_RIGHT:
        {
            // if we are on the saves menu, check there are games in the games list to move to
            if(DoesSavesListHaveFocus())
            {
                if( m_buttonListGames.getItemCount() > 0)
                {
                    sendInputToMovie(key, repeat, pressed, released);
                }
            }
            else
            {
                sendInputToMovie(key, repeat, pressed, released);
            }
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
    case ACTION_MENU_OTHER_STICK_UP:
        sendInputToMovie(ACTION_MENU_UP, repeat, pressed, released);
        handled = true;
        break;
    case ACTION_MENU_OTHER_STICK_DOWN:
        sendInputToMovie(ACTION_MENU_DOWN, repeat, pressed, released);
        handled = true;
        break;
    }
}

int UIScene_LoadOrJoinMenu::KeyboardCompleteWorldNameCallback(LPVOID lpParam,bool bRes)
{
    // 4J HEG - No reason to set value if keyboard was cancelled
    UIScene_LoadOrJoinMenu *pClass=(UIScene_LoadOrJoinMenu *)lpParam;
	pClass->m_bIgnoreInput=false;
    if (bRes)
    {
        uint16_t ui16Text[128];
        ZeroMemory(ui16Text, 128 * sizeof(uint16_t) );
#ifdef _WINDOWS64
        Win64_GetKeyboardText(ui16Text, 128);
#else
        InputManager.GetText(ui16Text);
#endif

        // check the name is valid
        if(ui16Text[0]!=0)
        {
#if (defined __PS3__ || defined __ORBIS__ || defined _DURANGO  || defined(__PSVITA__))
            // open the save and overwrite the metadata
            StorageManager.RenameSaveData(pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC, ui16Text,&UIScene_LoadOrJoinMenu::RenameSaveDataReturned,pClass);
#elif defined(_WINDOWS64)
            {
                int listPos = pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC;

                // Convert the ui16Text input to a wide string
                wchar_t wNewName[128] = {};
                for (int k = 0; k < 127 && ui16Text[k]; k++)
                    wNewName[k] = (wchar_t)ui16Text[k];

                // Convert to narrow for storage and in-memory update
                char narrowName[128] = {};
                wcstombs(narrowName, wNewName, 127);

                // Build the sidecar path: Windows64\GameHDD\{folder}\worldname.txt
                wchar_t wFilename[MAX_SAVEFILENAME_LENGTH] = {};
                mbstowcs(wFilename, pClass->m_saveDetails[listPos].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH - 1);
                wstring sidecarPath = wstring(L"Windows64\\GameHDD\\") + wstring(wFilename) + wstring(L"\\worldname.txt");

                FILE *fw = NULL;
                if (_wfopen_s(&fw, sidecarPath.c_str(), L"w") == 0 && fw)
                {
                    fputs(narrowName, fw);
                    fclose(fw);
                }

                // Update the in-memory display name so the list reflects it immediately
                strncpy_s(pClass->m_saveDetails[listPos].UTF8SaveName, narrowName, 127);
                pClass->m_saveDetails[listPos].UTF8SaveName[127] = '\0';

                // Reuse the existing callback to trigger the list repopulate
                UIScene_LoadOrJoinMenu::RenameSaveDataReturned(pClass, true);
            }
#endif
        }
        else
        {
            pClass->m_bIgnoreInput=false;
            pClass->updateTooltips();
        }
    }
    else
    {
        pClass->m_bIgnoreInput=false;
        pClass->updateTooltips();
    }


    return 0;
}
void UIScene_LoadOrJoinMenu::handleInitFocus(F64 controlId, F64 childId)
{
    app.DebugPrintf(app.USER_SR, "UIScene_LoadOrJoinMenu::handleInitFocus - %d , %d\n", (int)controlId, (int)childId);
}

void UIScene_LoadOrJoinMenu::handleFocusChange(F64 controlId, F64 childId)
{
    app.DebugPrintf(app.USER_SR, "UIScene_LoadOrJoinMenu::handleFocusChange - %d , %d\n", (int)controlId, (int)childId);

	switch((int)controlId)
	{
	case eControl_GamesList:
		m_iGameListIndex = childId;
		m_buttonListGames.updateChildFocus( (int) childId );
		break;
	case eControl_SavesList:
		m_iSaveListIndex = childId;
        m_bUpdateSaveSize = true;
		break;
	};
    updateTooltips();
}


#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
void UIScene_LoadOrJoinMenu::remoteStorageGetSaveCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
    app.DebugPrintf("remoteStorageGetCallback err : 0x%08x\n", error_code);
    assert(error_code == 0);
    ((UIScene_LoadOrJoinMenu*)lpParam)->LoadSaveFromCloud();
}
#endif

void UIScene_LoadOrJoinMenu::handlePress(F64 controlId, F64 childId)
{
    switch((int)controlId)
    {
    case eControl_SavesList:
        {
            m_bIgnoreInput=true;

            int lGenID = (int)childId - 1;

            //CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

            if((int)childId == JOIN_LOAD_CREATE_BUTTON_INDEX)
            {
                app.SetTutorialMode( false );

                m_controlJoinTimer.setVisible( false );

                app.SetCorruptSaveDeleted(false);

                CreateWorldMenuInitData *params = new CreateWorldMenuInitData();
                params->iPad = m_iPad;
                ui.NavigateToScene(m_iPad,eUIScene_CreateWorldMenu,(void *)params);
            }
            else if (lGenID < m_generators.size())
            {
                LevelGenerationOptions *levelGen = m_generators.at(lGenID);
                app.SetTutorialMode( levelGen->isTutorial() );
                // Reset the autosave time
                app.SetAutosaveTimerTime();

                if(levelGen->isTutorial())
                {
                    LoadLevelGen(levelGen);
                }
                else
                {
                    LoadMenuInitData *params = new LoadMenuInitData();
                    params->iPad = m_iPad;
                    // need to get the iIndex from the list item, since the position in the list doesn't correspond to the GetSaveGameInfo list because of sorting
                    params->iSaveGameInfoIndex=-1;
                    //params->pbSaveRenamed=&m_bSaveRenamed;
                    params->levelGen = levelGen;
                    params->saveDetails = NULL;

                    // navigate to the settings scene
                    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LoadMenu, params);
                }
            }
            else
            {
#ifdef __ORBIS__
                // check if this is a damaged save
				PSAVE_INFO pSaveInfo = &m_pSaveDetails->SaveInfoA[((int)childId)-m_iDefaultButtonsC];
                if(pSaveInfo->thumbnailData == NULL && pSaveInfo->modifiedTime == 0)		// no thumbnail data and time of zero and zero blocks useset for corrupt files
                {
                    // give the option to delete the save
                    UINT uiIDA[2];
                    uiIDA[0]=IDS_CONFIRM_CANCEL;
                    uiIDA[1]=IDS_CONFIRM_OK;
                    ui.RequestAlertMessage(IDS_CORRUPT_OR_DAMAGED_SAVE_TITLE, IDS_CORRUPT_OR_DAMAGED_SAVE_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_LoadOrJoinMenu::DeleteSaveDialogReturned,this);

                }
                else
#endif
                {
                    app.SetTutorialMode( false );

                    if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
                    {
                        LoadSaveFromDisk(m_saves->at((int)childId-m_iDefaultButtonsC));
                    }
                    else
                    {
                        LoadMenuInitData *params = new LoadMenuInitData();
                        params->iPad = m_iPad;
                        // need to get the iIndex from the list item, since the position in the list doesn't correspond to the GetSaveGameInfo list because of sorting
                        params->iSaveGameInfoIndex=m_saveDetails[((int)childId)-m_iDefaultButtonsC].saveId;
                        //params->pbSaveRenamed=&m_bSaveRenamed;
                        params->levelGen = NULL;
                        params->saveDetails = &m_saveDetails[ ((int)childId)-m_iDefaultButtonsC ];

#ifdef _XBOX_ONE
                        // On XB1, saves might need syncing, in which case inform the user so they can decide whether they want to wait for this to happen
                        if( m_pSaveDetails->SaveInfoA[params->iSaveGameInfoIndex].needsSync )
                        {
                            unsigned int uiIDA[2];
                            uiIDA[0]=IDS_CONFIRM_SYNC;
                            uiIDA[1]=IDS_CONFIRM_CANCEL;

                            m_loadMenuInitData = params;
                            ui.RequestAlertMessage(IDS_LOAD_SAVED_WORLD, IDS_CONFIRM_SYNC_REQUIRED, uiIDA, 2, ProfileManager.GetPrimaryPad(),&NeedSyncMessageReturned,this);
                        }
                        else
#endif
                        {
                            // navigate to the settings scene
                            ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_LoadMenu, params);
                        }
                    }
                }
            }
        }
        break;
    case eControl_GamesList:
        {
            m_bIgnoreInput=true;

			m_eAction = eAction_JoinGame;

            //CD - Added for audio
            ui.PlayUISFX(eSFX_Press);

			{
				int nIndex = (int)childId;
				m_iGameListIndex = nIndex;
				CheckAndJoinGame(nIndex);
			}

            break;
        }
    }
}

void UIScene_LoadOrJoinMenu::CheckAndJoinGame(int gameIndex)
{
	if( m_buttonListGames.getItemCount() > 0 && gameIndex < m_currentSessions->size() )
	{
#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
		// 4J-PB - is the player allowed to join games?
		bool noUGC=false;
		bool bContentRestricted=false;

		// we're online, since we are joining a game
		ProfileManager.GetChatAndContentRestrictions(m_iPad,true,&noUGC,&bContentRestricted,NULL);

#ifdef __ORBIS__
		// 4J Stu - On PS4 we don't restrict playing multiplayer based on chat restriction, so remove this check
		noUGC = false;

		bool bPlayStationPlus=true;
		int iPadWithNoPlaystationPlus=0;
		bool isSignedInLive = true;
		int iPadNotSignedInLive = -1;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if( InputManager.IsPadConnected(i) || ProfileManager.IsSignedIn(i) )
			{
				if (isSignedInLive && !ProfileManager.IsSignedInLive(i))
				{
					// Record the first non signed in live pad
					iPadNotSignedInLive = i;
				}

				isSignedInLive = isSignedInLive && ProfileManager.IsSignedInLive(i);
				if(ProfileManager.HasPlayStationPlus(i)==false)
				{
					bPlayStationPlus=false;
					break;
				}
			}
		}
#endif
#ifdef __PSVITA__
		if( CGameNetworkManager::usingAdhocMode() )
		{
			bContentRestricted = false;
			noUGC = false;
		}
#endif

		if(noUGC)
		{
			// not allowed to join
#ifndef __PSVITA__
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;
			// Not allowed to play online
			ui.RequestAlertMessage(IDS_ONLINE_GAME, IDS_CHAT_RESTRICTION_UGC, uiIDA, 1, m_iPad,NULL,this);
#else
			// Not allowed to play online
			ProfileManager.ShowSystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, 0 );
#endif

			m_bIgnoreInput=false;
			return;
		}
		else if(bContentRestricted)
		{
			ui.RequestContentRestrictedMessageBox();

			m_bIgnoreInput=false;
			return;
		}
#ifdef __ORBIS__
		// If this is an online game but not all players are signed in to Live, stop!
		else if (!isSignedInLive)
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_CONFIRM_OK;

			// Check if PSN is unavailable because of age restriction
			int npAvailability = ProfileManager.getNPAvailability(iPadNotSignedInLive);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				m_bIgnoreInput = false;
				// 4J Stu - This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive if the npAvailability isn't SCE_OK
				ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPadNotSignedInLive);
			}
			else
			{
				ui.RequestErrorMessage( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA,1,iPadNotSignedInLive, &UIScene_LoadOrJoinMenu::MustSignInReturnedPSN, this);
			}
			return;
		}
		else if(bPlayStationPlus==false)
		{

			if(ProfileManager.RequestingPlaystationPlus(iPadWithNoPlaystationPlus))
			{
				// MGH -  added this so we don't try and upsell when we don't know if the player has PS Plus yet (if it can't connect to the PS Plus server).
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestAlertMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, ProfileManager.GetPrimaryPad(), NULL, NULL);
				return;
			}

			// PS Plus upsell
			// 4J-PB - we're not allowed to show the text Playstation Plus - have to call the upsell all the time!
			// upsell psplus
			int32_t iResult=sceNpCommerceDialogInitialize();

			SceNpCommerceDialogParam param;
			sceNpCommerceDialogParamInitialize(&param);
			param.mode=SCE_NP_COMMERCE_DIALOG_MODE_PLUS;
			param.features = SCE_NP_PLUS_FEATURE_REALTIME_MULTIPLAY;
			param.userId = ProfileManager.getUserID(iPadWithNoPlaystationPlus);

			iResult=sceNpCommerceDialogOpen(&param);

			//                     UINT uiIDA[2];
			//                     uiIDA[0]=IDS_CONFIRM_OK;
			//                     uiIDA[1]=IDS_PLAYSTATIONPLUS_SIGNUP;
			//                     ui.RequestMessageBox( IDS_FAILED_TO_CREATE_GAME_TITLE, IDS_NO_PLAYSTATIONPLUS, uiIDA,2,ProfileManager.GetPrimaryPad(),&UIScene_LoadOrJoinMenu::PSPlusReturned,this, app.GetStringTable(),NULL,0,false);

			m_bIgnoreInput=false;
			return;
		}

#endif
#endif

		//CScene_MultiGameInfo::JoinMenuInitData *initData = new CScene_MultiGameInfo::JoinMenuInitData();
		m_initData->iPad = 0;;
		m_initData->selectedSession = m_currentSessions->at( gameIndex );

		// check that we have the texture pack available
		// If it's not the default texture pack
		if(m_initData->selectedSession->data.texturePackParentId!=0)
		{
			int texturePacksCount = Minecraft::GetInstance()->skins->getTexturePackCount();
			bool bHasTexturePackInstalled=false;

			for(int i=0;i<texturePacksCount;i++)
			{
				TexturePack *tp = Minecraft::GetInstance()->skins->getTexturePackByIndex(i);
				if(tp->getDLCParentPackId()==m_initData->selectedSession->data.texturePackParentId)
				{
					bHasTexturePackInstalled=true;
					break;
				}
			}

			if(bHasTexturePackInstalled==false)
			{
				// upsell the texture pack
				// tell sentient about the upsell of the full version of the skin pack
#ifdef _XBOX
				ULONGLONG ullOfferID_Full;
				app.GetDLCFullOfferIDForPackID(m_initData->selectedSession->data.texturePackParentId,&ullOfferID_Full);

				TelemetryManager->RecordUpsellPresented(m_iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif
				UINT uiIDA[2];

				uiIDA[0]=IDS_TEXTUREPACK_FULLVERSION;
				//uiIDA[1]=IDS_TEXTURE_PACK_TRIALVERSION;
				uiIDA[1]=IDS_CONFIRM_CANCEL;


				// Give the player a warning about the texture pack missing
				ui.RequestAlertMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE, IDS_DLC_TEXTUREPACK_NOT_PRESENT, uiIDA, 2, m_iPad,&UIScene_LoadOrJoinMenu::TexturePackDialogReturned,this);

				return;
			}

#ifdef __PSVITA__
			if(CGameNetworkManager::usingAdhocMode() && !SQRNetworkManager_AdHoc_Vita::GetAdhocStatus())
			{
				// not connected to adhoc anymore, must have connected back to PSN to buy texture pack so sign in again
				SQRNetworkManager_AdHoc_Vita::AttemptAdhocSignIn(&UIScene_LoadOrJoinMenu::SignInAdhocReturned, this);
				return;
		}
#endif
		}
		m_controlJoinTimer.setVisible( false );

#ifdef _XBOX
		// Reset the background downloading, in case we changed it by attempting to download a texture pack
		XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
#endif

		m_bIgnoreInput=true;
		ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_JoinMenu,m_initData);
	}
}

void UIScene_LoadOrJoinMenu::LoadLevelGen(LevelGenerationOptions *levelGen)
{
    // Load data from disc
    //File saveFile( L"Tutorial\\Tutorial" );
    //LoadSaveFromDisk(&saveFile);

    // clear out the app's terrain features list
    app.ClearTerrainFeaturePosition();

    StorageManager.ResetSaveData();
    // Make our next save default to the name of the level
    StorageManager.SetSaveTitle(levelGen->getDefaultSaveName().c_str());

    bool isClientSide = false;
    bool isPrivate = false;
    // TODO int maxPlayers = MINECRAFT_NET_MAX_PLAYERS;
    int maxPlayers = 8;

    if( app.GetTutorialMode() )
    {
        isClientSide = false;
        maxPlayers = 4;
    }

    g_NetworkManager.HostGame(0,isClientSide,isPrivate,maxPlayers,0);

    NetworkGameInitData *param = new NetworkGameInitData();
    param->seed = 0;
    param->saveData = NULL;
    param->settings = app.GetGameHostOption( eGameHostOption_Tutorial );
    param->levelGen = levelGen;

    if(levelGen->requiresTexturePack())
    {
        param->texturePackId = levelGen->getRequiredTexturePackId();

        Minecraft *pMinecraft = Minecraft::GetInstance();
        pMinecraft->skins->selectTexturePackById(param->texturePackId);
        //pMinecraft->skins->updateUI();
    }

#ifndef _XBOX
    g_NetworkManager.FakeLocalPlayerJoined();
#endif

    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = (LPVOID)param;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

void UIScene_LoadOrJoinMenu::UpdateGamesListCallback(LPVOID pParam)
{
    if(pParam != NULL)
    {
        UIScene_LoadOrJoinMenu *pScene = (UIScene_LoadOrJoinMenu *)pParam;
        pScene->UpdateGamesList();
    }
}

void UIScene_LoadOrJoinMenu::UpdateGamesList()
{
    // If we're ignoring input scene isn't active so do nothing
    if (m_bIgnoreInput) return;

    // If a texture pack is loading, or will be loading, then ignore this ( we are going to be destroyed anyway)
    if( Minecraft::GetInstance()->skins->getSelected()->isLoadingData() || (Minecraft::GetInstance()->skins->needsUIUpdate() || ui.IsReloadingSkin()) ) return;

    // if we're retrieving save info, don't show the list yet as we will be ignoring press events
    if(!m_bSavesDisplayed)
    {
        return;
    }


    FriendSessionInfo *pSelectedSession = NULL;
    if(DoesGamesListHaveFocus() && m_buttonListGames.getItemCount() > 0)
    {
        unsigned int nIndex = m_buttonListGames.getCurrentSelection();
        pSelectedSession = m_currentSessions->at( nIndex );
    }

    SessionID selectedSessionId;
	ZeroMemory(&selectedSessionId,sizeof(SessionID));
    if( pSelectedSession != NULL )selectedSessionId = pSelectedSession->sessionId;
    pSelectedSession = NULL;

    m_controlJoinTimer.setVisible( false );

    // if the saves list has focus, then we should show the Delete Save tooltip
    // if the games list has focus, then we should show the View Gamercard tooltip
    int iRB=-1;
    int iY = -1;
    int iX=-1;

    delete m_currentSessions;
    m_currentSessions = g_NetworkManager.GetSessionList( m_iPad, 1, m_bShowingPartyGamesOnly );

    // Update the xui list displayed
    unsigned int xuiListSize = m_buttonListGames.getItemCount();
    unsigned int filteredListSize = (unsigned int)m_currentSessions->size();

    BOOL gamesListHasFocus = DoesGamesListHaveFocus();

    if(filteredListSize > 0)
    {
#if TO_BE_IMPLEMENTED
        if( !m_pGamesList->IsEnabled() )
        {
            m_pGamesList->SetEnable(TRUE);
            m_pGamesList->SetCurSel( 0 );
        }
#endif
        m_labelNoGames.setVisible( false );
        m_controlJoinTimer.setVisible( false );
    }
    else
    {
#if TO_BE_IMPLEMENTED
        m_pGamesList->SetEnable(FALSE);
#endif
        m_controlJoinTimer.setVisible( false );
        m_labelNoGames.setVisible( true );

#if TO_BE_IMPLEMENTED
        if( gamesListHasFocus ) m_pGamesList->InitFocus(m_iPad);
#endif
    }

    // clear out the games list and re-fill
    m_buttonListGames.clearList();

    if( filteredListSize > 0 )
    {
        // Reset the focus to the selected session if it still exists
        unsigned int sessionIndex = 0;
        m_buttonListGames.setCurrentSelection(0);

        for( FriendSessionInfo *sessionInfo : *m_currentSessions )
        {
            wchar_t textureName[64] = L"\0";

            // Is this a default game or a texture pack game?
            if(sessionInfo->data.texturePackParentId!=0)
            {
                // Do we have the texture pack
                Minecraft *pMinecraft = Minecraft::GetInstance();
                TexturePack *tp = pMinecraft->skins->getTexturePackById(sessionInfo->data.texturePackParentId);
                HRESULT hr;

                DWORD dwImageBytes=0;
                PBYTE pbImageData=NULL;

                if(tp==NULL)
                {
                    DWORD dwBytes=0;
                    PBYTE pbData=NULL;
                    app.GetTPD(sessionInfo->data.texturePackParentId,&pbData,&dwBytes);

                    // is it in the tpd data ?
                    app.GetFileFromTPD(eTPDFileType_Icon,pbData,dwBytes,&pbImageData,&dwImageBytes );
                    if(dwImageBytes > 0 && pbImageData)
                    {
                        swprintf(textureName,64,L"%ls",sessionInfo->displayLabel);
                        registerSubstitutionTexture(textureName,pbImageData,dwImageBytes);
                    }
                }
                else
                {
                    pbImageData = tp->getPackIcon(dwImageBytes);
                    if(dwImageBytes > 0 && pbImageData)
                    {
                        swprintf(textureName,64,L"%ls",sessionInfo->displayLabel);
                        registerSubstitutionTexture(textureName,pbImageData,dwImageBytes);
                    }
                }
            }
            else
            {
                // default texture pack
                Minecraft *pMinecraft = Minecraft::GetInstance();
                TexturePack *tp = pMinecraft->skins->getTexturePackByIndex(0);

                DWORD dwImageBytes;
                PBYTE pbImageData = tp->getPackIcon(dwImageBytes);

                if(dwImageBytes > 0 && pbImageData)
                {
                    swprintf(textureName,64,L"%ls",sessionInfo->displayLabel);
                    registerSubstitutionTexture(textureName,pbImageData,dwImageBytes);
                }
            }

            m_buttonListGames.addItem( sessionInfo->displayLabel, textureName );

            if(memcmp( &selectedSessionId, &sessionInfo->sessionId, sizeof(SessionID) ) == 0)
            {
                m_buttonListGames.setCurrentSelection(sessionIndex);
                break;
            }
            ++sessionIndex;
        }
    }

	updateTooltips();
}

void UIScene_LoadOrJoinMenu::HandleDLCMountingComplete()
{
    Initialise();
}

bool UIScene_LoadOrJoinMenu::DoesSavesListHaveFocus()
{
	if( m_buttonListSaves.hasFocus() )
	{
		// check it's not the first or second element (new world or tutorial)
		if(m_iSaveListIndex > (m_iDefaultButtonsC-1))
		{
			return true;
		}
	}
	return false;
}

bool UIScene_LoadOrJoinMenu::DoesMashUpWorldHaveFocus()
{
	if(m_buttonListSaves.hasFocus())
	{
		// check it's not the first or second element (new world or tutorial)
		if(m_iSaveListIndex > (m_iDefaultButtonsC - 1))
		{
			return false;
		}

		if(m_iSaveListIndex > (m_iDefaultButtonsC - 1 - m_iMashUpButtonsC))
		{
			return true;
		}
		else return false;
	}
	else return false;
}

bool UIScene_LoadOrJoinMenu::DoesGamesListHaveFocus()
{
    return m_buttonListGames.hasFocus();
}

void UIScene_LoadOrJoinMenu::handleTimerComplete(int id)
{
    switch(id)
    {
    case JOIN_LOAD_ONLINE_TIMER_ID:
        {
#ifdef _XBOX
            XPARTY_USER_LIST partyList;

            if((XPartyGetUserList(  &partyList ) != XPARTY_E_NOT_IN_PARTY ) && (partyList.dwUserCount>1))
            {
                m_bInParty=true;
            }
            else
            {
                m_bInParty=false;
            }
#endif

            bool bMultiplayerAllowed = ProfileManager.IsSignedInLive( m_iPad ) && ProfileManager.AllowedToPlayMultiplayer(m_iPad);
            if(bMultiplayerAllowed != m_bMultiplayerAllowed)
            {
                if( bMultiplayerAllowed )
                {
                    // 					m_CheckboxOnline.SetEnable(TRUE);
                    // 					m_CheckboxPrivate.SetEnable(TRUE);
                }
                else
                {
                    m_bInParty = false;
                    m_buttonListGames.clearList();
                    m_controlJoinTimer.setVisible( true );
                    m_labelNoGames.setVisible( false );
                }

                m_bMultiplayerAllowed = bMultiplayerAllowed;
            }
        }
        break;
        // 4J-PB - Only Xbox will not have trial DLC patched into the game
#ifdef _XBOX
    case CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID:
        {

#if defined(__PS3__) || defined(__ORBIS__) || defined(__PSVITA__)
            for(int i=0;i<m_iTexturePacksNotInstalled;i++)
            {
                if(m_iConfigA[i]!=-1)
                {
                    DLC_INFO *pDLCInfo=app.GetDLCInfoFromTPackID(m_iConfigA[i]);

                    if(pDLCInfo)
                    {
                        // retrieve the image - if we haven't already
                        wstring textureName = filenametowstring(pDLCInfo->chImageURL);

                        if(hasRegisteredSubstitutionTexture(textureName)==false)
                        {
                            PBYTE pbImageData;
                            int iImageDataBytes=0;
                            SonyHttp::getDataFromURL(pDLCInfo->chImageURL,(void **)&pbImageData,&iImageDataBytes);

                            if(iImageDataBytes!=0)
                            {
                                // set the image
                                registerSubstitutionTexture(textureName,pbImageData,iImageDataBytes,true);
                                m_iConfigA[i]=-1;
                            }

                        }
                    }
                }
            }

            bool bAllDone=true;
            for(int i=0;i<m_iTexturePacksNotInstalled;i++)
            {
                if(m_iConfigA[i]!=-1)
                {
                    bAllDone = false;
                }
            }

            if(bAllDone)
            {
                // kill this timer
                killTimer(CHECKFORAVAILABLETEXTUREPACKS_TIMER_ID);
            }
#endif

        }
        break;
#endif
    }

}

void UIScene_LoadOrJoinMenu::LoadSaveFromDisk(File *saveFile, ESavePlatform savePlatform /*= SAVE_FILE_PLATFORM_LOCAL*/)
{
    // we'll only be coming in here when the tutorial is loaded now

    StorageManager.ResetSaveData();

    // Make our next save default to the name of the level
    StorageManager.SetSaveTitle(saveFile->getName().c_str());

    __int64 fileSize = saveFile->length();
    FileInputStream fis(*saveFile);
    byteArray ba(fileSize);
    fis.read(ba);
    fis.close();



    bool isClientSide = false;
    bool isPrivate = false;
    int maxPlayers = MINECRAFT_NET_MAX_PLAYERS;

    if( app.GetTutorialMode() )
    {
        isClientSide = false;
        maxPlayers = 4;
    }

    app.SetGameHostOption(eGameHostOption_GameType,GameType::CREATIVE->getId() );

    g_NetworkManager.HostGame(0,isClientSide,isPrivate,maxPlayers,0);

    LoadSaveDataThreadParam *saveData = new LoadSaveDataThreadParam(ba.data, ba.length, saveFile->getName());

    NetworkGameInitData *param = new NetworkGameInitData();
    param->seed = 0;
    param->saveData = saveData;
    param->settings = app.GetGameHostOption( eGameHostOption_All );
    param->savePlatform = savePlatform;

#ifndef _XBOX
    g_NetworkManager.FakeLocalPlayerJoined();
#endif

    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = (LPVOID)param;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

#ifdef SONY_REMOTE_STORAGE_DOWNLOAD
void UIScene_LoadOrJoinMenu::LoadSaveFromCloud()
{

    wchar_t wFileName[128];
    mbstowcs(wFileName, app.getRemoteStorage()->getLocalFilename(), strlen(app.getRemoteStorage()->getLocalFilename())+1); // plus null
    File cloudFile(wFileName);


    StorageManager.ResetSaveData();

    // Make our next save default to the name of the level
    wchar_t wSaveName[128];
    mbstowcs(wSaveName, app.getRemoteStorage()->getSaveNameUTF8(), strlen(app.getRemoteStorage()->getSaveNameUTF8())+1); // plus null
    StorageManager.SetSaveTitle(wSaveName);

    __int64 fileSize = cloudFile.length();
    FileInputStream fis(cloudFile);
    byteArray ba(fileSize);
    fis.read(ba);
    fis.close();



    bool isClientSide = false;
    bool isPrivate = false;
    int maxPlayers = MINECRAFT_NET_MAX_PLAYERS;

    if( app.GetTutorialMode() )
    {
        isClientSide = false;
        maxPlayers = 4;
    }

	app.SetGameHostOption(eGameHostOption_All, app.getRemoteStorage()->getSaveHostOptions() );

    g_NetworkManager.HostGame(0,isClientSide,isPrivate,maxPlayers,0);

    LoadSaveDataThreadParam *saveData = new LoadSaveDataThreadParam(ba.data, ba.length, cloudFile.getName());

    NetworkGameInitData *param = new NetworkGameInitData();
    param->seed = app.getRemoteStorage()->getSaveSeed();
    param->saveData = saveData;
    param->settings = app.GetGameHostOption( eGameHostOption_All );
    param->savePlatform = app.getRemoteStorage()->getSavePlatform();
	param->texturePackId = app.getRemoteStorage()->getSaveTexturePack();

#ifndef _XBOX
    g_NetworkManager.FakeLocalPlayerJoined();
#endif

    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &CGameNetworkManager::RunNetworkGameThreadProc;
    loadingParams->lpParam = (LPVOID)param;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_CloseAllPlayersUIScenes;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

    ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_FullscreenProgress, loadingParams);
}

#endif //SONY_REMOTE_STORAGE_DOWNLOAD

#ifdef _WINDOWS64
static bool Win64_DeleteSaveDirectory(const wchar_t* wPath)
{
    wchar_t wSearch[MAX_PATH];
    swprintf_s(wSearch, MAX_PATH, L"%s\\*", wPath);
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(wSearch, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (wcscmp(fd.cFileName, L".") == 0 || wcscmp(fd.cFileName, L"..") == 0) continue;
            wchar_t wChild[MAX_PATH];
            swprintf_s(wChild, MAX_PATH, L"%s\\%s", wPath, fd.cFileName);
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                Win64_DeleteSaveDirectory(wChild);
            else
                DeleteFileW(wChild);
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
    return RemoveDirectoryW(wPath) != 0;
}
#endif // _WINDOWS64

int UIScene_LoadOrJoinMenu::DeleteSaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;
    // results switched for this dialog

	// Check that we have a valid save selected (can get a bad index if the save list has been refreshed)
	bool validSelection= pClass->m_iDefaultButtonsC != 0 && pClass->m_iSaveListIndex >= pClass->m_iDefaultButtonsC;

    if(result==C4JStorage::EMessage_ResultDecline && validSelection)
    {
        if(app.DebugSettingsOn() && app.GetLoadSavesFromFolderEnabled())
        {
            pClass->m_bIgnoreInput=false;
        }
        else
        {
#ifdef _WINDOWS64
            {
                // Use m_saveDetails (sorted display order) so the correct folder is targeted
                int displayIdx = pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC;
                bool bSuccess = false;
                if (pClass->m_saveDetails && displayIdx >= 0 && pClass->m_saveDetails[displayIdx].UTF8SaveFilename[0])
                {
                    wchar_t wFilename[MAX_SAVEFILENAME_LENGTH] = {};
                    mbstowcs_s(NULL, wFilename, MAX_SAVEFILENAME_LENGTH, pClass->m_saveDetails[displayIdx].UTF8SaveFilename, MAX_SAVEFILENAME_LENGTH - 1);
                    wchar_t wFolderPath[MAX_PATH] = {};
                    swprintf_s(wFolderPath, MAX_PATH, L"Windows64\\GameHDD\\%s", wFilename);
                    bSuccess = Win64_DeleteSaveDirectory(wFolderPath);
                }
                UIScene_LoadOrJoinMenu::DeleteSaveDataReturned((LPVOID)pClass->GetCallbackUniqueId(), bSuccess);
            }
#else
			StorageManager.DeleteSaveData(&pClass->m_pSaveDetails->SaveInfoA[pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC], UIScene_LoadOrJoinMenu::DeleteSaveDataReturned, (LPVOID)pClass->GetCallbackUniqueId());
#endif
            pClass->m_controlSavesTimer.setVisible( true );
        }
    }
    else
    {
        pClass->m_bIgnoreInput=false;
    }

    return 0;
}

int UIScene_LoadOrJoinMenu::DeleteSaveDataReturned(LPVOID lpParam,bool bRes)
{
	ui.EnterCallbackIdCriticalSection();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParam);

	if(pClass)
	{
		if(bRes)
		{
			// wipe the list and repopulate it
			pClass->m_iState=e_SavesRepopulateAfterDelete;
		}
		else pClass->m_bIgnoreInput=false;

		pClass->updateTooltips();
	}
	ui.LeaveCallbackIdCriticalSection();
    return 0;
}


int UIScene_LoadOrJoinMenu::RenameSaveDataReturned(LPVOID lpParam,bool bRes)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)lpParam;

    if(bRes)
    {
        pClass->m_iState=e_SavesRepopulate;
    }
    else pClass->m_bIgnoreInput=false;

    pClass->updateTooltips();

    return 0;
}

#ifdef __ORBIS__


void UIScene_LoadOrJoinMenu::LoadRemoteFileFromDisk(char* remoteFilename)
{
    wchar_t wSaveName[128];
    mbstowcs(wSaveName, remoteFilename, strlen(remoteFilename)+1); // plus null

    // 	processConsoleSave(wSaveName, L"ProcessedSave.bin");

    // 	File remoteFile(L"ProcessedSave.bin");
    File remoteFile(wSaveName);
    LoadSaveFromDisk(&remoteFile, SAVE_FILE_PLATFORM_PS3);
}
#endif


int UIScene_LoadOrJoinMenu::SaveOptionsDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;

    // results switched for this dialog
    // EMessage_ResultAccept means cancel
    switch(result)
    {
    case C4JStorage::EMessage_ResultDecline:  // rename
        {
			pClass->m_bIgnoreInput=true;
#ifdef _WINDOWS64
            {
                wchar_t wSaveName[128];
                ZeroMemory(wSaveName, 128 * sizeof(wchar_t));
                mbstowcs_s(NULL, wSaveName, 128, pClass->m_saveDetails[pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC].UTF8SaveName, _TRUNCATE);
                UIKeyboardInitData kbData;
                kbData.title       = app.GetString(IDS_RENAME_WORLD_TITLE);
                kbData.defaultText = wSaveName;
                kbData.maxChars    = 25;
                kbData.callback    = &UIScene_LoadOrJoinMenu::KeyboardCompleteWorldNameCallback;
                kbData.lpParam     = pClass;
                kbData.pcMode      = !Win64_IsControllerConnected();
                ui.NavigateToScene(pClass->m_iPad, eUIScene_Keyboard, &kbData);
            }
#elif defined _DURANGO
            // bring up a keyboard
            InputManager.RequestKeyboard(app.GetString(IDS_RENAME_WORLD_TITLE), (pClass->m_saveDetails[pClass->m_iSaveListIndex-pClass->m_iDefaultButtonsC]).UTF16SaveName,(DWORD)0,25,&UIScene_LoadOrJoinMenu::KeyboardCompleteWorldNameCallback,pClass,C_4JInput::EKeyboardMode_Default);
#else
            // bring up a keyboard
            wchar_t wSaveName[128];
            //CD - Fix - We must memset the SaveName
            ZeroMemory(wSaveName, 128 * sizeof(wchar_t) );
            mbstowcs(wSaveName, pClass->m_saveDetails[pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC].UTF8SaveName, strlen(pClass->m_saveDetails->UTF8SaveName)+1); // plus null
            LPWSTR ptr = wSaveName;
            InputManager.RequestKeyboard(app.GetString(IDS_RENAME_WORLD_TITLE),wSaveName,(DWORD)0,25,&UIScene_LoadOrJoinMenu::KeyboardCompleteWorldNameCallback,pClass,C_4JInput::EKeyboardMode_Default);
#endif
        }
        break;

    case C4JStorage::EMessage_ResultThirdOption:  // delete -
        {
            // delete the save game
            // Have to ask the player if they are sure they want to delete this game
            UINT uiIDA[2];
            uiIDA[0]=IDS_CONFIRM_CANCEL;
            uiIDA[1]=IDS_CONFIRM_OK;
            ui.RequestAlertMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, uiIDA, 2, iPad,&UIScene_LoadOrJoinMenu::DeleteSaveDialogReturned,pClass);
        }
        break;

#ifdef SONY_REMOTE_STORAGE_UPLOAD
    case C4JStorage::EMessage_ResultFourthOption: // upload to cloud
        {
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			ui.RequestAlertMessage(IDS_TOOLTIPS_SAVETRANSFER_UPLOAD, IDS_SAVE_TRANSFER_TEXT, uiIDA, 2, iPad,&UIScene_LoadOrJoinMenu::SaveTransferDialogReturned,pClass);
        }
        break;
#endif // SONY_REMOTE_STORAGE_UPLOAD
#if defined _XBOX_ONE  || defined __ORBIS__
    case C4JStorage::EMessage_ResultFourthOption: // copy save
        {
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			ui.RequestAlertMessage(IDS_COPYSAVE, IDS_TEXT_COPY_SAVE, uiIDA, 2, iPad,&UIScene_LoadOrJoinMenu::CopySaveDialogReturned,pClass);
        }
        break;
#endif

    case C4JStorage::EMessage_Cancelled:
    default:
        {
            // reset the tooltips
            pClass->updateTooltips();
            pClass->m_bIgnoreInput=false;
        }
        break;
    }
    return 0;
}


#if defined (__PSVITA__)

int UIScene_LoadOrJoinMenu::SignInAdhocReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;
	pClass->m_bIgnoreInput = false;
	return 0;

}



int UIScene_LoadOrJoinMenu::MustSignInTexturePack(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{
		SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_LoadOrJoinMenu::MustSignInReturnedTexturePack, pClass);
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}

	return 0;
}


int UIScene_LoadOrJoinMenu::MustSignInReturnedTexturePack(void *pParam,bool bContinue, int iPad)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;

	int commerceState = app.GetCommerceState();
	while(	commerceState != CConsoleMinecraftApp::eCommerce_State_Offline &&
			commerceState != CConsoleMinecraftApp::eCommerce_State_Online &&
			commerceState != CConsoleMinecraftApp::eCommerce_State_Error)
	{
		Sleep(10);
		commerceState = app.GetCommerceState();
	}

	if(bContinue==true)
	{
		SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo(pClass->m_initData->selectedSession->data.texturePackParentId);
		if(pSONYDLCInfo!=NULL)
		{
			char chName[42];
			char chKeyName[20];
			char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

			memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
			// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
			// So we assume the first sku for the product is the one we want
			// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a NULL terminating char
			memset(chKeyName, 0, sizeof(chKeyName));
			strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef __ORBIS__
			strcpy(chName, chKeyName);
#else
			sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
			app.GetDLCSkuIDFromProductList(chName,chSkuID);
			// 4J-PB - need to check for an empty store
			if(app.CheckForEmptyStore(iPad)==false)
			{
				if(app.DLCAlreadyPurchased(chSkuID))
				{
					app.DownloadAlreadyPurchased(chSkuID);
				}
				else
				{
					app.Checkout(chSkuID);
				}
			}
		}
	}
	pClass->m_bIgnoreInput = false;
    return 0;
}

#endif

int UIScene_LoadOrJoinMenu::TexturePackDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu *pClass = (UIScene_LoadOrJoinMenu *)pParam;

    // Exit with or without saving
    if(result==C4JStorage::EMessage_ResultAccept)
    {
        // we need to enable background downloading for the DLC
        XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
#if defined __PSVITA__ || defined __PS3__ || defined __ORBIS__

#ifdef __PSVITA__
		if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && CGameNetworkManager::usingAdhocMode())
		{
			// get them to sign in to online
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_LoadOrJoinMenu::MustSignInTexturePack,pClass);
			return;
		}
#endif

		SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo(pClass->m_initData->selectedSession->data.texturePackParentId);
		if(pSONYDLCInfo!=NULL)
		{
			char chName[42];
			char chKeyName[20];
			char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

			memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
			// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
			// So we assume the first sku for the product is the one we want
			// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a NULL terminating char
			memset(chKeyName, 0, sizeof(chKeyName));
			strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef __ORBIS__
			strcpy(chName, chKeyName);
#else
			sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
			app.GetDLCSkuIDFromProductList(chName,chSkuID);
			// 4J-PB - need to check for an empty store
			if(app.CheckForEmptyStore(iPad)==false)
			{
				if(app.DLCAlreadyPurchased(chSkuID))
				{
					app.DownloadAlreadyPurchased(chSkuID);
				}
				else
				{
					app.Checkout(chSkuID);
				}
			}
		}
#endif


#if defined _XBOX_ONE
		if(ProfileManager.IsSignedIn(iPad))
		{
			if (ProfileManager.IsSignedInLive(iPad))
			{
				wstring ProductId;
				app.GetDLCFullOfferIDForPackID(pClass->m_initData->selectedSession->data.texturePackParentId,ProductId);

				StorageManager.InstallOffer(1,(WCHAR *)ProductId.c_str(),NULL,NULL);
			}
			else
			{
				// 4J-JEV: Fix for XB1: #165863 - XR-074: Compliance: With no active network connection user is unable to convert from Trial to Full texture pack and is not messaged why.
				UINT uiIDA[1] = { IDS_CONFIRM_OK };
				ui.RequestErrorMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1, iPad);
			}
		}
#endif

    }
    pClass->m_bIgnoreInput=false;
    return 0;
}

#if defined __PS3__ || defined __PSVITA__ || defined __ORBIS__
int UIScene_LoadOrJoinMenu::MustSignInReturnedPSN(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{
#if defined(__PS3__)
		SQRNetworkManager_PS3::AttemptPSNSignIn(&UIScene_LoadOrJoinMenu::PSN_SignInReturned, pClass);
#elif defined __PSVITA__
		SQRNetworkManager_Vita::AttemptPSNSignIn(&UIScene_LoadOrJoinMenu::PSN_SignInReturned, pClass);
#else
		SQRNetworkManager_Orbis::AttemptPSNSignIn(&UIScene_LoadOrJoinMenu::PSN_SignInReturned, pClass, false, iPad);
#endif
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}

    return 0;
}

int UIScene_LoadOrJoinMenu::PSN_SignInReturned(void *pParam,bool bContinue, int iPad)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;
	if(bContinue==true)
	{
		switch(pClass->m_eAction)
		{
		case eAction_ViewInvites:
			// Check if we're signed in to LIVE
			if(ProfileManager.IsSignedInLive(iPad))
			{
#if defined(__PS3__)
				int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
				app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )\n", ret, ret);
#elif defined __PSVITA__
				SQRNetworkManager_Vita::RecvInviteGUI();
#else
				SQRNetworkManager_Orbis::RecvInviteGUI();
#endif
			}
			break;
		case eAction_JoinGame:
			pClass->CheckAndJoinGame(pClass->m_iGameListIndex);
			break;
		}
	}
	else
	{
		pClass->m_bIgnoreInput = false;
	}
    return 0;
}
#endif

#ifdef SONY_REMOTE_STORAGE_DOWNLOAD

void UIScene_LoadOrJoinMenu::LaunchSaveTransfer()
{
    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &UIScene_LoadOrJoinMenu::DownloadSonyCrossSaveThreadProc;
    loadingParams->lpParam = (LPVOID)this;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_NavigateBackToScene;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

    loadingParams->cancelFunc=&UIScene_LoadOrJoinMenu::CancelSaveTransferCallback;
	loadingParams->m_cancelFuncParam=this;
    loadingParams->cancelText=IDS_TOOLTIPS_CANCEL;

    ui.NavigateToScene(m_iPad,eUIScene_FullscreenProgress, loadingParams);
}




int UIScene_LoadOrJoinMenu::CreateDummySaveDataCallback(LPVOID lpParam,bool bRes)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;
	if(bRes)
	{
		pClass->m_eSaveTransferState = eSaveTransfer_GetSavesInfo;
	}
	else
	{
		pClass->m_eSaveTransferState = eSaveTransfer_Error;
		app.DebugPrintf("CreateDummySaveDataCallback failed\n");

	}
	return 0;
}

int UIScene_LoadOrJoinMenu::CrossSaveGetSavesInfoCallback(LPVOID lpParam, SAVE_DETAILS *pSaveDetails, bool bRes)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;
	if(bRes)
	{
		pClass->m_eSaveTransferState = eSaveTransfer_GetFileData;
	}
	else
	{
		pClass->m_eSaveTransferState = eSaveTransfer_Error;
		app.DebugPrintf("CrossSaveGetSavesInfoCallback failed\n");
	}
	return 0;
}

int UIScene_LoadOrJoinMenu::LoadCrossSaveDataCallback( void *pParam,bool bIsCorrupt, bool bIsOwner )
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) pParam;
	if(bIsCorrupt == false && bIsOwner)
	{
		pClass->m_eSaveTransferState = eSaveTransfer_CreatingNewSave;
	}
	else
	{
		pClass->m_eSaveTransferState = eSaveTransfer_Error;
		app.DebugPrintf("LoadCrossSaveDataCallback failed \n");

	}
	return 0;
}

int UIScene_LoadOrJoinMenu::CrossSaveFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) pParam;
	pClass->m_eSaveTransferState = eSaveTransfer_Idle;
	return 0;
}


int UIScene_LoadOrJoinMenu::CrossSaveDeleteOnErrorReturned(LPVOID lpParam,bool bRes)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;
	pClass->m_eSaveTransferState = eSaveTransfer_ErrorMesssage;
	return 0;
}

int UIScene_LoadOrJoinMenu::RemoteSaveNotFoundCallback(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) pParam;
	pClass->m_eSaveTransferState = eSaveTransfer_Idle;
	return 0;
}

// MGH -  added this global to force the delete of the previous data, for the remote storage saves
//	need to speak to Chris why this is necessary
bool g_bForceVitaSaveWipe = false;


int UIScene_LoadOrJoinMenu::DownloadSonyCrossSaveThreadProc( LPVOID lpParameter )
{
	m_bSaveTransferRunning = true;
#ifdef __PS3__
	StorageManager.SetSaveTransferInProgress(true);
#endif
    Compression::UseDefaultThreadStorage();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParameter;
 	pClass->m_saveTransferDownloadCancelled = false;
	m_bSaveTransferRunning = true;
	bool bAbortCalled = false;
	Minecraft *pMinecraft=Minecraft::GetInstance();
	bool bSaveFileCreated = false;
	wchar_t wSaveName[128];

    // get the save file size
	pMinecraft->progressRenderer->progressStagePercentage(0);
    pMinecraft->progressRenderer->progressStart(IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD);
    pMinecraft->progressRenderer->progressStage( IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD );

	ConsoleSaveFile* pSave = NULL;

	pClass->m_eSaveTransferState = eSaveTransfer_GetRemoteSaveInfo;


    while(pClass->m_eSaveTransferState!=eSaveTransfer_Idle)
    {
        switch(pClass->m_eSaveTransferState)
        {
        case eSaveTransfer_Idle:
            break;
		case eSaveTransfer_GetRemoteSaveInfo:
			app.DebugPrintf("UIScene_LoadOrJoinMenu getSaveInfo\n");
			app.getRemoteStorage()->getSaveInfo();
			pClass->m_eSaveTransferState = eSaveTransfer_GettingRemoteSaveInfo;
		break;
		case eSaveTransfer_GettingRemoteSaveInfo:
			if(pClass->m_saveTransferDownloadCancelled)
			{
				pClass->m_eSaveTransferState = eSaveTransfer_Error;
				break;
			}
			if(app.getRemoteStorage()->waitingForSaveInfo() == false)
			{
				if(app.getRemoteStorage()->saveIsAvailable())
				{
					if(app.getRemoteStorage()->saveVersionSupported())
					{
						pClass->m_eSaveTransferState = eSaveTransfer_CreateDummyFile;
					}
					else
					{
						// must be a newer version of the save in the cloud that we don't support yet
						UINT uiIDA[1];
						uiIDA[0]=IDS_CONFIRM_OK;
						ui.RequestAlertMessage(IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD, IDS_SAVE_TRANSFER_WRONG_VERSION, uiIDA, 1, ProfileManager.GetPrimaryPad(),RemoteSaveNotFoundCallback,pClass);
					}
				}
				else
				{
					// no save available, inform the user about the functionality
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestAlertMessage(IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD, IDS_SAVE_TRANSFER_NOT_AVAILABLE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(),RemoteSaveNotFoundCallback,pClass);
				}
			}
			break;
		case eSaveTransfer_CreateDummyFile:
			{
				StorageManager.ResetSaveData();
				byte *compData = (byte *)StorageManager.AllocateSaveData( app.getRemoteStorage()->getSaveFilesize() );
				// Make our next save default to the name of the level
				const char* pNameUTF8 = app.getRemoteStorage()->getSaveNameUTF8();
				mbstowcs(wSaveName, pNameUTF8, strlen(pNameUTF8)+1); // plus null
				StorageManager.SetSaveTitle(wSaveName);
				PBYTE pbThumbnailData=NULL;
				DWORD dwThumbnailDataSize=0;

				PBYTE pbDataSaveImage=NULL;
				DWORD dwDataSizeSaveImage=0;

				StorageManager.GetDefaultSaveImage(&pbDataSaveImage, &dwDataSizeSaveImage);			// Get the default save thumbnail (as set by SetDefaultImages) for use on saving games t
				StorageManager.GetDefaultSaveThumbnail(&pbThumbnailData,&dwThumbnailDataSize);		// Get the default save image (as set by SetDefaultImages) for use on saving games that

				BYTE bTextMetadata[88];
				ZeroMemory(bTextMetadata,88);
				unsigned int hostOptions = app.getRemoteStorage()->getSaveHostOptions();
#ifdef __ORBIS__
				app.SetGameHostOption(hostOptions, eGameHostOption_WorldSize, e_worldSize_Classic);		// force the classic world size on, otherwise it's unknown and we can't expand
#endif
				int iTextMetadataBytes = app.CreateImageTextData(bTextMetadata, app.getRemoteStorage()->getSaveSeed(), true, hostOptions, app.getRemoteStorage()->getSaveTexturePack() );

				// set the icon and save image
				StorageManager.SetSaveImages(pbThumbnailData,dwThumbnailDataSize,pbDataSaveImage,dwDataSizeSaveImage,bTextMetadata,iTextMetadataBytes);

				app.getRemoteStorage()->waitForStorageManagerIdle();
				C4JStorage::ESaveGameState saveState = StorageManager.SaveSaveData( &UIScene_LoadOrJoinMenu::CreateDummySaveDataCallback, lpParameter );
				if(saveState == C4JStorage::ESaveGame_Save)
				{
					pClass->m_eSaveTransferState = eSaveTransfer_CreatingDummyFile;
				}
				else
				{
					app.DebugPrintf("Failed to create dummy save file\n");
					pClass->m_eSaveTransferState = eSaveTransfer_Error;
				}
			}
			break;
		case eSaveTransfer_CreatingDummyFile:
			break;
		case eSaveTransfer_GetSavesInfo:
			{
				// we can't cancel here, we need the saves info so we can delete the file
				if(pClass->m_saveTransferDownloadCancelled)
				{
					WCHAR wcTemp[256];
					swprintf(wcTemp,256, app.GetString(IDS_CANCEL));		// MGH - should change this string to "cancelling download"
					m_wstrStageText=wcTemp;
					pMinecraft->progressRenderer->progressStage( m_wstrStageText );
				}

				app.getRemoteStorage()->waitForStorageManagerIdle();
				app.DebugPrintf("CALL GetSavesInfo B\n");
				C4JStorage::ESaveGameState eSGIStatus= StorageManager.GetSavesInfo(pClass->m_iPad,&UIScene_LoadOrJoinMenu::CrossSaveGetSavesInfoCallback,pClass,"save");
				pClass->m_eSaveTransferState = eSaveTransfer_GettingSavesInfo;
			}
			break;
		case eSaveTransfer_GettingSavesInfo:
			if(pClass->m_saveTransferDownloadCancelled)
			{
				WCHAR wcTemp[256];
				swprintf(wcTemp,256, app.GetString(IDS_CANCEL));		// MGH - should change this string to "cancelling download"
				m_wstrStageText=wcTemp;
				pMinecraft->progressRenderer->progressStage( m_wstrStageText );
			}
			break;

		case eSaveTransfer_GetFileData:
		{
			bSaveFileCreated = true;
			StorageManager.GetSaveUniqueFileDir(pClass->m_downloadedUniqueFilename);

			if(pClass->m_saveTransferDownloadCancelled)
			{
				pClass->m_eSaveTransferState = eSaveTransfer_Error;
				break;
			}
			PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
			int idx = pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC;
			app.getRemoteStorage()->waitForStorageManagerIdle();
			bool bGettingOK = app.getRemoteStorage()->getSaveData(pClass->m_downloadedUniqueFilename, SaveTransferReturned, pClass);
			if(bGettingOK)
			{
				pClass->m_eSaveTransferState = eSaveTransfer_GettingFileData;
			}
			else
			{
				pClass->m_eSaveTransferState = eSaveTransfer_Error;
				app.DebugPrintf("app.getRemoteStorage()->getSaveData failed\n");

			}
		}

        case eSaveTransfer_GettingFileData:
            {
                WCHAR wcTemp[256];

                int dataProgress = app.getRemoteStorage()->getDataProgress();
                pMinecraft->progressRenderer->progressStagePercentage(dataProgress);

                //swprintf(wcTemp, 256, L"Downloading data : %d", dataProgress);//app.GetString(IDS_SAVETRANSFER_STAGE_GET_DATA),0,pClass->m_ulFileSize);
                swprintf(wcTemp,256, app.GetString(IDS_SAVETRANSFER_STAGE_GET_DATA),dataProgress);
                m_wstrStageText=wcTemp;
                pMinecraft->progressRenderer->progressStage( m_wstrStageText );
				if(pClass->m_saveTransferDownloadCancelled && bAbortCalled == false)
				{
					app.getRemoteStorage()->abort();
					bAbortCalled = true;
				}
            }
            break;
        case eSaveTransfer_FileDataRetrieved:
			pClass->m_eSaveTransferState = eSaveTransfer_LoadSaveFromDisc;
		break;
		case eSaveTransfer_LoadSaveFromDisc:
		{
			if(pClass->m_saveTransferDownloadCancelled)
			{
				pClass->m_eSaveTransferState = eSaveTransfer_Error;
				break;
			}

			PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
			int saveInfoIndex = -1;
			for(int i=0;i<pSaveDetails->iSaveC;i++)
			{
				if(strcmp(pSaveDetails->SaveInfoA[i].UTF8SaveFilename, pClass->m_downloadedUniqueFilename) == 0)
				{
					//found it
					saveInfoIndex = i;
				}
			}
			if(saveInfoIndex == -1)
			{
				pClass->m_eSaveTransferState = eSaveTransfer_Error;
				app.DebugPrintf("CrossSaveGetSavesInfoCallback failed - couldn't find save\n");
			}
			else
			{
#ifdef __PS3__
				// ignore the CRC on PS3
				C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveData(&pSaveDetails->SaveInfoA[saveInfoIndex],&LoadCrossSaveDataCallback,pClass, true);
#else
				C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveData(&pSaveDetails->SaveInfoA[saveInfoIndex],&LoadCrossSaveDataCallback,pClass);
#endif
				if(eLoadStatus == C4JStorage::ESaveGame_Load)
				{
					pClass->m_eSaveTransferState = eSaveTransfer_LoadingSaveFromDisc;
				}
				else
				{
					pClass->m_eSaveTransferState = eSaveTransfer_Error;
				}
			}
		}
		break;
		case eSaveTransfer_LoadingSaveFromDisc:

			break;
        case eSaveTransfer_CreatingNewSave:
			{
				unsigned int fileSize = StorageManager.GetSaveSize();
				byteArray ba(fileSize);
				StorageManager.GetSaveData(ba.data, &fileSize);
				assert(ba.length == fileSize);


                StorageManager.ResetSaveData();
				{
					PBYTE pbThumbnailData=NULL;
					DWORD dwThumbnailDataSize=0;

					PBYTE pbDataSaveImage=NULL;
					DWORD dwDataSizeSaveImage=0;

					StorageManager.GetDefaultSaveImage(&pbDataSaveImage, &dwDataSizeSaveImage);			// Get the default save thumbnail (as set by SetDefaultImages) for use on saving games t
					StorageManager.GetDefaultSaveThumbnail(&pbThumbnailData,&dwThumbnailDataSize);		// Get the default save image (as set by SetDefaultImages) for use on saving games that

					BYTE bTextMetadata[88];
					ZeroMemory(bTextMetadata,88);
					unsigned int remoteHostOptions = app.getRemoteStorage()->getSaveHostOptions();
					app.SetGameHostOption(eGameHostOption_All, remoteHostOptions );
					int iTextMetadataBytes = app.CreateImageTextData(bTextMetadata, app.getRemoteStorage()->getSaveSeed(), true, remoteHostOptions, app.getRemoteStorage()->getSaveTexturePack() );

					// set the icon and save image
					StorageManager.SetSaveImages(pbThumbnailData,dwThumbnailDataSize,pbDataSaveImage,dwDataSizeSaveImage,bTextMetadata,iTextMetadataBytes);
				}


#ifdef SPLIT_SAVES
                ConsoleSaveFileOriginal oldFormatSave( wSaveName, ba.data, ba.length, false, app.getRemoteStorage()->getSavePlatform() );
                pSave = new ConsoleSaveFileSplit( &oldFormatSave, false, pMinecraft->progressRenderer );

                pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_STAGE_SAVING);
                pSave->Flush(false,false);
                pClass->m_eSaveTransferState = eSaveTransfer_Saving;
#else
                pSave = new ConsoleSaveFileOriginal( wSaveName, ba.data, ba.length, false, app.getRemoteStorage()->getSavePlatform() );
                pClass->m_eSaveTransferState = eSaveTransfer_Converting;
				pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_STAGE_CONVERTING);
#endif
                delete ba.data;
            }
			break;
        case eSaveTransfer_Converting:
			{
            pSave->ConvertToLocalPlatform(); // check if we need to convert this file from PS3->PS4
            pClass->m_eSaveTransferState = eSaveTransfer_Saving;
            pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_STAGE_SAVING);
			StorageManager.SetSaveTitle(wSaveName);
			StorageManager.SetSaveUniqueFilename(pClass->m_downloadedUniqueFilename);

			app.getRemoteStorage()->waitForStorageManagerIdle();	// we need to wait for the save system to be idle here, as Flush doesn't check for it.
            pSave->Flush(false, false);
			}
            break;
        case eSaveTransfer_Saving:
			{
				// On Durango/Orbis, we need to wait for all the asynchronous saving processes to complete before destroying the levels, as that will ultimately delete
				// the directory level storage & therefore the ConsoleSaveSplit instance, which needs to be around until all the sub files have completed saving.
#if defined(_DURANGO) || defined(__ORBIS__)
				while(StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
				{
					Sleep(10);
					StorageManager.Tick();
				}
#endif

				delete pSave;


				pMinecraft->progressRenderer->progressStage(IDS_PROGRESS_SAVING_TO_DISC);
				pClass->m_eSaveTransferState = eSaveTransfer_Succeeded;
			}
            break;

		case eSaveTransfer_Succeeded:
			{
				// if we've arrived here, the save has been created successfully
				pClass->m_iState=e_SavesRepopulate;
				pClass->updateTooltips();
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				app.getRemoteStorage()->waitForStorageManagerIdle();	// wait for everything to complete before we hand control back to the player
				ui.RequestErrorMessage( IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD, IDS_SAVE_TRANSFER_DOWNLOADCOMPLETE, uiIDA,1,ProfileManager.GetPrimaryPad(),CrossSaveFinishedCallback,pClass);
				pClass->m_eSaveTransferState = eSaveTransfer_Finished;
			}
			break;

		case eSaveTransfer_Cancelled: // this is no longer used
			{
				assert(0); //pClass->m_eSaveTransferState = eSaveTransfer_Idle;
			}
			break;
        case eSaveTransfer_Error:
			{
				if(bSaveFileCreated)
				{
					if(pClass->m_saveTransferDownloadCancelled)
					{
						WCHAR wcTemp[256];
					swprintf(wcTemp,256, app.GetString(IDS_CANCEL));		// MGH - should change this string to "cancelling download"
						m_wstrStageText=wcTemp;
						pMinecraft->progressRenderer->progressStage( m_wstrStageText );
						pMinecraft->progressRenderer->progressStage( m_wstrStageText );
					}
					// if the save file has already been created we have to delete it again if there's been an error
					PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
					int saveInfoIndex = -1;
					for(int i=0;i<pSaveDetails->iSaveC;i++)
					{
						if(strcmp(pSaveDetails->SaveInfoA[i].UTF8SaveFilename, pClass->m_downloadedUniqueFilename) == 0)
						{
							//found it
							saveInfoIndex = i;
						}
					}
					if(saveInfoIndex == -1)
					{
						app.DebugPrintf("eSaveTransfer_Error failed - couldn't find save\n");
						assert(0);
						pClass->m_eSaveTransferState = eSaveTransfer_ErrorMesssage;
					}
					else
					{
						// delete the save file
						app.getRemoteStorage()->waitForStorageManagerIdle();
							C4JStorage::ESaveGameState eDeleteStatus = StorageManager.DeleteSaveData(&pSaveDetails->SaveInfoA[saveInfoIndex],UIScene_LoadOrJoinMenu::CrossSaveDeleteOnErrorReturned,pClass);
						if(eDeleteStatus == C4JStorage::ESaveGame_Delete)
						{
							pClass->m_eSaveTransferState = eSaveTransfer_ErrorDeletingSave;
						}
						else
						{
							app.DebugPrintf("StorageManager.DeleteSaveData failed!!\n");
							pClass->m_eSaveTransferState = eSaveTransfer_ErrorMesssage;
						}
					}
				}
				else
				{
					pClass->m_eSaveTransferState = eSaveTransfer_ErrorMesssage;
				}
			}
            break;

		case eSaveTransfer_ErrorDeletingSave:
			break;
		case eSaveTransfer_ErrorMesssage:
			{
				app.getRemoteStorage()->waitForStorageManagerIdle();	// wait for everything to complete before we hand control back to the player
				if(pClass->m_saveTransferDownloadCancelled)
				{
					pClass->m_eSaveTransferState = eSaveTransfer_Idle;
				}
				else
				{
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					UINT errorMessage = IDS_SAVE_TRANSFER_DOWNLOADFAILED;
					if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
					{
						errorMessage = IDS_ERROR_NETWORK;			// show "A network error has occurred."
#ifdef __ORBIS__
						if(!ProfileManager.isSignedInPSN(ProfileManager.GetPrimaryPad()))
						{
							errorMessage = IDS_PRO_NOTONLINE_TEXT;		// show "not signed into PSN"
						}
#endif
#ifdef __VITA__
						if(!ProfileManager.IsSignedInPSN(ProfileManager.GetPrimaryPad()))
						{
							errorMessage = IDS_PRO_NOTONLINE_TEXT;		// show "not signed into PSN"
						}
#endif

					}
					ui.RequestErrorMessage( IDS_TOOLTIPS_SAVETRANSFER_DOWNLOAD, errorMessage, uiIDA,1,ProfileManager.GetPrimaryPad(),CrossSaveFinishedCallback,pClass);
					pClass->m_eSaveTransferState = eSaveTransfer_Finished;
				}
				if(bSaveFileCreated)		// save file has been created, then deleted.
					pClass->m_iState=e_SavesRepopulateAfterDelete;
				else
					pClass->m_iState=e_SavesRepopulate;
				pClass->updateTooltips();
			}
			break;
		case eSaveTransfer_Finished:
			{
			}
			// waiting to dismiss the dialog
			break;
        }
        Sleep(50);
    }
	m_bSaveTransferRunning = false;
#ifdef __PS3__
	StorageManager.SetSaveTransferInProgress(false);
#endif
    return 0;

}

void UIScene_LoadOrJoinMenu::SaveTransferReturned(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;

    if(s == SonyRemoteStorage::e_getDataSucceeded)
	{
        pClass->m_eSaveTransferState = eSaveTransfer_FileDataRetrieved;
	}
    else
	{
        pClass->m_eSaveTransferState = eSaveTransfer_Error;
		app.DebugPrintf("SaveTransferReturned failed with error code : 0x%08x\n", error_code);
	}

}
ConsoleSaveFile* UIScene_LoadOrJoinMenu::SonyCrossSaveConvert()
{
    return NULL;
}

void UIScene_LoadOrJoinMenu::CancelSaveTransferCallback(LPVOID lpParam)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;
	pClass->m_saveTransferDownloadCancelled = true;
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1, -1, -1, -1,-1,-1,-1,-1);		// MGH -  added - remove the "cancel" tooltip, so the player knows it's underway (really needs a "cancelling" message)
}

#endif



#ifdef SONY_REMOTE_STORAGE_UPLOAD

void UIScene_LoadOrJoinMenu::LaunchSaveUpload()
{
    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &UIScene_LoadOrJoinMenu::UploadSonyCrossSaveThreadProc;
    loadingParams->lpParam = (LPVOID)this;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_NavigateBackToScene;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    loadingParams->completionData = completionData;

// 4J-PB - Waiting for Sony to fix canceling a save upload
	loadingParams->cancelFunc=&UIScene_LoadOrJoinMenu::CancelSaveUploadCallback;
	loadingParams->m_cancelFuncParam = this;
 loadingParams->cancelText=IDS_TOOLTIPS_CANCEL;

    ui.NavigateToScene(m_iPad,eUIScene_FullscreenProgress, loadingParams);

}

int UIScene_LoadOrJoinMenu::CrossSaveUploadFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) pParam;
	pClass->m_eSaveUploadState = eSaveUpload_Idle;

	return 0;
}


int UIScene_LoadOrJoinMenu::UploadSonyCrossSaveThreadProc( LPVOID lpParameter )
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParameter;
	pClass->m_saveTransferUploadCancelled = false;
	bool bAbortCalled = false;
    Minecraft *pMinecraft=Minecraft::GetInstance();

    // get the save file size
	pMinecraft->progressRenderer->progressStagePercentage(0);
    pMinecraft->progressRenderer->progressStart(IDS_TOOLTIPS_SAVETRANSFER_UPLOAD);
    pMinecraft->progressRenderer->progressStage( IDS_TOOLTIPS_SAVETRANSFER_UPLOAD );

    PSAVE_DETAILS pSaveDetails=StorageManager.ReturnSavesInfo();
    int idx = pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC;
    bool bSettingOK = app.getRemoteStorage()->setSaveData(&pSaveDetails->SaveInfoA[idx], SaveUploadReturned, pClass);

	if(bSettingOK)
    {
        pClass->m_eSaveUploadState = eSaveUpload_UploadingFileData;
        pMinecraft->progressRenderer->progressStagePercentage(0);
    }
    else
	{
        pClass->m_eSaveUploadState = eSaveUpload_Error;
	}

    while(pClass->m_eSaveUploadState!=eSaveUpload_Idle)
    {
        switch(pClass->m_eSaveUploadState)
        {
        case eSaveUpload_Idle:
            break;
        case eSaveUpload_UploadingFileData:
            {
                WCHAR wcTemp[256];
                int dataProgress = app.getRemoteStorage()->getDataProgress();
                pMinecraft->progressRenderer->progressStagePercentage(dataProgress);

                //swprintf(wcTemp, 256, L"Uploading data : %d", dataProgress);//app.GetString(IDS_SAVETRANSFER_STAGE_GET_DATA),0,pClass->m_ulFileSize);
                swprintf(wcTemp,256, app.GetString(IDS_SAVETRANSFER_STAGE_PUT_DATA),dataProgress);

                m_wstrStageText=wcTemp;
                pMinecraft->progressRenderer->progressStage( m_wstrStageText );
// 4J-PB - Waiting for Sony to fix canceling a save upload
				if(pClass->m_saveTransferUploadCancelled && bAbortCalled == false)
				{
					// we only really want to be able to cancel during the download of data, if it's taking a long time
					app.getRemoteStorage()->abort();
					bAbortCalled = true;
				}
            }
            break;
		case eSaveUpload_FileDataUploaded:
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage( IDS_TOOLTIPS_SAVETRANSFER_UPLOAD, IDS_SAVE_TRANSFER_UPLOADCOMPLETE, uiIDA,1,ProfileManager.GetPrimaryPad(),CrossSaveUploadFinishedCallback,pClass);
				pClass->m_eSaveUploadState = esaveUpload_Finished;
			}
			break;
		case eSaveUpload_Cancelled: // this is no longer used
			assert(0);//			pClass->m_eSaveUploadState = eSaveUpload_Idle;
			break;
        case eSaveUpload_Error:
			{
				if(pClass->m_saveTransferUploadCancelled)
				{
					pClass->m_eSaveUploadState = eSaveUpload_Idle;
				}
				else
				{
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestErrorMessage( IDS_TOOLTIPS_SAVETRANSFER_UPLOAD, IDS_SAVE_TRANSFER_UPLOADFAILED, uiIDA,1,ProfileManager.GetPrimaryPad(),CrossSaveUploadFinishedCallback,pClass);
					pClass->m_eSaveUploadState = esaveUpload_Finished;
				}
			}
            break;
		case esaveUpload_Finished:
			// waiting for dialog to be dismissed
			break;
        }
        Sleep(50);
    }

    return 0;

}

void UIScene_LoadOrJoinMenu::SaveUploadReturned(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;

	if(pClass->m_saveTransferUploadCancelled)
	{
		UINT uiIDA[1] = { IDS_CONFIRM_OK };
		ui.RequestErrorMessage( IDS_CANCEL_UPLOAD_TITLE, IDS_CANCEL_UPLOAD_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(), CrossSaveUploadFinishedCallback, pClass );
		pClass->m_eSaveUploadState=esaveUpload_Finished;
	}
	else
    {
		if(s == SonyRemoteStorage::e_setDataSucceeded)
		   pClass->m_eSaveUploadState = eSaveUpload_FileDataUploaded;
		else if ( !pClass->m_saveTransferUploadCancelled )
			pClass->m_eSaveUploadState = eSaveUpload_Error;
	}
}

void UIScene_LoadOrJoinMenu::CancelSaveUploadCallback(LPVOID lpParam)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu *) lpParam;
	pClass->m_saveTransferUploadCancelled = true;
	app.DebugPrintf("m_saveTransferUploadCancelled = true\n");
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1, -1, -1, -1,-1,-1,-1,-1);		// MGH -  added - remove the "cancel" tooltip, so the player knows it's underway (really needs a "cancelling" message)

	pClass->m_bIgnoreInput = true;
}

int UIScene_LoadOrJoinMenu::SaveTransferDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;
	// results switched for this dialog
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		// upload the save
		pClass->LaunchSaveUpload();

		pClass->m_bIgnoreInput=false;
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}
	return 0;
}
#endif // SONY_REMOTE_STORAGE_UPLOAD


#if defined _XBOX_ONE
void UIScene_LoadOrJoinMenu::LaunchSaveTransfer()
{
	SaveTransferStateContainer *stateContainer = new SaveTransferStateContainer();
	stateContainer->m_iProgress = 0;
	stateContainer->m_bSaveTransferInProgress = false;
	stateContainer->m_bSaveTransferCancelled = false;
	stateContainer->m_iPad = m_iPad;
	stateContainer->m_eSaveTransferState = C4JStorage::eSaveTransfer_Idle;
	stateContainer->m_pClass = this;

    LoadingInputParams *loadingParams = new LoadingInputParams();
    loadingParams->func = &UIScene_LoadOrJoinMenu::DownloadXbox360SaveThreadProc;
    loadingParams->lpParam = (LPVOID)stateContainer;

    UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
    completionData->bShowBackground=TRUE;
    completionData->bShowLogo=TRUE;
    completionData->type = e_ProgressCompletion_NavigateBackToScene;
    completionData->iPad = DEFAULT_XUI_MENU_USER;
    completionData->bRequiresUserAction=TRUE;
    loadingParams->completionData = completionData;

    loadingParams->cancelFunc=&UIScene_LoadOrJoinMenu::CancelSaveTransferCallback;
    loadingParams->m_cancelFuncParam=stateContainer;
    loadingParams->cancelText=IDS_TOOLTIPS_CANCEL;

    ui.NavigateToScene(m_iPad,eUIScene_FullscreenProgress, loadingParams);
}



int UIScene_LoadOrJoinMenu::DownloadXbox360SaveThreadProc( LPVOID lpParameter )
{
    Compression::UseDefaultThreadStorage();

	SaveTransferStateContainer *pStateContainer = (SaveTransferStateContainer *) lpParameter;
    Minecraft *pMinecraft=Minecraft::GetInstance();
    ConsoleSaveFile* pSave = NULL;

	while(StorageManager.SaveTransferClearState()!=C4JStorage::eSaveTransfer_Idle)
	{
		Sleep(5);
	}

    pStateContainer->m_bSaveTransferInProgress=true;

    UIScene_LoadOrJoinMenu::s_eSaveTransferFile = eSaveTransferFile_Marker;
    RequestFileSize( pStateContainer, L"completemarker" );

    while((pStateContainer->m_eSaveTransferState!=C4JStorage::eSaveTransfer_Idle) && pStateContainer->m_bSaveTransferInProgress && !pStateContainer->m_bSaveTransferCancelled)
    {
        switch(pStateContainer->m_eSaveTransferState)
        {
        case C4JStorage::eSaveTransfer_Idle:
            break;
        case C4JStorage::eSaveTransfer_FileSizeRetrieved:
            switch(UIScene_LoadOrJoinMenu::s_eSaveTransferFile)
            {
            case eSaveTransferFile_Marker:
				if(UIScene_LoadOrJoinMenu::s_ulFileSize == 0)
				{
					pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_NONE_FOUND);
					pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;
				}
				else
				{
					RequestFileData( pStateContainer, L"completemarker" );
				}
                break;
            case eSaveTransferFile_Metadata:
                RequestFileData( pStateContainer, L"metadata" );
                break;
            case eSaveTransferFile_SaveData:
                RequestFileData( pStateContainer, L"savedata" );
                break;
            };
            break;
        case C4JStorage::eSaveTransfer_GettingFileData:

            break;
        case C4JStorage::eSaveTransfer_FileDataRetrieved:
            switch(UIScene_LoadOrJoinMenu::s_eSaveTransferFile)
            {
            case eSaveTransferFile_Marker:
				// MGH - the marker file now contains the save file version number
				// if the version is higher than we handle, cancel the download.
				if(UIScene_LoadOrJoinMenu::s_transferData[0] > SAVE_FILE_VERSION_NUMBER)
				{
					pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_NONE_FOUND);
					pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;
				}
				else
                {
					UIScene_LoadOrJoinMenu::s_eSaveTransferFile = eSaveTransferFile_Metadata;
					RequestFileSize( pStateContainer, L"metadata" );
				}
                break;
            case eSaveTransferFile_Metadata:
                {
                    ByteArrayInputStream bais(UIScene_LoadOrJoinMenu::s_transferData);
                    DataInputStream dis(&bais);

                    wstring saveTitle = dis.readUTF();
                    StorageManager.SetSaveTitle(saveTitle.c_str());

                    wstring saveUniqueName = dis.readUTF();

					// 4J Stu - Don't set this any more. We added it so that we could share the ban list data for this save
					// However if the player downloads the same save multiple times, it will overwrite the previous version
					// with that filname, and they could have made changes to it.
                    //StorageManager.SetSaveUniqueFilename((wchar_t *)saveUniqueName.c_str());

                    int thumbnailSize = dis.readInt();
                    if(thumbnailSize > 0)
                    {
                        byteArray ba(thumbnailSize);
                        dis.readFully(ba);



						// retrieve the seed value from the image metadata, we need to change to host options, then set it back again
						bool bHostOptionsRead = false;
						unsigned int uiHostOptions = 0;
						DWORD dwTexturePack;
						__int64 seedVal;

						char szSeed[50];
						ZeroMemory(szSeed,50);
						app.GetImageTextData(ba.data,ba.length,(unsigned char *)&szSeed,uiHostOptions,bHostOptionsRead,dwTexturePack);
						sscanf_s(szSeed, "%I64d", &seedVal);

						app.SetGameHostOption(uiHostOptions, eGameHostOption_WorldSize, e_worldSize_Classic);		// force the classic world size on, otherwise it's unknown and we can't expand


						BYTE bTextMetadata[88];
						ZeroMemory(bTextMetadata,88);

						int iTextMetadataBytes = app.CreateImageTextData(bTextMetadata, seedVal, true, uiHostOptions, dwTexturePack);
						// set the icon and save image
						StorageManager.SetSaveImages(ba.data, ba.length, NULL, 0, bTextMetadata, iTextMetadataBytes);

                        delete ba.data;
                    }

                    UIScene_LoadOrJoinMenu::s_transferData = byteArray();
                    UIScene_LoadOrJoinMenu::s_eSaveTransferFile = eSaveTransferFile_SaveData;
                    RequestFileSize( pStateContainer, L"savedata" );
                }
                break;
            case eSaveTransferFile_SaveData:
                {
#ifdef SPLIT_SAVES
					if(!pStateContainer->m_bSaveTransferCancelled)
					{
						ConsoleSaveFileOriginal oldFormatSave( L"Temp name", UIScene_LoadOrJoinMenu::s_transferData.data, UIScene_LoadOrJoinMenu::s_transferData.length, false, SAVE_FILE_PLATFORM_X360 );
						pSave = new ConsoleSaveFileSplit( &oldFormatSave, false, pMinecraft->progressRenderer );

						pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_STAGE_SAVING);
						if(!pStateContainer->m_bSaveTransferCancelled) pSave->Flush(false,false);
					}
                    pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Saving;

#else
                    pSave = new ConsoleSaveFileOriginal( wSaveName, m_transferData.data, m_transferData.length, false, SAVE_FILE_PLATFORM_X360 );
                    pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Converting;
#endif
                    delete UIScene_LoadOrJoinMenu::s_transferData.data;
                    UIScene_LoadOrJoinMenu::s_transferData = byteArray();
                }
                break;
            };

            pStateContainer->m_iProgress=0;
            break;
        case C4JStorage::eSaveTransfer_Converting:
#if 0
            pSave->ConvertToLocalPlatform();

            pMinecraft->progressRenderer->progressStage(IDS_SAVETRANSFER_STAGE_SAVING);
            if(!pStateContainer->m_bSaveTransferCancelled) pSave->Flush(false,false);

            pStateContainer->m_iProgress+=1;
            if(pStateContainer->m_iProgress==101)
            {
                pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Saving;
                pStateContainer->m_iProgress=0;
                break;
            }
            pMinecraft->progressRenderer->progressStagePercentage(pStateContainer->m_iProgress);
#endif
            break;
        case C4JStorage::eSaveTransfer_Saving:
            // On Durango/Orbis, we need to wait for all the asynchronous saving processes to complete before destroying the levels, as that will ultimately delete
            // the directory level storage & therefore the ConsoleSaveSplit instance, which needs to be around until all the sub files have completed saving.
#if defined(_DURANGO) || defined(__ORBIS__)
			pMinecraft->progressRenderer->progressStage(IDS_PROGRESS_SAVING_TO_DISC);

			while(StorageManager.GetSaveState() != C4JStorage::ESaveGame_Idle )
            {
                Sleep(10);

				// 4J Stu - DO NOT tick this here. The main thread should be the only place ticking the StorageManager. You WILL get crashes.
                //StorageManager.Tick();
            }
#endif

            delete pSave;

#ifdef _XBOX_ONE
			pMinecraft->progressRenderer->progressStage(IDS_SAVE_TRANSFER_DOWNLOAD_AND_CONVERT_COMPLETE);
#endif

            pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;

			 // wipe the list and repopulate it
			 if(!pStateContainer->m_bSaveTransferCancelled) pStateContainer->m_pClass->m_iState=e_SavesRepopulateAfterTransferDownload;

            //pClass->m_iProgress+=1;
            //if(pClass->m_iProgress==101)
            //{
            //	pClass->m_iProgress=0;
            //	pClass->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;
            //	pMinecraft->progressRenderer->progressStage( IDS_SAVE_TRANSFER_DOWNLOAD_AND_CONVERT_COMPLETE );

            //	break;
            //}
            //pMinecraft->progressRenderer->progressStagePercentage(pClass->m_iProgress);

            break;
        }
        Sleep(50);
    }

    if(pStateContainer->m_bSaveTransferCancelled)
    {
        WCHAR wcTemp[256];

        pStateContainer->m_bSaveTransferCancelled=false;
        swprintf(wcTemp,app.GetString(IDS_SAVE_TRANSFER_DOWNLOAD_CANCELLED));
        m_wstrStageText=wcTemp;
        pMinecraft->progressRenderer->progressStage( m_wstrStageText );

    }

    pStateContainer->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;
    pStateContainer->m_bSaveTransferInProgress=false;

	delete pStateContainer;

    return 0;
}

void UIScene_LoadOrJoinMenu::RequestFileSize( SaveTransferStateContainer *pClass, wchar_t *filename )
{
    Minecraft *pMinecraft=Minecraft::GetInstance();

    // get the save file size
    pMinecraft->progressRenderer->progressStart(IDS_SAVETRANSFER_TITLE_GET);
    pMinecraft->progressRenderer->progressStage( IDS_SAVETRANSFER_STAGE_GET_DETAILS );

#ifdef _DEBUG_MENUS_ENABLED
    if(app.GetLoadSavesFromFolderEnabled())
    {
        ZeroMemory(&m_debugTransferDetails, sizeof(C4JStorage::SAVETRANSFER_FILE_DETAILS) );

        File targetFile( wstring(L"FakeTMSPP\\").append(filename) );
        if(targetFile.exists()) m_debugTransferDetails.ulFileLen = targetFile.length();

        SaveTransferReturned(pClass,&m_debugTransferDetails);
    }
    else
#endif
    {
        do
		{
			pMinecraft->progressRenderer->progressStart(IDS_SAVETRANSFER_TITLE_GET);
			pMinecraft->progressRenderer->progressStage( IDS_SAVETRANSFER_STAGE_GET_DETAILS );
			Sleep(1);
			pClass->m_eSaveTransferState=StorageManager.SaveTransferGetDetails(pClass->m_iPad,C4JStorage::eGlobalStorage_TitleUser,filename,&UIScene_LoadOrJoinMenu::SaveTransferReturned,pClass);
		}
		while(pClass->m_eSaveTransferState == C4JStorage::eSaveTransfer_Busy && !pClass->m_bSaveTransferCancelled );
    }
}

void UIScene_LoadOrJoinMenu::RequestFileData( SaveTransferStateContainer *pClass, wchar_t *filename )
{
    Minecraft *pMinecraft=Minecraft::GetInstance();
    WCHAR wcTemp[256];

    pMinecraft->progressRenderer->progressStagePercentage(0);

    swprintf(wcTemp,app.GetString(IDS_SAVETRANSFER_STAGE_GET_DATA),0,UIScene_LoadOrJoinMenu::s_ulFileSize);
    m_wstrStageText=wcTemp;

    pMinecraft->progressRenderer->progressStage( m_wstrStageText );

#ifdef _DEBUG_MENUS_ENABLED
    if(app.GetLoadSavesFromFolderEnabled())
    {
        File targetFile( wstring(L"FakeTMSPP\\").append(filename) );
        if(targetFile.exists())
        {
            HANDLE hSaveFile = CreateFile( targetFile.getPath().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);

            m_debugTransferDetails.pbData = new BYTE[m_debugTransferDetails.ulFileLen];

            DWORD numberOfBytesRead = 0;
            ReadFile( hSaveFile,m_debugTransferDetails.pbData,m_debugTransferDetails.ulFileLen,&numberOfBytesRead,NULL);
            assert(numberOfBytesRead == m_debugTransferDetails.ulFileLen);

            CloseHandle(hSaveFile);

            SaveTransferReturned(pClass,&m_debugTransferDetails);
        }
    }
    else
#endif
    {
        do
		{
			pMinecraft->progressRenderer->progressStart(IDS_SAVETRANSFER_TITLE_GET);
			pMinecraft->progressRenderer->progressStage( -1 );
			Sleep(1);
			pClass->m_eSaveTransferState=StorageManager.SaveTransferGetData(pClass->m_iPad,C4JStorage::eGlobalStorage_TitleUser,filename,&UIScene_LoadOrJoinMenu::SaveTransferReturned,&UIScene_LoadOrJoinMenu::SaveTransferUpdateProgress,pClass,pClass);
		}
		while(pClass->m_eSaveTransferState == C4JStorage::eSaveTransfer_Busy && !pClass->m_bSaveTransferCancelled );
    }
}

int UIScene_LoadOrJoinMenu::SaveTransferReturned(LPVOID lpParam,C4JStorage::SAVETRANSFER_FILE_DETAILS *pSaveTransferDetails)
{
    SaveTransferStateContainer* pClass = (SaveTransferStateContainer *) lpParam;
    app.DebugPrintf("Save Transfer - size is %d\n",pSaveTransferDetails->ulFileLen);

    // if the file data is null, then assume this is the file size retrieval
    if(pSaveTransferDetails->pbData==NULL)
    {
        pClass->m_eSaveTransferState=C4JStorage::eSaveTransfer_FileSizeRetrieved;
        UIScene_LoadOrJoinMenu::s_ulFileSize=pSaveTransferDetails->ulFileLen;
    }
    else
    {
        delete UIScene_LoadOrJoinMenu::s_transferData.data;
        UIScene_LoadOrJoinMenu::s_transferData = byteArray(pSaveTransferDetails->pbData, UIScene_LoadOrJoinMenu::s_ulFileSize);
        pClass->m_eSaveTransferState=C4JStorage::eSaveTransfer_FileDataRetrieved;
    }

    return 0;
}

int UIScene_LoadOrJoinMenu::SaveTransferUpdateProgress(LPVOID lpParam,unsigned long ulBytesReceived)
{
    WCHAR wcTemp[256];

    SaveTransferStateContainer* pClass = (SaveTransferStateContainer *) lpParam;
    Minecraft *pMinecraft=Minecraft::GetInstance();

    if(pClass->m_bSaveTransferCancelled) // was cancelled
    {
        pMinecraft->progressRenderer->progressStage(IDS_SAVE_TRANSFER_DOWNLOAD_CANCELLING);
        swprintf(wcTemp,app.GetString(IDS_SAVE_TRANSFER_DOWNLOAD_CANCELLING));
        m_wstrStageText=wcTemp;
        pMinecraft->progressRenderer->progressStage( m_wstrStageText );
    }
    else
    {
        unsigned int uiProgress=(unsigned int)(((float)ulBytesReceived/float(UIScene_LoadOrJoinMenu::s_ulFileSize))*100.0f);

        pMinecraft->progressRenderer->progressStagePercentage(uiProgress);
        swprintf(wcTemp,app.GetString(IDS_SAVETRANSFER_STAGE_GET_DATA),((float)(ulBytesReceived))/1024000.0f,((float)UIScene_LoadOrJoinMenu::s_ulFileSize)/1024000.0f);
        m_wstrStageText=wcTemp;
        pMinecraft->progressRenderer->progressStage( m_wstrStageText );
    }

    return 0;
}

void UIScene_LoadOrJoinMenu::CancelSaveTransferCallback(LPVOID lpParam)
{
    SaveTransferStateContainer* pClass = (SaveTransferStateContainer *) lpParam;

    if(!pClass->m_bSaveTransferCancelled)
    {
        StorageManager.CancelSaveTransfer(UIScene_LoadOrJoinMenu::CancelSaveTransferCompleteCallback,pClass);

        pClass->m_bSaveTransferCancelled=true;
    }
    //pClass->m_bSaveTransferInProgress=false;
}

int UIScene_LoadOrJoinMenu::CancelSaveTransferCompleteCallback(LPVOID lpParam)
{
    SaveTransferStateContainer* pClass = (SaveTransferStateContainer *) lpParam;
    // change the state to idle to get the download thread to terminate
    pClass->m_eSaveTransferState=C4JStorage::eSaveTransfer_Idle;
    return 0;
}

int UIScene_LoadOrJoinMenu::NeedSyncMessageReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu *pClass = (UIScene_LoadOrJoinMenu *)pParam;
    LoadMenuInitData *params = (LoadMenuInitData *)pParam;

    if( result == C4JStorage::EMessage_ResultAccept )
    {
        // navigate to the settings scene
        ui.NavigateToScene(ProfileManager.GetPrimaryPad(), eUIScene_LoadMenu, pClass->m_loadMenuInitData);
    }
    else
    {
        delete pClass->m_loadMenuInitData;
        pClass->m_bIgnoreInput = false;
    }

    return 0;
}


#endif


#ifdef _XBOX_ONE
void UIScene_LoadOrJoinMenu::HandleDLCLicenseChange()
{
	// may have installed Halloween on this menu
	app.StartInstallDLCProcess(m_iPad);
}
#endif

#if defined _XBOX_ONE || defined __ORBIS__
int UIScene_LoadOrJoinMenu::CopySaveDialogReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)pParam;

	if(result==C4JStorage::EMessage_ResultAccept)
	{

		LoadingInputParams *loadingParams = new LoadingInputParams();
		void *uniqueId = (LPVOID)pClass->GetCallbackUniqueId();
		loadingParams->func = &UIScene_LoadOrJoinMenu::CopySaveThreadProc;
		loadingParams->lpParam = uniqueId;
		loadingParams->waitForThreadToDelete = true;

		UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
		completionData->bShowBackground=TRUE;
		completionData->bShowLogo=TRUE;
		completionData->type = e_ProgressCompletion_NavigateBackToScene;
		completionData->iPad = DEFAULT_XUI_MENU_USER;
		loadingParams->completionData = completionData;

		loadingParams->cancelFunc=&UIScene_LoadOrJoinMenu::CancelCopySaveCallback;
		loadingParams->m_cancelFuncParam=uniqueId;
		loadingParams->cancelText=IDS_TOOLTIPS_CANCEL;

		ui.NavigateToScene(iPad,eUIScene_FullscreenProgress, loadingParams);
	}
	else
	{
		pClass->m_bIgnoreInput=false;
	}

    return 0;
}

int UIScene_LoadOrJoinMenu::CopySaveThreadProc( LPVOID lpParameter )
{
	Minecraft *pMinecraft=Minecraft::GetInstance();
	pMinecraft->progressRenderer->progressStart(IDS_PROGRESS_COPYING_SAVE);
	pMinecraft->progressRenderer->progressStage( -1 );

	ui.EnterCallbackIdCriticalSection();
	UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParameter);
	if( pClass )
	{
		pClass->m_bCopying = true;
		pClass->m_bCopyingCancelled = false;
		ui.LeaveCallbackIdCriticalSection();
		// Copy save data takes two callbacks - one for completion, and one for progress. The progress callback also lets us cancel the operation, if we return false.
		StorageManager.CopySaveData(&pClass->m_pSaveDetails->SaveInfoA[pClass->m_iSaveListIndex - pClass->m_iDefaultButtonsC],UIScene_LoadOrJoinMenu::CopySaveDataReturned,UIScene_LoadOrJoinMenu::CopySaveDataProgress,lpParameter);

		bool bContinue = true;
		do
		{
			Sleep(100);
			ui.EnterCallbackIdCriticalSection();
			pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParameter);
			if( pClass )
			{
				bContinue = pClass->m_bCopying;
			}
			else
			{
				bContinue = false;
			}
			ui.LeaveCallbackIdCriticalSection();
		} while( bContinue );
	}
	else
	{
		ui.LeaveCallbackIdCriticalSection();
	}

	return 0;
}

int UIScene_LoadOrJoinMenu::CopySaveDataReturned(LPVOID lpParam, bool success, C4JStorage::ESaveGameState stat)
{
	ui.EnterCallbackIdCriticalSection();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParam);

	if(pClass)
	{
		if(success)
		{
			pClass->m_bCopying = false;
			// wipe the list and repopulate it
			pClass->m_iState=e_SavesRepopulateAfterDelete;
			ui.LeaveCallbackIdCriticalSection();
		}
		else
		{
#ifdef __ORBIS__
			UINT uiIDA[1];
			// you cancelled the save on exit after choosing exit and save? You go back to the Exit choices then.
			uiIDA[0]=IDS_OK;

			if( stat == C4JStorage::ESaveGame_CopyCompleteFailLocalStorage )
			{
				ui.LeaveCallbackIdCriticalSection();
				ui.RequestErrorMessage(IDS_COPYSAVE_FAILED_TITLE, IDS_COPYSAVE_FAILED_LOCAL, uiIDA, 1, ProfileManager.GetPrimaryPad(), CopySaveErrorDialogFinishedCallback, lpParam);
			}
			else if( stat == C4JStorage::ESaveGame_CopyCompleteFailQuota )
			{
				ui.LeaveCallbackIdCriticalSection();
				ui.RequestErrorMessage(IDS_COPYSAVE_FAILED_TITLE, IDS_COPYSAVE_FAILED_QUOTA, uiIDA, 1, ProfileManager.GetPrimaryPad(), CopySaveErrorDialogFinishedCallback, lpParam);
			}
			else
			{
				pClass->m_bCopying = false;
				ui.LeaveCallbackIdCriticalSection();
			}
#else
			pClass->m_bCopying = false;
			ui.LeaveCallbackIdCriticalSection();
#endif
		}
	}
	else
	{
		ui.LeaveCallbackIdCriticalSection();
	}
	return 0;
}

bool UIScene_LoadOrJoinMenu::CopySaveDataProgress(LPVOID lpParam, int percent)
{
	bool bContinue = false;
	ui.EnterCallbackIdCriticalSection();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParam);
	if( pClass )
	{
		bContinue = !pClass->m_bCopyingCancelled;
	}
	ui.LeaveCallbackIdCriticalSection();
	Minecraft *pMinecraft=Minecraft::GetInstance();
	pMinecraft->progressRenderer->progressStagePercentage(percent);

	return bContinue;
}

void UIScene_LoadOrJoinMenu::CancelCopySaveCallback(LPVOID lpParam)
{
	ui.EnterCallbackIdCriticalSection();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)lpParam);
	if( pClass )
	{
		pClass->m_bCopyingCancelled = true;
	}
	ui.LeaveCallbackIdCriticalSection();
}

int UIScene_LoadOrJoinMenu::CopySaveErrorDialogFinishedCallback(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	ui.EnterCallbackIdCriticalSection();
    UIScene_LoadOrJoinMenu* pClass = (UIScene_LoadOrJoinMenu*)ui.GetSceneFromCallbackId((size_t)pParam);
	if( pClass )
	{
		pClass->m_bCopying = false;
	}
	ui.LeaveCallbackIdCriticalSection();

	return 0;
}

#endif // _XBOX_ONE
