#include "stdafx.h"

#include "MultiPlayerLocalPlayer.h"

#include "..\Minecraft.World\LevelSettings.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\Level.h"

#include "TelemetryManager.h"

#if !defined(_DURANGO) && !defined(_XBOX)

CTelemetryManager *TelemetryManager = new CTelemetryManager();

#endif

HRESULT CTelemetryManager::Init()
{
	return S_OK;
}

HRESULT CTelemetryManager::Tick()
{
	return S_OK;
}

HRESULT CTelemetryManager::Flush()
{
	return S_OK;
}

bool CTelemetryManager::RecordPlayerSessionStart(int iPad)
{
	return true;
}

bool CTelemetryManager::RecordPlayerSessionExit(int iPad, int exitStatus)
{
	return true;
}

bool CTelemetryManager::RecordHeartBeat(int iPad)
{
	return true;
}

bool CTelemetryManager::RecordLevelStart(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers)
{
	if(iPad == ProfileManager.GetPrimaryPad() ) m_bFirstFlush = true;

	++m_levelInstanceID;
	m_fLevelStartTime[iPad] = app.getAppTime();

	return true;
}

bool CTelemetryManager::RecordLevelExit(int iPad, ESen_LevelExitStatus levelExitStatus)
{
	return true;
}

bool CTelemetryManager::RecordLevelSaveOrCheckpoint(int iPad, int saveOrCheckPointID, int saveSizeInBytes)
{
	return true;
}

bool CTelemetryManager::RecordLevelResume(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers, int saveOrCheckPointID)
{
	return true;
}

bool CTelemetryManager::RecordPauseOrInactive(int iPad)
{
	return true;
}

bool CTelemetryManager::RecordUnpauseOrActive(int iPad)
{
	return true;
}

bool CTelemetryManager::RecordMenuShown(int iPad, EUIScene menuID, int optionalMenuSubID)
{
	return true;
}

bool CTelemetryManager::RecordAchievementUnlocked(int iPad, int achievementID, int achievementGamerscore)
{
	return true;
}

bool CTelemetryManager::RecordMediaShareUpload(int iPad, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType)
{
	return true;
}

bool CTelemetryManager::RecordUpsellPresented(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID)
{
	return true;
}

bool CTelemetryManager::RecordUpsellResponded(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID, ESen_UpsellOutcome upsellOutcome)
{
	return true;
}

bool CTelemetryManager::RecordPlayerDiedOrFailed(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	return true;
}

bool CTelemetryManager::RecordEnemyKilledOrOvercome(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	return true;
}

bool CTelemetryManager::RecordTexturePackLoaded(int iPad, int texturePackId, bool purchased)
{
	return true;
}

bool CTelemetryManager::RecordSkinChanged(int iPad, int dwSkinId)
{
	return true;
}

bool CTelemetryManager::RecordBanLevel(int iPad)
{
	return true;
}

bool CTelemetryManager::RecordUnBanLevel(int iPad)
{
	return true;
}


	 ///////////////////////////////////////////////////////////////////
	// 4J-JEV: FOLLOWING LOGIC TAKEN FROM XBOX 'SentientManager.cpp' //
   ///////////////////////////////////////////////////////////////////


/*
Number of seconds elapsed since Sentient initialize.
Title needs to track this and report it as a property.
These times will be used to create timelines and understand durations. 
This should be tracked independently of saved games (restoring a save should not reset the seconds since initialize)
*/
INT CTelemetryManager::GetSecondsSinceInitialize()
{
	return (INT)(app.getAppTime() - m_initialiseTime);
}

/*
An in-game setting that significantly differentiates the play style of the game.
(This should be captured as an integer and correspond to mode specific to the game.)
Teams will have to provide the game mappings that correspond to the integers.
The intent is to allow teams to capture data on the highest level categories of gameplay in their game.
For example, a game mode could be the name of the specific mini game (eg: golf vs darts) or a specific multiplayer mode (eg: hoard vs beast.) ModeID = 0 means undefined or unknown.
The intent is to answer the question "How are players playing your game?"
*/
INT CTelemetryManager::GetMode(DWORD dwUserId)
{
	INT mode = (INT)eTelem_ModeId_Undefined;
	
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localplayers[dwUserId] != NULL && pMinecraft->localplayers[dwUserId]->level != NULL && pMinecraft->localplayers[dwUserId]->level->getLevelData() != NULL )
	{
		GameType *gameType = pMinecraft->localplayers[dwUserId]->level->getLevelData()->getGameType();

		if (gameType->isSurvival())
		{
			mode = (INT)eTelem_ModeId_Survival;
		}
		else if (gameType->isCreative())
		{
			mode = (INT)eTelem_ModeId_Creative;
		}
		else
		{
			mode = (INT)eTelem_ModeId_Undefined;
		}
	}
	return mode;
}

/*
Used when a title has more heirarchy required.
OptionalSubMode ID = 0 means undefined or unknown.
For titles that have sub-modes (Sports/Football).
Mode is always an indicator of "How is the player choosing to play my game?" so these do not have to be consecutive.
LevelIDs and SubLevelIDs can be reused as they will always be paired with a Mode/SubModeID, Mode should be unique - SubMode can be shared between modes.
*/
INT CTelemetryManager::GetSubMode(DWORD dwUserId)
{
	INT subMode = (INT)eTelem_SubModeId_Undefined;

	if(Minecraft::GetInstance()->isTutorial())
	{
		subMode = (INT)eTelem_SubModeId_Tutorial;
	}
	else
	{
		subMode = (INT)eTelem_SubModeId_Normal;
	}

	return subMode;
}

/*
This is a more granular view of mode, allowing teams to get a sense of the levels or maps players are playing and providing some insight into how players progress through a game.
Teams will have to provide the game mappings that correspond to the integers.
The intent is that a level is highest level at which modes can be dissected and provides an indication of player progression in a game.
The intent is that level start and ends do not occur more than every 2 minutes or so, otherwise the data reported will be difficult to understand.
Levels are unique only within a given modeID - so you can have a ModeID =1, LevelID =1 and a different ModeID=2, LevelID = 1 indicate two completely different levels.
LevelID = 0 means undefined or unknown.
*/
INT CTelemetryManager::GetLevelId(DWORD dwUserId)
{
	INT levelId = (INT)eTelem_LevelId_Undefined;

	levelId = (INT)eTelem_LevelId_PlayerGeneratedLevel;

	return levelId;
}

/*
Used when a title has more heirarchy required. OptionalSubLevel ID = 0 means undefined or unknown.
For titles that have sub-levels.
Level is always an indicator of "How far has the player progressed." so when possible these should be consecutive or at least monotonically increasing.
LevelIDs and SubLevelIDs can be reused as they will always be paired with a Mode/SubModeID
*/
INT CTelemetryManager::GetSubLevelId(DWORD dwUserId)
{
	INT subLevelId = (INT)eTelem_SubLevelId_Undefined;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if(pMinecraft->localplayers[dwUserId] != NULL)	
	{
		switch(pMinecraft->localplayers[dwUserId]->dimension)
		{
		case 0:
			subLevelId = (INT)eTelem_SubLevelId_Overworld;
			break;
		case -1:
			subLevelId = (INT)eTelem_SubLevelId_Nether;
			break;
		case 1:
			subLevelId = (INT)eTelem_SubLevelId_End;
			break;
		};
	}

	return subLevelId;
}

/*
Build version of the title, used to track changes in development as well as patches/title updates
Allows developer to separate out stats from different builds
*/
INT CTelemetryManager::GetTitleBuildId()
{
	return (INT)VER_PRODUCTBUILD;
}

/*
Generated by the game every time LevelStart or LevelResume is called.
This should be a unique ID (can be sequential) within a session.
Helps differentiate level attempts when a play plays the same mode/level - especially with aggregated stats
*/
INT CTelemetryManager::GetLevelInstanceID()
{
	return (INT)m_levelInstanceID;
}

/*
MultiplayerinstanceID is a title-generated value that is the same for all players in the same multiplayer session.
Link up players into a single multiplayer session ID.
*/
INT CTelemetryManager::GetMultiplayerInstanceID()
{
	return m_multiplayerInstanceID;
}

INT CTelemetryManager::GenerateMultiplayerInstanceId()
{
#if defined(_DURANGO) || defined(_XBOX)
	FILETIME SystemTimeAsFileTime;
	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );
	return *((INT *)&SystemTimeAsFileTime.dwLowDateTime);
#else
	return 0;
#endif
}

void CTelemetryManager::SetMultiplayerInstanceId(INT value)
{
	m_multiplayerInstanceID = value;
}

/*
Indicates whether the game is being played in single or multiplayer mode and whether multiplayer is being played locally or over live.
How social is your game?  How do people play it?
*/
INT CTelemetryManager::GetSingleOrMultiplayer()
{
	INT singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Undefined;

	// Unused
	//eSen_SingleOrMultiplayer_Single_Player
	//eSen_SingleOrMultiplayer_Multiplayer_Live

	if(app.GetLocalPlayerCount() == 1 && g_NetworkManager.GetOnlinePlayerCount() == 0)
	{
		singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Single_Player;
	}
	else if(app.GetLocalPlayerCount() > 1 && g_NetworkManager.GetOnlinePlayerCount() == 0)
	{
		singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Multiplayer_Local;
	}
	else if(app.GetLocalPlayerCount() == 1 && g_NetworkManager.GetOnlinePlayerCount() > 0)
	{
		singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Multiplayer_Live;
	}
	else if(app.GetLocalPlayerCount() > 1 && g_NetworkManager.GetOnlinePlayerCount() > 0)
	{
		singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Multiplayer_Both_Local_and_Live;
	}

	return singleOrMultiplayer;
}

/*
An in-game setting that differentiates the challenge imposed on the user.
Normalized to a standard 5-point scale.	Are players changing the difficulty?
*/
INT CTelemetryManager::GetDifficultyLevel(INT diff)
{
	INT difficultyLevel = (INT)eSen_DifficultyLevel_Undefined;

	switch(diff)
	{
	case 0:
		difficultyLevel = (INT)eSen_DifficultyLevel_Easiest;
		break;
	case 1:
		difficultyLevel = (INT)eSen_DifficultyLevel_Easier;
		break;
	case 2:
		difficultyLevel = (INT)eSen_DifficultyLevel_Normal;
		break;
	case 3:
		difficultyLevel = (INT)eSen_DifficultyLevel_Harder;
		break;
	}

	// Unused
	//eSen_DifficultyLevel_Hardest = 5,

	return difficultyLevel;
}

/*
Differentiates trial/demo from full purchased titles
Is this a full title or demo?
*/
INT CTelemetryManager::GetLicense()
{
	INT license = eSen_License_Undefined;

	if(ProfileManager.IsFullVersion())
	{
		license = (INT)eSen_License_Full_Purchased_Title;
	}
	else
	{
		license = (INT)eSen_License_Trial_or_Demo;
	}
	return license;
}

/*
This is intended to capture whether players played using default control scheme or customized the control scheme.
Are players customizing your controls?
*/
INT CTelemetryManager::GetDefaultGameControls()
{
	INT defaultGameControls = eSen_DefaultGameControls_Undefined;

	// Unused	
	//eSen_DefaultGameControls_Custom_controls

	defaultGameControls = eSen_DefaultGameControls_Default_controls;

	return defaultGameControls;
}

/*
Are players changing default audio settings?
This is intended to capture whether players are playing with or without volume and whether they make changes from the default audio settings.
*/
INT CTelemetryManager::GetAudioSettings(DWORD dwUserId)
{
	INT audioSettings = (INT)eSen_AudioSettings_Undefined;

	if(dwUserId == ProfileManager.GetPrimaryPad())
	{
		BYTE volume = app.GetGameSettings(dwUserId,eGameSetting_SoundFXVolume);

		if(volume == 0)
		{
			audioSettings = (INT)eSen_AudioSettings_Off;
		}
		else if(volume == DEFAULT_VOLUME_LEVEL)
		{
			audioSettings = (INT)eSen_AudioSettings_On_Default;
		}
		else
		{
			audioSettings = (INT)eSen_AudioSettings_On_CustomSetting;
		}
	}
	return audioSettings;
}

/*
Refers to the highest level performance metric for your game.
For example, a performance metric could points earned, race time, total kills, etc.
This is entirely up to you and will help us understand how well the player performed, or how far the player progressed  in the level before exiting.
How far did users progress before failing/exiting the level?
*/
INT CTelemetryManager::GetLevelExitProgressStat1()
{
	// 4J Stu - Unused
	return 0;
}

/*
Refers to the highest level performance metric for your game.
For example, a performance metric could points earned, race time, total kills, etc.
This is entirely up to you and will help us understand how well the player performed, or how far the player progressed  in the level before exiting.
How far did users progress before failing/exiting the level?
*/
INT CTelemetryManager::GetLevelExitProgressStat2()
{
	// 4J Stu - Unused
	return 0;
}