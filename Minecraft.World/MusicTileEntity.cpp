#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"

#include "MusicTileEntity.h"



MusicTileEntity::MusicTileEntity() : TileEntity()
{
	note = 0;

	on = false;
}

void MusicTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putByte(L"note", note);
}

void MusicTileEntity::load(CompoundTag *tag) 
{
	TileEntity::load(tag);
	note = tag->getByte(L"note");
	if (note < 0) note = 0;
	if (note > 24) note = 24;
}

void MusicTileEntity::tune() 
{
	note = static_cast<byte>((note + 1) % 25);
	setChanged();
}

void MusicTileEntity::playNote(Level *level, int x, int y, int z)
{
	if (level->getMaterial(x, y + 1, z) != Material::air) return;

	Material *m = level->getMaterial(x, y - 1, z);

	int i = 0;
	if (m == Material::stone) i = 1;
	if (m == Material::sand) i = 2;
	if (m == Material::glass) i = 3;
	if (m == Material::wood) i = 4;

	level->tileEvent(x, y, z, Tile::noteblock_Id, i, note);
}

// 4J Added
shared_ptr<TileEntity> MusicTileEntity::clone()
{
	shared_ptr<MusicTileEntity> result = std::make_shared<MusicTileEntity>();
	TileEntity::clone(result);

	result->note = note;
	return result;
}

