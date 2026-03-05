#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.pathfinder.h"

// 4J - added for common ctor code
// Do all the default initialisations done in the java class
void Node::_init()
{
	heapIdx = -1;

	closed = false;

	cameFrom = NULL;
}

Node::Node(const int x, const int y, const int z) :
x(x),
y(y),
z(z),
hash(createHash(x, y, z))
{
	_init();

	//this->x = x;
	//this->y = y;
	//this->z = z;

	//hash = createHash(x, y, z);
}

int Node::createHash(const int x, const int y, const int z)
{
	return (y & 0xff) | ((x & 0x7fff) << 8) | ((z & 0x7fff) << 24) | ((x < 0) ? 0x0080000000 : 0) | ((z < 0) ? 0x0000008000 : 0);
}

float Node::distanceTo(Node *to)
{
	float xd = (float) ( to->x - x );
	float yd = (float) ( to->y - y );
	float zd = (float) ( to->z - z );
	return Mth::sqrt(xd * xd + yd * yd + zd * zd);
}

float Node::distanceToSqr(Node *to)
{
	float xd = to->x - x;
	float yd = to->y - y;
	float zd = to->z - z;
	return xd * xd + yd * yd + zd * zd;
}

bool Node::equals(Node *o)
{
	//4J Jev, never used anything other than a node.
	//if (dynamic_cast<Node *>((Node *) o) != NULL)
	//{
		return hash == o->hash && x == o->x && y == o->y && z == o->z;
	//}
	//return false;
}

int Node::hashCode()
{
	return hash;
}

bool Node::inOpenSet()
{
	return heapIdx >= 0;
}

wstring Node::toString()
{
	return std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z);
}