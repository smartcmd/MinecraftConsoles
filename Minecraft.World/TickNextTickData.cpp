#include "stdafx.h"
#include "net.minecraft.world.level.tile.h"
#include "TickNextTickData.h"

int64_t TickNextTickData::C = 0;

TickNextTickData::TickNextTickData(int x, int y, int z, int tileId)
{
	m_delay = 0;
	c = C++;

	this->x = x;
	this->y = y;
	this->z = z;
	this->tileId = tileId;
	priorityTilt = 0;
}


bool TickNextTickData::equals(const TickNextTickData *o) const
{
	// TODO 4J Is this safe to cast it before we do a dynamic_cast? Will the dynamic_cast still fail?
	// We cannot dynamic_cast a void*
	if ( o != NULL)
	{
		TickNextTickData *t = (TickNextTickData *) o;
		return x == t->x && y == t->y && z == t->z && Tile::isMatching(tileId, t->tileId);
	}
	return false;
}

int TickNextTickData::hashCode() const
{
	return (((x * 1024 * 1024) + (z * 1024) + y) * 256);
}

TickNextTickData *TickNextTickData::delay(int64_t l)
{
	m_delay = l;
	return this;
}

void TickNextTickData::setPriorityTilt(int priorityTilt)
{
	this->priorityTilt = priorityTilt;
}

int TickNextTickData::compareTo(const TickNextTickData *tnd) const
{
	if (m_delay < tnd->m_delay) return -1;
	if (m_delay > tnd->m_delay) return 1;
	if (priorityTilt != tnd->priorityTilt) return priorityTilt - tnd->priorityTilt;
	if (c < tnd->c) return -1;
	if (c > tnd->c) return 1;
	return 0;
}

bool TickNextTickData::operator==(const TickNextTickData &k)
{
	return equals( &k );
}

//A class that takes two arguments of the same type as the container elements and returns a bool.
//The expression comp(a,b), where comp is an object of this comparison class and a and b are elements of the container,
//shall return true if a is to be placed at an earlier position than b in a strict weak ordering operation.
//This can either be a class implementing a function call operator or a pointer to a function (see constructor for an example).
//This defaults to less<Key>, which returns the same as applying the less-than operator (a<b).
bool TickNextTickData::compare_fnct(const TickNextTickData &x, const TickNextTickData &y)
{
	return x.compareTo( &y ) < 0;
}

int TickNextTickData::hash_fnct(const TickNextTickData &k)
{
	return k.hashCode();
}

bool TickNextTickData::eq_test(const TickNextTickData &x, const TickNextTickData &y)
{
	return x.equals(&y);
}