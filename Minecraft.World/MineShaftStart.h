#pragma once

#include "StructureStart.h"

class MineShaftStart : public StructureStart
{
public:
	static StructureStart *Create() { return new MineShaftStart(); }
	virtual EStructureStart GetType() { return eStructureStart_MineShaftStart; }

public:
	MineShaftStart();
	MineShaftStart(Level *level, Random *random, int chunkX, int chunkZ);
};