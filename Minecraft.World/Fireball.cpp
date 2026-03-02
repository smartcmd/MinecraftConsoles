#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "com.mojang.nbt.h"
#include "Fireball.h"
#include "net.minecraft.world.level.dimension.h"
#include "SharedConstants.h"


// 4J - added common ctor code.
void Fireball::_init()
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	lastTile = 0;
	inGround = false;
	flightTime = 0;

	life = 0;
	owner = nullptr;
	xPower = 0.0;
	yPower = 0.0;
	zPower = 0.0;
}

Fireball::Fireball(Level *level) : Entity( level ) 
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	_init();

    setSize(16 / 16.0f, 16 / 16.0f);

}

void Fireball::defineSynchedData()
{

}

bool Fireball::shouldRenderAtSqrDistance(double distance)
{
    double size = bb->getSize() * 4;
    size *= 64.0f;
    return distance < size * size;
}


Fireball::Fireball(Level *level, double x, double y, double z, double xa, double ya, double za) : Entity( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	_init();

    setSize(16 / 16.0f, 16 / 16.0f);

    this->moveTo(x, y, z, yRot, xRot);
    this->setPos(x, y, z);

    double dd = sqrt(xa * xa + ya * ya + za * za);

	// Fix for #69150 - [CRASH] TU8: Code: Gameplay: Nether portal mechanics can become permanently broken, causing a hard lock upon usage.
	// IF xa, ya and za are 0 then dd is 0 and the xa/dd etc return NAN
	if(dd == 0.0)
	{
		xPower = 0.0;
		yPower = 0.0;
		zPower = 0.0;
	}
	else
	{
		xPower = xa / dd * 0.10;
		yPower = ya / dd * 0.10;
		zPower = za / dd * 0.10;
	}
}

Fireball::Fireball(Level *level, shared_ptr<Mob> mob, double xa, double ya, double za) : Entity ( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	_init();

    this->owner = mob;

    setSize(16 / 16.0f, 16 / 16.0f);

    this->moveTo(mob->x, mob->y, mob->z, mob->yRot, mob->xRot);
    this->setPos(x, y, z);
    this->heightOffset = 0;


    xd = yd = zd = 0.0;

    xa += random->nextGaussian() * 0.4;
    ya += random->nextGaussian() * 0.4;
    za += random->nextGaussian() * 0.4;
    double dd = sqrt(xa * xa + ya * ya + za * za);

	// Fix for #69150 - [CRASH] TU8: Code: Gameplay: Nether portal mechanics can become permanently broken, causing a hard lock upon usage.
	// IF xa, ya and za are 0 then dd is 0 and the xa/dd etc return NAN
	if(dd == 0.0)
	{
		xPower = 0.0;
		yPower = 0.0;
		zPower = 0.0;
	}
	else
	{
		xPower = xa / dd * 0.10;
		yPower = ya / dd * 0.10;
		zPower = za / dd * 0.10;
	}
}

void Fireball::tick()
{
	// 4J-PB - Moved forward from 1.2.3
	//if (!level->isClientSide && (owner == NULL || owner->removed))
	if (!level->isClientSide)
	{
		if((owner != NULL && owner->removed) || !level->hasChunkAt((int) x, (int) y, (int) z))
		{
			app.DebugPrintf("Fireball removed - owner is null or removed is true for owner\n");
			remove();
			return;
		}
		else
		{
			// 4J-PB - TU9 bug fix - fireballs can hit the edge of the world, and stay there
			int minXZ = - (level->dimension->getXZSize() * 16 ) / 2;
			int maxXZ = (level->dimension->getXZSize() * 16 ) / 2 - 1;

			if ((x<=minXZ) || (x>=maxXZ) || (z<=minXZ) || (z>=maxXZ)) 
			{
				remove();
				app.DebugPrintf("Fireball removed - end of world\n");
				return;
			}
		}
	}

    Entity::tick();

	//app.DebugPrintf("Fireball x %d, y %d, z%d\n",(int)x,(int)y,(int)z);

    if(shouldBurn()) setOnFire(1);

    if (inGround)
	{
        int tile = level->getTile(xTile, yTile, zTile);
        if (tile == lastTile)
		{
            life++;
            if (life == SharedConstants::TICKS_PER_SECOND * 30) 
			{
				remove();
				app.DebugPrintf("Fireball removed - life is 20*60\n");
			}
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

	MemSect(41);
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
        shared_ptr<Entity> e = *it; //objects->at(i);
        if (!e->isPickable() || (e->is(owner) )) continue; //4J Stu - Never collide with the owner (Enderdragon) // && flightTime < 25)) continue;

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
	MemSect(0);

    if (res != NULL)
	{
		onHit(res);
    }
	delete res;
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


    float inertia = 0.95f;
    if (isInWater())
	{
        for (int i = 0; i < 4; i++) 
		{
            float s = 1 / 4.0f;
            level->addParticle(eParticleType_bubble, x - xd * s, y - yd * s, z - zd * s, xd, yd, zd);
        }
        inertia = 0.80f;
    }

    xd += xPower;
    yd += yPower;
    zd += zPower;
    xd *= inertia;
    yd *= inertia;
    zd *= inertia;

	// 4J-PB - bug fix for the fireballs in a saved game - they are saved with no/very small velocity, so end up hanging around in the air
	if (!level->isClientSide)
	{
		if((abs(xd)<0.002) && (abs(yd)<0.002) && (abs(zd)<0.002))
		{
			xd=0.0;
			zd=0.0;
			yd=0.0;
			app.DebugPrintf("Removing a fireball with zero velocity\n");
			remove();
		}
	}

    level->addParticle(getTrailParticleType(), x, y + 0.5f, z, 0, 0.01, 0);

    setPos(x, y, z);
}

void Fireball::onHit(HitResult *res)
{
	if (!level->isClientSide)
	{
		if (res->entity != NULL)
		{
			DamageSource *damageSource = DamageSource::fireball(dynamic_pointer_cast<Fireball>( shared_from_this() ), owner);
			if (res->entity->hurt(damageSource, 6))
			{
			}
			else
			{
			}
			delete damageSource;
		}

		bool destroyBlocks = true;//level.getGameRules().getBoolean(GameRules.RULE_MOBGRIEFING);
		level->explode(nullptr, x, y, z, 1, true, destroyBlocks);

		remove();
	}
}

void Fireball::addAdditonalSaveData(CompoundTag *tag)
{
    tag->putShort(L"xTile", (short) xTile);
    tag->putShort(L"yTile", (short) yTile);
    tag->putShort(L"zTile", (short) zTile);
    tag->putByte(L"inTile", (byte) lastTile);
    tag->putByte(L"inGround", (byte) (inGround ? 1 : 0));
	tag->put(L"direction", this->newDoubleList(3, this->xd, this->yd, this->zd));
}

void Fireball::readAdditionalSaveData(CompoundTag *tag)
{
    xTile = tag->getShort(L"xTile");
    yTile = tag->getShort(L"yTile");
    zTile = tag->getShort(L"zTile");
    lastTile = tag->getByte(L"inTile") & 0xff;
    inGround = tag->getByte(L"inGround") == 1;

	// Load the stored direction and apply it to the fireball
	//   if it has no stored direction, remove it.
	if (tag->contains(L"direction"))
	{
		ListTag<DoubleTag> *listTag = (ListTag<DoubleTag> *)tag->getList(L"direction");
		this->xd = ((DoubleTag *) listTag->get(0))->data;
		this->yd = ((DoubleTag *) listTag->get(1))->data;
		this->zd = ((DoubleTag *) listTag->get(2))->data;
	}
	else
	{
		this->remove();
	}
}

bool Fireball::isPickable()
{
    return true;
}

float Fireball::getPickRadius()
{
    return 1;
}

bool Fireball::hurt(DamageSource *source, int damage)
{
    markHurt();

    if (source->getEntity() != NULL)
	{
        Vec3 *lookAngle = source->getEntity()->getLookAngle();
        if (lookAngle != NULL)
		{
            xd = lookAngle->x;
            yd = lookAngle->y;
            zd = lookAngle->z;
            xPower = xd * 0.1;
            yPower = yd * 0.1;
            zPower = zd * 0.1;
        }
		shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>( source->getEntity() );
		if (mob != NULL)
		{
			owner = mob;
		}
        return true;
    }
    return false;
}

float Fireball::getShadowHeightOffs() 
{
    return 0;
}

float Fireball::getBrightness(float a)
{
	return 1.0f;
}

int Fireball::getLightColor(float a)
{
	return 15 << 20 | 15 << 4;
}

bool Fireball::shouldBurn()
{
	return true;
}

int Fireball::getIcon()
{
	return 14 + 2 * 16;
}

ePARTICLE_TYPE Fireball::getTrailParticleType()
{
	return eParticleType_smoke;
}