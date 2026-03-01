#include "stdafx.h"
#include "net.minecraft.world.scores.h"
#include "Objective.h"

Objective::Objective(Scoreboard *scoreboard, const wstring &name, ObjectiveCriteria *criteria)
{
	this->scoreboard = scoreboard;
	this->name = name;
	this->criteria = criteria;

	displayName = name;
}

Scoreboard *Objective::getScoreboard()
{
	return scoreboard;
}

wstring Objective::getName()
{
	return name;
}

ObjectiveCriteria *Objective::getCriteria()
{
	return criteria;
}

wstring Objective::getDisplayName()
{
	return displayName;
}

void Objective::setDisplayName(const wstring &name)
{
	displayName = name;
	scoreboard->onObjectiveChanged(this);
}