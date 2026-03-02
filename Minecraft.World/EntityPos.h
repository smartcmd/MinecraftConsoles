#pragma once

class EntityPos
{
public:
	double x, y, z;
	float yRot, xRot;
	bool rot;
	bool move;

	EntityPos(double x, double y, double z, float yRot, float xRot);
	EntityPos(double x, double y, double z);
	EntityPos(float yRot, float xRot);
	EntityPos *lerp(shared_ptr<Entity> e, float f);
};