#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "FarmTile.h"


FarmTile::FarmTile(int id) : Tile(id, Material::dirt,isSolidRender())
{
    iconWet = NULL;
	iconDry = NULL;

    setTicking(true);
    updateDefaultShape();
    setLightBlock(255);
}

// 4J Added override
void FarmTile::updateDefaultShape()
{
    setShape(0, 0, 0, 1, 15 / 16.0f, 1);
}

AABB *FarmTile::getAABB(Level *level, int x, int y, int z)
{
	return AABB::newTemp(x + 0, y + 0, z + 0, x + 1, y + 1, z + 1);
}

bool FarmTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool FarmTile::isCubeShaped()
{
	return false;
}

Icon *FarmTile::getTexture(int face, int data)
{
    if (face == Facing::UP)
	{
		if(data > 0)
		{
			return iconWet;
		}
		else
		{
			return iconDry;
		}
	}
    return Tile::dirt->getTexture(face);
}

void FarmTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (isNearWater(level, x, y, z) || level->isRainingAt(x, y + 1, z))
	{
        level->setData(x, y, z, 7);
    } else
	{
        int moisture = level->getData(x, y, z);
        if (moisture > 0)
		{
            level->setData(x, y, z, moisture - 1);
        } else
		{
            if (!isUnderCrops(level, x, y, z))
			{
                level->setTile(x, y, z, Tile::dirt_Id);
            }
        }
    }
}

void FarmTile::fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance)
{
	// 4J Stu - Fix for #86148 - Code: Gameplay: Jumping on Farmland does not always result in turning to Dirt Block
	// We should not be setting tiles on the client based on random values!
	if (!level->isClientSide && level->random->nextFloat() < (fallDistance - .5f))
	{
		// Fix for #60547 - TU7: Content: Gameplay: Players joining a game can destroy crops even with Trust Players option disabled.
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(entity);
		if(player == NULL || player->isAllowedToMine()) level->setTile(x, y, z, Tile::dirt_Id);
    }
}

bool FarmTile::isUnderCrops(Level *level, int x, int y, int z)
{
    int r = 0;
    for (int xx = x - r; xx <= x + r; xx++)
        for (int zz = z - r; zz <= z + r; zz++)
		{
            int tile = level->getTile(xx, y + 1, zz);
            if (tile == Tile::crops_Id || tile == Tile::melonStem_Id || tile == Tile::pumpkinStem_Id || tile == Tile::potatoes_Id || tile == Tile::carrots_Id)
			{
                return true;
            }
        }
    return false;
}

bool FarmTile::isNearWater(Level *level, int x, int y, int z)
{
    for (int xx = x - 4; xx <= x + 4; xx++)
        for (int yy = y; yy <= y + 1; yy++)
            for (int zz = z - 4; zz <= z + 4; zz++)
			{
                if (level->getMaterial(xx, yy, zz) == Material::water)
				{
                    return true;
                }
            }
    return false;
}

void FarmTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    Tile::neighborChanged(level, x, y, z, type);
    Material *above = level->getMaterial(x, y + 1, z);
    if (above->isSolid())
	{
        level->setTile(x, y, z, Tile::dirt_Id);
    }
}

bool FarmTile::blocksLight()
{
	return true;
}

int FarmTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::dirt->getResource(0, random, playerBonusLevel);
}

int FarmTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Tile::dirt_Id;
}

void FarmTile::registerIcons(IconRegister *iconRegister)
{
	iconWet = iconRegister->registerIcon(L"farmland_wet");
	iconDry = iconRegister->registerIcon(L"farmland_dry");
}
