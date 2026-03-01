#pragma once

#include "Spider.h"

class CaveSpider : public Spider
{
public:
	eINSTANCEOF GetType() { return eTYPE_CAVESPIDER; }
	static Entity *create(Level *level) { return new CaveSpider(level); }

public:
	CaveSpider(Level *level);

protected:
	void registerAttributes();

public:
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
};