#pragma once

#include "Common\Leaderboards\SonyLeaderboardManager.h"
#include "Common\Leaderboards\LeaderboardManager.h"

#include "Conf.h"

#include <np.h>

class PSVitaLeaderboardManager : public SonyLeaderboardManager
{
public:
	PSVitaLeaderboardManager();

protected:

	virtual HRESULT initialiseScoreUtility();

	virtual bool scoreUtilityAlreadyInitialised(HRESULT hr);

	virtual HRESULT createTitleContext(const SceNpId &npId);

	virtual HRESULT destroyTitleContext(int titleContext);

	virtual HRESULT createTransactionContext(int titleContext);

	virtual HRESULT abortTransactionContext(int transactionContext);

	virtual HRESULT destroyTransactionContext(int transactionContext);

	virtual HRESULT getFriendsList(sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::FriendsList> &friendsList);

	virtual char *	getComment(SceNpScoreComment *comment);
};
