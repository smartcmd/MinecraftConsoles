#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include "XUI_Leaderboards.h"
#include "..\..\..\Minecraft.World\Tile.h"
#include "..\..\..\Minecraft.World\Item.h"
#include "XUI_Ctrl_CraftIngredientSlot.h"
#include "XUI_XZP_Icons.h"

LPCWSTR CScene_Leaderboards::m_TitleIconNameA[7]=
{
	L"XuiHSlot1",
	L"XuiHSlot2",
	L"XuiHSlot3",
	L"XuiHSlot4",
	L"XuiHSlot5",
	L"XuiHSlot6",
	L"XuiHSlot7",
};

LPCWSTR CScene_Leaderboards::m_TextColumnNameA[7]=
{
	L"text_Column1",
	L"text_Column2",
	L"text_Column3",
	L"text_Column4",
	L"text_Column5",
	L"text_Column6",
	L"text_Column7",
};


// if the value is greater than 511, it's an xzp icon that needs displayed, rather than the game icon
const int CScene_Leaderboards::TitleIcons[CScene_Leaderboards::NUM_LEADERBOARDS][7] = 
{
	{ XZP_ICON_WALKED,		XZP_ICON_FALLEN,		Item::minecart_Id,		Item::boat_Id,					NULL },
	{ Tile::dirt_Id,		Tile::stoneBrick_Id,	Tile::sand_Id,			Tile::rock_Id,			Tile::gravel_Id,		Tile::clay_Id,			Tile::obsidian_Id },
	{ Item::egg_Id,			Item::wheat_Id,			Tile::mushroom1_Id,		Tile::reeds_Id,			Item::milk_Id,	Tile::pumpkin_Id,		NULL },
	{ XZP_ICON_ZOMBIE,		XZP_ICON_SKELETON,		XZP_ICON_CREEPER,		XZP_ICON_SPIDER,		XZP_ICON_SPIDERJOCKEY,	XZP_ICON_ZOMBIEPIGMAN,	XZP_ICON_SLIME },
};

const int CScene_Leaderboards::LEADERBOARD_HEADERS[CScene_Leaderboards::NUM_LEADERBOARDS][4] = {
	{ SPASTRING_LB_TRAVELLING_PEACEFUL_NAME,		SPASTRING_LB_TRAVELLING_EASY_NAME,		SPASTRING_LB_TRAVELLING_NORMAL_NAME,		SPASTRING_LB_TRAVELLING_HARD_NAME },
	{ SPASTRING_LB_MINING_BLOCKS_PEACEFUL_NAME,		SPASTRING_LB_MINING_BLOCKS_EASY_NAME,	SPASTRING_LB_MINING_BLOCKS_NORMAL_NAME,		SPASTRING_LB_MINING_BLOCKS_HARD_NAME },
	{ SPASTRING_LB_FARMING_PEACEFUL_NAME,			SPASTRING_LB_FARMING_EASY_NAME,			SPASTRING_LB_FARMING_NORMAL_NAME,			SPASTRING_LB_FARMING_HARD_NAME },
	{ NULL,											SPASTRING_LB_KILLS_EASY_NAME,			SPASTRING_LB_KILLS_NORMAL_NAME,				SPASTRING_LB_KILLS_HARD_NAME },
};

const CScene_Leaderboards::LeaderboardDescriptor CScene_Leaderboards::LEADERBOARD_DESCRIPTORS[CScene_Leaderboards::NUM_LEADERBOARDS][4] = {
	{
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_PEACEFUL, 4,	STATS_COLUMN_TRAVELLING_PEACEFUL_WALKED,	STATS_COLUMN_TRAVELLING_PEACEFUL_FALLEN,	STATS_COLUMN_TRAVELLING_PEACEFUL_MINECART,	STATS_COLUMN_TRAVELLING_PEACEFUL_BOAT,	NULL, NULL, NULL,NULL),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_EASY,		4,	STATS_COLUMN_TRAVELLING_EASY_WALKED,		STATS_COLUMN_TRAVELLING_EASY_FALLEN,		STATS_COLUMN_TRAVELLING_EASY_MINECART,		STATS_COLUMN_TRAVELLING_EASY_BOAT,		NULL, NULL, NULL,NULL),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_NORMAL,	4,	STATS_COLUMN_TRAVELLING_NORMAL_WALKED,		STATS_COLUMN_TRAVELLING_NORMAL_FALLEN,		STATS_COLUMN_TRAVELLING_NORMAL_MINECART,	STATS_COLUMN_TRAVELLING_NORMAL_BOAT,	NULL, NULL, NULL,NULL),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_TRAVELLING_HARD,		4,	STATS_COLUMN_TRAVELLING_HARD_WALKED,		STATS_COLUMN_TRAVELLING_HARD_FALLEN,		STATS_COLUMN_TRAVELLING_HARD_MINECART,		STATS_COLUMN_TRAVELLING_HARD_BOAT,		NULL, NULL, NULL,NULL),
	},
	{
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_PEACEFUL,	7,	STATS_COLUMN_MINING_BLOCKS_PEACEFUL_DIRT,  STATS_COLUMN_MINING_BLOCKS_PEACEFUL_STONE, STATS_COLUMN_MINING_BLOCKS_PEACEFUL_SAND, STATS_COLUMN_MINING_BLOCKS_PEACEFUL_COBBLESTONE, STATS_COLUMN_MINING_BLOCKS_PEACEFUL_GRAVEL, STATS_COLUMN_MINING_BLOCKS_PEACEFUL_CLAY, STATS_COLUMN_MINING_BLOCKS_PEACEFUL_OBSIDIAN,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_EASY,		7,	STATS_COLUMN_MINING_BLOCKS_EASY_DIRT,  STATS_COLUMN_MINING_BLOCKS_EASY_STONE, STATS_COLUMN_MINING_BLOCKS_EASY_SAND, STATS_COLUMN_MINING_BLOCKS_EASY_COBBLESTONE, STATS_COLUMN_MINING_BLOCKS_EASY_GRAVEL, STATS_COLUMN_MINING_BLOCKS_EASY_CLAY, STATS_COLUMN_MINING_BLOCKS_EASY_OBSIDIAN,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_NORMAL,	7,	STATS_COLUMN_MINING_BLOCKS_NORMAL_DIRT,  STATS_COLUMN_MINING_BLOCKS_NORMAL_STONE, STATS_COLUMN_MINING_BLOCKS_NORMAL_SAND, STATS_COLUMN_MINING_BLOCKS_NORMAL_COBBLESTONE, STATS_COLUMN_MINING_BLOCKS_NORMAL_GRAVEL, STATS_COLUMN_MINING_BLOCKS_NORMAL_CLAY, STATS_COLUMN_MINING_BLOCKS_NORMAL_OBSIDIAN,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_MINING_BLOCKS_HARD,		7,	STATS_COLUMN_MINING_BLOCKS_HARD_DIRT,  STATS_COLUMN_MINING_BLOCKS_HARD_STONE, STATS_COLUMN_MINING_BLOCKS_HARD_SAND, STATS_COLUMN_MINING_BLOCKS_HARD_COBBLESTONE, STATS_COLUMN_MINING_BLOCKS_HARD_GRAVEL, STATS_COLUMN_MINING_BLOCKS_HARD_CLAY, STATS_COLUMN_MINING_BLOCKS_HARD_OBSIDIAN,NULL ),
	},
	{
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_FARMING_PEACEFUL,	6, STATS_COLUMN_FARMING_PEACEFUL_EGGS, STATS_COLUMN_FARMING_PEACEFUL_WHEAT, STATS_COLUMN_FARMING_PEACEFUL_MUSHROOMS, STATS_COLUMN_FARMING_PEACEFUL_SUGARCANE, STATS_COLUMN_FARMING_PEACEFUL_MILK, STATS_COLUMN_FARMING_PEACEFUL_PUMPKINS, NULL,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_FARMING_EASY,		6, STATS_COLUMN_FARMING_EASY_EGGS, STATS_COLUMN_FARMING_PEACEFUL_WHEAT, STATS_COLUMN_FARMING_EASY_MUSHROOMS, STATS_COLUMN_FARMING_EASY_SUGARCANE, STATS_COLUMN_FARMING_EASY_MILK, STATS_COLUMN_FARMING_EASY_PUMPKINS, NULL,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_FARMING_NORMAL,		6, STATS_COLUMN_FARMING_NORMAL_EGGS, STATS_COLUMN_FARMING_NORMAL_WHEAT, STATS_COLUMN_FARMING_NORMAL_MUSHROOMS, STATS_COLUMN_FARMING_NORMAL_SUGARCANE, STATS_COLUMN_FARMING_NORMAL_MILK, STATS_COLUMN_FARMING_NORMAL_PUMPKINS, NULL,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_FARMING_HARD,		6, STATS_COLUMN_FARMING_HARD_EGGS, STATS_COLUMN_FARMING_HARD_WHEAT, STATS_COLUMN_FARMING_HARD_MUSHROOMS, STATS_COLUMN_FARMING_HARD_SUGARCANE, STATS_COLUMN_FARMING_HARD_MILK, STATS_COLUMN_FARMING_HARD_PUMPKINS, NULL,NULL ),
	},
	{
		CScene_Leaderboards::LeaderboardDescriptor( NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_KILLS_EASY,		7, STATS_COLUMN_KILLS_EASY_ZOMBIES, STATS_COLUMN_KILLS_EASY_SKELETONS, STATS_COLUMN_KILLS_EASY_CREEPERS, STATS_COLUMN_KILLS_EASY_SPIDERS, STATS_COLUMN_KILLS_EASY_SPIDERJOCKEYS, STATS_COLUMN_KILLS_EASY_ZOMBIEPIGMEN, STATS_COLUMN_KILLS_EASY_SLIME,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_KILLS_NORMAL,	7, STATS_COLUMN_KILLS_NORMAL_ZOMBIES, STATS_COLUMN_KILLS_NORMAL_SKELETONS, STATS_COLUMN_KILLS_NORMAL_CREEPERS, STATS_COLUMN_KILLS_NORMAL_SPIDERS, STATS_COLUMN_KILLS_NORMAL_SPIDERJOCKEYS, STATS_COLUMN_KILLS_NORMAL_ZOMBIEPIGMEN, STATS_COLUMN_KILLS_NORMAL_SLIME,NULL ),
		CScene_Leaderboards::LeaderboardDescriptor( STATS_VIEW_KILLS_HARD,		7, STATS_COLUMN_KILLS_HARD_ZOMBIES, STATS_COLUMN_KILLS_HARD_SKELETONS, STATS_COLUMN_KILLS_HARD_CREEPERS, STATS_COLUMN_KILLS_HARD_SPIDERS, STATS_COLUMN_KILLS_HARD_SPIDERJOCKEYS, STATS_COLUMN_KILLS_HARD_ZOMBIEPIGMEN, STATS_COLUMN_KILLS_HARD_SLIME,NULL ),
	},
};

HRESULT CScene_Leaderboards::OnInit(XUIMessageInit *pInitData, BOOL &bHandled)
{
	m_iPad = *(int *)pInitData->pvInitData;
	MapChildControls();
	m_bReady=false;

	// if we're not in the game, we need to use basescene 0 
	if(Minecraft::GetInstance()->level==NULL)
	{
		m_iPad=DEFAULT_XUI_MENU_USER;
	}

	m_bPopulatedOnce = false;

	ui.SetTooltips(m_iPad,-1, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGE_FILTER, -1);
	CXuiSceneBase::ShowLogo( m_iPad, FALSE );

	m_friends = NULL;
	m_numFriends = 0;
	m_filteredFriends = NULL;
	m_numFilteredFriends = 0;

	m_newTop = m_newSel = -1;

	m_isProcessingStatsRead = false;

	// Alert the app the we want to be informed of ethernet connections
	app.SetLiveLinkRequired( true );

	LeaderboardManager::Instance()->OpenSession();

	//GetFriends();

	m_currentLeaderboard = 0;
	m_currentDifficulty = 2;
	SetLeaderboardHeader();

	m_currentFilter = LeaderboardManager::eFM_Friends;
	wchar_t filterBuffer[40];
	swprintf(filterBuffer, 40, L"%ls%ls", app.GetString(IDS_LEADERBOARD_FILTER), app.GetString(IDS_LEADERBOARD_FILTER_FRIENDS));
	m_textFilter.SetText(filterBuffer);

	wchar_t entriesBuffer[40];
	swprintf(entriesBuffer, 40, L"%ls%i", app.GetString(IDS_LEADERBOARD_ENTRIES), 0);
	m_textEntries.SetText(entriesBuffer);

	ReadStats(-1);

	// title icons
	for(int i=0;i<7;i++)
	{
		m_pHTitleIconSlots[i]=NULL;
		m_fTitleIconXPositions[i]=0.0f;
		m_fTextXPositions[i]=0.0f;
		m_hTextEntryA[i]=NULL;
	}


	bHandled = TRUE;
	return S_OK;
}


void CScene_Leaderboards::Reposition(int iNumber)
{
	float fIconSize; // including gap
	float fNewIconIncrement;
	D3DXVECTOR3 vPos;

	fIconSize=(m_fTitleIconXPositions[6]-m_fTitleIconXPositions[0])/6.0f;
	fNewIconIncrement=(fIconSize*7.0f)/(float)iNumber;

	// reposition the title icons based on the number there are
	for(int i=0;i<iNumber;i++)
	{
		m_pHTitleIconSlots[i]->GetPosition(&vPos);
		vPos.x=m_fTitleIconXPositions[0]+(((float)i)*fNewIconIncrement)+(fNewIconIncrement-fIconSize)/2.0f;
		m_pHTitleIconSlots[i]->SetPosition(&vPos);
	}
}

void CScene_Leaderboards::RepositionText(int iNumber)
{
	float fTextSize; // including gap
	float fNewTextIncrement;
	D3DXVECTOR3 vPos;

	fTextSize=(m_fTextXPositions[6]-m_fTextXPositions[0])/6.0f;
	fNewTextIncrement=(fTextSize*7.0f)/(float)iNumber;

	// reposition the title icons based on the number there are
	for(int i=0;i<iNumber;i++)
	{
		// and reposition the text
		XuiElementGetPosition(m_hTextEntryA[i],&vPos);
		vPos.x=m_fTextXPositions[0]+(((float)i)*fNewTextIncrement);
		XuiElementSetPosition(m_hTextEntryA[i],&vPos);
		// and change the size
		float fWidth,fHeight;
		XuiElementGetBounds(m_hTextEntryA[i],&fWidth,&fHeight);
		XuiElementSetBounds(m_hTextEntryA[i],fNewTextIncrement,fHeight);
	}
}

HRESULT CScene_Leaderboards::OnDestroy()
{
	LeaderboardManager::Instance()->CancelOperation();
	LeaderboardManager::Instance()->CloseSession();

	// Alert the app the we no longer want to be informed of ethernet connections
	app.SetLiveLinkRequired( false );

	while( m_isProcessingStatsRead )
	{
		Sleep( 10 );
	}

	if( m_friends != NULL )
		delete [] m_friends;

	if( m_filteredFriends != NULL )
		delete [] m_filteredFriends;

	return S_OK;
}

HRESULT CScene_Leaderboards::OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	UpdateTooltips();
	return S_OK;
}

HRESULT CScene_Leaderboards::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{

	if(m_bReady && pNotifySelChangedData->iOldItem!=-1)
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}

	return S_OK;
}

void CScene_Leaderboards::UpdateTooltips()
{
	int iTooltipFriendRequest=-1;
	int iTooltipGamerCardOrProfile=-1;
	if( m_leaderboard.m_currentEntryCount > 0 )
	{
		unsigned int selection = (unsigned int)m_listGamers.GetCurSel();

		// if the selected user is me, don't show Send Friend Request, and show the gamer profile, not the gamer card

		// Check that the index is actually within range of the data we've got before accessing the m_leaderboard.m_entries array
		int idx = selection - (m_leaderboard.m_entryStartIndex-1);
		if( ( idx < 0 ) || ( idx >= NUM_ENTRIES ) )
		{
			return;
		}
		if(m_leaderboard.m_entries[idx].m_bPlayer)
		{
			iTooltipGamerCardOrProfile=IDS_TOOLTIPS_VIEW_GAMERPROFILE;
		}
		else
		{
			iTooltipGamerCardOrProfile=IDS_TOOLTIPS_VIEW_GAMERCARD;
			// if we're on the friends filter, then don't show the Send Friend Request
			bool bIsFriend = m_currentFilter == LeaderboardManager::eFM_Friends;

			if(!bIsFriend)
			{
				// check the entry we're on
				if( m_leaderboard.m_currentEntryCount > 0 )
				{
					if( selection >= m_leaderboard.m_entryStartIndex-1 &&
						selection < (m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount-1) )
					{
						if( (m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1)].m_bFriend==false) &&  (m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1)].m_bRequestedFriend==false))
						{
							iTooltipFriendRequest=IDS_TOOLTIPS_SEND_FRIEND_REQUEST;
						}
					}
				}
			}
		}
	}

	// 4J-PB - no room on the screen for the LT/RT prompt
	/*
	if(m_leaderboard.m_currentEntryCount>11)
	{	
		ui.SetTooltips(m_iPad, iTooltipFriendRequest, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGE_FILTER, iTooltipGamerCardOrProfile, IDS_TOOLTIPS_PAGEUP, IDS_TOOLTIPS_PAGEDOWN);	
	}
	else*/
	{
		ui.SetTooltips(m_iPad, iTooltipFriendRequest, IDS_TOOLTIPS_BACK, IDS_TOOLTIPS_CHANGE_FILTER, iTooltipGamerCardOrProfile);	
	}
}

HRESULT CScene_Leaderboards::OnKeyDown(XUIMessageInput* pInputData, BOOL& bHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_RSHOULDER:
	case VK_PAD_LSHOULDER:
		{
			//Do nothing if a stats read is currently in progress, otherwise the system complains about to many read requests
			if(m_bPopulatedOnce && LeaderboardManager::Instance()->isIdle() )
			{
				if( pInputData->dwKeyCode == VK_PAD_RSHOULDER )
				{
					++m_currentDifficulty;
					if( m_currentDifficulty == 4 )
						m_currentDifficulty = 0;

					if( m_currentLeaderboard == LEADERBOARD_KILLS_POSITION && m_currentDifficulty == 0 )
						m_currentDifficulty = 1;
				}
				else
				{
					if( m_currentDifficulty == 0 )
						m_currentDifficulty = 4;
					--m_currentDifficulty;

					if( m_currentLeaderboard == LEADERBOARD_KILLS_POSITION && m_currentDifficulty == 0 )
						m_currentDifficulty = 3;
				}
		
				SetLeaderboardHeader();
				ClearLeaderboardTitlebar();

				ReadStats(-1);
				CXuiSceneBase::PlayUISFX(eSFX_Press);
			}

			bHandled = TRUE;
		}
		break;
	case VK_PAD_LTHUMB_RIGHT:
	case VK_PAD_LTHUMB_LEFT:
	case VK_PAD_DPAD_LEFT:
	case VK_PAD_DPAD_RIGHT:
		{
			//Do nothing if a stats read is currently in progress, otherwise the system complains about to many read requests
			if ( m_bPopulatedOnce && LeaderboardManager::Instance()->isIdle() )
			{
				m_bReady=false;
				if(( pInputData->dwKeyCode == VK_PAD_LTHUMB_RIGHT ) ||(pInputData->dwKeyCode == VK_PAD_DPAD_RIGHT))
				{
					++m_currentLeaderboard;
					if( m_currentLeaderboard == NUM_LEADERBOARDS )
						m_currentLeaderboard = 0;
				}
				else
				{
					if( m_currentLeaderboard == 0 )
						m_currentLeaderboard = NUM_LEADERBOARDS;
					--m_currentLeaderboard;
				}

				if( m_currentLeaderboard == LEADERBOARD_KILLS_POSITION && m_currentDifficulty == 0 )
					m_currentDifficulty = 1;
			
				SetLeaderboardHeader();
				ClearLeaderboardTitlebar();


				ReadStats(-1);	
				CXuiSceneBase::PlayUISFX(eSFX_Press);
			}
			bHandled = TRUE;
		}
		break;
	case VK_PAD_LTRIGGER:
	case VK_PAD_RTRIGGER:
		{
			//Do nothing if a stats read is currently in progress, otherwise the system complains about to many read requests
			if( m_bPopulatedOnce && LeaderboardManager::Instance()->isIdle() )
			{
				if( m_leaderboard.m_totalEntryCount <= 10 )
					break;

				if( pInputData->dwKeyCode == VK_PAD_LTRIGGER )
				{
					m_newTop = m_listGamers.GetTopItem() - 10;
					if( m_newTop < 0 )
						m_newTop = 0;

					m_newSel = m_newTop;
				}
				else
				{
					m_newTop = m_listGamers.GetTopItem() + 10;
					if( m_newTop+10 > (int)m_leaderboard.m_totalEntryCount )
					{
						m_newTop = m_leaderboard.m_totalEntryCount - 10;
						if( m_newTop < 0 )
							m_newTop = 0;
					}

					m_newSel = m_newTop;
				}
				//CXuiSceneBase::PlayUISFX(eSFX_Press);
			}
			bHandled = TRUE;
		}
		break;
	case VK_PAD_X:
		{
			//Do nothing if a stats read is currently in progress, otherwise the system complains about to many read requests
			if( m_bPopulatedOnce && LeaderboardManager::Instance()->isIdle() )
			{
				switch( m_currentFilter )
				{
				case LeaderboardManager::eFM_Friends:
					{
						m_currentFilter = LeaderboardManager::eFM_MyScore;
						wchar_t filterBuffer[40];
						swprintf_s(filterBuffer, 40, L"%ls%ls", app.GetString(IDS_LEADERBOARD_FILTER), app.GetString(IDS_LEADERBOARD_FILTER_MYSCORE));
						m_textFilter.SetText(filterBuffer);
					}
					break;
				case LeaderboardManager::eFM_MyScore:
					{
						m_currentFilter = LeaderboardManager::eFM_TopRank;
						wchar_t filterBuffer[40];
						swprintf_s(filterBuffer, 40, L"%ls%ls", app.GetString(IDS_LEADERBOARD_FILTER), app.GetString(IDS_LEADERBOARD_FILTER_OVERALL));
						m_textFilter.SetText(filterBuffer);
					}
					break;
				case LeaderboardManager::eFM_TopRank:
					{
						m_currentFilter = LeaderboardManager::eFM_Friends;
						wchar_t filterBuffer[40];
						swprintf_s(filterBuffer, 40, L"%ls%ls", app.GetString(IDS_LEADERBOARD_FILTER), app.GetString(IDS_LEADERBOARD_FILTER_FRIENDS));
						m_textFilter.SetText(filterBuffer);
					}
					break;
				}

				ReadStats(-1);
				CXuiSceneBase::PlayUISFX(eSFX_Press);
			}
			bHandled = TRUE;
		}
		break;
	case VK_PAD_Y:
		{
			//Show gamercard
			if( m_leaderboard.m_currentEntryCount > 0 )
			{
				unsigned int selection = (unsigned int)m_listGamers.GetCurSel();
				if( selection >= m_leaderboard.m_entryStartIndex-1 &&
					selection < (m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount-1) )
				{
					PlayerUID xuid = m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1)].m_xuid;
					if( xuid != INVALID_XUID )
					{
						XShowGamerCardUI(ProfileManager.GetLockedProfile(), xuid);
						CXuiSceneBase::PlayUISFX(eSFX_Press);
					}
				}
			}
			bHandled = TRUE;
		}
		break;
	case VK_PAD_A:
		{
			//Send friend request if the filter mode is not friend, and they're not a friend or a pending friend
			if( m_currentFilter != LeaderboardManager::eFM_Friends )
			{
				if( m_leaderboard.m_currentEntryCount > 0 )
				{
					unsigned int selection = (unsigned int)m_listGamers.GetCurSel();
					if( selection >= m_leaderboard.m_entryStartIndex-1 &&
						selection < (m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount-1) )
					{
						//If not the player and neither currently a friend or requested to be a friend
						if( !m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1) ].m_bPlayer && 
							!m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1) ].m_bFriend && 
							!m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1) ].m_bRequestedFriend )
						{
							PlayerUID xuid = m_leaderboard.m_entries[selection - (m_leaderboard.m_entryStartIndex-1) ].m_xuid;
							if( xuid != INVALID_XUID )
							{
								XShowFriendRequestUI(ProfileManager.GetLockedProfile(), xuid);
								CXuiSceneBase::PlayUISFX(eSFX_Press);
							}
						}
					}
				}
			}
			bHandled = TRUE;
		}
		break;
	case VK_PAD_B:
	case VK_ESCAPE:
		{
			BYTE userIndex = pInputData->UserIndex;
			if( !app.IsPauseMenuDisplayed(userIndex) )
			{
				// If we are not from a pause menu, then we are from the main menu
				userIndex = XUSER_INDEX_ANY;
			}

			app.NavigateBack(userIndex);
			bHandled = TRUE;
		}
		break;
	}
	return S_OK;
}

// DELETING //
#if 0 
void CScene_Leaderboards::GetFriends()
{
	DWORD resultsSize;
	HANDLE hEnumerator;
	DWORD ret;

	m_numFriends = 0;

	//First, get a list of (up to 100) friends (this is the maximum that the enumerator currently supports)
	ret = XFriendsCreateEnumerator( ProfileManager.GetLockedProfile(), 0, 100, &resultsSize, &hEnumerator);
	if( ret != ERROR_SUCCESS ) 
		return;

	m_friends = (XONLINE_FRIEND*) new BYTE[ resultsSize ];
	DWORD numFriends;

	ret = XEnumerate(
		hEnumerator,
		m_friends,
		resultsSize,
		&numFriends,
		NULL );

	if( ret != ERROR_SUCCESS )
		numFriends = 0;

	m_numFriends = numFriends;

	m_filteredFriends = new PlayerUID[m_numFriends+1];

	m_numFilteredFriends = 0;
	for( unsigned int friendIndex=0 ; friendIndex<numFriends ; ++friendIndex )
	{
		if( ( m_friends[friendIndex].dwFriendState & ( XONLINE_FRIENDSTATE_FLAG_SENTREQUEST | XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST ) ) == 0 )
		{
			m_filteredFriends[m_numFilteredFriends++] = m_friends[friendIndex].xuid;
		}
	}
	m_filteredFriends[m_numFilteredFriends++] = LeaderboardManager::Instance()->GetMyXUID();
}
#endif

void CScene_Leaderboards::ReadStats(int startIndex)
{
	//If startIndex == -1, then use default values
	if( startIndex == -1 )
	{
		m_newEntryIndex = 1;
		m_newReadSize	= READ_SIZE;

		m_leaderboard.m_totalEntryCount		= 0;
		m_leaderboard.m_currentEntryCount	= 0;
		
		m_listGamers.DeleteItems(0, m_listGamers.GetItemCount());
	}
	else
	{
		m_newEntryIndex = (unsigned int)startIndex;
		m_newReadSize	= min((int)READ_SIZE, (int)m_leaderboard.m_totalEntryCount-(startIndex-1));
	}

	//Setup the spec structure for the read request
	/* XUSER_STATS_SPEC* spec = new XUSER_STATS_SPEC[1]; // 4j-jev, moved into xboxLeaderboardManager
	spec[0].dwViewId = LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_viewId;
	spec[0].dwNumColumnIds = LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_columnCount;
	for( unsigned int i=0 ; i<spec[0].dwNumColumnIds ; ++i )
		spec[0].rgwColumnIds[i] = LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_columnIds[i]; */
	
	/*//This call takes ownership of spec and will free it when done
	LeaderboardManager::Instance()->ReadStats(
		m_newEntryIndex,
		m_newReadSize,
		1,
		spec,
		(startIndex == -1) ? m_currentFilter : LeaderboardManager::eFM_TopRank,
		CScene_Leaderboards::OnStatsReadComplete,
		reinterpret_cast<void*>(this),
		m_numFilteredFriends,
		m_filteredFriends);*/

	switch (startIndex == -1 ? m_currentFilter : LeaderboardManager::eFM_TopRank)
	{
	case LeaderboardManager::eFM_TopRank:
		LeaderboardManager::Instance()->ReadStats_TopRank(	this, 
															m_currentDifficulty, (LeaderboardManager::EStatsType) m_currentLeaderboard, 
															m_newEntryIndex, m_newReadSize
														 );
		break;
	case LeaderboardManager::eFM_MyScore:
		LeaderboardManager::Instance()->ReadStats_MyScore(	this,
															m_currentDifficulty, (LeaderboardManager::EStatsType) m_currentLeaderboard,
															INVALID_XUID/*ignored*/, 
															m_newReadSize
														 );
		break;
	case LeaderboardManager::eFM_Friends:
		LeaderboardManager::Instance()->ReadStats_Friends(	this,
															m_currentDifficulty, (LeaderboardManager::EStatsType) m_currentLeaderboard,
															INVALID_XUID /*ignored*/,
															0 /*ignored*/, 0 /*ignored*/
														 );
		break;
	}

	//Show the loading message
	m_textInfo.SetText(app.GetString(IDS_LEADERBOARD_LOADING));
	m_textInfo.SetShow(true);
}

bool CScene_Leaderboards::OnStatsReadComplete(bool success, int numResults, LeaderboardManager::ViewOut results)
{
	//CScene_Leaderboards* scene = reinterpret_cast<CScene_Leaderboards*>(userdata);

	m_isProcessingStatsRead = true;

	//bool noResults = LeaderboardManager::Instance()->GetStatsState() != XboxLeaderboardManager::eStatsState_Ready;
	bool ret;

	if (success)
	{
		m_numStats = numResults;
		m_stats = results;
		ret = RetrieveStats();		
	}
	else ret = true;
	
	//else LeaderboardManager::Instance()->SetStatsRetrieved(false);
	
	PopulateLeaderboard(!success);

	m_isProcessingStatsRead = false;

	return ret;
}


bool CScene_Leaderboards::RetrieveStats()
{

	if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
	{
		m_leaderboard.m_totalEntryCount		= NUM_ENTRIES;
		m_leaderboard.m_currentEntryCount	= NUM_ENTRIES;
		m_leaderboard.m_entryStartIndex		= 1;
		m_leaderboard.m_numColumns			= LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_columnCount;
	
		//For each entry in the leaderboard
		for( unsigned int entryIndex=0 ; entryIndex<m_leaderboard.m_currentEntryCount ; entryIndex++ )
		{
			m_leaderboard.m_entries[entryIndex].m_xuid = INVALID_XUID;

			m_leaderboard.m_entries[entryIndex].m_rank = entryIndex+1;
			swprintf(m_leaderboard.m_entries[entryIndex].m_wcRank, 12, L"12345678");//(int)m_leaderboard.m_entries[entryIndex].m_rank);
					
			swprintf(m_leaderboard.m_entries[entryIndex].m_gamerTag, 17, L"WWWWWWWWWWWWWWWW");

			//m_leaderboard.m_entries[entryIndex].m_locale = (entryIndex % 37) + 1;

			bool isDistanceLeaderboard = LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_viewId == STATS_VIEW_TRAVELLING_PEACEFUL || LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_viewId == STATS_VIEW_TRAVELLING_EASY || LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_viewId == STATS_VIEW_TRAVELLING_NORMAL || LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_viewId == STATS_VIEW_TRAVELLING_HARD;

			for( unsigned int i=0 ; i<m_leaderboard.m_numColumns ; i++ )
			{
				if( !isDistanceLeaderboard )
				{
					m_leaderboard.m_entries[entryIndex].m_columns[i] = USHRT_MAX;
					swprintf(m_leaderboard.m_entries[entryIndex].m_wcColumns[i], 12, L"%u", m_leaderboard.m_entries[entryIndex].m_columns[i]);
				}
				else
				{
					m_leaderboard.m_entries[entryIndex].m_columns[i] = UINT_MAX;
					swprintf(m_leaderboard.m_entries[entryIndex].m_wcColumns[i], 12, L"%.1fkm", ((float)m_leaderboard.m_entries[entryIndex].m_columns[i])/100.f/1000.f);
				}
			}

			m_leaderboard.m_entries[entryIndex].m_bPlayer = (entryIndex == 0);
			m_leaderboard.m_entries[entryIndex].m_bOnline = (entryIndex != 0);
			m_leaderboard.m_entries[entryIndex].m_bFriend = (entryIndex != 0);
			m_leaderboard.m_entries[entryIndex].m_bRequestedFriend = false;
		}

		//LeaderboardManager::Instance()->SetStatsRetrieved(true);

		return true;
	}

	//assert( LeaderboardManager::Instance()->GetStats() != NULL );
	//PXUSER_STATS_READ_RESULTS stats = LeaderboardManager::Instance()->GetStats();
	//if( m_currentFilter == LeaderboardManager::eFM_Friends  )	LeaderboardManager::Instance()->SortFriendStats();

	bool isDistanceLeaderboard = m_stats->pViews[0].dwViewId == STATS_VIEW_TRAVELLING_PEACEFUL || m_stats->pViews[0].dwViewId == STATS_VIEW_TRAVELLING_EASY || m_stats->pViews[0].dwViewId == STATS_VIEW_TRAVELLING_NORMAL || m_stats->pViews[0].dwViewId == STATS_VIEW_TRAVELLING_HARD;

	//First read
	if( m_leaderboard.m_totalEntryCount == 0 )
	{
		m_leaderboard.m_totalEntryCount = (m_currentFilter == LeaderboardManager::eFM_Friends) ? m_stats->pViews[0].dwNumRows : m_stats->pViews[0].dwTotalViewRows;
		m_leaderboard.m_currentEntryCount = m_stats->pViews[0].dwNumRows;

		if( m_leaderboard.m_totalEntryCount == 0 || m_leaderboard.m_currentEntryCount == 0 )
		{
			//LeaderboardManager::Instance()->SetStatsRetrieved(false);
			return false;
		}
		
		m_leaderboard.m_numColumns = m_stats->pViews[0].pRows[0].dwNumColumns;

		m_leaderboard.m_entryStartIndex	= (m_currentFilter == LeaderboardManager::eFM_Friends) ? 1 : m_stats->pViews[0].pRows[0].dwRank;

		for( unsigned int entryIndex=0 ; entryIndex<m_leaderboard.m_currentEntryCount ; ++entryIndex )
			CopyLeaderboardEntry(&(m_stats->pViews[0].pRows[entryIndex]), &(m_leaderboard.m_entries[entryIndex]), isDistanceLeaderboard);

		//If the filter mode is "My Score" then centre the list around the entries and select the player's score
		if( m_currentFilter == LeaderboardManager::eFM_MyScore )
		{
			//Centre the leaderboard list on the entries
			m_newTop = m_leaderboard.m_entryStartIndex-1;

			//Select the player entry
			for( unsigned int i=m_leaderboard.m_entryStartIndex ; i<m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount ; ++i )
				if( m_leaderboard.m_entries[i-m_leaderboard.m_entryStartIndex].m_bPlayer )
				{
					m_newSel = i-1; // this might be off the screen!
					// and reposition the top one
					if(m_newSel-m_newTop>9)
					{
						m_newTop=m_newSel-9;
					}
					break;
				}
		}
	}
	//Additional read
	else
	{
		unsigned int insertPosition = 0;

		//If the first new entry is at a smaller index than the current first entry
		if( m_newEntryIndex < m_leaderboard.m_entryStartIndex )
		{
			if( (m_leaderboard.m_entryStartIndex-1) < m_newReadSize )
				m_newReadSize = m_leaderboard.m_entryStartIndex-1;

			//Move current entries forward
			memmove((void*)(m_leaderboard.m_entries+m_newReadSize), (void*)m_leaderboard.m_entries, sizeof(LeaderboardEntry)*(NUM_ENTRIES-m_newReadSize));

			//Set the (now smaller) entry start index
			m_leaderboard.m_entryStartIndex = m_newEntryIndex;
		
			//We will be inserting the new entries at the start of the array
			insertPosition = 0;

			//Entry count is either max possible entries or current entry count + read size, whichever is smaller
			m_leaderboard.m_currentEntryCount = min((int)NUM_ENTRIES, (int)(m_leaderboard.m_currentEntryCount+m_newReadSize));
		}
		//If the last new entry is at a greater position than the last possible entry
		else if( m_newEntryIndex+m_newReadSize-1 >= m_leaderboard.m_entryStartIndex+NUM_ENTRIES )
		{
			//Calculate the overlap (this is by how much new entries would overhang the end of the array)
			int overlap = (((m_newEntryIndex-1)+m_newReadSize-1) - (m_leaderboard.m_entryStartIndex-1)) - NUM_ENTRIES + 1;

			//Move current entries backwards
			memmove((void*)m_leaderboard.m_entries, (void*)(m_leaderboard.m_entries+overlap), sizeof(LeaderboardEntry)*(NUM_ENTRIES-overlap));
		
			//Set the (now larger) start index
			m_leaderboard.m_entryStartIndex += overlap;

			//We will be inserting the new entries at the end of the array
			insertPosition = NUM_ENTRIES - m_newReadSize;

			//Entry count is max possible entries
			m_leaderboard.m_currentEntryCount = NUM_ENTRIES;
		}
		//Otherwise we're inserting at the end of the array and there is plenty of room
		else
		{
			insertPosition = m_leaderboard.m_currentEntryCount;
			m_leaderboard.m_currentEntryCount += m_newReadSize;
		}
		
		//For each entry in the leaderboard
		for( unsigned int entryIndex=0 ; entryIndex<m_newReadSize ; ++entryIndex )
		{
			CopyLeaderboardEntry(&(m_stats->pViews[0].pRows[entryIndex]), &(m_leaderboard.m_entries[insertPosition]), isDistanceLeaderboard);
			insertPosition++;
		}
	}

	//LeaderboardManager::Instance()->SetStatsRetrieved(true);
	return true;
}

HRESULT CScene_Leaderboards::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled)
{
	pGetItemCountData->cItems = m_leaderboard.m_totalEntryCount;
	
	//if(LeaderboardManager::Instance()->GetStatsRead())
	//{
	//	m_bReady=true;
	//}
	bHandled = TRUE;

	return S_OK;
}

HRESULT CScene_Leaderboards::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( pGetSourceTextData->bItemData )
	{
		if( m_newTop != -1 && m_newSel != -1 )
		{
			HRESULT ret = m_listGamers.SetTopItem(m_newTop);
			assert( ret == S_OK );

			ret = m_listGamers.SetCurSel(m_newSel);
			assert( ret == S_OK );

			// update the tooltips

			m_newTop = m_newSel = -1;

			bHandled = true;
			return S_OK;
		}

		unsigned int item = pGetSourceTextData->iItem;

		if( m_leaderboard.m_totalEntryCount > 0 && (item+1) < m_leaderboard.m_entryStartIndex )
		{
			if( LeaderboardManager::Instance()->isIdle() )
			{
				int readIndex = m_leaderboard.m_entryStartIndex - READ_SIZE;
				if( readIndex <= 0 )
					readIndex = 1;
				assert( readIndex >= 1 && readIndex <= (int)m_leaderboard.m_totalEntryCount );
				ReadStats(readIndex);
			}
		}
		else if( m_leaderboard.m_totalEntryCount > 0 && (item+1) >= (m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount) )
		{
			if( LeaderboardManager::Instance()->isIdle() )
			{
				int readIndex = m_leaderboard.m_entryStartIndex + m_leaderboard.m_currentEntryCount;
				assert( readIndex >= 1 && readIndex <= (int)m_leaderboard.m_totalEntryCount );
				ReadStats(readIndex);
			}
		}
		else
		{
			unsigned int index = item - (m_leaderboard.m_entryStartIndex-1);
			if( 0 == pGetSourceTextData->iData ) 
			{
				pGetSourceTextData->szText = m_leaderboard.m_entries[index].m_wcRank;
				bHandled = TRUE;
			}
			else if( 1 == pGetSourceTextData->iData ) 
			{
				pGetSourceTextData->szText = m_leaderboard.m_entries[index].m_gamerTag;
				bHandled = TRUE;
			}
			else if( pGetSourceTextData->iData >= 3 && pGetSourceTextData->iData <= 9 )
			{
				if( m_leaderboard.m_numColumns <= (unsigned int)(pGetSourceTextData->iData-3) )
					pGetSourceTextData->szText = L"";
				else
					pGetSourceTextData->szText = m_leaderboard.m_entries[index].m_wcColumns[pGetSourceTextData->iData-3];
				bHandled = TRUE;
			}
		}
	}
	return S_OK;
}

HRESULT CScene_Leaderboards::OnGetSourceDataImage(XUIMessageGetSourceImage* pGetImage, BOOL& bHandled)
{
	if( (pGetImage->iData == 2) && (pGetImage->bItemData) )
	{
		if( m_newTop != -1 && m_newSel != -1 )
		{
			//Do nothing, alignment handled in OnGetSourceDataText

			bHandled = true;
			return S_OK;
		}

		unsigned int item = pGetImage->iItem;
		if( m_leaderboard.m_totalEntryCount > 0 && (item+1) < m_leaderboard.m_entryStartIndex )
		{
			//Do nothing, reading handled in OnGetSourceDataText
		}
		else if( m_leaderboard.m_totalEntryCount > 0 && (item+1) >= (m_leaderboard.m_entryStartIndex+m_leaderboard.m_currentEntryCount) )
		{
			//Do nothing, reading handled in OnGetSourceDataText
		}
		// 4J-PB - we're not allowed to show flags any more
// 		else
// 		{
// 			unsigned int index = item - (m_leaderboard.m_entryStartIndex-1);
// 			if( m_leaderboard.m_entries[index].m_locale > 0 && m_leaderboard.m_entries[index].m_locale <= XC_LOCALE_RUSSIAN_FEDERATION )
// 				pGetImage->szPath = FLAG_ICON_PATHS[ m_leaderboard.m_entries[index].m_locale-1 ];		
// 			bHandled = TRUE;
// 		}	
	}

	return S_OK;
}

void CScene_Leaderboards::PopulateLeaderboard(bool noResults)
{
	HRESULT hr;
	HXUIOBJ visual=NULL;
	HXUIOBJ hTemp=NULL;
	hr=XuiControlGetVisual(m_listGamers.m_hObj,&visual);

	if(m_pHTitleIconSlots[0]==NULL)
	{	
		VOID *pObj;
		HXUIOBJ button;
		D3DXVECTOR3 vPos;

		for(int i=0;i<7;i++)
		{
			// retrieve the visual for the title icon
			hr=XuiElementGetChildById(visual,m_TitleIconNameA[i],&button);

			XuiObjectFromHandle( button, &pObj );
			m_pHTitleIconSlots[i] = (CXuiCtrlCraftIngredientSlot *)pObj;

			// store the default position, since we'll be repositioning these depending on how many are valid for each board
			m_pHTitleIconSlots[i]->GetPosition(&vPos);
			m_fTitleIconXPositions[i]= vPos.x;
		}
	}

	int iValidSlots=SetLeaderboardTitleIcons();

	if( !noResults && m_leaderboard.m_totalEntryCount > 0 )
	{
		hr=XuiElementGetChildById(visual,L"XuiLabel_Gamertag",&hTemp);
		if(hTemp)
		{
			XuiControlSetText(hTemp,app.GetString( IDS_LEADERBOARD_GAMERTAG ));
			XuiElementSetShow(hTemp, TRUE);
		}
		hr=XuiElementGetChildById(visual,L"XuiLabel_Rank",&hTemp);
		if(hTemp)
		{
			XuiControlSetText(hTemp,app.GetString( IDS_LEADERBOARD_RANK ));
			XuiElementSetShow(hTemp, TRUE);
		}

		//Update entries display
		wchar_t entriesBuffer[40];
		if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
		{
			swprintf(entriesBuffer, 40, L"%ls12345678", app.GetString(IDS_LEADERBOARD_ENTRIES));
		}
		else
		{
			swprintf(entriesBuffer, 40, L"%ls%i", app.GetString(IDS_LEADERBOARD_ENTRIES), m_leaderboard.m_totalEntryCount);
		}

		m_textEntries.SetText(entriesBuffer);

		//Hide the loading message
		m_textInfo.SetShow(false);

		//Add space for new entries
		m_listGamers.InsertItems(0, m_leaderboard.m_totalEntryCount);

		// 4J Stu - Fix for leaderboards taking forever to update. Was looping over m_totalEntryCount which is ~2mil in
		// some leaderboards in production atm.
		// Changed to loop over the range of cached values, although even that is probably overkill.
		// Really only the newly updated rows need changed, but this shouldn't cause any performance issues
		for(DWORD i = m_leaderboard.m_entryStartIndex - 1; i < (m_leaderboard.m_entryStartIndex - 1) + m_leaderboard.m_currentEntryCount; ++i)
		{	
			HXUIOBJ visual=NULL;
			HXUIOBJ button;
			D3DXVECTOR3 vPos;
			// 4J-PB - fix for #13768 - Leaderboards: Player scores appear misaligned when viewed under the "My Score" leaderboard filter
			HXUIOBJ hTop=m_listGamers.GetItemControl(i-(m_leaderboard.m_entryStartIndex - 1));
			HRESULT hr=XuiControlGetVisual(hTop,&visual);

			for(int j=0;j<7;j++)
			{
				hr=XuiElementGetChildById(visual,m_TextColumnNameA[j],&button);
				m_hTextEntryA[j]=button;

				XuiElementGetPosition(button,&vPos);
				m_fTextXPositions[j]=vPos.x;
			}
			RepositionText(iValidSlots);
		}

		// hide empty icon boxes
		for(int i = (LEADERBOARD_DESCRIPTORS[m_currentLeaderboard][m_currentDifficulty].m_columnCount);i<7;i++)
		{
			m_pHTitleIconSlots[i]->SetShow(FALSE);
		}
	}
	else
	{
		hr=XuiElementGetChildById(visual,L"XuiLabel_Gamertag",&hTemp);
		if(hTemp)
		{
			XuiElementSetShow(hTemp, FALSE);
		}
		hr=XuiElementGetChildById(visual,L"XuiLabel_Rank",&hTemp);
		if(hTemp)
		{
			XuiElementSetShow(hTemp, FALSE);
		}

		//Update entries display (to zero)
		wchar_t entriesBuffer[40];
		swprintf(entriesBuffer, 40, L"%ls0", app.GetString(IDS_LEADERBOARD_ENTRIES));
		m_textEntries.SetText(entriesBuffer);

		//Show the no results message
		m_textInfo.SetText(app.GetString(IDS_LEADERBOARD_NORESULTS));
		m_textInfo.SetShow(true);

		// hide the icons
		for(int i=0;i<7;i++)
		{
			m_pHTitleIconSlots[i]->SetShow(FALSE);
		}

	}

	m_bPopulatedOnce = true;
}

void CScene_Leaderboards::CopyLeaderboardEntry(PXUSER_STATS_ROW statsRow, LeaderboardEntry* leaderboardEntry, bool isDistanceLeaderboard)
{
	leaderboardEntry->m_xuid = statsRow->xuid;

	//Copy the rank
	leaderboardEntry->m_rank = statsRow->dwRank;
	DWORD displayRank = leaderboardEntry->m_rank;
	if(displayRank > 9999999) displayRank = 9999999;
	swprintf_s(leaderboardEntry->m_wcRank, 12, L"%u", displayRank);
					
	//strcpy(statsRow->szGamertag,"WWWWWWWWWWWWWWWW");

	//Convert the gamertag from char to wchar_t
	MultiByteToWideChar(
		CP_UTF8,
		0,
		statsRow->szGamertag,
		strlen(statsRow->szGamertag),
		leaderboardEntry->m_gamerTag,
		XUSER_NAME_SIZE );
	//Null terminate the gamertag
	leaderboardEntry->m_gamerTag[strlen(statsRow->szGamertag)] = L'\0';

	//Copy the locale
	//leaderboardEntry->m_locale = statsRow->pColumns[0].Value.nData;

	//Copy the other columns
	for( unsigned int i=0 ; i<statsRow->dwNumColumns ; i++ )
	{
		leaderboardEntry->m_columns[i] = statsRow->pColumns[i].Value.nData;
		if( !isDistanceLeaderboard )
		{
			DWORD displayValue = leaderboardEntry->m_columns[i];
			if(displayValue > 99999) displayValue = 99999;
			swprintf_s(leaderboardEntry->m_wcColumns[i], 12, L"%u",displayValue);
#ifdef _DEBUG
			app.DebugPrintf("Value - %d\n",leaderboardEntry->m_columns[i]);
#endif
		}
		else
		{
			// check how many digits we have
			int iDigitC=0;
			unsigned int uiVal=leaderboardEntry->m_columns[i];
// 			uiVal=0xFFFFFFFF;
// 			leaderboardEntry->m_columns[i-1]=uiVal;

			while(uiVal!=0)
			{
				uiVal/=10;
				iDigitC++;
			}

#ifdef _DEBUG
			app.DebugPrintf("Value - %d\n",leaderboardEntry->m_columns[i]);
#endif
			if(iDigitC<4)
			{
				// m
				swprintf_s(leaderboardEntry->m_wcColumns[i], 12, L"%um", leaderboardEntry->m_columns[i]);
#ifdef _DEBUG
				app.DebugPrintf("Display - %um\n", leaderboardEntry->m_columns[i]);
#endif
			}
			else if(iDigitC<8)
			{
				// km with a .X
				swprintf_s(leaderboardEntry->m_wcColumns[i], 12, L"%.1fkm", ((float)leaderboardEntry->m_columns[i])/1000.f);
#ifdef _DEBUG
				app.DebugPrintf("Display - %.1fkm\n", ((float)leaderboardEntry->m_columns[i])/1000.f);
#endif
			}
			else
			{
				// bigger than that, so no decimal point
				swprintf_s(leaderboardEntry->m_wcColumns[i], 12, L"%.0fkm", ((float)leaderboardEntry->m_columns[i])/1000.f);
#ifdef _DEBUG
				app.DebugPrintf("Display - %.0fkm\n", ((float)leaderboardEntry->m_columns[i])/1000.f);
#endif
			}
		}
	}

	//Is the player
	if( statsRow->xuid == ((XboxLeaderboardManager*)LeaderboardManager::Instance())->GetMyXUID() )
	{
		leaderboardEntry->m_bPlayer = true;
		leaderboardEntry->m_bOnline = false;
		leaderboardEntry->m_bFriend = false;
		leaderboardEntry->m_bRequestedFriend = false;
	}
	else
	{
		leaderboardEntry->m_bPlayer = false;
		leaderboardEntry->m_bOnline = false;
		leaderboardEntry->m_bFriend = false;
		leaderboardEntry->m_bRequestedFriend = false;

		//Check for friend status
		for( unsigned int friendIndex=0 ; friendIndex<m_numFriends ; ++friendIndex )
		{
			if( m_friends[friendIndex].xuid == statsRow->xuid )
			{
				if( ( m_friends[friendIndex].dwFriendState & ( XONLINE_FRIENDSTATE_FLAG_SENTREQUEST | XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST ) ) == 0 )
				{
					//Is friend, might be online
					leaderboardEntry->m_bFriend = true;
					leaderboardEntry->m_bOnline = ( m_friends[friendIndex].dwFriendState & XONLINE_FRIENDSTATE_FLAG_ONLINE );
					leaderboardEntry->m_bRequestedFriend = false;
				}
				else
				{
					//Friend request sent but not accepted yet
					leaderboardEntry->m_bOnline = false;
					leaderboardEntry->m_bFriend = false;
					leaderboardEntry->m_bRequestedFriend = true;
				}

				break;
			}
		}
	}
}

void CScene_Leaderboards::SetLeaderboardHeader()
{
	WCHAR buffer[40];
	DWORD bufferLength = 40;

	DWORD ret = XResourceGetString(LEADERBOARD_HEADERS[m_currentLeaderboard][m_currentDifficulty], buffer, &bufferLength, NULL);

	if( ret == ERROR_SUCCESS )
		m_textLeaderboard.SetText(buffer);
}

int CScene_Leaderboards::SetLeaderboardTitleIcons()
{
	int iValidIcons=0;

	for(int i=0;i<7;i++)
	{
		if(TitleIcons[m_currentLeaderboard][i]==0)
		{
			m_pHTitleIconSlots[i]->SetShow(FALSE);
		}
		else
		{
			iValidIcons++;
			m_pHTitleIconSlots[i]->SetIcon(DEFAULT_XUI_MENU_USER,TitleIcons[m_currentLeaderboard][i],0,1,10,31,false,FALSE);
		}
	}

	Reposition(iValidIcons);
		
	return iValidIcons;
}
void CScene_Leaderboards::ClearLeaderboardTitlebar()
{
	for(int i=0;i<7;i++)
	{
		m_pHTitleIconSlots[i]->SetShow(FALSE);
	}

	HXUIOBJ visual=NULL;
	HXUIOBJ hTemp=NULL;
	HRESULT hr;
	hr=XuiControlGetVisual(m_listGamers.m_hObj,&visual);

	hr=XuiElementGetChildById(visual,L"XuiLabel_Gamertag",&hTemp);
	if(hTemp)
	{
		XuiElementSetShow(hTemp, FALSE);
	}
	hr=XuiElementGetChildById(visual,L"XuiLabel_Rank",&hTemp);
	if(hTemp)
	{
		XuiElementSetShow(hTemp, FALSE);
	}
}
