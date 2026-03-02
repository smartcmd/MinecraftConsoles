#pragma once

class ObjectiveCriteria
{
public:
	static unordered_map<wstring, ObjectiveCriteria *> CRITERIA_BY_NAME;

	static ObjectiveCriteria *DUMMY;
	static ObjectiveCriteria *DEATH_COUNT;
	static ObjectiveCriteria *KILL_COUNT_PLAYERS;
	static ObjectiveCriteria *KILL_COUNT_ALL;
	static ObjectiveCriteria *HEALTH;

	virtual wstring getName() = 0;
	virtual int getScoreModifier(vector<shared_ptr<Player> > *players) = 0;
	virtual bool isReadOnly() = 0;
};