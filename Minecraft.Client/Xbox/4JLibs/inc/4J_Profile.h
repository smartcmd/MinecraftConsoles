#pragma once

enum eAwardType
{
	eAwardType_Achievement	= 0,
	eAwardType_GamerPic,
	eAwardType_Theme,
	eAwardType_AvatarItem,
};

enum eUpsellType
{
	eUpsellType_Custom = 0, // This is the default, and means that the upsell dialog was initiated in the app code
	eUpsellType_Achievement,
	eUpsellType_GamerPic,
	eUpsellType_Theme,
	eUpsellType_AvatarItem,
};

enum eUpsellResponse
{
	eUpsellResponse_Declined,
	eUpsellResponse_Accepted_NoPurchase,
	eUpsellResponse_Accepted_Purchase,
};



class C_4JProfile
{
public:
	struct PROFILESETTINGS
	{
		int iYAxisInversion;
		int iControllerSensitivity;
		int iVibration;
		bool bSwapSticks;
	};


	// 4 players have game defined data, puiGameDefinedDataChangedBitmask needs to be checked by the game side to see if there's an update needed - it'll have the bits set for players to be updated
	void				Initialise( DWORD dwTitleID,
									DWORD dwOfferID,
									unsigned short usProfileVersion,
									UINT uiProfileValuesC,
									UINT uiProfileSettingsC,
									DWORD *pdwProfileSettingsA, 
									int iGameDefinedDataSizeX4,
									unsigned int *puiGameDefinedDataChangedBitmask); 
	void				SetTrialTextStringTable(CXuiStringTable *pStringTable,int iAccept,int iReject);
	void				SetTrialAwardText(eAwardType AwardType,int iTitle,int iText); // achievement popup in the trial game
	int					GetLockedProfile();
	void				SetLockedProfile(int iProf);
	bool				IsSignedIn(int iQuadrant);
	bool				IsSignedInLive(int iProf);
	bool				IsGuest(int iQuadrant);
	UINT				RequestSignInUI(bool bFromInvite,bool bLocalGame,bool bNoGuestsAllowed,bool bMultiplayerSignIn,bool bAddUser, int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	UINT				DisplayOfflineProfile(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	UINT				RequestConvertOfflineToGuestUI(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	void				SetPrimaryPlayerChanged(bool bVal);								
	bool				QuerySigninStatus(void);
	void				GetXUID(int iPad, XUID *pXuid,bool bOnlineXuid);
	BOOL				AreXUIDSEqual(XUID xuid1,XUID xuid2);
	BOOL				XUIDIsGuest(XUID xuid);
	bool				AllowedToPlayMultiplayer(int iProf);
	void				StartTrialGame(); // disables saves and leaderboard, and change state to readyforgame from pregame
	void				AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL *allAllowed, BOOL *friendsAllowed);
	BOOL				CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PXUID pXuids, DWORD dwXuidCount );

				
	// SYS
	int					GetPrimaryPad();
	void				SetPrimaryPad(int iPad);
	char*				GetGamertag(int iPad);
	wchar_t*			GetDisplayName(int iPad);
	bool				IsFullVersion();
	void				SetSignInChangeCallback(void ( *Func)(LPVOID, bool, unsigned int),LPVOID lpParam);
	void				SetNotificationsCallback(void ( *Func)(LPVOID, DWORD, unsigned int),LPVOID lpParam);
	bool				RegionIsNorthAmerica(void);
	bool				LocaleIsUSorCanada(void);
	HRESULT				GetLiveConnectionStatus();
	bool				IsSystemUIDisplayed();
	void				SetProfileReadErrorCallback(void ( *Func)(LPVOID), LPVOID lpParam);


	// PROFILE DATA
	int					SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam);
	int					SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam);
	PROFILESETTINGS *	GetDashboardProfileSettings(int iPad);
	void				WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged=false, bool bOverride5MinuteLimitOnProfileWrites=false);
	void				ForceQueuedProfileWrites(int iPad=XUSER_INDEX_ANY);
	void				*GetGameDefinedProfileData(int iQuadrant);
	void				ResetProfileProcessState(); // after a sign out from the primary player, call this
	void				Tick( void );

	// ACHIEVEMENTS & AWARDS

	void				RegisterAward(int iAwardNumber,int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected=false, 
										CXuiStringTable*pStringTable=nullptr, int iTitleStr=-1, int iTextStr=-1, int iAcceptStr=-1, char *pszThemeName=nullptr, unsigned int uiThemeSize=0L);
	int					GetAwardId(int iAwardNumber);
	eAwardType			GetAwardType(int iAwardNumber);
	bool				CanBeAwarded(int iQuadrant, int iAwardNumber);
	void				Award(int iQuadrant, int iAwardNumber, bool bForce=false);
	bool				IsAwardsFlagSet(int iQuadrant, int iAward);	

	// RICH PRESENCE

	void				RichPresenceInit(int iPresenceCount, int iContextCount);
	void				RegisterRichPresenceContext(int iGameConfigContextID);
	void				SetRichPresenceContextValue(int iPad,int iContextID, int iVal);
	void				SetCurrentGameActivity(int iPad,int iNewPresence, bool bSetOthersToIdle=false);

	// PURCHASE
	void				DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam = -1);
	void				SetUpsellCallback(void ( *Func)(LPVOID lpParam, eUpsellType type, eUpsellResponse response, int iUserData),LPVOID lpParam);

	// Debug 
	void				SetDebugFullOverride(bool bVal); // To override the license version (trail/full). Only in debug/release, not ContentPackage

};

// Singleton
extern C_4JProfile ProfileManager;

