#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.network.packet.h"
#include "SkullTileEntity.h"

SkullTileEntity::SkullTileEntity()
{
	skullType = 0;
	rotation = 0;
	extraType = L"";
}

void SkullTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putByte(L"SkullType", (BYTE) (skullType & 0xff));
	tag->putByte(L"Rot", (BYTE) (rotation & 0xff));
	tag->putString(L"ExtraType", extraType);
}

void SkullTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	skullType = tag->getByte(L"SkullType");
	rotation = tag->getByte(L"Rot");
	if (tag->contains(L"ExtraType")) extraType = tag->getString(L"ExtraType");
}

shared_ptr<Packet> SkullTileEntity::getUpdatePacket()
{
	CompoundTag *tag = new CompoundTag();
	save(tag);
	return shared_ptr<TileEntityDataPacket>(new TileEntityDataPacket(x, y, z, TileEntityDataPacket::TYPE_SKULL, tag));
}

void SkullTileEntity::setSkullType(int skullType, const wstring &extra)
{
	this->skullType = skullType;
	this->extraType = extra;
}

int SkullTileEntity::getSkullType()
{
	return skullType;
}

int SkullTileEntity::getRotation()
{
	return rotation;
}

void SkullTileEntity::setRotation(int rot)
{
	rotation = rot;
}

wstring SkullTileEntity::getExtraType()
{
	return extraType;
}

// 4J Added
shared_ptr<TileEntity> SkullTileEntity::clone()
{
	shared_ptr<SkullTileEntity> result = shared_ptr<SkullTileEntity>( new SkullTileEntity() );
	TileEntity::clone(result);
	
	result->skullType = skullType;
	result->rotation = rotation;
	result->extraType = extraType;
	return result;
}