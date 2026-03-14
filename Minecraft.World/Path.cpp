#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "Path.h"

Path::~Path()
{
	if( nodes.data )
	{
		for( int i = 0; i < nodes.length; i++ )
			delete nodes.data[i];
		delete[] nodes.data;
	}
}

Path::Path(NodeArray nodes) 
{
	index = 0;

	length = nodes.length;
	// 4J - copying these nodes over from a NodeArray (which is an array of Node * references) to just a straight array of Nodes,
	// so that this Path is no longer dependent of Nodes allocated elsewhere and can handle its own destruction
	// Note: cameFrom pointer will be useless now but that isn't used once this is just a path
	this->nodes = NodeArray(length);

	for( int i = 0; i < length; i++ )
	{
		this->nodes.data[i] = new Node();
		memcpy(this->nodes.data[i],nodes[i],sizeof(Node));
	}
}

void Path::next() 
{
	index++;
}

bool Path::isDone() 
{
	return index >= length;
}

Node *Path::last() 
{
	if (length > 0) 
	{
		return nodes[length - 1];
	}
	return nullptr;
}

Node *Path::get(int i) 
{
	return nodes[i];
}

int Path::getSize()
{
	return length;
}

void Path::setSize(int length)
{
	this->length = length;
}

int Path::getIndex()
{
	return index;
}

void Path::setIndex(int index)
{
	this->index = index;
}

Vec3 *Path::getPos(shared_ptr<Entity> e, int index) 
{
	double x = nodes[index]->x + static_cast<int>(e->bbWidth + 1) * 0.5;
	double y = nodes[index]->y;
	double z = nodes[index]->z + static_cast<int>(e->bbWidth + 1) * 0.5;
	return Vec3::newTemp(x, y, z);
}

Vec3 *Path::currentPos(shared_ptr<Entity> e)
{
	return getPos(e, index);
}

Vec3 *Path::currentPos()
{
	return Vec3::newTemp( nodes[index]->x, nodes[index]->y, nodes[index]->z );
}

bool Path::sameAs(Path *path)
{
	if (path == nullptr) return false;
	if (path->nodes.length != nodes.length) return false;
	for (int i = 0; i < nodes.length; ++i)
		if (nodes[i]->x != path->nodes[i]->x || nodes[i]->y != path->nodes[i]->y || nodes[i]->z != path->nodes[i]->z) return false;
	return true;
}

bool Path::endsIn(Vec3 *pos)
{
	Node *lastNode = last();
	if (lastNode == nullptr) return false;
	return lastNode->x == static_cast<int>(pos->x) && lastNode->y == static_cast<int>(pos->y) && lastNode->z == static_cast<int>(pos->z);
}

bool Path::endsInXZ(Vec3 *pos)
{
	Node *lastNode = last();
	if (lastNode == nullptr) return false;
	return lastNode->x == static_cast<int>(pos->x) && lastNode->z == static_cast<int>(pos->z);
}