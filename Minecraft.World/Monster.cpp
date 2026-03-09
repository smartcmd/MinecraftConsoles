#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.enchantment.h"
#include "Monster.h"

#include "..\Minecraft.Client\Minecraft.h"


Monster::Monster(Level *level) : PathfinderMob( level )
{
	xpReward = Enemy::XP_REWARD_MEDIUM;
}

void Monster::aiStep() 
{
	updateSwingTime();
	float br = getBrightness(1);
	if (br > 0.5f)
	{
		noActionTime += 2;
	}

	PathfinderMob::aiStep();
}

void Monster::tick()
{
	PathfinderMob::tick();
	if (!level->isClientSide && (level->difficulty == Difficulty::PEACEFUL || Minecraft::GetInstance()->isTutorial() ) ) remove();
}

shared_ptr<Entity> Monster::findAttackTarget()
{
#ifndef _FINAL_BUILD
	if(app.GetMobsDontAttackEnabled())
	{
		return shared_ptr<Player>();
	}
#endif

	shared_ptr<Player> player = level->getNearestAttackablePlayer(shared_from_this(), 16);
	if (player != nullptr && canSee(player) ) return player;
	return shared_ptr<Player>();
}

bool Monster::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	if (PathfinderMob::hurt(source, dmg))
	{
		shared_ptr<Entity> sourceEntity = source->getEntity();
		if (rider.lock() == sourceEntity || riding == sourceEntity) return true;

		if (sourceEntity != shared_from_this()) 
		{
			attackTarget = sourceEntity;
		}
		return true;
	}
	return false;
}

/**
* Performs hurt action, returns if successful
* 
* @param target
* @return
*/
bool Monster::doHurtTarget(shared_ptr<Entity> target)
{
	float dmg = static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());
	int knockback = 0;

	
	if ( target->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> livingTarget = dynamic_pointer_cast<LivingEntity>(target);
		dmg += EnchantmentHelper::getDamageBonus(dynamic_pointer_cast<LivingEntity>(shared_from_this()), livingTarget);
		knockback += EnchantmentHelper::getKnockbackBonus(dynamic_pointer_cast<LivingEntity>(shared_from_this()), livingTarget);
	}

	boolean wasHurt = target->hurt(DamageSource::mobAttack(dynamic_pointer_cast<LivingEntity>(shared_from_this())), dmg);

	if (wasHurt)
	{
		if (knockback > 0)
		{
			target->push(-Mth::sin(yRot * PI / 180) * knockback * .5f, 0.1, Mth::cos(yRot * PI / 180) * knockback * .5f);
			xd *= 0.6;
			zd *= 0.6;
		}

		int fireAspect = EnchantmentHelper::getFireAspect(dynamic_pointer_cast<LivingEntity>(shared_from_this()));
		if (fireAspect > 0)
		{
			target->setOnFire(fireAspect * 4);
		}

		if (target->instanceof(eTYPE_LIVINGENTITY))
		{
			shared_ptr<LivingEntity> livingTarget = dynamic_pointer_cast<LivingEntity>(target);
			ThornsEnchantment::doThornsAfterAttack(shared_from_this(), livingTarget, random);
		}
	}

	return wasHurt;
}

void Monster::checkHurtTarget(shared_ptr<Entity> target, float distance)
{
	if (attackTime <= 0 && distance < 2.0f && target->bb->y1 > bb->y0 && target->bb->y0 < bb->y1)
	{
		attackTime = 20;
		doHurtTarget(target);
	}
}

float Monster::getWalkTargetValue(int x, int y, int z)
{
	return 0.5f - level->getBrightness(x, y, z);
}

bool Monster::isDarkEnoughToSpawn()
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);
	if (level->getBrightness(LightLayer::Sky, xt, yt, zt) > random->nextInt(32)) return false;

	int br = level->getRawBrightness(xt, yt, zt);

	if (level->isThundering()) 
	{
		int tmp = level->skyDarken;
		level->skyDarken = 10;
		br = level->getRawBrightness(xt, yt, zt);
		level->skyDarken = tmp;
	}

	return br <= random->nextInt(8);
}

bool Monster::canSpawn()
{
	return level->difficulty > Difficulty::PEACEFUL && isDarkEnoughToSpawn() && PathfinderMob::canSpawn();
}

void Monster::registerAttributes()
{
	PathfinderMob::registerAttributes();

	getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
}