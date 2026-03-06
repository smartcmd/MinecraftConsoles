#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "BinaryHeap.h"
#include "Node.h"
#include "Path.h"
#include "PathFinder.h"

PathFinder::PathFinder(LevelSource *level, bool canPassDoors, bool canOpenDoors, bool avoidWater, bool canFloat)
{
	neighbors = new NodeArray(32);

	this->canPassDoors = canPassDoors;
	this->canOpenDoors = canOpenDoors;
	this->avoidWater = avoidWater;
	this->canFloat = canFloat;
	this->level = level;
}

PathFinder::~PathFinder()
{
	// All the nodes should be uniquely referenced in the nodes map, and everything else should just be duplicate
	// references to the same things, so just need to destroy their containers
	delete [] neighbors->data;
	delete neighbors;
	for(auto& node : nodes)
	{
		delete node.second;
	}
}

Path *PathFinder::findPath(Entity *from, Entity *to, float maxDist)
{
	return findPath(from, to->x, to->bb->y0, to->z, maxDist);
}

Path *PathFinder::findPath(Entity *from, int x, int y, int z, float maxDist)
{
	return findPath(from, x + 0.5f, y + 0.5f, z + 0.5f, maxDist);
}

Path *PathFinder::findPath(Entity *e, double xt, double yt, double zt, float maxDist)
{
	openSet.clear();
	nodes.clear();

	bool resetAvoidWater = avoidWater;
	int startY = Mth::floor(e->bb->y0 + 0.5f);
	if (canFloat && e->isInWater())
	{
		startY = (int) (e->bb->y0);
		int tileId = level->getTile((int) Mth::floor(e->x), startY, (int) Mth::floor(e->z));
		while (tileId == Tile::water_Id || tileId == Tile::calmWater_Id)
		{
			++startY;
			tileId = level->getTile((int) Mth::floor(e->x), startY, (int) Mth::floor(e->z));
		}
		resetAvoidWater = avoidWater;
		avoidWater = false;
	} else startY = Mth::floor(e->bb->y0 + 0.5f);

	Node *from = getNode((int) floor(e->bb->x0), startY, (int) floor(e->bb->z0));
	Node *to = getNode((int) floor(xt - e->bbWidth / 2), (int) floor(yt), (int) floor(zt - e->bbWidth / 2));

	Node *size = new Node((int) floor(e->bbWidth + 1), (int) floor(e->bbHeight + 1), (int) floor(e->bbWidth + 1));
	Path *path = findPath(e, from, to, size, maxDist);
	delete size;

	avoidWater = resetAvoidWater;
	return path;
}

// function A*(start,goal)
Path *PathFinder::findPath(Entity *e, Node *from, Node *to, Node *size, float maxDist)
{
	from->g = 0;
	from->h = from->distanceToSqr(to);
	from->f = from->h;

	openSet.clear();
	openSet.insert(from);

	Node *closest = from;

	while (!openSet.isEmpty())
	{
		Node *x = openSet.pop();

		if (x->equals(to))
		{
			return reconstruct_path(from, to);
		}

		if (x->distanceToSqr(to) < closest->distanceToSqr(to))
		{
			closest = x;
		}
		x->closed = true;

		int neighborCount = getNeighbors(e, x, size, to, maxDist);
		for (int i = 0; i < neighborCount; i++)
		{
			Node *y = neighbors->data[i];

			float tentative_g_score = x->g + x->distanceToSqr(y);
			if (!y->inOpenSet() || tentative_g_score < y->g)
			{
				y->cameFrom = x;
				y->g = tentative_g_score;
				y->h = y->distanceToSqr(to);
				if (y->inOpenSet())
				{
					openSet.changeCost(y, y->g + y->h);
				}
				else
				{
					y->f = y->g + y->h;
					openSet.insert(y);
				}
			}
		}
	}

	if (closest == from) return NULL;
	return reconstruct_path(from, closest);
}

int PathFinder::getNeighbors(Entity *entity, Node *pos, Node *size, Node *target, float maxDist)
{
	int p = 0;

	int jumpSize = 0;
	if (isFree(entity, pos->x, pos->y + 1, pos->z, size) == TYPE_OPEN) jumpSize = 1;

	Node *n = getNode(entity, pos->x, pos->y, pos->z + 1, size, jumpSize);
	Node *w = getNode(entity, pos->x - 1, pos->y, pos->z, size, jumpSize);
	Node *e = getNode(entity, pos->x + 1, pos->y, pos->z, size, jumpSize);
	Node *s = getNode(entity, pos->x, pos->y, pos->z - 1, size, jumpSize);

	if (n != NULL && !n->closed && n->distanceTo(target) < maxDist) neighbors->data[p++] = n;
	if (w != NULL && !w->closed && w->distanceTo(target) < maxDist) neighbors->data[p++] = w;
	if (e != NULL && !e->closed && e->distanceTo(target) < maxDist) neighbors->data[p++] = e;
	if (s != NULL && !s->closed && s->distanceTo(target) < maxDist) neighbors->data[p++] = s;

	return p;
}

Node *PathFinder::getNode(Entity *entity, int x, int y, int z, Node *size, int jumpSize)
{
	Node *best = NULL;
	int pathType = isFree(entity, x, y, z, size);
	if (pathType == TYPE_WALKABLE) return getNode(x, y, z);
	if (pathType == TYPE_OPEN) best = getNode(x, y, z);
	if (best == NULL && jumpSize > 0 && pathType != TYPE_FENCE && pathType != TYPE_TRAP && isFree(entity, x, y + jumpSize, z, size) == TYPE_OPEN)
	{
		best = getNode(x, y + jumpSize, z);
		y += jumpSize;
	}

	if (best != NULL)
	{
		int drop = 0;
		int cost = 0;
		while (y > 0)
		{
			cost = isFree(entity, x, y - 1, z, size);
			if (avoidWater && cost == TYPE_WATER) return NULL;
			if (cost != TYPE_OPEN) break;
			// fell too far?
            if (++drop >= 4) return NULL;								// 4J - rolling this back to pre-java 1.6.4 version as we're suspicious of the performance implications of this
//			if (drop++ >= entity->getMaxFallDistance()) return NULL;
			y--;

			if (y > 0) best = getNode(x, y, z);
		}
		// fell into lava?
		if (cost == TYPE_LAVA) return NULL;
	}

	return best;
}

/*final*/ Node *PathFinder::getNode(int x, int y, int z)
{
	int i = Node::createHash(x, y, z);
	Node *node = nullptr;
    auto it = nodes.find(i);
    if ( it == nodes.end() )
	{
		MemSect(54);
		node = new Node(x, y, z);
		MemSect(0);
		nodes.insert( unordered_map<int, Node *>::value_type(i, node) );
	}
	else
	{
		node = (*it).second;
	}
	return node;
}

int PathFinder::isFree(Entity *entity, int x, int y, int z, Node *size)
{
	return isFree(entity, x, y, z, size, avoidWater, canOpenDoors, canPassDoors);
}

int PathFinder::isFree(Entity *entity, int x, int y, int z, Node *size, bool avoidWater, bool canOpenDoors, bool canPassDoors)
{
	bool walkable = false;
	for (int xx = x; xx < x + size->x; xx++)
		for (int yy = y; yy < y + size->y; yy++)
			for (int zz = z; zz < z + size->z; zz++)
			{
				int tileId = entity->level->getTile(xx, yy, zz);
				if(tileId <= 0) continue;
				if (tileId == Tile::trapdoor_Id) walkable = true;
				else if (tileId == Tile::water_Id || tileId == Tile::calmWater_Id)
				{
					if (avoidWater) return TYPE_WATER;
					else walkable = true;
				}
				else if (!canPassDoors && tileId == Tile::door_wood_Id)
				{
					return TYPE_BLOCKED;
				}

				Tile *tile = Tile::tiles[tileId];

				// 4J Stu - Use new getTileRenderShape passing in the tileId we have already got
				if (entity->level->getTileRenderShape(tileId) == Tile::SHAPE_RAIL)
				{
					int xt = Mth::floor(entity->x);
					int yt = Mth::floor(entity->y);
					int zt = Mth::floor(entity->z);
					if (entity->level->getTileRenderShape(xt, yt, zt) == Tile::SHAPE_RAIL
						|| entity->level->getTileRenderShape(xt, yt - 1, zt) == Tile::SHAPE_RAIL)
					{
							continue;
					}
					else
					{
						return TYPE_FENCE;
					}
				}

				if (tile->isPathfindable(entity->level, xx, yy, zz)) continue;
				if (canOpenDoors && tileId == Tile::door_wood_Id) continue;

				int renderShape = tile->getRenderShape();
				if (renderShape == Tile::SHAPE_FENCE || tileId == Tile::fenceGate_Id || renderShape == Tile::SHAPE_WALL) return TYPE_FENCE;
				if (tileId == Tile::trapdoor_Id) return TYPE_TRAP;
				Material *m = tile->material;
				if (m == Material::lava)
				{
					if (entity->isInLava()) continue;
					return TYPE_LAVA;
				}
				return TYPE_BLOCKED;
			}

			return walkable ? TYPE_WALKABLE : TYPE_OPEN;
}

// function reconstruct_path(came_from,current_node)
Path *PathFinder::reconstruct_path(Node *from, Node *to)
{
	int count = 1;
	Node *n = to;
	while (n->cameFrom != NULL)
	{
		count++;
		n = n->cameFrom;
	}

	NodeArray nodes = NodeArray(count);
	n = to;
	nodes.data[--count] = n;
	while (n->cameFrom != NULL)
	{
		n = n->cameFrom;
		nodes.data[--count] = n;
	}
	Path *ret = new Path(nodes);
	delete [] nodes.data;
	return ret;
}