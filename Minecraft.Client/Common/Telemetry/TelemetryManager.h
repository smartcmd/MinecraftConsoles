#pragma once

#include "..\..\Common\UI\UIEnums.h"

class CTelemetryManager
{
public:
	virtual HRESULT Init();
	virtual HRESULT Tick();
	virtual HRESULT Flush();

	virtual bool RecordPlayerSessionStart(int iPad);
	virtual bool RecordPlayerSessionExit(int iPad, int exitStatus);
	virtual bool RecordHeartBeat(int iPad);
	virtual bool RecordLevelStart(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers);
	virtual bool RecordLevelExit(int iPad, ESen_LevelExitStatus levelExitStatus);
	virtual bool RecordLevelSaveOrCheckpoint(int iPad, int saveOrCheckPointID, int saveSizeInBytes);
	virtual bool RecordLevelResume(int iPad, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, int numberOfLocalPlayers, int numberOfOnlinePlayers, int saveOrCheckPointID);
	virtual bool RecordPauseOrInactive(int iPad);
	virtual bool RecordUnpauseOrActive(int iPad);
	virtual bool RecordMenuShown(int iPad, EUIScene menuID, int optionalMenuSubID);
	virtual bool RecordAchievementUnlocked(int iPad, int achievementID, int achievementGamerscore);
	virtual bool RecordMediaShareUpload(int iPad, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType);
	virtual bool RecordUpsellPresented(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID);
	virtual bool RecordUpsellResponded(int iPad, ESen_UpsellID upsellId, int marketplaceOfferID, ESen_UpsellOutcome upsellOutcome);
	virtual bool RecordPlayerDiedOrFailed(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID);
	virtual bool RecordEnemyKilledOrOvercome(int iPad, int lowResMapX, int lowResMapY, int lowResMapZ, int mapID, int playerWeaponID, int enemyWeaponID, ETelemetryChallenges enemyTypeID);
	virtual bool RecordTexturePackLoaded(int iPad, int texturePackId, bool purchased);

	virtual bool RecordSkinChanged(int iPad, int dwSkinId);
	virtual bool RecordBanLevel(int iPad);
	virtual bool RecordUnBanLevel(int iPad);

	virtual int GetMultiplayerInstanceID();
	virtual int GenerateMultiplayerInstanceId();
	virtual void SetMultiplayerInstanceId(int value);

protected:
	float m_initialiseTime;
	float m_lastHeartbeat;
	bool m_bFirstFlush;

	float m_fLevelStartTime[XUSER_MAX_COUNT];

	INT m_multiplayerInstanceID;
	DWORD m_levelInstanceID;

	// Helper functions to get the various common settings
	INT GetSecondsSinceInitialize();
	INT GetMode(DWORD dwUserId);
	INT GetSubMode(DWORD dwUserId);
	INT GetLevelId(DWORD dwUserId);
	INT GetSubLevelId(DWORD dwUserId);
	INT GetTitleBuildId();
	INT GetLevelInstanceID();
	INT GetSingleOrMultiplayer();
	INT GetDifficultyLevel(INT diff);
	INT GetLicense();
	INT GetDefaultGameControls();
	INT GetAudioSettings(DWORD dwUserId);
	INT GetLevelExitProgressStat1();
	INT GetLevelExitProgressStat2();
};

extern CTelemetryManager *TelemetryManager;