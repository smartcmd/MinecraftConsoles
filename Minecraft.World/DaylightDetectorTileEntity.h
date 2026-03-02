#pragma once

class DaylightDetectorTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_DAYLIGHTDETECTORTILEENTITY; }
	static TileEntity *create() { return new DaylightDetectorTileEntity(); }

	// 4J Added
	virtual shared_ptr<TileEntity> clone();

public:
	DaylightDetectorTileEntity();

	void tick();
};