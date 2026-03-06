#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "BasicTypeContainers.h"
#include "BreedGoal.h"
#include "ExperienceOrb.h"

#include "GenericStats.h"

BreedGoal::BreedGoal(Animal *animal, double speedModifier)
{
	partner = weak_ptr<Animal>();
	loveTime = 0;

	this->animal = animal;
	this->level = animal->level;
	this->speedModifier = speedModifier;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool BreedGoal::canUse()
{
	if (!animal->isInLove()) return false;
	partner = weak_ptr<Animal>(getFreePartner());
	return partner.lock() != NULL;
}

bool BreedGoal::canContinueToUse()
{
	return partner.lock() != NULL && partner.lock()->isAlive() && partner.lock()->isInLove() && loveTime < 20 * 3;
}

void BreedGoal::stop()
{
	partner = weak_ptr<Animal>();
	loveTime = 0;
}

void BreedGoal::tick()
{
	animal->getLookControl()->setLookAt(partner.lock(), 10, animal->getMaxHeadXRot());
	animal->getNavigation()->moveTo(partner.lock(), speedModifier);
	++loveTime;
	if (loveTime >= 20 * 3 && animal->distanceToSqr(partner.lock()) < 3*3) breed();
}

shared_ptr<Animal> BreedGoal::getFreePartner()
{
	float r = 8;
	vector<shared_ptr<Entity> > *others = level->getEntitiesOfClass(typeid(*animal), animal->bb->grow(r, r, r));
	double dist = Double::MAX_VALUE;
	shared_ptr<Animal> partner = nullptr;
	if ( others )
	{
		for ( auto& it : *others )
		{
			shared_ptr<Animal> p = dynamic_pointer_cast<Animal>(it);
			if ( p && animal->canMate(p) && animal->distanceToSqr(p) < dist)
			{
				partner = p;
				dist = animal->distanceToSqr(p);
			}
		}
		delete others;
	}
	return partner;
}

void BreedGoal::breed()
{
	shared_ptr<AgableMob> offspring = animal->getBreedOffspring(partner.lock());
	animal->setDespawnProtected();
	partner.lock()->setDespawnProtected();
	if (offspring == NULL)
	{
		// This will be NULL if we've hit our limits for spawning any particular type of animal... reset things as normally as we can, without actually producing any offspring
		animal->resetLove();
		partner.lock()->resetLove();
		return;
	}

	shared_ptr<Player> loveCause = animal->getLoveCause();
	if (loveCause == NULL && partner.lock()->getLoveCause() != NULL)
	{
		loveCause = partner.lock()->getLoveCause();
	}

	if (loveCause != NULL)
	{
		// Record mob bred stat.
		loveCause->awardStat(GenericStats::breedEntity(offspring->GetType()),GenericStats::param_breedEntity(offspring->GetType()));

		if (animal->GetType() == eTYPE_COW)
		{
			//loveCause->awardStat(Achievements.breedCow);
		}
	}

	animal->setAge(5 * 60 * 20);
	partner.lock()->setAge(5 * 60 * 20);
	animal->resetLove();
	partner.lock()->resetLove();
	offspring->setAge(AgableMob::BABY_START_AGE);
	offspring->moveTo(animal->x, animal->y, animal->z, 0, 0);
	offspring->setDespawnProtected();
	level->addEntity(offspring);

	Random *random = animal->getRandom();
	for (int i = 0; i < 7; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		level->addParticle(eParticleType_heart, animal->x + random->nextFloat() * animal->bbWidth * 2 - animal->bbWidth, animal->y + .5f + random->nextFloat() * animal->bbHeight, animal->z + random->nextFloat()
			* animal->bbWidth * 2 - animal->bbWidth, xa, ya, za);
	}
	// 4J-PB - Fix for 106869- Customer Encountered: TU12: Content: Gameplay: Breeding animals does not give any Experience Orbs.
	level->addEntity( shared_ptr<ExperienceOrb>( new ExperienceOrb(level, animal->x, animal->y, animal->z, random->nextInt(7) + 1) ) );
}
