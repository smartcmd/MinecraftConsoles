#include "stdafx.h"

#include "PSVitaLeaderboardManager.h"

#include "PSVita\PSVita_App.h"
#include "PSVita\PSVitaExtras\ShutdownManager.h"

#include "Common\Consoles_App.h"
#include "Common\Network\Sony\SQRNetworkManager.h"

#include "..\..\..\Minecraft.World\StringHelpers.h"

#include <cstdlib>

#include <np.h>

LeaderboardManager *LeaderboardManager::m_instance = new PSVitaLeaderboardManager(); //Singleton instance of the LeaderboardManager

PSVitaLeaderboardManager::PSVitaLeaderboardManager() : SonyLeaderboardManager() {}

HRESULT PSVitaLeaderboardManager::initialiseScoreUtility() 
{
	return sceNpScoreInit( SCE_KERNEL_DEFAULT_PRIORITY_USER, SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT, NULL);
}

bool PSVitaLeaderboardManager::scoreUtilityAlreadyInitialised(HRESULT hr) 
{ 
	return hr == SCE_NP_COMMUNITY_ERROR_ALREADY_INITIALIZED; 
}

HRESULT PSVitaLeaderboardManager::createTitleContext(const SceNpId &npId)
{ 
	return sceNpScoreCreateTitleCtx(&s_npCommunicationId, &s_npCommunicationPassphrase, &npId); 
}

HRESULT PSVitaLeaderboardManager::destroyTitleContext(int titleContext)
{
	return sceNpScoreDestroyTitleCtx(titleContext);
}

HRESULT PSVitaLeaderboardManager::createTransactionContext(int titleContext)
{
	return sceNpScoreCreateRequest(titleContext);
}

HRESULT PSVitaLeaderboardManager::abortTransactionContext(int transactionContext)
{
	return sceNpScoreAbortRequest(transactionContext);
}

HRESULT PSVitaLeaderboardManager::destroyTransactionContext(int transactionContext)
{
	return sceNpScoreDeleteRequest(transactionContext);
}

HRESULT PSVitaLeaderboardManager::getFriendsList(sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::FriendsList> &friendsList)
{
	return sce::Toolkit::NP::Friends::Interface::getFriendslist(&friendsList, false);
}

char *PSVitaLeaderboardManager::getComment(SceNpScoreComment *comment)
{
	return comment->utf8Comment;
}