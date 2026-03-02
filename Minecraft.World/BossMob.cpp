#include "stdafx.h"
#include "BossMobPart.h"
#include "BossMob.h"



BossMob::BossMob(Level *level) : Mob( level )
{
	
	maxHealth = 100;

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();
}

int BossMob::getMaxHealth()
{
	return maxHealth;
}

bool BossMob::hurt(shared_ptr<BossMobPart> bossMobPart, DamageSource *source, int damage)
{
	return hurt(source, damage);
}

bool BossMob::hurt(DamageSource *source, int damage)
{
	return false;
}

bool BossMob::reallyHurt(DamageSource *source, int damage)
{

	return Mob::hurt(source, damage);

}