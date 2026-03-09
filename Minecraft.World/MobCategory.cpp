#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "Creature.h"
#include "Material.h"
#include "MobCategory.h"

int MobCategory::maxNaturalMonsters = 0;
int MobCategory::maxNaturalAnimals = 0;
int MobCategory::maxNaturalAmbient = 0;
int MobCategory::maxNaturalSquid = 0;
int MobCategory::maxNaturalChickens = 0;
int MobCategory::maxNaturalWolves = 0;
int MobCategory::maxNaturalMushroomCows = 0;

int MobCategory::maxSnowGolems = 0;
int MobCategory::maxIronGolems = 0;
int MobCategory::maxBosses = 0;

int MobCategory::maxAnimalsWithBreeding = 0;
int MobCategory::maxChickensWithBreeding = 0;
int MobCategory::maxMushroomCowsWithBreeding = 0;
int MobCategory::maxWolvesWithBreeding = 0;
int MobCategory::maxVillagersWithBreeding = 0;

int MobCategory::maxAnimalsWithSpawnEgg = 0;
int MobCategory::maxChickensWithSpawnEgg = 0;
int MobCategory::maxWolvesWithSpawnEgg = 0;
int MobCategory::maxMonstersWithSpawnEgg = 0;
int MobCategory::maxVillagersWithSpawnEgg = 0;
int MobCategory::maxMushroomCowsWithSpawnEgg = 0;
int MobCategory::maxSquidsWithSpawnEgg = 0;
int MobCategory::maxAmbientWithSpawnEgg = 0;

MobCategory *MobCategory::monster = nullptr;
MobCategory *MobCategory::creature = nullptr;
MobCategory *MobCategory::ambient = nullptr;
MobCategory *MobCategory::waterCreature = nullptr;
// 4J - added these extra categories
MobCategory *MobCategory::creature_wolf = nullptr;
MobCategory *MobCategory::creature_chicken = nullptr;
MobCategory *MobCategory::creature_mushroomcow = nullptr;

MobCategoryArray MobCategory::values = MobCategoryArray(7);

void MobCategory::staticCtor()
{
	// 4J - adjusted the max levels here for the xbox version, which now represent the max levels in the whole world
	monster = new MobCategory(70, Material::air, false, false, eTYPE_MONSTER, false);
	// Raised these to be identical to base LCE
	creature = new MobCategory(50, Material::air, true, true, eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, eTYPE_ANIMAL, false);
	ambient = new MobCategory(20, Material::air, true, false, eTYPE_AMBIENT, false),
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

MobCategory::MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType)
	: m_max(maxVar), spawnPositionMaterial(spawnPositionMaterial), m_isFriendly(isFriendly), m_isPersistent(isPersistent), m_eBase(eBase), m_eJavaBase(eBase), m_isSingleType(isSingleType)
{
}

MobCategory::MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, eINSTANCEOF eJavaBase, bool isSingleType)
	: m_max(maxVar), spawnPositionMaterial(spawnPositionMaterial), m_isFriendly(isFriendly), m_isPersistent(isPersistent), m_eBase(eBase), m_eJavaBase(eJavaBase), m_isSingleType(isSingleType)
{
}

// 4J - added
const eINSTANCEOF MobCategory::getEnumBaseClass()
{
	if (app.GetGameHostOption(eGameHostOption_NoMobCap)) return m_eJavaBase;
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