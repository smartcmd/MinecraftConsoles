#include "stdafx.h"

#include "Durango/ServiceConfig/Events-XBLA.8-149E11AEEvents.h"

#include "../Minecraft.World/DurangoStats.h"

#include "GameProgress.h"

namespace WFC = Windows::Foundation::Collections;
namespace MXSA = Microsoft::Xbox::Services::Achievements;
namespace CC = concurrency;

GameProgress *GameProgress::instance = NULL;

void GameProgress::Flush(int iPad)
{
	if (instance == NULL) 
		instance = new GameProgress();

	instance->updatePlayer(iPad);
}

void GameProgress::Tick()
{
	if (instance == NULL)
		instance = new GameProgress();

	long long currentTime = System::currentTimeMillis();
	if ( (currentTime - instance->m_lastUpdate) > (UPDATE_FREQUENCY / 4) )
	{
		instance->updatePlayer(instance->m_nextPad);
		instance->m_nextPad = ++instance->m_nextPad % MAX_LOCAL_PLAYERS;
		instance->m_lastUpdate = currentTime;
	}
}

GameProgress::GameProgress()
{
	m_nextPad = 0;
	m_lastUpdate = 0;
}

void GameProgress::updatePlayer(int iPad)
{
	if ( ProfileManager.IsGuest(iPad) || !ProfileManager.IsSignedInLive(iPad) ) return;
	
	PlayerUID uid;
	ProfileManager.GetXUID(iPad, &uid, true);

	WXS::User^ user = ProfileManager.GetUser(iPad);

	if (user == nullptr) return;

	MXS::XboxLiveContext ^xlc = ref new MXS::XboxLiveContext(user);

	// Get these while they are still valid.
	LPCGUID playerSession = DurangoStats::getPlayerSession();

	CC::create_task(
		xlc->AchievementService->GetAchievementsForTitleIdAsync(
			ref new Platform::String(uid.toString().c_str()),	// Xuid
			0x149E11AE,											// TitleId
			MXSA::AchievementType::Persistent,					// Use regular achievements (not challenges)
			false,												// Unlocked only
			MXSA::AchievementOrderBy::UnlockTime,				// Order (we don't really care)
			0,													// skipItems (start index)
			200													// MaxItems
			)
		).then( [this,iPad,uid,playerSession] (CC::task<MXSA::AchievementsResult^> resultTask)
		{
			try
			{
				int achievementsUnlocked = 0;

				MXSA::AchievementsResult^ result = resultTask.get();
				if(result)
				{
					for (unsigned int i = 0, iMax = result->Items->Size; i < iMax; i++)
					{
						MXSA::Achievement^ ach = result->Items->GetAt(i);
						if (ach->ProgressState == MXSA::AchievementProgressState::Achieved)
							achievementsUnlocked++;
					}

					float gameprogress;
					if (EventWriteGameProgress(
						uid.toString().c_str(),
						playerSession,
						gameprogress = calcGameProgress(achievementsUnlocked) )
						== 0)
					{
						app.DebugPrintf("<%ls> GameProgress(%.1f)\n", uid.toString().c_str(), gameprogress);
					}
				}
			}
			catch (Platform::Exception ^ex)
			{
				app.DebugPrintf("GameProgress:: Error, couldn't contact the achievments service (?): %ls", ex->Message->Data());
			}
		});
	
}

float GameProgress::calcGameProgress(int achievementsUnlocked)
{
	return (float) achievementsUnlocked / 0.60f;
}