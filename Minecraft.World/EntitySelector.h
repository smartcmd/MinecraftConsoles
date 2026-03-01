#pragma once

class EntitySelector
{
public:
	static const EntitySelector *ENTITY_STILL_ALIVE;
	static const EntitySelector *CONTAINER_ENTITY_SELECTOR; 

	virtual bool matches(shared_ptr<Entity> entity) const = 0;
};

class AliveEntitySelector : public EntitySelector
{
public:
	bool matches(shared_ptr<Entity> entity) const;
};

class ContainerEntitySelector : public EntitySelector
{
public:
	bool matches(shared_ptr<Entity> entity) const;
};

class MobCanWearArmourEntitySelector : public EntitySelector
{
private:
	shared_ptr<ItemInstance> item;

public:
	MobCanWearArmourEntitySelector(shared_ptr<ItemInstance> item);
	bool matches(shared_ptr<Entity> entity) const;
};