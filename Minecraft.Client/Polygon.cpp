#include "stdafx.h"
#include "Polygon.h"

// 4J added for common init code
void _Polygon::_init(VertexArray vertices)
{
    vertexCount = 0;
    _flipNormal = false;

    this->vertices = vertices;
    vertexCount = vertices.length;
}

_Polygon::_Polygon(VertexArray vertices)
{
	_init(vertices);
}

_Polygon::_Polygon(VertexArray vertices, int u0, int v0, int u1, int v1, float xTexSize, float yTexSize)
{
	_init(vertices);

	// 4J - added - don't assume that u1 > u0, v1 > v0
    float us = ( u1 > u0 ) ? ( 0.1f / xTexSize ) : ( -0.1f / xTexSize );
	float vs = ( v1 > v0 ) ? ( 0.1f / yTexSize ) : ( -0.1f / yTexSize );

    vertices[0] = vertices[0]->remap(u1 / xTexSize - us, v0 / yTexSize + vs);
    vertices[1] = vertices[1]->remap(u0 / xTexSize + us, v0 / yTexSize + vs);
    vertices[2] = vertices[2]->remap(u0 / xTexSize + us, v1 / yTexSize - vs);
    vertices[3] = vertices[3]->remap(u1 / xTexSize - us, v1 / yTexSize - vs);
}

_Polygon::_Polygon(VertexArray vertices, float u0, float v0, float u1, float v1)
{
    _init(vertices);

    vertices[0] = vertices[0]->remap(u1, v0);
    vertices[1] = vertices[1]->remap(u0, v0);
    vertices[2] = vertices[2]->remap(u0, v1);
    vertices[3] = vertices[3]->remap(u1, v1);
}

void _Polygon::mirror()
{
    VertexArray newVertices = VertexArray(vertices.length);
    for (unsigned int i = 0; i < vertices.length; i++)
        newVertices[i] = vertices[vertices.length - i - 1];
	delete [] vertices.data;
    vertices = newVertices;
}

void _Polygon::render(Tesselator *t, float scale)
{
    Vec3 *v0 = vertices[1]->pos->vectorTo(vertices[0]->pos);
    Vec3 *v1 = vertices[1]->pos->vectorTo(vertices[2]->pos);
    Vec3 *n = v1->cross(v0)->normalize();
        
    t->begin();
    if (_flipNormal)
	{
        t->normal(-static_cast<float>(n->x), -static_cast<float>(n->y), -static_cast<float>(n->z));
    }
	else
	{
        t->normal(static_cast<float>(n->x), static_cast<float>(n->y), static_cast<float>(n->z));
    }

    for (int i = 0; i < 4; i++)
	{
        Vertex *v = vertices[i];
        t->vertexUV(static_cast<float>(v->pos->x * scale), static_cast<float>(v->pos->y * scale), static_cast<float>(v->pos->z * scale), (float)( v->u), (float)( v->v));
    }
    t->end();
}

_Polygon *_Polygon::flipNormal()
{
    _flipNormal = true;
    return this;
}