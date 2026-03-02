#pragma once
#include "Textures.h"
#include "..\Minecraft.World\JavaIntHash.h"
class Mob;
class Level;
class TileEntityRenderer;
class TileEntity;

class TileEntityRenderDispatcher
{
public:
	static void staticCtor();	 // 4J added

private:
	typedef unordered_map<eINSTANCEOF, TileEntityRenderer *, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq> classToTileRendererMap;
	classToTileRendererMap renderers;

public:
	static TileEntityRenderDispatcher *instance;
private:
	Font *font;

public:
	static double xOff, yOff, zOff;

    Textures *textures;
    Level *level;
    shared_ptr<Mob> cameraEntity;
    float playerRotY;
    float playerRotX;
    double xPlayer, yPlayer, zPlayer;

private:
	TileEntityRenderDispatcher();

public:
	TileEntityRenderer *getRenderer(eINSTANCEOF e);
    bool hasRenderer(shared_ptr<TileEntity> e);
    TileEntityRenderer * getRenderer(shared_ptr<TileEntity> e);
	void prepare(Level *level, Textures *textures, Font *font, shared_ptr<Mob> player, float a);
	void render(shared_ptr<TileEntity> e, float a, bool setColor = true);
	void render(shared_ptr<TileEntity> entity, double x, double y, double z, float a, bool setColor = true, float alpha=1.0f, bool useCompiled = true); // 4J Added useCompiled
	void setLevel(Level *level);
	double distanceToSqr(double x, double y, double z);
	Font *getFont();
};
