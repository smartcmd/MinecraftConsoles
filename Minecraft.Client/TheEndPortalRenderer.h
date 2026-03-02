#pragma once

#include "TileEntityRenderer.h"

class TheEndPortalRenderer : public TileEntityRenderer
{
private:
	static ResourceLocation END_SKY_LOCATION;
    static ResourceLocation END_PORTAL_LOCATION;
    static int RANDOM_SEED;
    static Random RANDOM;

public:
	virtual void render(shared_ptr<TileEntity> _table, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true);

	FloatBuffer *lb;

	TheEndPortalRenderer();

private:
	FloatBuffer *getBuffer(float a, float b, float c, float d);
};
