#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.network.packet.h"
#include "../Minecraft.Client/ServerPlayer.h"
#include "../Minecraft.Client/PlayerConnection.h"
#include "com.mojang.nbt.h"
#include "Arrow.h"

// 4J : WESTY : Added for other award, kill creeper with arrow.
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.stats.h"
#include "SoundTypes.h"



// base damage, multiplied with velocity
const double Arrow::ARROW_BASE_DAMAGE = 2.0f;

// 4J - added common ctor code.
void Arrow::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	xTile = -1;
	yTile = -1;
	zTile = -1;
	lastTile = 0;
	lastData = 0;
	inGround = false;
	pickup = PICKUP_DISALLOWED;
	shakeTime = 0;
	flightTime = 0;

	owner = nullptr;
	life = 0;

	baseDamage = ARROW_BASE_DAMAGE;
	knockback = 0;
}


Arrow::Arrow(Level *level) : Entity( level )
{
	_init();
	
	viewScale = 10;
	setSize(0.5f, 0.5f);
}

Arrow::Arrow(Level *level, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> target, float power, float uncertainty) : Entity( level )
{
	_init();
	
	viewScale = 10;
	owner = mob;
	if ( mob->instanceof(eTYPE_PLAYER) ) pickup = PICKUP_ALLOWED;

	y = mob->y + mob->getHeadHeight() - 0.1f;

	double xd = target->x - mob->x;
	double yd = (target->y + target->getHeadHeight() - 0.7f) - y;
	double zd = target->z - mob->z;
	double sd = sqrt(xd * xd + zd * zd);
	if (sd < 0.0000001) return;

	float yRot = (float) (atan2(zd, xd) * 180 / PI) - 90;
	float xRot = (float) -(atan2(yd, sd) * 180 / PI);

	double xdn = xd / sd;
	double zdn = zd / sd;
	moveTo(mob->x + xdn, y, mob->z + zdn, yRot, xRot);
	heightOffset = 0;

	float yo = (float) sd * 0.2f;
	shoot(xd, yd + yo, zd, power, uncertainty);
}

Arrow::Arrow(Level *level, double x, double y, double z) : Entity( level )
{
	_init();
	
	viewScale = 10;
	setSize(0.5f, 0.5f);

	setPos(x, y, z);
	heightOffset = 0;
}
 
Arrow::Arrow(Level *level, shared_ptr<LivingEntity> mob, float power) : Entity( level )
{
	_init();

	viewScale = 10;
	owner = mob;
	if ( mob->instanceof(eTYPE_PLAYER) ) pickup = PICKUP_ALLOWED;

	setSize(0.5f, 0.5f);

	moveTo(mob->x, mob->y + mob->getHeadHeight(), mob->z, mob->yRot, mob->xRot);

	x -= Mth::cos(yRot / 180 * PI) * 0.16f;
	y -= 0.1f;
	z -= Mth::sin(yRot / 180 * PI) * 0.16f;
	setPos(x, y, z);
	heightOffset = 0;

	xd = -Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	zd = Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	yd = -Mth::sin(xRot / 180 * PI);

	shoot(xd, yd, zd, power * 1.5f, 1);
}


void Arrow::defineSynchedData()
{
	entityData->define(ID_FLAGS, (byte) 0);
}


void Arrow::shoot(double xd, double yd, double zd, float pow, float uncertainty)
{
	float dist = (float) sqrt(xd * xd + yd * yd + zd * zd);

	xd /= dist;
	yd /= dist;
	zd /= dist;

	xd += (random->nextGaussian() * (random->nextBoolean() ? -1 : 1)) * 0.0075f * uncertainty;
	yd += (random->nextGaussian() * (random->nextBoolean() ? -1 : 1)) * 0.0075f * uncertainty;
	zd += (random->nextGaussian() * (random->nextBoolean() ? -1 : 1)) * 0.0075f * uncertainty;

	xd *= pow;
	yd *= pow;
	zd *= pow;

	this->xd = xd;
	this->yd = yd;
	this->zd = zd;

	double sd = sqrt(xd * xd + zd * zd);

	yRotO = yRot = (float) (atan2(xd, zd) * 180 / PI);
	xRotO = xRot = (float) (atan2(yd, sd) * 180 / PI);
	life = 0;
}

void Arrow::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	setPos(x, y, z);
	setRot(yRot, xRot);
}

void Arrow::lerpMotion(double xd, double yd, double zd)
{
	this->xd = xd;
	this->yd = yd;
	this->zd = zd;
	if (xRotO == 0 && yRotO == 0)
	{
		double sd = sqrt(xd * xd + zd * zd);
		yRotO = yRot = (float) (atan2( xd, zd) * 180 / PI);
		xRotO = xRot = (float) (atan2( yd, sd) * 180 / PI);
		xRotO = xRot;
		yRotO = yRot;
		app.DebugPrintf("%f %f : 0x%x\n",xRot,yRot,&yRot);
		moveTo(x, y, z, yRot, xRot);
		life = 0;
	}
}

void Arrow::tick() 
{
	Entity::tick();


	if (xRotO == 0 && yRotO == 0) 
	{
		double sd = sqrt(xd * xd + zd * zd);
		yRotO = yRot = (float) (atan2(xd, zd) * 180 / PI);
		xRotO = xRot = (float) (atan2(yd, sd) * 180 / PI);
	}


	{
		int t = level->getTile(xTile, yTile, zTile);
		if (t > 0)
		{
			Tile::tiles[t]->updateShape(level, xTile, yTile, zTile);
			AABB *aabb = Tile::tiles[t]->getAABB(level, xTile, yTile, zTile);
			if (aabb != NULL && aabb->contains(Vec3::newTemp(x, y, z)))
			{
				inGround = true;
			}
		}

	}

	if (shakeTime > 0) shakeTime--;

	if (inGround)
	{
		int tile = level->getTile(xTile, yTile, zTile);
		int data = level->getData(xTile, yTile, zTile);
		if (tile != lastTile || data != lastData)
		{
			inGround = false;

			xd *= random->nextFloat() * 0.2f;
			yd *= random->nextFloat() * 0.2f;
			zd *= random->nextFloat() * 0.2f;
			life = 0;
			flightTime = 0;
			return;
		} 

		else 
		{
			life++;
			if (life == 20 * 60) remove();
			return;
		}
	} 
	
	else 
	{
		flightTime++;
	}

	Vec3 *from = Vec3::newTemp(x, y, z);
	Vec3 *to = Vec3::newTemp(x + xd, y + yd, z + zd);
	HitResult *res = level->clip(from, to, false, true);

	from = Vec3::newTemp(x, y, z);
	to = Vec3::newTemp(x + xd, y + yd, z + zd);
	if (res != NULL)
	{
		to = Vec3::newTemp(res->pos->x, res->pos->y, res->pos->z);
	}
	shared_ptr<Entity> hitEntity = nullptr;
	vector<shared_ptr<Entity> > *objects = level->getEntities(shared_from_this(), this->bb->expand(xd, yd, zd)->grow(1, 1, 1));
	double nearest = 0;
	for ( auto& e : *objects )
	{
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

	if ( (res != NULL) && (res->entity != NULL) && res->entity->instanceof(eTYPE_PLAYER))
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(res->entity);
		// 4J: Check for owner being null
		if ( player->abilities.invulnerable || ((owner != NULL) && (owner->instanceof(eTYPE_PLAYER) && !dynamic_pointer_cast<Player>(owner)->canHarmPlayer(player))))
		{
			res = NULL;
		}
	}

	if (res != NULL)
	{
		if (res->entity != NULL)
		{
			float pow = Mth::sqrt(xd * xd + yd * yd + zd * zd);
			int dmg = (int) Mth::ceil((float)(pow * baseDamage));

			if(isCritArrow()) dmg += random->nextInt(dmg / 2 + 2);

			DamageSource *damageSource = NULL;
			if (owner == NULL)
			{
				damageSource = DamageSource::arrow(dynamic_pointer_cast<Arrow>(shared_from_this()), shared_from_this());
			}
			else
			{
				damageSource = DamageSource::arrow(dynamic_pointer_cast<Arrow>(shared_from_this()), owner);
			}

			if(res->entity->hurt(damageSource, dmg))
			{
				// Firx for #67839 - Customer Encountered: Bows enchanted with "Flame" still set things on fire if pvp/attack animals is turned off
				// 4J Stu - We should not set the entity on fire unless we can cause some damage (this doesn't necessarily mean that the arrow hit lowered their health)
				// set targets on fire first because we want cooked
				// pork/chicken/steak
				if (isOnFire() && res->entity->GetType() != eTYPE_ENDERMAN)
				{
					res->entity->setOnFire(5);
				}

				if (res->entity->instanceof(eTYPE_LIVINGENTITY))
				{
					shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(res->entity);

					if (!level->isClientSide)
					{
						mob->setArrowCount(mob->getArrowCount() + 1);
					}
					if (knockback > 0)
					{
						float pushLen = sqrt(xd * xd + zd * zd);
						if (pushLen > 0)
						{
							res->entity->push(xd * knockback * .6f / pushLen, 0.1, zd * knockback * .6f / pushLen);
						}
					}

					if (owner != NULL)
					{
						ThornsEnchantment::doThornsAfterAttack(owner, mob, random);
					}

					if (owner != NULL && res->entity != owner && owner->GetType() == eTYPE_SERVERPLAYER)
					{
						dynamic_pointer_cast<ServerPlayer>(owner)->connection->send( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::SUCCESSFUL_BOW_HIT, 0)) );
					}
				}

				// 4J : WESTY : For award, need to track if creeper was killed by arrow from the player.
				if (owner != NULL && owner->instanceof(eTYPE_PLAYER)				// arrow owner is a player
					&&	!res->entity->isAlive()						// target is now dead
					&&	(res->entity->GetType() == eTYPE_CREEPER))	// target is a creeper

				{
					dynamic_pointer_cast<Player>(owner)->awardStat(
						GenericStats::arrowKillCreeper(),
						GenericStats::param_arrowKillCreeper()
						);
				}

				playSound( eSoundType_RANDOM_BOW_HIT, 1.0f, 1.2f / (random->nextFloat() * 0.2f + 0.9f));
				if (res->entity->GetType() != eTYPE_ENDERDRAGON) remove();
			}			
			else
			{
				xd *= -0.1f;
				yd *= -0.1f;
				zd *= -0.1f;
				yRot += 180;
				yRotO += 180;
				flightTime = 0;
			}
			
			delete damageSource;
		}
		else
		{
			xTile = res->x;
			yTile = res->y;
			zTile = res->z;
			lastTile = level->getTile(xTile, yTile, zTile);
			lastData = level->getData(xTile, yTile, zTile);
			xd = (float) (res->pos->x - x);
			yd = (float) (res->pos->y - y);
			zd = (float) (res->pos->z - z);
			float dd = (float) sqrt(xd * xd + yd * yd + zd * zd);
			// 4J added check - zero dd here was creating NaNs
			if( dd > 0.0001f )
			{
				x -= (xd / dd) * 0.05f;
				y -= (yd / dd) * 0.05f;
				z -= (zd / dd) * 0.05f;
			}

			playSound(eSoundType_RANDOM_BOW_HIT, 1.0f, 1.2f / (random->nextFloat() * 0.2f + 0.9f));
			inGround = true;
			shakeTime = 7;
			setCritArrow(false);

			if (lastTile != 0)
			{
				Tile::tiles[lastTile]->entityInside(level, xTile, yTile, zTile, shared_from_this() );
			}
		}
	}
	delete res;

	if(isCritArrow())
	{
		for (int i = 0; i < 4; i++)
		{
			level->addParticle(eParticleType_crit, x + xd * i / 4.0f, y + yd * i / 4.0f, z + zd * i / 4.0f, -xd, -yd + 0.2, -zd);
		}
	}

	x += xd;
	y += yd;
	z += zd;

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


	float inertia = 0.99f;
	float gravity = 0.05f;

	if (isInWater())
	{
		for (int i = 0; i < 4; i++)
		{
			float s = 1 / 4.0f;
			level->addParticle(eParticleType_bubble, x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
		}
		inertia = 0.80f;
	}

	xd *= inertia;
	yd *= inertia;
	zd *= inertia;
	yd -= gravity;

	setPos(x, y, z);

	checkInsideTiles();
}

void Arrow::addAdditonalSaveData(CompoundTag *tag)
{
	tag->putShort(L"xTile", (short) xTile);
	tag->putShort(L"yTile", (short) yTile);
	tag->putShort(L"zTile", (short) zTile);
	tag->putByte(L"inTile", (byte) lastTile);
	tag->putByte(L"inData", (byte) lastData);
	tag->putByte(L"shake", (byte) shakeTime);
	tag->putByte(L"inGround", (byte) (inGround ? 1 : 0));
	tag->putByte(L"pickup", (byte) pickup);
	tag->putDouble(L"damage", baseDamage);
}

void Arrow::readAdditionalSaveData(CompoundTag *tag)
{
	xTile = tag->getShort(L"xTile");
	yTile = tag->getShort(L"yTile");
	zTile = tag->getShort(L"zTile");
	lastTile = tag->getByte(L"inTile") & 0xff;
	lastData = tag->getByte(L"inData") & 0xff;
	shakeTime = tag->getByte(L"shake") & 0xff;
	inGround = tag->getByte(L"inGround") == 1;
	if (tag->contains(L"damage"))
	{
		baseDamage = tag->getDouble(L"damage");
	}

	if (tag->contains(L"pickup"))
	{
		pickup = tag->getByte(L"pickup");
	}
	else if (tag->contains(L"player"))
	{
		pickup = tag->getBoolean(L"player") ? PICKUP_ALLOWED : PICKUP_DISALLOWED;
	}
}

void Arrow::playerTouch(shared_ptr<Player> player)
{
	if (level->isClientSide || !inGround || shakeTime > 0) return;

	bool bRemove = pickup == PICKUP_ALLOWED || (pickup == PICKUP_CREATIVE_ONLY && player->abilities.instabuild);

	if (pickup == PICKUP_ALLOWED)
	{
		if (!player->inventory->add( shared_ptr<ItemInstance>( new ItemInstance(Item::arrow, 1) ) ))
		{
			bRemove = false;
		}
	}

	if (bRemove)
	{
		playSound(eSoundType_RANDOM_POP, 0.2f, ((random->nextFloat() - random->nextFloat()) * 0.7f + 1.0f) * 2.0f);
		player->take(shared_from_this(), 1);
		remove();
	}
}

bool Arrow::makeStepSound()
{
	return false;
}

float Arrow::getShadowHeightOffs()
{
	return 0;
}

void Arrow::setBaseDamage(double baseDamage)
{
	this->baseDamage = baseDamage;
}

double Arrow::getBaseDamage()
{
	return baseDamage;
}

void Arrow::setKnockback(int knockback)
{
	this->knockback = knockback;
}

bool Arrow::isAttackable()
{
	return false;
}

void Arrow::setCritArrow(bool critArrow)
{
	byte flags = entityData->getByte(ID_FLAGS);
	if (critArrow)
	{
		entityData->set(ID_FLAGS, (byte) (flags | FLAG_CRIT));
	}
	else
	{
		entityData->set(ID_FLAGS, (byte) (flags & ~FLAG_CRIT));
	}
}

bool Arrow::isCritArrow()
{
	byte flags = entityData->getByte(ID_FLAGS);
	return (flags & FLAG_CRIT) != 0;
}
