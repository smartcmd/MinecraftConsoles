#pragma once

class Entity;
class TilePos;

class ChunkPos
{
public:
	int x, z;	// 4J - these were const but needed to make an assignment operator so we could make a vector of ChunkPos

	ChunkPos(int x, int z);

	static int64_t hashCode(int x, int z);
	int hashCode();

	double distanceToSqr(shared_ptr<Entity> e);
	double distanceToSqr(double px, double pz);	// 4J added

	int getMiddleBlockX();
	int getMiddleBlockZ();

	TilePos getMiddleBlockPosition(int y);
	wstring toString();

	static int64_t hash_fnct(const ChunkPos &k);
	static bool eq_test(const ChunkPos &x, const ChunkPos &y);
	bool operator == (const ChunkPos &k) const { return (this->x == k.x) && ( this->z == k.z); }
	ChunkPos & operator= (const ChunkPos & other) { x = other.x; z = other.z; return *this; }
};

struct ChunkPosKeyHash
{
	inline int64_t operator()(const ChunkPos &k) const
	{ return ChunkPos::hash_fnct(k); }
};

struct ChunkPosKeyEq
{
	inline bool operator()(const ChunkPos &x, const ChunkPos &y) const
	{ return ChunkPos::eq_test(x, y); }
};