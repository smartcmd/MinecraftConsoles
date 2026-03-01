#include "stdafx.h"
#include "net.minecraft.world.scores.criteria.h"
#include "ObjectiveCriteria.h"


unordered_map<wstring, ObjectiveCriteria *> ObjectiveCriteria::CRITERIA_BY_NAME;

ObjectiveCriteria *ObjectiveCriteria::DUMMY = new DummyCriteria(L"dummy");
ObjectiveCriteria *ObjectiveCriteria::DEATH_COUNT = new DummyCriteria(L"deathCount");
ObjectiveCriteria *ObjectiveCriteria::KILL_COUNT_PLAYERS = new DummyCriteria(L"playerKillCount");
ObjectiveCriteria *ObjectiveCriteria::KILL_COUNT_ALL = new DummyCriteria(L"totalKillCount");
ObjectiveCriteria *ObjectiveCriteria::HEALTH = new HealthCriteria(L"health");