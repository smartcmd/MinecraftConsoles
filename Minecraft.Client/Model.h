#pragma once
using namespace std;
#include "..\Minecraft.World\Random.h"
#include "..\Minecraft.Client\SkinBox.h"
class Mob;
class ModelPart;
class TexOffs;


class  Model
{
public:
	float attackTime;
    bool riding;
	vector<ModelPart *> cubes;
	bool young;
	unordered_map<wstring, TexOffs * > mappedTexOffs;
	int texWidth;
	int texHeight;

	Model();		// 4J added
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled) {}
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0) {}
	virtual void prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a) {}
	virtual ModelPart *getRandomCube(Random random) {return cubes.at(random.nextInt((int)cubes.size()));}
	virtual ModelPart * AddOrRetrievePart(SKIN_BOX *pBox) { return NULL;}

	void setMapTex(wstring id, int x, int y);
	TexOffs *getMapTex(wstring id);

protected:
	float yHeadOffs;
	float zHeadOffs;
};
