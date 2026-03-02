#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "ControlledByPlayerGoal.h"

ControlledByPlayerGoal::ControlledByPlayerGoal(Mob *mob, float maxSpeed, float walkSpeed)
{
	this->mob = mob;
	this->maxSpeed = maxSpeed;
	this->walkSpeed = walkSpeed;
	speed = 0;
	boosting = false;
	boostTime = 0;
	boostTimeTotal = 0;
	setRequiredControlFlags(Control::MoveControlFlag | Control::JumpControlFlag | Control::LookControlFlag);
}

void ControlledByPlayerGoal::start()
{
	speed = 0;

	// 4J Stu - Need to initialise this otherwise the pig will never move if you jump on before another goal has made it move and set the speed
	if(mob->getSpeed() < walkSpeed) mob->setSpeed(walkSpeed);
}

void ControlledByPlayerGoal::stop()
{
	boosting = false;
	speed = 0;
}

bool ControlledByPlayerGoal::canUse()
{
	shared_ptr<Player> player = dynamic_pointer_cast<Player>( mob->rider.lock() );
	return mob->isAlive() && player && (boosting || mob->canBeControlledByRider());
}

void ControlledByPlayerGoal::tick()
{
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(mob->rider.lock());
	PathfinderMob *pig = (PathfinderMob *)mob;

	float yrd = Mth::wrapDegrees(player->yRot - mob->yRot) * 0.5f;
	if (yrd > 5) yrd = 5;
	if (yrd < -5) yrd = -5;

	mob->yRot = Mth::wrapDegrees(mob->yRot + yrd);
	if (speed < maxSpeed) speed += (maxSpeed - speed) * 0.01f;
	if (speed > maxSpeed) speed = maxSpeed;

	int x = Mth::floor(mob->x);
	int y = Mth::floor(mob->y);
	int z = Mth::floor(mob->z);
	float moveSpeed = speed;
	if (boosting)
	{
		if (boostTime++ > boostTimeTotal)
		{
			boosting = false;
		}
		moveSpeed += moveSpeed * 1.15f * Mth::sin((float) boostTime / boostTimeTotal * PI);
	}

	float friction = 0.91f;
	if (mob->onGround)
	{
		friction = 0.6f * 0.91f;
		int t = mob->level->getTile(x,y,z);
		if (t > 0)
		{
			friction = Tile::tiles[t]->friction * 0.91f;
		}
	}
	float friction2 = (0.6f * 0.6f * 0.91f * 0.91f * 0.6f * 0.91f) / (friction * friction * friction);
	float sin = Mth::sin(pig->yRot * PI / 180);
	float cos = Mth::cos(pig->yRot * PI / 180);
	float aproxSpeed = pig->getSpeed() * friction2;
	float dist = max((int)moveSpeed, 1);
	dist = aproxSpeed / dist;
	float normMoveSpeed = moveSpeed * dist;
	float xa = -(normMoveSpeed * sin);
	float za = normMoveSpeed * cos;

	if (Mth::abs(xa) > Mth::abs(za))
	{
		if (xa < 0) xa -= mob->bbWidth / 2.0f;
		if (xa > 0) xa += mob->bbWidth / 2.0f;
		za = 0;
	}
	else
	{
		xa = 0;
		if (za < 0) za -= mob->bbWidth / 2.0f;
		if (za > 0) za += mob->bbWidth / 2.0f;
	}

	int xt = Mth::floor(mob->x + xa);
	int zt = Mth::floor(mob->z + za);

	Node *size = new Node(Mth::floor(mob->bbWidth + 1), Mth::floor(mob->bbHeight + player->bbHeight + 1), Mth::floor(mob->bbWidth + 1));

	if (x != xt || z != zt)
	{
		if (PathFinder::isFree(mob, xt, y, zt, size, false, false, true) == PathFinder::TYPE_BLOCKED
			&& PathFinder::isFree(mob, x, y + 1, z, size, false, false, true) == PathFinder::TYPE_OPEN
			&& PathFinder::isFree(mob, xt, y + 1, zt, size, false, false, true) == PathFinder::TYPE_OPEN)
		{
			pig->getJumpControl()->jump();
		}
	}

	if (!player->abilities.instabuild && speed >= maxSpeed * 0.5f && mob->getRandom()->nextFloat() < 0.006f && !boosting)
	{
		shared_ptr<ItemInstance> carriedItem = player->getCarriedItem();

		if (carriedItem != NULL && carriedItem->id == Item::carrotOnAStick_Id)
		{
			carriedItem->hurt(1, player);

			if (carriedItem->count == 0)
			{
				shared_ptr<ItemInstance> replacement = shared_ptr<ItemInstance>(new ItemInstance(Item::fishingRod));
				replacement->setTag(carriedItem->tag);
				player->inventory->items[player->inventory->selected] = replacement;
			}
		}
	}

	mob->travel(0, moveSpeed);
}

bool ControlledByPlayerGoal::isBoosting()
{
	return boosting;
}

void ControlledByPlayerGoal::boost()
{
	boosting = true;
	boostTime = 0;
	boostTimeTotal = mob->getRandom()->nextInt(MAX_BOOST_TIME + MIN_BOOST_TIME + 1) + MIN_BOOST_TIME;
}

bool ControlledByPlayerGoal::canBoost()
{
	return !isBoosting() && speed > maxSpeed * 0.3f;
}