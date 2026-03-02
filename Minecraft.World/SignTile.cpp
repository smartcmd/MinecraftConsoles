#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "Material.h"
#include "SignTileEntity.h"
#include "SignTile.h"

SignTile::SignTile(int id, eINSTANCEOF clas, bool onGround) : EntityTile(id, Material::wood, isSolidRender())
{
	this->onGround = onGround;
	this->clas = clas;
	updateDefaultShape();
}

Icon *SignTile::getTexture(int face, int data)
{
	return Tile::wood->getTexture(face);
}

void SignTile::updateDefaultShape()
{
	float r = 4 / 16.0f;
	float h = 16 / 16.0f;
	this->setShape(0.5f - r, 0, 0.5f - r, 0.5f + r, h, 0.5f + r);
}

AABB *SignTile::getAABB(Level *level, int x, int y, int z)
{
	return NULL;
}

AABB *SignTile::getTileAABB(Level *level, int x, int y, int z)
{
	updateShape(level, x, y, z);
	return EntityTile::getTileAABB(level, x, y, z);
}

void SignTile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	if (onGround) return;

	int face = level->getData(x, y, z);

	float h0 = (4 + 0.5f) / 16.0f;
	float h1 = (12 + 0.5f) / 16.0f;
	float w0 = 0 / 16.0f;
	float w1 = 16 / 16.0f;

	float d0 = 2 / 16.0f;

	setShape(0, 0, 0, 1, 1, 1);
	if (face == 2) setShape(w0, h0, 1 - d0, w1, h1, 1);
	if (face == 3) setShape(w0, h0, 0, w1, h1, d0);
	if (face == 4) setShape(1 - d0, h0, w0, 1, h1, w1);
	if (face == 5) setShape(0, h0, w0, d0, h1, w1);
}

int SignTile::getRenderShape()
{
	return Tile::SHAPE_INVISIBLE;
}

bool SignTile::isCubeShaped()
{
	return false;
}

bool SignTile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return true;
}

bool SignTile::isSolidRender(bool isServerLevel)
{
	return false;
}

shared_ptr<TileEntity> SignTile::newTileEntity(Level *level)
{
	//try {
	// 4J Stu - For some reason the newInstance wasn't working right, but doing it like the other TileEntities is fine
	return shared_ptr<TileEntity>( new SignTileEntity() );
	//return dynamic_pointer_cast<TileEntity>( clas->newInstance() );
	//} catch (Exception e) {
	// TODO 4J Stu - Exception handling
	//    throw new RuntimeException(e);
	//}
}

int SignTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Item::sign->id;
}

void SignTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	bool remove = false;

	if (onGround)
	{
		if (!level->getMaterial(x, y - 1, z)->isSolid()) remove = true;
	}
	else
	{
		int face = level->getData(x, y, z);
		remove = true;
		if (face == 2 && level->getMaterial(x, y, z + 1)->isSolid()) remove = false;
		if (face == 3 && level->getMaterial(x, y, z - 1)->isSolid()) remove = false;
		if (face == 4 && level->getMaterial(x + 1, y, z)->isSolid()) remove = false;
		if (face == 5 && level->getMaterial(x - 1, y, z)->isSolid()) remove = false;
	}
	if (remove)
	{
		spawnResources(level, x, y, z, level->getData(x, y, z), 0);
		level->setTile(x, y, z, 0);
	}

	EntityTile::neighborChanged(level, x, y, z, type);
}

int SignTile::cloneTileId(Level *level, int x, int y, int z)
{
	return Item::sign_Id;
}

void SignTile::registerIcons(IconRegister *iconRegister)
{
	// None
}