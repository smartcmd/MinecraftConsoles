#pragma once

#include "Model.h"

class SilverfishModel : public Model
{

private:
	static const int BODY_COUNT = 7;

private:
	ModelPartArray bodyParts;
	ModelPartArray bodyLayers;
	float zPlacement[BODY_COUNT];

	static const int BODY_SIZES[BODY_COUNT][3];

	static const int BODY_TEXS[BODY_COUNT][2];

public:
	SilverfishModel();

	int modelVersion();
	void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
	virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, unsigned int uiBitmaskOverrideAnim=0);
};