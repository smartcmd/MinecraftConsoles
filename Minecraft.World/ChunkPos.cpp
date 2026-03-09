#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "ChunkPos.h"

ChunkPos::ChunkPos(int x, int z) : x( x ), z( z )
{
}

int64_t ChunkPos::hashCode(int x, int z)
{
	int64_t xx = x;
	int64_t zz = z;
	return (xx & 0xffffffffl) | ((zz & 0xffffffffl) << 32l);
}

int ChunkPos::hashCode()
{
	int64_t hash = hashCode(x, z);
	const int h1 = static_cast<int>(hash);
	const int h2 = static_cast<int>(hash >> 32l);
	return h1 ^ h2;
}

double ChunkPos::distanceToSqr(shared_ptr<Entity> e)
{
	double xPos = x * 16 + 8;
	double zPos = z * 16 + 8;

	double xd = xPos - e->x;
	double zd = zPos - e->z;

	return xd * xd + zd * zd;
}

double ChunkPos::distanceToSqr(double px, double pz)
{
	double xPos = x * 16 + 8;
	double zPos = z * 16 + 8;

	double xd = xPos - px;
	double zd = zPos - pz;

	return xd * xd + zd * zd;
}

int ChunkPos::getMiddleBlockX()
{
	return ( x << 4 ) + 8;
}

int ChunkPos::getMiddleBlockZ()
{
	return ( z << 4 ) + 8;
}

TilePos ChunkPos::getMiddleBlockPosition(int y)
{
	return TilePos(getMiddleBlockX(), y, getMiddleBlockZ());
}

wstring ChunkPos::toString()
{
    return L"[" + std::to_wstring(x) + L", " + std::to_wstring(z) + L"]";
}

int64_t ChunkPos::hash_fnct(const ChunkPos &k)
{
	return k.hashCode(k.x,k.z);
}

bool ChunkPos::eq_test(const ChunkPos &x, const ChunkPos &y)
{
	return x.x == y.x && x.z == y.z;
}

