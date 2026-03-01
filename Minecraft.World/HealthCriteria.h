#pragma once

#include "DummyCriteria.h"

class HealthCriteria : public DummyCriteria
{
public:
	HealthCriteria(const wstring &id);

	int getScoreModifier(vector<shared_ptr<Player> > *players);
	bool isReadOnly();
};