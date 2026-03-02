#include "stdafx.h"
#include "WaterLilyTile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "AABB.h"

WaterlilyTile::WaterlilyTile(int id) : Bush(id)
{
    this->updateDefaultShape();
}

// 4J Added override
void WaterlilyTile::updateDefaultShape()
{
    float ss = 0.5f;
    float hh = 0.25f / 16.0f;
    this->setShape(0.5f - ss, 0, 0.5f - ss, 0.5f + ss, hh, 0.5f + ss);
}

int WaterlilyTile::getRenderShape()
{
	 return Tile::SHAPE_LILYPAD;
}

void WaterlilyTile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	if (source == NULL || !(dynamic_pointer_cast<Boat>(source)))
	{
		Bush::addAABBs(level, x, y, z, box, boxes, source);
	}
}

AABB *WaterlilyTile::getAABB(Level *level, int x, int y, int z)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1);
}

int WaterlilyTile::getColor() const
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Tile_WaterLily); //0x208030
}

int WaterlilyTile::getColor(int auxData)
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Tile_WaterLily); //0x208030
}

int WaterlilyTile::getColor(LevelSource *level, int x, int y, int z)
{
	return Minecraft::GetInstance()->getColourTable()->getColor(eMinecraftColour_Tile_WaterLily); //0x208030
}

int WaterlilyTile::getColor(LevelSource *level, int x, int y, int z, int data) //0x208030
{
	return getColor(level, x, y, z);
}

bool WaterlilyTile::mayPlaceOn(int tile)
{
	return tile == Tile::calmWater_Id;
}

bool WaterlilyTile::canSurvive(Level *level, int x, int y, int z)
{
	if (y < 0 || y >= Level::maxBuildHeight) return false;
    return level->getMaterial(x, y - 1, z) == Material::water && level->getData(x, y - 1, z) == 0;
}

bool WaterlilyTile::growTree(Level *level, int x, int y, int z, Random *random)
{
	return false;
}
