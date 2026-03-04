#pragma once
#include "EntityRenderer.h"
#include "..\Minecraft.World\Entity.h"
#include "..\Minecraft.World\JavaIntHash.h"
class font;
using namespace std;

class EntityRenderDispatcher
{
public:
	static void staticCtor();	// 4J added
private:
	typedef unordered_map<eINSTANCEOF, EntityRenderer *, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq> classToRendererMap;
	classToRendererMap renderers;
	// 4J - was:
//	Map<Class<? extends Entity>, EntityRenderer<? extends Entity>> renderers = new HashMap<Class<? extends Entity>, EntityRenderer<? extends Entity>>();

public:
	static EntityRenderDispatcher *instance;
private:
	Font *font;

public:
	static double xOff, yOff, zOff;

    Textures *textures;
    ItemInHandRenderer *itemInHandRenderer;
    Level *level;
    shared_ptr<LivingEntity> cameraEntity;
	shared_ptr<LivingEntity> crosshairPickMob;
    float playerRotY;
    float playerRotX;
    Options *options;
	bool isGuiRender;		// 4J added

    double xPlayer, yPlayer, zPlayer;

private:
	EntityRenderDispatcher();

public:
	EntityRenderer *getRenderer(eINSTANCEOF e);
    EntityRenderer *getRenderer(shared_ptr<Entity> e);
    void prepare(Level *level, Textures *textures, Font *font, shared_ptr<LivingEntity> player, shared_ptr<LivingEntity> crosshairPickMob, Options *options, float a);
    void render(shared_ptr<Entity> entity, float a);
    void render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a, bool bItemFrame = false, bool bRenderPlayerShadow = true);
    void setLevel(Level *level);
    double distanceToSqr(double x, double y, double z);
    Font *getFont();
	void registerTerrainTextures(IconRegister *iconRegister);

private:
	void renderHitbox(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a);

};
