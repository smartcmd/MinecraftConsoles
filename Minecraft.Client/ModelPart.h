#pragma once
#include "../Minecraft.World/ArrayWithLength.h"
#include "Vertex.h"
#include "Polygon.h"
#include "Model.h"
#include "../Minecraft.Client/SkinBox.h"

class Cube;

class ModelPart 
{
public:
    float xTexSize;
    float yTexSize;
	float x, y, z;
	float xRot, yRot, zRot;
	bool bMirror;
	bool visible;
	bool neverRender;
    vector <Cube *> cubes;
    vector <ModelPart *> children;
    static const float RAD;
	float translateX, translateY, translateZ;

private:
    wstring id;
	int xTexOffs, yTexOffs;
    boolean compiled;
    int list;
    Model *model;
 
public:
	void _init(); // 4J added
	ModelPart();
	ModelPart(Model *model, const wstring &id);
    ModelPart(Model *model);
    ModelPart(Model *model, int xTexOffs, int yTexOffs);

	// MGH - had to add these for PS3, as calling constructors from others was only introduced in c++11 - https://en.wikipedia.org/wiki/C++11#Object_construction_improvement
	void construct(Model *model, const wstring &id);
	void construct(Model *model);
	void construct(Model *model, int xTexOffs, int yTexOffs);

    void addChild(ModelPart *child);
	ModelPart * retrieveChild(SKIN_BOX *pBox);  
    ModelPart *mirror();
    ModelPart *texOffs(int xTexOffs, int yTexOffs);
    ModelPart *addBox(wstring id, float x0, float y0, float z0, int w, int h, int d);
	ModelPart *addBox(float x0, float y0, float z0, int w, int h, int d);
	ModelPart *addBoxWithMask(float x0, float y0, float z0, int w, int h, int d, int faceMask);		// 4J added
    void addBox(float x0, float y0, float z0, int w, int h, int d, float g);
	void addHumanoidBox(float x0, float y0, float z0, int w, int h, int d, float g); // 4J - to flip the poly 3 uvs so the skin maps correctly
    void addTexBox(float x0, float y0, float z0, int w, int h, int d, int tex);
    void setPos(float x, float y, float z);
    void render(float scale, bool usecompiled,bool bHideParentBodyPart=false);
    void renderRollable(float scale, bool usecompiled);
    void translateTo(float scale);
    ModelPart *setTexSize(int xs, int ys);
    void mimic(ModelPart *o);
	void compile(float scale);
	int getfU() {return xTexOffs;}
	int getfV() {return yTexOffs;}
 };
