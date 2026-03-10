//package net.minecraft.world.phys;

//import java->util.ArrayList;
//import java->util.List;

#include "stdafx.h"
#include "AABB.h"
#include "HitResult.h"

DWORD AABB::tlsIdx = 0;
AABB::ThreadStorage *AABB::tlsDefault = nullptr;

AABB::ThreadStorage::ThreadStorage()
{
	pool = new AABB[POOL_SIZE];
	poolPointer = 0;
}


AABB::ThreadStorage::~ThreadStorage()
{
	delete pool;
}

void AABB::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	if(tlsDefault == nullptr )
	{
		tlsIdx = TlsAlloc();
		tlsDefault = tls;
	}

	TlsSetValue(tlsIdx, tls);
}

void AABB::UseDefaultThreadStorage()
{
	TlsSetValue(tlsIdx, tlsDefault);
}

void AABB::ReleaseThreadStorage()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));
	if( tls == tlsDefault ) return;

	delete tls;
}

AABB *AABB::newPermanent(double x0, double y0, double z0, double x1, double y1, double z1)
{
	return new AABB(x0, y0, z0, x1, y1, z1);
}

void AABB::clearPool()
{
}

void AABB::resetPool()
{
}

AABB *AABB::newTemp(double x0, double y0, double z0, double x1, double y1, double z1)
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));
	AABB *thisAABB = &tls->pool[tls->poolPointer];
	thisAABB->set(x0, y0, z0, x1, y1, z1);
	tls->poolPointer = ( tls->poolPointer + 1 ) % ThreadStorage::POOL_SIZE;
	return thisAABB;
}

AABB::AABB(double x0, double y0, double z0, double x1, double y1, double z1)
{
	this->x0 = x0;
	this->y0 = y0;
	this->z0 = z0;
	this->x1 = x1;
	this->y1 = y1;
	this->z1 = z1;
}


AABB *AABB::set(double x0, double y0, double z0, double x1, double y1, double z1)
{
	this->x0 = x0;
	this->y0 = y0;
	this->z0 = z0;
	this->x1 = x1;
	this->y1 = y1;
	this->z1 = z1;
	return this;
}

AABB *AABB::expand(double xa, double ya, double za)
{
	double _x0 = x0;
	double _y0 = y0;
	double _z0 = z0;
	double _x1 = x1;
	double _y1 = y1;
	double _z1 = z1;

	if (xa < 0) _x0 += xa;
	if (xa > 0) _x1 += xa;

	if (ya < 0) _y0 += ya;
	if (ya > 0) _y1 += ya;

	if (za < 0) _z0 += za;
	if (za > 0) _z1 += za;

	return AABB::newTemp(_x0, _y0, _z0, _x1, _y1, _z1);
}

AABB *AABB::grow(double xa, double ya, double za)
{
	double _x0 = x0 - xa;
	double _y0 = y0 - ya;
	double _z0 = z0 - za;
	double _x1 = x1 + xa;
	double _y1 = y1 + ya;
	double _z1 = z1 + za;

	return AABB::newTemp(_x0, _y0, _z0, _x1, _y1, _z1);
}

AABB *AABB::minmax(AABB *other)
{
	double _x0 = min(x0, other->x0);
	double _y0 = min(y0, other->y0);
	double _z0 = min(z0, other->z0);
	double _x1 = max(x1, other->x1);
	double _y1 = max(y1, other->y1);
	double _z1 = max(z1, other->z1);

	return newTemp(_x0, _y0, _z0, _x1, _y1, _z1);
}

AABB *AABB::cloneMove(double xa, double ya, double za)
{
	return AABB::newTemp(x0 + xa, y0 + ya, z0 + za, x1 + xa, y1 + ya, z1 + za);
}

double AABB::clipXCollide(AABB *c, double xa)
{
	if (c->y1 <= y0 || c->y0 >= y1) return xa;
	if (c->z1 <= z0 || c->z0 >= z1) return xa;

	if (xa > 0 && c->x1 <= x0)
	{
		double max = x0 - c->x1;
		if (max < xa) xa = max;
	}
	if (xa < 0 && c->x0 >= x1)
	{
		double max = x1 - c->x0;
		if (max > xa) xa = max;
	}

	return xa;
}

double AABB::clipYCollide(AABB *c, double ya)
{
	if (c->x1 <= x0 || c->x0 >= x1) return ya;
	if (c->z1 <= z0 || c->z0 >= z1) return ya;

	if (ya > 0 && c->y1 <= y0)
	{
		double max = y0 - c->y1;
		if (max < ya) ya = max;
	}
	if (ya < 0 && c->y0 >= y1)
	{
		double max = y1 - c->y0;
		if (max > ya) ya = max;
	}

	return ya;
}

double AABB::clipZCollide(AABB *c, double za)
{
	if (c->x1 <= x0 || c->x0 >= x1) return za;
	if (c->y1 <= y0 || c->y0 >= y1) return za;

	if (za > 0 && c->z1 <= z0)
	{
		double max = z0 - c->z1;
		if (max < za) za = max;
	}
	if (za < 0 && c->z0 >= z1)
	{
		double max = z1 - c->z0;
		if (max > za) za = max;
	}

	return za;
}

bool AABB::intersects(AABB *c)
{
	if (c->x1 <= x0 || c->x0 >= x1) return false;
	if (c->y1 <= y0 || c->y0 >= y1) return false;
	if (c->z1 <= z0 || c->z0 >= z1) return false;
	return true;
}

bool AABB::intersectsInner(AABB *c)
{
	if (c->x1 < x0 || c->x0 > x1) return false;
	if (c->y1 < y0 || c->y0 > y1) return false;
	if (c->z1 < z0 || c->z0 > z1) return false;
	return true;
}

AABB *AABB::move(double xa, double ya, double za)
{
	x0 += xa;
	y0 += ya;
	z0 += za;
	x1 += xa;
	y1 += ya;
	z1 += za;
	return this;
}

bool AABB::intersects(double x02, double y02, double z02, double x12, double y12, double z12)
{
	if (x12 <= x0 || x02 >= x1) return false;
	if (y12 <= y0 || y02 >= y1) return false;
	if (z12 <= z0 || z02 >= z1) return false;
	return true;
}

bool AABB::contains(Vec3 *p)
{
	if (p->x <= x0 || p->x >= x1) return false;
	if (p->y <= y0 || p->y >= y1) return false;
	if (p->z <= z0 || p->z >= z1) return false;
	return true;
}

// 4J Added
bool AABB::containsIncludingLowerBound(Vec3 *p)
{
	if (p->x < x0 || p->x >= x1) return false;
	if (p->y < y0 || p->y >= y1) return false;
	if (p->z < z0 || p->z >= z1) return false;
	return true;
}

double AABB::getSize()
{
	double xs = x1 - x0;
	double ys = y1 - y0;
	double zs = z1 - z0;
	return (xs + ys + zs) / 3.0f;
}

AABB *AABB::shrink(double xa, double ya, double za)
{
	double _x0 = x0 + xa;
	double _y0 = y0 + ya;
	double _z0 = z0 + za;
	double _x1 = x1 - xa;
	double _y1 = y1 - ya;
	double _z1 = z1 - za;

	return AABB::newTemp(_x0, _y0, _z0, _x1, _y1, _z1);
}

AABB *AABB::copy()
{
	return AABB::newTemp(x0, y0, z0, x1, y1, z1);
}

HitResult *AABB::clip(Vec3 *a, Vec3 *b)
{
	Vec3 *xh0 = a->clipX(b, x0);
	Vec3 *xh1 = a->clipX(b, x1);

	Vec3 *yh0 = a->clipY(b, y0);
	Vec3 *yh1 = a->clipY(b, y1);

	Vec3 *zh0 = a->clipZ(b, z0);
	Vec3 *zh1 = a->clipZ(b, z1);

	if (!containsX(xh0)) xh0 = nullptr;
	if (!containsX(xh1)) xh1 = nullptr;
	if (!containsY(yh0)) yh0 = nullptr;
	if (!containsY(yh1)) yh1 = nullptr;
	if (!containsZ(zh0)) zh0 = nullptr;
	if (!containsZ(zh1)) zh1 = nullptr;

	Vec3 *closest = nullptr;

	if (xh0 != nullptr && (closest == nullptr || a->distanceToSqr(xh0) < a->distanceToSqr(closest))) closest = xh0;
	if (xh1 != nullptr && (closest == nullptr || a->distanceToSqr(xh1) < a->distanceToSqr(closest))) closest = xh1;
	if (yh0 != nullptr && (closest == nullptr || a->distanceToSqr(yh0) < a->distanceToSqr(closest))) closest = yh0;
	if (yh1 != nullptr && (closest == nullptr || a->distanceToSqr(yh1) < a->distanceToSqr(closest))) closest = yh1;
	if (zh0 != nullptr && (closest == nullptr || a->distanceToSqr(zh0) < a->distanceToSqr(closest))) closest = zh0;
	if (zh1 != nullptr && (closest == nullptr || a->distanceToSqr(zh1) < a->distanceToSqr(closest))) closest = zh1;

	if (closest == nullptr) return nullptr;

	int face = -1;

	if (closest == xh0) face = 4;
	if (closest == xh1) face = 5;
	if (closest == yh0) face = 0;
	if (closest == yh1) face = 1;
	if (closest == zh0) face = 2;
	if (closest == zh1) face = 3;

	return new HitResult(0, 0, 0, face, closest);
}


bool AABB::containsX(Vec3 *v)
{
	if (v == nullptr) return false;
	return v->y >= y0 && v->y <= y1 && v->z >= z0 && v->z <= z1;
}

bool AABB::containsY(Vec3 *v)
{
	if (v == nullptr) return false;
	return v->x >= x0 && v->x <= x1 && v->z >= z0 && v->z <= z1;
}

bool AABB::containsZ(Vec3 *v)
{
	if (v == nullptr) return false;
	return v->x >= x0 && v->x <= x1 && v->y >= y0 && v->y <= y1;
}


void AABB::set(AABB *b)
{
	x0 = b->x0;
	y0 = b->y0;
	z0 = b->z0;
	x1 = b->x1;
	y1 = b->y1;
	z1 = b->z1;
}

wstring AABB::toString()
{
	return L"box[" + std::to_wstring(x0) + L", " + std::to_wstring(y0) + L", " + std::to_wstring(z0) + L" -> " +
		std::to_wstring(x1) + L", " + std::to_wstring(y1) + L", " + std::to_wstring(z1) + L"]";
}

