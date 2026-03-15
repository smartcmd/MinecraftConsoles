#pragma once

#include "Cow.h"
#include "Entity.h"

class MushroomCow : public Cow
{
public:
	eINSTANCEOF GetType() { return eTYPE_MUSHROOMCOW; }
	static Entity *create(Level *level) { return new LightningBolt(level); }

public:
	MushroomCow(Level *level);

	virtual bool mobInteract(shared_ptr<Player> player);
	virtual bool canSpawn();	// 4J added
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> target);
};