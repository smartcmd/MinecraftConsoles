#pragma once

#include "TileEntity.h"

class SkullTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_SKULLTILEENTITY; }
	static TileEntity *create() { return new SkullTileEntity(); }
public:
	static const int TYPE_SKELETON = 0;
	static const int TYPE_WITHER = 1;
	static const int TYPE_ZOMBIE = 2;
	static const int TYPE_CHAR = 3;
	static const int TYPE_CREEPER = 4;

private:
	int skullType;
	int rotation;
	wstring extraType;

public:
	SkullTileEntity();

	void save(CompoundTag *tag);
	void load(CompoundTag *tag);
	shared_ptr<Packet> getUpdatePacket();
	void setSkullType(int skullType, const wstring &extra);
	int getSkullType();
	int getRotation();
	void setRotation(int rot);
	wstring getExtraType();

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};