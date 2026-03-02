#include "stdafx.h"

#include "DummyCriteria.h"

DummyCriteria::DummyCriteria(const wstring &name)
{
	this->name = name;
	ObjectiveCriteria::CRITERIA_BY_NAME[name] = this;
}

wstring DummyCriteria::getName()
{
	return name;
}

int DummyCriteria::getScoreModifier(vector<shared_ptr<Player> > *players)
{
	return 0;
}

bool DummyCriteria::isReadOnly()
{
	return false;
}