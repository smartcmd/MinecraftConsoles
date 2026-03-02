#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "Minecart.h"
#include "SharedConstants.h"



const int Minecart::EXITS[][2][3] = { //
	//
	{
		{
			+0, +0, -1
		}, {
			+0, +0, +1
		}
	}, // 0
	{
		{
			-1, +0, +0
		}, {
			+1, +0, +0
		}
	}, // 1
	{
		{
			-1, -1, +0
		}, {
			+1, +0, +0
		}
	}, // 2
	{
		{
			-1, +0, +0
		}, {
			+1, -1, +0
		}
	}, // 3
	{
		{
			+0, +0, -1
		}, {
			+0, -1, +1
		}
	}, // 4
	{
		{
			+0, -1, -1
		}, {
			+0, +0, +1
		}
	}, // 5

	{
		{
			+0, +0, +1
		}, {
			+1, +0, +0
		}
	}, // 6
	{
		{
			+0, +0, +1
		}, {
			-1, +0, +0
		}
	}, // 7
	{
		{
			+0, +0, -1
		}, {
			-1, +0, +0
		}
	}, // 8
	{
		{
			+0, +0, -1
		}, {
			+1, +0, +0
		}
	}, // 9
};

void Minecart::_init()
{
	// 4J TODO This gets replaced again later so should maybe be inited as NULL?
	items = new ItemInstanceArray(9 * 4);

	flipped = false;

	type = fuel = 0;
	xPush = zPush = 0.0;

	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;
	lxd = lyd = lzd = 0.0;

	// Java default ctor
	blocksBuilding = true;
	setSize(0.98f, 0.7f);
	heightOffset = bbHeight / 2.0f;
	//

	// 4J Added
	m_bHasPushedCartThisTick = false;
}

Minecart::Minecart(Level *level) : Entity( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	_init();
}


bool Minecart::makeStepSound()
{
	return false;
}

void Minecart::defineSynchedData()
{
	entityData->define(DATA_ID_FUEL, (byte) 0);
	entityData->define(DATA_ID_HURT, 0);
	entityData->define(DATA_ID_HURTDIR, 1);
	entityData->define(DATA_ID_DAMAGE, 0);
}


AABB *Minecart::getCollideAgainstBox(shared_ptr<Entity> entity)
{
	return entity->bb;
}

AABB *Minecart::getCollideBox()
{
	// if (level->isClientSide) return NULL;
	return NULL;
}

bool Minecart::isPushable()
{
	return true;
}

Minecart::Minecart(Level *level, double x, double y, double z, int type) : Entity( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	_init();
	setPos(x, y + heightOffset, z);

	xd = 0;
	yd = 0;
	zd = 0;

	xo = x;
	yo = y;
	zo = z;
	this->type = type;
}

double Minecart::getRideHeight()
{
	return bbHeight * 0.0 - 0.3f;
}

bool Minecart::hurt(DamageSource *source, int hurtDamage)
{
	if (level->isClientSide || removed) return true;
	
	// 4J-JEV: Fix for #88212,
	// Untrusted players shouldn't be able to damage minecarts or boats.
	if (dynamic_cast<EntityDamageSource *>(source) != NULL)
	{
		shared_ptr<Entity> attacker = source->getDirectEntity();

		if (dynamic_pointer_cast<Player>(attacker) != NULL && 
			!dynamic_pointer_cast<Player>(attacker)->isAllowedToHurtEntity( shared_from_this() ))
			return false;
	}

	setHurtDir(-getHurtDir());
	setHurtTime(10);
	markHurt();

	// 4J Stu - If someone is riding in this, then it can tick multiple times which causes the damage to
	// decrease too quickly. So just make the damage a bit higher to start with for similar behaviour
	// to an unridden one. Only do this change if the riding player is attacking it.
	if( rider.lock() != NULL && rider.lock() == source->getEntity() ) hurtDamage += 1;

	// 4J Stu - Brought froward from 12w36 to fix #46611 - TU5: Gameplay: Minecarts and boat requires more hits than one to be destroyed in creative mode
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(source->getEntity());
	if (player != NULL && player->abilities.instabuild) this->setDamage(100);

	this->setDamage(getDamage() + (hurtDamage * 10));
	if (this->getDamage() > 20 * 2)
	{
		// 4J HEG - Fixed issue with player falling through the ground on destroying a minecart while riding (issue #160607)
		if (rider.lock() != NULL) rider.lock()->ride(nullptr);
		remove();

		spawnAtLocation(Item::minecart->id, 1, 0);
		if (type == Minecart::CHEST)
		{
			shared_ptr<Container> container = dynamic_pointer_cast<Container>( shared_from_this() );
			for (unsigned int i = 0; i < container->getContainerSize(); i++)
			{
				shared_ptr<ItemInstance> item = container->getItem(i);
				if (item != NULL)
				{
					float xo = random->nextFloat() * 0.8f + 0.1f;
					float yo = random->nextFloat() * 0.8f + 0.1f;
					float zo = random->nextFloat() * 0.8f + 0.1f;

					while (item->count > 0)
					{
						int count = random->nextInt(21) + 10;
						if (count > item->count) count = item->count;
						item->count -= count;

						shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()) ) ) );
						float pow = 0.05f;
						itemEntity->xd = (float) random->nextGaussian() * pow;
						itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
						itemEntity->zd = (float) random->nextGaussian() * pow;
						if (item->hasTag())
						{
							itemEntity->getItem()->setTag((CompoundTag *) item->getTag()->copy());
						}
						level->addEntity(itemEntity);
					}
				}
			}
			spawnAtLocation(Tile::chest_Id, 1, 0);
		}
		else if (type == Minecart::FURNACE)
		{
			spawnAtLocation(Tile::furnace_Id, 1, 0);
		}
	}
	return true;
}

void Minecart::animateHurt()
{
	setHurtDir(-getHurtDir());
	setHurtTime(10);
	this->setDamage(this->getDamage() + (getDamage() * 10));
}

bool Minecart::isPickable()
{
	return !removed;
}

void Minecart::remove()
{
	for (unsigned int i = 0; i < getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> item = getItem(i);
		if (item != NULL) 
		{
			float xo = random->nextFloat() * 0.8f + 0.1f;
			float yo = random->nextFloat() * 0.8f + 0.1f;
			float zo = random->nextFloat() * 0.8f + 0.1f;

			while (item->count > 0) 
			{
				int count = random->nextInt(21) + 10;
				if (count > item->count) count = item->count;
				item->count -= count;

				shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, shared_ptr<ItemInstance>( new ItemInstance(item->id, count, item->getAuxValue()) ) ) );
				float pow = 0.05f;
				itemEntity->xd = (float) random->nextGaussian() * pow;
				itemEntity->yd = (float) random->nextGaussian() * pow + 0.2f;
				itemEntity->zd = (float) random->nextGaussian() * pow;
				if (item->hasTag())
				{
					itemEntity->getItem()->setTag((CompoundTag *) item->getTag()->copy());
				}
				level->addEntity(itemEntity);
			}
		}
	}
	Entity::remove();
}


void Minecart::tick()
{
	// 4J - make minecarts (server-side) tick twice, to put things back to how they were when we were accidently ticking them twice
	for( int i = 0; i < 2; i++ )
	{
		if (getHurtTime() > 0) setHurtTime(getHurtTime() - 1);
		if (getDamage() > 0) setDamage(getDamage() - 1);
		if(y < -64)
		{
			outOfWorld();
		}

		if (hasFuel() && random->nextInt(4) == 0)
		{
			level->addParticle(eParticleType_largesmoke, x, y + 0.8, z, 0, 0, 0);
		}

		// 4J Stu - Fix for #8284 - Gameplay: Collision: Minecart clips into/ through blocks at the end of the track, prevents player from riding
		if (level->isClientSide) // && lSteps > 0)
		{
			if (lSteps > 0)
			{
				double xt = x + (lx - x) / lSteps;
				double yt = y + (ly - y) / lSteps;
				double zt = z + (lz - z) / lSteps;

				double yrd = Mth::wrapDegrees(lyr - yRot);

				yRot += (float) ( (yrd) / lSteps );
				xRot += (float) ( (lxr - xRot) / lSteps );

				lSteps--;
				this->setPos(xt, yt, zt);
				this->setRot(yRot, xRot);
			}
			else
			{
				this->setPos(x, y, z);
				this->setRot(yRot, xRot);
			}

			return;	// 4J - return here stops the client-side version of this from ticking twice
		}
		xo = x;
		yo = y;
		zo = z;

		yd -= 0.04f;

		int xt = Mth::floor(x);
		int yt = Mth::floor(y);
		int zt = Mth::floor(z);
		if (RailTile::isRail(level, xt, yt - 1, zt))
		{
			yt--;
		}

		double max = 0.4;

		double slideSpeed = 1 / 128.0;
		int tile = level->getTile(xt, yt, zt);
		if (RailTile::isRail(tile))
		{
			Vec3 *oldPos = getPos(x, y, z);
			int data = level->getData(xt, yt, zt);
			y = yt;

			bool powerTrack = false;
			bool haltTrack = false;
			if (tile == Tile::goldenRail_Id)
			{
				powerTrack = (data & RailTile::RAIL_DATA_BIT) != 0;
				haltTrack = !powerTrack;
			}
			if (((RailTile *) Tile::tiles[tile])->isUsesDataBit())
			{
				data &= RailTile::RAIL_DIRECTION_MASK;
			}

			if (data >= 2 && data <= 5)
			{
				y = yt + 1;
			}

			if (data == 2) xd -= slideSpeed;
			if (data == 3) xd += slideSpeed;
			if (data == 4) zd += slideSpeed;
			if (data == 5) zd -= slideSpeed;
		
			// 4J TODO Is this a good way to copy the bit of the array that we need?
			int exits[2][3];
			memcpy( &exits, (void *)EXITS[data], sizeof(int) * 2 * 3);
			//int exits[2][3] = EXITS[data];

			double xD = exits[1][0] - exits[0][0];
			double zD = exits[1][2] - exits[0][2];
			double dd = sqrt(xD * xD + zD * zD);

			double flip = xd * xD + zd * zD;
			if (flip < 0)
			{
				xD = -xD;
				zD = -zD;
			}

			double pow = sqrt(xd * xd + zd * zd);

			xd = pow * xD / dd;
			zd = pow * zD / dd;
			
			shared_ptr<Entity> sharedRider = rider.lock();
			if (sharedRider != NULL)
			{
				double riderDist = (sharedRider->xd * sharedRider->xd + sharedRider->zd * sharedRider->zd);
				double ownDist = xd * xd + zd * zd;

				if (riderDist > 0.0001 && ownDist < 0.01)
				{
					xd += sharedRider->xd * 0.1;
					zd += sharedRider->zd * 0.1;

					haltTrack = false;
				}
			}

			// on golden rails without power, stop the cart
			if (haltTrack)
			{
				double speedLength = sqrt(xd * xd + zd * zd);
				if (speedLength < 0.03)
				{
					xd *= 0;
					yd *= 0;
					zd *= 0;
				}
				else
				{
					xd *= 0.5f;
					yd *= 0;
					zd *= 0.5f;
				}
			}

			double progress = 0;
			double x0 = xt + 0.5 + exits[0][0] * 0.5;
			double z0 = zt + 0.5 + exits[0][2] * 0.5;
			double x1 = xt + 0.5 + exits[1][0] * 0.5;
			double z1 = zt + 0.5 + exits[1][2] * 0.5;

			xD = x1 - x0;
			zD = z1 - z0;

			if (xD == 0)
			{
				x = xt + 0.5;
				progress = z - zt;
			}
			else if (zD == 0)
			{
				z = zt + 0.5;
				progress = x - xt;
			}
			else
			{

				double xx = x - x0;
				double zz = z - z0;

				progress = (xx * xD + zz * zD) * 2;
			}

			x = x0 + xD * progress;
			z = z0 + zD * progress;

			setPos(x, y + heightOffset, z);

			double xdd = xd;
			double zdd = zd;
			if (rider.lock() != NULL)
			{
				xdd *= 0.75;
				zdd *= 0.75;
			}
			if (xdd < -max) xdd = -max;
			if (xdd > +max) xdd = +max;
			if (zdd < -max) zdd = -max;
			if (zdd > +max) zdd = +max;
			move(xdd, 0, zdd);

			if (exits[0][1] != 0 && Mth::floor(x) - xt == exits[0][0] && Mth::floor(z) - zt == exits[0][2])
			{
				setPos(x, y + exits[0][1], z);
			}
			else if (exits[1][1] != 0 && Mth::floor(x) - xt == exits[1][0] && Mth::floor(z) - zt == exits[1][2])
			{
				setPos(x, y + exits[1][1], z);
			}
			else
			{
			}

			if (rider.lock() != NULL)
			{
				xd *= 0.997f;
				yd *= 0;
				zd *= 0.997f;
			}
			else
			{
				if (type == Minecart::FURNACE)
				{
					double sd = xPush * xPush + zPush * zPush;
					if (sd > 0.01 * 0.01)
					{
						sd = sqrt(sd);
						xPush /= sd;
						zPush /= sd;
						double speed = 0.04;
						xd *= 0.8f;
						yd *= 0;
						zd *= 0.8f;
						xd += xPush * speed;
						zd += zPush * speed;
					}
					else
					{
						xd *= 0.9f;
						yd *= 0;
						zd *= 0.9f;
					}
				}
				xd *= 0.96f;
				yd *= 0;
				zd *= 0.96f;

			}

			Vec3 *newPos = getPos(x, y, z);
			if (newPos != NULL && oldPos != NULL)
			{
				double speed = (oldPos->y - newPos->y) * 0.05;

				pow = sqrt(xd * xd + zd * zd);
				if (pow > 0)
				{
					xd = xd / pow * (pow + speed);
					zd = zd / pow * (pow + speed);
				}
				setPos(x, newPos->y, z);
			}

			int xn = Mth::floor(x);
			int zn = Mth::floor(z);
			if (xn != xt || zn != zt)
			{
				pow = sqrt(xd * xd + zd * zd);

				xd = pow * (xn - xt);
				zd = pow * (zn - zt);
			}

			if (type == Minecart::FURNACE)
			{
				double sd = xPush * xPush + zPush * zPush;
				if (sd > 0.01 * 0.01 && xd * xd + zd * zd > 0.001)
				{
					sd = sqrt(sd);
					xPush /= sd;
					zPush /= sd;

					if (xPush * xd + zPush * zd < 0)
					{
						xPush = 0;
						zPush = 0;
					}
					else
					{
						xPush = xd;
						zPush = zd;
					}
				}
			}

			// if on golden rail with power, increase speed
			if (powerTrack)
			{
				double speedLength = sqrt(xd * xd + zd * zd);
				if (speedLength > .01)
				{
					double speed = 0.06;
					xd += xd / speedLength * speed;
					zd += zd / speedLength * speed;
				}
				else
				{
					// if the minecart is standing still, accelerate it away
					// from potentional walls
					if (data == RailTile::DIR_FLAT_X)
					{
						if (level->isSolidBlockingTile(xt - 1, yt, zt))
						{
							xd = .02;
						}
						else if (level->isSolidBlockingTile(xt + 1, yt, zt))
						{
							xd = -.02;
						}
					}
					else if (data == RailTile::DIR_FLAT_Z)
					{
						if (level->isSolidBlockingTile(xt, yt, zt - 1))
						{
							zd = .02;
						}
						else if (level->isSolidBlockingTile(xt, yt, zt + 1))
						{
							zd = -.02;
						}
					}
				}
			}

			checkInsideTiles();
		}
		else
		{
			if (xd < -max) xd = -max;
			if (xd > +max) xd = +max;
			if (zd < -max) zd = -max;
			if (zd > +max) zd = +max;
			if (onGround)
			{
				xd *= 0.5f;
				yd *= 0.5f;
				zd *= 0.5f;
			}
			move(xd, yd, zd);

			if (onGround)
			{
			}
			else
			{
				xd *= 0.95f;
				yd *= 0.95f;
				zd *= 0.95f;
			}
		}

		xRot = 0;
		double xDiff = xo - x;
		double zDiff = zo - z;
		if (xDiff * xDiff + zDiff * zDiff > 0.001)
		{
			yRot = (float) (atan2(zDiff, xDiff) * 180 / PI);
			if (flipped) yRot += 180;
		}

		double rotDiff = Mth::wrapDegrees(yRot - yRotO);

		if (rotDiff < -170 || rotDiff >= 170)
		{
			yRot += 180;
			flipped = !flipped;
		}
		setRot(yRot, xRot);

		// if (!level->isClientSide) {
		{
			vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), this->bb->grow(0.2f, 0, 0.2f));
			if (entities != NULL && !entities->empty())
			{
				AUTO_VAR(itEnd, entities->end());
				for (AUTO_VAR(it, entities->begin()); it != itEnd; it++)
				{
					shared_ptr<Entity> e = (*it); //entities->at(i);
					if (e != rider.lock() && e->isPushable() && e->GetType() == eTYPE_MINECART)
					{
						shared_ptr<Minecart> cart = dynamic_pointer_cast<Minecart>(e);
						cart->m_bHasPushedCartThisTick = false;
						cart->push(shared_from_this());

						// 4J Added - We should only be pushed by one minecart per tick, the closest one
						// Fix for #46937 - TU5: Gameplay: Crash/Freeze occurs when a minecart with an animal inside will be forced to despawn
						if( cart->m_bHasPushedCartThisTick ) break;
					}
				}
			}
		}

		if (rider.lock() != NULL)
		{
			if (rider.lock()->removed)
			{
				if (rider.lock()->riding == shared_from_this())
				{
					rider.lock()->riding = nullptr;
				}
				rider = weak_ptr<Entity>();
			}
		}

		if (fuel > 0)
		{
			fuel--;
		}
		if (fuel <= 0)
		{
			xPush = zPush = 0;
		}
		setHasFuel(fuel > 0);
	}
}

Vec3 *Minecart::getPosOffs(double x, double y, double z, double offs)
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);
	if (RailTile::isRail(level, xt, yt - 1, zt))
	{
		yt--;
	}

	int tile = level->getTile(xt, yt, zt);
	if (RailTile::isRail(tile))
	{
		int data = level->getData(xt, yt, zt);

		if (((RailTile *) Tile::tiles[tile])->isUsesDataBit())
		{
			data &= RailTile::RAIL_DIRECTION_MASK;
		}

		y = yt;
		if (data >= 2 && data <= 5)
		{
			y = yt + 1;
		}

		// 4J TODO Is this a good way to copy the bit of the array that we need?
		int exits[2][3];
		memcpy( &exits, (void *)EXITS[data], sizeof(int) * 2 * 3);
		//int exits[2][3] = EXITS[data];

		double xD = exits[1][0] - exits[0][0];
		double zD = exits[1][2] - exits[0][2];
		double dd = sqrt(xD * xD + zD * zD);
		xD /= dd;
		zD /= dd;

		x += xD * offs;
		z += zD * offs;

		if (exits[0][1] != 0 && Mth::floor(x) - xt == exits[0][0] && Mth::floor(z) - zt == exits[0][2])
		{
			y += exits[0][1];
		}
		else if (exits[1][1] != 0 && Mth::floor(x) - xt == exits[1][0] && Mth::floor(z) - zt == exits[1][2])
		{
			y += exits[1][1];
		}
		else
		{
		}

		return getPos(x, y, z);
	}
	return NULL;
}

Vec3 *Minecart::getPos(double x, double y, double z)
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);
	if (RailTile::isRail(level, xt, yt - 1, zt))
	{
		yt--;
	}

	int tile = level->getTile(xt, yt, zt);
	if (RailTile::isRail(tile))
	{
		int data = level->getData(xt, yt, zt);
		y = yt;

		if (((RailTile *) Tile::tiles[tile])->isUsesDataBit())
		{
			data &= RailTile::RAIL_DIRECTION_MASK;
		}

		if (data >= 2 && data <= 5)
		{
			y = yt + 1;
		}

		// 4J TODO Is this a good way to copy the bit of the array that we need?
		int exits[2][3];
		memcpy( &exits, (void *)EXITS[data], sizeof(int) * 2 * 3);
		//int exits[2][3] = EXITS[data];

		double progress = 0;
		double x0 = xt + 0.5 + exits[0][0] * 0.5;
		double y0 = yt + 0.5 + exits[0][1] * 0.5;
		double z0 = zt + 0.5 + exits[0][2] * 0.5;
		double x1 = xt + 0.5 + exits[1][0] * 0.5;
		double y1 = yt + 0.5 + exits[1][1] * 0.5;
		double z1 = zt + 0.5 + exits[1][2] * 0.5;

		double xD = x1 - x0;
		double yD = (y1 - y0) * 2;
		double zD = z1 - z0;

		if (xD == 0)
		{
			x = xt + 0.5;
			progress = z - zt;
		}
		else if (zD == 0)
		{
			z = zt + 0.5;
			progress = x - xt;
		}
		else
		{

			double xx = x - x0;
			double zz = z - z0;

			progress = (xx * xD + zz * zD) * 2;
		}

		x = x0 + xD * progress;
		y = y0 + yD * progress;
		z = z0 + zD * progress;
		if (yD < 0) y += 1;
		if (yD > 0) y += 0.5;
		return Vec3::newTemp(x, y, z);
	}
	return NULL;
}


void Minecart::addAdditonalSaveData(CompoundTag *base)
{
	base->putInt(L"Type", type);

	if (type == Minecart::FURNACE) 
	{
		base->putDouble(L"PushX", xPush);
		base->putDouble(L"PushZ", zPush);
		base->putShort(L"Fuel", (short) fuel);
	}
	else if (type == Minecart::CHEST)
	{
		ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

		for (unsigned int i = 0; i < items->length; i++)
		{
			if ( (*items)[i] != NULL)
			{
				CompoundTag *tag = new CompoundTag();
				tag->putByte(L"Slot", (byte) i);
				(*items)[i]->save(tag);
				listTag->add(tag);
			}
		}
		base->put(L"Items", listTag);
	}
}

void Minecart::readAdditionalSaveData(CompoundTag *base)
{
	type = base->getInt(L"Type");
	if (type == Minecart::FURNACE)
	{
		xPush = base->getDouble(L"PushX");
		zPush = base->getDouble(L"PushZ");
		fuel = base->getShort(L"Fuel");
	}
	else if (type == Minecart::CHEST)
	{
		ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) base->getList(L"Items");
		items = new ItemInstanceArray( getContainerSize() );
		for (int i = 0; i < inventoryList->size(); i++)
		{
			CompoundTag *tag = inventoryList->get(i);
			unsigned int slot = tag->getByte(L"Slot") & 0xff;
			if (slot >= 0 && slot < items->length) (*items)[slot] = shared_ptr<ItemInstance>( ItemInstance::fromTag(tag) );
		}
	}
}


float Minecart::getShadowHeightOffs()
{
	return 0;
}

void Minecart::push(shared_ptr<Entity> e)
{
	if (level->isClientSide) return;

	if (e == rider.lock()) return;
	if (( dynamic_pointer_cast<Mob>(e)!=NULL) && dynamic_pointer_cast<Player>(e)==NULL && dynamic_pointer_cast<VillagerGolem>(e) == NULL && type == Minecart::RIDEABLE && xd * xd + zd * zd > 0.01)
	{
		if (rider.lock() == NULL && e->riding == NULL)
		{
			e->ride( shared_from_this() );
		}
	}

	double xa = e->x - x;
	double za = e->z - z;

	double dd = xa * xa + za * za;
	if (dd >= 0.0001f)
	{
		dd = sqrt(dd);
		xa /= dd;
		za /= dd;
		double pow = 1 / dd;
		if (pow > 1) pow = 1;
		xa *= pow;
		za *= pow;
		xa *= 0.1f;
		za *= 0.1f;

		xa *= 1 - pushthrough;
		za *= 1 - pushthrough;
		xa *= 0.5;
		za *= 0.5;

		if (e->GetType() == eTYPE_MINECART)
		{
			double xo = e->x - x;
			double zo = e->z - z;

			//4J Stu - Brought forward changes to fix minecarts pushing each other
			// Fix for #38882 - TU5: Gameplay: Minecart with furnace is not able to move another minecart on the rail.
			Vec3 *dir = Vec3::newTemp(xo, 0, zo)->normalize();
			Vec3 *facing = Vec3::newTemp(cos(yRot * PI / 180), 0, sin(yRot * PI / 180))->normalize();

			double dot = abs(dir->dot(facing));

			if (dot < 0.8f)
			{
				return;
			}

			double xdd = (e->xd + xd);
			double zdd = (e->zd + zd);

			shared_ptr<Minecart> cart = dynamic_pointer_cast<Minecart>(e);
			if (cart != NULL && cart->type == Minecart::FURNACE && type != Minecart::FURNACE)
			{
				xd *= 0.2f;
				zd *= 0.2f;
				this->Entity::push( e->xd - xa, 0, e->zd - za);
				e->xd *= 0.95f;
				e->zd *= 0.95f;
				m_bHasPushedCartThisTick = true;
			}
			else if (cart != NULL && cart->type != Minecart::FURNACE && type == Minecart::FURNACE)
			{
				e->xd *= 0.2f;
				e->zd *= 0.2f;
				e->push(xd + xa, 0, zd + za);
				xd *= 0.95f;
				zd *= 0.95f;
				m_bHasPushedCartThisTick = true;
			}
			else
			{
				xdd /= 2;
				zdd /= 2;
				xd *= 0.2f;
				zd *= 0.2f;
				this->Entity::push(xdd - xa, 0, zdd - za);
				e->xd *= 0.2f;
				e->zd *= 0.2f;
				e->push(xdd + xa, 0, zdd + za);
				m_bHasPushedCartThisTick = true;

				// 4J Stu - Fix for #46937 - TU5: Gameplay: Crash/Freeze occurs when a minecart with an animal inside will be forced to despawn
				// Minecarts can end up stuck inside each other, so if they are too close then they should separate quickly
				double modifier = 1.0;
				if( abs(xo) < 1 && abs(zo) < 1)
				{
					modifier += 1-( (abs(xo) + abs(zo))/2);
				}
				// 4J Stu - Decelerate the cart that is pushing this one if they are too close
				e->xd /= modifier;
				e->zd /= modifier;

				// 4J Backup fix for QNAN
				if( !(xd==xd) ) xd = 0;
				if( !(zd==zd) ) zd = 0;
				if( !(e->xd == e->xd) ) e->xd = 0;
				if( !(e->zd == e->zd) ) e->zd = 0;
			}

		}
		else
		{
			this->Entity::push(-xa, 0, -za);
			e->push(xa / 4, 0, za / 4);
		}
	}
}

unsigned int Minecart::getContainerSize()
{
	return 9 * 3;
}

shared_ptr<ItemInstance> Minecart::getItem(unsigned int slot)
{
	return (*items)[slot];
}

shared_ptr<ItemInstance> Minecart::removeItem(unsigned int slot, int count)
{
	if ( (*items)[slot] != NULL)
	{
		if ( (*items)[slot]->count <= count)
		{
			shared_ptr<ItemInstance> item =  (*items)[slot];
			(*items)[slot] = nullptr;
			return item;
		}
		else
		{
			shared_ptr<ItemInstance> i = (*items)[slot]->remove(count);
			if ((*items)[slot]->count == 0)  (*items)[slot] = nullptr;
			return i;
		}
	}
	return nullptr;
}

shared_ptr<ItemInstance> Minecart::removeItemNoUpdate(int slot)
{
	if ( (*items)[slot] != NULL)
	{
		shared_ptr<ItemInstance> item = (*items)[slot];
		(*items)[slot] = nullptr;
		return item;
	}
	return nullptr;
}

void Minecart::setItem(unsigned int slot, shared_ptr<ItemInstance> item)
{
	(*items)[slot] = item;
	if (item != NULL && item->count > getMaxStackSize()) item->count = getMaxStackSize();
}

int Minecart::getName()
{
	return IDS_ITEM_MINECART;
}

int Minecart::getMaxStackSize()
{
	return Container::LARGE_MAX_STACK_SIZE;
}

void Minecart::setChanged()
{
}

bool Minecart::interact(shared_ptr<Player> player)
{
	if (type == Minecart::RIDEABLE)
	{
		if (rider.lock() != NULL && dynamic_pointer_cast<Player>(rider.lock())!=NULL && rider.lock() != player) return true;
		if (!level->isClientSide)
		{
			// 4J HEG - Fixed issue with player not being able to dismount minecart (issue #4455)
			player->ride( rider.lock() == player ? nullptr : shared_from_this() );
		}
	}
	else if (type == Minecart::CHEST)
	{
		if ( player->isAllowedToInteract(shared_from_this()) )
		{
			if (!level->isClientSide)
				player->openContainer( dynamic_pointer_cast<Container>( shared_from_this() ) );
		}
		else
		{
			return false;
		}
	}
	else if (type == Minecart::FURNACE)
	{
		shared_ptr<ItemInstance> selected = player->inventory->getSelected();
		if (selected != NULL && selected->id == Item::coal->id)
		{
			if (--selected->count == 0) player->inventory->setItem(player->inventory->selected, nullptr);
			fuel += SharedConstants::TICKS_PER_SECOND * 180;

		}
		xPush = x - player->x;
		zPush = z - player->z;
	}
	return true;
}

float Minecart::getLootContent()
{
	int count = 0;
	for (unsigned int i = 0; i < items->length; i++)
	{
		if ( (*items)[i] != NULL) count++;
	}
	return count / (float) items->length;
}


void Minecart::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	lSteps = steps + 2;

	this->xd = lxd;
	this->yd = lyd;
	this->zd = lzd;
}

void Minecart::lerpMotion(double xd, double yd, double zd)
{
	lxd = this->xd = xd;
	lyd = this->yd = yd;
	lzd = this->zd = zd;
}

bool Minecart::stillValid(shared_ptr<Player> player)
{
	if (this->removed) return false;
	if (player->distanceToSqr(shared_from_this()) > 8 * 8) return false;
	return true;
}

bool Minecart::hasFuel()
{
	return (entityData->getByte(DATA_ID_FUEL) & 1) != 0;
}

void Minecart::setHasFuel(bool fuel)
{
	if (fuel)
	{
		entityData->set(DATA_ID_FUEL, (byte) (entityData->getByte(DATA_ID_FUEL) | 1));
	}
	else
	{
		entityData->set(DATA_ID_FUEL, (byte) (entityData->getByte(DATA_ID_FUEL) & ~1));
	}
}

void Minecart::startOpen()
{
	// TODO Auto-generated method stub

}

void Minecart::stopOpen()
{
	// TODO Auto-generated method stub

}

void Minecart::setDamage(int damage)
{
	entityData->set(DATA_ID_DAMAGE, damage);
}

int Minecart::getDamage()
{
	return entityData->getInteger(DATA_ID_DAMAGE);
}

void Minecart::setHurtTime(int hurtTime)
{
	entityData->set(DATA_ID_HURT, hurtTime);
}

int Minecart::getHurtTime()
{
	return entityData->getInteger(DATA_ID_HURT);
}

void Minecart::setHurtDir(int hurtDir)
{
	entityData->set(DATA_ID_HURTDIR, hurtDir);
}

int Minecart::getHurtDir()
{
	return entityData->getInteger(DATA_ID_HURTDIR);
}