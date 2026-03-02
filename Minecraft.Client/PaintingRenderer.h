#pragma once
#include "EntityRenderer.h"

class Painting;
class Random;

class PaintingRenderer : public EntityRenderer
{
private:
	Random *random;

public:
	PaintingRenderer();		// 4J -added
	virtual void render(shared_ptr<Entity> _painting, double x, double y, double z, float rot, float a);

private:
	void renderPainting(shared_ptr<Painting> painting, int w, int h, int uo, int vo);
    void setBrightness(shared_ptr<Painting> painting, float ss, float ya);
};
