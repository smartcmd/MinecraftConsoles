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

	potionItem = nullptr;
}

ThrownPotion::ThrownPotion(Level *level) : Throwable(level)
{
	_init();
}

ThrownPotion::ThrownPotion(Level *level, shared_ptr<LivingEntity> mob, int potionValue) : Throwable(level,mob)
{
	_init();

	potionItem = std::make_shared<ItemInstance>(Item::potion, 1, potionValue);
}

ThrownPotion::ThrownPotion(Level *level, shared_ptr<LivingEntity> mob, shared_ptr<ItemInstance> potion) : Throwable(level, mob)
{
	_init();

	potionItem = potion;
}

ThrownPotion::ThrownPotion(Level *level, double x, double y, double z, int potionValue) : Throwable(level,x,y,z)
{
	_init();

	potionItem = std::make_shared<ItemInstance>(Item::potion, 1, potionValue);
}

ThrownPotion::ThrownPotion(Level *level, double x, double y, double z, shared_ptr<ItemInstance> potion) : Throwable(level, x, y, z)
{
	_init();

	potionItem = potion;
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
	if (potionItem == nullptr) potionItem = std::make_shared<ItemInstance>(Item::potion, 1, 0);
	potionItem->setAuxValue(potionValue);
}

int ThrownPotion::getPotionValue()
{
	if (potionItem == nullptr) potionItem = std::make_shared<ItemInstance>(Item::potion, 1, 0);
	return potionItem->getAuxValue();
}

void ThrownPotion::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		vector<MobEffectInstance *> *mobEffects = Item::potion->getMobEffects(potionItem);

		if (mobEffects != nullptr && !mobEffects->empty())
		{
			AABB *aoe = bb->grow(SPLASH_RANGE, SPLASH_RANGE / 2, SPLASH_RANGE);
			vector<shared_ptr<Entity> > *entitiesOfClass = level->getEntitiesOfClass(typeid(LivingEntity), aoe);

			if (entitiesOfClass != nullptr && !entitiesOfClass->empty())
			{
				for(auto & it : *entitiesOfClass)
				{
					shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>( it );
					double dist = distanceToSqr(e);
					if (dist < SPLASH_RANGE_SQ)
					{
						double scale = 1.0 - (sqrt(dist) / SPLASH_RANGE);
						if (e == res->entity)
						{
							scale = 1;
						}

						//for (MobEffectInstance effect : mobEffects)
						for(auto& effect : *mobEffects)
						{
							int id = effect->getId();
							if (MobEffect::effects[id]->isInstantenous())
							{
								MobEffect::effects[id]->applyInstantenousEffect(getOwner(), e, effect->getAmplifier(), scale);
							}
							else
							{
								int duration = static_cast<int>(scale * (double)effect->getDuration() + .5);
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
		level->levelEvent(LevelEvent::PARTICLES_POTION_SPLASH, static_cast<int>(Math::round(x)), static_cast<int>(Math::round(y)), static_cast<int>(Math::round(z)), getPotionValue() );

		remove();
	}
}

void ThrownPotion::readAdditionalSaveData(CompoundTag *tag)
{
	Throwable::readAdditionalSaveData(tag);

	if (tag->contains(L"Potion"))
	{
		potionItem = ItemInstance::fromTag(tag->getCompound(L"Potion"));
	}
	else
	{
		setPotionValue(tag->getInt(L"potionValue"));
	}

	if (potionItem == nullptr) remove();
}

void ThrownPotion::addAdditonalSaveData(CompoundTag *tag)
{
	Throwable::addAdditonalSaveData(tag);

	if (potionItem != nullptr) tag->putCompound(L"Potion", potionItem->save(new CompoundTag()));
}