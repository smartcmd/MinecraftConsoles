#include "stdafx.h"

#include "..\Minecraft.World\StringHelpers.h"

#include "..\Minecraft.World\Player.h"
#include "..\Minecraft.World\Level.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\LevelSettings.h"

#include "Common\Network\GameNetworkManager.h"
#include "MultiPlayerLocalPlayer.h"

#include "..\ServiceConfig\Events-XBLA.8-149E11AEEvents.h"

#include "DurangoTelemetry.h"

CTelemetryManager *TelemetryManager = new CDurangoTelemetryManager();

// Empty GUID
GUID CDurangoTelemetryManager::ZERO_GUID = GUID();

/*
HRESULT CDurangoTelemetryManager::Init()
{
	app.DebugPrintf("[CDurangoTelemetryManager] Init().\n");

	m_initialiseTime = app.getAppTime();

	return S_OK;
}
*/

CDurangoTelemetryManager::CDurangoTelemetryManager()
{
	m_initialiseTime = app.getAppTime();
}

/*
HRESULT CDurangoTelemetryManager::Tick()
{
	app.DebugPrintf("[CDurangoTelemetryManager] Tick().\n");
	return S_OK;
}

HRESULT CDurangoTelemetryManager::Flush()
{
	app.DebugPrintf("[CDurangoTelemetryManager] Flush().\n");
	return S_OK;
} */

bool CDurangoTelemetryManager::RecordPlayerSessionStart(int iPad)
{
	durangoStats()->generatePlayerSession();
	
	return true;
}

bool CDurangoTelemetryManager::RecordPlayerSessionExit(int iPad, int exitStatus)
{
	PlayerUID puid; shared_ptr<Player> plr;
	ProfileManager.GetXUID(iPad, &puid, true);
	plr = Minecraft::GetInstance()->localplayers[iPad];

	// 4J-JEV: Still needed to flush cached travel stats.
	DurangoStats::playerSessionEnd(iPad);

	if (plr != NULL && plr->level != NULL && plr->level->getLevelData() != NULL)
	{
		ULONG hr = EventWritePlayerSessionEnd(
			DurangoStats::getUserId(iPad),
			DurangoStats::getPlayerSession(),
			DurangoStats::getMultiplayerCorrelationId(),
			plr->level->getLevelData()->getGameType()->isSurvival(),
			plr->level->difficulty,
			exitStatus);

		if (hr == 0) // Debug.
		{
			app.DebugPrintf("<%ls> PlayerSessionEnd(%ls,%ls,%i,%i,%i)\n",
				DurangoStats::getUserId(iPad),
				guid2str(DurangoStats::getPlayerSession()).c_str(),
				DurangoStats::getMultiplayerCorrelationId(),
				plr->level->getLevelData()->getGameType()->isSurvival(),
				plr->level->difficulty,
				exitStatus);
		}

		if ( !g_NetworkManager.IsLocalGame() )
		{
			float roundLength = app.getAppTime() - m_multiplayerRoundStartTimes[DurangoStats::getUserId(iPad)];

			hr = EventWriteMultiplayerRoundEnd(
				DurangoStats::getUserId(iPad),
				&ZERO_GUID,
				0,
				DurangoStats::getPlayerSession(),
				DurangoStats::getMultiplayerCorrelationId(),
				plr->level->getLevelData()->getGameType()->isSurvival(), // GameMode
				0,
				plr->level->difficulty,
				roundLength, // Time (seconds)
				exitStatus
				);

			if (hr == 0) // Debug.
			{
				app.DebugPrintf(
					"<%ls> MultiplayerRoundEnd(%ls,%ls,%i,%i,%.1f,%i).\n",
					DurangoStats::getUserId(iPad),
					guid2str(DurangoStats::getPlayerSession()).c_str(),
					DurangoStats::getMultiplayerCorrelationId(),
					plr->level->getLevelData()->getGameType()->isSurvival(),
					plr->level->difficulty,
					roundLength,
					exitStatus);
			}
		}
	}

	return true;
}

bool CDurangoTelemetryManager::RecordLevelStart(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers)
{
	CTelemetryManager::RecordLevelStart(iPad, friendsOrMatch, competeOrCoop, difficulty, numberOfLocalPlayers, numberOfOnlinePlayers);
	
	ULONG hr = 0;

	// Grab player info.
	PlayerUID puid; shared_ptr<Player> plr;
	ProfileManager.GetXUID(iPad, &puid, true);
	plr = Minecraft::GetInstance()->localplayers[iPad];

	if (plr != NULL && plr->level != NULL && plr->level->getLevelData() != NULL)
	{
		hr = EventWritePlayerSessionStart(
			DurangoStats::getUserId(iPad),
			DurangoStats::getPlayerSession(),
			DurangoStats::getMultiplayerCorrelationId(),
			plr->level->getLevelData()->getGameType()->isSurvival(),
			plr->level->difficulty
			);

		if (hr == 0) // Debug.
		{
			app.DebugPrintf("<%ls> PlayerSessionStart(%ls,%ls,%i,%i)\n",
				DurangoStats::getUserId(iPad),
				guid2str(DurangoStats::getPlayerSession()).c_str(),
				DurangoStats::getMultiplayerCorrelationId(),
				plr->level->getLevelData()->getGameType()->isSurvival(),
				plr->level->difficulty);
		}

		m_multiplayerRoundStartTimes[DurangoStats::getUserId(iPad)] = -1.0f;

		// Send 'MultiplayerRoundStart' if we're starting an online game.
		if ( !g_NetworkManager.IsLocalGame() )
		{
			hr = EventWriteMultiplayerRoundStart(
				DurangoStats::getUserId(iPad),
				&ZERO_GUID,	// RoundId
				0,			// SectionId
				DurangoStats::getPlayerSession(),
				DurangoStats::getMultiplayerCorrelationId(),
				plr->level->getLevelData()->getGameType()->isSurvival(), // GameMode
				0,			// MatchType
				plr->level->difficulty);

			if (hr == 0) // Debug.
			{
				app.DebugPrintf(
					"<%ls> MultiplayerRoundStart(%ls,%ls,%i,%i).\n",
					DurangoStats::getUserId(iPad),
					guid2str(DurangoStats::getPlayerSession()).c_str(),
					DurangoStats::getMultiplayerCorrelationId(),
					plr->level->getLevelData()->getGameType()->isSurvival(),
					plr->level->difficulty);
			}

			m_multiplayerRoundStartTimes[DurangoStats::getUserId(iPad)] = app.getAppTime();
		}
	}

	hr = EventWriteLevelStart(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		friendsOrMatch, 
		competeOrCoop, 
		difficulty, 
		numberOfLocalPlayers, 
		numberOfOnlinePlayers,
		&ZERO_GUID
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordLevelStart("
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i,%i,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	friendsOrMatch, 
			/* int */	competeOrCoop, 
			/* int */	difficulty, 
			/* int */	numberOfLocalPlayers, 
			/* int */	numberOfOnlinePlayers

			);
	}

	return true;
}

bool CDurangoTelemetryManager::RecordLevelExit(int iPad, ESen_LevelExitStatus levelExitStatus)
{
	CTelemetryManager::RecordLevelExit(iPad, levelExitStatus);

	ULONG hr = EventWriteLevelExit(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		levelExitStatus,
		&ZERO_GUID
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordLevelExit(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	levelExitStatus
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordLevelSaveOrCheckpoint(int iPad, int saveOrCheckPointID, int saveSizeInBytes)
{
	CTelemetryManager::RecordLevelSaveOrCheckpoint(iPad, saveOrCheckPointID, saveSizeInBytes);

	ULONG hr = EventWriteLevelSaveOrCheckpoint(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		saveOrCheckPointID,
		saveSizeInBytes
		);

	if (hr == 0)
	{
		app.DebugPrintf(
		"<%ls> RecordLevelSaveOrCheckpoint(",
		"%is,%i,%i,%i,%i,%i,"
		"%ls,%ls,%i,%i).\n",

		/* WSTR */	DurangoStats::getUserId(iPad),

		// Sentient //
		/* int */	GetSecondsSinceInitialize(),
		/* int */	GetMode(iPad),
		/* int */	GetSubMode(iPad),
		/* int */	GetLevelId(iPad),
		/* int */	GetSubLevelId(iPad),
		/* int */	GetLevelInstanceID(),

		// Durango //
		/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
		/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
		/* int */	saveOrCheckPointID,
		/* int */	saveSizeInBytes
		);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordLevelResume(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers, int saveOrCheckPointID)
{
	CTelemetryManager::RecordLevelResume(iPad, friendsOrMatch, competeOrCoop, difficulty, numberOfLocalPlayers, numberOfOnlinePlayers, saveOrCheckPointID);

	ULONG hr = EventWriteLevelResume(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		friendsOrMatch,
		competeOrCoop,
		difficulty,
		numberOfLocalPlayers,
		numberOfOnlinePlayers,
		saveOrCheckPointID
		);

	if (hr == 0) // Debug
	{
		app.DebugPrintf(
			"<%ls> RecordLevelResume(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i,%i,%i,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	friendsOrMatch,
			/* int */	competeOrCoop,
			/* int */	difficulty,
			/* int */	numberOfLocalPlayers,
			/* int */	numberOfOnlinePlayers,
			/* int */	saveOrCheckPointID
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordPauseOrInactive(int iPad)
{
	CTelemetryManager::RecordPauseOrInactive(iPad);

	ULONG hr = EventWritePauseOrInactive(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId()
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordUnpauseOrActive(int iPad)
{
	CTelemetryManager::RecordUnpauseOrActive(iPad);

	ULONG hr = EventWriteUnpauseOrActive(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId()
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordMenuShown(int iPad, EUIScene menuID, int optionalMenuSubID)
{
	CTelemetryManager::RecordMenuShown(iPad, menuID, optionalMenuSubID);

	ULONG hr = EventWriteMenuShown(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		menuID,
		optionalMenuSubID
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordMenuShown(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	menuID,
			/* int */	optionalMenuSubID
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordAchievementUnlocked(int iPad, int achievementID, int achievementGamerscore)
{
	CTelemetryManager::RecordAchievementUnlocked(iPad, achievementID, achievementGamerscore);

	ULONG hr = EventWriteAchievemntUnlocked(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		achievementID,
		achievementGamerscore
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	achievementID,
			/* int */	achievementGamerscore
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordMediaShareUpload(int iPad, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType)
{
	CTelemetryManager::RecordMediaShareUpload(iPad, mediaDestination, mediaType);

#if 0
	ULONG hr = EventWriteRecordMediaShareUpload(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		mediaDestination,
		mediaType
		);
#else 
	ULONG hr = -1;
#endif

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	mediaDestination,
			/* int */	mediaType
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordUpsellPresented(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID)
{
	//CTelemetryManager::RecordUpsellPresented(iPad, upsellId

	ULONG hr = EventWriteUpsellPresented(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		upsellId,
		marketplaceOfferID
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	upsellId,
			/* int */	marketplaceOfferID
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordUpsellResponded(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID, ESen_UpsellOutcome upsellOutcome)
{
	ULONG hr = EventWriteUpsellResponded(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		upsellId,
		marketplaceOfferID,
		upsellOutcome
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	upsellId,
			/* int */	marketplaceOfferID,
			/* int */	upsellOutcome
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordPlayerDiedOrFailed(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	ULONG hr = EventWritePlayerDiedOrFailed(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		lowResMapX,
		lowResMapY,
		lowResMapZ,
		mapID,
		playerWeaponID,
		enemyWeaponID,
		enemyTypeID
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i,%i,%i,%i,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	lowResMapX,
			/* int */	lowResMapY,
			/* int */	lowResMapZ,
			/* int */	mapID,
			/* int */	playerWeaponID,
			/* int */	enemyWeaponID,
			/* int */	enemyTypeID
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordEnemyKilledOrOvercome(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID)
{
	ULONG hr = -1;

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%i,%i,%i,%i,%i,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	lowResMapX,
			/* int */	lowResMapY,
			/* int */	lowResMapZ,
			/* int */	mapID,
			/* int */	playerWeaponID,
			/* int */	enemyWeaponID,
			/* int */	enemyTypeID
			);
	}
	// NO EVENT TO SEND, ALREADY COVERED BY STATISTIC CODE!

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordTexturePackLoaded(int iPad, int texturePackId, bool purchased)
{
	ULONG hr = EventWriteTexturePackLoaded(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		texturePackId,
		purchased
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordPauseOrInactive(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i,%s).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	texturePackId,
			/* bool */	(purchased ? "Purchased" : "NotPurchased")
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordSkinChanged(int iPad, int dwSkinId)
{
	ULONG hr = EventWriteSkinChanged(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID,
		dwSkinId
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordSkinChanged(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls,%i).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId(),
			/* int */	dwSkinId
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordBanLevel(int iPad)
{
	ULONG hr = EventWriteBanLevel(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID
		);

	if (hr == 0)
	{
		app.DebugPrintf(
			"<%ls> RecordBanLevel(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId()
			);
	}

	return hr == 0;
}

bool CDurangoTelemetryManager::RecordUnBanLevel(int iPad)
{
	ULONG hr = EventWriteUnbanLevel(
		DurangoStats::getUserId(iPad),
		DurangoStats::getPlayerSession(),
		GetSecondsSinceInitialize(),
		GetMode(iPad),
		GetSubMode(iPad),
		GetLevelId(iPad),
		GetSubLevelId(iPad),
		GetLevelInstanceID(),
		&ZERO_GUID,
		&ZERO_GUID
		);

	if (hr == 0) // Debug.
	{
		app.DebugPrintf(
			"<%ls> RecordUnBanLevel(",
			"%is,%i,%i,%i,%i,%i,"
			"%ls,%ls).\n",

			/* WSTR */	DurangoStats::getUserId(iPad),

			// Sentient //
			/* int */	GetSecondsSinceInitialize(),
			/* int */	GetMode(iPad),
			/* int */	GetSubMode(iPad),
			/* int */	GetLevelId(iPad),
			/* int */	GetSubLevelId(iPad),
			/* int */	GetLevelInstanceID(),

			// Durango //
			/* GUID */	guid2str(DurangoStats::getPlayerSession()).c_str(),
			/* WSTR */	DurangoStats::getMultiplayerCorrelationId()
			);
	}

	return hr == 0;
}

DurangoStats *CDurangoTelemetryManager::durangoStats()
{
	return (DurangoStats*) GenericStats::getInstance();
}

wstring CDurangoTelemetryManager::guid2str(LPCGUID guid)
{
	wstring out = L"GUID<";
	out += _toString<unsigned long>(guid->Data1);
	out += L":";
	out += _toString<unsigned short>(guid->Data2);
	out += L":";
	out += _toString<unsigned short>(guid->Data3);
	//out += L":";
	//out += convStringToWstring(string((char*)&guid->Data4,8));
	out += L">";
	return out;
}