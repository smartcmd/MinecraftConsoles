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
	monster = new MobCategory(70, Material::air, false, false, eTYPE_MONSTER, false, CONSOLE_MONSTERS_HARD_LIMIT);
	creature = new MobCategory(10, Material::air, true, true, eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, false, CONSOLE_ANIMALS_HARD_LIMIT);
	ambient = new MobCategory(15, Material::air, true, false, eTYPE_AMBIENT, false, CONSOLE_AMBIENT_HARD_LIMIT),
	waterCreature = new MobCategory(5, Material::water, true, false, eTYPE_WATERANIMAL, false, CONSOLE_SQUID_HARD_LIMIT);

	values[0] = monster;
	values[1] = creature;
	values[2] = ambient;
	values[3] = waterCreature;
	// 4J - added 2 new categories to give us better control over spawning wolves & chickens
	creature_wolf = new MobCategory(3, Material::air, true, true, eTYPE_WOLF, true, MAX_XBOX_WOLVES);
	creature_chicken = new MobCategory( 2, Material::air, true, true, eTYPE_CHICKEN, true, MAX_XBOX_CHICKENS);
	creature_mushroomcow = new MobCategory(2, Material::air, true, true, eTYPE_MUSHROOMCOW, true, MAX_XBOX_MUSHROOMCOWS);
	values[4] = creature_wolf;
	values[5] = creature_chicken;
	values[6] = creature_mushroomcow;
}

MobCategory::MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType, int maxPerLevel)
	: m_max(maxVar), spawnPositionMaterial(spawnPositionMaterial), m_isFriendly(isFriendly), m_isPersistent(isPersistent), m_eBase(eBase), m_isSingleType(isSingleType), m_maxPerLevel(maxPerLevel)
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
