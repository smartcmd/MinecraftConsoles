#pragma once

class MerchantRecipeList;
class MerchantRecipe;
class Player;

class Merchant
{
public:
	virtual void setTradingPlayer(shared_ptr<Player> player) = 0;
	virtual shared_ptr<Player> getTradingPlayer() = 0;
	virtual MerchantRecipeList *getOffers(shared_ptr<Player> forPlayer) = 0;
	virtual void overrideOffers(MerchantRecipeList *recipeList) = 0;
	virtual void notifyTrade(MerchantRecipe *activeRecipe) = 0;
	virtual void notifyTradeUpdated(shared_ptr<ItemInstance> item) = 0;
	virtual wstring getDisplayName() = 0;
};