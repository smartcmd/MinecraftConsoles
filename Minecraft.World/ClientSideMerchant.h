#pragma once

#include "Merchant.h"

class MerchantContainer;
class MerchantRecipeList;
class MerchantRecipe;

class ClientSideMerchant : public Merchant, public enable_shared_from_this<ClientSideMerchant>
{
private:
	MerchantContainer *container;
	shared_ptr<Player> source;
	MerchantRecipeList *currentOffers;
	wstring m_name;

public:
	ClientSideMerchant(shared_ptr<Player> source, const wstring &name);
	~ClientSideMerchant();

	void createContainer(); // 4J Added
	Container *getContainer();
	shared_ptr<Player> getTradingPlayer();
	void setTradingPlayer(shared_ptr<Player> player);
	MerchantRecipeList *getOffers(shared_ptr<Player> forPlayer);
	void overrideOffers(MerchantRecipeList *recipeList);
	void notifyTrade(MerchantRecipe *activeRecipe);
	void notifyTradeUpdated(shared_ptr<ItemInstance> item);
	wstring getDisplayName();
};