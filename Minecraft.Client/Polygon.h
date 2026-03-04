#pragma once
#include "Vertex.h"
#include "Tesselator.h"
#include "..\Minecraft.World\ArrayWithLength.h"

class _Polygon
{
public:
	VertexArray vertices;
    int vertexCount;
private:
	bool _flipNormal;

public:
	void _init(VertexArray vertices);	// 4J added for common init code
	_Polygon(VertexArray vertices);
    _Polygon(VertexArray vertices, int u0, int v0, int u1, int v1, float xTexSize, float yTexSize);
    _Polygon(VertexArray vertices, float u0, float v0, float u1, float v1);
    void mirror();
    void render(Tesselator *t, float scale);
    _Polygon *flipNormal();
};
