#pragma once

class Scoreboard;

class ScoreHolder
{
public:
	virtual Scoreboard *getScoreboard() = 0;
};