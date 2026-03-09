#pragma once

namespace WXAMS = Windows::Xbox::ApplicationModel::Store;
namespace WXS = Windows::Xbox::System;
namespace MXS = Microsoft::Xbox::Services;

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

// Public version of PlayerUID that hides any C++/Cx functions
class PlayerUID
{
protected:
	std::wstring m_UID;

public:
	class Hash
	{
	public:		
		std::size_t operator()(PlayerUID const& k) const;
	};

	PlayerUID();
	PlayerUID(std::wstring fromString);

	bool operator==(const PlayerUID &rhs) const;
	bool operator!=(const PlayerUID &rhs);

	std::wstring toString() const;
};
typedef PlayerUID *PPlayerUID;

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
	void				Initialise( const std::wstring &serviceConfigId, const std::wstring &titleProductId ); 
	//void				SetTrialTextStringTable(CXuiStringTable *pStringTable,int iAccept,int iReject);
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
	void				GetXUID(int iPad, PlayerUID *pXuid,bool bOnlineXuid);
	BOOL				AreXUIDSEqual(PlayerUID xuid1,PlayerUID xuid2);
	BOOL				XUIDIsGuest(PlayerUID xuid);
	bool				AllowedToPlayMultiplayer(int iProf);
	void				StartTrialGame(); // disables saves and leaderboard, and change state to readyforgame from pregame
	void				AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL *allAllowed, BOOL *friendsAllowed);
	BOOL				CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pXuids, DWORD dwXuidCount );
	bool				GetProfileAvatar(int iPad,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam);
	void				CancelProfileAvatarRequest();
	void				ShowProfileCard(int iPad, PlayerUID targetUid);
	void				ShowAddFriend(int iPad, PlayerUID targetUid);

	void				CheckPrivilege(int iPad, bool thisQuadrantOnly, WXAMS::KnownPrivileges privilege, void( *Func)(LPVOID, bool, int),LPVOID lpParam);
	void				CheckPrivileges(int iPad, bool thisQuadrantOnly, const std::vector<WXAMS::KnownPrivileges> &privileges, void( *Func)(LPVOID, bool, int),LPVOID lpParam);

	// Some helper functions to wrap common combinations of privileges
	void				CheckMultiplayerPrivileges(int iPad, bool thisQuadrantOnly, void( *Func)(LPVOID, bool, int),LPVOID lpParam);

				
	// SYS
	WXS::User^			GetUser(int iPad, bool incUsersSigningOut = false);
	int					GetPrimaryPad();
	void				SetPrimaryPad(int iPad);
	int					AddGamepadToGame(int iPad);
	void				RemoveGamepadFromGame(int iPad);
	void				ClearGameUsers();
	char*				GetGamertag(int iPad);
	std::wstring		GetDisplayName(int iPad);
	bool				IsFullVersion();
	void				SetSignInChangeCallback(void ( *Func)(LPVOID, bool, unsigned int),LPVOID lpParam);
	void				SetNotificationsCallback(void ( *Func)(LPVOID, DWORD, unsigned int),LPVOID lpParam);
	bool				RegionIsNorthAmerica(void);
	bool				LocaleIsUSorCanada(void);
	HRESULT				GetLiveConnectionStatus();
	bool				IsSystemUIDisplayed();
	void				SetProfileReadErrorCallback(void ( *Func)(LPVOID), LPVOID lpParam);
	void				CompleteDeferredSignouts();
	void				SetDeferredSignoutEnabled(bool enabled);

	// PROFILE DATA
	//int					SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam);
	//int					SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam);
	//PROFILESETTINGS *	GetDashboardProfileSettings(int iPad);
	//void				WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged=false, bool bOverride5MinuteLimitOnProfileWrites=false);
	//void				ForceQueuedProfileWrites(int iPad=XUSER_INDEX_ANY);
	//void				*GetGameDefinedProfileData(int iQuadrant);
	void				ResetProfileProcessState(); // after a sign out from the primary player, call this
	void				Tick( void );

	void GetProfile(PlayerUID xuid, void (*func)(LPVOID, Microsoft::Xbox::Services::Social::XboxUserProfile^), LPVOID param);
	void GetProfiles(std::vector<PlayerUID> xuids, void (*func)(LPVOID, std::vector<Microsoft::Xbox::Services::Social::XboxUserProfile^>), LPVOID param);

	// ACHIEVEMENTS & AWARDS

	//void				RegisterAward(int iAwardNumber,int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected=false, 
	//									CXuiStringTable*pStringTable=nullptr, int iTitleStr=-1, int iTextStr=-1, int iAcceptStr=-1, char *pszThemeName=nullptr, unsigned int uiThemeSize=0L);
	//int					GetAwardId(int iAwardNumber);
	eAwardType			GetAwardType(int iAwardNumber);
	bool				CanBeAwarded(int iQuadrant, int iAwardNumber);
	void				Award(int iQuadrant, int iAwardNumber, bool bForce=false);
	//bool				IsAwardsFlagSet(int iQuadrant, int iAward);	

	// RICH PRESENCE

	//void				RichPresenceInit(int iPresenceCount, int iContextCount);
	//void				RegisterRichPresenceContext(int iGameConfigContextID);
	//void				SetRichPresenceContextValue(int iPad,int iContextID, int iVal);
	void				RegisterPresence(int presenceId, const std::wstring &presence);
	void				SetCurrentGameActivity(int iPad,int iNewPresence, bool bSetOthersToIdle=false);
	void				SetGameActivityForAllActiveUsers(int iNewPresence);

	// PURCHASE
	void				DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam = -1);
	void				SetUpsellCallback(void ( *Func)(LPVOID lpParam, eUpsellType type, eUpsellResponse response, int iUserData),LPVOID lpParam);

	// Debug 
	void				SetDebugFullOverride(bool bVal); // To override the license version (trail/full). Only in debug/release, not ContentPackage

};

// Singleton
extern C_4JProfile ProfileManager;

