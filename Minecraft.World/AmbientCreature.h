#pragma once

#include "Mob.h"
#include "Creature.h"

class AmbientCreature : public Mob, public Creature
{

public:
	AmbientCreature(Level *level);

	virtual bool canBeLeashed();

protected:
	virtual bool mobInteract(shared_ptr<Player> player);
};