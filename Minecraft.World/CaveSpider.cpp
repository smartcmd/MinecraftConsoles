#include "stdafx.h"
#include "SharedConstants.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "../Minecraft.Client/Textures.h"
#include "CaveSpider.h"



CaveSpider::CaveSpider(Level *level) : Spider(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	registerAttributes();

	this->setSize(0.7f, 0.5f);
}

void CaveSpider::registerAttributes()
{
	Spider::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(12);
}

bool CaveSpider::doHurtTarget(shared_ptr<Entity> target)
{
	if (Spider::doHurtTarget(target))
	{
		if ( target->instanceof(eTYPE_LIVINGENTITY) )
		{
			int poisonTime = 0;
			if (level->difficulty <= Difficulty::EASY)
			{
				// No poison!
			}
			else if (level->difficulty == Difficulty::NORMAL)
			{
				poisonTime = 7;
			}
			else if (level->difficulty == Difficulty::HARD)
			{
				poisonTime = 15;
			}

			if (poisonTime > 0)
			{
				dynamic_pointer_cast<LivingEntity>(target)->addEffect(new MobEffectInstance(MobEffect::poison->id, poisonTime * SharedConstants::TICKS_PER_SECOND, 0));
			}
		}

		return true;
	}
	return false;
}

MobGroupData *CaveSpider::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	// do nothing
	return groupData;
}