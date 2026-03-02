#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "DoorTile.h"
#include "LevelEvent.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"

const wstring DoorTile::TEXTURES[] = { L"doorWood_lower", L"doorWood_upper", L"doorIron_lower", L"doorIron_upper" };

DoorTile::DoorTile(int id, Material *material) : Tile(id, material,isSolidRender())
{
	if (material == Material::metal)
	{
		texBase = 2;
	}
	else
	{
		texBase = 0;
	}

	float r = 0.5f;
	float h = 1.0f;
	Tile::setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
}

Icon *DoorTile::getTexture(int face, int data)
{
	return iconBottom[TEXTURE_NORMAL];
}

Icon *DoorTile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	if (face == Facing::UP || face == Facing::DOWN) return iconBottom[TEXTURE_NORMAL];

	int compositeData = getCompositeData(level, x, y, z);
	int dir = compositeData & C_DIR_MASK;
	bool isOpen = (compositeData & C_OPEN_MASK) != 0;
	bool flip = false;
	bool upper = (compositeData & C_IS_UPPER_MASK) != 0;

	if (isOpen)
	{
		if (dir == 0 && face == 2) flip = !flip;
		else if (dir == 1 && face == 5) flip = !flip;
		else if (dir == 2 && face == 3) flip = !flip;
		else if (dir == 3 && face == 4) flip = !flip;
	}
	else
	{
		if (dir == 0 && face == 5) flip = !flip;
		else if (dir == 1 && face == 3) flip = !flip;
		else if (dir == 2 && face == 4) flip = !flip;
		else if (dir == 3 && face == 2) flip = !flip;
		if ((compositeData & C_RIGHT_HINGE_MASK) != 0) flip = !flip;
	}

	if (upper)
	{
		return iconTop[flip ? TEXTURE_FLIPPED : TEXTURE_NORMAL];
	}
	else
	{
		return iconBottom[flip ? TEXTURE_FLIPPED : TEXTURE_NORMAL];
	}
}

void DoorTile::registerIcons(IconRegister *iconRegister)
{
	iconTop[TEXTURE_NORMAL] = iconRegister->registerIcon(getIconName() + L"_upper");
	iconBottom[TEXTURE_NORMAL] = iconRegister->registerIcon(getIconName() + L"_lower");
	iconTop[TEXTURE_FLIPPED] = new FlippedIcon(iconTop[TEXTURE_NORMAL], true, false);
	iconBottom[TEXTURE_FLIPPED] = new FlippedIcon(iconBottom[TEXTURE_NORMAL], true, false);
}

bool DoorTile::blocksLight()
{
	return false;
}

bool DoorTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool DoorTile::isCubeShaped()
{
	return false;
}

int DoorTile::getRenderShape()
{
	return Tile::SHAPE_DOOR;
}

AABB *DoorTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	AABB *retval = Tile::getTileAABB(level, x, y, z);
	return retval;
}

AABB *DoorTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	AABB *retval = Tile::getAABB(level, x, y, z);
	return retval;
}

void DoorTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	setShape(getCompositeData(level,x, y, z));
}

int DoorTile::getDir(LevelSource *level, int x, int y, int z)
{
	return getCompositeData(level, x, y, z) & C_DIR_MASK;
}

bool DoorTile::isOpen(LevelSource *level, int x, int y, int z)
{
	return (getCompositeData(level, x, y, z) & C_OPEN_MASK) != 0;
}

void DoorTile::setShape(int compositeData)
{
	float r = 3 / 16.0f;
	Tile::setShape(0, 0, 0, 1, 2, 1);
	int dir = compositeData & C_DIR_MASK;
	bool open = (compositeData & C_OPEN_MASK) != 0;
	bool hasRightHinge = (compositeData & C_RIGHT_HINGE_MASK) != 0;
	if (dir == 0)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 0, 1, 1, r);
			else setShape(0, 0, 1 - r, 1, 1, 1);
		}
		else setShape(0, 0, 0, r, 1, 1);
	}
	else if (dir == 1)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(1 - r, 0, 0, 1, 1, 1);
			else setShape(0, 0, 0, r, 1, 1);
		}
		else setShape(0, 0, 0, 1, 1, r);
	}
	else if (dir == 2)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 1 - r, 1, 1, 1);
			else setShape(0, 0, 0, 1, 1, r);
		}
		else setShape(1 - r, 0, 0, 1, 1, 1);
	}
	else if (dir == 3)
	{
		if (open)
		{
			if (!hasRightHinge) setShape(0, 0, 0, r, 1, 1);
			else setShape(1 - r, 0, 0, 1, 1, 1);
		}
		else setShape(0, 0, 1 - r, 1, 1, 1);
	}
}

void DoorTile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	// Fix for #92957 - TU11: Content: Multiplayer: Wooden Doors splits in half and glitch in open / close motion while being mined.
	// In lastest PC version this is commented out, so do that now to fix bug above
	//use(level, x, y, z, player);
}

// 4J-PB - Adding a TestUse for tooltip display
bool DoorTile::TestUse()
{
	return id == Tile::door_wood_Id;
}

bool DoorTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly)
	{
		// 4J - added - just do enough to play the sound
		if (material != Material::metal)
		{
			level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
		}
		return false;
	}

	if (material == Material::metal) return true;

	int compositeData = getCompositeData(level, x, y, z);
	int lowerData = compositeData & C_LOWER_DATA_MASK;
	lowerData ^= 4;
	if ((compositeData & C_IS_UPPER_MASK) == 0)
	{
		level->setData(x, y, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y, z, x, y, z);
	}
	else
	{
		level->setData(x, y - 1, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y - 1, z, x, y, z);
	}

	level->levelEvent(player, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
	return true;
}

void DoorTile::setOpen(Level *level, int x, int y, int z, bool shouldOpen)
{
	int compositeData = getCompositeData(level, x, y, z);
	bool isOpen = (compositeData & C_OPEN_MASK) != 0;
	if (isOpen == shouldOpen) return;

	int lowerData = compositeData & C_LOWER_DATA_MASK;
	lowerData ^= 4;
	if ((compositeData & C_IS_UPPER_MASK) == 0)
	{
		level->setData(x, y, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y, z, x, y, z);
	}
	else
	{
		level->setData(x, y - 1, z, lowerData, Tile::UPDATE_CLIENTS);
		level->setTilesDirty(x, y - 1, z, x, y, z);
	}

	level->levelEvent(nullptr, LevelEvent::SOUND_OPEN_DOOR, x, y, z, 0);
}

void DoorTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	int data = level->getData(x, y, z);
	if ((data & UPPER_BIT) == 0)
	{
		bool spawn = false;
		if (level->getTile(x, y + 1, z) != id)
		{
			level->removeTile(x, y, z);
			spawn = true;
		}
		if (!level->isSolidBlockingTile(x, y - 1, z))
		{
			level->removeTile(x, y, z);
			spawn = true;
			if (level->getTile(x, y + 1, z) == id)
			{
				level->removeTile(x, y + 1, z);
			}
		}
		if (spawn)
		{
			if (!level->isClientSide)
			{
				spawnResources(level, x, y, z, data, 0);
			}
		}
		else
		{
			bool signal = level->hasNeighborSignal(x, y, z) || level->hasNeighborSignal(x, y + 1, z);
			if ((signal || (type > 0 && Tile::tiles[type]->isSignalSource())) && type != id)
			{
				setOpen(level, x, y, z, signal);
			}
		}
	}
	else
	{
		if (level->getTile(x, y - 1, z) != id)
		{
			level->removeTile(x, y, z);
		}
		if (type > 0 && type != id)
		{
			neighborChanged(level, x, y - 1, z, type);
		}
	}
}

int DoorTile::getResource(int data, Random *random, int playerBonusLevel)
{
	if ((data & 8) != 0) return 0;
	if (material == Material::metal) return Item::door_iron->id;
	return Item::door_wood->id;
}

HitResult *DoorTile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);
	return Tile::clip(level, xt, yt, zt, a, b);
}

bool DoorTile::mayPlace(Level *level, int x, int y, int z)
{
	if (y >= Level::maxBuildHeight - 1) return false;

	return (level->isTopSolidBlocking(x, y - 1, z) && Tile::mayPlace(level, x, y, z) && Tile::mayPlace(level, x, y + 1, z));
}

bool DoorTile::isOpen(int data)
{
	return (data & 4) != 0;
}

int DoorTile::getPistonPushReaction()
{
	return Material::PUSH_DESTROY;
}

int DoorTile::getCompositeData(LevelSource *level, int x, int y, int z)
{
	int data = level->getData(x, y, z);
	bool isUpper = (data & UPPER_BIT) != 0;
	int lowerData;
	int upperData;
	if (isUpper)
	{
		lowerData = level->getData(x, y - 1, z);
		upperData = data;
	}
	else
	{
		lowerData = data;
		upperData = level->getData(x, y + 1, z);
	}

	// bits: dir, dir, open/closed, isUpper, isRightHinge
	bool isRightHinge = (upperData & 1) != 0;
	return (lowerData & C_LOWER_DATA_MASK) | (isUpper ? 8 : 0) | (isRightHinge ? 16 : 0);
}

int DoorTile::cloneTileId(Level *level, int x, int y, int z)
{
	return material == Material::metal ? Item::door_iron_Id : Item::door_wood_Id;
}

void DoorTile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
	if (player->abilities.instabuild)
	{
		if ((data & UPPER_BIT) != 0)
		{
			if (level->getTile(x, y - 1, z) == id)
			{
				level->removeTile(x, y - 1, z);
			}
		}
	}
}