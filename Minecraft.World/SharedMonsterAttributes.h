#pragma once

class SharedMonsterAttributes
{
public:
	static Attribute *MAX_HEALTH;
	static Attribute *FOLLOW_RANGE;
	static Attribute *KNOCKBACK_RESISTANCE;
	static Attribute *MOVEMENT_SPEED;
	static Attribute *ATTACK_DAMAGE;

	static ListTag<CompoundTag> *saveAttributes(BaseAttributeMap *attributes);

private:
	static CompoundTag *saveAttribute(AttributeInstance *instance);
	static CompoundTag *saveAttributeModifier(AttributeModifier *modifier);

public:
	static void loadAttributes(BaseAttributeMap *attributes, ListTag<CompoundTag> *list);

private:
	static void loadAttribute(AttributeInstance *instance, CompoundTag *tag);

public:
	static AttributeModifier *loadAttributeModifier(CompoundTag *tag);
};