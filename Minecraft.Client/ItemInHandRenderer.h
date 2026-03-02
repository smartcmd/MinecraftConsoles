#pragma once

class Minecraft;
class ItemInstance;
class Minimap;
class LivingEntity;
class TileRenderer;
class Tesselator;

class ItemInHandRenderer
{
public:
	// 4J - made these public
	static ResourceLocation ENCHANT_GLINT_LOCATION;
    static ResourceLocation MAP_BACKGROUND_LOCATION;
    static ResourceLocation UNDERWATER_LOCATION;

private:
	Minecraft *minecraft;
    shared_ptr<ItemInstance> selectedItem;
    float height;
    float oHeight;
    TileRenderer *tileRenderer;
	static int listItem, listGlint, listTerrain;

public:
	// 4J Stu - Made public so we can use it from ItemFramRenderer
    Minimap *minimap;

public:
	ItemInHandRenderer(Minecraft *mc, bool optimisedMinimap = true); // 4J Added optimisedMinimap param
    void renderItem(shared_ptr<LivingEntity> mob, shared_ptr<ItemInstance> item, int layer, bool setColor = true);	// 4J added setColor parameter
	static void renderItem3D(Tesselator *t, float u0, float v0, float u1, float v1, int width, int height, float depth, bool isGlint, bool isTerrain);	// 4J added isGlint and isTerrain parameter
public:
    void render(float a);
    void renderScreenEffect(float a);
private:
	void renderTex(float a, Icon *slot);
    void renderWater(float a);
    void renderFire(float a);
    int lastSlot;
public:
	void tick();
    void itemPlaced();
    void itemUsed();
};
