#pragma once

#include "ObjectiveCriteria.h"

class DummyCriteria : public ObjectiveCriteria
{
private:
	wstring name;

public:
	DummyCriteria(const wstring &name);

	virtual wstring getName();
	virtual int getScoreModifier(vector<shared_ptr<Player> > *players);
	virtual bool isReadOnly();
};