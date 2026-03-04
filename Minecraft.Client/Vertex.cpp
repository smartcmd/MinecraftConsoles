#include "stdafx.h"
#include "Vertex.h"

Vertex::Vertex(float x, float y, float z, float u, float v)
{
    this->pos = Vec3::newPermanent(x,y,z);
    this->u = u;
    this->v = v;
}

Vertex *Vertex::remap(float u, float v)
{
	return new Vertex(this, u, v);
}

Vertex::Vertex(Vertex *vertex, float u, float v)
{
    this->pos = vertex->pos;
    this->u = u;
    this->v = v;
}

Vertex::Vertex(Vec3 *pos, float u, float v)
{
    this->pos = pos;
    this->u = u;
    this->v = v;
}