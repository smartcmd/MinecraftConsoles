#pragma once
#include "..\Minecraft.World\ArrayWithLength.h"
#include "Vertex.h"
#include "Polygon.h"

class Model;

class Cube
{

private:
	VertexArray vertices;
    PolygonArray polygons;
  
public:

	const float x0, y0, z0, x1, y1, z1;
	wstring id;

public:
	Cube(ModelPart *modelPart, int xTexOffs, int yTexOffs, float x0, float y0, float z0, int w, int h, int d, float g, int faceMask = 63, bool bFlipPoly3UVs = false);		// 4J - added faceMask

private:
	VertexArray VertexArray4(Vertex *v0, Vertex *v1, Vertex *v2, Vertex *v3);	// 4J added

public:
	void render(Tesselator *t,float scale);
	Cube *setId(const wstring &id); 
};
