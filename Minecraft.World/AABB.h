#pragma once

#include "Vec3.h"
#include "Definitions.h"

class HitResult;
using namespace std;

class AABB 
{
	// 4J added so we can have separate pools for different threads
	class ThreadStorage
	{
	public:
		static const int POOL_SIZE = 1024;
		AABB *pool;
		unsigned int poolPointer;
		ThreadStorage();
		~ThreadStorage();
	};
	static DWORD tlsIdx;
	static ThreadStorage *tlsDefault;
public:
	// Each new thread that needs to use Vec3 pools will need to call one of the following 2 functions, to either create its own
	// local storage, or share the default storage already allocated by the main thread
	static void CreateNewThreadStorage();
	static void UseDefaultThreadStorage();
	static void ReleaseThreadStorage();

	static AABB *newPermanent(double x0, double y0, double z0, double x1, double y1, double z1); 
	static void clearPool();
	static void resetPool();
	static AABB *newTemp(double x0, double y0, double z0, double x1, double y1, double z1);

	double x0, y0, z0;
	double x1, y1, z1;

private:
	AABB(double x0, double y0, double z0, double x1, double y1, double z1);
	AABB() {}
public:
	AABB *set(double x0, double y0, double z0, double x1, double y1, double z1);
	AABB *expand(double xa, double ya, double za);
	AABB *grow(double xa, double ya, double za);
	AABB *minmax(AABB *other);
	AABB *cloneMove(double xa, double ya, double za);
	double clipXCollide(AABB *c, double xa);
	double clipYCollide(AABB *c, double ya);
	double clipZCollide(AABB *c, double za);
	bool intersects(AABB *c);
	bool intersectsInner(AABB *c);
	AABB *move(double xa, double ya, double za);
	bool intersects(double x02, double y02, double z02, double x12, double y12, double z12);
	bool contains(Vec3 *p);
	bool containsIncludingLowerBound(Vec3 *p); // 4J Added
	double getSize();
	AABB *shrink(double xa, double ya, double za);
	AABB *copy();
	HitResult *clip(Vec3 *a, Vec3 *b);
	bool containsX(Vec3 *v);
	bool containsY(Vec3 *v);
	bool containsZ(Vec3 *v);
	void set(AABB *b);
	wstring toString();
};
