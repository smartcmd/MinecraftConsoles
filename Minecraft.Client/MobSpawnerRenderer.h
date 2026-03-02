#pragma once
#include "TileEntityRenderer.h"
using namespace std;

class MobSpawnerRenderer : public TileEntityRenderer
{
private:
	unordered_map<wstring, shared_ptr<Entity> > models;
public:
	virtual void render(shared_ptr<TileEntity> _spawner, double x, double y, double z, float a, bool setColor, float alpha=1.0f, bool useCompiled = true); // 4J added setColor param
};
