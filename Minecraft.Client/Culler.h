#pragma once
#include "..\Minecraft.World\AABB.h"

class Culler
{
public:
	virtual bool isVisible(AABB *bb) = 0;
	virtual bool cubeInFrustum(double x0, double y0, double z0, double x1, double y1, double z1) = 0;
	virtual bool cubeFullyInFrustum(double x0, double y0, double z0, double x1, double y1, double z1) = 0;
	virtual void prepare(double xOff, double yOff, double zOff) = 0;
};