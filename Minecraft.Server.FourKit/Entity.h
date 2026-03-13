#pragma once

#include "CommandSender.h"

using namespace System;
using namespace System::Collections::Generic;

ref class Location;
ref class World;
ref class EntityDamageEvent;

public enum class EntityType
{
	UNKNOWN = 0,
	DROPPED_ITEM = 1,
	EXPERIENCE_ORB = 2,
	LEASH_HITCH = 3,
	PAINTING = 4,
	ARROW = 5,
	SNOWBALL = 6,
	FIREBALL = 7,
	SMALL_FIREBALL = 8,
	ENDER_PEARL = 9,
	ENDER_SIGNAL = 10,
	THROWN_EXP_BOTTLE = 11,
	ITEM_FRAME = 12,
	WITHER_SKULL = 13,
	PRIMED_TNT = 14,
	FALLING_BLOCK = 15,
	FIREWORK = 16,
	MINECART = 17,
	MINECART_CHEST = 18,
	MINECART_FURNACE = 19,
	MINECART_TNT = 20,
	MINECART_HOPPER = 21,
	MINECART_MOB_SPAWNER = 22,
	MINECART_COMMAND = 23,
	BOAT = 24,
	CREEPER = 25,
	SKELETON = 26,
	SPIDER = 27,
	GIANT = 28,
	ZOMBIE = 29,
	SLIME = 30,
	GHAST = 31,
	PIG_ZOMBIE = 32,
	ENDERMAN = 33,
	CAVE_SPIDER = 34,
	SILVERFISH = 35,
	BLAZE = 36,
	MAGMA_CUBE = 37,
	ENDER_DRAGON = 38,
	WITHER = 39,
	BAT = 40,
	WITCH = 41,
	ENDERMITE = 42,
	GUARDIAN = 43,
	PIG = 44,
	SHEEP = 45,
	COW = 46,
	CHICKEN = 47,
	SQUID = 48,
	WOLF = 49,
	MUSHROOM_COW = 50,
	SNOWMAN = 51,
	OCELOT = 52,
	IRON_GOLEM = 53,
	HORSE = 54,
	RABBIT = 55,
	VILLAGER = 56,
	FISHING_HOOK = 57,
	LIGHTNING = 58,
	WEATHER = 59,
	PLAYER = 60,
	COMPLEX_PART = 61,
	EGG = 62,
	SPLASH_POTION = 63,
	ARMOR_STAND = 64,
	UNKNOWN_ENTITY = 65
};

public ref class Entity : public CommandSender
{
public:
	Entity();
	Entity(int entityId, EntityType type, double x, double y, double z, int dimension);

	virtual double getX() { return m_x; }
	virtual double getY() { return m_y; }
	virtual double getZ() { return m_z; }
	virtual int getDimension() { return m_dimension; }
	virtual EntityType getType() { return m_entityType; }
	virtual float getFallDistance() { return m_fallDistance; }

	int getEntityId() { return m_entityId; }

	Location^ getLocation();
	World^ getWorld();

	virtual void setFallDistance(float distance) { m_fallDistance = distance; }

	virtual bool teleport(Location^ location);

internal:
	void SetEntityData(int entityId, EntityType type, double x, double y, double z, int dimension,
	                   float fallDistance, bool onGround, bool dead);

protected:
	int m_entityId;
	EntityType m_entityType;
	double m_x;
	double m_y;
	double m_z;
	int m_dimension;
	float m_fallDistance;
};
