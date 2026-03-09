#include "stdafx.h"
#include "XUI_SkinSelect.h"
#include "XUI_Ctrl_MinecraftSkinPreview.h"

#define SKIN_SELECT_PACK_DEFAULT 0
#define SKIN_SELECT_PACK_FAVORITES 1
//#define SKIN_SELECT_PACK_PLAYER_CUSTOM 1
#define SKIN_SELECT_MAX_DEFAULTS 2

WCHAR *CScene_SkinSelect::wchDefaultNamesA[]=
{
	L"USE LOCALISED VERSION", // Server selected
	L"Steve",
	L"Tennis Steve",
	L"Tuxedo Steve",
	L"Athlete Steve",
	L"Scottish Steve",
	L"Prisoner Steve",
	L"Cyclist Steve",
	L"Boxer Steve",
};


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_SkinSelect::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad=*static_cast<int *>(pInitData->pvInitData);
	// if we're not in the game, we need to use basescene 0 
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);
	m_bIgnoreInput=false;

	// 4J Stu - Added this so that we have skins loaded
	// 4J-PB - Need to check for installed DLC
	//if( (!app.DLCInstalled() || app.m_dlcManager.NeedsUpdated()) && !app.DLCInstallPending()) app.StartInstallDLCProcess(m_iPad);

	// StartInstallDLCProcess will check for all conditions within the call
	MapChildControls();

	m_selectedText.SetText( app.GetString( IDS_SELECTED ) );

	updateClipping();

	m_packIndex = SKIN_SELECT_PACK_DEFAULT;
	m_skinIndex = 0;
	m_currentSkinPath = app.GetPlayerSkinName(m_iPad);
	m_originalSkinId = app.GetPlayerSkinId(m_iPad);
	m_currentPack = nullptr;
	m_bSlidingSkins = false;
	m_bAnimatingMove = false;
	currentPackCount = 0;

	m_currentNavigation = eSkinNavigation_Skin;
	m_normalTabs.SetShow( TRUE );
	m_selectedTabs.SetShow( FALSE );
	m_packLeft.SetEnable(FALSE);
	m_packRight.SetEnable(FALSE);


	for(BYTE i = 0; i < sidePreviewControls; ++i)
	{
		//m_previewNextControl->SetAutoRotate(true);
		m_previewNextControls[i]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Left);
		//m_previewPreviousControl->SetAutoRotate(true);
		m_previewPreviousControls[i]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Right);
	}

	// block input if we're waiting for DLC to install. The end of dlc mounting custom message will fill the save list
	if(app.StartInstallDLCProcess(m_iPad))
	{
		// DLC mounting in progress, so disable input
		m_bIgnoreInput=true;
		m_timer.SetShow( TRUE );
		m_charactersGroup.SetShow( FALSE );
		m_skinDetails.SetShow( FALSE );
		m_imagePadlock.SetShow( FALSE );
		m_selectedGroup.SetShow( FALSE );
	}
	else
	{
		m_timer.SetShow( FALSE );

		if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin)>0)
		{
			// Change to display the favorites if there are any. The current skin will be in there (probably) - need to check for it
			m_currentPack = app.m_dlcManager.getPackContainingSkin(m_currentSkinPath);
			bool bFound;
			if(m_currentPack != nullptr)
			{
				m_packIndex = app.m_dlcManager.getPackIndex(m_currentPack,bFound,DLCManager::e_DLCType_Skin) + SKIN_SELECT_MAX_DEFAULTS;
			}
		}

		// If we have any favourites, set this to the favourites
		// first validate the favorite skins - we might have uninstalled the DLC needed for them
		app.ValidateFavoriteSkins(m_iPad);

		if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
		{
			m_packIndex = SKIN_SELECT_PACK_FAVORITES;
		}

		handlePackIndexChanged();
		updateCurrentFocus();
	}

	// Display the tooltips

	// if we're not in the game, we need to use basescene 0 
	if(bNotInGame)
	{
		ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT_SKIN,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
		CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
	}
	else
	{		
		ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT_SKIN,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
		CXuiSceneBase::ShowBackground( m_iPad, FALSE );
		CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );
	}


	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad, false);
		CXuiSceneBase::ShowLogo( m_iPad, FALSE );
	}
	else
	{
		if(bNotInGame)
		{
			CXuiSceneBase::ShowLogo( DEFAULT_XUI_MENU_USER, FALSE );
		}
		else
		{
			CXuiSceneBase::ShowLogo( m_iPad, FALSE );
		}
	}

	return S_OK;
}


HRESULT CScene_SkinSelect::OnKeyUp(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_RTHUMB_LEFT:
		m_previewControl->m_incYRot = false;
		break;
	case VK_PAD_RTHUMB_RIGHT:
		m_previewControl->m_decYRot = false;
		break;
	case VK_PAD_RTHUMB_UP:
		//m_previewControl->m_incXRot = false;
		break;
	case VK_PAD_RTHUMB_DOWN:
		//m_previewControl->m_decXRot = false;
		break;
	case VK_PAD_RTHUMB_UPLEFT:
		m_previewControl->m_incYRot = false;
		//m_previewControl->m_incXRot = false;
		break;
	case VK_PAD_RTHUMB_UPRIGHT:
		m_previewControl->m_decYRot = false;
		//m_previewControl->m_incXRot = false;
		break;
	case VK_PAD_RTHUMB_DOWNRIGHT:
		m_previewControl->m_decYRot = false;
		//m_previewControl->m_decXRot = false;
		break;
	case VK_PAD_RTHUMB_DOWNLEFT:
		m_previewControl->m_incYRot = false;
		//m_previewControl->m_decXRot = false;
		break;
	}
	return S_OK;
}


HRESULT CScene_SkinSelect::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// 4J Stu - We don't want the press anim to play for the scrolling unless we are actually scrolling
	//ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	// ignore any key press if we are animating a move
	//if(m_bAnimatingMove) return S_OK;

	HRESULT hr=S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_A:
		ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
		// if the profile data has been changed, then force a profile write
		// It seems we're allowed to break the 5 minute rule if it's the result of a user action
		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:
			app.SetPlayerSkin(pInputData->UserIndex, m_skinIndex);
			app.SetPlayerCape(pInputData->UserIndex, 0);
			m_currentSkinPath = app.GetPlayerSkinName(m_iPad);
			m_originalSkinId = app.GetPlayerSkinId(m_iPad);
			m_selectedGroup.SetShow( TRUE );
			CXuiSceneBase::PlayUISFX(eSFX_Press);
			break;
		case SKIN_SELECT_PACK_FAVORITES:
			if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
			{		
				// get the pack number from the skin id
				wchar_t chars[256];
				swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(m_iPad,m_skinIndex));

				DLCPack *Pack=app.m_dlcManager.getPackContainingSkin(chars);	

				if(Pack)
				{
					DLCSkinFile *skinFile = Pack->getSkinFile(chars);
					app.SetPlayerSkin(pInputData->UserIndex, skinFile->getPath());
					app.SetPlayerCape(pInputData->UserIndex, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
					m_selectedGroup.SetShow( TRUE );
					m_currentSkinPath = app.GetPlayerSkinName(m_iPad);
					m_originalSkinId = app.GetPlayerSkinId(m_iPad);
					app.SetPlayerFavoriteSkinsPos(m_iPad,m_skinIndex);
				}
			}
			break;
		default:
			if( m_currentPack != nullptr )
			{
				DLCSkinFile *skinFile = m_currentPack->getSkinFile(m_skinIndex);
				
				// Is this a free skin?

				if(!skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free ))
				{
					// do we have a license?
					if(!m_currentPack->hasPurchasedFile( DLCManager::e_DLCType_Skin, skinFile->getPath() ))
					{
						// no
						UINT uiIDA[1];
						uiIDA[0]=IDS_OK;

						// We need to upsell the full version
						if(ProfileManager.IsGuest(m_iPad))
						{
							// can't buy
							StorageManager.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1);
						}
						// are we online?
						else if(!ProfileManager.IsSignedInLive(pInputData->UserIndex))
						{
							// need to be signed in to live
							StorageManager.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_XBOXLIVE_NOTIFICATION, uiIDA, 1);
						}
						else
						{
							// upsell

							DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(m_currentPack->getPurchaseOfferId());
							ULONGLONG ullOfferID_Full;

							if(pDLCInfo!=nullptr)
							{
								ullOfferID_Full=pDLCInfo->ullOfferID_Full;
							}
							else
							{
								ullOfferID_Full=m_currentPack->getPurchaseOfferId();
							}

							// tell sentient about the upsell of the full version of the skin pack
							TelemetryManager->RecordUpsellPresented(pInputData->UserIndex, eSet_UpsellID_Skin_DLC, ullOfferID_Full & 0xFFFFFFFF);

							UINT uiIDA[2];
							uiIDA[0]=IDS_CONFIRM_OK;
							uiIDA[1]=IDS_CONFIRM_CANCEL;

							StorageManager.RequestMessageBox(IDS_UNLOCK_DLC_TITLE, IDS_UNLOCK_DLC_SKIN, uiIDA, 2, pInputData->UserIndex,&CScene_SkinSelect::UnlockSkinReturned,this,app.GetStringTable());
						}
					}
					else
					{
						app.SetPlayerSkin(pInputData->UserIndex, skinFile->getPath());
						app.SetPlayerCape(pInputData->UserIndex, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
						m_selectedGroup.SetShow( TRUE );
						m_currentSkinPath = app.GetPlayerSkinName(m_iPad);
						m_originalSkinId = app.GetPlayerSkinId(m_iPad);

						// push this onto the favorite list
						AddFavoriteSkin(m_iPad,GET_DLC_SKIN_ID_FROM_BITMASK(m_originalSkinId));
					}
				}
				else
				{
					app.SetPlayerSkin(pInputData->UserIndex, skinFile->getPath());
					app.SetPlayerCape(pInputData->UserIndex, skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape));
					m_selectedGroup.SetShow( TRUE );
					m_currentSkinPath = app.GetPlayerSkinName(m_iPad);
					m_originalSkinId = app.GetPlayerSkinId(m_iPad);

					// push this onto the favorite list
					AddFavoriteSkin(m_iPad,GET_DLC_SKIN_ID_FROM_BITMASK(m_originalSkinId));

				}
			}

			CXuiSceneBase::PlayUISFX(eSFX_Press);
			break;
		}
				
	break;
	case VK_PAD_B:
	case VK_ESCAPE:
		ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
		app.CheckGameSettingsChanged(true,pInputData->UserIndex);

		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
		break;
#if 0
	case VK_PAD_RSHOULDER:
		{			
			DWORD startingIndex = m_packIndex;
			m_packIndex = getNextPackIndex(m_packIndex);
			if(startingIndex != m_packIndex)
			{
				handlePackIndexChanged();
			}
		}
		break;
	case VK_PAD_LSHOULDER:
		{
			DWORD startingIndex = m_packIndex;
			m_packIndex = getPreviousPackIndex(m_packIndex);
			if(startingIndex != m_packIndex)
			{
				handlePackIndexChanged();
			}
		}
		break;
#endif
	case VK_PAD_DPAD_UP:
	case VK_PAD_LTHUMB_UP:
	case VK_PAD_DPAD_DOWN:
	case VK_PAD_LTHUMB_DOWN:
		{
			if(m_packIndex==SKIN_SELECT_PACK_FAVORITES)
			{
				if(app.GetPlayerFavoriteSkinsCount(m_iPad)==0)
				{
					// ignore this, since there are no skins being displayed
					break;
				}
			}
		
			ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
			switch(m_currentNavigation)
			{
			case eSkinNavigation_Pack:
				m_currentNavigation = eSkinNavigation_Skin;
				break;
			case eSkinNavigation_Skin:
				m_currentNavigation = eSkinNavigation_Pack;
				break;
			};		
			updateCurrentFocus();
		}
		break;
	case VK_PAD_DPAD_LEFT:
	case VK_PAD_LTHUMB_LEFT:
		{
			if( m_currentNavigation == eSkinNavigation_Skin )
			{
				if(!m_bAnimatingMove)
				{
					ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
					CXuiSceneBase::PlayUISFX(eSFX_Scroll);
					
					m_skinIndex = getPreviousSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

					m_bSlidingSkins = true;
					m_bAnimatingMove = true;

					m_previewControl->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Left, true);
					m_previewPreviousControls[0]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Forward, true);

					int startFrame, endFrame;
					HRESULT hr = m_charactersGroup.FindNamedFrame(L"CycleRight", &startFrame);
					hr = m_charactersGroup.FindNamedFrame( L"EndCycleRight", &endFrame);
					hr = m_charactersGroup.PlayTimeline(startFrame, startFrame,endFrame,FALSE,FALSE);
				}
			}
			else if( m_currentNavigation == eSkinNavigation_Pack )
			{
				ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
				CXuiSceneBase::PlayUISFX(eSFX_Scroll);
				DWORD startingIndex = m_packIndex;
				m_packIndex = getPreviousPackIndex(m_packIndex);
				if(startingIndex != m_packIndex)
				{
					handlePackIndexChanged();
				}
			}
		}
		break;
	case VK_PAD_DPAD_RIGHT:
	case VK_PAD_LTHUMB_RIGHT:
		{
			if( m_currentNavigation == eSkinNavigation_Skin )
			{
				if(!m_bAnimatingMove)
				{
					ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
					CXuiSceneBase::PlayUISFX(eSFX_Scroll);
					m_skinIndex = getNextSkinIndex(m_skinIndex);
					//handleSkinIndexChanged();

					m_bSlidingSkins = true;
					m_bAnimatingMove = true;

					m_previewControl->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Right, true);
					m_previewNextControls[0]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Forward, true);

					int startFrame, endFrame;
					HRESULT hr = m_charactersGroup.FindNamedFrame(L"CycleLeft", &startFrame);
					hr = m_charactersGroup.FindNamedFrame( L"EndCycleLeft", &endFrame);
					hr = m_charactersGroup.PlayTimeline(startFrame, startFrame,endFrame,FALSE,FALSE);
				}
			}
			else if( m_currentNavigation == eSkinNavigation_Pack )
			{
				ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
				CXuiSceneBase::PlayUISFX(eSFX_Scroll);
				DWORD startingIndex = m_packIndex;
				m_packIndex = getNextPackIndex(m_packIndex);
				if(startingIndex != m_packIndex)
				{
					handlePackIndexChanged();
				}
			}
		}
		break;
	case VK_PAD_RTHUMB_PRESS:
		CXuiSceneBase::PlayUISFX(eSFX_Press);
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->ResetRotation();
		}
		break;
	case VK_PAD_RTHUMB_LEFT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_incYRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_RIGHT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_decYRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_UP:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			//m_previewControl->m_incXRot = true;
			m_previewControl->CyclePreviousAnimation();
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_DOWN:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			//m_previewControl->m_decXRot = true;
			m_previewControl->CycleNextAnimation();
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_UPLEFT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_incYRot = true;
			//m_previewControl->m_incXRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_UPRIGHT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_decYRot = true;
			//m_previewControl->m_incXRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_DOWNRIGHT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_decYRot = true;
			//m_previewControl->m_decXRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	case VK_PAD_RTHUMB_DOWNLEFT:
		if( m_currentNavigation == eSkinNavigation_Skin )
		{
			m_previewControl->m_incYRot = true;
			//m_previewControl->m_decXRot = true;
		}
		else
		{
			CXuiSceneBase::PlayUISFX(eSFX_Scroll);
		}
		break;
	}

	return hr;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_SkinSelect::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	return S_OK;
}

HRESULT CScene_SkinSelect::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransType == XUI_TRANSITION_TO || pTransition->dwTransType == XUI_TRANSITION_BACKTO)
	{		

	}

	return S_OK;
}

HRESULT CScene_SkinSelect::OnTimelineEnd(HXUIOBJ hObjSource, BOOL& bHandled)
{
    if( hObjSource == m_charactersGroup )
	{
		if(m_bSlidingSkins)
		{
			m_bSlidingSkins = false;

			int startFrame, endFrame;
			HRESULT hr = m_charactersGroup.FindNamedFrame(L"Normal", &startFrame);
			hr = m_charactersGroup.FindNamedFrame( L"Normal", &endFrame);
			hr = m_charactersGroup.PlayTimeline(startFrame, startFrame,endFrame,FALSE,FALSE);
		}
		else
		{
			m_previewControl->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Forward, false);
			m_previewNextControls[0]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Left, false);
			m_previewPreviousControls[0]->SetFacing(CXuiCtrlMinecraftSkinPreview::e_SkinPreviewFacing_Right, false);

			handleSkinIndexChanged();

			m_bAnimatingMove = false;

			bHandled = TRUE;
		}
	}
    return S_OK;
}


HRESULT CScene_SkinSelect::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining,false);
}




HRESULT CScene_SkinSelect::OnBasePositionChanged()
{
	updateClipping();

	return S_OK;
}

void CScene_SkinSelect::handleSkinIndexChanged()
{
	BOOL showPrevious = FALSE, showNext = FALSE;
	DWORD previousIndex = 0, nextIndex = 0;
	wstring skinName = L"";
	wstring skinOrigin = L"";
	bool bSkinIsFree=false;
	bool bLicensed=false;
	DLCSkinFile *skinFile=nullptr;
	DLCPack *Pack=nullptr;
	BYTE sidePreviewControlsL,sidePreviewControlsR;
	bool bNoSkinsToShow=false;

	TEXTURE_NAME backupTexture = TN_MOB_CHAR;
	m_selectedGroup.SetShow( FALSE );
	m_skinDetails.SetShow( FALSE );

	if( m_currentPack != nullptr )
	{
		skinFile = m_currentPack->getSkinFile(m_skinIndex);
		m_selectedSkinPath = skinFile->getPath();
		m_selectedCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
		m_vAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
		
		skinName = skinFile->getParameterAsString( DLCManager::e_DLCParamType_DisplayName );
		skinOrigin = skinFile->getParameterAsString( DLCManager::e_DLCParamType_ThemeName );

		if( m_selectedSkinPath.compare( m_currentSkinPath ) == 0 )
		{
			m_selectedGroup.SetShow( TRUE );
		}
		else
		{
			m_selectedGroup.SetShow( FALSE );
		}
			
		bSkinIsFree = skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free );
		bLicensed = m_currentPack->hasPurchasedFile( DLCManager::e_DLCType_Skin, m_selectedSkinPath );

		m_imagePadlock.SetShow( (bSkinIsFree || bLicensed) ?FALSE:TRUE );	
		m_previewControl->SetShow(TRUE);
		m_skinDetails.SetShow( TRUE );
	}
	else
	{	
		m_selectedSkinPath = L"";
		m_selectedCapePath = L"";
		m_vAdditionalSkinBoxes = nullptr;

		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:
			backupTexture = getTextureId(m_skinIndex);

			if( m_skinIndex ==  eDefaultSkins_ServerSelected )
			{
				skinName = app.GetString(IDS_DEFAULT_SKINS);
			}
			else
			{			
				skinName = wchDefaultNamesA[m_skinIndex];
			}

			if( m_originalSkinId == m_skinIndex )
			{
				m_selectedGroup.SetShow( TRUE );
			}
			else
			{
				m_selectedGroup.SetShow( FALSE );
			}
			m_imagePadlock.SetShow( FALSE );
			m_previewControl->SetShow(TRUE);
			m_skinDetails.SetShow( TRUE );

			break;
		case SKIN_SELECT_PACK_FAVORITES:

			if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
			{		
				// get the pack number from the skin id
				wchar_t chars[256];
				swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(m_iPad,m_skinIndex));

				Pack=app.m_dlcManager.getPackContainingSkin(chars);	
				if(Pack)
				{			
					skinFile = Pack->getSkinFile(chars);

					m_selectedSkinPath = skinFile->getPath();
					m_selectedCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
					m_vAdditionalSkinBoxes = skinFile->getAdditionalBoxes();

					skinName = skinFile->getParameterAsString( DLCManager::e_DLCParamType_DisplayName );
					skinOrigin = skinFile->getParameterAsString( DLCManager::e_DLCParamType_ThemeName );

					if( m_selectedSkinPath.compare( m_currentSkinPath ) == 0 )
					{
						m_selectedGroup.SetShow( TRUE );
					}
					else
					{
						m_selectedGroup.SetShow( FALSE );
					}

					bSkinIsFree = skinFile->getParameterAsBool( DLCManager::e_DLCParamType_Free );
					bLicensed = Pack->hasPurchasedFile( DLCManager::e_DLCType_Skin, m_selectedSkinPath );

					m_imagePadlock.SetShow( (bSkinIsFree || bLicensed) ?FALSE:TRUE );	
					m_skinDetails.SetShow( TRUE );
				}
				else
				{
					m_selectedGroup.SetShow( FALSE );
					m_imagePadlock.SetShow( FALSE );
				}
			}
			else
			{
				//disable the display
				m_previewControl->SetShow(FALSE);
				// change the tooltips
				bNoSkinsToShow=true;
			}
			break;

		}
	}
	m_text.SetText(skinName.c_str());
	m_originText.SetText(skinOrigin.c_str());

	if(m_vAdditionalSkinBoxes && m_vAdditionalSkinBoxes->size()!=0)
	{
		// add the boxes to the humanoid model, but only if we've not done this already
		vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
		if(pAdditionalModelParts==nullptr)
		{
			pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),m_vAdditionalSkinBoxes);
 		}
  	}

	if(skinFile!=nullptr)
	{
		app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
	}

	m_previewControl->SetTexture(m_selectedSkinPath, backupTexture);
	m_previewControl->SetCapeTexture(m_selectedCapePath);
	
	showNext = TRUE;		
	showPrevious = TRUE;
	nextIndex = getNextSkinIndex(m_skinIndex);
	previousIndex = getPreviousSkinIndex(m_skinIndex);

	wstring otherSkinPath = L"";
	wstring otherCapePath = L"";
	vector<SKIN_BOX *> *othervAdditionalSkinBoxes=nullptr;
	wchar_t chars[256];

	// turn off all displays
	for(BYTE i = 0; i < sidePreviewControls; ++i)
	{
		m_previewNextControls[i]->SetShow(FALSE);
		m_previewPreviousControls[i]->SetShow(FALSE);
	}	

	unsigned int uiCurrentFavoriteC=app.GetPlayerFavoriteSkinsCount(m_iPad);

	if(m_packIndex==SKIN_SELECT_PACK_FAVORITES)
	{
		// might not be enough to cycle through
		if(uiCurrentFavoriteC<((sidePreviewControls*2)+1))
		{
			if(uiCurrentFavoriteC==0)
			{
				sidePreviewControlsL=sidePreviewControlsR=0;
			}
			// might be an odd number
			else if((uiCurrentFavoriteC-1)%2==1)
			{
				sidePreviewControlsL=1+(uiCurrentFavoriteC-1)/2;
				sidePreviewControlsR=(uiCurrentFavoriteC-1)/2;
			}
			else
			{
				sidePreviewControlsL=sidePreviewControlsR=(uiCurrentFavoriteC-1)/2;
			}
		}
		else
		{
			sidePreviewControlsL=sidePreviewControlsR=sidePreviewControls;
		}
	}
	else
	{
		sidePreviewControlsL=sidePreviewControlsR=sidePreviewControls;
	}

	for(BYTE i = 0; i < sidePreviewControlsR; ++i)
	{
		if(showNext)
		{
			skinFile=nullptr;
			m_previewNextControls[i]->SetShow(TRUE);

			if( m_currentPack != nullptr )
			{
				skinFile = m_currentPack->getSkinFile(nextIndex);
				otherSkinPath = skinFile->getPath();
				otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
				othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
				backupTexture = TN_MOB_CHAR;
			}
			else
			{	
				otherSkinPath = L"";
				otherCapePath = L"";
				othervAdditionalSkinBoxes=nullptr;
				switch(m_packIndex)
				{
				case SKIN_SELECT_PACK_DEFAULT:
					backupTexture = getTextureId(nextIndex);
					break;
				case SKIN_SELECT_PACK_FAVORITES:
					if(uiCurrentFavoriteC>0)
					{				
						// get the pack number from the skin id
						swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(m_iPad,nextIndex));

						Pack=app.m_dlcManager.getPackContainingSkin(chars);	
						if(Pack)
						{				
							skinFile = Pack->getSkinFile(chars);

							otherSkinPath = skinFile->getPath();
							otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
							othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
							backupTexture = TN_MOB_CHAR;
						}
					}
					break;
				default:
					break;
				}

			}
  			if(othervAdditionalSkinBoxes && othervAdditionalSkinBoxes->size()!=0)
  			{
 				vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
 				if(pAdditionalModelParts==nullptr)
 				{
 					pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),othervAdditionalSkinBoxes);
 				}
  			}
			// 4J-PB - anim override needs set before SetTexture
			if(skinFile!=nullptr)
			{
				app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
			}			
			m_previewNextControls[i]->SetTexture(otherSkinPath, backupTexture);
			m_previewNextControls[i]->SetCapeTexture(otherCapePath);


		}
		
		nextIndex = getNextSkinIndex(nextIndex);
	}



	for(BYTE i = 0; i < sidePreviewControlsL; ++i)
	{
		if(showPrevious)
		{
			skinFile=nullptr;
			m_previewPreviousControls[i]->SetShow(TRUE);

			if( m_currentPack != nullptr )
			{
				skinFile = m_currentPack->getSkinFile(previousIndex);
				otherSkinPath = skinFile->getPath();
				otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
				othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
				backupTexture = TN_MOB_CHAR;
			}
			else
			{	
				otherSkinPath = L"";
				otherCapePath = L"";
				othervAdditionalSkinBoxes=nullptr;
				switch(m_packIndex)
				{
				case SKIN_SELECT_PACK_DEFAULT:
					backupTexture = getTextureId(previousIndex);
					break;
				case SKIN_SELECT_PACK_FAVORITES:
					if(uiCurrentFavoriteC>0)
					{	
						// get the pack number from the skin id
						swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(m_iPad,previousIndex));

						Pack=app.m_dlcManager.getPackContainingSkin(chars);	
						if(Pack)
						{
							skinFile = Pack->getSkinFile(chars);

							otherSkinPath = skinFile->getPath();
							otherCapePath = skinFile->getParameterAsString(DLCManager::e_DLCParamType_Cape);
							othervAdditionalSkinBoxes = skinFile->getAdditionalBoxes();
							backupTexture = TN_MOB_CHAR;
						}
					}

					break;
				default:
					break;
				}
			}
 			if(othervAdditionalSkinBoxes && othervAdditionalSkinBoxes->size()!=0)
 			{
				vector<ModelPart *> *pAdditionalModelParts = app.GetAdditionalModelParts(skinFile->getSkinID());
				if(pAdditionalModelParts==nullptr)
				{
					pAdditionalModelParts = app.SetAdditionalSkinBoxes(skinFile->getSkinID(),othervAdditionalSkinBoxes);
				}
  			}
			// 4J-PB - anim override needs set before SetTexture
			if(skinFile)
			{
				app.SetAnimOverrideBitmask(skinFile->getSkinID(),skinFile->getAnimOverrideBitmask());
			}			
			m_previewPreviousControls[i]->SetTexture(otherSkinPath, backupTexture);
			m_previewPreviousControls[i]->SetCapeTexture(otherCapePath);
		}
		
		previousIndex = getPreviousSkinIndex(previousIndex);
	}

	// update the tooltips
	bool bNotInGame=(Minecraft::GetInstance()->level==nullptr);

	if(bNoSkinsToShow)
	{
		if(bNotInGame)
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, -1,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
			CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
		}
		else
		{		
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT_SKIN,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
			CXuiSceneBase::ShowBackground( m_iPad, FALSE );
			CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );
		}

	}
	else
	{
		if(bNotInGame)
		{
			ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT_SKIN,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
			CXuiSceneBase::ShowBackground( DEFAULT_XUI_MENU_USER, TRUE );
		}
		else
		{		
			ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT_SKIN,IDS_TOOLTIPS_CANCEL,-1,-1,-1,-1,-1,-1,IDS_TOOLTIPS_NAVIGATE);
			CXuiSceneBase::ShowBackground( m_iPad, FALSE );
			CXuiSceneBase::ShowDarkOverlay( m_iPad, TRUE );
		}

	}

	updateCurrentFocus();
}

void CScene_SkinSelect::handlePackIndexChanged()
{
	if(m_packIndex >= SKIN_SELECT_MAX_DEFAULTS)
	{
		m_currentPack = app.m_dlcManager.getPack(m_packIndex - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);	
	}
	else
	{
		m_currentPack = nullptr;
	}
	m_skinIndex = 0;
	if(m_currentPack != nullptr)
	{
		bool found;
		DWORD currentSkinIndex = m_currentPack->getSkinIndexAt(m_currentSkinPath, found);
		if(found) m_skinIndex = currentSkinIndex;
	}
	else
	{
		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:
			if( !GET_IS_DLC_SKIN_FROM_BITMASK(m_originalSkinId) )
			{	
				DWORD ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(m_originalSkinId);
				DWORD defaultSkinIndex = GET_DEFAULT_SKIN_ID_FROM_BITMASK(m_originalSkinId);
				if( ugcSkinIndex == 0 )
				{
					m_skinIndex = static_cast<EDefaultSkins>(defaultSkinIndex);
				}
			}	
			break;
		case SKIN_SELECT_PACK_FAVORITES:
			if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
			{
				bool found;
				wchar_t chars[256];
				// get the pack number from the skin id
				swprintf(chars, 256, L"dlcskin%08d.png", app.GetPlayerFavoriteSkin(m_iPad,app.GetPlayerFavoriteSkinsPos(m_iPad)));

				DLCPack *Pack=app.m_dlcManager.getPackContainingSkin(chars);	
				if(Pack)
				{
					DWORD currentSkinIndex = Pack->getSkinIndexAt(m_currentSkinPath, found);
					if(found) m_skinIndex = app.GetPlayerFavoriteSkinsPos(m_iPad);
				}
			}
			break;
		default:
			break;
		}
	}
	handleSkinIndexChanged();
	updatePackDisplay();
}

void CScene_SkinSelect::updatePackDisplay()
{
	currentPackCount = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) + SKIN_SELECT_MAX_DEFAULTS;
	
	m_packLeft.SetShow(TRUE);
	m_packRight.SetShow(TRUE);
	
	if(m_packIndex >= SKIN_SELECT_MAX_DEFAULTS)
	{
		DLCPack *thisPack = app.m_dlcManager.getPack(m_packIndex - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);
		m_packCenter.SetText(thisPack->getName().c_str());
	}
	else
	{
		switch(m_packIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:				
			m_packCenter.SetText(app.GetString(IDS_NO_SKIN_PACK));
			break;
		case SKIN_SELECT_PACK_FAVORITES:				
			m_packCenter.SetText(app.GetString(IDS_FAVORITES_SKIN_PACK));
			break;
		}
	}

	int nextPackIndex = getNextPackIndex(m_packIndex);
	if(nextPackIndex >= SKIN_SELECT_MAX_DEFAULTS)
	{
		DLCPack *thisPack = app.m_dlcManager.getPack(nextPackIndex - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);
		m_packRight.SetText(thisPack->getName().c_str());
	}
	else
	{
		switch(nextPackIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:				
			m_packRight.SetText(app.GetString(IDS_NO_SKIN_PACK));
			break;
		case SKIN_SELECT_PACK_FAVORITES:				
			m_packRight.SetText(app.GetString(IDS_FAVORITES_SKIN_PACK));
			break;
		}
	}

	int previousPackIndex = getPreviousPackIndex(m_packIndex);
	if(previousPackIndex >= SKIN_SELECT_MAX_DEFAULTS)
	{
		DLCPack *thisPack = app.m_dlcManager.getPack(previousPackIndex - SKIN_SELECT_MAX_DEFAULTS, DLCManager::e_DLCType_Skin);
		m_packLeft.SetText(thisPack->getName().c_str());
	}
	else
	{
		switch(previousPackIndex)
		{
		case SKIN_SELECT_PACK_DEFAULT:				
			m_packLeft.SetText(app.GetString(IDS_NO_SKIN_PACK));
			break;
		case SKIN_SELECT_PACK_FAVORITES:				
			m_packLeft.SetText(app.GetString(IDS_FAVORITES_SKIN_PACK));
			break;
		}
	}
	
}

void CScene_SkinSelect::updateCurrentFocus()
{
	XUITimeline *timeline;
	XUINamedFrame *startFrame;
	XuiElementGetTimeline( m_skinDetails.m_hObj, &timeline);
	switch(m_currentNavigation)
	{
	case eSkinNavigation_Pack:
		XuiElementSetUserFocus( m_packCenter.m_hObj, m_iPad );
		startFrame = timeline->FindNamedFrame( L"Unselected" );
		m_normalTabs.SetShow( FALSE );
		m_selectedTabs.SetShow( TRUE );
		m_packLeft.SetEnable(TRUE);
		m_packRight.SetEnable(TRUE);
		break;
	case eSkinNavigation_Skin:
		XuiElementSetUserFocus( m_skinDetails.m_hObj, m_iPad );
		startFrame = timeline->FindNamedFrame( L"Selected" );
		m_normalTabs.SetShow( TRUE );
		m_selectedTabs.SetShow( FALSE );
		m_packLeft.SetEnable(FALSE);
		m_packRight.SetEnable(FALSE);
		break;
	};
	timeline->Play( startFrame->m_dwFrame, startFrame->m_dwFrame, startFrame->m_dwFrame, FALSE, FALSE );
}

TEXTURE_NAME CScene_SkinSelect::getTextureId(int skinIndex)
{
	TEXTURE_NAME texture = TN_MOB_CHAR;
	switch(skinIndex)
	{
	case eDefaultSkins_ServerSelected:
	case eDefaultSkins_Skin0:
		texture = TN_MOB_CHAR;
		break;
	case eDefaultSkins_Skin1:
		texture = TN_MOB_CHAR1;
		break;
	case eDefaultSkins_Skin2:
		texture = TN_MOB_CHAR2;
		break;
	case eDefaultSkins_Skin3:
		texture = TN_MOB_CHAR3;
		break;
	case eDefaultSkins_Skin4:
		texture = TN_MOB_CHAR4;
		break;
	case eDefaultSkins_Skin5:
		texture = TN_MOB_CHAR5;
		break;
	case eDefaultSkins_Skin6:
		texture = TN_MOB_CHAR6;
		break;
	case eDefaultSkins_Skin7:
		texture = TN_MOB_CHAR7;
		break;
	};

	return texture;
}

int CScene_SkinSelect::getNextSkinIndex(DWORD sourceIndex)
{
	int nextSkin = sourceIndex;

	// special case for favourites
	switch(m_packIndex)
	{

	case SKIN_SELECT_PACK_FAVORITES:
		++nextSkin;
		if(nextSkin>=app.GetPlayerFavoriteSkinsCount(m_iPad))
		{
			nextSkin=0;
		}
	
		break;
	default:
		++nextSkin;

		if(m_packIndex == SKIN_SELECT_PACK_DEFAULT && nextSkin >= eDefaultSkins_Count)
		{
			nextSkin = eDefaultSkins_ServerSelected;
		}
		else if(m_currentPack != nullptr && nextSkin>=m_currentPack->getSkinCount())
		{
			nextSkin = 0;
		}		
		break;
	}


	return nextSkin;
}

int CScene_SkinSelect::getPreviousSkinIndex(DWORD sourceIndex)
{
	int previousSkin = sourceIndex;
	switch(m_packIndex)
	{

	case SKIN_SELECT_PACK_FAVORITES:
		if(previousSkin==0)
		{
			previousSkin = app.GetPlayerFavoriteSkinsCount(m_iPad) - 1;
		}
		else
		{
			--previousSkin;
		}		
		break;
	default:
		if(previousSkin==0)
		{
			if(m_packIndex == SKIN_SELECT_PACK_DEFAULT)
			{
				previousSkin = eDefaultSkins_Count - 1;
			}
			else if(m_currentPack != nullptr)
			{
				previousSkin = m_currentPack->getSkinCount()-1;
			}
		}
		else
		{
			--previousSkin;
		}			
		break;
	}


	return previousSkin;
}

int CScene_SkinSelect::getNextPackIndex(DWORD sourceIndex)
{
	int nextPack = sourceIndex;
	++nextPack;
	if(nextPack > app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) - 1 + SKIN_SELECT_MAX_DEFAULTS)
	{
		nextPack = SKIN_SELECT_PACK_DEFAULT;
	}

	return nextPack;
}

int CScene_SkinSelect::getPreviousPackIndex(DWORD sourceIndex)
{
	int previousPack = sourceIndex;
	if(previousPack == SKIN_SELECT_PACK_DEFAULT)
	{
		if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) > 0)
		{
			previousPack = app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin) - 1 + SKIN_SELECT_MAX_DEFAULTS;
		}
		else
		{
			previousPack = SKIN_SELECT_MAX_DEFAULTS - 1;
		}
	}
	else
	{
		--previousPack;
	}

	return previousPack;
}

void CScene_SkinSelect::updateClipping()
{
	DWORD dwPropertyId;
	XUIElementPropVal propertyVal;
	propertyVal.Clear();
	HRESULT hr = XuiObjectGetPropertyId( m_charactersGroup.m_hObj, L"ClipChildren", &dwPropertyId);
	switch( CXuiSceneBase::GetPlayerBasePosition(m_iPad) )
	{
	case CXuiSceneBase::e_BaseScene_Left:
	case CXuiSceneBase::e_BaseScene_Right:
	case CXuiSceneBase::e_BaseScene_Top_Left:
	case CXuiSceneBase::e_BaseScene_Top_Right:
	case CXuiSceneBase::e_BaseScene_Bottom_Left:
	case CXuiSceneBase::e_BaseScene_Bottom_Right:	
	case CXuiSceneBase::e_BaseScene_Top:
	case CXuiSceneBase::e_BaseScene_Bottom:	
		propertyVal.SetBoolVal(TRUE);
		break;
	case CXuiSceneBase::e_BaseScene_Fullscreen:
	default:		
		propertyVal.SetBoolVal(FALSE);
		break;
	};
	hr = XuiObjectSetProperty(m_charactersGroup.m_hObj,dwPropertyId,0,&propertyVal);
}

int CScene_SkinSelect::UnlockSkinReturned(void *pParam,int iPad,C4JStorage::EMessageResult result)
{
	CScene_SkinSelect* pScene = static_cast<CScene_SkinSelect *>(pParam);
#ifdef _XBOX
	if(result==C4JStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{	
			ULONGLONG ullIndexA[1];
			DLC_INFO *pDLCInfo = app.GetDLCInfoForTrialOfferID(pScene->m_currentPack->getPurchaseOfferId());

			if(pDLCInfo!=nullptr)
			{
				ullIndexA[0]=pDLCInfo->ullOfferID_Full;
			}
			else
			{
				ullIndexA[0]=pScene->m_currentPack->getPurchaseOfferId();
			}

			// If we're in-game, then we need to enable DLC downloads. They'll be set back to Auto on leaving the pause menu
			if(Minecraft::GetInstance()->level!=nullptr)
			{
				// need to allow downloads here, or the player would need to quit the game to let the download of a skin pack happen. This might affect the network traffic, since the download could take all the bandwidth...
				XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_ALWAYS_ALLOW);
			}

			StorageManager.InstallOffer(1,ullIndexA,nullptr,nullptr);

			// the license change coming in when the offer has been installed will cause this scene to refresh	
		}
	}
	else
	{
		TelemetryManager->RecordUpsellResponded(iPad, eSet_UpsellID_Skin_DLC, ( pScene->m_currentPack->getPurchaseOfferId() & 0xFFFFFFFF ), eSen_UpsellOutcome_Declined);
	}
#endif
	return 0;
}

HRESULT CScene_SkinSelect::OnCustomMessage_DLCInstalled()
{
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		m_bIgnoreInput=false;
	}
	else
	{
		m_bIgnoreInput=true;
		m_timer.SetShow( TRUE );
		m_charactersGroup.SetShow( FALSE );
		m_skinDetails.SetShow( FALSE );
	}
	
	// this will send a CustomMessage_DLCMountingComplete when done
	return S_OK;
}

HRESULT CScene_SkinSelect::OnCustomMessage_DLCMountingComplete()
{	

	m_timer.SetShow( FALSE );
	m_charactersGroup.SetShow( TRUE );
	m_skinDetails.SetShow( TRUE );
	m_packIndex = SKIN_SELECT_PACK_DEFAULT;
	
	if(app.m_dlcManager.getPackCount(DLCManager::e_DLCType_Skin)>0)
	{
		m_currentPack = app.m_dlcManager.getPackContainingSkin(m_currentSkinPath);
		if(m_currentPack != nullptr)
		{
			bool bFound = false;
			m_packIndex = app.m_dlcManager.getPackIndex(m_currentPack,bFound,DLCManager::e_DLCType_Skin) + SKIN_SELECT_MAX_DEFAULTS;
		}
	}

	// If we have any favourites, set this to the favourites
	// first validate the favorite skins - we might have uninstalled the DLC needed for them
	app.ValidateFavoriteSkins(m_iPad);

	if(app.GetPlayerFavoriteSkinsCount(m_iPad)>0)
	{
		m_packIndex = SKIN_SELECT_PACK_FAVORITES;
	}

	handlePackIndexChanged();
	updateCurrentFocus();
	m_bIgnoreInput=false;
	app.m_dlcManager.checkForCorruptDLCAndAlert();
	bool bInGame=(Minecraft::GetInstance()->level!=nullptr);

	if(bInGame) XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);

	return S_OK;
}

void CScene_SkinSelect::AddFavoriteSkin(int iPad,int iSkinID)
{
	// Is this favorite skin already in the array?
	unsigned int uiCurrentFavoriteSkinsCount=app.GetPlayerFavoriteSkinsCount(m_iPad);

	for(int i=0;i<uiCurrentFavoriteSkinsCount;i++)
	{
		if(app.GetPlayerFavoriteSkin(m_iPad,i)==iSkinID)
		{
			app.SetPlayerFavoriteSkinsPos(m_iPad,i);
			return;
		}
	}

	unsigned char ucPos=app.GetPlayerFavoriteSkinsPos(m_iPad);
	if(ucPos==(MAX_FAVORITE_SKINS-1))
	{
		ucPos=0;
	}
	else
	{
		if(uiCurrentFavoriteSkinsCount>0)
		{
			ucPos++;
		}
		else
		{
			ucPos=0;
		}
	}

	app.SetPlayerFavoriteSkin(iPad,(int)ucPos,iSkinID);
	app.SetPlayerFavoriteSkinsPos(m_iPad,ucPos);
	
}