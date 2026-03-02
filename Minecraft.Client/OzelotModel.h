#pragma once

#include "Model.h"

class OzelotModel : public Model
{
private:
	ModelPart *backLegL, *backLegR;
	ModelPart *frontLegL, *frontLegR;
	ModelPart *tail1, *tail2, *head, *body;

	static const int SNEAK_STATE = 0;
	static const int WALK_STATE = 1;
	static const int SPRINT_STATE = 2;
	static const int SITTING_STATE = 3;

	int state;

	static const float xo;
	static const float yo;
	static const float zo;

	static const float headWalkY;
	static const float headWalkZ;
	static const float bodyWalkY;
	static const float bodyWalkZ;
	static const float tail1WalkY;
	static const float tail1WalkZ;
	static const float tail2WalkY;
	static const float tail2WalkZ;
	static const float backLegY;
	static const float backLegZ;
	static const float frontLegY;
	static const float frontLegZ ;

public:
	OzelotModel();

	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	void render(OzelotModel *model, float scale, bool usecompiled);
	void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
	void prepareMobModel(shared_ptr<Mob> mob, float time, float r, float a);
};