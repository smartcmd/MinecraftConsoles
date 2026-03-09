#include "stdafx.h"
#include "Vec3.h"
#include "AABB.h"

DWORD Vec3::tlsIdx = 0;
Vec3::ThreadStorage *Vec3::tlsDefault = nullptr;

Vec3::ThreadStorage::ThreadStorage()
{
	pool = new Vec3[POOL_SIZE];
	poolPointer = 0;
}

Vec3::ThreadStorage::~ThreadStorage()
{
	delete [] pool;
}

void Vec3::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	if(tlsDefault == nullptr )
	{
		tlsIdx = TlsAlloc();
		tlsDefault = tls;
	}
	TlsSetValue(tlsIdx, tls);
}

void Vec3::UseDefaultThreadStorage()
{
	TlsSetValue(tlsIdx, tlsDefault);
}

void Vec3::ReleaseThreadStorage()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));
	if( tls == tlsDefault ) return;

	delete tls;
}

Vec3 *Vec3::newPermanent(double x, double y, double z)
{
	return new Vec3(x,y,z);
};

void Vec3::clearPool()
{
}

void Vec3::resetPool()
{
}

Vec3 *Vec3::newTemp(double x, double y, double z)
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));
	Vec3 *thisVec = &tls->pool[tls->poolPointer];
	thisVec->set(x, y, z);
	tls->poolPointer = ( tls->poolPointer + 1 ) % ThreadStorage::POOL_SIZE;
	return thisVec;
}

Vec3::Vec3(double x, double y, double z)
{
	if (x == -0.0) x = 0.0;
	if (y == -0.0) y = 0.0;
	if (z == -0.0) z = 0.0;
	this->x = x;
	this->y = y;
	this->z = z;
}

Vec3 *Vec3::set(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
	return this;
}


Vec3 *Vec3::interpolateTo(Vec3 *t, double p)
{
	double xt = x + (t->x - x) * p;
	double yt = y + (t->y - y) * p;
	double zt = z + (t->z - z) * p;

	return Vec3::newTemp(xt, yt, zt);
}

Vec3 *Vec3::vectorTo(Vec3 *p)
{
	return Vec3::newTemp(p->x - x, p->y - y, p->z - z);
}

Vec3 *Vec3::normalize()
{
	double dist = (double) (sqrt(x * x + y * y + z * z));
	if (dist < 0.0001) return Vec3::newTemp(0, 0, 0);
	return Vec3::newTemp(x / dist, y / dist, z / dist);
}

double Vec3::dot(Vec3 *p)
{
	return x * p->x + y * p->y + z * p->z;
}

Vec3 *Vec3::cross(Vec3 *p)
{
	return Vec3::newTemp(y * p->z - z * p->y, z * p->x - x * p->z, x * p->y - y * p->x);
}

Vec3 *Vec3::add(double x, double y, double z)
{
	return Vec3::newTemp(this->x + x, this->y + y, this->z + z);
}

double Vec3::distanceTo(Vec3 *p)
{
	double xd = p->x - x;
	double yd = p->y - y;
	double zd = p->z - z;
	return (double) sqrt(xd * xd + yd * yd + zd * zd);
}

double Vec3::distanceToSqr(Vec3 *p)
{
	double xd = p->x - x;
	double yd = p->y - y;
	double zd = p->z - z;
	return xd * xd + yd * yd + zd * zd;
}

double Vec3::distanceToSqr(double x2, double y2, double z2)
{
	double xd = x2 - x;
	double yd = y2 - y;
	double zd = z2 - z;
	return xd * xd + yd * yd + zd * zd;
}

Vec3 *Vec3::scale(double l)
{
	return Vec3::newTemp(x * l, y * l, z * l);
}

double Vec3::length()
{
	return sqrt(x * x + y * y + z * z);
}

Vec3 *Vec3::clipX(Vec3 *b, double xt)
{
	double xd = b->x - x;
	double yd = b->y - y;
	double zd = b->z - z;

	if (xd * xd < 0.0000001f) return nullptr;

	double d = (xt - x) / xd;
	if (d < 0 || d > 1) return nullptr;
	return Vec3::newTemp(x + xd * d, y + yd * d, z + zd * d);
}

Vec3 *Vec3::clipY(Vec3 *b, double yt)
{
	double xd = b->x - x;
	double yd = b->y - y;
	double zd = b->z - z;

	if (yd * yd < 0.0000001f) return nullptr;

	double d = (yt - y) / yd;
	if (d < 0 || d > 1) return nullptr;
	return Vec3::newTemp(x + xd * d, y + yd * d, z + zd * d);
}

Vec3 *Vec3::clipZ(Vec3 *b, double zt)
{
	double xd = b->x - x;
	double yd = b->y - y;
	double zd = b->z - z;

	if (zd * zd < 0.0000001f) return nullptr;

	double d = (zt - z) / zd;
	if (d < 0 || d > 1) return nullptr;
	return Vec3::newTemp(x + xd * d, y + yd * d, z + zd * d);
}

wstring Vec3::toString()
{
	static wchar_t buf[128];
	swprintf(buf, 128, L"(%f,%f,%f)",x,y,z);
	return wstring(buf);
}

Vec3 *Vec3::lerp(Vec3 *v, double a)
{
	return Vec3::newTemp(x + (v->x - x) * a, y + (v->y - y) * a, z + (v->z - z) * a);
}

void Vec3::xRot(float degs)
{
	double _cos = cos(degs);		// 4J - cos/sin were floats but seems pointless wasting precision here
	double _sin = sin(degs);

	double xx = x;
	double yy = y * _cos + z * _sin;
	double zz = z * _cos - y * _sin;

	x = xx;
	y = yy;
	z = zz;
}

void Vec3::yRot(float degs)
{
	double _cos = cos(degs);		// 4J - cos/sin were floats but seems pointless wasting precision here
	double _sin = sin(degs);

	double xx = x * _cos + z * _sin;
	double yy = y;
	double zz = z * _cos - x * _sin;

	x = xx;
	y = yy;
	z = zz;
}

void Vec3::zRot(float degs)
{
	double _cos = cos(degs);		// 4J - cos/sin were floats but seems pointless wasting precision here
	double _sin = sin(degs);

	double xx = x * _cos + y * _sin;
	double yy = y * _cos - x * _sin;
	double zz = z;

	x = xx;
	y = yy;
	z = zz;
}

// Returns 0 if this point is within the box
// Otherwise returns the distance to the box
double Vec3::distanceTo(AABB *box)
{
	if(box->contains(this)) return 0;

	double xd = 0, yd = 0, zd = 0;

	if(x < box->x0) xd = box->x0 - x;
	else if( x > box->x1) xd = x - box->x1;

	if(y < box->y0) yd = box->y0 - y;
	else if( y > box->y1) yd = y - box->y1;

	if(z < box->z0) zd = box->z0 - z;
	else if( z > box->z1) zd = z - box->z1;

	return sqrt(xd * xd + yd * yd + zd * zd);
}



Vec3* Vec3::closestPointOnLine(Vec3* p1, Vec3* p2)
{
	Vec3* diff = newTemp(x-p1->x, y-p1->y, z-p1->z);
	Vec3* dir = newTemp(p2->x-p1->x, p2->y-p1->y, p2->z-p1->z);
	float dot1 = diff->dot(dir);
	if (dot1 <= 0.0f) 
		return p1;

	float dot2 = dir->dot(dir);

	if (dot2 <= dot1) 
		return p2;

	float t=dot1/dot2;
	return newTemp(p1->x + t * dir->x, p1->y + t * dir->y, p1->z + t * dir->z);
}


double Vec3::distanceFromLine(Vec3* p1, Vec3* p2)
{
	Vec3* closestPoint = closestPointOnLine(p1, p2);
	Vec3* diff = newTemp(x-closestPoint->x, y-closestPoint->y, z-closestPoint->z);
	return diff->length();
}
