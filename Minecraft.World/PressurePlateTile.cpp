#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.h"
#include "PressurePlateTile.h"
#include "SoundTypes.h"

PressurePlateTile::PressurePlateTile(int id, const wstring &tex, Material *material, Sensitivity sensitivity) : Tile(id, material, isSolidRender())
{
	this->sensitivity = sensitivity;
	this->setTicking(true);
	this->texture = tex;

	float o = 1 / 16.0f;
	setShape(o, 0, o, 1 - o, 0.5f / 16.0f, 1 - o);
}

int PressurePlateTile::getTickDelay()
{
	return 20;
}

AABB *PressurePlateTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool PressurePlateTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool PressurePlateTile::blocksLight()
{
	return false;
}

bool PressurePlateTile::isCubeShaped()
{
	return false;
}

bool PressurePlateTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return true;
}

bool PressurePlateTile::mayPlace(Level *level, int x, int y, int z)
{
	return level->isTopSolidBlocking(x, y - 1, z) || FenceTile::isFence(level->getTile(x, y - 1, z));
}

void PressurePlateTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
    bool replace = false;

   if (!level->isTopSolidBlocking(x, y - 1, z) && !FenceTile::isFence(level->getTile(x, y - 1, z))) replace = true;

    if (replace)
	{
        this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
        level->setTile(x, y, z, 0);
    }
}

void PressurePlateTile::tick(Level *level, int x, int y, int z, Random *random)
{
    if (level->isClientSide) return;
    if (level->getData(x, y, z) == 0)
	{
        return;
    }

    checkPressed(level, x, y, z);
}

void PressurePlateTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
    if (level->isClientSide) return;

    if (level->getData(x, y, z) == 1)
	{
        return;
    }

    checkPressed(level, x, y, z);
}

void PressurePlateTile::checkPressed(Level *level, int x, int y, int z)
{
    bool wasPressed = level->getData(x, y, z) == 1;
    bool shouldBePressed = false;

    float b = 2 / 16.0f;
    vector<shared_ptr<Entity> > *entities = NULL;

	bool entitiesToBeFreed = false;
    if (sensitivity == PressurePlateTile::everything) entities = level->getEntities(nullptr, AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 0.25, z + 1 - b));

    if (sensitivity == PressurePlateTile::mobs)
	{
		entities = level->getEntitiesOfClass(typeid(Mob), AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 0.25, z + 1 - b));
		entitiesToBeFreed = true;
	}
    if (sensitivity == PressurePlateTile::players)
	{
		entities = level->getEntitiesOfClass(typeid(Player), AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 0.25, z + 1 - b));
		entitiesToBeFreed = true;
	}

    if (!entities->empty())
	{
        shouldBePressed = true;
    }

    if (shouldBePressed && !wasPressed)
	{
        level->setData(x, y, z, 1);
        level->updateNeighborsAt(x, y, z, id);
        level->updateNeighborsAt(x, y - 1, z, id);
        level->setTilesDirty(x, y, z, x, y, z);

        level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.6f);
    }
    if (!shouldBePressed && wasPressed)
	{
        level->setData(x, y, z, 0);
        level->updateNeighborsAt(x, y, z, id);
        level->updateNeighborsAt(x, y - 1, z, id);
        level->setTilesDirty(x, y, z, x, y, z);

        level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.5f);
    }

    if (shouldBePressed)
	{
        level->addToTickNextTick(x, y, z, id, getTickDelay());
    }

	if( entitiesToBeFreed )
	{
		delete entities;
	}
}

void PressurePlateTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
    if (data > 0)
	{
        level->updateNeighborsAt(x, y, z, this->id);
        level->updateNeighborsAt(x, y - 1, z, this->id);
    }
    Tile::onRemove(level, x, y, z, id, data);
}

void PressurePlateTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
    bool pressed = level->getData(x, y, z) == 1;

    float o = 1 / 16.0f;
    if (pressed)
	{
        this->setShape(o, 0, o, 1 - o, 0.5f / 16.0f, 1 - o);
    }
	else
	{
        setShape(o, 0, o, 1 - o, 1 / 16.0f, 1 - o);
    }
}

bool PressurePlateTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z)) > 0;
}

bool PressurePlateTile::getDirectSignal(Level *level, int x, int y, int z, int dir)
{
    if (level->getData(x, y, z) == 0) return false;
    return (dir == 1);
}

bool PressurePlateTile::isSignalSource()
{
	return true;
}

void PressurePlateTile::updateDefaultShape()
{
    float x = 8 / 16.0f;
    float y = 2 / 16.0f;
    float z = 8 / 16.0f;
    setShape(0.5f - x, 0.5f - y, 0.5f - z, 0.5f + x, 0.5f + y, 0.5f + z);

}

bool PressurePlateTile::shouldTileTick(Level *level, int x,int y,int z)
{
    return level->getData(x, y, z) != 0;
}

int PressurePlateTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

void PressurePlateTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(texture);
}