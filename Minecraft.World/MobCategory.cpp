#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "Creature.h"
#include "Material.h"
#include "MobCategory.h"

MobCategory *MobCategory::monster = NULL;
MobCategory *MobCategory::creature = NULL;
MobCategory *MobCategory::ambient = NULL;
MobCategory *MobCategory::waterCreature = NULL;
// 4J - added these extra categories
MobCategory *MobCategory::creature_wolf = NULL;
MobCategory *MobCategory::creature_chicken = NULL;
MobCategory *MobCategory::creature_mushroomcow = NULL;

MobCategoryArray MobCategory::values = MobCategoryArray(7);

void MobCategory::staticCtor()
{
	// 4J - adjusted the max levels here for the xbox version, which now represent the max levels in the whole world
	monster = new MobCategory(70, Material::air, false, false, eTYPE_MONSTER, false);
	creature = new MobCategory(10, Material::air, true, true, eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false);
	ambient = new MobCategory(15, Material::air, true, false, eTYPE_AMBIENT, false),
	waterCreature = new MobCategory(5, Material::water, true, false, eTYPE_WATERANIMAL, false);

	values[0] = monster;
	values[1] = creature;
	values[2] = ambient;
	values[3] = waterCreature;
	// 4J - added 2 new categories to give us better control over spawning wolves & chickens
	creature_wolf = new MobCategory(3, Material::air, true, true, eTYPE_WOLF, true);
	creature_chicken = new MobCategory(2, Material::air, true, true, eTYPE_CHICKEN, true);
	creature_mushroomcow = new MobCategory(2, Material::air, true, true, eTYPE_MUSHROOMCOW, true);
	values[4] = creature_wolf;
	values[5] = creature_chicken;
	values[6] = creature_mushroomcow;

	updateMobCaps(0); // Set the mob caps to the default limits
}

MobCategory::MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType)
	: m_max(maxVar), spawnPositionMaterial(spawnPositionMaterial), m_isFriendly(isFriendly), m_isPersistent(isPersistent), m_eBase(eBase), m_isSingleType(isSingleType)
{
}

// 4J - added
const eINSTANCEOF MobCategory::getEnumBaseClass()
{
	return m_eBase;
}

int MobCategory::getMaxInstancesPerChunk()
{
	return m_max;
}

int MobCategory::getMaxInstancesPerLevel()	// 4J added
{
	return m_maxPerLevel;
}

Material *MobCategory::getSpawnPositionMaterial()
{
	return (Material *) spawnPositionMaterial;
}

bool MobCategory::isFriendly()
{
	return m_isFriendly;
}

bool MobCategory::isSingleType()
{
	return m_isSingleType;
}

bool MobCategory::isPersistent()
{
	return m_isPersistent;
}

void MobCategory::updateMobCaps(int mode)
{
	switch (mode)
	{
		case 0: // Original limits
			maxNaturalMonsters = 50;
			maxNaturalAnimals = 50;
			maxNaturalAmbient = 20;
			maxNaturalSquid = 5;
			maxNaturalChickens = 8;
			maxNaturalWolves = 8;
			maxNaturalMushroomCows = 2;
			maxSnowGolems = 16;
			maxIronGolems = 16;
			maxBosses = 1;
			maxVillagersWithBreeding = 35;
			break;
		case 1:
			// Match previous definition of limits
			maxNaturalMonsters = 70;
			maxNaturalAnimals = 70;
			maxNaturalAmbient = 20;
			maxNaturalSquid = 10;
			maxNaturalChickens = 16;
			maxNaturalWolves = 16;
			maxNaturalMushroomCows = 8;
			maxSnowGolems = 16;
			maxIronGolems = 16;
			maxBosses = 1;
			maxVillagersWithBreeding = 50;
			break;
		case 2: // Large, higher than both original and new limits
			maxNaturalMonsters = 100;
			maxNaturalAnimals = 100;
			maxNaturalAmbient = 40;
			maxNaturalSquid = 15;
			maxNaturalChickens = 24;
			maxNaturalWolves = 24;
			maxNaturalMushroomCows = 16;
			maxSnowGolems = 32;
			maxIronGolems = 32;
			maxBosses = 3;
			maxVillagersWithBreeding = 100;
			break;
		case 3:
			return; // Keep current limits, as all related checks are bypassed in this case
	}
	monster->m_maxPerLevel = maxNaturalMonsters;
	creature->m_maxPerLevel = maxNaturalAnimals;
	ambient->m_maxPerLevel = maxNaturalAmbient;
	waterCreature->m_maxPerLevel = maxNaturalSquid;
	creature_wolf->m_maxPerLevel = maxNaturalWolves;
	creature_chicken->m_maxPerLevel = maxNaturalChickens;
	creature_mushroomcow->m_maxPerLevel = maxNaturalMushroomCows;
	maxAnimalsWithBreeding = maxNaturalAnimals + 20;
	maxChickensWithBreeding = maxNaturalChickens + 8;
	maxMushroomCowsWithBreeding = maxNaturalMushroomCows + 20;
	maxWolvesWithBreeding = maxNaturalWolves + 8;
	maxAnimalsWithSpawnEgg = maxAnimalsWithBreeding + 20;
	maxChickensWithSpawnEgg = maxChickensWithBreeding + 10;
	maxWolvesWithSpawnEgg = maxWolvesWithBreeding + 10;
	maxMonstersWithSpawnEgg = maxNaturalMonsters + 20;
	maxVillagersWithSpawnEgg = maxVillagersWithBreeding + 15;
	maxMushroomCowsWithSpawnEgg = maxMushroomCowsWithBreeding + 8;
	maxSquidsWithSpawnEgg = maxNaturalSquid + 8;
	maxAmbientWithSpawnEgg = maxNaturalAmbient + 8;
}