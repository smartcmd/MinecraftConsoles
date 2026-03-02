#pragma once

class Vec3;
class TilePos
{
public:
	int x, y, z;

public:
	TilePos(int x, int y, int z);
	TilePos(Vec3 *p);	// 4J - brought forward from 1.2.3

	static int hash_fnct(const TilePos &k);
	static bool eq_test(const TilePos &x, const TilePos &y);
};

struct TilePosKeyHash
{
	inline int operator()(const TilePos &k) const 
	{ return TilePos::hash_fnct (k); }
};

struct TilePosKeyEq
{
	inline bool operator()(const TilePos &x, const TilePos &y) const 
	{ return TilePos::eq_test (x, y); }
};

