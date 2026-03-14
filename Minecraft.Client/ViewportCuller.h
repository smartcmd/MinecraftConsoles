#pragma once
#include "Culler.h"
#include "../Minecraft.World/Mob.h"

class Mob;
class ViewportCuller : public Culler
{
private:
	class Face
	{
	private:
        double xc, yc, zc;
        double xd, yd, zd;
        double cullOffs;
	public:
		Face() {}	// 4J - added so we can declare an array of these (unitialised) in the class
		Face(double x, double y, double z, float yRot, float xRot);  
        bool inFront(double x, double y, double z, double r);
        bool inFront(double x0, double y0, double z0, double x1, double y1, double z1);
        bool fullyInFront(double x0, double y0, double z0, double x1, double y1, double z1);      
    };

private:
	Face faces[6];
    double xOff, yOff, zOff;
public:
	ViewportCuller(shared_ptr<LivingEntity> mob, double fogDistance, float a);
    virtual bool isVisible(AABB bb);
    virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
    virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1);
private:
	double _max(double a, double b, double c);		// 4J - renamed from _max due to #define conflict in stdlib
public:
    virtual void prepare(double xOff, double yOff, double zOff);
};
