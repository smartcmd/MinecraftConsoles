#pragma once
#include "EntityRenderer.h"

class Textures;
class ItemInstance;
class Random;
class ItemEntity;

class ItemRenderer : public EntityRenderer
{
private:
//	TileRenderer *tileRenderer;	// 4J - removed - this is shadowing the tilerenderer from entityrenderer
    Random *random;
	bool m_bItemFrame;
public:
	bool setColor;
	float blitOffset;

    ItemRenderer();
	virtual ~ItemRenderer();
    virtual void render(shared_ptr<Entity> _itemEntity, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	virtual ResourceLocation *getTextureLocation(int iconType);

private:
	virtual void renderItemBillboard(shared_ptr<ItemEntity> entity, Icon *icon, int count, float a, float red, float green, float blue);

public:
	// 4J - original 2 interface variants
	void renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y);
    void renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, int x, int y);
	// 4J - new interfaces added
	void renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, float x, float y, float fScale, float fAlpha);
	void renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, float x, float y, float fScaleX,float fScaleY, float fAlpha, bool useCompiled); // 4J Added useCompiled
	void renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, float x, float y, float fScale, float fAlpha, bool isFoil);
	void renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, float x, float y, float fScaleX, float fScaleY, float fAlpha, bool isFoil, bool isConstantBlended, bool useCompiled = true);	// 4J - added isConstantBlended and useCompiled

	// 4J Added
	virtual void SetItemFrame(bool bSet)	{m_bItemFrame=bSet;}

	static const int m_iPotionStrengthBarWidth[4];

private:
	void blitGlint(int id, float x, float y, float w, float h);		// 4J - changed x,y,w,h to floats

public:
    void renderGuiItemDecorations(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y, float fAlpha = 1.0f);
	void renderGuiItemDecorations(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y, const wstring &countText, float fAlpha = 1.0f);
private:
	void fillRect(Tesselator *t, int x, int y, int w, int h, int c);
public:
	void blit(float x, float y, int sx, int sy, float w, float h);		// 4J - changed x,y,w,h to floats
	void blit(float x, float y, Icon *tex, float w, float h);
};
