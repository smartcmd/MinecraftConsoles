#pragma once

#include "TileEntityRenderer.h"

class SkeletonHeadModel;

class SkullTileRenderer : public TileEntityRenderer
{
public:
	static SkullTileRenderer *instance;

private:
	// note: this head fits most mobs, just change texture
	SkeletonHeadModel *skeletonModel;
	SkeletonHeadModel *zombieModel;

public:
	SkullTileRenderer();
	~SkullTileRenderer();

	void render(shared_ptr<TileEntity> skull, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled = true);
	void init(TileEntityRenderDispatcher *tileEntityRenderDispatcher);
	void renderSkull(float x, float y, float z, int face, float rot, int type, const wstring &extra);
};
