#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "BasePressurePlateTile.h"

BasePressurePlateTile::BasePressurePlateTile(int id, const wstring &tex, Material *material) : Tile(id, material, isSolidRender())
{
	texture = tex;
	setTicking(true);

	// 4J Stu - Move this to derived classes
	//updateShape(getDataForSignal(Redstone::SIGNAL_MAX));
}

void BasePressurePlateTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	updateShape(level->getData(x, y, z));
}

void BasePressurePlateTile::updateShape(int data)
{
	bool pressed = getSignalForData(data) > Redstone::SIGNAL_NONE;
	float o = 1 / 16.0f;

	if (pressed)
	{
		setShape(o, 0, o, 1 - o, 0.5f / 16.0f, 1 - o);
	}
	else
	{
		setShape(o, 0, o, 1 - o, 1 / 16.0f, 1 - o);
	}
}

int BasePressurePlateTile::getTickDelay(Level *level)
{
	return SharedConstants::TICKS_PER_SECOND;
}

AABB *BasePressurePlateTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

bool BasePressurePlateTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool BasePressurePlateTile::blocksLight()
{
	return false;
}

bool BasePressurePlateTile::isCubeShaped()
{
	return false;
}

bool BasePressurePlateTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return true;
}

bool BasePressurePlateTile::mayPlace(Level *level, int x, int y, int z)
{
	return level->isTopSolidBlocking(x, y - 1, z) || FenceTile::isFence(level->getTile(x, y - 1, z));
}

void BasePressurePlateTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	bool replace = false;

	if (!level->isTopSolidBlocking(x, y - 1, z) && !FenceTile::isFence(level->getTile(x, y - 1, z))) replace = true;

	if (replace)
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
	}
}

void BasePressurePlateTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;
	int signal = getSignalForData(level->getData(x, y, z));
	if (signal > Redstone::SIGNAL_NONE) checkPressed(level, x, y, z, signal);
}

void BasePressurePlateTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (level->isClientSide) return;
	int signal = getSignalForData(level->getData(x, y, z));
	if (signal == Redstone::SIGNAL_NONE) checkPressed(level, x, y, z, signal);
}

void BasePressurePlateTile::checkPressed(Level *level, int x, int y, int z, int oldSignal)
{
	int signal = getSignalStrength(level, x, y, z);
	bool wasPressed = oldSignal > Redstone::SIGNAL_NONE;
	bool shouldBePressed = signal > Redstone::SIGNAL_NONE;

	if (oldSignal != signal)
	{
		level->setData(x, y, z, getDataForSignal(signal), Tile::UPDATE_CLIENTS);
		updateNeighbours(level, x, y, z);
		level->setTilesDirty(x, y, z, x, y, z);
	}

	if (!shouldBePressed && wasPressed)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.5f);
	}
	else if (shouldBePressed && !wasPressed)
	{
		level->playSound(x + 0.5, y + 0.1, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.6f);
	}

	if (shouldBePressed)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}
}

AABB *BasePressurePlateTile::getSensitiveAABB(int x, int y, int z)
{
	float b = 2 / 16.0f;
	return AABB::newTemp(x + b, y, z + b, x + 1 - b, y + 0.25, z + 1 - b);
}

void BasePressurePlateTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if (getSignalForData(data) > 0)
	{
		updateNeighbours(level, x, y, z);
	}

	Tile::onRemove(level, x, y, z, id, data);
}

void BasePressurePlateTile::updateNeighbours(Level *level, int x, int y, int z)
{
	level->updateNeighborsAt(x, y, z, id);
	level->updateNeighborsAt(x, y - 1, z, id);
}

int BasePressurePlateTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return getSignalForData(level->getData(x, y, z));
}

int BasePressurePlateTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	if (dir == Facing::UP)
	{
		return getSignalForData(level->getData(x, y, z));
	}
	else
	{
		return Redstone::SIGNAL_NONE;
	}
}

bool BasePressurePlateTile::isSignalSource()
{
	return true;
}

void BasePressurePlateTile::updateDefaultShape()
{
	float x = 8 / 16.0f;
	float y = 2 / 16.0f;
	float z = 8 / 16.0f;
	setShape(0.5f - x, 0.5f - y, 0.5f - z, 0.5f + x, 0.5f + y, 0.5f + z);
}

int BasePressurePlateTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

void BasePressurePlateTile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(texture);
}