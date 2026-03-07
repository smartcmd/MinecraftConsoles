#include "stdafx.h"
#include "Minecraft.h"
#include "Minimap.h"
#include "Font.h"
#include "Options.h"
#include "Textures.h"
#include "Tesselator.h"
#include "../Minecraft.World/net.minecraft.world.level.saveddata.h"
#include "../Minecraft.World/net.minecraft.world.level.material.h"

#ifdef __ORBIS__
short Minimap::LUT[256];	// 4J added
#else
int Minimap::LUT[256];	// 4J added
#endif
bool Minimap::genLUT = true;		// 4J added

Minimap::Minimap(Font *font, Options *options, Textures *textures, bool optimised)
{
#ifdef __PS3__
	// we're using the RSX now to upload textures to vram, so we need the main ram textures allocated from io space
	this->pixels = intArray((int*)RenderManager.allocIOMem(w*h*sizeof(int)), 16*16);

#elif defined __ORBIS__
	this->pixels = shortArray(w*h);
#else
	this->pixels = intArray(w*h);
#endif
    this->options = options;
    this->font = font;
	BufferedImage *img = new BufferedImage(w, h, BufferedImage::TYPE_INT_ARGB);
#ifdef __ORBIS__
	mapTexture = textures->getTexture(img, C4JRender::TEXTURE_FORMAT_RxGyBzAw5551, false );	// 4J - make sure we aren't mipmapping as we never set the data for mipmaps
#else
    mapTexture = textures->getTexture(img, C4JRender::TEXTURE_FORMAT_RxGyBzAw, false );	// 4J - make sure we aren't mipmapping as we never set the data for mipmaps
#endif
	delete img;
    for (int i = 0; i < w * h; i++)
	{
        pixels[i] = 0x00000000;
	}

	// 4J added - generate the colour mapping that we'll be needing as a LUT to minimise processing we actually need to do during normal rendering
	if( genLUT )
	{
		reloadColours();
	}
	renderCount = 0;	// 4J added
	m_optimised = optimised;
}

void Minimap::reloadColours()
{
	ColourTable *colourTable = Minecraft::GetInstance()->getColourTable();
	// 4J note that this code has been extracted pretty much as it was in Minimap::render, although with some byte order changes
	for( int i = 0; i < (14 * 4); i++ )	// 14 material colours currently, 4 brightnesses of each
	{
		if (i / 4 == 0)
		{
			// 4J - changed byte order to save having to reorder later
#ifdef __ORBIS__
			LUT[i] = 0;
#else
			LUT[i] = (((i + i / w) & 1) * 8 + 16);
#endif
			//pixels[i] = (((i + i / w) & 1) * 8 + 16) << 24;
		}
		else
		{
			int color = colourTable->getColor( MaterialColor::colors[i / 4]->col );
			int brightness = i & 3;

			int br = 220;
			if (brightness == 2) br = 255;
			if (brightness == 0) br = 180;

			int r = ((color >> 16) & 0xff) * br / 255;
			int g = ((color >> 8) & 0xff) * br / 255;
			int b = ((color) & 0xff) * br / 255;

			// 4J - changed byte order to save having to reorder later
#if ( defined _DURANGO || defined _WIN64 || __PSVITA__ )
			LUT[i] =  255 << 24 | b << 16 | g << 8 | r;
#elif defined _XBOX
			LUT[i] =  255 << 24 | r << 16 | g << 8 | b;
#elif defined __ORBIS__
			r >>= 3; g >>= 3; b >>= 3;
			LUT[i] = 1 << 15 | ( r << 10 ) | ( g << 5 ) | b;
#else
			LUT[i] =  r << 24 | g << 16 | b << 8 | 255;
#endif

			//pixels[i] = (255) << 24 | r << 16 | g << 8 | b;
		}

	}
	genLUT = false;
}

// 4J added entityId
void Minimap::render(shared_ptr<Player> player, Textures *textures, shared_ptr<MapItemSavedData> data, int entityId)
{
	// 4J - only update every 8 renders, as an optimisation
	// We don't want to use this for ItemFrame renders of maps, as then we can't have different maps together
	if( !m_optimised || ( renderCount & 7 ) == 0 )
	{
		for (int i = 0; i < w * h; i++)
		{
			int val = data->colors[i];
			// 4J - moved the code that used to run here into a LUT that is generated once in the ctor above
			pixels[i] = LUT[val];
		}
	}
	renderCount++;

	// 4J - changed - have changed texture generation here to put the bytes in the right order already, so we don't have to do any copying round etc. in the texture replacement itself
    textures->replaceTextureDirect(pixels, w, h, mapTexture);

    int x = 0;
    int y = 0;
    Tesselator *t = Tesselator::getInstance();

    float vo = 0;

    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    t->begin();
	// 4J - moved to -0.02 to stop z fighting ( was -0.01)
	// AP - Vita still has issues so push it a bit more
	float Offset = -0.02f;
#ifdef __PSVITA__
	Offset = -0.03f;
#endif
    t->vertexUV((float)(x + 0 + vo), (float)( y + h - vo), (float)( Offset), (float)( 0), (float)( 1));
    t->vertexUV((float)(x + w - vo), (float)( y + h - vo), (float)( Offset), (float)( 1), (float)( 1));
    t->vertexUV((float)(x + w - vo), (float)( y + 0 + vo), (float)( Offset), (float)( 1), (float)( 0));
    t->vertexUV((float)(x + 0 + vo), (float)( y + 0 + vo), (float)( Offset), (float)( 0), (float)( 0));
    t->end();
    glEnable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);


    textures->bind(textures->loadTexture(TN_MISC_MAPICONS));//L"/misc/mapicons.png"));

#ifdef _LARGE_WORLDS
	vector<MapItemSavedData::MapDecoration *> m_edgeIcons;
#endif

	// 4J-PB - stack the map icons
	float fIconZ=-0.04f;// 4J - moved to -0.04 (was -0.02) to stop z fighting
	for( MapItemSavedData::MapDecoration *dec : data->decorations )
	{
		if(!dec->visible) continue;

		char imgIndex = dec->img;

#ifdef _LARGE_WORLDS
		// For edge icons, use a different texture
		if(imgIndex >= 16)
		{
			m_edgeIcons.push_back(dec);
			continue;
		}
#endif

		// 4J Stu - For item frame renders, the player is NULL. We do not want to show player icons on the frames.
		if(player == NULL && (imgIndex != 12)) continue;
		else if (player != NULL && imgIndex == 12) continue;
		else if( imgIndex == 12 && dec->entityId != entityId) continue;

        glPushMatrix();
        glTranslatef(x + dec->x / 2.0f + w / 2, y + dec->y / 2.0f + h / 2, fIconZ);
        glRotatef(dec->rot * 360 / 16.0f, 0, 0, 1);
        glScalef(4, 4, 3);
        glTranslatef(-1.0f / 8.0f, +1.0f / 8.0f, 0);

        float u0 = (imgIndex % 4 + 0) / 4.0f;
        float v0 = (imgIndex / 4 + 0) / 4.0f;
        float u1 = (imgIndex % 4 + 1) / 4.0f;
        float v1 = (imgIndex / 4 + 1) / 4.0f;

        t->begin();
        t->vertexUV((float)(-1), (float)( +1), (float)( 0), (float)( u0), (float)( v0));
        t->vertexUV((float)(+1), (float)( +1), (float)( 0), (float)( u1), (float)( v0));
        t->vertexUV((float)(+1), (float)( -1), (float)( 0), (float)( u1), (float)( v1));
        t->vertexUV((float)(-1), (float)( -1), (float)( 0), (float)( u0), (float)( v1));
        t->end();
        glPopMatrix();
		fIconZ-=0.01f;
    }

#ifdef _LARGE_WORLDS
	// For players on the edge of the world
	textures->bind(textures->loadTexture(TN_MISC_ADDITIONALMAPICONS));

	fIconZ=-0.04f;// 4J - moved to -0.04 (was -0.02) to stop z fighting
	for( MapItemSavedData::MapDecoration *dec : m_edgeIcons )
	{
		char imgIndex = dec->img;
		imgIndex -= 16;

		// 4J Stu - For item frame renders, the player is NULL. We do not want to show player icons on the frames.
		if(player == NULL && (imgIndex != 12)) continue;
		else if (player != NULL && imgIndex == 12) continue;
		else if( imgIndex == 12 && dec->entityId != entityId) continue;

        glPushMatrix();
        glTranslatef(x + dec->x / 2.0f + w / 2, y + dec->y / 2.0f + h / 2, fIconZ);
        glRotatef(dec->rot * 360 / 16.0f, 0, 0, 1);
        glScalef(4, 4, 3);
        glTranslatef(-1.0f / 8.0f, +1.0f / 8.0f, 0);

        float u0 = (imgIndex % 4 + 0) / 4.0f;
        float v0 = (imgIndex / 4 + 0) / 4.0f;
        float u1 = (imgIndex % 4 + 1) / 4.0f;
        float v1 = (imgIndex / 4 + 1) / 4.0f;

        t->begin();
        t->vertexUV((float)(-1), (float)( +1), (float)( 0), (float)( u0), (float)( v0));
        t->vertexUV((float)(+1), (float)( +1), (float)( 0), (float)( u1), (float)( v0));
        t->vertexUV((float)(+1), (float)( -1), (float)( 0), (float)( u1), (float)( v1));
        t->vertexUV((float)(-1), (float)( -1), (float)( 0), (float)( u0), (float)( v1));
        t->end();
        glPopMatrix();
		fIconZ-=0.01f;
    }
#endif

    glPushMatrix();
//        glRotatef(0, 1, 0, 0);
    glTranslatef(0, 0, -0.06f);
    glScalef(1, 1, 1);
// 4J Stu - Don't render the text name, except in debug
//#if 1
//#ifdef _DEBUG
//    font->draw(data->id, x, y, 0xff000000);
//#else
	// 4J Stu - TU-1 hotfix
	// DCR: Render the players current position here instead
	if(player != NULL)
	{
		wchar_t playerPosText[32];
		ZeroMemory(&playerPosText, sizeof(wchar_t) * 32);
		int posx = floor(player->x);
		int posy = floor(player->y);
		int posz = floor(player->z);
		swprintf(playerPosText, 32, L"X: %d, Y: %d, Z: %d", posx, posy, posz);

		font->draw(playerPosText, x, y, Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Map_Text));
	}
//#endif
    glPopMatrix();

}
