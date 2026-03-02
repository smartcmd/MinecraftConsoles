#pragma once

class Scoreboard;
class ObjectiveCriteria;

class Objective
{
public:
	static const int MAX_NAME_LENGTH = 16;
	static const int MAX_DISPLAY_NAME_LENGTH = 32;

private:
	Scoreboard *scoreboard;
	wstring name;
	ObjectiveCriteria *criteria;
	wstring displayName;

public:
	Objective(Scoreboard *scoreboard, const wstring &name, ObjectiveCriteria *criteria);

	Scoreboard *getScoreboard();
	wstring getName();
	ObjectiveCriteria *getCriteria();
	wstring getDisplayName();
	void setDisplayName(const wstring &name);
};