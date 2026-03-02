#pragma once

class MultiEntityMobPart;

class MultiEntityMob
{
public:
	virtual Level *getLevel() = 0;
	virtual bool hurt(shared_ptr<MultiEntityMobPart> MultiEntityMobPart, DamageSource *source, float damage) = 0;
};