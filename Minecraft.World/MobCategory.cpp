#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "Creature.h"
#include "Material.h"
#include "MobCategory.h"

int MobCategory::max_natural_monsters = 50;
int MobCategory::max_natural_animals = 50;
int MobCategory::max_natural_ambient = 20;
int MobCategory::max_natural_squid = 5;
int MobCategory::max_natural_chickens = 8;
int MobCategory::max_natural_wolves = 8;
int MobCategory::max_natural_mushroomcows = 2;
int MobCategory::max_snow_golems = 16;
int MobCategory::max_iron_golems = 16;
int MobCategory::max_bosses = 1;
int MobCategory::max_villagers_with_breeding = 35;
int MobCategory::max_animals_with_breeding = max_natural_animals + 20;
int MobCategory::max_chickens_with_breeding = max_natural_chickens + 8;
int MobCategory::max_mushroomcows_with_breeding = max_natural_mushroomcows + 20;
int MobCategory::max_wolves_with_breeding = max_natural_wolves + 8;
int MobCategory::max_animals_with_spawn_egg = max_animals_with_breeding + 20;
int MobCategory::max_chickens_with_spawn_egg = max_chickens_with_breeding + 10;
int MobCategory::max_wolves_with_spawn_egg = max_wolves_with_breeding + 10;
int MobCategory::max_monsters_with_spawn_egg = max_natural_monsters + 20;
int MobCategory::max_villagers_with_spawn_egg = max_villagers_with_breeding + 15;
int MobCategory::max_mushroomcows_with_spawn_egg = max_mushroomcows_with_breeding + 8;
int MobCategory::max_squids_with_spawn_egg = max_natural_squid + 8;
int MobCategory::max_ambient_with_spawn_egg = max_natural_ambient + 8;

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
	
	monster->m_maxPerLevel = max_natural_monsters;
	creature->m_maxPerLevel = max_natural_animals;
	ambient->m_maxPerLevel = max_natural_ambient;
	waterCreature->m_maxPerLevel = max_natural_squid;
	creature_wolf->m_maxPerLevel = max_natural_wolves;
	creature_chicken->m_maxPerLevel = max_natural_chickens;
	creature_mushroomcow->m_maxPerLevel = max_natural_mushroomcows;
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