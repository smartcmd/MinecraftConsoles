#include "stdafx.h"
#include "ItemInHandRenderer.h"
#include "TileRenderer.h"
#include "Tesselator.h"
#include "Textures.h"
#include "TextureAtlas.h"
#include "EntityRenderer.h"
#include "PlayerRenderer.h"
#include "EntityRenderDispatcher.h"
#include "Lighting.h"
#include "MultiplayerLocalPlayer.h"
#include "Minimap.h"
#include "MultiPlayerLevel.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.h"

ResourceLocation ItemInHandRenderer::ENCHANT_GLINT_LOCATION = ResourceLocation(TN__BLUR__MISC_GLINT);
ResourceLocation ItemInHandRenderer::MAP_BACKGROUND_LOCATION = ResourceLocation(TN_MISC_MAPBG);
ResourceLocation ItemInHandRenderer::UNDERWATER_LOCATION = ResourceLocation(TN_MISC_WATER);

int ItemInHandRenderer::listItem = -1;
int ItemInHandRenderer::listTerrain = -1;
int ItemInHandRenderer::listGlint = -1;

ItemInHandRenderer::ItemInHandRenderer(Minecraft *minecraft, bool optimisedMinimap)
{
	// 4J - added
	height = 0;
	oHeight = 0;
	selectedItem = nullptr;
	tileRenderer = new TileRenderer();
	lastSlot = -1;

    this->minecraft = minecraft;
    minimap = new Minimap(minecraft->font, minecraft->options, minecraft->textures, optimisedMinimap);

	// 4J - replaced mesh that is used to render held items with individual cubes, so we can make it all join up properly without seams. This
	// has a lot more quads in it than the original, so is now precompiled with a UV matrix offset to put it in the final place for the
	// current icon. Compile it on demand for the first ItemInHandRenderer (list is static)
	if( listItem == -1 )
	{
		listItem = MemoryTracker::genLists(1);
		float dd = 1 / 16.0f;

		glNewList(listItem, GL_COMPILE);
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		for( int yp = 0; yp < 16; yp++ )
			for( int xp = 0; xp < 16; xp++ )
			{
				float u = (15-xp) / 256.0f;
				float v = (15-yp) / 256.0f;
				u += 0.5f / 256.0f;
				v += 0.5f / 256.0f;
				float x0 = xp / 16.0f;
				float x1 = x0 + 1.0f/16.0f;
				float y0 = yp / 16.0f;
				float y1 = y0 + 1.0f/16.0f;
				float z0 = 0.0f;
				float z1 = -dd;

				t->normal(0, 0, 1);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x1, y1, z0, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->normal(0, 0, -1);
				t->vertexUV(x0, y1, z1, u, v);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->vertexUV(x0, y0, z1, u, v);
				t->normal(-1, 0, 0);
				t->vertexUV(x0, y0, z1, u, v);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->vertexUV(x0, y1, z1, u, v);
				t->normal(1, 0, 0);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x1, y1, z0, u, v);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->normal(0, 1, 0);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x0, y0, z1, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->normal(0, -1, 0);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x0, y1, z1, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->vertexUV(x1, y1, z0, u, v);
			}					
		t->end();
		glEndList();
	}

	// Terrain texture is a different layout from the item texture
	if( listTerrain == -1 )
	{
		listTerrain = MemoryTracker::genLists(1);
		float dd = 1 / 16.0f;

		glNewList(listTerrain, GL_COMPILE);
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		for( int yp = 0; yp < 16; yp++ )
			for( int xp = 0; xp < 16; xp++ )
			{
				float u = (15-xp) / 256.0f;
				float v = (15-yp) / 512.0f;
				u += 0.5f / 256.0f;
				v += 0.5f / 512.0f;
				float x0 = xp / 16.0f;
				float x1 = x0 + 1.0f/16.0f;
				float y0 = yp / 16.0f;
				float y1 = y0 + 1.0f/16.0f;
				float z0 = 0.0f;
				float z1 = -dd;

				t->normal(0, 0, 1);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x1, y1, z0, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->normal(0, 0, -1);
				t->vertexUV(x0, y1, z1, u, v);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->vertexUV(x0, y0, z1, u, v);
				t->normal(-1, 0, 0);
				t->vertexUV(x0, y0, z1, u, v);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->vertexUV(x0, y1, z1, u, v);
				t->normal(1, 0, 0);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x1, y1, z0, u, v);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->normal(0, 1, 0);
				t->vertexUV(x1, y0, z0, u, v);
				t->vertexUV(x0, y0, z0, u, v);
				t->vertexUV(x0, y0, z1, u, v);
				t->vertexUV(x1, y0, z1, u, v);
				t->normal(0, -1, 0);
				t->vertexUV(x1, y1, z1, u, v);
				t->vertexUV(x0, y1, z1, u, v);
				t->vertexUV(x0, y1, z0, u, v);
				t->vertexUV(x1, y1, z0, u, v);
			}					
		t->end();
		glEndList();
	}

	// Also create special object for glint overlays - this is the same as the previous one, with a different UV scalings, and depth test set to equal
	if( listGlint == -1 )
	{
		listGlint = MemoryTracker::genLists(1);
		float dd = 1 / 16.0f;

		glNewList(listGlint, GL_COMPILE);
		glDepthFunc(GL_EQUAL);
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		for( int yp = 0; yp < 16; yp++ )
			for( int xp = 0; xp < 16; xp++ )
			{
				float u0 = (15-xp) / 16.0f;
				float v0 = (15-yp) / 16.0f;
				float u1 = u0 - (1.0f/16.0f);
				float v1 = v0 - (1.0f/16.0f);;

				float x0 = xp / 16.0f;
				float x1 = x0 + 1.0f/16.0f;
				float y0 = yp / 16.0f;
				float y1 = y0 + 1.0f/16.0f;
				float z0 = 0.0f;
				float z1 = -dd;

				float br = 0.76f;
				t->color(0.5f * br, 0.25f * br, 0.8f * br, 1.0f); // MGH - added the color here, as the glColour below wasn't making it through to render

				t->normal(0, 0, 1);
				t->vertexUV(x0, y0, z0, u0, v0);
				t->vertexUV(x1, y0, z0, u1, v0);
				t->vertexUV(x1, y1, z0, u1, v1);
				t->vertexUV(x0, y1, z0, u0, v1);
				t->normal(0, 0, -1);
				t->vertexUV(x0, y1, z1, u0, v1);
				t->vertexUV(x1, y1, z1, u1, v1);
				t->vertexUV(x1, y0, z1, u1, v0);
				t->vertexUV(x0, y0, z1, u0, v0);
				t->normal(-1, 0, 0);
				t->vertexUV(x0, y0, z1, u0, v0);
				t->vertexUV(x0, y0, z0, u0, v0);
				t->vertexUV(x0, y1, z0, u0, v1);
				t->vertexUV(x0, y1, z1, u0, v1);
				t->normal(1, 0, 0);
				t->vertexUV(x1, y1, z1, u1, v1);
				t->vertexUV(x1, y1, z0, u1, v1);
				t->vertexUV(x1, y0, z0, u1, v0);
				t->vertexUV(x1, y0, z1, u1, v0);
				t->normal(0, 1, 0);
				t->vertexUV(x1, y0, z0, u1, v0);
				t->vertexUV(x0, y0, z0, u0, v0);
				t->vertexUV(x0, y0, z1, u0, v0);
				t->vertexUV(x1, y0, z1, u1, v0);
				t->normal(0, -1, 0);
				t->vertexUV(x1, y1, z1, u1, v1);
				t->vertexUV(x0, y1, z1, u0, v1);
				t->vertexUV(x0, y1, z0, u0, v1);
				t->vertexUV(x1, y1, z0, u1, v1);
			}					
		t->end();
		glDepthFunc(GL_LEQUAL);
		glEndList();
	}

}

void ItemInHandRenderer::renderItem(shared_ptr<LivingEntity> mob, shared_ptr<ItemInstance> item, int layer, bool setColor/* = true*/)
{
	// 4J - code borrowed from render method below, although not factoring in brightness as that should already be being taken into account
	// by texture lighting. This is for colourising things held in 3rd person view.
    if ( (setColor) && (item != nullptr) )
	{
        int col = Item::items[item->id]->getColor(item, layer);
        float red = ((col >> 16) & 0xff) / 255.0f;
        float g = ((col >> 8) & 0xff) / 255.0f;
        float b = ((col) & 0xff) / 255.0f;

        glColor4f(red, g, b, 1);
    }

    glPushMatrix();
	Tile *tile = Tile::tiles[item->id];
    if (item->getIconType() == Icon::TYPE_TERRAIN && tile != nullptr && TileRenderer::canRender(tile->getRenderShape()))
	{
		MemSect(31);
        minecraft->textures->bindTexture(minecraft->textures->getTextureLocation(Icon::TYPE_TERRAIN));
		MemSect(0);
        tileRenderer->renderTile(Tile::tiles[item->id], item->getAuxValue(), SharedConstants::TEXTURE_LIGHTING ? 1.0f : mob->getBrightness(1));		// 4J - change brought forward from 1.8.2
    }
	else
	{
		MemSect(31);
		Icon *icon = mob->getItemInHandIcon(item, layer);
		if (icon == nullptr)
		{
			glPopMatrix();
			MemSect(0);
			return;
		}

		bool bIsTerrain = item->getIconType() == Icon::TYPE_TERRAIN;
		minecraft->textures->bindTexture(minecraft->textures->getTextureLocation(item->getIconType()));

		MemSect(0);
        Tesselator *t = Tesselator::getInstance();

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

		// 4J Original comment
		// Yes, these are backwards.
		// No, I don't know why.
		// 4J Stu - Make them the right way round...u coords were swapped
		float u0 = icon->getU0();
		float u1 = icon->getU1();
		float v0 = icon->getV0();
		float v1 = icon->getV1();

        float xo = 0.0f;
        float yo = 0.3f;

        glEnable(GL_RESCALE_NORMAL);
        glTranslatef(-xo, -yo, 0);
        float s = 1.5f;
        glScalef(s, s, s);

        glRotatef(50, 0, 1, 0);
        glRotatef(45 + 290, 0, 0, 1);
        glTranslatef(-15 / 16.0f, -1 / 16.0f, 0);
        float dd = 1 / 16.0f;

        renderItem3D(t, u0, v0, u1, v1, icon->getSourceWidth(), icon->getSourceHeight(), 1 / 16.0f, false, bIsTerrain);

        if (item != nullptr && item->isFoil() && layer == 0)
		{
            glDepthFunc(GL_EQUAL);
            glDisable(GL_LIGHTING);
            minecraft->textures->bindTexture(&ENCHANT_GLINT_LOCATION);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_COLOR, GL_ONE);
            float br = 0.76f;
			glColor4f(0.5f * br, 0.25f * br, 0.8f * br, 1);		// MGH - for some reason this colour isn't making it through to the render, so I've added to the tesselator for the glint geom above
             glMatrixMode(GL_TEXTURE);
             glPushMatrix();
             float ss = 1 / 8.0f;
             glScalef(ss, ss, ss);
             float sx = Minecraft::currentTimeMillis() % (3000) / (3000.0f) * 8;
             glTranslatef(sx, 0, 0);
             glRotatef(-50, 0, 0, 1);

            renderItem3D(t, 0, 0, 1, 1, 256, 256, 1 / 16.0f, true, bIsTerrain);
            glPopMatrix();
            glPushMatrix();
            glScalef(ss, ss, ss);
            sx = System::currentTimeMillis() % (3000 + 1873) / (3000 + 1873.0f) * 8;
            glTranslatef(-sx, 0, 0);
            glRotatef(10, 0, 0, 1);
            renderItem3D(t, 0, 0, 1, 1, 256, 256, 1 / 16.0f, true, bIsTerrain);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
            glDepthFunc(GL_LEQUAL);
        }

		RenderManager.StateSetForceLOD(-1);

        glDisable(GL_RESCALE_NORMAL);
    }
    glPopMatrix();
}

// 4J added useList parameter
void ItemInHandRenderer::renderItem3D(Tesselator *t, float u0, float v0, float u1, float v1, int width, int height, float depth, bool isGlint, bool isTerrain)
{
    float r = 1.0f;

	// 4J - replaced mesh that is used to render held items with individual cubes, so we can make it all join up properly without seams. This
	// has a lot more quads in it than the original, so is now precompiled with a UV matrix offset to put it in the final place for the
	// current icon

	if( isGlint )
	{
		glCallList(listGlint);
	}
	else
	{
		// 4J - replaced mesh that is used to render held items with individual cubes, so we can make it all join up properly without seams. This
		// has a lot more quads in it than the original, so is now precompiled with a UV matrix offset to put it in the final place for the
		// current icon

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glTranslatef(u0, v0, 0);
		glCallList(isTerrain? listTerrain : listItem);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
	}
	// 4J added since we are setting the colour to other values at the start of the function now
	glColor4f(1.0f,1.0f,1.0f,1.0f);
}

void ItemInHandRenderer::render(float a)
{
    float h = oHeight + (height - oHeight) * a;
    shared_ptr<Player> player = minecraft->player;

	if (player == nullptr)
	{
		return;
	}

	// 4J - added so we can adjust the position of the hands for horizontal & vertical split screens
	float fudgeX = 0.0f;
	float fudgeY = 0.0f;
	float fudgeZ = 0.0f;
	bool splitHoriz = false;
	shared_ptr<LocalPlayer> localPlayer = dynamic_pointer_cast<LocalPlayer>(player);
	if( localPlayer )
	{
		if( localPlayer->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM ||
			localPlayer->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_TOP )
		{
			fudgeY = 0.08f;
			splitHoriz = true;
		}
		else if( localPlayer->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT ||
				 localPlayer->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT )
		{
			fudgeX = -0.18f;
		}
	}

    float xr = player->xRotO + (player->xRot - player->xRotO) * a;

    glPushMatrix();
    glRotatef(xr, 1, 0, 0);
    glRotatef(player->yRotO + (player->yRot - player->yRotO) * a, 0, 1, 0);
    Lighting::turnOn();
    glPopMatrix();

    if (localPlayer)
	{
        float xrr = localPlayer->xBobO + (localPlayer->xBob - localPlayer->xBobO) * a;
        float yrr = localPlayer->yBobO + (localPlayer->yBob - localPlayer->yBobO) * a;
		// 4J - was using player->xRot and yRot directly here rather than interpolating between old & current with a
		float yr = player->yRotO + (player->yRot - player->yRotO) * a;
        glRotatef((xr - xrr) * 0.1f, 1, 0, 0);
        glRotatef((yr - yrr) * 0.1f, 0, 1, 0);
    }

    shared_ptr<ItemInstance> item = selectedItem;

    float br = minecraft->level->getBrightness(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
	// 4J - change brought forward from 1.8.2
    if (SharedConstants::TEXTURE_LIGHTING)
	{
        br = 1;
        int col = minecraft->level->getLightColor(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z), 0);
        int u = col % 65536;
        int v = col / 65536;
        glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
        glColor4f(1, 1, 1, 1);
    }
    if (item != nullptr)
	{
        int col = Item::items[item->id]->getColor(item,0);
        float red = ((col >> 16) & 0xff) / 255.0f;
        float g = ((col >> 8) & 0xff) / 255.0f;
        float b = ((col) & 0xff) / 255.0f;

        glColor4f(br * red, br * g, br * b, 1);
    }
	else
	{
	    glColor4f(br, br, br, 1);
	}

    if (item != nullptr && item->id == Item::map->id)
	{
        glPushMatrix();
        float d = 0.8f;

		// 4J - move the map away a bit if we're in horizontal split screen, so it doesn't clip out of the save zone
		if( splitHoriz )
		{
			glTranslatef(0.0f, 0.0f, -0.3f );
		}

        {
            float swing = player->getAttackAnim(a);

            float swing1 = Mth::sin(swing * PI);
            float swing2 = Mth::sin((sqrt(swing)) * PI);
            glTranslatef(-swing2 * 0.4f, Mth::sin(sqrt(swing) * PI * 2) * 0.2f, -swing1 * 0.2f);
        }

        float tilt = 1 - xr / 45.0f + 0.1f;
        if (tilt < 0) tilt = 0;
        if (tilt > 1) tilt = 1;
        tilt = -Mth::cos(tilt * PI) * 0.5f + 0.5f;

        glTranslatef(0.0f, 0.0f * d - (1 - h) * 1.2f - tilt * 0.5f + 0.04f, -0.9f * d);

        glRotatef(90, 0, 1, 0);
        glRotatef((tilt) * -85, 0, 0, 1);
        glEnable(GL_RESCALE_NORMAL);


        {
			// 4J-PB - if we've got a player texture, use that
			//glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadHttpTexture(minecraft->player->customTextureUrl, minecraft->player->getTexture()));
			glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadMemTexture(minecraft->player->customTextureUrl, minecraft->player->getTexture()));
			minecraft->textures->clearLastBoundId();
            for (int i = 0; i < 2; i++)
			{
                int flip = i * 2 - 1;
                glPushMatrix();

                glTranslatef(-0.0f, -0.6f, 1.1f * flip);
                glRotatef(static_cast<float>(-45 * flip), 1, 0, 0);
                glRotatef(-90, 0, 0, 1);
                glRotatef(59, 0, 0, 1);
                glRotatef(static_cast<float>(-65 * flip), 0, 1, 0);

                EntityRenderer *er = EntityRenderDispatcher::instance->getRenderer(minecraft->player);
                PlayerRenderer *playerRenderer = static_cast<PlayerRenderer *>(er);
                float ss = 1;
                glScalef(ss, ss, ss);

				// Can't turn off the hand if the player is holding a map
				shared_ptr<ItemInstance> itemInstance = player->inventory->getSelected();
				if ((itemInstance && (itemInstance->getItem()->id==Item::map_Id)) || app.GetGameSettings(localPlayer->GetXboxPad(),eGameSetting_DisplayHand)!=0 )
                {
					playerRenderer->renderHand();
				}
                glPopMatrix();
            }
        }

        {
            float swing = player->getAttackAnim(a);
            float swing3 = Mth::sin(swing * swing * PI);
            float swing2 = Mth::sin(sqrt(swing) * PI);
            glRotatef(-swing3 * 20, 0, 1, 0);
            glRotatef(-swing2 * 20, 0, 0, 1);
            glRotatef(-swing2 * 80, 1, 0, 0);
        }

        float ss = 0.38f;
        glScalef(ss, ss, ss);

        glRotatef(90, 0, 1, 0);
        glRotatef(180, 0, 0, 1);

        glTranslatef(-1, -1, +0);

        float s = 2 / 128.0f;
        glScalef(s, s, s);

		MemSect(31);
        minecraft->textures->bindTexture(&MAP_BACKGROUND_LOCATION);	// 4J was L"/misc/mapbg.png"
		MemSect(0);
        Tesselator *t = Tesselator::getInstance();

//        glNormal3f(0, 0, -1);	// 4J - changed to use tesselator
        t->begin();
		int vo = 7;
		t->normal(0,0,-1);
        t->vertexUV(static_cast<float>(0 - vo), static_cast<float>(128 + vo), static_cast<float>(0), static_cast<float>(0), static_cast<float>(1));
        t->vertexUV(static_cast<float>(128 + vo), static_cast<float>(128 + vo), static_cast<float>(0), static_cast<float>(1), static_cast<float>(1));
        t->vertexUV(static_cast<float>(128 + vo), static_cast<float>(0 - vo), static_cast<float>(0), static_cast<float>(1), static_cast<float>(0));
        t->vertexUV(static_cast<float>(0 - vo), static_cast<float>(0 - vo), static_cast<float>(0), static_cast<float>(0), static_cast<float>(0));
        t->end();

        shared_ptr<MapItemSavedData> data = Item::map->getSavedData(item, minecraft->level);
		PIXBeginNamedEvent(0,"Minimap render");
		if(data != nullptr) minimap->render(minecraft->player, minecraft->textures, data, minecraft->player->entityId);
		PIXEndNamedEvent();

        glPopMatrix();
    }
	else if (item != nullptr)
	{
        glPushMatrix();
        float d = 0.8f;

#if defined __ORBIS__ || defined __PS3__ || defined _WINDOWS64
		static const float swingPowFactor = 1.0f;
#else
		static const float swingPowFactor = 4.0f;		// 4J added, to slow the swing down when nearest the player for avoiding luminance flash issues
#endif
        if (player->getUseItemDuration() > 0)
		{
            UseAnim anim = item->getUseAnimation();
            if ( (anim == UseAnim_eat) || (anim == UseAnim_drink) )
			{
                float t = (player->getUseItemDuration() - a + 1);
                float swing = 1 - (t / item->getUseDuration());

                float is = 1 - swing;
                is = is * is * is;
                is = is * is * is;
                is = is * is * is;
                float iss = 1 - is;
                glTranslatef(0, Mth::abs(Mth::cos(t / 4 * PI) * 0.1f) * (swing > 0.2 ? 1 : 0), 0);
                glTranslatef(iss * 0.6f, -iss * 0.5f, 0);
                glRotatef(iss * 90, 0, 1, 0);
                glRotatef(iss * 10, 1, 0, 0);
                glRotatef(iss * 30, 0, 0, 1);
            }
        }
		else
		{
			float swing = powf(player->getAttackAnim(a),swingPowFactor);

			float swing1 = Mth::sin(swing * PI);
			float swing2 = Mth::sin((sqrt(swing)) * PI);
			glTranslatef(-swing2 * 0.4f, Mth::sin(sqrt(swing) * PI * 2) * 0.2f, -swing1 * 0.2f);

		}

        glTranslatef(0.7f * d, -0.65f * d - (1 - h) * 0.6f, -0.9f * d);
		glTranslatef(fudgeX, fudgeY, fudgeZ);	// 4J added

        glRotatef(45, 0, 1, 0);
        glEnable(GL_RESCALE_NORMAL);

		float swing = powf(player->getAttackAnim(a),swingPowFactor);
		float swing3 = Mth::sin(swing * swing * PI);
		float swing2 = Mth::sin(sqrt(swing) * PI);
		glRotatef(-swing3 * 20, 0, 1, 0);
		glRotatef(-swing2 * 20, 0, 0, 1);
		glRotatef(-swing2 * 80, 1, 0, 0);

        float ss = 0.4f;
        glScalef(ss, ss, ss);

        if (player->getUseItemDuration() > 0)
		{
            UseAnim anim = item->getUseAnimation();
            if (anim == UseAnim_block)
			{
                glTranslatef(-0.5f, 0.2f, 0.0f);
                glRotatef(30, 0, 1, 0);
                glRotatef(-80, 1, 0, 0);
                glRotatef(60, 0, 1, 0);
            }
			else if (anim == UseAnim_bow)
			{

                glRotatef(-18, 0, 0, 1);
                glRotatef(-12, 0, 1, 0);
                glRotatef(-8, 1, 0, 0);
                glTranslatef(-0.9f, 0.2f, 0.0f);
                float timeHeld = (item->getUseDuration() - (player->getUseItemDuration() - a + 1));
                float pow = timeHeld / static_cast<float>(BowItem::MAX_DRAW_DURATION);
                pow = ((pow * pow) + pow * 2) / 3;
                if (pow > 1) pow = 1;
                if (pow > 0.1f)
				{
                    glTranslatef(0, Mth::sin((timeHeld - 0.1f) * 1.3f) * 0.01f * (pow - 0.1f), 0);
                }
                glTranslatef(0, 0, pow * 0.1f);

                glRotatef(-45 - 290, 0, 0, 1);
                glRotatef(-50, 0, 1, 0);
                glTranslatef(0, 0.5f, 0);
                float ys = 1 + pow * 0.2f;
                glScalef(1, 1, ys);
                glTranslatef(0, -0.5f, 0);
                glRotatef(50, 0, 1, 0);
                glRotatef(45 + 290, 0, 0, 1);
            }
        }


        if (item->getItem()->isMirroredArt())
		{
            glRotatef(180, 0, 1, 0);
        }

        if (item->getItem()->hasMultipleSpriteLayers())
		{
            // special case for potions, refactor this when we get more
            // items that have two layers
            renderItem(player, item, 0, false);

			int col = Item::items[item->id]->getColor(item, 1);
			float red = ((col >> 16) & 0xff) / 255.0f;
			float g = ((col >> 8) & 0xff) / 255.0f;
			float b = ((col) & 0xff) / 255.0f;

			glColor4f(br * red, br * g, br * b, 1);

            renderItem(player, item, 1, false);
        }
		else
		{
            renderItem(player, item, 0, false);
        }
        glPopMatrix();
    }
	else if (!player->isInvisible())
	{
        glPushMatrix();
        float d = 0.8f;

        {
            float swing = player->getAttackAnim(a);

            float swing1 = Mth::sin(swing * PI);
            float swing2 = Mth::sin((sqrt(swing)) * PI);
            glTranslatef(-swing2 * 0.3f, Mth::sin(sqrt(swing) * PI * 2) * 0.4f, -swing1 * 0.4f);
        }

        glTranslatef(0.8f * d, -0.75f * d - (1 - h) * 0.6f, -0.9f * d);
		glTranslatef(fudgeX, fudgeY, fudgeZ);	// 4J added

        glRotatef(45, 0, 1, 0);
        glEnable(GL_RESCALE_NORMAL);
        {
            float swing = player->getAttackAnim(a);
            float swing3 = Mth::sin(swing * swing * PI);
            float swing2 = Mth::sin(sqrt(swing) * PI);
            glRotatef(swing2 * 70, 0, 1, 0);
            glRotatef(-swing3 * 20, 0, 0, 1);
        }

		// 4J-PB - if we've got a player texture, use that

		//glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadHttpTexture(minecraft->player->customTextureUrl, minecraft->player->getTexture()));

		MemSect(31);
		glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadMemTexture(minecraft->player->customTextureUrl, minecraft->player->getTexture()));
		MemSect(0);
		minecraft->textures->clearLastBoundId();
        glTranslatef(-1.0f, +3.6f, +3.5f);
        glRotatef(120, 0, 0, 1);
        glRotatef(180 + 20, 1, 0, 0);
        glRotatef(-90 - 45, 0, 1, 0);
        glScalef(1.5f / 24.0f * 16, 1.5f / 24.0f * 16, 1.5f / 24.0f * 16);
        glTranslatef(5.6f, 0, 0);

        EntityRenderer *er = EntityRenderDispatcher::instance->getRenderer(minecraft->player);
        PlayerRenderer *playerRenderer = static_cast<PlayerRenderer *>(er);
        float ss = 1;
        glScalef(ss, ss, ss);
		MemSect(31);
		// Can't turn off the hand if the player is holding a map
		shared_ptr<ItemInstance> itemInstance = player->inventory->getSelected();

		if ( (itemInstance && (itemInstance->getItem()->id==Item::map_Id)) || app.GetGameSettings(localPlayer->GetXboxPad(),eGameSetting_DisplayHand)!=0 )
		{
			playerRenderer->renderHand();
		}
		MemSect(0);
        glPopMatrix();
    }

    glDisable(GL_RESCALE_NORMAL);
    Lighting::turnOff();

}

void ItemInHandRenderer::renderScreenEffect(float a)
{
    glDisable(GL_ALPHA_TEST);
    if (minecraft->player->isOnFire())
	{
        renderFire(a);
    }

    if (minecraft->player->isInWall()) // Inside a tile
    {
        int x = Mth::floor(minecraft->player->x);
        int y = Mth::floor(minecraft->player->y);
        int z = Mth::floor(minecraft->player->z);

        int tile = minecraft->level->getTile(x, y, z);
        if (minecraft->level->isSolidBlockingTile(x, y, z))
		{
            renderTex(a, Tile::tiles[tile]->getTexture(2));
        }
		else
		{
            for (int i = 0; i < 8; i++)
			{
                float xo = ((i >> 0) % 2 - 0.5f) * minecraft->player->bbWidth * 0.9f;
                float yo = ((i >> 1) % 2 - 0.5f) * minecraft->player->bbHeight * 0.2f;
                float zo = ((i >> 2) % 2 - 0.5f) * minecraft->player->bbWidth * 0.9f;
                int xt = Mth::floor(x + xo);
                int yt = Mth::floor(y + yo);
                int zt = Mth::floor(z + zo);
                if (minecraft->level->isSolidBlockingTile(xt, yt, zt))
				{
                    tile = minecraft->level->getTile(xt, yt, zt);
                }
            }
        }

        if (Tile::tiles[tile] != nullptr) renderTex(a, Tile::tiles[tile]->getTexture(2));
    }

    if (minecraft->player->isUnderLiquid(Material::water))
	{
		MemSect(31);
        minecraft->textures->bindTexture(&UNDERWATER_LOCATION);	// 4J was L"/misc/water.png"
		MemSect(0);
        renderWater(a);
    }
    glEnable(GL_ALPHA_TEST);

}

void ItemInHandRenderer::renderTex(float a, Icon *slot)
{
	minecraft->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS); // TODO: get this data from Icon

    Tesselator *t = Tesselator::getInstance();

    float br = 0.1f;
    br = 0.1f;
    glColor4f(br, br, br, 0.5f);

    glPushMatrix();

    float x0 = -1;
    float x1 = +1;
    float y0 = -1;
    float y1 = +1;
    float z0 = -0.5f;

    float r = 2 / 256.0f;
    float u0 = slot->getU0();
    float u1 = slot->getU1();
    float v0 = slot->getV0();
    float v1 = slot->getV1();

    t->begin();
    t->vertexUV((float)(x0), (float)( y0), (float)( z0), (float)( u1), (float)( v1));
    t->vertexUV((float)(x1), (float)( y0), (float)( z0), (float)( u0), (float)( v1));
    t->vertexUV((float)(x1), (float)( y1), (float)( z0), (float)( u0), (float)( v0));
    t->vertexUV((float)(x0), (float)( y1), (float)( z0), (float)( u1), (float)( v0));
    t->end();
    glPopMatrix();

    glColor4f(1, 1, 1, 1);

}

void ItemInHandRenderer::renderWater(float a)
{
	minecraft->textures->bindTexture(&UNDERWATER_LOCATION);

    Tesselator *t = Tesselator::getInstance();

    float br = minecraft->player->getBrightness(a);
    glColor4f(br, br, br, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    float size = 4;

    float x0 = -1;
    float x1 = +1;
    float y0 = -1;
    float y1 = +1;
    float z0 = -0.5f;

    float uo = -minecraft->player->yRot / 64.0f;
    float vo = +minecraft->player->xRot / 64.0f;

    t->begin();
    t->vertexUV((float)(x0), (float)( y0), (float)( z0), (float)( size + uo), (float)( size + vo));
    t->vertexUV((float)(x1), (float)( y0), (float)( z0), (float)( 0 + uo), (float)( size + vo));
    t->vertexUV((float)(x1), (float)( y1), (float)( z0), (float)( 0 + uo), (float)( 0 + vo));
    t->vertexUV((float)(x0), (float)( y1), (float)( z0), (float)( size + uo), (float)( 0 + vo));
    t->end();
    glPopMatrix();

    glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);

}

void ItemInHandRenderer::renderFire(float a)
{
    Tesselator *t = Tesselator::getInstance();

	unsigned int col = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Fire_Overlay );
	float aCol = ( (col>>24)&0xFF )/255.0f;
	float rCol = ( (col>>16)&0xFF )/255.0f;
	float gCol = ( (col>>8)&0xFF )/255.0;
	float bCol = ( col&0xFF )/255.0;

    glColor4f(rCol, gCol, bCol, aCol);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float size = 1;
    for (int i = 0; i < 2; i++)
	{
        glPushMatrix();
		Icon *slot = Tile::fire->getTextureLayer(1);
		minecraft->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS); // TODO: Get this from Icon

		float u0 = slot->getU0(true);
		float u1 = slot->getU1(true);
		float v0 = slot->getV0(true);
		float v1 = slot->getV1(true);

        float x0 = (0 - size) / 2;
        float x1 = x0 + size;
        float y0 = 0 - size / 2;
        float y1 = y0 + size;
        float z0 = -0.5f;
        glTranslatef(-(i * 2 - 1) * 0.24f, -0.3f, 0);
        glRotatef((i * 2 - 1) * 10.0f, 0, 1, 0);

        t->begin();
        t->vertexUV((float)(x0), (float)( y0), (float)( z0), (float)( u1), (float)( v1));
        t->vertexUV((float)(x1), (float)( y0), (float)( z0), (float)( u0), (float)( v1));
        t->vertexUV((float)(x1), (float)( y1), (float)( z0), (float)( u0), (float)( v0));
        t->vertexUV((float)(x0), (float)( y1), (float)( z0), (float)( u1), (float)( v0));
        t->end();
        glPopMatrix();
    }
    glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);

}

void ItemInHandRenderer::tick()
{
    oHeight = height;


    shared_ptr<Player> player = minecraft->player;
    shared_ptr<ItemInstance> nextTile = player->inventory->getSelected();

    bool matches = lastSlot == player->inventory->selected && nextTile == selectedItem;
    if (selectedItem == nullptr && nextTile == nullptr)
	{
        matches = true;
    }
    if (nextTile != nullptr && selectedItem != nullptr && nextTile != selectedItem && nextTile->id == selectedItem->id && nextTile->getAuxValue() == selectedItem->getAuxValue())
	{
        selectedItem = nextTile;
        matches = true;
    }

    float max = 0.4f;
    float tHeight = matches ? 1.0f : 0;
    float dd = tHeight - height;
    if (dd < -max) dd = -max;
    if (dd > max) dd = max;

    height += dd;
    if (height < 0.1f)
	{
        selectedItem = nextTile;
        lastSlot = player->inventory->selected;
    }

}

void ItemInHandRenderer::reset()
{
	selectedItem = nullptr;
	lastSlot = -1;
	height = 0.0f;
	oHeight = 0.0f;
}

void ItemInHandRenderer::itemPlaced()
{
	height = 0;
}

void ItemInHandRenderer::itemUsed()
{
	height = 0;
}

