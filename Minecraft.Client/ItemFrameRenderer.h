#pragma once
#include "EntityRenderer.h"

class ItemFrameRenderer : public EntityRenderer
{
private:
	static ResourceLocation MAP_BACKGROUND_LOCATION;
	Icon *backTexture;

public:
	void registerTerrainTextures(IconRegister *iconRegister);
	virtual void render(shared_ptr<Entity> _itemframe, double x, double y, double z, float rot, float a); 

private:
 	void drawFrame(shared_ptr<ItemFrame> itemFrame);
 	void drawItem(shared_ptr<ItemFrame> entity); 
};
