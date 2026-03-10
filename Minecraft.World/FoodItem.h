#pragma once

#include "Item.h"

class Player;
class Level;

class FoodItem : public Item
{
public:
	static const int EAT_DURATION = static_cast<int>(20 * 1.6);

private:
	const int nutrition;
	const float saturationModifier;
	const bool m_isMeat;
	bool canAlwaysEat;

	int effectId;
	int effectDurationSeconds;
	int effectAmplifier;
	float effectProbability;

	void _init();
public:
	FoodItem(int id, int nutrition, float saturationMod, bool isMeat);
	FoodItem(int id, int nutrition, bool isMeat);

	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);

protected:
	virtual void addEatEffect(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);

public:
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);

	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player);

	int getNutrition();
	float getSaturationModifier();
	bool isMeat();

	FoodItem *setEatEffect(int id, int durationInSecods, int amplifier, float effectProbability);
	FoodItem *setCanAlwaysEat();

	// 4J Added
	bool canEat(shared_ptr<Player> player);
};