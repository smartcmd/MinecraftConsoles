#pragma once

class DispenseItemBehavior;

class BehaviorRegistry 
{
private:
	unordered_map<Item*, DispenseItemBehavior*> storage;
	DispenseItemBehavior *defaultBehavior;

public:
	BehaviorRegistry(DispenseItemBehavior *defaultValue);
	~BehaviorRegistry();

	DispenseItemBehavior *get(Item *key);
	void add(Item *key, DispenseItemBehavior *value);
};