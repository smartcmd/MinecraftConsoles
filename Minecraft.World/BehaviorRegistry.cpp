#include "stdafx.h"

#include "BehaviorRegistry.h"

BehaviorRegistry::BehaviorRegistry(DispenseItemBehavior *defaultValue)
{
	defaultBehavior = defaultValue;
}

BehaviorRegistry::~BehaviorRegistry()
{
	for( auto& it : storage )
	{
		delete it.second;
	}

	delete defaultBehavior;
}

DispenseItemBehavior *BehaviorRegistry::get(Item *key)
{
	auto it = storage.find(key);

	return (it == storage.end()) ? defaultBehavior : it->second;
}

void BehaviorRegistry::add(Item *key, DispenseItemBehavior *value)
{
	storage.insert(make_pair(key, value));
}