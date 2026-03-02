#pragma once
#include "Model.h"
#include "ModelPart.h"

class SkiModel : public Model 
{
public:
    ModelPartArray cubes;
    
private:
	bool leftSki;

public:
	SkiModel(); // 4J added
    SkiModel(bool leftSki);
	void _init(bool leftSki); // 4J added
	virtual void render(shared_ptr<Entity> entity, float time, float r, float bob, float yRot, float xRot, float scale, bool usecompiled);
    virtual void setupAnim(float time, float r, float bob, float yRot, float xRot, float scale, shared_ptr<Entity> entity);
};