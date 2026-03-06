#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "HealthCriteria.h"

HealthCriteria::HealthCriteria(const wstring &id) : DummyCriteria(id)
{
}

int HealthCriteria::getScoreModifier(vector<shared_ptr<Player> > *players)
{
	float health = 0;

	for (auto& player : *players)
	{
		health += player->getHealth() + player->getAbsorptionAmount();
	}

	if (players->size() > 0) health /= players->size();

	return Mth::ceil(health);
}

bool HealthCriteria::isReadOnly()
{
	return true;
}