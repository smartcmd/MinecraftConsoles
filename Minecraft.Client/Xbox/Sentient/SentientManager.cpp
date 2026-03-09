#include "stdafx.h"
#include "Include\SenClientCore.h"
#include "Include\SenClientMain.h"
#include "Include\SenClientMain.h"

#include "..\GameConfig\Minecraft.spa.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\Dimension.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.storage.h"

#include "SentientManager.h"
#include "MinecraftTelemetry.h"
#include "DynamicConfigurations.h"

// Global instance
CTelemetryManager *TelemetryManager = new CSentientManager();

HRESULT CSentientManager::Init()
{
	Sentient::SenSysTitleID sentitleID;
	sentitleID = static_cast<Sentient::SenSysTitleID>(TITLEID_MINECRAFT);

	HRESULT hr = SentientInitialize( sentitleID ); 

	m_lastHeartbeat = m_initialiseTime;

	m_bFirstFlush = true;

	return hr;
}

HRESULT CSentientManager::Tick()
{
	HRESULT hr = S_OK;

	// Update Sentient System   
	HRESULT sentientResult = Sentient::SentientUpdate();

	switch(sentientResult)
	{
	case S_OK:
		{
			// Sentient is connected 
			//OutputDebugString ("\nSentient: CONNECTED\n");
			if(g_NetworkManager.IsInSession())
			{
				float currentTime = app.getAppTime();
				if(currentTime - m_lastHeartbeat > 60)
				{
					m_lastHeartbeat = currentTime;
					for(DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
					{
						if(Minecraft::GetInstance()->localplayers[i] != nullptr)
						{
							SenStatHeartBeat(i, m_lastHeartbeat - m_initialiseTime);
						}
					}
				}
				
				if(m_bFirstFlush)
				{
					for(DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
					{
						if(Minecraft::GetInstance()->localplayers[i] != nullptr && m_fLevelStartTime[i] - currentTime > 60)
						{
							Flush();
						}
					}
				}
			}

			MinecraftDynamicConfigurations::Tick();
		}  
		break;

	case Sentient::SENTIENT_S_NOT_SIGNED_IN_TO_LIVE:
		{        
			// Login required
			//DebugPrintf("\nSentient: WARNING: an Xbox LIVE-enabled user needs to be logged in.\n");

			// Add title specific code here. . .
		}    
		break;

	case Sentient::SENTIENT_S_INITIALIZING_CONNECTION:
		{  
			// Server connection in progress 
			app.DebugPrintf("Sentient: Establishing connection to sentient server.\n");

			// Add title specific code here. . .
		}
		break;

	case Sentient::SENTIENT_S_SERVER_CONNECTION_FAILED:
		{       
			// Server connection failed
			app.DebugPrintf("\nSentient: WARNING: connection to sentient server failed.\n");

			// Add title specific code here. . .
		}	
		break;

	default:
		{
			// Unknown failure   
			app.DebugPrintf("Sentient: Unknown result from SentientUpdate()");

			// Add title specific code here. . .	
		}   
		break;
	}

	return hr;
}

HRESULT CSentientManager::Flush()
{
	m_bFirstFlush = false;
	return Sentient::SentientFlushStats();
}

BOOL CSentientManager::RecordPlayerSessionStart(DWORD dwUserId)
{
	return SenStatPlayerSessionStart( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetTitleBuildId(), 0, 0, 0, static_cast<INT>(app.getDeploymentType()) );
}

BOOL CSentientManager::RecordPlayerSessionExit(DWORD dwUserId, int _)
{
	return SenStatPlayerSessionExit( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId) );
}

BOOL CSentientManager::RecordHeartBeat(DWORD dwUserId)
{
	// Handled elswhere
	return FALSE;
}

BOOL CSentientManager::RecordLevelStart(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers)
{
	if(dwUserId == ProfileManager.GetPrimaryPad() ) m_bFirstFlush = true;

	++m_levelInstanceID;
	m_fLevelStartTime[dwUserId] = app.getAppTime();
	return SenStatLevelStart( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), GetSingleOrMultiplayer(), friendsOrMatch, competeOrCoop, GetDifficultyLevel(difficulty), numberOfLocalPlayers, numberOfOnlinePlayers, GetLicense(), GetDefaultGameControls(), GetAudioSettings(dwUserId), 0, 0 );
}

BOOL CSentientManager::RecordLevelExit(DWORD dwUserId, ESen_LevelExitStatus levelExitStatus)
{
	float levelDuration = app.getAppTime() - m_fLevelStartTime[dwUserId];
	return SenStatLevelExit( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), levelExitStatus, GetLevelExitProgressStat1(), GetLevelExitProgressStat2(), static_cast<INT>(levelDuration) );
}

BOOL CSentientManager::RecordLevelSaveOrCheckpoint(DWORD dwUserId, INT saveOrCheckPointID, INT saveSizeInBytes)
{
	float levelDuration = app.getAppTime() - m_fLevelStartTime[dwUserId];
	return SenStatLevelSaveOrCheckpoint( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), GetLevelExitProgressStat1(), GetLevelExitProgressStat2(), static_cast<INT>(levelDuration), saveOrCheckPointID, saveSizeInBytes );
}

BOOL CSentientManager::RecordLevelResume(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers, INT saveOrCheckPointID)
{
	return SenStatLevelResume( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), GetSingleOrMultiplayer(), friendsOrMatch, competeOrCoop, GetDifficultyLevel(difficulty), numberOfLocalPlayers, numberOfOnlinePlayers, GetLicense(), GetDefaultGameControls(), saveOrCheckPointID, GetAudioSettings(dwUserId), 0, 0 );
}


BOOL CSentientManager::RecordPauseOrInactive(DWORD dwUserId)
{
	return SenStatPauseOrInactive( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID() );
}

BOOL CSentientManager::RecordUnpauseOrActive(DWORD dwUserId)
{
	return SenStatUnpauseOrActive( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID() );
}

BOOL CSentientManager::RecordMenuShown(DWORD dwUserId, INT menuID, INT optionalMenuSubID)
{
	return SenStatMenuShown( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), menuID, optionalMenuSubID, GetLevelInstanceID(), GetMultiplayerInstanceID() );
}

BOOL CSentientManager::RecordAchievementUnlocked(DWORD dwUserId, INT achievementID, INT achievementGamerscore)
{
	return SenStatAchievementUnlocked( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), achievementID, achievementGamerscore );
}

BOOL CSentientManager::RecordMediaShareUpload(DWORD dwUserId, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType)
{
	return SenStatMediaShareUpload( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), mediaDestination, mediaType );
}

BOOL CSentientManager::RecordUpsellPresented(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID)
{
	return SenStatUpsellPresented( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), upsellId, marketplaceOfferID );
}

BOOL CSentientManager::RecordUpsellResponded(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID, ESen_UpsellOutcome upsellOutcome)
{
	return SenStatUpsellResponded( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), upsellId, marketplaceOfferID, upsellOutcome );
}

BOOL CSentientManager::RecordPlayerDiedOrFailed(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	INT secs = GetSecondsSinceInitialize();
	return SenStatPlayerDiedOrFailed( dwUserId, GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), lowResMapX, lowResMapY, lowResMapZ, mapID, playerWeaponID, enemyWeaponID, enemyTypeID, secs, secs );
}

BOOL CSentientManager::RecordEnemyKilledOrOvercome(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	INT secs = GetSecondsSinceInitialize();
	return SenStatEnemyKilledOrOvercome( dwUserId, GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), lowResMapX, lowResMapY, lowResMapZ, mapID, playerWeaponID, enemyWeaponID, enemyTypeID, secs, secs );
}

BOOL CSentientManager::RecordSkinChanged(DWORD dwUserId, DWORD dwSkinId)
{
	return SenStatSkinChanged( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), dwSkinId );
}

BOOL CSentientManager::RecordBanLevel(DWORD dwUserId)
{
	return SenStatBanLevel( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID() );
}

BOOL CSentientManager::RecordUnBanLevel(DWORD dwUserId)
{
	return SenStatUnBanLevel( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID() );
}

BOOL CSentientManager::RecordTexturePackLoaded(DWORD dwUserId, INT texturePackId, INT purchased)
{
	return SenStatTexturePackChanged( dwUserId, GetSecondsSinceInitialize(), GetMode(dwUserId), GetSubMode(dwUserId), GetLevelId(dwUserId), GetSubLevelId(dwUserId), GetLevelInstanceID(), GetMultiplayerInstanceID(), texturePackId, purchased );
}


/*
Number of seconds elapsed since Sentient initialize.
Title needs to track this and report it as a property.
These times will be used to create timelines and understand durations. 
This should be tracked independently of saved games (restoring a save should not reset the seconds since initialize)
*/
INT CSentientManager::GetSecondsSinceInitialize()
{
	return static_cast<INT>(app.getAppTime() - m_initialiseTime);
}

/*
An in-game setting that significantly differentiates the play style of the game.
(This should be captured as an integer and correspond to mode specific to the game.)
Teams will have to provide the game mappings that correspond to the integers.
The intent is to allow teams to capture data on the highest level categories of gameplay in their game.
For example, a game mode could be the name of the specific mini game (eg: golf vs darts) or a specific multiplayer mode (eg: hoard vs beast.) ModeID = 0 means undefined or unknown.
The intent is to answer the question "How are players playing your game?"
*/
INT CSentientManager::GetMode(DWORD dwUserId)
{
	INT mode = (INT)eTelem_ModeId_Undefined;
	
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localplayers[dwUserId] != nullptr && pMinecraft->localplayers[dwUserId]->level != nullptr && pMinecraft->localplayers[dwUserId]->level->getLevelData() != nullptr )
	{
		GameType *gameType = pMinecraft->localplayers[dwUserId]->level->getLevelData()->getGameType();

		if (gameType->isSurvival())
		{
			mode = static_cast<INT>(eTelem_ModeId_Survival);
		}
		else if (gameType->isCreative())
		{
			mode = static_cast<INT>(eTelem_ModeId_Creative);
		}
		else
		{
			mode = static_cast<INT>(eTelem_ModeId_Undefined);
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
INT CSentientManager::GetSubMode(DWORD dwUserId)
{
	INT subMode = (INT)eTelem_SubModeId_Undefined;

	if(Minecraft::GetInstance()->isTutorial())
	{
		subMode = static_cast<INT>(eTelem_SubModeId_Tutorial);
	}
	else
	{
		subMode = static_cast<INT>(eTelem_SubModeId_Normal);
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
INT CSentientManager::GetLevelId(DWORD dwUserId)
{
	INT levelId = (INT)eTelem_LevelId_Undefined;

	levelId = static_cast<INT>(eTelem_LevelId_PlayerGeneratedLevel);

	return levelId;
}

/*
Used when a title has more heirarchy required. OptionalSubLevel ID = 0 means undefined or unknown.
For titles that have sub-levels.
Level is always an indicator of "How far has the player progressed." so when possible these should be consecutive or at least monotonically increasing.
LevelIDs and SubLevelIDs can be reused as they will always be paired with a Mode/SubModeID
*/
INT CSentientManager::GetSubLevelId(DWORD dwUserId)
{
	INT subLevelId = (INT)eTelem_SubLevelId_Undefined;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if(pMinecraft->localplayers[dwUserId] != nullptr)
	{
		switch(pMinecraft->localplayers[dwUserId]->dimension)
		{
		case 0:
			subLevelId = static_cast<INT>(eTelem_SubLevelId_Overworld);
			break;
		case -1:
			subLevelId = static_cast<INT>(eTelem_SubLevelId_Nether);
			break;
		case 1:
			subLevelId = static_cast<INT>(eTelem_SubLevelId_End);
			break;
		};
	}

	return subLevelId;
}

/*
Build version of the title, used to track changes in development as well as patches/title updates
Allows developer to separate out stats from different builds
*/
INT CSentientManager::GetTitleBuildId()
{
	return (INT)VER_PRODUCTBUILD;
}

/*
Generated by the game every time LevelStart or LevelResume is called.
This should be a unique ID (can be sequential) within a session.
Helps differentiate level attempts when a play plays the same mode/level - especially with aggregated stats
*/
INT CSentientManager::GetLevelInstanceID()
{
	return static_cast<INT>(m_levelInstanceID);
}

/*
MultiplayerinstanceID is a title-generated value that is the same for all players in the same multiplayer session.
Link up players into a single multiplayer session ID.
*/
INT CSentientManager::GetMultiplayerInstanceID()
{
	return m_multiplayerInstanceID;
}

INT CSentientManager::GenerateMultiplayerInstanceId()
{
	FILETIME SystemTimeAsFileTime;
	GetSystemTimeAsFileTime( &SystemTimeAsFileTime );

	return *((INT *)&SystemTimeAsFileTime.dwLowDateTime);
}

void CSentientManager::SetMultiplayerInstanceId(INT value)
{
	m_multiplayerInstanceID = value;
}

/*
Indicates whether the game is being played in single or multiplayer mode and whether multiplayer is being played locally or over live.
How social is your game?  How do people play it?
*/
INT CSentientManager::GetSingleOrMultiplayer()
{
	INT singleOrMultiplayer = (INT)eSen_SingleOrMultiplayer_Undefined;

	// Unused
	//eSen_SingleOrMultiplayer_Single_Player
	//eSen_SingleOrMultiplayer_Multiplayer_Live

	if(app.GetLocalPlayerCount() == 1 && g_NetworkManager.GetOnlinePlayerCount() == 0)
	{
		singleOrMultiplayer = static_cast<INT>(eSen_SingleOrMultiplayer_Single_Player);
	}
	else if(app.GetLocalPlayerCount() > 1 && g_NetworkManager.GetOnlinePlayerCount() == 0)
	{
		singleOrMultiplayer = static_cast<INT>(eSen_SingleOrMultiplayer_Multiplayer_Local);
	}
	else if(app.GetLocalPlayerCount() == 1 && g_NetworkManager.GetOnlinePlayerCount() > 0)
	{
		singleOrMultiplayer = static_cast<INT>(eSen_SingleOrMultiplayer_Multiplayer_Live);
	}
	else if(app.GetLocalPlayerCount() > 1 && g_NetworkManager.GetOnlinePlayerCount() > 0)
	{
		singleOrMultiplayer = static_cast<INT>(eSen_SingleOrMultiplayer_Multiplayer_Both_Local_and_Live);
	}

	return singleOrMultiplayer;
}

/*
An in-game setting that differentiates the challenge imposed on the user.
Normalized to a standard 5-point scale.	Are players changing the difficulty?
*/
INT CSentientManager::GetDifficultyLevel(INT diff)
{
	INT difficultyLevel = (INT)eSen_DifficultyLevel_Undefined;

	switch(diff)
	{
	case 0:
		difficultyLevel = static_cast<INT>(eSen_DifficultyLevel_Easiest);
		break;
	case 1:
		difficultyLevel = static_cast<INT>(eSen_DifficultyLevel_Easier);
		break;
	case 2:
		difficultyLevel = static_cast<INT>(eSen_DifficultyLevel_Normal);
		break;
	case 3:
		difficultyLevel = static_cast<INT>(eSen_DifficultyLevel_Harder);
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
INT CSentientManager::GetLicense()
{
	INT license = eSen_License_Undefined;

	if(ProfileManager.IsFullVersion())
	{
		license = static_cast<INT>(eSen_License_Full_Purchased_Title);
	}
	else
	{
		license = static_cast<INT>(eSen_License_Trial_or_Demo);
	}
	return license;
}

/*
This is intended to capture whether players played using default control scheme or customized the control scheme.
Are players customizing your controls?
*/
INT CSentientManager::GetDefaultGameControls()
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
INT CSentientManager::GetAudioSettings(DWORD dwUserId)
{
	INT audioSettings = (INT)eSen_AudioSettings_Undefined;

	if(dwUserId == ProfileManager.GetPrimaryPad())
	{
		BYTE volume = app.GetGameSettings(dwUserId,eGameSetting_SoundFXVolume);

		if(volume == 0)
		{
			audioSettings = static_cast<INT>(eSen_AudioSettings_Off);
		}
		else if(volume == DEFAULT_VOLUME_LEVEL)
		{
			audioSettings = static_cast<INT>(eSen_AudioSettings_On_Default);
		}
		else
		{
			audioSettings = static_cast<INT>(eSen_AudioSettings_On_CustomSetting);
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
INT CSentientManager::GetLevelExitProgressStat1()
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
INT CSentientManager::GetLevelExitProgressStat2()
{
	// 4J Stu - Unused
	return 0;
}