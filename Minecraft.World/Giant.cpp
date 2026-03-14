#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "Giant.h"
#include "../Minecraft.Client/Textures.h"



Giant::Giant(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();

	heightOffset*=6;
	setSize(bbWidth * 6, bbHeight * 6);
}

void Giant::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(100);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5f);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(50);
}

float Giant::getWalkTargetValue(int x, int y, int z)
{
	return level->getBrightness(x, y, z)-0.5f;
}