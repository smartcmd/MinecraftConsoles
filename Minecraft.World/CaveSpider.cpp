#include "stdafx.h"
#include "SharedConstants.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "..\Minecraft.Client\Textures.h"
#include "CaveSpider.h"



CaveSpider::CaveSpider(Level *level) : Spider(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_CAVE_SPIDER; // 4J was  "/mob/cavespider.png";
	this->setSize(0.7f, 0.5f);
}

int CaveSpider::getMaxHealth()
{
	return 12;
}

float CaveSpider::getModelScale()
{
	return .7f;
}


bool CaveSpider::doHurtTarget(shared_ptr<Entity> target)
{
	if (Spider::doHurtTarget(target))
	{
		if ( dynamic_pointer_cast<Mob>(target) != NULL)
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

			if (poisonTime > 0) {
				dynamic_pointer_cast<Mob>(target)->addEffect(new MobEffectInstance(MobEffect::poison->id, poisonTime * SharedConstants::TICKS_PER_SECOND, 0));
			}
		}

		return true;
	}
	return false;
}

void CaveSpider::finalizeMobSpawn()
{
	// do nothing
}