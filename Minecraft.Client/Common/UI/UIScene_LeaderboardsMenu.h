#pragma once

#include "UIScene.h"
// #include "..\Leaderboards\LeaderboardManager.h"
#include "..\Leaderboards\LeaderboardInterface.h"

class UIScene_LeaderboardsMenu : public UIScene, public LeaderboardReadListener
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

	struct LeaderboardDescriptor {
		unsigned int m_columnCount;
		bool m_isDistanceLeaderboard;
		unsigned int m_title;

		LeaderboardDescriptor(unsigned int columnCount, bool isDistanceLeaderboard, unsigned int title)
		{
			m_columnCount = columnCount;
			m_isDistanceLeaderboard = isDistanceLeaderboard;
			m_title = title;
		}
	};

	static const LeaderboardDescriptor LEADERBOARD_DESCRIPTORS[NUM_LEADERBOARDS][4];
	static const int TitleIcons[NUM_LEADERBOARDS][7];

	struct LeaderboardEntry {
		PlayerUID		m_xuid;
		unsigned int m_row; // Row identifier for passing to Iggy as a unique identifier
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
		int			m_idsErrorMessage;		// 4J-JEV: Non-zero if this entry has an error message instead of results.
	};

	struct Leaderboard {
		DWORD						m_totalEntryCount;		//Either total number of entries in leaderboard, or total number of results for a friends query
		vector<LeaderboardEntry>	m_entries;
		DWORD						m_numColumns;
	};

	Leaderboard m_leaderboard;				//All leaderboard data for the currently selected filter
	
	unsigned int	m_currentLeaderboard;	//The current leaderboard selected for view
	LeaderboardManager::EFilterMode m_currentFilter;		//The current filter selected
	unsigned int	m_currentDifficulty;	//The current difficulty selected

	unsigned int	m_newEntryIndex;		//Index of the first entry being read
	unsigned int	m_newReadSize;			//Number of entries in the current read operation

	unsigned int	m_newEntriesCount;		// Number of new entries in this update

	int				m_newTop;				//Index of the element that should be at the top of the list
	int				m_newSel;				//Index of the element that should be selected in the list

	bool			m_isProcessingStatsRead;
	bool			m_bPopulatedOnce;
	bool			m_bReady;

	LeaderboardInterface m_interface;

	UIControl_LeaderboardList m_listEntries;
	UIControl_Label m_labelFilter, m_labelLeaderboard, m_labelEntries, m_labelInfo;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_listEntries, "Gamers")

		UI_MAP_ELEMENT( m_labelFilter, "Filter")
		UI_MAP_ELEMENT( m_labelLeaderboard, "Leaderboard")
		UI_MAP_ELEMENT( m_labelEntries, "Entries")
		UI_MAP_ELEMENT( m_labelInfo, "Info")
	UI_END_MAP_ELEMENTS_AND_NAMES()

	static int ExitLeaderboards(void *pParam,int iPad,C4JStorage::EMessageResult result);

public:
	UIScene_LeaderboardsMenu(int iPad, void *initData, UILayer *parentLayer);
	~UIScene_LeaderboardsMenu();
	
	virtual void updateTooltips();
	virtual void updateComponents();

	virtual EUIScene getSceneType() { return eUIScene_LeaderboardsMenu;}

	// Returns true if this scene has focus for the pad passed in
	virtual bool hasFocus(int iPad) { return bHasFocus; }
	virtual void handleTimerComplete(int id);

private:
	int GetEntryStartIndex();

protected:
	virtual wstring getMoviePath();

public:
	virtual void tick();
	virtual void handleReload();

	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

private:
	//Start a read request with the current parameters
	void ReadStats(int startIndex);

	//Copy the stats from the raw m_stats structure into the m_leaderboards structure
	int m_numStats;
	LeaderboardManager::ViewOut m_stats;
	bool RetrieveStats();

	// Copy a leaderboard entry from the stats row
	void CopyLeaderboardEntry(LeaderboardManager::ReadScore *statsRow, int leaderboardEntryIndex, bool isDistanceLeaderboard);

	//Populate the XUI leaderboard with the contents of m_leaderboards
	void PopulateLeaderboard(LeaderboardManager::eStatsReturn ret);

	//Set the header text of the leaderboard
	void SetLeaderboardHeader();
	
	// Set the title icons
	int SetLeaderboardTitleIcons();

	//Callback function called when stats read completes, userdata contains pointer to instance of CScene_Leaderboards
	virtual bool OnStatsReadComplete(LeaderboardManager::eStatsReturn ret, int numResults, LeaderboardManager::ViewOut results);

	virtual void customDraw(IggyCustomDrawCallbackRegion *region);

	virtual void handleSelectionChanged(F64 selectedId);
	virtual void handleRequestMoreData(F64 startIndex, bool up);

	bool m_bIgnoreInput;
};
