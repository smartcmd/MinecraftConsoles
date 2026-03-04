#pragma once

#include "..\..\..\Minecraft.World\DurangoStats.h"

#include "..\..\Common\Telemetry\TelemetryManager.h"

class CDurangoTelemetryManager : public CTelemetryManager
{
public:
	static GUID ZERO_GUID;

	unordered_map<wstring, float> m_multiplayerRoundStartTimes;

	CDurangoTelemetryManager();

	//virtual HRESULT Init();
	//virtual HRESULT Tick();
	//virtual HRESULT Flush();

	virtual bool RecordPlayerSessionStart(int iPad);
	virtual bool RecordPlayerSessionExit(int iPad, int exitStatus);
	//virtual bool RecordHeartBeat(int iPad);
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

	//virtual int GetMultiplayerInstanceID();
	//virtual int GenerateMultiplayerInstanceId();
	//virtual void SetMultiplayerInstanceId(int value);

protected:
	DurangoStats *durangoStats();

	wstring guid2str(LPCGUID guid);
};