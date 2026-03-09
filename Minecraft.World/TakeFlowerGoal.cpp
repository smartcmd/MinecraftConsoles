#include "stdafx.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "TakeFlowerGoal.h"

TakeFlowerGoal::TakeFlowerGoal(Villager *villager)
{
	takeFlower = false;
	pickupTick = 0;
	golem = weak_ptr<VillagerGolem>();

	this->villager = villager;
	setRequiredControlFlags(Control::MoveControlFlag | Control::LookControlFlag);
}

bool TakeFlowerGoal::canUse()
{
	if (villager->getAge() >= 0) return false;
	if (!villager->level->isDay()) return false;

	vector<shared_ptr<Entity> > *golems = villager->level->getEntitiesOfClass(typeid(VillagerGolem), villager->bb->grow(6, 2, 6));
	if ( golems == nullptr )
		return false;

	if ( golems->size() == 0)
	{
		delete golems;
		return false;
	}

    for (auto entity : *golems )
    {
		if ( entity == nullptr )
			continue;

		// safe to call std::static_pointer_cast because of getEntitiesOfClass(typeid(VillagerGolem), ...) makes sure we do not have entities of any other type.
		auto vg = std::static_pointer_cast<VillagerGolem>(entity);
		if ( vg && vg->getOfferFlowerTick() > 0)
		{
			golem = vg;
			delete golems;
			return true;
		}
	}
	delete golems;
	return false;
}

bool TakeFlowerGoal::canContinueToUse()
{
	return golem.lock() != nullptr && golem.lock()->getOfferFlowerTick() > 0;
}

void TakeFlowerGoal::start()
{
	pickupTick = villager->getRandom()->nextInt(static_cast<int>(OfferFlowerGoal::OFFER_TICKS * 0.8));
	takeFlower = false;
	golem.lock()->getNavigation()->stop();
}

void TakeFlowerGoal::stop()
{
	golem = weak_ptr<VillagerGolem>();
	villager->getNavigation()->stop();
}

void TakeFlowerGoal::tick()
{
	villager->getLookControl()->setLookAt(golem.lock(), 30, 30);
	if (golem.lock()->getOfferFlowerTick() == pickupTick)
	{
		villager->getNavigation()->moveTo(golem.lock(), 0.5f);
		takeFlower = true;
	}

	if (takeFlower)
	{
		if (villager->distanceToSqr(golem.lock()) < 2 * 2)
		{
			golem.lock()->offerFlower(false);
			villager->getNavigation()->stop();
		}
	}
}