#pragma once
#include <np/np_common.h>
#include <np_toolkit.h>
#include <np.h>
#include <message_dialog.h>

using namespace sce::Toolkit::NP;
using namespace sce::Toolkit::NP::Utilities;

class CXuiStringTable;

// Note - there are now 3 types of PlayerUID
// (1) A full online ID - either the primary login, or a sub-signin through to PSN. This has m_onlineID set up as a normal SceNpOnlineId, with dummy[0] set to 0
// (2) An offline ID, where there is also a primary login on the system. This has m_onlineID set up to copy the primary SceNpOnlineId, except with dummy[0] set to the controller ID of this other player
// (3) An offline ID, where there isn't a primary PSN login on the system. This has SceNpOnlineId fully zeroed.

class PlayerUID
{
	char m_onlineID[SCE_NP_ONLINEID_MAX_LENGTH];
	char term;
	bool m_bSignedIntoPSN : 1;
	unsigned char m_quadrant : 2;
	uint8_t m_macAddress[SCE_NET_ETHER_ADDR_LEN];
	int	m_userID;		// user logged on to the XMB

public:

	class Hash 
	{
	public:		
		std::size_t operator()(const PlayerUID& k) const;
	};

	PlayerUID();
	PlayerUID(int userID, SceNpOnlineId& onlineID, bool bSignedInPSN, int quadrant);
	PlayerUID(std::wstring fromString);

	bool operator==(const PlayerUID& rhs) const;
	bool operator!=(const PlayerUID& rhs);
	void setCurrentMacAddress();
	std::wstring macAddressStr() const;
	std::wstring userIDStr() const;
	std::wstring toString() const;
	void setOnlineID(SceNpOnlineId& id, bool bSignedIntoPSN);
	void setUserID(unsigned int id);


	const char*			getOnlineID() const		{ return m_onlineID; }
	int					getUserID() const		{ return m_userID; }
	int					getQuadrant() const		{ return m_quadrant;  }
	bool				isPrimaryUser() const;	// only true if we're on the local machine and signed into the first quadrant;
	bool				isSignedIntoPSN() const	{ return m_bSignedIntoPSN; }
	void				setForAdhoc();
private:
};

typedef PlayerUID *PPlayerUID;

class GameSessionUID
{
	char m_onlineID[SCE_NP_ONLINEID_MAX_LENGTH];
	char term;
	bool m_bSignedIntoPSN : 1;
	unsigned char m_quadrant : 2;
public:
	GameSessionUID();
	GameSessionUID(int nullVal);

	bool operator==(const GameSessionUID& rhs) const;
	bool operator!=(const GameSessionUID& rhs);
	GameSessionUID& operator=(const PlayerUID& rhs);

	const char*			getOnlineID() const		{ return m_onlineID; }
	int					getQuadrant() const		{ return m_quadrant;  }
	bool				isSignedIntoPSN() const	{ return m_bSignedIntoPSN; }
	void				setForAdhoc();

};

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
	eUpsellResponse_UserNotSignedInPSN
};

class C_4JProfile
{
public:

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// INIT
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 4 players have game defined data, puiGameDefinedDataChangedBitmask needs to be checked by the game side to see if there's an update needed - it'll have the bits set for players to be updated
	void				Initialise( const SceNpCommunicationConfig _commsId,
									const std::string _serviceID,
									unsigned short usProfileVersion,
									UINT uiProfileValuesC,
									UINT uiProfileSettingsC,
									DWORD *pdwProfileSettingsA, 
									int iGameDefinedDataSizeX4,
									unsigned int *puiGameDefinedDataChangedBitmask); 

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SIGN-IN/USERS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool				IsSignedIn(int iQuadrant);
	bool				IsSignedInLive(int iProf);
	bool				IsSignedInPSN(int iProf);
	bool				IsGuest(int iQuadrant);
	UINT				RequestSignInUI(bool bFromInvite,bool bLocalGame,bool bNoGuestsAllowed,bool bMultiplayerSignIn,bool bAddUser, int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	UINT				DisplayOfflineProfile(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	UINT				RequestConvertOfflineToGuestUI(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant=XUSER_INDEX_ANY);
	void				SetPrimaryPlayerChanged(bool bVal);								
	bool				QuerySigninStatus(void);
	void				GetXUID(int iPad, PlayerUID *pXuid,bool bOnlineXuid);
	BOOL				AreXUIDSEqual(PlayerUID xuid1,PlayerUID xuid2);
	void				GetSceNpId(int iPad, SceNpId *npId);
	DWORD				GetSignedInUsersMask();
	void				SetNetworkStatus(bool bOnlinePSN, bool bSignedInPSN);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// MISC
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	int					GetLockedProfile();
	void				SetLockedProfile(int iProf);
	void				SetGetStringFunc(LPCWSTR ( *Func)(int));
	void				SetPlayerListTitleID(int id);
	bool				AllowedToPlayMultiplayer(int iProf);
	bool				HasPlayStationPlus(int iProf);
	void				StartTrialGame(); // disables saves and leaderboard, and change state to readyforgame from pregame
	void				AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL *allAllowed, BOOL *friendsAllowed);
	BOOL				CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pXuids, DWORD dwXuidCount );
	void				ResetProfileProcessState(); // after a sign out from the primary player, call this	
	void				Tick( void );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// AVATAR
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	typedef struct 
	{
		int					iPad;
		int					( *m_fnFunc)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes);
		LPVOID				m_fnFunc_Param;
	}
	FUNCPARAMS;
	bool				GetProfileAvatar(int iPad,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam);
	void				CancelProfileAvatarRequest();	
		
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SYS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int					GetPrimaryPad();
	void				SetPrimaryPad(int iPad);
	char*				GetGamertag(int iPad);
	std::wstring		GetDisplayName(int iPad);										

	bool				IsFullVersion();
	void				SetFullVersion(bool bFull);
	void				SetSignInChangeCallback(void ( *Func)(LPVOID, bool, unsigned int),LPVOID lpParam);
	void				SetNotificationsCallback(void ( *Func)(LPVOID, DWORD, unsigned int),LPVOID lpParam);
	bool				RegionIsNorthAmerica(void);
	bool				LocaleIsUSorCanada(void);
	HRESULT				GetLiveConnectionStatus();
	bool				IsSystemUIDisplayed();
	void				SetSysUIShowing( bool bUIDisplayed );
	void				DisplaySystemMessage( SceMsgDialogSystemMessageType _type, int iQuadrant);
	void				SetProfileReadErrorCallback(void ( *Func)(LPVOID), LPVOID lpParam);
	void				ShowSystemMessage( int _type, int _val );

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ACHIEVEMENTS & AWARDS
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void				InitialiseTrophies();				//CD - Don't use this, auto setup after login
	void				RegisterAward(int iAwardNumber,int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected=false, 
										CXuiStringTable*pStringTable=NULL, int iTitleStr=-1, int iTextStr=-1, int iAcceptStr=-1, char *pszThemeName=NULL, unsigned int uiThemeSize=0L);
	int					GetAwardId(int iAwardNumber);
	eAwardType			GetAwardType(int iAwardNumber);
	bool				CanBeAwarded(int iQuadrant, int iAwardNumber);
	void				Award(int iQuadrant, int iAwardNumber, bool bForce=false);
	bool				IsAwardsFlagSet(int iQuadrant, int iAward);	
	void				Terminate();
	void				SetFatalTrophyErrorID(int id);		//CD - Deprecated	
	int					WaitTrophyInitComplete();			//CD - Deprecated
	int					tryWaitTrophyInitComplete();		//CD - Deprecated
	void				SetTrialTextStringTable(CXuiStringTable *pStringTable,int iAccept,int iReject);
	void				SetTrialAwardText(eAwardType AwardType,int iTitle,int iText); // achievement popup in the trial game
	void				SetHDDFreeKB(int iHDDFreeKB);
	void				SetMinSaveKB(int iMinSaveKB);
	int					GetHDDFreeKB(void);	
	bool				AreTrophiesInstalled();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// RICH PRESENCE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void				RichPresenceRegisterPresenceString(int index, const char* str);
	void				RichPresenceRegisterContext(int ctxID, const char* token);
	void				RichPresenceRegisterContextString(int ctxID, int strIndex, const char* str);
	void				RichPresenceInit(int iPresenceCount, int iContextCount);
	void				SetRichPresenceContextValue(int iPad,int iContextID, int iVal);
	void				SetCurrentGameActivity(int iPad,int iNewPresence, bool bSetOthersToIdle=false);
	void				SetRichPresenceSettingFn(int ( *SetPresenceInfoFn)(const void *data));

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PURCHASE
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void				DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam = -1);
	void				SetUpsellCallback(void ( *Func)(LPVOID lpParam, eUpsellType type, eUpsellResponse response, int iUserData),LPVOID lpParam);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Debug 
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void				SetDebugFullOverride(bool bVal); // To override the license version (trail/full). Only in debug/release, not ContentPackage
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Chat and content restrictions
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool				GetChatAndContentRestrictions(int iPad, bool thisQuadrantOnly,bool *pbChatRestricted,bool *pbContentRestricted,int *piAge);
	void				SetServiceID(char *pchServiceID); // needed for the ticket request for the chat restrictions of secondary PSN players
	void				HandleNetworkTicket(int result,void *arg);
	void				SetMinimumAge(int iAge, int iRegion);// 0 - SCEE, 1- SCEA, 2 - SCEJ
	int					GetMinimumAge();
	void				SetGermanyMinimumAge(int iAge);
	int					GetGermanyMinimumAge();
	void				SetRussiaMinimumAge(int iAge);
	int					GetRussiaMinimumAge();
	void				SetAustraliaMinimumAge(int iAge);
	int					GetAustraliaMinimumAge();
	void				SetJapanMinimumAge(int iAge);
	int					GetJapanMinimumAge();
	void				SetKoreaMinimumAge(int iAge);
	int					GetKoreaMinimumAge();
	int					getUserID(int iQuadrant); //  grab the PS4 userID for this quadrant (SCE_USER_SERVICE_USER_ID_INVALID if it's not signed in)

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Http calls
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool SonyHttp_init();
	void SonyHttp_shutdown();
	bool SonyHttp_getDataFromURL(const char* szURL, void** ppOutData, int* pDataSize);

};

// Singleton
extern C_4JProfile ProfileManager;

