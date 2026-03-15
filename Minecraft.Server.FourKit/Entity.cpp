#include "Entity.h"
#include "Events.h"
#include "World.h"

Entity::Entity()
	: m_entityId(-1)
	, m_entityType(EntityType::UNKNOWN)
	, m_x(0.0)
	, m_y(0.0)
	, m_z(0.0)
	, m_dimension(0)
	, m_fallDistance(0.0f)
{
}

Entity::Entity(int entityId, EntityType type, double x, double y, double z, int dimension)
	: m_entityId(entityId)
	, m_entityType(type)
	, m_x(x)
	, m_y(y)
	, m_z(z)
	, m_dimension(dimension)
	, m_fallDistance(0.0f)
{
}

void Entity::SetEntityData(int entityId, EntityType type, double x, double y, double z, int dimension,
                           float fallDistance, bool onGround, bool dead)
{
	m_entityId = entityId;
	m_entityType = type;
	m_x = x;
	m_y = y;
	m_z = z;
	m_dimension = dimension;
	m_fallDistance = fallDistance;
}

Location^ Entity::getLocation()
{
	return gcnew Location(m_x, m_y, m_z, m_dimension);
}

World^ Entity::getWorld()
{
	return gcnew World(m_dimension);
}

bool Entity::teleport(Location^ location)
{
	if (location == nullptr) return false;
	m_x = location->getX();
	m_y = location->getY();
	m_z = location->getZ();
	return true;
}
