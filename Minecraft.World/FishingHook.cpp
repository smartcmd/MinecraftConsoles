#include "stdafx.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "FishingHook.h"
#include "SoundTypes.h"



// 4J - added common ctor code.
void FishingHook::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	xTile = -1;
	yTile = -1;
	zTile = -1;
	lastTile = 0;
	inGround = false;
	shakeTime = 0;
	flightTime = 0;
	nibble = 0;
	hookedIn = nullptr;

	lSteps = 0;
	lx = 0.0;
	ly = 0.0;
	lz = 0.0;
	lyr = 0.0;
	lxr = 0.0;
	lxd = 0.0;
	lyd = 0.0; 
	lzd = 0.0;
	owner = nullptr;
	life = 0;

	setSize(0.25f, 0.25f);
	noCulling = true;
}

FishingHook::FishingHook(Level *level) : Entity( level )
{
	_init();
}

FishingHook::FishingHook(Level *level, double x, double y, double z, shared_ptr<Player> owner) : Entity( level )
{
	_init();

	this->owner = owner;
	// 4J Stu - Moved this outside the ctor
	//owner->fishing = dynamic_pointer_cast<FishingHook>( shared_from_this() );

	setPos(x, y, z);
}

FishingHook::FishingHook(Level *level, shared_ptr<Player> mob) : Entity( level )
{
	_init();

	this->owner = mob;
	// 4J Stu - Moved this outside the ctor
	//owner->fishing = dynamic_pointer_cast<FishingHook>( shared_from_this() );

	this->moveTo(mob->x, mob->y + 1.62 - mob->heightOffset, mob->z, mob->yRot, mob->xRot);


	x -= Mth::cos(yRot / 180 * PI) * 0.16f;
	y -= 0.1f;
	z -= Mth::sin(yRot / 180 * PI) * 0.16f;
	this->setPos(x, y, z);
	this->heightOffset = 0;


	float speed = 0.4f;
	xd = (-Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI)) * speed;
	zd = (Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI)) * speed;
	yd = (-Mth::sin(xRot / 180 * PI)) * speed;

	shoot(xd, yd, zd, 1.5f, 1);
}

void FishingHook::defineSynchedData()
{
}

bool FishingHook::shouldRenderAtSqrDistance(double distance)
{
	double size = bb->getSize() * 4;
	size *= 64.0f;
	return distance < size * size;
}

void FishingHook::shoot(double xd, double yd, double zd, float pow, float uncertainty)
{
	float dist = (float) sqrt(xd * xd + yd * yd + zd * zd);

	xd /= dist;
	yd /= dist;
	zd /= dist;

	xd += (random->nextGaussian()) * 0.0075f * uncertainty;
	yd += (random->nextGaussian()) * 0.0075f * uncertainty;
	zd += (random->nextGaussian()) * 0.0075f * uncertainty;

	xd *= pow;
	yd *= pow;
	zd *= pow;

	this->xd = xd;
	this->yd = yd;
	this->zd = zd;

	double sd = sqrt(xd * xd + zd * zd);

	yRotO = this->yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRotO = this->xRot = (float) (atan2(yd, sd) * 180 / PI);
	life = 0;
}

void FishingHook::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	lSteps = steps;

	this->xd = lxd;
	this->yd = lyd;
	this->zd = lzd;
}

void FishingHook::lerpMotion(double xd, double yd, double zd)
{
	lxd = this->xd = xd;
	lyd = this->yd = yd;
	lzd = this->zd = zd;
}

void FishingHook::tick()
{
	Entity::tick();

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
		return;
	}

	if (!level->isClientSide)
	{
		shared_ptr<ItemInstance> selectedItem = owner->getSelectedItem();
		if (owner->removed || !owner->isAlive() || selectedItem == NULL || selectedItem->getItem() != Item::fishingRod || this->distanceToSqr(owner) > 32 * 32)
		{
			remove();
			owner->fishing = nullptr;
			return;
		}

		if (hookedIn != NULL)
		{
			if (hookedIn->removed) hookedIn = nullptr;
			else
			{
				x = hookedIn->x;
				y = hookedIn->bb->y0 + hookedIn->bbHeight * 0.8;
				z = hookedIn->z;
				return;
			}
		}
	}

	if (shakeTime > 0) shakeTime--;

	if (inGround) 
	{
		int tile = level->getTile(xTile, yTile, zTile);
		if (tile != lastTile)
		{
			life++;
			if (life == 20 * 60) remove();
			return;
		}
		else
		{
			inGround = false;

			xd *= random->nextFloat() * 0.2f;
			yd *= random->nextFloat() * 0.2f;
			zd *= random->nextFloat() * 0.2f;
			life = 0;
			flightTime = 0;
		}
	}
	else
	{
		flightTime++;
	}

	Vec3 *from = Vec3::newTemp(x, y, z);
	Vec3 *to = Vec3::newTemp(x + xd, y + yd, z + zd);
	HitResult *res = level->clip(from, to);

	from = Vec3::newTemp(x, y, z);
	to = Vec3::newTemp(x + xd, y + yd, z + zd);
	if (res != NULL) 
	{
		to = Vec3::newTemp(res->pos->x, res->pos->y, res->pos->z);
	}
	shared_ptr<Entity> hitEntity = nullptr;
	vector<shared_ptr<Entity> > *objects = level->getEntities(shared_from_this(), this->bb->expand(xd, yd, zd)->grow(1, 1, 1));
	double nearest = 0;
	AUTO_VAR(itEnd, objects->end());
	for (AUTO_VAR(it, objects->begin()); it != itEnd; it++)
	{
		shared_ptr<Entity> e = *it; // objects->at(i);
		if (!e->isPickable() || (e == owner && flightTime < 5)) continue;

		float rr = 0.3f;
		AABB *bb = e->bb->grow(rr, rr, rr);
		HitResult *p = bb->clip(from, to);
		if (p != NULL)
		{
			double dd = from->distanceTo(p->pos);
			if (dd < nearest || nearest == 0)
			{
				hitEntity = e;
				nearest = dd;
			}
			delete p;
		}
	}

	if (hitEntity != NULL)
	{
		delete res;
		res = new HitResult(hitEntity);
	}

	if (res != NULL)
	{
		if (res->entity != NULL) 
		{
			// 4J Stu Move fix for : fix for #48587 - CRASH: Code: Gameplay: Hitting another player with the fishing bobber crashes the game. [Fishing pole, line]
			// Incorrect dynamic_pointer_cast used around the shared_from_this()
			DamageSource *damageSource = DamageSource::thrown(shared_from_this(), owner);
			if (res->entity->hurt(damageSource, 0))
			{
				hookedIn = res->entity;
			}
			delete damageSource;
		}
		else
		{
			inGround = true;
		}
	}
	delete res;

	if (inGround) return;

	move(xd, yd, zd);

	double sd = sqrt(xd * xd + zd * zd);
	yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRot = (float) (atan2(yd, sd) * 180 / PI);

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	xRot = xRotO + (xRot - xRotO) * 0.2f;
	yRot = yRotO + (yRot - yRotO) * 0.2f;


	float inertia = 0.92f;

	if (onGround || horizontalCollision)
	{
		inertia = 0.5f;
	}

	int steps = 5;
	double waterPercentage = 0;
	for (int i = 0; i < steps; i++)
	{
		double y0 = bb->y0 + (bb->y1 - bb->y0) * (i + 0) / steps - 2 / 16.0f + 2 / 16.0f;
		double y1 = bb->y0 + (bb->y1 - bb->y0) * (i + 1) / steps - 2 / 16.0f + 2 / 16.0f;
		AABB *bb2 = AABB::newTemp(bb->x0, y0, bb->z0, bb->x1, y1, bb->z1);
		if (level->containsLiquid(bb2, Material::water))
		{
			waterPercentage += 1.0 / steps;
		}
	}

	if (waterPercentage > 0)
	{
		if (nibble > 0)
		{
			nibble--;
		} 
		else
		{
			int nibbleOdds = 500;
			if (level->isRainingAt( Mth::floor(x), Mth::floor(y) + 1, Mth::floor(z))) nibbleOdds = 300;

			if (random->nextInt(nibbleOdds) == 0)
			{
				nibble = random->nextInt(30) + 10;
				yd -= 0.2f;
				level->playSound(shared_from_this(), eSoundType_RANDOM_SPLASH, 0.25f, 1 + (random->nextFloat() - random->nextFloat()) * 0.4f);
				float yt = (float) Mth::floor(bb->y0);
				for (int i = 0; i < 1 + bbWidth * 20; i++)
				{
					float xo = (random->nextFloat() * 2 - 1) * bbWidth;
					float zo = (random->nextFloat() * 2 - 1) * bbWidth;
					level->addParticle(eParticleType_bubble, x + xo, yt + 1, z + zo, xd, yd - random->nextFloat() * 0.2f, zd);
				}
				for (int i = 0; i < 1 + bbWidth * 20; i++)
				{
					float xo = (random->nextFloat() * 2 - 1) * bbWidth;
					float zo = (random->nextFloat() * 2 - 1) * bbWidth;
					level->addParticle(eParticleType_splash, x + xo, yt + 1, z + zo, xd, yd, zd);
				}
			}
		}

	}

	if (nibble > 0) 
	{
		yd -= random->nextFloat() * random->nextFloat() * random->nextFloat() * 0.2;
	}

	double bob = waterPercentage * 2 - 1;
	yd += 0.04f * bob;
	if (waterPercentage > 0)
	{
		inertia *= 0.9;
		yd *= 0.8;
	}

	xd *= inertia;
	yd *= inertia;
	zd *= inertia;

	setPos(x, y, z);
}

void FishingHook::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putShort(L"xTile", (short) xTile);
	tag->putShort(L"yTile", (short) yTile);
	tag->putShort(L"zTile", (short) zTile);
	tag->putByte(L"inTile", (byte) lastTile);
	tag->putByte(L"shake", (byte) shakeTime);
	tag->putByte(L"inGround", (byte) (inGround ? 1 : 0));
}

void FishingHook::readAdditionalSaveData(CompoundTag *tag)
{
	xTile = tag->getShort(L"xTile");
	yTile = tag->getShort(L"yTile");
	zTile = tag->getShort(L"zTile");
	lastTile = tag->getByte(L"inTile") & 0xff;
	shakeTime = tag->getByte(L"shake") & 0xff;
	inGround = tag->getByte(L"inGround") == 1;
}

float FishingHook::getShadowHeightOffs()
{
	return 0;
}

int FishingHook::retrieve()
{
	if (level->isClientSide) return 0;

	int dmg = 0;
	if (hookedIn != NULL)
	{
		double xa = owner->x - x;
		double ya = owner->y - y;
		double za = owner->z - z;

		double dist = sqrt(xa * xa + ya * ya + za * za);
		double speed = 0.1;
		hookedIn->xd += xa * speed;
		hookedIn->yd += ya * speed + sqrt(dist) * 0.08;
		hookedIn->zd += za * speed;
		dmg = 3;
	}
	else if (nibble > 0)
	{
		shared_ptr<ItemEntity> ie = shared_ptr<ItemEntity>( new ItemEntity(this->Entity::level, x, y, z, shared_ptr<ItemInstance>( new ItemInstance(Item::fish_raw) ) ) );
		double xa = owner->x - x;
		double ya = owner->y - y;
		double za = owner->z - z;

		double dist = sqrt(xa * xa + ya * ya + za * za);
		double speed = 0.1;
		ie->Entity::xd = xa * speed;
		ie->Entity::yd = ya * speed + sqrt(dist) * 0.08;
		ie->Entity::zd = za * speed;
		level->addEntity(ie);
		owner->level->addEntity( shared_ptr<ExperienceOrb>( new ExperienceOrb(owner->level, owner->x, owner->y + 0.5f, owner->z + 0.5f, random->nextInt(3) + 1) ) ); // 4J Stu brought forward from 1.4
		dmg = 1;
	}
	if (inGround) dmg = 2;

	remove();
	owner->fishing = nullptr;
	return dmg;
}

// 4J Stu - Brought forward from 1.4
void FishingHook::remove()
{
	Entity::remove();
	if (owner != NULL) owner->fishing = nullptr;
}