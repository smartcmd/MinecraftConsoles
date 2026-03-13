/************************************************************************/
/* THIS FILE WAS AUTOMATICALLY GENERATED                                */
/* PLEASE DO NOT MODIFY                                                 */
/************************************************************************/
// Generated from Version: 24, on (9/4/2013 8:47:23 AM)

#include <xtl.h>

#include "SenClientStats.h"

using namespace Sentient;

/************************************************************************/
/* STATS                                                                */
/************************************************************************/

// PlayerSessionStart
BOOL	SenStatPlayerSessionStart              	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT TitleBuildID, INT SkeletonDistanceInInches, INT EnrollmentType, INT NumberOfSkeletonsInView, INT DeploymentType )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	TitleBuildID;
		INT	SkeletonDistanceInInches;
		INT	EnrollmentType;
		INT	NumberOfSkeletonsInView;
		INT	DeploymentType;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, TitleBuildID, SkeletonDistanceInInches, EnrollmentType, NumberOfSkeletonsInView, DeploymentType };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 128;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 10;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("PlayerSessionStart", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// PlayerSessionExit
BOOL	SenStatPlayerSessionExit               	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 129;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 5;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("PlayerSessionExit", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// HeartBeat
BOOL	SenStatHeartBeat                       	( DWORD dwUserID, INT SecondsSinceInitialize )
{
	struct
	{
		INT	SecondsSinceInitialize;
	} LocalStruct = { SecondsSinceInitialize };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 130;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 1;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("HeartBeat", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// LevelStart
BOOL	SenStatLevelStart                      	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT SingleOrMultiplayer, INT FriendsOrMatch, INT CompeteOrCoop, INT DifficultyLevel, INT NumberOfLocalPlayers, INT NumberOfOnlinePlayers, INT License, INT DefaultGameControls, INT AudioSettings, INT SkeletonDistanceInInches, INT NumberOfSkeletonsInView )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	SingleOrMultiplayer;
		INT	FriendsOrMatch;
		INT	CompeteOrCoop;
		INT	DifficultyLevel;
		INT	NumberOfLocalPlayers;
		INT	NumberOfOnlinePlayers;
		INT	License;
		INT	DefaultGameControls;
		INT	AudioSettings;
		INT	SkeletonDistanceInInches;
		INT	NumberOfSkeletonsInView;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, SingleOrMultiplayer, FriendsOrMatch, CompeteOrCoop, DifficultyLevel, NumberOfLocalPlayers, NumberOfOnlinePlayers, License, DefaultGameControls, AudioSettings, SkeletonDistanceInInches, NumberOfSkeletonsInView };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 131;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 18;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("LevelStart", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// LevelExit
BOOL	SenStatLevelExit                       	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT LevelExitStatus, INT LevelExitProgressStat1, INT LevelExitProgressStat2, INT LevelDurationInSeconds )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	LevelExitStatus;
		INT	LevelExitProgressStat1;
		INT	LevelExitProgressStat2;
		INT	LevelDurationInSeconds;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, LevelExitStatus, LevelExitProgressStat1, LevelExitProgressStat2, LevelDurationInSeconds };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 132;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 11;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("LevelExit", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// LevelSaveOrCheckpoint
BOOL	SenStatLevelSaveOrCheckpoint           	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT LevelExitProgressStat1, INT LevelExitProgressStat2, INT LevelDurationInSeconds, INT SaveOrCheckPointID, INT SaveSizeInBytes )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	LevelExitProgressStat1;
		INT	LevelExitProgressStat2;
		INT	LevelDurationInSeconds;
		INT	SaveOrCheckPointID;
		INT	SaveSizeInBytes;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, LevelExitProgressStat1, LevelExitProgressStat2, LevelDurationInSeconds, SaveOrCheckPointID, SaveSizeInBytes };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 133;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 12;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("LevelSaveOrCheckpoint", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// LevelResume
BOOL	SenStatLevelResume                     	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT SingleOrMultiplayer, INT FriendsOrMatch, INT CompeteOrCoop, INT DifficultyLevel, INT NumberOfLocalPlayers, INT NumberOfOnlinePlayers, INT License, INT DefaultGameControls, INT SaveOrCheckPointID, INT AudioSettings, INT SkeletonDistanceInInches, INT NumberOfSkeletonsInView )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	SingleOrMultiplayer;
		INT	FriendsOrMatch;
		INT	CompeteOrCoop;
		INT	DifficultyLevel;
		INT	NumberOfLocalPlayers;
		INT	NumberOfOnlinePlayers;
		INT	License;
		INT	DefaultGameControls;
		INT	SaveOrCheckPointID;
		INT	AudioSettings;
		INT	SkeletonDistanceInInches;
		INT	NumberOfSkeletonsInView;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, SingleOrMultiplayer, FriendsOrMatch, CompeteOrCoop, DifficultyLevel, NumberOfLocalPlayers, NumberOfOnlinePlayers, License, DefaultGameControls, SaveOrCheckPointID, AudioSettings, SkeletonDistanceInInches, NumberOfSkeletonsInView };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 134;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 19;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("LevelResume", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// PauseOrInactive
BOOL	SenStatPauseOrInactive                 	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 135;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 7;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("PauseOrInactive", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// UnpauseOrActive
BOOL	SenStatUnpauseOrActive                 	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 136;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 7;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("UnpauseOrActive", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// MenuShown
BOOL	SenStatMenuShown                       	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT MenuID, INT OptionalMenuSubID, INT LevelInstanceID, INT MultiplayerInstanceID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	MenuID;
		INT	OptionalMenuSubID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, MenuID, OptionalMenuSubID, LevelInstanceID, MultiplayerInstanceID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 137;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 9;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("MenuShown", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// AchievementUnlocked
BOOL	SenStatAchievementUnlocked             	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT AchievementID, INT AchievementGamerscore )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	AchievementID;
		INT	AchievementGamerscore;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, AchievementID, AchievementGamerscore };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 138;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 9;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("AchievementUnlocked", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// MediaShareUpload
BOOL	SenStatMediaShareUpload                	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT MediaDestination, INT MediaType )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	MediaDestination;
		INT	MediaType;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, MediaDestination, MediaType };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 139;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 9;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("MediaShareUpload", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// UpsellPresented
BOOL	SenStatUpsellPresented                 	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT UpsellID, INT MarketplaceOfferID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	UpsellID;
		INT	MarketplaceOfferID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, UpsellID, MarketplaceOfferID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 140;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 9;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("UpsellPresented", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// UpsellResponded
BOOL	SenStatUpsellResponded                 	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT UpsellID, INT MarketplaceOfferID, INT UpsellOutcome )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	UpsellID;
		INT	MarketplaceOfferID;
		INT	UpsellOutcome;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, UpsellID, MarketplaceOfferID, UpsellOutcome };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 141;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 10;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("UpsellResponded", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// PlayerDiedOrFailed
BOOL	SenStatPlayerDiedOrFailed              	( DWORD dwUserID, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT LowResMapX, INT LowResMapY, INT LowResMapZ, INT MapID, INT PlayerWeaponID, INT EnemyWeaponID, INT EnemyTypeID, INT SecondsSinceInitialize, INT CopyOfSecondsSinceInitialize )
{
	struct
	{
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	LowResMapX;
		INT	LowResMapY;
		INT	LowResMapZ;
		INT	MapID;
		INT	PlayerWeaponID;
		INT	EnemyWeaponID;
		INT	EnemyTypeID;
		INT	SecondsSinceInitialize;
		INT	CopyOfSecondsSinceInitialize;
		INT	Count;
	} LocalStruct = { ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, LowResMapX, LowResMapY, LowResMapZ, MapID, PlayerWeaponID, EnemyWeaponID, EnemyTypeID, SecondsSinceInitialize, CopyOfSecondsSinceInitialize, 1 };
	DWORD arrValueFlags[] = { SenStatValueFlag_Min, SenStatValueFlag_Max, SenStatValueFlag_Inc };

	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 142;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 13;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 3;
	st.arrValues       = (CHAR*)&LocalStruct.SecondsSinceInitialize;
	st.arrValueFlags   = arrValueFlags;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("PlayerDiedOrFailed", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// EnemyKilledOrOvercome
BOOL	SenStatEnemyKilledOrOvercome           	( DWORD dwUserID, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT LowResMapX, INT LowResMapY, INT LowResMapZ, INT MapID, INT PlayerWeaponID, INT EnemyWeaponID, INT EnemyTypeID, INT SecondsSinceInitialize, INT CopyOfSecondsSinceInitialize )
{
	struct
	{
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	LowResMapX;
		INT	LowResMapY;
		INT	LowResMapZ;
		INT	MapID;
		INT	PlayerWeaponID;
		INT	EnemyWeaponID;
		INT	EnemyTypeID;
		INT	SecondsSinceInitialize;
		INT	CopyOfSecondsSinceInitialize;
		INT	Count;
	} LocalStruct = { ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, LowResMapX, LowResMapY, LowResMapZ, MapID, PlayerWeaponID, EnemyWeaponID, EnemyTypeID, SecondsSinceInitialize, CopyOfSecondsSinceInitialize, 1 };
	DWORD arrValueFlags[] = { SenStatValueFlag_Min, SenStatValueFlag_Max, SenStatValueFlag_Inc };

	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 143;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 13;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 3;
	st.arrValues       = (CHAR*)&LocalStruct.SecondsSinceInitialize;
	st.arrValueFlags   = arrValueFlags;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("EnemyKilledOrOvercome", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// SkinChanged
BOOL	SenStatSkinChanged                     	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT SkinID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	SkinID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, SkinID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 144;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 8;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("SkinChanged", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// BanLevel
BOOL	SenStatBanLevel                        	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 145;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 7;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("BanLevel", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// UnBanLevel
BOOL	SenStatUnBanLevel                      	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 146;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 7;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("UnBanLevel", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

// TexturePackChanged
BOOL	SenStatTexturePackChanged              	( DWORD dwUserID, INT SecondsSinceInitialize, INT ModeID, INT OptionalSubModeID, INT LevelID, INT OptionalSubLevelID, INT LevelInstanceID, INT MultiplayerInstanceID, INT TexturePackId, INT Purchased )
{
	struct
	{
		INT	SecondsSinceInitialize;
		INT	ModeID;
		INT	OptionalSubModeID;
		INT	LevelID;
		INT	OptionalSubLevelID;
		INT	LevelInstanceID;
		INT	MultiplayerInstanceID;
		INT	TexturePackId;
		INT	Purchased;
	} LocalStruct = { SecondsSinceInitialize, ModeID, OptionalSubModeID, LevelID, OptionalSubLevelID, LevelInstanceID, MultiplayerInstanceID, TexturePackId, Purchased };
	SenStat      st;
	st.dwUserID        = dwUserID;
	st.dwStatID        = 147;
	st.dwFlags         = SenStatFlag_Normal;
	st.dwNumProperties = 9;
	st.arrProperties   = (CHAR*)&LocalStruct;
	st.dwNumValues     = 0;
	st.arrValues       = nullptr;
	st.arrValueFlags   = nullptr;

#ifdef SEN_LOGTELEMETRY
	// if we're in debug build with logging then log the stat to a file for testing
	SentientDebugLogStatSend("TexturePackChanged", &st );
#endif


	return Sentient::SentientStatsSend( &st ) == S_OK;
}

namespace Sentient
{

	extern int statsVersion;

	class StatsVersionClass {
		public: StatsVersionClass() { statsVersion = 24; }
	};

	static StatsVersionClass versionClass;
}

