#pragma once
using namespace std;

#include "Stat.h"

class DescFormatter;

class Achievement : public Stat
{
public:
	const int x, y;
	Achievement *requires;

private: 
	const wstring desc;
    DescFormatter *descFormatter;

public:
	const shared_ptr<ItemInstance> icon;

private:
    bool isGoldenVar;
	void _init();

public:
	Achievement(int id, const wstring& name, int x, int y, Item *icon, Achievement *requires);
    Achievement(int id, const wstring& name, int x, int y, Tile *icon, Achievement *requires);
    Achievement(int id, const wstring& name, int x, int y, shared_ptr<ItemInstance> icon, Achievement *requires);

	Achievement *setAwardLocallyOnly();
	Achievement *setGolden();
	Achievement *postConstruct();
	bool isAchievement();
	wstring getDescription();
	Achievement *setDescFormatter(DescFormatter *descFormatter);
	bool isGolden();
	int getAchievementID();
};
