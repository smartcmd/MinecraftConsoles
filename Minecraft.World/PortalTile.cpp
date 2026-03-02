#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.item.h"
#include "PortalTile.h"
#include "FireTile.h"

PortalTile::PortalTile(int id) : HalfTransparentTile(id, L"portal", Material::portal, false)
{
	setTicking(true);
}

void PortalTile::tick(Level *level, int x, int y, int z, Random *random)
{
	HalfTransparentTile::tick(level, x, y, z, random);

	if (level->dimension->isNaturalDimension() && random->nextInt(2000) < level->difficulty)
	{
		// locate floor
		int y0 = y;
		while (!level->isTopSolidBlocking(x, y0, z) && y0 > 0)
		{
			y0--;
		}
		if (y0 > 0 && !level->isSolidBlockingTile(x, y0 + 1, z))
		{
			// spawn a pig man here
			int result = 0;
			bool spawned = MonsterPlacerItem::spawnMobAt(level, 57, x + .5, y0 + 1.1, z + .5, &result) != NULL;
		}
	}
}

AABB *PortalTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

void PortalTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
    if (level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id)
	{
        float xr = 8 / 16.0f;
        float yr = 2 / 16.0f;
        this->setShape(0.5f - xr, 0, 0.5f - yr, 0.5f + xr, 1, 0.5f + yr);
    }
	else
	{
        float xr = 2 / 16.0f;
        float yr = 8 / 16.0f;
        this->setShape(0.5f - xr, 0, 0.5f - yr, 0.5f + xr, 1, 0.5f + yr);
    }
}

bool PortalTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PortalTile::isCubeShaped()
{
	return false;
}

bool PortalTile::trySpawnPortal(Level *level, int x, int y, int z, bool actuallySpawn)
{
    int xd = 0;
    int zd = 0;
    if (level->getTile(x - 1, y, z) == Tile::obsidian_Id || level->getTile(x + 1, y, z) == Tile::obsidian_Id) xd = 1;
    if (level->getTile(x, y, z - 1) == Tile::obsidian_Id || level->getTile(x, y, z + 1) == Tile::obsidian_Id) zd = 1;

    if (xd == zd) return false;

    if (level->getTile(x - xd, y, z - zd) == 0)
	{
        x -= xd;
        z -= zd;
    }

    for (int xx = -1; xx <= 2; xx++)
	{
        for (int yy = -1; yy <= 3; yy++)
		{
            bool edge = (xx == -1) || (xx == 2) || (yy == -1) || (yy == 3);
            if ((xx == -1 || xx == 2) && (yy == -1 || yy == 3)) continue;

            int t = level->getTile(x + xd * xx, y + yy, z + zd * xx);

            if (edge)
			{
                if (t != Tile::obsidian_Id) return false;
            }
			else
			{
                if (t != 0 && t != Tile::fire_Id) return false;
            }
        }
    }

	if( !actuallySpawn )
		return true;

    level->noNeighborUpdate = true;
    for (int xx = 0; xx < 2; xx++)
	{
        for (int yy = 0; yy < 3; yy++)
		{
            level->setTile(x + xd * xx, y + yy, z + zd * xx, Tile::portalTile_Id);
        }
    }
    level->noNeighborUpdate = false;

    return true;

}

void PortalTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    int xd = 0;
    int zd = 1;
    if (level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id)
	{
        xd = 1;
        zd = 0;
    }

    int yBottom = y;
    while (level->getTile(x, yBottom - 1, z) == id)
        yBottom--;

    if (level->getTile(x, yBottom - 1, z) != Tile::obsidian_Id)
	{
        level->setTile(x, y, z, 0);
        return;
    }

    int height = 1;
    while (height < 4 && level->getTile(x, yBottom + height, z) == id)
        height++;

    if (height != 3 || level->getTile(x, yBottom + height, z) != Tile::obsidian_Id)
	{
        level->setTile(x, y, z, 0);
        return;
    }

    bool we = level->getTile(x - 1, y, z) == id || level->getTile(x + 1, y, z) == id;
    bool ns = level->getTile(x, y, z - 1) == id || level->getTile(x, y, z + 1) == id;
    if (we && ns)
	{
        level->setTile(x, y, z, 0);
        return;
    }

    if (!(//
    (level->getTile(x + xd, y, z + zd) == Tile::obsidian_Id && level->getTile(x - xd, y, z - zd) == id) || //
    (level->getTile(x - xd, y, z - zd) == Tile::obsidian_Id && level->getTile(x + xd, y, z + zd) == id)//
    ))
	{
        level->setTile(x, y, z, 0);
        return;
    }

}

bool PortalTile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
    if (level->getTile(x, y, z) == id) return false;

    bool w = level->getTile(x - 1, y, z) == id && level->getTile(x - 2, y, z) != id;
    bool e = level->getTile(x + 1, y, z) == id && level->getTile(x + 2, y, z) != id;

    bool n = level->getTile(x, y, z - 1) == id && level->getTile(x, y, z - 2) != id;
    bool s = level->getTile(x, y, z + 1) == id && level->getTile(x, y, z + 2) != id;

    bool we = w || e;
    bool ns = n || s;

    if (we && face == 4) return true;
    if (we && face == 5) return true;
    if (ns && face == 2) return true;
    if (ns && face == 3) return true;

    return false;
}

int PortalTile::getResourceCount(Random *random)
{
	return 0;
}

int PortalTile::getRenderLayer()
{
	return 1;
}

void PortalTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (entity->riding == NULL && entity->rider.lock() == NULL) entity->handleInsidePortal();
}

void PortalTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
    if (random->nextInt(100) == 0)
	{
        level->playLocalSound(xt + 0.5, yt + 0.5, zt + 0.5, eSoundType_PORTAL_PORTAL, 0.5f, random->nextFloat() * 0.4f + 0.8f);
    }
    for (int i = 0; i < 4; i++)
	{
        double x = xt + random->nextFloat();
        double y = yt + random->nextFloat();
        double z = zt + random->nextFloat();
        double xa = 0;
        double ya = 0;
        double za = 0;
        int flip = random->nextInt(2) * 2 - 1;
        xa = (random->nextFloat() - 0.5) * 0.5;
        ya = (random->nextFloat() - 0.5) * 0.5;
        za = (random->nextFloat() - 0.5) * 0.5;
        if (level->getTile(xt - 1, yt, zt) == id || level->getTile(xt + 1, yt, zt) == id)
		{
            z = zt + 0.5 + (0.25) * flip;
            za = (random->nextFloat() * 2) * flip;
        }
		else
		{
            x = xt + 0.5 + (0.25) * flip;
            xa = (random->nextFloat() * 2) * flip;
        }

        level->addParticle(eParticleType_netherportal, x, y, z, xa, ya, za);
    }
}

int PortalTile::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}

// 4J Added - We cannot collect the portal tile, so don't consider it as a hit result
// Bug #754 - Riding a minecart into a portal will trap the player
bool PortalTile::mayPick()
{
	return false;
}
