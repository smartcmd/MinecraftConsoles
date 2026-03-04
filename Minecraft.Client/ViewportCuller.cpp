#include "stdafx.h"
#include "ViewportCuller.h"
#include "..\Minecraft.World\Mth.h"

ViewportCuller::Face::Face(double x, double y, double z, float yRot, float xRot)
{
    this->xc = x;
    this->yc = y;
    this->zc = z;
            
    xd = Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
    zd = -Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
    yd = -Mth::sin(xRot / 180 * PI);
        
    cullOffs = xc*xd+yc*yd+zc*zd; 
}
        
bool ViewportCuller::Face::inFront(double x, double y, double z, double r)
{
    return x*xd+y*yd+z*zd>cullOffs-r; 
}

bool ViewportCuller::Face::inFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
    if (
            x0*xd+y0*yd+z0*zd>cullOffs ||
            x1*xd+y0*yd+z0*zd>cullOffs ||
            x0*xd+y1*yd+z0*zd>cullOffs ||
            x1*xd+y1*yd+z0*zd>cullOffs ||
            x0*xd+y0*yd+z1*zd>cullOffs ||
            x1*xd+y0*yd+z1*zd>cullOffs ||
            x0*xd+y1*yd+z1*zd>cullOffs ||
            x1*xd+y1*yd+z1*zd>cullOffs 
        ) return true;
    return false;
}

bool ViewportCuller::Face::fullyInFront(double x0, double y0, double z0, double x1, double y1, double z1)
{
	if (	x0*xd+y0*yd+z0*zd<cullOffs ||
			x1*xd+y0*yd+z0*zd<cullOffs ||
			x0*xd+y1*yd+z0*zd<cullOffs ||
			x1*xd+y1*yd+z0*zd<cullOffs ||
			x0*xd+y0*yd+z1*zd<cullOffs ||
			x1*xd+y0*yd+z1*zd<cullOffs ||
			x0*xd+y1*yd+z1*zd<cullOffs ||
			x1*xd+y1*yd+z1*zd<cullOffs 
		) return false;
    return true;
}

ViewportCuller::ViewportCuller(shared_ptr<LivingEntity> mob, double fogDistance, float a)
{
	float yRot = mob->yRotO+(mob->yRot-mob->yRotO)*a;
	float xRot = mob->xRotO+(mob->xRot-mob->xRotO)*a;
       
	double x = mob->xOld+(mob->x-mob->xOld)*a; 
	double y = mob->yOld+(mob->y-mob->yOld)*a; 
	double z = mob->zOld+(mob->z-mob->zOld)*a;

	double xd = Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	double zd = -Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	double yd = -Mth::sin(xRot / 180 * PI);

	float xFov = 30;
	float yFov = 45;
	faces[0] = Face(x, y, z, yRot, xRot);
	faces[1] = Face(x, y, z, yRot+xFov, xRot);
	faces[2] = Face(x, y, z, yRot-xFov, xRot);
	faces[3] = Face(x, y, z, yRot, xRot+yFov);
	faces[4] = Face(x, y, z, yRot, xRot-yFov);
	faces[5] = Face(x+xd*fogDistance, y+yd*fogDistance, z+zd*fogDistance, yRot+180, -xRot);   
}

bool ViewportCuller::isVisible(AABB bb)
{
	return cubeInFrustum(bb.x0, bb.y0, bb.z0, bb.x1, bb.y1, bb.z1);
}

bool ViewportCuller::cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0-=xOff;
    y0-=yOff;
    z0-=zOff;
    x1-=xOff;
    y1-=yOff;
    z1-=zOff;
    double xd = (x1-x0)/2.0f;
    double yd = (y1-y0)/2.0f;
    double zd = (z1-z0)/2.0f;
        
    double xc = x0+xd;
    double yc = y0+yd;
    double zc = z0+zd;
    double r = _max(xd, yd, zd)*1.5f;
        
    if (!faces[0].inFront(xc, yc, zc, r)) return false;
    if (!faces[1].inFront(xc, yc, zc, r)) return false;
    if (!faces[2].inFront(xc, yc, zc, r)) return false;
    if (!faces[3].inFront(xc, yc, zc, r)) return false;
    if (!faces[4].inFront(xc, yc, zc, r)) return false;
    if (!faces[5].inFront(xc, yc, zc, r)) return false;

    if (!faces[0].inFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[1].inFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[2].inFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[3].inFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[4].inFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[5].inFront(x0, y0, z0, x1, y1, z1)) return false;
    return true;

}

bool ViewportCuller::cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1)
{
    x0-=xOff;
    y0-=yOff;
    z0-=zOff;
    x1-=xOff;
    y1-=yOff;
    z1-=zOff;

    double xd = (x1-x0)/2.0f;
    double yd = (y1-y0)/2.0f;
    double zd = (z1-z0)/2.0f;
        
    double xc = x0+xd;
    double yc = y0+yd;
    double zc = z0+zd;
    double r = _max(xd, yd, zd)*1.5f;

    if (!faces[0].inFront(xc, yc, zc, r)) return false;
    if (!faces[1].inFront(xc, yc, zc, r)) return false;
    if (!faces[2].inFront(xc, yc, zc, r)) return false;
    if (!faces[3].inFront(xc, yc, zc, r)) return false;
    if (!faces[4].inFront(xc, yc, zc, r)) return false;
    if (!faces[5].inFront(xc, yc, zc, r)) return false;
        
        
    if (!faces[0].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[1].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[2].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[3].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[4].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    if (!faces[5].fullyInFront(x0, y0, z0, x1, y1, z1)) return false;
    return true;
}

double ViewportCuller::_max(double a, double b, double c)
{
	return (a>b?a>c?c:a:b>c?b:c);
}

void ViewportCuller::prepare(double xOff, double yOff, double zOff)
{
    this->xOff = xOff;
    this->yOff = yOff;
    this->zOff = zOff;
}