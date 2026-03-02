#include "stdafx.h"
#include "ItemRenderer.h"
#include "TileRenderer.h"
#include "entityRenderDispatcher.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.alchemy.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "Options.h"

ItemRenderer::ItemRenderer() : EntityRenderer()
{
    random = new Random();
    setColor = true;
	blitOffset = 0;

    this->shadowRadius = 0.15f;
    this->shadowStrength = 0.75f;

	// 4J added
	m_bItemFrame= false;
}

ItemRenderer::~ItemRenderer()
{
	delete random;
}

void ItemRenderer::render(shared_ptr<Entity> _itemEntity, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<ItemEntity> itemEntity = dynamic_pointer_cast<ItemEntity>(_itemEntity);

    random->setSeed(187);
    shared_ptr<ItemInstance> item = itemEntity->getItem();

    glPushMatrix();
    float bob = Mth::sin((itemEntity->age + a) / 10.0f + itemEntity->bobOffs) * 0.1f + 0.1f;
    float spin = ((itemEntity->age + a) / 20.0f + itemEntity->bobOffs) * Mth::RADDEG;

    int count = 1;
    if (itemEntity->getItem()->count > 1) count = 2;
    if (itemEntity->getItem()->count > 5) count = 3;
    if (itemEntity->getItem()->count > 20) count = 4;

    glTranslatef((float) x, (float) y + bob, (float) z);
    glEnable(GL_RESCALE_NORMAL);

	Tile *tile = Tile::tiles[item->id];
	if (item->getIconType() == Icon::TYPE_TERRAIN && tile != NULL && TileRenderer::canRender(tile->getRenderShape()))
	{
        glRotatef(spin, 0, 1, 0);

		if (m_bItemFrame) 
		{
			glScalef(1.25f, 1.25f, 1.25f);
			glTranslatef(0, 0.05f, 0);
			glRotatef(-90, 0, 1, 0);
		}

        bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
        float s = 1 / 4.0f;
		int shape = tile->getRenderShape();
        if (shape == Tile::SHAPE_CROSS_TEXTURE || shape == Tile::SHAPE_STEM || shape == Tile::SHAPE_LEVER || shape == Tile::SHAPE_TORCH )
		{
            s = 0.5f;
        }

        glScalef(s, s, s);
        for (int i = 0; i < count; i++)
		{
            glPushMatrix();
            if (i > 0) 
			{
                float xo = (random->nextFloat() * 2 - 1) * 0.2f / s;
                float yo = (random->nextFloat() * 2 - 1) * 0.2f / s;
                float zo = (random->nextFloat() * 2 - 1) * 0.2f / s;
                glTranslatef(xo, yo, zo);
            }
			// 4J - change brought forward from 1.8.2
			float br = SharedConstants::TEXTURE_LIGHTING ? 1.0f : itemEntity->getBrightness(a);
            tileRenderer->renderTile(tile, item->getAuxValue(), br);
            glPopMatrix();
        }
	}
	else if (item->getItem()->hasMultipleSpriteLayers())
	{
		if (m_bItemFrame) 
		{
			glScalef(1 / 1.95f, 1 / 1.95f, 1 / 1.95f);
			glTranslatef(0, -0.05f, 0);
			glDisable(GL_LIGHTING);
		} 
		else 
		{
			glScalef(1 / 2.0f, 1 / 2.0f, 1 / 2.0f);
		}

		bindTexture(TN_GUI_ITEMS); // 4J was "/gui/items.png"

		for (int layer = 0; layer <= 1; layer++)
		{
			random->setSeed(187);
			Icon *icon = item->getItem()->getLayerIcon(item->getAuxValue(), layer);
			float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : itemEntity->getBrightness(a);
			if (setColor)
			{
				int col = Item::items[item->id]->getColor(item, layer);
				float red = ((col >> 16) & 0xff) / 255.0f;
				float g = ((col >> 8) & 0xff) / 255.0f;
				float b = ((col) & 0xff) / 255.0f;

				glColor4f(red * brightness, g * brightness, b * brightness, 1);
				renderItemBillboard(itemEntity, icon, count, a, red * brightness, g * brightness, b * brightness);
			}
			else
			{
				renderItemBillboard(itemEntity, icon, count, a, 1, 1, 1);
			}
		}
	}
	else
	{
		if (m_bItemFrame) 
		{
			glScalef(1 / 1.95f, 1 / 1.95f, 1 / 1.95f);
			glTranslatef(0, -0.05f, 0);
			glDisable(GL_LIGHTING);
		} 
		else 
		{
			glScalef(1 / 2.0f, 1 / 2.0f, 1 / 2.0f);
		}
		
		// 4J Stu - For rendering the static compass, we give it a non-zero aux value
		if(item->id == Item::compass_Id) item->setAuxValue(255);
		Icon *icon = item->getIcon();
		if(item->id == Item::compass_Id) item->setAuxValue(0);
        if (item->getIconType() == Icon::TYPE_TERRAIN)
		{
            bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
        }
		else
		{
            bindTexture(TN_GUI_ITEMS);	// 4J was L"/gui/items.png"
        }
		if (setColor)
		{
			int col = Item::items[item->id]->getColor(item,0);
			float red = ((col >> 16) & 0xff) / 255.0f;
			float g = ((col >> 8) & 0xff) / 255.0f;
			float b = ((col) & 0xff) / 255.0f;
			float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : itemEntity->getBrightness(a);

			glColor4f(red * brightness, g * brightness, b * brightness, 1);
			renderItemBillboard(itemEntity, icon, count, a, red * brightness, g * brightness, b * brightness);
		}
		else
		{
			renderItemBillboard(itemEntity, icon, count, a, 1, 1, 1);
		}

	}
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if( m_bItemFrame )
	{
		glEnable(GL_LIGHTING);
	}
}

void ItemRenderer::renderItemBillboard(shared_ptr<ItemEntity> entity, Icon *icon, int count, float a, float red, float green, float blue)
{
    Tesselator *t = Tesselator::getInstance();

	if (icon == NULL) icon = entityRenderDispatcher->textures->getMissingIcon(entity->getItem()->getIconType());
    float u0 = icon->getU0();
    float u1 = icon->getU1();
    float v0 = icon->getV0();
    float v1 = icon->getV1();

    float r = 1.0f;
    float xo = 0.5f;
    float yo = 0.25f;

	if (entityRenderDispatcher->options->fancyGraphics)
	{
		// Consider forcing the mipmap LOD level to use, if this is to be rendered from a larger than standard source texture. 
		int iconWidth = icon->getWidth();
		int LOD = -1;	// Default to not doing anything special with LOD forcing
		if( iconWidth == 32 )
		{
			LOD = 1;	// Force LOD level 1 to achieve texture reads from 256x256 map
		}
		else if( iconWidth == 64 )
		{
			LOD = 2;	// Force LOD level 2 to achieve texture reads from 256x256 map
		}
		RenderManager.StateSetForceLOD(LOD);

		glPushMatrix();
		if (m_bItemFrame)
		{
			glRotatef(180, 0, 1, 0);
		}
		else
		{
			glRotatef(((entity->age + a) / 20.0f + entity->bobOffs) * Mth::RADDEG, 0, 1, 0);
		}

		float width = 1 / 16.0f;
		float margin = 0.35f / 16.0f;
		shared_ptr<ItemInstance> item = entity->getItem();
		int items = item->count;

		if (items < 2)
		{
			count = 1;
		}
		else if (items < 16)
		{
			count = 2;
		}
		else if (items < 32)
		{
			count = 3;
		}
		else
		{
			count = 4;
		}

		glTranslatef(-xo, -yo, -((width + margin) * count / 2));

		for (int i = 0; i < count; i++)
		{
			glTranslatef(0, 0, width + margin);
			if (item->getIconType() == Icon::TYPE_TERRAIN && Tile::tiles[item->id] != NULL)
			{
				bindTexture(TN_TERRAIN); // Was L"/terrain.png");
			}
			else
			{
				bindTexture(TN_GUI_ITEMS); //L"/gui/items.png");
			}
			glColor4f(red, green, blue, 1);
			// 4J Stu - u coords were swapped in Java
			//ItemInHandRenderer::renderItem3D(t, u1, v0, u0, v1, icon->getSourceWidth(), icon->getSourceHeight(), width, false);
			ItemInHandRenderer::renderItem3D(t, u0, v0, u1, v1, icon->getSourceWidth(), icon->getSourceHeight(), width, false);

			if (item != NULL && item->isFoil())
			{
				glDepthFunc(GL_EQUAL);
				glDisable(GL_LIGHTING);
				entityRenderDispatcher->textures->bindTexture(TN__BLUR__MISC_GLINT); // was L"%blur%/misc/glint.png");
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_COLOR, GL_ONE);
				float br = 0.76f;
				glColor4f(0.5f * br, 0.25f * br, 0.8f * br, 1);
				glMatrixMode(GL_TEXTURE);
				glPushMatrix();
				float ss = 1 / 8.0f;
				glScalef(ss, ss, ss);
				float sx = Minecraft::currentTimeMillis() % (3000) / (3000.0f) * 8;
				glTranslatef(sx, 0, 0);
				glRotatef(-50, 0, 0, 1);

				ItemInHandRenderer::renderItem3D(t, 0, 0, 1, 1, 255, 255, width, true);
				glPopMatrix();
				glPushMatrix();
				glScalef(ss, ss, ss);
				sx = Minecraft::currentTimeMillis() % (3000 + 1873) / (3000 + 1873.0f) * 8;
				glTranslatef(-sx, 0, 0);
				glRotatef(10, 0, 0, 1);
				ItemInHandRenderer::renderItem3D(t, 0, 0, 1, 1, 255, 255, width, true);
				glPopMatrix();
				glMatrixMode(GL_MODELVIEW);
				glDisable(GL_BLEND);
				glEnable(GL_LIGHTING);
				glDepthFunc(GL_LEQUAL);
			}
		}

		glPopMatrix();

		RenderManager.StateSetForceLOD(-1);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			glPushMatrix();
			if (i > 0)
			{
				float _xo = (random->nextFloat() * 2 - 1) * 0.3f;
				float _yo = (random->nextFloat() * 2 - 1) * 0.3f;
				float _zo = (random->nextFloat() * 2 - 1) * 0.3f;
				glTranslatef(_xo, _yo, _zo);
			}
			if (!m_bItemFrame) glRotatef(180 - entityRenderDispatcher->playerRotY, 0, 1, 0);
			glColor4f(red, green, blue, 1);
			t->begin();
			t->normal(0, 1, 0);
			t->vertexUV((float)(0 - xo), (float)( 0 - yo), (float)( 0), (float)( u0), (float)( v1));
			t->vertexUV((float)(r - xo), (float)( 0 - yo), (float)( 0), (float)( u1), (float)( v1));
			t->vertexUV((float)(r - xo), (float)( 1 - yo), (float)( 0), (float)( u1), (float)( v0));
			t->vertexUV((float)(0 - xo), (float)( 1 - yo), (float)( 0), (float)( u0), (float)( v0));
			t->end();

			glPopMatrix();
		}
}
}

void ItemRenderer::renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, float x, float y, float fScale, float fAlpha)
{
	renderGuiItem(font,textures,item,x,y,fScale,fScale,fAlpha, true);
}

#ifdef _XBOX
extern IDirect3DDevice9 *g_pD3DDevice;
#endif

// 4J - this used to take x and y as ints, and no scale and alpha - but this interface is now implemented as a wrapper round this more fully featured one
void ItemRenderer::renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, float x, float y, float fScaleX,float fScaleY, float fAlpha, bool useCompiled)
{
	int itemId = item->id;
	int itemAuxValue = item->getAuxValue();
	Icon *itemIcon = item->getIcon();

    if (item->getIconType() == Icon::TYPE_TERRAIN && TileRenderer::canRender(Tile::tiles[itemId]->getRenderShape()))
	{
		PIXBeginNamedEvent(0,"3D gui item render %d\n",itemId);
		MemSect(31);
        textures->bindTexture(TN_TERRAIN);//L"/terrain.png"));
		MemSect(0);

        Tile *tile = Tile::tiles[itemId];
        glPushMatrix();
		// 4J - original code left here for reference
#if 0
		glTranslatef((float)(x), (float)(y), 0.0f);
		glScalef(fScale, fScale, fScale);
		glTranslatef(-2.0f,3.0f, -3.0f + blitOffset);
		glScalef(10.0f, 10.0f, 10.0f);
        glTranslatef(1.0f, 0.5f, 8.0f);
        glScalef(1.0f, 1.0f, -1.0f);
        glRotatef(180.0f + 30.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
#else
		glTranslatef(x, y, 0.0f);						// Translate to screen coords
		glScalef(16.0f*fScaleX, 16.0f*fScaleY, 1.0f);		// Scale to 0 to 16*scale range
		glTranslatef(0.5f,0.5f,0.0f);					// Translate to 0 to 1 range
		glScalef(0.55f,0.55f, -1.0f);					// Scale to occupy full -0.5 to 0.5 bounding region (just touching top & bottom)
														// 0.55 comes from 1/(1+sqrt(2)/sqrt(3)) which is determined by the angles that the cube is rotated in an orthographic projection
        glRotatef(180.0f + 30.0f, 1.0f, 0.0f, 0.0f);	// Rotate round x axis (centre at origin)
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);				// Rotate round y axis (centre at origin)
#endif
		// 4J-PB - pass the alpha value in - the grass block render has the top surface coloured differently to the rest of the block
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        tileRenderer->renderTile(tile, itemAuxValue, 1, fAlpha, useCompiled);

        glPopMatrix();
		PIXEndNamedEvent();
	}
	else if (Item::items[itemId]->hasMultipleSpriteLayers())
	{
		PIXBeginNamedEvent(0,"Potion gui item render %d\n",itemIcon);
		// special double-layered
		glDisable(GL_LIGHTING);
		textures->bindTexture(TN_GUI_ITEMS); // "/gui/items.png"

		for (int layer = 0; layer <= 1; layer++)
		{
			Icon *fillingIcon = Item::items[itemId]->getLayerIcon(itemAuxValue, layer);

			int col = Item::items[itemId]->getColor(item, layer);
			float r = ((col >> 16) & 0xff) / 255.0f;
			float g = ((col >> 8) & 0xff) / 255.0f;
			float b = ((col) & 0xff) / 255.0f;

			if (setColor) glColor4f(r, g, b, fAlpha);
			// scale the x and y by the scale factor
			if((fScaleX!=1.0f) ||(fScaleY!=1.0f))
			{
				blit(x, y, fillingIcon, 16 * fScaleX, 16 * fScaleY);
			}
			else
			{
				blit((int)x, (int)y, fillingIcon, 16, 16);
			}
		}
		glEnable(GL_LIGHTING);
		PIXEndNamedEvent();
    }
	else
	{
		PIXBeginNamedEvent(0,"2D gui item render %d\n",itemIcon);
        glDisable(GL_LIGHTING);
		MemSect(31);
		if (item->getIconType() == Icon::TYPE_TERRAIN)
		{
            textures->bindTexture(TN_TERRAIN);//L"/terrain.png"));
        }
		else
		{
            textures->bindTexture(TN_GUI_ITEMS);//L"/gui/items.png"));
#ifdef _XBOX
			// 4J - make sure we've got linear sampling on minification here as non-mipmapped things like this currently
			// default to having point sampling, which makes very small icons render rather badly
			g_pD3DDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
#endif

		}
		MemSect(0);

		if (itemIcon == NULL)
		{
			itemIcon = textures->getMissingIcon(item->getIconType());
		}

        int col = Item::items[itemId]->getColor(item,0);
        float r = ((col >> 16) & 0xff) / 255.0f;
        float g = ((col >> 8) & 0xff) / 255.0f;
        float b = ((col) & 0xff) / 255.0f;

        if (setColor) glColor4f(r, g, b, fAlpha);

		// scale the x and y by the scale factor
		if((fScaleX!=1.0f) ||(fScaleY!=1.0f))
		{
			blit(x, y, itemIcon, 16 * fScaleX, 16 * fScaleY);
		}
		else
		{
			blit((int)x, (int)y, itemIcon, 16, 16);
		}
        glEnable(GL_LIGHTING);
		PIXEndNamedEvent();

    }
    glEnable(GL_CULL_FACE);

}

// 4J - original interface, now just a wrapper for preceding overload
void ItemRenderer::renderGuiItem(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y)
{
	renderGuiItem(font, textures, item, (float)x, (float)y, 1.0f, 1.0f );
}

// 4J - this used to take x and y as ints, and no scale, alpha or foil - but this interface is now implemented as a wrapper round this more fully featured one
void ItemRenderer::renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, float x, float y,float fScale,float fAlpha, bool isFoil)
{
	if(item==NULL) return;
	renderAndDecorateItem(font, textures, item, x, y,fScale, fScale, fAlpha, isFoil, true);
}

// 4J - added isConstantBlended and blendFactor parameters. This is true if the gui item is being rendered from a context where it already has blending enabled to do general interface fading
// (ie from the gui rather than xui). In this case we dno't want to enable/disable blending, and do need to restore the blend state when we are done.
void ItemRenderer::renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, float x, float y,float fScaleX, float fScaleY,float fAlpha, bool isFoil, bool isConstantBlended, bool useCompiled)	
{
    if (item == NULL)
	{
        return;
    }

    renderGuiItem(font, textures, item, x, y,fScaleX,fScaleY,fAlpha, useCompiled);
	
	if (isFoil || item->isFoil())
	{
		glDepthFunc(GL_GREATER);
		glDisable(GL_LIGHTING);
		glDepthMask(false);
		textures->bindTexture(TN__BLUR__MISC_GLINT); // 4J was "%blur%/misc/glint.png"
		blitOffset -= 50;
		if( !isConstantBlended )	glEnable(GL_BLEND);
		
		glBlendFunc(GL_DST_COLOR, GL_ONE);		// 4J - changed blend equation from GL_DST_COLOR, GL_DST_COLOR so we can fade this out

		float blendFactor = isConstantBlended ? Gui::currentGuiBlendFactor : 1.0f;

		glColor4f(0.5f * blendFactor, 0.25f * blendFactor, 0.8f * blendFactor, 1);		// 4J - scale back colourisation with blendFactor
		// scale the x and y by the scale factor
		if((fScaleX!=1.0f) ||(fScaleY!=1.0f))
		{
			// 4J Stu - Scales were multiples of 20, making 16 to not overlap in xui scenes
			blitGlint(x * 431278612 + y * 32178161, x - 2, y - 2, 16 * fScaleX, 16 * fScaleY);
		}
		else
		{
			blitGlint(x * 431278612 + y * 32178161, x - 2, y - 2, 20, 20);
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1);	// 4J added
		if( !isConstantBlended ) glDisable(GL_BLEND);

		glDepthMask(true);
		blitOffset += 50;
		glEnable(GL_LIGHTING);
		glDepthFunc(GL_LEQUAL);

		if( isConstantBlended ) glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	}
}

// 4J - original interface, now just a wrapper for preceding overload
void ItemRenderer::renderAndDecorateItem(Font *font, Textures *textures, const shared_ptr<ItemInstance> item, int x, int y)
{
	renderAndDecorateItem( font, textures, item, (float)x, (float)y, 1.0f, 1.0f, item->isFoil() );
}

// 4J - a few changes here to get x, y, w, h in as floats (for xui rendering accuracy), and to align
// final pixels to the final screen resolution
void ItemRenderer::blitGlint(int id, float x, float y, float w, float h)
{
	float us = 1.0f / 64.0f / 4;
	float vs = 1.0f / 64.0f / 4;

	// 4J - calculate what the pixel coordinates will be in final screen coordinates
	float sfx = (float)Minecraft::GetInstance()->width / (float)Minecraft::GetInstance()->width_phys;
	float sfy = (float)Minecraft::GetInstance()->height / (float)Minecraft::GetInstance()->height_phys;
	float xx0 = x * sfx;
	float xx1 = ( x + w ) * sfx;
	float yy0 = y * sfy;
	float yy1 = ( y + h ) * sfy;
	// Round to whole pixels - rounding inwards so that we don't overlap any surrounding graphics
	xx0 = ceilf(xx0);
	xx1 = floorf(xx1);
	yy0 = ceilf(yy0);
	yy1 = floorf(yy1);
	// Offset by half to get actual centre of pixel - again moving inwards to avoid overlap with surrounding graphics
	xx0 += 0.5f;
	xx1 -= 0.5f;
	yy0 += 0.5f;
	yy1 -= 0.5f;
	// Convert back to game coordinate space
	float xx0f = xx0 / sfx;
	float xx1f = xx1 / sfx;
	float yy0f = yy0 / sfy;
	float yy1f = yy1 / sfy;

	for (int i = 0; i < 2; i++)
	{
		if (i == 0) glBlendFunc(GL_SRC_COLOR, GL_ONE);
		if (i == 1) glBlendFunc(GL_SRC_COLOR, GL_ONE);
		float sx = Minecraft::currentTimeMillis() % (3000 + i * 1873) / (3000.0f + i * 1873) * 256;
		float sy = 0;
		Tesselator *t = Tesselator::getInstance();
		float vv = 4;
		if (i == 1) vv = -1;
		t->begin();
		t->vertexUV(xx0f, yy1f, blitOffset, (sx + h * vv) * us, (sy + h) * vs);
		t->vertexUV(xx1f, yy1f, blitOffset, (sx + w + h * vv) * us, (sy + h) * vs);
		t->vertexUV(xx1f, yy0f, blitOffset, (sx + w) * us, (sy + 0) * vs);
		t->vertexUV(xx0f, yy0f, blitOffset, (sx + 0) * us, (sy + 0) * vs);
		t->end();
	}
}

void ItemRenderer::renderGuiItemDecorations(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y, float fAlpha)
{
	renderGuiItemDecorations(font, textures, item, x, y, L"", fAlpha);
}

void ItemRenderer::renderGuiItemDecorations(Font *font, Textures *textures, shared_ptr<ItemInstance> item, int x, int y, const wstring &countText, float fAlpha)
{
    if (item == NULL)
	{
        return;
    }
	
	glEnable(GL_BLEND);
	RenderManager.StateSetBlendFactor(0xffffff |(((unsigned int)(fAlpha * 0xff))<<24));
	glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    if (item->count > 1 || !countText.empty() || item->GetForceNumberDisplay())
	{
		MemSect(31);
        wstring amount = countText;
		if(amount.empty())
		{
			int count = item->count;
			if(count > 64)
			{
				amount = _toString<int>(64) + L"+";
			}
			else
			{
				amount = _toString<int>(item->count);
			}
		}
		MemSect(0);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        font->drawShadow(amount, x + 19 - 2 - font->width(amount), y + 6 + 3, 0xffffff);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

    if (item->isDamaged())
	{
        int p = (int) Math::round(13.0 - (double) item->getDamageValue() * 13.0 / (double) item->getMaxDamage());
        int cc = (int) Math::round(255.0 - (double) item->getDamageValue() * 255.0 / (double) item->getMaxDamage());
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);

        Tesselator *t = Tesselator::getInstance();

        int ca = (255 - cc) << 16 | (cc) << 8;
        int cb = ((255 - cc) / 4) << 16 | (255 / 4) << 8;
        fillRect(t, x + 2, y + 13, 13, 2, 0x000000);
        fillRect(t, x + 2, y + 13, 12, 1, cb);
        fillRect(t, x + 2, y + 13, p, 1, ca);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glColor4f(1, 1, 1, 1);
    }
	else if(item->hasPotionStrengthBar())
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_TEXTURE_2D);

		Tesselator *t = Tesselator::getInstance();

		fillRect(t, x + 3, y + 13, 11, 2, 0x000000);
		//fillRect(t, x + 2, y + 13, 13, 1, 0x1dabc0);
		fillRect(t, x + 3, y + 13, m_iPotionStrengthBarWidth[item->GetPotionStrength()], 2, 0x00e1eb);
		fillRect(t, x + 2 + 3, y + 13, 1, 2, 0x000000);
		fillRect(t, x + 2 + 3+3, y + 13, 1, 2, 0x000000);
		fillRect(t, x + 2 + 3+3+3, y + 13, 1, 2, 0x000000);


		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glColor4f(1, 1, 1, 1);
	}
	glDisable(GL_BLEND);
}

const int ItemRenderer::m_iPotionStrengthBarWidth[]=
{
	3,6,9,11
};

void ItemRenderer::fillRect(Tesselator *t, int x, int y, int w, int h, int c)
{
	t->begin();
	t->color(c);
	t->vertex((float)(x + 0), (float)( y + 0), (float)( 0));
	t->vertex((float)(x + 0), (float)( y + h), (float)( 0));
	t->vertex((float)(x + w), (float)( y + h), (float)( 0));
	t->vertex((float)(x + w), (float)( y + 0), (float)( 0));
	t->end();
}

// 4J - a few changes here to get x, y, w, h in as floats (for xui rendering accuracy), and to align
// final pixels to the final screen resolution
void ItemRenderer::blit(float x, float y, int sx, int sy, float w, float h)
{
	float us = 1 / 256.0f;
	float vs = 1 / 256.0f;
	Tesselator *t = Tesselator::getInstance();
	t->begin();

	// 4J - calculate what the pixel coordinates will be in final screen coordinates
	float sfx = (float)Minecraft::GetInstance()->width / (float)Minecraft::GetInstance()->width_phys;
	float sfy = (float)Minecraft::GetInstance()->height / (float)Minecraft::GetInstance()->height_phys;
	float xx0 = x * sfx;
	float xx1 = ( x + w ) * sfx;
	float yy0 = y * sfy;
	float yy1 = ( y + h ) * sfy;
	// Round to whole pixels - rounding inwards so that we don't overlap any surrounding graphics
	xx0 = ceilf(xx0);
	xx1 = floorf(xx1);
	yy0 = ceilf(yy0);
	yy1 = floorf(yy1);
	// Offset by half to get actual centre of pixel - again moving inwards to avoid overlap with surrounding graphics
	xx0 += 0.5f;
	xx1 -= 0.5f;
	yy0 += 0.5f;
	yy1 -= 0.5f;
	// Convert back to game coordinate space
	float xx0f = xx0 / sfx;
	float xx1f = xx1 / sfx;
	float yy0f = yy0 / sfy;
	float yy1f = yy1 / sfy;

	// 4J - subtracting 0.5f (actual screen pixels, so need to compensate for physical & game width) from each x & y coordinate to compensate for centre of pixels in directx vs openGL
	float f = ( 0.5f * (float)Minecraft::GetInstance()->width ) / (float)Minecraft::GetInstance()->width_phys;

	t->vertexUV(xx0f, yy1f, (float)( blitOffset), (float)( (sx + 0) * us), (float)( (sy + 16) * vs));
	t->vertexUV(xx1f, yy1f, (float)( blitOffset), (float)( (sx + 16) * us), (float)( (sy + 16) * vs));
	t->vertexUV(xx1f, yy0f, (float)( blitOffset), (float)( (sx + 16) * us), (float)( (sy + 0) * vs));
	t->vertexUV(xx0f, yy0f, (float)( blitOffset), (float)( (sx + 0) * us), (float)( (sy + 0) * vs));
	t->end();
}

void ItemRenderer::blit(float x, float y, Icon *tex, float w, float h)
{
	Tesselator *t = Tesselator::getInstance();
	t->begin();

	// 4J - calculate what the pixel coordinates will be in final screen coordinates
	float sfx = (float)Minecraft::GetInstance()->width / (float)Minecraft::GetInstance()->width_phys;
	float sfy = (float)Minecraft::GetInstance()->height / (float)Minecraft::GetInstance()->height_phys;
	float xx0 = x * sfx;
	float xx1 = ( x + w ) * sfx;
	float yy0 = y * sfy;
	float yy1 = ( y + h ) * sfy;
	// Round to whole pixels - rounding inwards so that we don't overlap any surrounding graphics
	xx0 = ceilf(xx0);
	xx1 = floorf(xx1);
	yy0 = ceilf(yy0);
	yy1 = floorf(yy1);
	// Offset by half to get actual centre of pixel - again moving inwards to avoid overlap with surrounding graphics
	xx0 += 0.5f;
	xx1 -= 0.5f;
	yy0 += 0.5f;
	yy1 -= 0.5f;
	// Convert back to game coordinate space
	float xx0f = xx0 / sfx;
	float xx1f = xx1 / sfx;
	float yy0f = yy0 / sfy;
	float yy1f = yy1 / sfy;

	// 4J - subtracting 0.5f (actual screen pixels, so need to compensate for physical & game width) from each x & y coordinate to compensate for centre of pixels in directx vs openGL
	float f = ( 0.5f * (float)Minecraft::GetInstance()->width ) / (float)Minecraft::GetInstance()->width_phys;

	t->vertexUV(xx0f, yy1f, blitOffset, tex->getU0(true), tex->getV1(true));
	t->vertexUV(xx1f, yy1f, blitOffset, tex->getU1(true), tex->getV1(true));
	t->vertexUV(xx1f, yy0f, blitOffset, tex->getU1(true), tex->getV0(true));
	t->vertexUV(xx0f, yy0f, blitOffset, tex->getU0(true), tex->getV0(true));
	t->end();
}
