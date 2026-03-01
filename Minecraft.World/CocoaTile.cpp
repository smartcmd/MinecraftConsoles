#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "CocoaTile.h"

const wstring CocoaTile::TEXTURE_AGES[] = {	L"cocoa_0", L"cocoa_1", L"cocoa_2"};

CocoaTile::CocoaTile(int id) : DirectionalTile(id, Material::plant, isSolidRender() )
{
	setTicking(true);
}

Icon *CocoaTile::getTexture(int face, int data)
{
	return icons[2];
}

Icon *CocoaTile::getTextureForAge(int age)
{
	if (age < 0 || age >= COCOA_TEXTURES_LENGTH)
	{
		age = COCOA_TEXTURES_LENGTH - 1;
	}
	return icons[age];
}

void CocoaTile::tick(Level *level, int x, int y, int z, Random *random)
{
	if (!canSurvive(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTileAndData(x, y, z, 0, 0, UPDATE_CLIENTS);
	}
	else if (level->random->nextInt(5) == 0)
	{
		int data = level->getData(x, y, z);
		int age = getAge(data);
		if (age < 2)
		{
			age++;
			level->setData(x, y, z, (age << 2) | (getDirection(data)), Tile::UPDATE_CLIENTS);
		}
	}
}

bool CocoaTile::canSurvive(Level *level, int x, int y, int z) 
{
	int dir = getDirection(level->getData(x, y, z));

	x += Direction::STEP_X[dir];
	z += Direction::STEP_Z[dir];
	int attachedTo = level->getTile(x, y, z);

	return attachedTo == Tile::treeTrunk_Id && TreeTile::getWoodType(level->getData(x, y, z)) == TreeTile::JUNGLE_TRUNK;
}

int CocoaTile::getRenderShape()
{
	return SHAPE_COCOA;
}

bool CocoaTile::isCubeShaped()
{
	return false;
}

bool CocoaTile::isSolidRender(bool isServerLevel)
{
	return false;
}

AABB *CocoaTile::getAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return DirectionalTile::getAABB(level, x, y, z);
}

AABB *CocoaTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return DirectionalTile::getTileAABB(level, x, y, z);
}

void CocoaTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity)
{
	int data = level->getData(x, y, z);
	int dir = getDirection(data);
	int age = getAge(data);

	int width = 4 + age * 2;
	int height = 5 + age * 2;

	float hWidth = width / 2.0f;

	switch (dir)
	{
	case Direction::SOUTH:
		setShape((8.0f - hWidth) / 16.0f, (12.0f - height) / 16.0f, (15.0f - width) / 16.0f, (8.0f + hWidth) / 16.0f, (12.0f) / 16.0f, (15.0f) / 16.0f);
		break;
	case Direction::NORTH:
		setShape((8.0f - hWidth) / 16.0f, (12.0f - height) / 16.0f, (1.0f) / 16.0f, (8.0f + hWidth) / 16.0f, (12.0f) / 16.0f, (1.0f + width) / 16.0f);
		break;
	case Direction::WEST:
		setShape((1.0f) / 16.0f, (12.0f - height) / 16.0f, (8.0f - hWidth) / 16.0f, (1.0f + width) / 16.0f, (12.0f) / 16.0f, (8.0f + hWidth) / 16.0f);
		break;
	case Direction::EAST:
		setShape((15.0f - width) / 16.0f, (12.0f - height) / 16.0f, (8.0f - hWidth) / 16.0f, (15.0f) / 16.0f, (12.0f) / 16.0f, (8.0f + hWidth) / 16.0f);
		break;
	}
}

void CocoaTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int dir = (((Mth::floor(by->yRot * 4 / (360) + 0.5)) & 3) + 0) % 4;
	level->setData(x, y, z, dir, Tile::UPDATE_CLIENTS);
}

int CocoaTile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	if (face == Facing::UP || face == Facing::DOWN)
	{
		face = Facing::NORTH;
	}
	return Direction::DIRECTION_OPPOSITE[Direction::FACING_DIRECTION[face]];
}

void CocoaTile::neighborChanged(Level *level, int x, int y, int z, int type) 
{
	if (!canSurvive(level, x, y, z))
	{
		this->spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTileAndData(x, y, z, 0, 0, UPDATE_CLIENTS);
	}
}

int CocoaTile::getAge(int data)
{
	return (data & DirectionalTile::DIRECTION_INV_MASK) >> 2;
}

void CocoaTile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	int age = getAge(data);
	int count = 1;
	if (age >= 2)
	{
		count = 3;
	}
	for (int i = 0; i < count; i++)
	{
		popResource(level, x, y, z, shared_ptr<ItemInstance>( new ItemInstance(Item::dye_powder, 1, DyePowderItem::BROWN) ));
	}
}

int CocoaTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::dye_powder_Id;
}

int CocoaTile::cloneTileData(Level *level, int x, int y, int z)
{
	return DyePowderItem::BROWN;
}

void CocoaTile::registerIcons(IconRegister *iconRegister)
{
	icons = new Icon*[COCOA_TEXTURES_LENGTH];

	for (int i = 0; i < COCOA_TEXTURES_LENGTH; i++)
	{
		icons[i] = iconRegister->registerIcon(TEXTURE_AGES[i]);
	}
}