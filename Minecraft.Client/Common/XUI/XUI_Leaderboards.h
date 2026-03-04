#pragma once
#include "XUI_Helper.h"
#include "../media/xuiscene_leaderboards.h"

#include "..\Leaderboards\LeaderboardManager.h"

class CXuiCtrlCraftIngredientSlot;

class CScene_Leaderboards : public CXuiSceneImpl, public LeaderboardReadListener
{
private:
	// 4J Stu - Because the kills leaderboard doesn't a peaceful entry there are some special
	// handling to make it skip that. We have re-arranged the order of the leaderboards so
	// I am making this in case we do it again.
	// 4J Stu - Made it a member of the class, rather than a #define
	static const int LEADERBOARD_KILLS_POSITION = 3;

	static const int NUM_LEADERBOARDS	= 4;//6;		//Number of leaderboards
	static const int NUM_ENTRIES		= 101;		//Cache up to this many entries
	static const int READ_SIZE			= 15;		//Read this many entries at a time

//	static LPCWSTR FLAG_ICON_PATHS[37];
	int m_iPad;

	bool m_bPopulatedOnce;

	static const int LEADERBOARD_HEADERS[NUM_LEADERBOARDS][4];

	static const int TitleIcons[NUM_LEADERBOARDS][7];
	static LPCWSTR m_TitleIconNameA[7];
	static LPCWSTR m_TextColumnNameA[7];
	HXUIOBJ m_hTextEntryA[7];

	struct LeaderboardDescriptor {
		DWORD	m_viewId;
		DWORD	m_columnCount;
		WORD	m_columnIds[8];

		LeaderboardDescriptor(DWORD viewId, DWORD columnCount, WORD columnId_0, WORD columnId_1, WORD columnId_2, WORD columnId_3, WORD columnId_4, WORD columnId_5, WORD columnId_6, WORD columnId_7)
		{
			m_viewId = viewId;
			m_columnCount = columnCount;
			m_columnIds[0] = columnId_0;
			m_columnIds[1] = columnId_1;
			m_columnIds[2] = columnId_2;
			m_columnIds[3] = columnId_3;
			m_columnIds[4] = columnId_4;
			m_columnIds[5] = columnId_5;
			m_columnIds[6] = columnId_6;
			m_columnIds[7] = columnId_7;
		}
	};

	static const LeaderboardDescriptor LEADERBOARD_DESCRIPTORS[NUM_LEADERBOARDS][4];

	struct LeaderboardEntry {
		PlayerUID		m_xuid;
		DWORD		m_rank;
		WCHAR		m_wcRank[12];
		WCHAR		m_gamerTag[XUSER_NAME_SIZE+1];
		//int			m_locale;
		unsigned int	m_columns[7];
		WCHAR		m_wcColumns[7][12];
        bool		m_bPlayer;				//Is the player
		bool		m_bOnline;				//Is online
		bool		m_bFriend;				//Is friend
		bool		m_bRequestedFriend;		//Friend request sent but not answered
	};

	struct Leaderboard {
		DWORD				m_totalEntryCount;		//Either total number of entries in leaderboard, or total number of results for a friends query
		DWORD				m_entryStartIndex;		//Index of first entry
		DWORD				m_currentEntryCount;	//Current number of entries
		LeaderboardEntry	m_entries[NUM_ENTRIES];
		DWORD				m_numColumns;
	};
		
	Leaderboard m_leaderboard;				//All leaderboard data for the currently selected filter

	unsigned int	m_currentLeaderboard;	//The current leaderboard selected for view

#ifdef _XBOX
	LeaderboardManager::EFilterMode m_currentFilter;		//The current filter selected
#endif
	unsigned int	m_currentDifficulty;	//The current difficulty selected
	
	unsigned int	m_newEntryIndex;		//Index of the first entry being read
	unsigned int	m_newReadSize;			//Number of entries in the current read operation

	int				m_newTop;				//Index of the element that should be at the top of the list
	int				m_newSel;				//Index of the element that should be selected in the list

	XONLINE_FRIEND*	m_friends;				//Current player's friends
	unsigned int	m_numFriends;			//Count of friends
	PlayerUID*		m_filteredFriends;		//List of all friend XUIDs (and player's), only counting actual friends, not pending
	unsigned int	m_numFilteredFriends;	//Count of filtered friends

	CXuiList		m_listGamers;			//The XUI list showing the leaderboard info
	CXuiControl		m_textLeaderboard;		//The XUI text box showing the current leaderboard name
	CXuiControl		m_textInfo;				//The XUI text box showing info messages (loading, no results, etc.)
	CXuiControl		m_textFilter;			//The XUI text box showing the current filter
	CXuiControl		m_textEntries;			//The XUI text box showing the total number of entries in this leaderboard
	CXuiCtrlCraftIngredientSlot *m_pHTitleIconSlots[7];
	float			m_fTitleIconXPositions[7];
	float			m_fTextXPositions[7];

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY( OnDestroy )
		XUI_ON_XM_KEYDOWN( OnKeyDown )
		XUI_ON_XM_GET_ITEMCOUNT_ALL(OnGetItemCountAll)
		XUI_ON_XM_GET_SOURCE_TEXT(OnGetSourceDataText)
		XUI_ON_XM_GET_SOURCE_IMAGE(OnGetSourceDataImage)
		XUI_ON_XM_NOTIFY_SELCHANGED(OnNotifySelChanged)
		XUI_ON_XM_NOTIFY_SET_FOCUS(OnNotifySetFocus)

	XUI_END_MSG_MAP()

	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiListGamers,			m_listGamers)
		MAP_CONTROL(IDC_XuiTextLeaderboard,		m_textLeaderboard)
		MAP_CONTROL(IDC_XuiTextInfo,			m_textInfo)
		MAP_CONTROL(IDC_XuiTextFilter,			m_textFilter)
		MAP_CONTROL(IDC_XuiTextEntries,			m_textEntries)
	END_CONTROL_MAP()

	HRESULT OnInit(XUIMessageInit* pInitData, BOOL& bHandled);
	HRESULT OnDestroy();
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& bHandled);
	HRESULT OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData, BOOL& bHandled);
	HRESULT OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData, BOOL& bHandled);
	HRESULT OnGetSourceDataImage(XUIMessageGetSourceImage* pGetImage, BOOL& bHandled);
	HRESULT OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
	HRESULT OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled);

	//Start a read request with the current parameters
	void ReadStats(int startIndex);
	
	//Callback function called when stats read completes, userdata contains pointer to instance of CScene_Leaderboards
	virtual bool OnStatsReadComplete(bool success, int numResults, LeaderboardManager::ViewOut results);
	
	//Copy the stats from the raw m_stats structure into the m_leaderboards structure
	int m_numStats;
	PXUSER_STATS_READ_RESULTS m_stats;
	bool RetrieveStats();

	//Populate the XUI leaderboard with the contents of m_leaderboards
	void PopulateLeaderboard(bool noResults);

	//Copy a leaderboard entry from the stats row
	void CopyLeaderboardEntry(PXUSER_STATS_ROW statsRow, LeaderboardEntry* leaderboardEntry, bool isDistanceLeaderboard);

	//Set the header text of the leaderboard
	void SetLeaderboardHeader();

	// Set the title icons
	int SetLeaderboardTitleIcons();
	void ClearLeaderboardTitlebar();
	void Reposition(int iNumber);
	void RepositionText(int iNumber);
	void UpdateTooltips();

protected:
	bool			m_isProcessingStatsRead;
	bool m_bReady;
public:

	XUI_IMPLEMENT_CLASS( CScene_Leaderboards, L"CScene_Leaderboards", XUI_CLASS_SCENE )
};
