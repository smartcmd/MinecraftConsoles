#pragma once

#include "TileEntity.h"

class ComparatorTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_COMPARATORTILEENTITY; }
	static TileEntity *create() { return new ComparatorTileEntity(); }

	// 4J Added
	virtual shared_ptr<TileEntity> clone();

private:
	int output;

public:
	void save(CompoundTag *tag);
	void load(CompoundTag *tag);
	int getOutputSignal();
	void setOutputSignal(int value);
};