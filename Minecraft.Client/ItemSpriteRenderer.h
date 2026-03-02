#pragma once
#include "EntityRenderer.h"

class Item;

class ItemSpriteRenderer : public EntityRenderer
{
private:
	Item *sourceItem;
	int sourceItemAuxValue;
public:
	ItemSpriteRenderer(Item *sourceItem, int sourceItemAuxValue = 0);
	//ItemSpriteRenderer(Item *icon);
    virtual void render(shared_ptr<Entity> e, double x, double y, double z, float rot, float a);

private:
	void renderIcon(Tesselator *t, Icon *icon);
};