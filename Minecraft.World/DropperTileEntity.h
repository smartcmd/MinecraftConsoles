#pragma once

#include "DispenserTileEntity.h"

class DropperTileEntity : public DispenserTileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_DROPPERTILEENTITY; }
	static TileEntity *create() { return new DropperTileEntity(); }
	// 4J Added
	virtual shared_ptr<TileEntity> clone();

public:
	wstring getName();
};