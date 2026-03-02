#pragma once
#include "Model.h"

class ModelHorse : public Model
{
private:
	ModelPart *Head;
	ModelPart *UMouth;
	ModelPart *LMouth;
	ModelPart *Ear1;
	ModelPart *Ear2;
	ModelPart *MuleEarL;
	ModelPart *MuleEarR;
	ModelPart *Neck;
	ModelPart *HeadSaddle;
	ModelPart *Mane;

	ModelPart *Body;
	ModelPart *TailA;
	ModelPart *TailB;
	ModelPart *TailC;

	ModelPart *Leg1A;
	ModelPart *Leg1B;
	ModelPart *Leg1C;

	ModelPart *Leg2A;
	ModelPart *Leg2B;
	ModelPart *Leg2C;

	ModelPart *Leg3A;
	ModelPart *Leg3B;
	ModelPart *Leg3C;

	ModelPart *Leg4A;
	ModelPart *Leg4B;
	ModelPart *Leg4C;

	ModelPart *Bag1;
	ModelPart *Bag2;

	ModelPart *Saddle;
	ModelPart *SaddleB;
	ModelPart *SaddleC;

	ModelPart *SaddleL;
	ModelPart *SaddleL2;

	ModelPart *SaddleR;
	ModelPart *SaddleR2;

	ModelPart *SaddleMouthL;
	ModelPart *SaddleMouthR;

	ModelPart *SaddleMouthLine;
	ModelPart *SaddleMouthLineR;

public:
	ModelHorse();
	void prepareMobModel(shared_ptr<LivingEntity> mob, float wp, float ws, float a);
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);

private:
	void setRotation(ModelPart *model, float x, float y, float z);
	float rotlerp(float from, float to, float a);
};