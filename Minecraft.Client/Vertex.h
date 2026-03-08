#pragma once
#include "../Minecraft.World/Vec3.h"

class Vertex
{
public:
	Vec3 *pos;
	float u,v;
public:
	Vertex(float x, float y, float z, float u, float v);
    Vertex *remap(float u, float v);
    Vertex(Vertex *vertex, float u, float v);
    Vertex(Vec3 *pos, float u, float v);
};