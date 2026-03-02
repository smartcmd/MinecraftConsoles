#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "FoodConstants.h"
#include "FoodData.h"

FoodData::FoodData()
{
	exhaustionLevel = 0;
	tickTimer = 0;

	this->foodLevel = FoodConstants::MAX_FOOD;
	this->lastFoodLevel = FoodConstants::MAX_FOOD;
	this->saturationLevel = FoodConstants::START_SATURATION;
}

void FoodData::eat(int food, float saturationModifier)
{
	foodLevel = min(food + foodLevel, FoodConstants::MAX_FOOD);
	saturationLevel = min(saturationLevel + (float) food * saturationModifier * 2.0f, (float)foodLevel);
}

void FoodData::eat(FoodItem *item)
{
	eat(item->getNutrition(), item->getSaturationModifier());
}

void FoodData::tick(shared_ptr<Player> player)
{

	int difficulty = player->level->difficulty;

	lastFoodLevel = foodLevel;

	if (exhaustionLevel > FoodConstants::EXHAUSTION_DROP)
	{
		exhaustionLevel -= FoodConstants::EXHAUSTION_DROP;

		if (saturationLevel > 0)
		{
			saturationLevel = max(saturationLevel - 1, 0.0f);
		}
		else if (difficulty > Difficulty::PEACEFUL)
		{
			foodLevel = max(foodLevel - 1, 0);
		}
	}

	// 4J Added - Allow host to disable using hunger. We don't deplete the hunger bar due to exhaustion
	// but I think we should deplete it to heal
	if(player->isAllowedToIgnoreExhaustion())
	{
		if(foodLevel > 0 && player->isHurt())
		{
			tickTimer++;
			if (tickTimer >= FoodConstants::HEALTH_TICK_COUNT)
			{
				player->heal(1);
				--foodLevel;
				tickTimer = 0;
			}
		}
	}
	else if (foodLevel >= FoodConstants::HEAL_LEVEL && player->isHurt())
	{
		tickTimer++;
		if (tickTimer >= FoodConstants::HEALTH_TICK_COUNT)
		{
			player->heal(1);
			tickTimer = 0;
		}
	}
	else if (foodLevel <= FoodConstants::STARVE_LEVEL)
	{
		tickTimer++;
		if (tickTimer >= FoodConstants::HEALTH_TICK_COUNT)
		{
			if (player->getHealth() > 10 || difficulty >= Difficulty::HARD || (player->getHealth() > 1 && difficulty >= Difficulty::NORMAL))
			{
				player->hurt(DamageSource::starve, 1);
			}
			tickTimer = 0;
		}
	}
	else
	{
		tickTimer = 0;
	}

}

void FoodData::readAdditionalSaveData(CompoundTag *entityTag)
{

	if (entityTag->contains(L"foodLevel"))
	{
		foodLevel = entityTag->getInt(L"foodLevel");
		tickTimer = entityTag->getInt(L"foodTickTimer");
		saturationLevel = entityTag->getFloat(L"foodSaturationLevel");
		exhaustionLevel = entityTag->getFloat(L"foodExhaustionLevel");
	}
}

void FoodData::addAdditonalSaveData(CompoundTag *entityTag)
{
	entityTag->putInt(L"foodLevel", foodLevel);
	entityTag->putInt(L"foodTickTimer", tickTimer);
	entityTag->putFloat(L"foodSaturationLevel", saturationLevel);
	entityTag->putFloat(L"foodExhaustionLevel", exhaustionLevel);
}

int FoodData::getFoodLevel()
{
	return foodLevel;
}

int FoodData::getLastFoodLevel()
{
	return lastFoodLevel;
}

bool FoodData::needsFood()
{
	return foodLevel < FoodConstants::MAX_FOOD;
}

void FoodData::addExhaustion(float amount)
{
	exhaustionLevel = min(exhaustionLevel + amount, FoodConstants::MAX_SATURATION * 2);
}

float FoodData::getExhaustionLevel()
{
	return exhaustionLevel;
}

float FoodData::getSaturationLevel()
{
	return saturationLevel;
}

void FoodData::setFoodLevel(int food)
{
	this->foodLevel = food;
}

void FoodData::setSaturation(float saturation)
{
	this->saturationLevel = saturation;
}

void FoodData::setExhaustion(float exhaustion)
{
	this->exhaustionLevel = exhaustion;
}