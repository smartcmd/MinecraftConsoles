#include "stdafx.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "com.mojang.nbt.h"
#include "Minecart.h"
#include "SharedConstants.h"



const int Minecart::EXITS[][2][3] = { //
	//
	{{+0, +0, -1}, {+0, +0, +1}}, // 0
	{{-1, +0, +0}, {+1, +0, +0}}, // 1
	{{-1, -1, +0}, {+1, +0, +0}}, // 2
	{{-1, +0, +0}, {+1, -1, +0}}, // 3
	{{+0, +0, -1}, {+0, -1, +1}}, // 4
	{{+0, -1, -1}, {+0, +0, +1}}, // 5

	{{+0, +0, +1}, {+1, +0, +0}}, // 6
	{{+0, +0, +1}, {-1, +0, +0}}, // 7
	{{+0, +0, -1}, {-1, +0, +0}}, // 8
	{{+0, +0, -1}, {+1, +0, +0}}, // 9
};

void Minecart::_init()
{
	flipped = false;

	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;
	lxd = lyd = lzd = 0.0;

	// Java default ctor
	blocksBuilding = true;
	setSize(0.98f, 0.7f);
	heightOffset = bbHeight / 2.0f;
	soundUpdater = NULL;
	name = L"";
	//

	// 4J Added
	m_bHasPushedCartThisTick = false;
}

Minecart::Minecart(Level *level) : Entity( level )
{
	_init();

	//soundUpdater = level != NULL ? level->makeSoundUpdater(this) : NULL;
}

Minecart::~Minecart()
{
	delete soundUpdater;
}

shared_ptr<Minecart> Minecart::createMinecart(Level *level, double x, double y, double z, int type)
{
	switch (type)
	{
	case TYPE_CHEST:
		return shared_ptr<MinecartChest>( new MinecartChest(level, x, y, z) );
	case TYPE_FURNACE:
		return shared_ptr<MinecartFurnace>( new MinecartFurnace(level, x, y, z) );
	case TYPE_TNT:
		return shared_ptr<MinecartTNT>( new MinecartTNT(level, x, y, z) );
	case TYPE_SPAWNER:
		return shared_ptr<MinecartSpawner>( new MinecartSpawner(level, x, y, z) );
	case TYPE_HOPPER:
		return shared_ptr<MinecartHopper>( new MinecartHopper(level, x, y, z) );
	default:
		return shared_ptr<MinecartRideable>( new MinecartRideable(level, x, y, z) );
	}
}

bool Minecart::makeStepSound()
{
	return false;
}

void Minecart::defineSynchedData()
{
	entityData->define(DATA_ID_HURT, 0);
	entityData->define(DATA_ID_HURTDIR, 1);
	entityData->define(DATA_ID_DAMAGE, 0.0f);
	entityData->define(DATA_ID_DISPLAY_TILE, 0);
	entityData->define(DATA_ID_DISPLAY_OFFSET, 6);
	entityData->define(DATA_ID_CUSTOM_DISPLAY, (byte) 0);
}


AABB *Minecart::getCollideAgainstBox(shared_ptr<Entity> entity)
{
	if (entity->isPushable())
	{
		return entity->bb;
	}
	return NULL;
}

AABB *Minecart::getCollideBox()
{
	return NULL;
}

bool Minecart::isPushable()
{
	return true;
}

Minecart::Minecart(Level *level, double x, double y, double z) : Entity( level )
{

	_init();
	setPos(x, y, z);

	xd = 0;
	yd = 0;
	zd = 0;

	xo = x;
	yo = y;
	zo = z;
}

double Minecart::getRideHeight()
{
	return bbHeight * 0.0 - 0.3f;
}

bool Minecart::hurt(DamageSource *source, float hurtDamage)
{
	if (level->isClientSide || removed) return true;
	if (isInvulnerable()) return false;

	// 4J-JEV: Fix for #88212,
	// Untrusted players shouldn't be able to damage minecarts or boats.
	if (dynamic_cast<EntityDamageSource *>(source) != NULL)
	{
		shared_ptr<Entity> attacker = source->getDirectEntity();

		if ( attacker->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(attacker)->isAllowedToHurtEntity( shared_from_this() ))
		{
			return false;
		}
	}

	setHurtDir(-getHurtDir());
	setHurtTime(10);
	markHurt();
	setDamage(getDamage() + (hurtDamage * 10));

	// 4J Stu - If someone is riding in this, then it can tick multiple times which causes the damage to
	// decrease too quickly. So just make the damage a bit higher to start with for similar behaviour
	// to an unridden one. Only do this change if the riding player is attacking it.
	if( rider.lock() != NULL && rider.lock() == source->getEntity() ) hurtDamage += 1;

	bool creativePlayer = source->getEntity() != NULL && source->getEntity()->instanceof(eTYPE_PLAYER) && dynamic_pointer_cast<Player>(source->getEntity())->abilities.instabuild;

	if (creativePlayer || getDamage() > 20 * 2)
	{
		// 4J HEG - Fixed issue with player falling through the ground on destroying a minecart while riding (issue #160607)
		if (rider.lock() != NULL) rider.lock()->ride(nullptr);

		if (!creativePlayer || hasCustomName())
		{
			destroy(source);
		}
		else
		{
			remove();
		}
	}
	return true;
}

void Minecart::destroy(DamageSource *source)
{
	remove();
	shared_ptr<ItemInstance> item = shared_ptr<ItemInstance>( new ItemInstance(Item::minecart, 1) );
	if (!name.empty()) item->setHoverName(name);
	spawnAtLocation(item, 0);
}

void Minecart::animateHurt()
{
	setHurtDir(-getHurtDir());
	setHurtTime(10);
	setDamage(getDamage() + (getDamage() * 10));
}

bool Minecart::isPickable()
{
	return !removed;
}

void Minecart::remove()
{
	Entity::remove();
	//if (soundUpdater != NULL) soundUpdater->tick();
}

void Minecart::tick()
{
	//if (soundUpdater != NULL) soundUpdater->tick();
	// 4J - make minecarts (server-side) tick twice, to put things back to how they were when we were accidently ticking them twice
	for( int i = 0; i < 2; i++ )
	{
		if (getHurtTime() > 0) setHurtTime(getHurtTime() - 1);
		if (getDamage() > 0) setDamage(getDamage() - 1);
		if(y < -64)
		{
			outOfWorld();
		}

		if (!level->isClientSide && dynamic_cast<ServerLevel *>(level) != NULL)
		{
			MinecraftServer *server = ((ServerLevel *) level)->getServer();
			int waitTime = getPortalWaitTime();

			if (isInsidePortal)
			{
				if (server->isNetherEnabled())
				{
					if (riding == NULL)
					{
						if (portalTime++ >= waitTime)
						{
							portalTime = waitTime;
							changingDimensionDelay = getDimensionChangingDelay();

							int targetDimension;

							if (level->dimension->id == -1)
							{
								targetDimension = 0;
							}
							else
							{
								targetDimension = -1;
							}

							changeDimension(targetDimension);
						}
					}
					isInsidePortal = false;
				}
			}
			else
			{
				if (portalTime > 0) portalTime -= 4;
				if (portalTime < 0) portalTime = 0;
			}
			if (changingDimensionDelay > 0) changingDimensionDelay--;
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
				setPos(xt, yt, zt);
				setRot(yRot, xRot);
			}
			else
			{
				setPos(x, y, z);
				setRot(yRot, xRot);
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
		if (BaseRailTile::isRail(level, xt, yt - 1, zt))
		{
			yt--;
		}

		double max = 0.4;

		double slideSpeed = 1 / 128.0;
		int tile = level->getTile(xt, yt, zt);
		if (BaseRailTile::isRail(tile))
		{
			int data = level->getData(xt, yt, zt);
			moveAlongTrack(xt, yt, zt, max, slideSpeed, tile, data);

			if (tile == Tile::activatorRail_Id)
			{
				activateMinecart(xt, yt, zt, (data & BaseRailTile::RAIL_DATA_BIT) != 0);
			}
		}
		else
		{
			comeOffTrack(max);
		}

		checkInsideTiles();

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

		vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), bb->grow(0.2f, 0, 0.2f));
		if (entities != NULL && !entities->empty())
		{
			for (auto& e : *entities)
			{
				if ( e && e != rider.lock() && e->isPushable() && e->instanceof(eTYPE_MINECART))
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
	}
}

void Minecart::activateMinecart(int xt, int yt, int zt, bool state)
{
}

void Minecart::comeOffTrack(double maxSpeed)
{
	if (xd < -maxSpeed) xd = -maxSpeed;
	if (xd > +maxSpeed) xd = +maxSpeed;
	if (zd < -maxSpeed) zd = -maxSpeed;
	if (zd > +maxSpeed) zd = +maxSpeed;
	if (onGround)
	{
		xd *= 0.5f;
		yd *= 0.5f;
		zd *= 0.5f;
	}
	move(xd, yd, zd);

	if (!onGround)
	{
		xd *= 0.95f;
		yd *= 0.95f;
		zd *= 0.95f;
	}
}

void Minecart::moveAlongTrack(int xt, int yt, int zt, double maxSpeed, double slideSpeed, int tile, int data)
{
	fallDistance = 0;

	Vec3 *oldPos = getPos(x, y, z);
	y = yt;

	bool powerTrack = false;
	bool haltTrack = false;
	if (tile == Tile::goldenRail_Id)
	{
		powerTrack = (data & BaseRailTile::RAIL_DATA_BIT) != 0;
		haltTrack = !powerTrack;
	}
	if (((BaseRailTile *) Tile::tiles[tile])->isUsesDataBit())
	{
		data &= BaseRailTile::RAIL_DIRECTION_MASK;
	}

	if (data >= 2 && data <= 5)
	{
		y = yt + 1;
	}

	if (data == 2) xd -= slideSpeed;
	if (data == 3) xd += slideSpeed;
	if (data == 4) zd += slideSpeed;
	if (data == 5) zd -= slideSpeed;

	int exits[2][3];
	memcpy( exits, EXITS[data], sizeof(int) * 2 * 3);

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
	if (pow > 2)
	{
		pow = 2;
	}

	xd = pow * xD / dd;
	zd = pow * zD / dd;


	if ( rider.lock() != NULL && rider.lock()->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(rider.lock());

		double forward = living->yya;

		if (forward > 0)
		{
			double riderXd = -sin(living->yRot * PI / 180);
			double riderZd = cos(living->yRot * PI / 180);

			double ownDist = xd * xd + zd * zd;

			if (ownDist < 0.01)
			{
				xd += riderXd * 0.1;
				zd += riderZd * 0.1;

				haltTrack = false;
			}
		}
	}

	// on golden rails without power, stop the cart
	if (haltTrack)
	{
		double speedLength = sqrt(xd * xd + zd * zd);
		if (speedLength < .03)
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
	if (xdd < -maxSpeed) xdd = -maxSpeed;
	if (xdd > +maxSpeed) xdd = +maxSpeed;
	if (zdd < -maxSpeed) zdd = -maxSpeed;
	if (zdd > +maxSpeed) zdd = +maxSpeed;

	move(xdd, 0, zdd);

	if (exits[0][1] != 0 && Mth::floor(x) - xt == exits[0][0] && Mth::floor(z) - zt == exits[0][2])
	{
		setPos(x, y + exits[0][1], z);
	}
	else if (exits[1][1] != 0 && Mth::floor(x) - xt == exits[1][0] && Mth::floor(z) - zt == exits[1][2])
	{
		setPos(x, y + exits[1][1], z);
	}

	applyNaturalSlowdown();

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
			// if the minecart is standing still, accelerate it away from
			// potential walls
			if (data == BaseRailTile::DIR_FLAT_X)
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
			else if (data == BaseRailTile::DIR_FLAT_Z)
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
}

void Minecart::applyNaturalSlowdown()
{
	if (rider.lock() != NULL)
	{
		xd *= 0.997f;
		yd *= 0;
		zd *= 0.997f;
	}
	else
	{
		xd *= 0.96f;
		yd *= 0;
		zd *= 0.96f;
	}
}

Vec3 *Minecart::getPosOffs(double x, double y, double z, double offs)
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);
	if (BaseRailTile::isRail(level, xt, yt - 1, zt))
	{
		yt--;
	}

	int tile = level->getTile(xt, yt, zt);
	if (BaseRailTile::isRail(tile))
	{
		int data = level->getData(xt, yt, zt);

		if (((BaseRailTile *) Tile::tiles[tile])->isUsesDataBit())
		{
			data &= BaseRailTile::RAIL_DIRECTION_MASK;
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

		return getPos(x, y, z);
	}
	return NULL;
}

Vec3 *Minecart::getPos(double x, double y, double z)
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(y);
	int zt = Mth::floor(z);
	if (BaseRailTile::isRail(level, xt, yt - 1, zt))
	{
		yt--;
	}

	int tile = level->getTile(xt, yt, zt);
	if (BaseRailTile::isRail(tile))
	{
		int data = level->getData(xt, yt, zt);
		y = yt;

		if (((BaseRailTile *) Tile::tiles[tile])->isUsesDataBit())
		{
			data &= BaseRailTile::RAIL_DIRECTION_MASK;
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

void Minecart::readAdditionalSaveData(CompoundTag *tag)
{
	if (tag->getBoolean(L"CustomDisplayTile"))
	{
		setDisplayTile(tag->getInt(L"DisplayTile"));
		setDisplayData(tag->getInt(L"DisplayData"));
		setDisplayOffset(tag->getInt(L"DisplayOffset"));
	}

	if (tag->contains(L"CustomName") && tag->getString(L"CustomName").length() > 0) name = tag->getString(L"CustomName");
}

void Minecart::addAdditonalSaveData(CompoundTag *tag)
{
	if (hasCustomDisplay())
	{
		tag->putBoolean(L"CustomDisplayTile", true);
		tag->putInt(L"DisplayTile", getDisplayTile() == NULL ? 0 : getDisplayTile()->id);
		tag->putInt(L"DisplayData", getDisplayData());
		tag->putInt(L"DisplayOffset", getDisplayOffset());
	}

	if (!name.empty()) tag->putString(L"CustomName", name);
}

float Minecart::getShadowHeightOffs()
{
	return 0;
}

void Minecart::push(shared_ptr<Entity> e)
{
	if (level->isClientSide) return;

	if (e == rider.lock()) return;
	if ( e->instanceof(eTYPE_LIVINGENTITY) && !e->instanceof(eTYPE_PLAYER) && !e->instanceof(eTYPE_VILLAGERGOLEM) && (getType() == TYPE_RIDEABLE) && (xd * xd + zd * zd > 0.01) )
	{
		if ( (rider.lock() == NULL) && (e->riding == NULL) )
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

		if (e->instanceof(eTYPE_MINECART))
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
			if (cart != NULL && cart->getType() == TYPE_FURNACE && getType() != TYPE_FURNACE)
			{
				xd *= 0.2f;
				zd *= 0.2f;
				push( e->xd - xa, 0, e->zd - za);
				e->xd *= 0.95f;
				e->zd *= 0.95f;
				m_bHasPushedCartThisTick = true;
			}
			else if (cart != NULL && cart->getType() != TYPE_FURNACE && getType() == TYPE_FURNACE)
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
				push(xdd - xa, 0, zdd - za);
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
			push(-xa, 0, -za);
			e->push(xa / 4, 0, za / 4);
		}
	}
}

void Minecart::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	lSteps = steps + 2;

	xd = lxd;
	yd = lyd;
	zd = lzd;
}

void Minecart::lerpMotion(double xd, double yd, double zd)
{
	lxd = this->xd = xd;
	lyd = this->yd = yd;
	lzd = this->zd = zd;
}

void Minecart::setDamage(float damage)
{
	entityData->set(DATA_ID_DAMAGE, damage);
}

float Minecart::getDamage()
{
	return entityData->getFloat(DATA_ID_DAMAGE);
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

Tile *Minecart::getDisplayTile()
{
	if (!hasCustomDisplay()) return getDefaultDisplayTile();
	int id = getEntityData()->getInteger(DATA_ID_DISPLAY_TILE) & 0xFFFF;
	return id > 0 && id < Tile::TILE_NUM_COUNT ? Tile::tiles[id] : NULL;
}

Tile *Minecart::getDefaultDisplayTile()
{
	return NULL;
}

int Minecart::getDisplayData()
{
	if (!hasCustomDisplay()) return getDefaultDisplayData();
	return getEntityData()->getInteger(DATA_ID_DISPLAY_TILE) >> 16;
}

int Minecart::getDefaultDisplayData()
{
	return 0;
}

int Minecart::getDisplayOffset()
{
	if (!hasCustomDisplay()) return getDefaultDisplayOffset();
	return getEntityData()->getInteger(DATA_ID_DISPLAY_OFFSET);
}

int Minecart::getDefaultDisplayOffset()
{
	return 6;
}

void Minecart::setDisplayTile(int id)
{
	getEntityData()->set(DATA_ID_DISPLAY_TILE, (id & 0xFFFF) | (getDisplayData() << 16));
	setCustomDisplay(true);
}

void Minecart::setDisplayData(int data)
{
	Tile *tile = getDisplayTile();
	int id = tile == NULL ? 0 : tile->id;

	getEntityData()->set(DATA_ID_DISPLAY_TILE, (id & 0xFFFF) | (data << 16));
	setCustomDisplay(true);
}

void Minecart::setDisplayOffset(int offset)
{
	getEntityData()->set(DATA_ID_DISPLAY_OFFSET, offset);
	setCustomDisplay(true);
}

bool Minecart::hasCustomDisplay()
{
	return getEntityData()->getByte(DATA_ID_CUSTOM_DISPLAY) == 1;
}

void Minecart::setCustomDisplay(bool value)
{
	getEntityData()->set(DATA_ID_CUSTOM_DISPLAY, (byte) (value ? 1 : 0));
}

void Minecart::setCustomName(const wstring &name)
{
	this->name = name;
}

wstring Minecart::getAName()
{
	if (!name.empty()) return name;
	return Entity::getAName();
}

bool Minecart::hasCustomName()
{
	return !name.empty();
}

wstring Minecart::getCustomName()
{
	return name;
}