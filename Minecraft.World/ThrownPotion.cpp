#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.effect.h"
#include "SharedConstants.h"
#include "JavaMath.h"
#include "ThrownPotion.h"



const double ThrownPotion::SPLASH_RANGE = 4.0;
const double ThrownPotion::SPLASH_RANGE_SQ = ThrownPotion::SPLASH_RANGE * ThrownPotion::SPLASH_RANGE;

void ThrownPotion::_init()
{		
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	potionValue = 0;
}

ThrownPotion::ThrownPotion(Level *level) : Throwable(level)
{
	_init();
}

ThrownPotion::ThrownPotion(Level *level, shared_ptr<Mob> mob, int potionValue) : Throwable(level,mob)
{
	_init();

	this->potionValue = potionValue;
}

ThrownPotion::ThrownPotion(Level *level, double x, double y, double z, int potionValue) : Throwable(level,x,y,z)
{
	_init();
	this->potionValue = potionValue;
}

float ThrownPotion::getGravity()
{
	return 0.05f;
}

float ThrownPotion::getThrowPower()
{
	return 0.5f;
}

float ThrownPotion::getThrowUpAngleOffset()
{
	return -20;
}

void ThrownPotion::setPotionValue(int potionValue)
{
	this->potionValue = potionValue;
}

int ThrownPotion::getPotionValue()
{
	return potionValue;
}

void ThrownPotion::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		vector<MobEffectInstance *> *mobEffects = Item::potion->getMobEffects(potionValue);

		if (mobEffects != NULL && !mobEffects->empty())
		{
			AABB *aoe = bb->grow(SPLASH_RANGE, SPLASH_RANGE / 2, SPLASH_RANGE);
			vector<shared_ptr<Entity> > *entitiesOfClass = level->getEntitiesOfClass(typeid(Mob), aoe);

			if (entitiesOfClass != NULL && !entitiesOfClass->empty())
			{
				//for (Entity e : entitiesOfClass)
				for(AUTO_VAR(it, entitiesOfClass->begin()); it != entitiesOfClass->end(); ++it)
				{
					//shared_ptr<Entity> e = *it;
					shared_ptr<Mob> e = dynamic_pointer_cast<Mob>( *it );
					double dist = distanceToSqr(e);
					if (dist < SPLASH_RANGE_SQ)
					{
						double scale = 1.0 - (sqrt(dist) / SPLASH_RANGE);
						if (e == res->entity)
						{
							scale = 1;
						}

						//for (MobEffectInstance effect : mobEffects)
						for(AUTO_VAR(itMEI, mobEffects->begin()); itMEI != mobEffects->end(); ++itMEI)
						{
							MobEffectInstance *effect = *itMEI;
							int id = effect->getId();
							if (MobEffect::effects[id]->isInstantenous())
							{
								MobEffect::effects[id]->applyInstantenousEffect(this->owner, e, effect->getAmplifier(), scale);
							}
							else
							{
								int duration = (int) (scale * (double) effect->getDuration() + .5);
								if (duration > SharedConstants::TICKS_PER_SECOND)
								{
									e->addEffect(new MobEffectInstance(id, duration, effect->getAmplifier()));
								}
							}
						}
					}
				}
			}
			delete entitiesOfClass;
		}
		level->levelEvent(LevelEvent::PARTICLES_POTION_SPLASH, (int) Math::round(x), (int) Math::round(y), (int) Math::round(z), potionValue);

		remove();
	}
}

void ThrownPotion::readAdditionalSaveData(CompoundTag *tag)
{
	Throwable::readAdditionalSaveData(tag);

	potionValue = tag->getInt(L"potionValue");
}

void ThrownPotion::addAdditonalSaveData(CompoundTag *tag)
{
	Throwable::addAdditonalSaveData(tag);

	tag->putInt(L"potionValue", potionValue);
}