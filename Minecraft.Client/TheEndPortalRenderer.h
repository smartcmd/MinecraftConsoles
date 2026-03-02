#pragma once

#include "TileEntityRenderer.h"

class TheEndPortalRenderer : public TileEntityRenderer
{
public:
	virtual void render(shared_ptr<TileEntity> _table, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true);

	FloatBuffer *lb;

	TheEndPortalRenderer();

private:
	FloatBuffer *getBuffer(float a, float b, float c, float d);
};
