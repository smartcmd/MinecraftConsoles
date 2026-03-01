#pragma once
using namespace std;

class AABB;

class Vec3
{
	// 4J added so we can have separate pools for different threads
	class ThreadStorage
	{
	public:
		static const int POOL_SIZE = 1024;
		Vec3 *pool;
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

    static Vec3 *newPermanent(double x, double y, double z);
    static void clearPool();
    static void resetPool();
    static Vec3 *newTemp(double x, double y, double z);
    double x, y, z;
private:
	Vec3() {}
    Vec3(double x, double y, double z);
    Vec3 *set(double x, double y, double z);
public:
    Vec3 *interpolateTo(Vec3 *t, double p);
    Vec3 *vectorTo(Vec3 *p);
    Vec3 *normalize();
    double dot(Vec3 *p);
    Vec3 *cross(Vec3 *p);
    Vec3 *add(double x, double y, double z);
    double distanceTo(Vec3 *p);
    double distanceToSqr(Vec3 *p);
    double distanceToSqr(double x2, double y2, double z2);
    Vec3 *scale(double l);
    double length();
    Vec3 *clipX(Vec3 *b, double xt);
    Vec3 *clipY(Vec3 *b, double yt);
    Vec3 *clipZ(Vec3 *b, double zt);
    wstring toString();
    Vec3 *lerp(Vec3 *v, double a);
    void xRot(float degs);
    void yRot(float degs);
    void zRot(float degs);

	// 4J Added
	double distanceTo(AABB *box);

	Vec3* closestPointOnLine(Vec3* p1, Vec3* p2);
	double distanceFromLine(Vec3* p1, Vec3* p2);

};