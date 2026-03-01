#include "stdafx.h"

#include "ComparatorTileEntity.h"

void ComparatorTileEntity::save(CompoundTag *tag)
{
	TileEntity::save(tag);
	tag->putInt(L"OutputSignal", output);
}

void ComparatorTileEntity::load(CompoundTag *tag)
{
	TileEntity::load(tag);
	output = tag->getInt(L"OutputSignal");
}

int ComparatorTileEntity::getOutputSignal()
{
	return output;
}

void ComparatorTileEntity::setOutputSignal(int value)
{
	output = value;
}

// 4J Added
shared_ptr<TileEntity> ComparatorTileEntity::clone()
{
	shared_ptr<ComparatorTileEntity> result = shared_ptr<ComparatorTileEntity>( new ComparatorTileEntity() );
	TileEntity::clone(result);

	result->output = output;

	return result;
}