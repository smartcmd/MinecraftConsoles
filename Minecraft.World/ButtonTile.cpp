#include "stdafx.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.h"
#include "ButtonTile.h"
#include "SoundTypes.h"

ButtonTile::ButtonTile(int id, bool sensitive) : Tile(id, Material::decoration,isSolidRender())
{
	this->setTicking(true);
	this->sensitive = sensitive;
}

Icon *ButtonTile::getTexture(int face, int data)
{
	if(id == Tile::button_wood_Id) return Tile::wood->getTexture(Facing::UP);
	else return Tile::stone->getTexture(Facing::UP);
}

AABB *ButtonTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

int ButtonTile::getTickDelay(Level *level)
{
	return sensitive ? 30 : 20;
}

bool ButtonTile::blocksLight()
{
	return false;
}

bool ButtonTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool ButtonTile::isCubeShaped()
{
	return false;
}

bool ButtonTile::mayPlace(Level *level, int x, int y, int z, int face)
{
	if (face == 2 && level->isSolidBlockingTile(x, y, z + 1)) return true;
	if (face == 3 && level->isSolidBlockingTile(x, y, z - 1)) return true;
	if (face == 4 && level->isSolidBlockingTile(x + 1, y, z)) return true;
	if (face == 5 && level->isSolidBlockingTile(x - 1, y, z)) return true;
	return false;
}

bool ButtonTile::mayPlace(Level *level, int x, int y, int z)
{
	if (level->isSolidBlockingTile(x - 1, y, z))
	{
		return true;
	}
	else if (level->isSolidBlockingTile(x + 1, y, z))
	{
		return true;
	}
	else if (level->isSolidBlockingTile(x, y, z - 1))
	{
		return true;
	}
	else if (level->isSolidBlockingTile(x, y, z + 1))
	{
		return true;
	}
	return false;
}

int ButtonTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	int dir = level->getData(x, y, z);

	int oldFlip = dir & 8;
	dir &= 7;

	if (face == 2 && level->isSolidBlockingTile(x, y, z + 1)) dir = 4;
	else if (face == 3 && level->isSolidBlockingTile(x, y, z - 1)) dir = 3;
	else if (face == 4 && level->isSolidBlockingTile(x + 1, y, z)) dir = 2;
	else if (face == 5 && level->isSolidBlockingTile(x - 1, y, z)) dir = 1;
	else dir = findFace(level, x, y, z);

	return dir + oldFlip;
}

int ButtonTile::findFace(Level *level, int x, int y, int z)
{
	if (level->isSolidBlockingTile(x - 1, y, z))
	{
		return 1;
	}
	else if (level->isSolidBlockingTile(x + 1, y, z))
	{
		return 2;
	}
	else if (level->isSolidBlockingTile(x, y, z - 1))
	{
		return 3;
	}
	else if (level->isSolidBlockingTile(x, y, z + 1))
	{
		return 4;
	}
	return 1;
}

void ButtonTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (checkCanSurvive(level, x, y, z))
	{
		int dir = level->getData(x, y, z) & 7;
		bool replace = false;

		if (!level->isSolidBlockingTile(x - 1, y, z) && dir == 1) replace = true;
		if (!level->isSolidBlockingTile(x + 1, y, z) && dir == 2) replace = true;
		if (!level->isSolidBlockingTile(x, y, z - 1) && dir == 3) replace = true;
		if (!level->isSolidBlockingTile(x, y, z + 1) && dir == 4) replace = true;

		if (replace)
		{
			spawnResources(level, x, y, z, level->getData(x, y, z), 0);
			level->removeTile(x, y, z);
		}
	}
}

bool ButtonTile::checkCanSurvive(Level *level, int x, int y, int z)
{
	if (!mayPlace(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->removeTile(x, y, z);
		return false;
	}
	return true;
}

void ButtonTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	int data = level->getData(x, y, z);
	updateShape(data);
}

void ButtonTile::updateShape(int data)
{
	int dir = data & 7;
	bool pressed = (data & 8) > 0;

	float h0 = 6 / 16.0f;
	float h1 = 10 / 16.0f;
	float r = 3 / 16.0f;
	float d = 2 / 16.0f;
	if (pressed) d = 1 / 16.0f;

	if (dir == 1)
	{
		setShape(0, h0, 0.5f - r, d, h1, 0.5f + r);
	}
	else if (dir == 2)
	{
		setShape(1 - d, h0, 0.5f - r, 1, h1, 0.5f + r);
	}
	else if (dir == 3)
	{
		setShape(0.5f - r, h0, 0, 0.5f + r, h1, d);
	}
	else if (dir == 4)
	{
		setShape(0.5f - r, h0, 1 - d, 0.5f + r, h1, 1);
	}
}

void ButtonTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	//use(level, x, y, z, player, 0, 0, 0, 0);
}

// 4J-PB - Adding a TestUse for tooltip display
bool ButtonTile::TestUse()
{
	return true;
}

bool ButtonTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly)
	{
		// 4J - added - just do enough to play the sound
		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.6f);
		return false;
	}

	int data = level->getData(x, y, z);
	int dir = data & 7;
	int open = 8 - (data & 8);
	if (open == 0) return true;

	level->setData(x, y, z, dir + open, Tile::UPDATE_ALL);
	level->setTilesDirty(x, y, z, x, y, z);

	level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.6f);

	updateNeighbours(level, x, y, z, dir);

	level->addToTickNextTick(x, y, z, id, getTickDelay(level));

	return true;
}

void ButtonTile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
	if ((data & 8) > 0)
	{
		int dir = data & 7;
		updateNeighbours(level, x, y, z, dir);
	}
	Tile::onRemove(level, x, y, z, id, data);
}

int ButtonTile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return (level->getData(x, y, z) & 8) > 0 ? Redstone::SIGNAL_MAX : Redstone::SIGNAL_NONE;
}

int ButtonTile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	int data = level->getData(x, y, z);
	if ((data & 8) == 0) return Redstone::SIGNAL_NONE;
	int myDir = data & 7;

	if (myDir == 5 && dir == 1) return Redstone::SIGNAL_MAX;
	if (myDir == 4 && dir == 2) return Redstone::SIGNAL_MAX;
	if (myDir == 3 && dir == 3) return Redstone::SIGNAL_MAX;
	if (myDir == 2 && dir == 4) return Redstone::SIGNAL_MAX;
	if (myDir == 1 && dir == 5) return Redstone::SIGNAL_MAX;

	return false;
}

bool ButtonTile::isSignalSource()
{
	return true;
}

void ButtonTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (level->isClientSide) return;
	int data = level->getData(x, y, z);
	if ((data & 8) == 0)
	{
		return;
	}
	if(sensitive)
	{
		checkPressed(level, x, y, z);
	}
	else
	{
		level->setData(x, y, z, data & 7, Tile::UPDATE_ALL);

		int dir = data & 7;
		updateNeighbours(level, x, y, z, dir);

		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.5f);
		level->setTilesDirty(x, y, z, x, y, z);
	}
}

void ButtonTile::updateDefaultShape()
{
	float x = 3 / 16.0f;
	float y = 2 / 16.0f;
	float z = 2 / 16.0f;
	setShape(0.5f - x, 0.5f - y, 0.5f - z, 0.5f + x, 0.5f + y, 0.5f + z);
}

void ButtonTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (level->isClientSide) return;
	if (!sensitive) return;

	if ((level->getData(x, y, z) & 8) != 0)
	{
		return;
	}

	checkPressed(level, x, y, z);
}

void ButtonTile::checkPressed(Level *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	int dir = data & 7;
	bool wasPressed = (data & 8) != 0;
	bool shouldBePressed;

	updateShape(data);
	Tile::ThreadStorage *tls = (Tile::ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(Arrow), AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1));
	shouldBePressed = !entities->empty();
	delete entities;

	if (shouldBePressed && !wasPressed)
	{
		level->setData(x, y, z, dir | 8, Tile::UPDATE_ALL);
		updateNeighbours(level, x, y, z, dir);
		level->setTilesDirty(x, y, z, x, y, z);

		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.6f);
	}
	if (!shouldBePressed && wasPressed)
	{
		level->setData(x, y, z, dir, Tile::UPDATE_ALL);
		updateNeighbours(level, x, y, z, dir);
		level->setTilesDirty(x, y, z, x, y, z);

		level->playSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_CLICK, 0.3f, 0.5f);
	}

	if (shouldBePressed)
	{
		level->addToTickNextTick(x, y, z, id, getTickDelay(level));
	}
}

void ButtonTile::updateNeighbours(Level *level, int x, int y, int z, int dir)
{
	level->updateNeighborsAt(x, y, z, id);

	if (dir == 1)
	{
		level->updateNeighborsAt(x - 1, y, z, id);
	}
	else if (dir == 2)
	{
		level->updateNeighborsAt(x + 1, y, z, id);
	}
	else if (dir == 3)
	{
		level->updateNeighborsAt(x, y, z - 1, id);
	}
	else if (dir == 4)
	{
		level->updateNeighborsAt(x, y, z + 1, id);
	}
	else
	{
		level->updateNeighborsAt(x, y - 1, z, id);
	}
}

bool ButtonTile::shouldTileTick(Level *level, int x,int y,int z)
{
	int currentData = level->getData(x, y, z);
	return (currentData & 8) != 0;
}

void ButtonTile::registerIcons(IconRegister *iconRegister)
{
	// None
}
