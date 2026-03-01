#include "stdafx.h"
#include "net.minecraft.world.scores.h"
#include "net.minecraft.world.scores.criteria.h"
#include "Score.h"

Score::Score(Scoreboard *scoreboard, Objective *objective, const wstring &owner)
{
	this->scoreboard = scoreboard;
	this->objective = objective;
	this->owner = owner;
	count = 0;
}

void Score::add(int count)
{
	//if (objective.getCriteria().isReadOnly()) throw new IllegalStateException("Cannot modify read-only score");
	setScore(getScore() + count);
}

void Score::remove(int count)
{
	//if (objective.getCriteria().isReadOnly()) throw new IllegalStateException("Cannot modify read-only score");
	setScore(getScore() - count);
}

void Score::increment()
{
	//if (objective.getCriteria().isReadOnly()) throw new IllegalStateException("Cannot modify read-only score");
	add(1);
}

void Score::decrement()
{
	//if (objective.getCriteria().isReadOnly()) throw new IllegalStateException("Cannot modify read-only score");
	remove(1);
}

int Score::getScore()
{
	return count;
}

void Score::setScore(int score)
{
	int old = count;
	count = score;
	if (old != score) getScoreboard()->onScoreChanged(this);
}

Objective *Score::getObjective()
{
	return objective;
}

wstring Score::getOwner()
{
	return owner;
}

Scoreboard *Score::getScoreboard()
{
	return scoreboard;
}

void Score::updateFor(vector<shared_ptr<Player> > *players)
{
	setScore(objective->getCriteria()->getScoreModifier(players));
}