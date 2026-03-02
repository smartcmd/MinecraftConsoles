#include "stdafx.h"
#include "Dimension.h"
#include "net.minecraft.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.h"
#include "BedTile.h"

int BedTile::HEAD_DIRECTION_OFFSETS[4][2] =
{
    { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 }
};

BedTile::BedTile(int id) : DirectionalTile(id, Material::cloth, isSolidRender())
{
	setShape();

	iconEnd = NULL;
	iconSide = NULL;
	iconTop = NULL;
}

// 4J Added override
void BedTile::updateDefaultShape()
{
    setShape();
}

// 4J-PB - Adding a TestUse for tooltip display
bool BedTile::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	//if (level->isClientSide) return true;

	int data = level->getData(x, y, z);

	if (!BedTile::isHeadPiece(data))
	{
		// fetch head piece instead
		int direction = getDirection(data);
		x += HEAD_DIRECTION_OFFSETS[direction][0];
		z += HEAD_DIRECTION_OFFSETS[direction][1];
		if (level->getTile(x, y, z) != id)
		{
			return true;
		}
		data = level->getData(x, y, z);
	}

	if (!level->dimension->mayRespawn())
	{
		return false;
	}
	if (BedTile::isOccupied(data))
	{
		return false;
	}

	Player::BedSleepingResult result = player->startSleepInBed(x, y, z, true); // true to just test the start sleep
	if (result == Player::OK)
	{
		return true;
	}

	return false;
}

bool BedTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if( soundOnly) return false;
    if (level->isClientSide) return true;

    int data = level->getData(x, y, z);

    if (!BedTile::isHeadPiece(data))
	{
        // fetch head piece instead
        int direction = getDirection(data);
        x += HEAD_DIRECTION_OFFSETS[direction][0];
        z += HEAD_DIRECTION_OFFSETS[direction][1];
        if (level->getTile(x, y, z) != id)
		{
            return true;
        }
        data = level->getData(x, y, z);
    }

    if (!level->dimension->mayRespawn())
	{
        double xc = x + 0.5;
        double yc = y + 0.5;
        double zc = z + 0.5;
        level->setTile(x, y, z, 0);
        int direction = getDirection(data);
        x += HEAD_DIRECTION_OFFSETS[direction][0];
        z += HEAD_DIRECTION_OFFSETS[direction][1];
        if (level->getTile(x, y, z) == id) {
            level->setTile(x, y, z, 0);
            xc = (xc + x + 0.5) / 2;
            yc = (yc + y + 0.5) / 2;
            zc = (zc + z + 0.5) / 2;
        }
        level->explode(nullptr, x + 0.5f, y + 0.5f, z + 0.5f, 5, true, true);
        return true;
    }

    if (BedTile::isOccupied(data))
	{
        shared_ptr<Player> sleepingPlayer = nullptr;
		AUTO_VAR(itEnd, level->players.end());
        for (AUTO_VAR(it, level->players.begin()); it != itEnd; it++ )
		{
			shared_ptr<Player> p = *it;
            if (p->isSleeping())
			{
                Pos pos = p->bedPosition;
                if (pos.x == x && pos.y == y && pos.z == z)
				{
                    sleepingPlayer = p;
                }
            }
        }

        if (sleepingPlayer == NULL)
		{
            BedTile::setOccupied(level, x, y, z, false);
        }
		else
		{
			player->displayClientMessage(IDS_TILE_BED_OCCUPIED );
			
            return true;
        }
    }

    Player::BedSleepingResult result = player->startSleepInBed(x, y, z);
    if (result == Player::OK)
	{
        BedTile::setOccupied(level, x, y, z, true);
		// 4J-PB added
		// are there multiple players in the same world as us?
		if(level->AllPlayersAreSleeping()==false)
		{
			player->displayClientMessage(IDS_TILE_BED_PLAYERSLEEP);
		}       
		return true;
    }

    if (result == Player::NOT_POSSIBLE_NOW)
	{
		player->displayClientMessage(IDS_TILE_BED_NO_SLEEP);
    }
	else if (result == Player::NOT_SAFE)
	{
            player->displayClientMessage(IDS_TILE_BED_NOTSAFE);
    }

    return true;
}

Icon *BedTile::getTexture(int face, int data)
{
	if (face == Facing::DOWN)
	{
		return Tile::wood->getTexture(face);
	}

	int direction = getDirection(data);
	int tileFacing = Direction::RELATIVE_DIRECTION_FACING[direction][face];

	int part = isHeadPiece(data) ? PART_HEAD : PART_FOOT;

	if ((part == PART_HEAD && tileFacing == Facing::NORTH) || (part == PART_FOOT && tileFacing == Facing::SOUTH))
	{
		return iconEnd[part];
	}
	if (tileFacing == Facing::EAST || tileFacing == Facing::WEST)
	{
		return iconSide[part];
	}
	return iconTop[part];
}

void BedTile::registerIcons(IconRegister *iconRegister)
{
	iconTop = new Icon *[2];
	iconTop[0] = iconRegister->registerIcon(L"bed_feet_top");
	iconTop[1] = iconRegister->registerIcon(L"bed_head_top");

	iconEnd = new Icon *[2];
	iconEnd[0] = iconRegister->registerIcon(L"bed_feet_end");
	iconEnd[1] = iconRegister->registerIcon(L"bed_head_end");

	iconSide = new Icon *[2];
	iconSide[0] = iconRegister->registerIcon(L"bed_feet_side");
	iconSide[1] = iconRegister->registerIcon(L"bed_head_side");
}

int BedTile::getRenderShape()
{
	return Tile::SHAPE_BED;
}

bool BedTile::isCubeShaped()
{
	return false;
}

bool BedTile::isSolidRender(bool isServerLevel)
{
	return false;
}

void BedTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape();
}

void BedTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    int data = level->getData(x, y, z);
    int direction = getDirection(data);

    if (isHeadPiece(data))
	{
        if (level->getTile(x - HEAD_DIRECTION_OFFSETS[direction][0], y, z - HEAD_DIRECTION_OFFSETS[direction][1]) != id)
		{
            level->setTile(x, y, z, 0);
        }
    } else
	{
        if (level->getTile(x + HEAD_DIRECTION_OFFSETS[direction][0], y, z + HEAD_DIRECTION_OFFSETS[direction][1]) != id)
		{
            level->setTile(x, y, z, 0);
            if (!level->isClientSide)
			{
                Tile::spawnResources(level, x, y, z, data, 0);		// 4J - had to add Tile:: here for C++ since this class doesn't have this overloaded method itself
            }
        }
    }
}

int BedTile::getResource(int data, Random *random, int playerBonusLevel)
{
    if (isHeadPiece(data))
	{
        return 0;
    }
    return Item::bed->id;
}

void BedTile::setShape()
{
	Tile::setShape(0, 0, 0, 1, 9 / 16.0f, 1);
}

bool BedTile::isHeadPiece(int data)
{
	return (data & HEAD_PIECE_DATA) != 0;
}

bool BedTile::isOccupied(int data)
{
	return (data & OCCUPIED_DATA) != 0;
}

void BedTile::setOccupied(Level *level, int x, int y, int z, bool occupied)
{
    int data = level->getData(x, y, z);
    if (occupied)
	{
        data = data | OCCUPIED_DATA;
    } else
	{
        data = data & ~OCCUPIED_DATA;
    }
    level->setData(x, y, z, data);
}

Pos *BedTile::findStandUpPosition(Level *level, int x, int y, int z, int skipCount)
{
    int data = level->getData(x, y, z);
    int direction = DirectionalTile::getDirection(data);

    // try to find a clear location near the bed
    for (int step = 0; step <= 1; step++)
	{
        int startX = x - BedTile::HEAD_DIRECTION_OFFSETS[direction][0] * step - 1;
        int startZ = z - BedTile::HEAD_DIRECTION_OFFSETS[direction][1] * step - 1;
        int endX = startX + 2;
        int endZ = startZ + 2;

        for (int standX = startX; standX <= endX; standX++)
		{
            for (int standZ = startZ; standZ <= endZ; standZ++)
			{
				// 4J Stu - Changed to check isSolidBlockingTile rather than isEmpty for the blocks that we wish to place the player
				// This allows the player to spawn in blocks with snow, grass etc
                if (level->isTopSolidBlocking(standX, y - 1, standZ) &&
					!level->isSolidBlockingTile(standX, y, standZ) &&
					!level->isSolidBlockingTile(standX, y + 1, standZ))
				{
                    if (skipCount > 0) {
                        skipCount--;
                        continue;
                    }
                    return new Pos(standX, y, standZ);
                }
            }
        }
    }

    return NULL;
}

void BedTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonus)
{
    if (!isHeadPiece(data))
	{
    	Tile::spawnResources(level, x, y, z, data, odds, 0);
    }
}

int BedTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

int BedTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::bed_Id;
}