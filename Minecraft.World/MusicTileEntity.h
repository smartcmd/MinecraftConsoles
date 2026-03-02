#pragma once
using namespace std;

#include "TileEntity.h"

class MusicTileEntity :  public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_MUSICTILEENTITY; }
	static TileEntity *create() { return new MusicTileEntity(); }

public:
	byte note;

	bool on;

	MusicTileEntity();

	virtual void save(CompoundTag *tag);
	virtual void load(CompoundTag *tag);
	void tune();
	void playNote(Level *level, int x, int y, int z);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};
