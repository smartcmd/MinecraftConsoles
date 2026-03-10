#include "stdafx.h"
#include "Textures.h"
#include "TexturePackRepository.h"
#include "HttpTexture.h"
#include "MemTexture.h"
#include "..\Minecraft.World\InputStream.h"
#include "..\Minecraft.World\IntBuffer.h"
#include "..\Minecraft.World\ByteBuffer.h"
#include "TexturePack.h"
#include "Options.h"
#include "..\Minecraft.Client\MemTextureProcessor.h"
#include "MobSkinMemTextureProcessor.h"
#include "PreStitchedTextureMap.h"
#include "StitchedTexture.h"
#include "Texture.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "ResourceLocation.h"
#include "..\Minecraft.World\ItemEntity.h"
#include "TextureAtlas.h"

bool Textures::MIPMAP = true;
C4JRender::eTextureFormat Textures::TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;

int Textures::preLoadedIdx[TN_COUNT];
const wchar_t *Textures::preLoaded[TN_COUNT] =
{
	L"%blur%misc/pumpkinblur",
//	L"%blur%/misc/vignette",		// Not currently used
	L"%clamp%misc/shadow",
//	L"/achievement/bg",				// Not currently used
	L"art/kz",
	L"environment/clouds",
	L"environment/rain",
	L"environment/snow",
	L"gui/gui",
	L"gui/icons",
	L"item/arrows",
	L"item/boat",
	L"item/cart",
	L"item/sign",
	L"misc/mapbg",
	L"misc/mapicons",
	L"misc/water",
	L"misc/footprint",
	L"mob/saddle",
	L"mob/sheep_fur",
	L"mob/spider_eyes",
	L"particles",
	L"mob/chicken",
	L"mob/cow",
	L"mob/pig",
	L"mob/sheep",
	L"mob/squid",
	L"mob/wolf",
	L"mob/wolf_tame",
	L"mob/wolf_angry",
	L"mob/creeper",
	L"mob/ghast",
	L"mob/ghast_fire",
	L"mob/zombie",
	L"mob/pigzombie",
	L"mob/skeleton",
	L"mob/slime",
	L"mob/spider",
	L"mob/char",
	L"mob/char1",
	L"mob/char2",
	L"mob/char3",
	L"mob/char4",
	L"mob/char5",
	L"mob/char6",
	L"mob/char7",
	L"terrain/moon",
	L"terrain/sun",
	L"armor/power",

	// 1.8.2
	L"mob/cavespider",
	L"mob/enderman",
	L"mob/silverfish",
	L"mob/enderman_eyes",
	L"misc/explosion",
	L"item/xporb",
	L"item/chest",
	L"item/largechest",

	// 1.3.2
	L"item/enderchest",

	// 1.0.1
	L"mob/redcow",
	L"mob/snowman",
	L"mob/enderdragon/ender",
	L"mob/fire",
	L"mob/lava",
	L"mob/villager/villager",
	L"mob/villager/farmer",
	L"mob/villager/librarian",
	L"mob/villager/priest",
	L"mob/villager/smith",
	L"mob/villager/butcher",
	L"mob/enderdragon/crystal",
	L"mob/enderdragon/shuffle",
	L"mob/enderdragon/beam",
	L"mob/enderdragon/ender_eyes",
	L"%blur%misc/glint",
	L"item/book",
	L"misc/tunnel",
	L"misc/particlefield",
	L"terrain/moon_phases",

	// 1.2.3
	L"mob/ozelot",
	L"mob/cat_black",
	L"mob/cat_red",
	L"mob/cat_siamese",
	L"mob/villager_golem",
	L"mob/skeleton_wither",

	// TU 14
	L"mob/wolf_collar",
	L"mob/zombie_villager",

	// 1.6.4
	L"item/lead_knot",

	L"misc/beacon_beam",

	L"mob/bat",

	L"mob/horse/donkey",
	L"mob/horse/horse_black",
	L"mob/horse/horse_brown",
	L"mob/horse/horse_chestnut",
	L"mob/horse/horse_creamy",
	L"mob/horse/horse_darkbrown",
	L"mob/horse/horse_gray",
	L"mob/horse/horse_markings_blackdots",
	L"mob/horse/horse_markings_white",
	L"mob/horse/horse_markings_whitedots",
	L"mob/horse/horse_markings_whitefield",
	L"mob/horse/horse_skeleton",
	L"mob/horse/horse_white",
	L"mob/horse/horse_zombie",
	L"mob/horse/mule",

	L"mob/horse/armor/horse_armor_diamond",
	L"mob/horse/armor/horse_armor_gold",
	L"mob/horse/armor/horse_armor_iron",

	L"mob/witch",

	L"mob/wither/wither",
	L"mob/wither/wither_armor",
	L"mob/wither/wither_invulnerable",

	L"item/trapped",
	L"item/trapped_double",
	//L"item/christmas",
	//L"item/christmas_double",

#ifdef _LARGE_WORLDS
	L"misc/additionalmapicons",
#endif

	L"font/Default",
	L"font/alternate",

	// skin packs
/*	L"/SP1",
	L"/SP2",
	L"/SP3",
	L"/SPF",

	// themes
	L"/ThSt",
	L"/ThIr",
	L"/ThGo",
	L"/ThDi",

	// gamerpics
	L"/GPAn",
	L"/GPCo",
	L"/GPEn",
	L"/GPFo",
	L"/GPTo",
	L"/GPBA",
	L"/GPFa",
	L"/GPME",
	L"/GPMF",
	L"/GPMM",
	L"/GPSE",

	// avatar items

	L"/AH_0006",
	L"/AH_0003",
	L"/AH_0007",
	L"/AH_0005",
	L"/AH_0004",
	L"/AH_0001",
	L"/AH_0002",
	L"/AT_0001",
	L"/AT_0002",
	L"/AT_0003",
	L"/AT_0004",
	L"/AT_0005",
	L"/AT_0006",
	L"/AT_0007",
	L"/AT_0008",
	L"/AT_0009",
	L"/AT_0010",
	L"/AT_0011",
	L"/AT_0012",
	L"/AP_0001",
	L"/AP_0002",
	L"/AP_0003",
	L"/AP_0004",
	L"/AP_0005",
	L"/AP_0006",
	L"/AP_0007",
	L"/AP_0009",
	L"/AP_0010",
	L"/AP_0011",
	L"/AP_0012",
	L"/AP_0013",
	L"/AP_0014",
	L"/AP_0015",
	L"/AP_0016",
	L"/AP_0017",
	L"/AP_0018",
	L"/AA_0001",
	L"/AT_0013",
	L"/AT_0014",
	L"/AT_0015",
	L"/AT_0016",
	L"/AT_0017",
	L"/AT_0018",
	L"/AP_0019",
	L"/AP_0020",
	L"/AP_0021",
	L"/AP_0022",
	L"/AP_0023",
	L"/AH_0008",
	L"/AH_0009",*/

	L"gui/items",
	L"terrain",
};

Textures::Textures(TexturePackRepository *skins, Options *options)
{
//    pixels = MemoryTracker::createIntBuffer(2048 * 2048);	// 4J removed - now just creating this buffer when we need it
	missingNo = new BufferedImage(16, 16, BufferedImage::TYPE_INT_ARGB);

    this->skins = skins;
    this->options = options;

	/* 4J - TODO, maybe...
	Graphics g = missingNo.getGraphics();
	g.setColor(Color.WHITE);
	g.fillRect(0, 0, 64, 64);
	g.setColor(Color.BLACK);
	int y = 10;
	int i = 0;
	while (y < 64) {
		String text = (i++ % 2 == 0) ? "missing" : "texture";
		g.drawString(text, 1, y);
		y += g.getFont().getSize();
		if (i % 2 == 0) y += 5;
	}

    g.dispose();
	*/

	// 4J Stu - Changed these to our PreStitchedTextureMap from TextureMap
	terrain = new PreStitchedTextureMap(Icon::TYPE_TERRAIN, L"terrain", L"textures/blocks/", missingNo, true);
	items = new PreStitchedTextureMap(Icon::TYPE_ITEM, L"items", L"textures/items/", missingNo, true);

	// 4J - added - preload a set of commonly used textures that can then be referenced directly be an enumerated type rather by string
	loadIndexedTextures();
}

void Textures::loadIndexedTextures()
{
	// 4J - added - preload a set of commonly used textures that can then be referenced directly be an enumerated type rather by string
	for( int i = 0; i < TN_COUNT - 2; i++ )
	{
		preLoadedIdx[i] = loadTexture(static_cast<TEXTURE_NAME>(i), wstring(preLoaded[i]) + L".png");
	}
}

intArray Textures::loadTexturePixels(TEXTURE_NAME texId, const wstring& resourceName)
{
    TexturePack *skin = skins->getSelected();

    {
        intArray id = pixelsMap[resourceName];
		// 4J - if resourceName isn't in the map, it should add an element and as that will use the default constructor, its
		// internal data pointer will be nullptr
        if (id.data != nullptr) return id;
    }

	// 4J - removed try/catch
//    try {
        intArray res;
		//wstring in = skin->getResource(resourceName);
		if (false)// 4J - removed - was ( in == nullptr)
		{
			res = loadTexturePixels(missingNo);
		}
		else
		{
			BufferedImage *bufImage = readImage(texId, resourceName); //in);
			res = loadTexturePixels(bufImage);
			delete bufImage;
		}

        pixelsMap[resourceName] = res;
        return res;
/*
}
	catch (IOException e) {
        e.printStackTrace();
        int[] res = loadTexturePixels(missingNo);
        pixelsMap.put(resourceName, res);
        return res;
    }
	*/
}

intArray Textures::loadTexturePixels(BufferedImage *img)
{
    int w = img->getWidth();
    int h = img->getHeight();
    intArray pixels(w*h);
    return loadTexturePixels(img, pixels);
}

intArray Textures::loadTexturePixels(BufferedImage *img, intArray pixels)
{
    int w = img->getWidth();
    int h = img->getHeight();
    img->getRGB(0, 0, w, h, pixels, 0, w);
    return pixels;
}

int Textures::loadTexture(int idx)
{
	if( idx == -1 )
	{
			return 0;
	}
	else
	{
		if ( idx == TN_TERRAIN )
		{
			terrain->getStitchedTexture()->bind(0);
			return terrain->getStitchedTexture()->getGlId();
		}
		if ( idx == TN_GUI_ITEMS)
		{
			items->getStitchedTexture()->bind(0);
			return items->getStitchedTexture()->getGlId();
		}
		return preLoadedIdx[idx];
	}
}

// 4J added - textures default to standard 32-bit RGBA format, but where we can, use an 8-bit format. There's 3 different varieties of these currently
// in the renderer that map the single 8-bit channel to RGBA differently.
void Textures::setTextureFormat(const wstring& resourceName)
{
	// 4J Stu - These texture formats are not currently in the render header
#ifdef _XBOX
	if(resourceName == L"/environment/clouds.png")
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_R1G1B1Ax;
	}
	else if(resourceName == L"%blur%/misc/pumpkinblur.png")
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_R0G0B0Ax;
	}
	else if(resourceName == L"%clamp%/misc/shadow.png")
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_R0G0B0Ax;
	}
	else if(resourceName == L"/environment/snow.png")
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGxBxAx;
	}
	else if(resourceName == L"/1_2_2/misc/explosion.png")
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGxBxAx;
	}
	else
#endif
	{
		TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
	}
}

void Textures::bindTexture(const wstring &resourceName)
{
	bind(loadTexture(TN_COUNT,resourceName));
}

// 4J Added
void Textures::bindTexture(ResourceLocation *resource)
{
	if(resource->isPreloaded())
	{
		bind(loadTexture(resource->getTexture()));
	}
	else
	{
		bind(loadTexture(TN_COUNT, resource->getPath()));
	}
}

void Textures::bindTextureLayers(ResourceLocation *resource)
{
	assert(resource->isPreloaded());

	// Hack: 4JLibs on Windows does not currently reproduce Minecraft's layered horse texture path reliably.
	// Merge the layers on the CPU and bind the cached result as a normal single texture instead.
	wstring cacheKey = L"%layered%";
	int layers = resource->getTextureCount();
	for( int i = 0; i < layers; i++ )
	{
		cacheKey += std::to_wstring(resource->getTexture(i));
		cacheKey += L"/";
	}

	int id = -1;
	bool inMap = ( idMap.find(cacheKey) != idMap.end() );
	if( inMap )
	{
		id = idMap[cacheKey];
	}
	else
	{
		// Cache by layer signature so the merge cost is only paid once per horse texture combination.
		intArray mergedPixels;
		int mergedWidth = 0;
		int mergedHeight = 0;
		bool hasMergedPixels = false;

		for( int i = 0; i < layers; i++ )
		{
			TEXTURE_NAME textureName = resource->getTexture(i);
			if( textureName == static_cast<_TEXTURE_NAME>(-1) )
			{
				continue;
			}

			wstring resourceName = wstring(preLoaded[textureName]) + L".png";
			BufferedImage *image = readImage(textureName, resourceName);
			if( image == nullptr )
			{
				continue;
			}

			int width = image->getWidth();
			int height = image->getHeight();
			intArray layerPixels = loadTexturePixels(image);
			delete image;

			if( !hasMergedPixels )
			{
				mergedWidth = width;
				mergedHeight = height;
				mergedPixels = intArray(width * height);
				memcpy(mergedPixels.data, layerPixels.data, width * height * sizeof(int));
				hasMergedPixels = true;
			}
			else if( width == mergedWidth && height == mergedHeight )
			{
				for( int p = 0; p < width * height; p++ )
				{
					int dst = mergedPixels[p];
					int src = layerPixels[p];

					float srcAlpha = ((src >> 24) & 0xff) / 255.0f;
					if( srcAlpha <= 0.0f )
					{
						continue;
					}

					float dstAlpha = ((dst >> 24) & 0xff) / 255.0f;
					float outAlpha = srcAlpha + dstAlpha * (1.0f - srcAlpha);
					if( outAlpha <= 0.0f )
					{
						mergedPixels[p] = 0;
						continue;
					}

					float srcFactor = srcAlpha / outAlpha;
					float dstFactor = (dstAlpha * (1.0f - srcAlpha)) / outAlpha;

					int outA = static_cast<int>(outAlpha * 255.0f + 0.5f);
					int outR = static_cast<int>((((src >> 16) & 0xff) * srcFactor) + (((dst >> 16) & 0xff) * dstFactor) + 0.5f);
					int outG = static_cast<int>((((src >> 8) & 0xff) * srcFactor) + (((dst >> 8) & 0xff) * dstFactor) + 0.5f);
					int outB = static_cast<int>(((src & 0xff) * srcFactor) + ((dst & 0xff) * dstFactor) + 0.5f);
					mergedPixels[p] = (outA << 24) | (outR << 16) | (outG << 8) | outB;
				}
			}

			delete[] layerPixels.data;
		}

		if( hasMergedPixels )
		{
			BufferedImage *mergedImage = new BufferedImage(mergedWidth, mergedHeight, BufferedImage::TYPE_INT_ARGB);
			memcpy(mergedImage->getData(), mergedPixels.data, mergedWidth * mergedHeight * sizeof(int));
			delete[] mergedPixels.data;
			id = getTexture(mergedImage, C4JRender::TEXTURE_FORMAT_RxGyBzAw, false);
		}
		else
		{
			id = 0;
		}

		idMap[cacheKey] = id;
	}

	RenderManager.TextureBind(id);
}

void Textures::bind(int id)
{
	//if (id != lastBoundId)
	{
		if(id < 0) return;
		glBindTexture(GL_TEXTURE_2D, id);
	//	lastBoundId = id;
	}
}

ResourceLocation *Textures::getTextureLocation(shared_ptr<Entity> entity)
{
	shared_ptr<ItemEntity> item = dynamic_pointer_cast<ItemEntity>(entity);
	int iconType = item->getItem()->getIconType();
	return getTextureLocation(iconType);
}

ResourceLocation *Textures::getTextureLocation(int iconType)
{
	switch(iconType)
	{
	case Icon::TYPE_TERRAIN:
		return &TextureAtlas::LOCATION_BLOCKS;
		break;
	case Icon::TYPE_ITEM:
		return &TextureAtlas::LOCATION_ITEMS;
		break;
	}
}

void Textures::clearLastBoundId()
{
	lastBoundId = -1;
}

int Textures::loadTexture(TEXTURE_NAME texId, const wstring& resourceName)
{
// 	char buf[256];
// 	wcstombs(buf, resourceName.c_str(), 256);
// 	printf("Textures::loadTexture name - %s\n",buf);

	//if (resourceName.compare(L"/terrain.png") == 0)
	//{
	//	terrain->getStitchedTexture()->bind(0);
	//	return terrain->getStitchedTexture()->getGlId();
	//}
	//if (resourceName.compare(L"/gui/items.png") == 0)
	//{
	//	items->getStitchedTexture()->bind(0);
	//	return items->getStitchedTexture()->getGlId();
	//}

	// If the texture is not present in the idMap, load it, otherwise return its id

    {
		bool inMap = ( idMap.find(resourceName) != idMap.end() );
        int id = idMap[resourceName];
        if (inMap) return id;
    }

	wstring pathName = resourceName;

	// 4J - added special cases to avoid mipmapping on clouds & shadows
	if( (resourceName == L"environment/clouds.png") ||
		(resourceName == L"%clamp%misc/shadow.png") ||
		(resourceName == L"%blur%misc/pumpkinblur.png") ||
		(resourceName == L"%clamp%misc/shadow.png") ||
		(resourceName == L"gui/icons.png" ) ||
		(resourceName == L"gui/gui.png" ) ||
		(resourceName == L"misc/footprint.png") )
	{
		MIPMAP = false;
	}
	setTextureFormat(resourceName);

	// 4J - removed try/catch
//    try {
	int id = MemoryTracker::genTextures();


	wstring prefix = L"%blur%";
	bool blur = resourceName.substr(0, prefix.size()).compare(prefix) == 0; //resourceName.startsWith("%blur%");
	if (blur) pathName = resourceName.substr(6);

	prefix = L"%clamp%";
	bool clamp = resourceName.substr(0, prefix.size()).compare(prefix) == 0; //resourceName.startsWith("%clamp%");
	if (clamp) pathName = resourceName.substr(7);

	//wstring in = skins->getSelected()->getResource(pathName);
	if (false ) // 4J - removed was ( in == nullptr)
	{
		loadTexture(missingNo, id, blur, clamp);
	}
	else
	{
		// 4J Stu - Get resource above just returns the name for texture packs
		BufferedImage *bufImage = readImage(texId, pathName); //in);
		loadTexture(bufImage, id, blur, clamp);
		delete bufImage;
	}

	idMap[resourceName] = id;
	MIPMAP = true; // 4J added
	TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
	return id;
		/*
    } catch (IOException e) {
        e.printStackTrace();
        MemoryTracker.genTextures(ib);
        int id = ib.get(0);
        loadTexture(missingNo, id);
        idMap.put(resourceName, id);
        return id;
    }
	*/
}

int Textures::getTexture(BufferedImage *img, C4JRender::eTextureFormat format, bool mipmap)
{
    int id = MemoryTracker::genTextures();
	TEXTURE_FORMAT = format;
	MIPMAP = mipmap;
    loadTexture(img, id);
	TEXTURE_FORMAT = C4JRender::TEXTURE_FORMAT_RxGyBzAw;
	MIPMAP = true;
    loadedImages[id] = img;
    return id;
}

void Textures::loadTexture(BufferedImage *img, int id)
{
//	printf("Textures::loadTexture BufferedImage %d\n",id);

	loadTexture(img, id, false, false);
}

void Textures::loadTexture(BufferedImage *img, int id, bool blur, bool clamp)
{
//	printf("Textures::loadTexture BufferedImage with blur and clamp %d\n",id);
	int iMipLevels=1;
	MemSect(33);
    glBindTexture(GL_TEXTURE_2D, id);

    if (MIPMAP)
	{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        /*
            * glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
            * glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
            * glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
            * glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
            */
    }
	else
	{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (blur)
	{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    if (clamp)
	{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
	else
	{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    int w = img->getWidth();
    int h = img->getHeight();

    intArray rawPixels(w*h);
    img->getRGB(0, 0, w, h, rawPixels, 0, w);

	if (options != nullptr && options->anaglyph3d)
	{
		rawPixels = anaglyph(rawPixels);
	}

    byteArray newPixels(w * h * 4);
    for (unsigned int i = 0; i < rawPixels.length; i++)
	{
        int a = (rawPixels[i] >> 24) & 0xff;
        int r = (rawPixels[i] >> 16) & 0xff;
        int g = (rawPixels[i] >> 8) & 0xff;
        int b = (rawPixels[i]) & 0xff;

#ifdef _XBOX
        newPixels[i * 4 + 0] = (byte) a;
        newPixels[i * 4 + 1] = (byte) r;
        newPixels[i * 4 + 2] = (byte) g;
        newPixels[i * 4 + 3] = (byte) b;
#else
        newPixels[i * 4 + 0] = static_cast<byte>(r);
        newPixels[i * 4 + 1] = static_cast<byte>(g);
        newPixels[i * 4 + 2] = static_cast<byte>(b);
        newPixels[i * 4 + 3] = static_cast<byte>(a);
#endif
    }
	// 4J - now creating a buffer of the size we require dynamically
	ByteBuffer *pixels =  MemoryTracker::createByteBuffer(w * h * 4);
    pixels->clear();
    pixels->put(newPixels);
    pixels->position(0)->limit(newPixels.length);

	delete[] rawPixels.data;
	delete[] newPixels.data;

	if (MIPMAP)
	{
		// 4J-PB - In the new XDK, the CreateTexture will fail if the number of mipmaps is higher than the width & height passed in will allow!
		int iWidthMips=1;
		int iHeightMips=1;
		while((8<<iWidthMips)<w) iWidthMips++;
		while((8<<iHeightMips)<h) iHeightMips++;

		iMipLevels=(iWidthMips<iHeightMips)?iWidthMips:iHeightMips;
		//RenderManager.TextureSetTextureLevels(5);	// 4J added
		if(iMipLevels>5)iMipLevels = 5;
		RenderManager.TextureSetTextureLevels(iMipLevels);	// 4J added
	}
	RenderManager.TextureData(w,h,pixels->getBuffer(),0,TEXTURE_FORMAT);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL12.GL_BGRA, GL12.GL_UNSIGNED_INT_8_8_8_8_REV, pixels);

	if (MIPMAP)
	{
        for (int level = 1; level < iMipLevels; level++)
		{
            int ow = w >> (level - 1);
            // int oh = h >> (level - 1);

            int ww = w >> level;
            int hh = h >> level;

			// 4J - added tempData so we aren't overwriting source data
			unsigned int *tempData = new unsigned int[ww * hh];
			// 4J - added - have we loaded mipmap data for this level? Use that rather than generating if possible
			if( img->getData( level ) )
			{
				memcpy( tempData, img->getData( level ), ww * hh * 4);
#ifndef _XBOX
				// Swap ARGB to RGBA
				for( int i = 0; i < ww * hh ; i++ )
				{
					tempData[i] = ( tempData[i] >> 24 ) | (tempData[i] << 8 );
				}
#endif
			}
			else
			{
				for (int x = 0; x < ww; x++)
					for (int y = 0; y < hh; y++)
					{
						int c0 = pixels->getInt(((x * 2 + 0) + (y * 2 + 0) * ow) * 4);
						int c1 = pixels->getInt(((x * 2 + 1) + (y * 2 + 0) * ow) * 4);
						int c2 = pixels->getInt(((x * 2 + 1) + (y * 2 + 1) * ow) * 4);
						int c3 = pixels->getInt(((x * 2 + 0) + (y * 2 + 1) * ow) * 4);
#ifndef _XBOX
						// 4J - convert our RGBA texels to ARGB that crispBlend is expecting
						c0 = ( ( c0 >> 8 ) & 0x00ffffff ) | ( c0 << 24 );
						c1 = ( ( c1 >> 8 ) & 0x00ffffff ) | ( c1 << 24 );
						c2 = ( ( c2 >> 8 ) & 0x00ffffff ) | ( c2 << 24 );
						c3 = ( ( c3 >> 8 ) & 0x00ffffff ) | ( c3 << 24 );
#endif
						int col = Texture::crispBlend(Texture::crispBlend(c0, c1), Texture::crispBlend(c2, c3));
#ifndef _XBOX
						// 4J - and back from ARGB -> RGBA
						col = ( col << 8 ) | (( col >> 24 ) & 0xff);
#endif
						tempData[x + y * ww] = col;
					}
			}
			for (int x = 0; x < ww; x++ )
                for (int y = 0; y < hh; y++)
				{
					pixels->putInt((x + y * ww) * 4, tempData[x + y * ww]);
				}
			delete [] tempData;
			RenderManager.TextureData(ww,hh,pixels->getBuffer(),level,TEXTURE_FORMAT);
        }
    }

    /*
        * if (MIPMAP) { GLU.gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h,
        * GL_RGBA, GL_UNSIGNED_BYTE, pixels); } else { }
        */
	delete pixels;	// 4J - now creating this dynamically
	MemSect(0);
}

intArray Textures::anaglyph(intArray rawPixels)
{
	intArray result(rawPixels.length);
	for (unsigned int i = 0; i < rawPixels.length; i++)
	{
		int a = (rawPixels[i] >> 24) & 0xff;
		int r = (rawPixels[i] >> 16) & 0xff;
		int g = (rawPixels[i] >> 8) & 0xff;
		int b = (rawPixels[i]) & 0xff;

		int rr = (r * 30 + g * 59 + b * 11) / 100;
		int gg = (r * 30 + g * 70) / (100);
		int bb = (r * 30 + b * 70) / (100);


		result[i] = a << 24 | rr << 16 | gg << 8 | bb;
	}

	delete[] rawPixels.data;

	return result;
}

void Textures::replaceTexture(intArray rawPixels, int w, int h, int id)
{
    bind(id);

	// Removed in Java
#if 0
	if (MIPMAP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		/*
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		*/
	}
	else
#endif
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (options != nullptr && options->anaglyph3d)
	{
		rawPixels = anaglyph(rawPixels);
	}

	byteArray newPixels(w * h * 4);
    for (unsigned int i = 0; i < rawPixels.length; i++)
	{
        int a = (rawPixels[i] >> 24) & 0xff;
        int r = (rawPixels[i] >> 16) & 0xff;
        int g = (rawPixels[i] >> 8) & 0xff;
        int b = (rawPixels[i]) & 0xff;

		if (options != nullptr && options->anaglyph3d)
		{
            int rr = (r * 30 + g * 59 + b * 11) / 100;
            int gg = (r * 30 + g * 70) / (100);
            int bb = (r * 30 + b * 70) / (100);

            r = rr;
            g = gg;
            b = bb;
        }

        newPixels[i * 4 + 0] = static_cast<byte>(r);
        newPixels[i * 4 + 1] = static_cast<byte>(g);
        newPixels[i * 4 + 2] = static_cast<byte>(b);
        newPixels[i * 4 + 3] = static_cast<byte>(a);
    }
    ByteBuffer *pixels =  MemoryTracker::createByteBuffer(w * h * 4);	// 4J - now creating dynamically
    pixels->put(newPixels);
    pixels->position(0)->limit(newPixels.length);
	delete [] newPixels.data;

	// New
	// glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL12.GL_BGRA, GL12.GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
#ifdef _XBOX
	RenderManager.TextureDataUpdate(pixels->getBuffer(),0);
#else
	RenderManager.TextureDataUpdate(0, 0,w,h,pixels->getBuffer(),0);
#endif
	// Old
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	delete pixels;
}

// 4J - added. This is a more minimal version of replaceTexture that assumes the texture bytes are already in order, and so doesn't do any of the extra copying round
// that the original java version does
void Textures::replaceTextureDirect(intArray rawPixels, int w, int h, int id)
{
    glBindTexture(GL_TEXTURE_2D, id);

	// Remove in Java
#if 0
	if (MIPMAP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		/*
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		*/
	}
	else
#endif
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifdef _XBOX
	RenderManager.TextureDataUpdate(rawPixels.data, 0);
#else
	RenderManager.TextureDataUpdate(0, 0, w, h, rawPixels.data, 0);
#endif
}

// 4J - added. This is a more minimal version of replaceTexture that assumes the texture bytes are already in order, and so doesn't do any of the extra copying round
// that the original java version does
void Textures::replaceTextureDirect(shortArray rawPixels, int w, int h, int id)
{
    glBindTexture(GL_TEXTURE_2D, id);

	// Remove in Java
#if 0
	if (MIPMAP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		/*
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 4);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		*/
	}
	else
#endif
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifdef _XBOX
	RenderManager.TextureDataUpdate(rawPixels.data, 0);
#else
	RenderManager.TextureDataUpdate(0, 0, w, h, rawPixels.data, 0);
#endif
}

void Textures::releaseTexture(int id)
{
    if (id <= 0) return;
    loadedImages.erase(id);
    // TextureFree() has no bounds check and crashes on stale IDs (e.g. after
    // RenderManager.Initialise() which memsets the texture table to zero).
    // TextureGetTexture() IS safe — returns NULL for invalid/unallocated IDs.
    if (RenderManager.TextureGetTexture(id) != NULL)
    {
        glDeleteTextures(id);
    }
}

int Textures::loadHttpTexture(const wstring& url, const wstring& backup)
{
    HttpTexture *texture = httpTextures[url];
    if (texture != nullptr)
	{
        if (texture->loadedImage != nullptr && !texture->isLoaded)
		{
            if (texture->id < 0)
			{
                texture->id = getTexture(texture->loadedImage);
            }
			else
			{
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
        }
    }
    if (texture == nullptr || texture->id < 0)
	{
		if (backup.empty() ) return -1;
        return loadTexture(TN_COUNT, backup);
    }
    return texture->id;
}

int Textures::loadHttpTexture(const wstring& url, int backup)
{
    HttpTexture *texture = httpTextures[url];
    if (texture != nullptr)
	{
        if (texture->loadedImage != nullptr && !texture->isLoaded)
		{
            if (texture->id < 0)
			{
                texture->id = getTexture(texture->loadedImage);
            }
			else
			{
                loadTexture(texture->loadedImage, texture->id);
            }
            texture->isLoaded = true;
        }
    }
    if (texture == nullptr || texture->id < 0)
	{
        return loadTexture(backup);
    }
    return texture->id;
}

bool Textures::hasHttpTexture(const wstring &url)
{
	return httpTextures.find(url) != httpTextures.end();
}

HttpTexture *Textures::addHttpTexture(const wstring& url, HttpTextureProcessor *processor)
{
    HttpTexture *texture = httpTextures[url];
    if (texture == nullptr)
	{
        httpTextures[url] = new HttpTexture(url, processor);
    }
	else
	{
        texture->count++;
    }
    return texture;
}

void Textures::removeHttpTexture(const wstring& url)
{
    HttpTexture *texture = httpTextures[url];
    if (texture != nullptr)
	{
        texture->count--;
        if (texture->count == 0)
		{
            if (texture->id >= 0) releaseTexture(texture->id);
            httpTextures.erase(url);
        }
    }
}

// 4J-PB - adding for texture in memory (from global title storage)
int Textures::loadMemTexture(const wstring& url, const wstring& backup)
{
	MemTexture *texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end())
	{
		texture = (*it).second;
	}
	if(texture == nullptr && app.IsFileInMemoryTextures(url))
	{
		// If we haven't loaded it yet, but we have the data for it then add it
		texture = addMemTexture(url, new MobSkinMemTextureProcessor() );
	}
	if(texture != nullptr)
	{
		if (texture->loadedImage != nullptr && !texture->isLoaded)
		{
			// 4J - Disable mipmapping in general for skins & capes. Have seen problems with edge-on polys for some eg mumbo jumbo
			if( ( url.substr(0,7) == L"dlcskin" ) ||
				( url.substr(0,7) == L"dlccape" ) )
			{
				MIPMAP = false;
			}

			if (texture->id < 0)
			{
				texture->id = getTexture(texture->loadedImage, C4JRender::TEXTURE_FORMAT_RxGyBzAw, MIPMAP);
			}
			else
			{
				loadTexture(texture->loadedImage, texture->id);
			}
			texture->isLoaded = true;
			MIPMAP = true;
		}
	}
	if (texture == nullptr || texture->id < 0)
	{
		if (backup.empty() ) return -1;
		return loadTexture(TN_COUNT,backup);
	}
	return texture->id;
}

int Textures::loadMemTexture(const wstring& url, int backup)
{
	MemTexture *texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end())
	{
		texture = (*it).second;
	}
	if(texture == nullptr && app.IsFileInMemoryTextures(url))
	{
		// If we haven't loaded it yet, but we have the data for it then add it
		texture = addMemTexture(url, new MobSkinMemTextureProcessor() );
	}
	if(texture != nullptr)
	{
		texture->ticksSinceLastUse = 0;
		if (texture->loadedImage != nullptr && !texture->isLoaded)
		{
			// 4J - Disable mipmapping in general for skins & capes. Have seen problems with edge-on polys for some eg mumbo jumbo
			if( ( url.substr(0,7) == L"dlcskin" ) ||
				( url.substr(0,7) == L"dlccape" ) )
			{
				MIPMAP = false;
			}
			if (texture->id < 0)
			{
				texture->id = getTexture(texture->loadedImage, C4JRender::TEXTURE_FORMAT_RxGyBzAw, MIPMAP);
			}
			else
			{
				loadTexture(texture->loadedImage, texture->id);
			}
			texture->isLoaded = true;
			MIPMAP = true;
		}
	}
	if (texture == nullptr || texture->id < 0)
	{
		return loadTexture(backup);
	}
	return texture->id;
}

MemTexture *Textures::addMemTexture(const wstring& name,MemTextureProcessor *processor)
{
	MemTexture *texture = nullptr;
    auto it = memTextures.find(name);
    if (it != memTextures.end())
	{
		texture = (*it).second;
	}
	if(texture == nullptr)
	{
		// can we find it in the app mem files?
		PBYTE pbData=nullptr;
		DWORD dwBytes=0;
		app.GetMemFileDetails(name,&pbData,&dwBytes);

		if(dwBytes!=0)
		{
			texture = new MemTexture(name, pbData, dwBytes, processor);
			memTextures[name] = texture;
		}
		else
		{
			// 4J Stu - Make an entry for this anyway and we can populate it later
			memTextures[name] = nullptr;
		}
	}
	else
	{
		texture->count++;
	}

	delete processor;

	return texture;
}

// MemTexture *Textures::getMemTexture(const wstring& url, MemTextureProcessor *processor)
// {
// 	MemTexture *texture = memTextures[url];
// 	if (texture != nullptr)
// 	{
// 		texture->count++;
// 	}
// 	return texture;
// }

void Textures::removeMemTexture(const wstring& url)
{
	MemTexture *texture = nullptr;
    auto it = memTextures.find(url);
    if (it != memTextures.end())
	{
		texture = (*it).second;

		// If it's nullptr then we should just remove the entry
		if( texture == nullptr ) memTextures.erase(url);
	}
	if(texture != nullptr)
	{
		texture->count--;
		if (texture->count == 0)
		{
			if (texture->id >= 0) releaseTexture(texture->id);
			memTextures.erase(url);
			delete texture;
		}
	}
}

void Textures::tick(bool updateTextures, bool tickDynamics)	// 4J added updateTextures parameter & tickDynamics
{
	MemSect(22);
	if(tickDynamics)
	{
		// 4J - added - if we aren't updating the final renderer textures, just tick each of the dynamic textures instead. This is used so that in frames were we have multiple
		// ticks due to framerate compensation, that we don't lock the renderer textures twice needlessly and force the CPU to sync with the GPU.
		if( !updateTextures )
		{
			MemSect(0);
			return;
		}

		// 4J - added - tell renderer that we're about to do a block of dynamic texture updates, so we can unlock the resources after they are done rather than a series of locks/unlocks
		//RenderManager.TextureDynamicUpdateStart();
		terrain->cycleAnimationFrames();
        items->cycleAnimationFrames();
		//RenderManager.TextureDynamicUpdateEnd();	// 4J added - see comment above
	}

	// 4J - go over all the memory textures once per frame, and free any that haven't been used for a while. Ones that are being used will
	// have their ticksSinceLastUse reset in Textures::loadMemTexture.
    for (auto it = memTextures.begin(); it != memTextures.end();)
    {
		MemTexture *tex = it->second;

		if( tex  && ( ++tex->ticksSinceLastUse > MemTexture::UNUSED_TICKS_TO_FREE ) )
		{
			if (tex->id >= 0) releaseTexture(tex->id);
			delete tex;
			it = memTextures.erase(it);
		}
		else
		{
			it++;
		}

	}
	MemSect(0);
}

void Textures::reloadAll()
{
    TexturePack *skin = skins->getSelected();

	for( int i = 0; i < TN_COUNT - 2; i++ )
	{
		releaseTexture(preLoadedIdx[i]);
	}

	idMap.clear();
	loadedImages.clear();

	loadIndexedTextures();

	pixelsMap.clear();
	// 4J Stu - These are not used any more
	//WaterColor::init(loadTexturePixels(L"misc/watercolor.png"));
	//GrassColor::init(loadTexturePixels(L"misc/grasscolor.png"));
	//FoliageColor::init(loadTexturePixels(L"misc/foliagecolor.png"));

	stitch();

	skins->clearInvalidTexturePacks();

#if 0
	for(auto it = loadedImages.begin(); it != loadedImages.end(); it++ )
	{
        BufferedImage *image = it->second;
        loadTexture(image, it->first);
    }

	for(auto it = httpTextures.begin(); it != httpTextures.end(); it++ )
	{
		it->second->isLoaded = false;
	}

	for(auto it = memTextures.begin(); it != memTextures.end(); it++ )
	{
		it->second->isLoaded = false;
	}


	for( auto it = idMap.begin(); it != idMap.end(); it++ )
	{
		wstring name = it->first;

		int id = idMap[name];
		BufferedImage *image;

		wstring prefix = L"%blur%";
		bool blur = name.substr(0, prefix.size()).compare(prefix) == 0; //name.startsWith("%blur%");
		if (blur) name = name.substr(6);

		prefix = L"%clamp%";
		bool clamp = name.substr(0, prefix.size()).compare(prefix) == 0; //name.startsWith("%clamp%");
		if (clamp) name = name.substr(7);

		image = readImage(skin->getResource(name));

		loadTexture(image, id, blur, clamp);
		delete image;
	}
	for( auto it = pixelsMap.begin(); it != pixelsMap.end(); it++ )
	{
		wstring name = it->first;
		BufferedImage *image = readImage(skin->getResource(name));

		loadTexturePixels(image, pixelsMap[name]);
		delete image;
	}
#endif

	// Recalculate fonts
	//Minecraft::GetInstance()->font->loadCharacterWidths();
	//Minecraft::GetInstance()->altFont->loadCharacterWidths();
}

void Textures::stitch()
{
	terrain->stitch();
	items->stitch();
}

Icon *Textures::getMissingIcon(int type)
{
	switch (type)
	{
	case Icon::TYPE_ITEM:
	default:
		return items->getMissingIcon();
	case Icon::TYPE_TERRAIN:
		return terrain->getMissingIcon();
	}
}

BufferedImage *Textures::readImage(TEXTURE_NAME texId, const wstring& name)	// 4J was InputStream *in
{
	BufferedImage *img=nullptr;
	MemSect(32);
	// is this image one of the Title Update ones?
	bool isTu = IsTUImage(texId, name);
	wstring drive = L"";

	if(!skins->isUsingDefaultSkin() && skins->getSelected()->hasFile(L"res/" + name,false))
	{
		drive = skins->getSelected()->getPath(isTu);
		img = skins->getSelected()->getImageResource(name, false, isTu, drive); //new BufferedImage(name,false,isTu,drive);
	}
	else
	{
		const char *pchName=wstringtofilename(name);
#ifdef __PS3__
		if(app.GetBootedFromDiscPatch() && app.IsFileInPatchList(pchName))
		{
			char *pchUsrDir = app.GetBDUsrDirPath(pchName);
			wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

			if(isTu)
			{
				drive= wstr + L"\\Common\\res\\TitleUpdate\\";

			}
			else
			{
				drive= wstr + L"\\Common\\";
			}
		}
		else
#endif
		{
			drive = skins->getDefault()->getPath(isTu);
		}

		const char *pchDrive=wstringtofilename(drive);

		if(IsOriginalImage(texId, name) || isTu)
		{
			img = skins->getDefault()->getImageResource(name,false,isTu,drive); //new BufferedImage(name,false,isTu,drive);
		}
		else
		{
			img = skins->getDefault()->getImageResource(L"1_2_2/" + name, false, isTu, drive); //new BufferedImage(L"/1_2_2" + name,false,isTu,drive);
		}
	}

	MemSect(0);
    return img;
}

// Match the preload images from their enum to avoid a ton of string comparisons
TEXTURE_NAME TUImages[] =
{
	TN_POWERED_CREEPER,
	TN_MOB_ENDERMAN_EYES,
	TN_MISC_EXPLOSION,
	TN_MOB_ZOMBIE,
	TN_MISC_FOOTSTEP,
	TN_MOB_RED_COW,
	TN_MOB_SNOWMAN,
	TN_MOB_ENDERDRAGON,
	TN_MOB_VILLAGER_VILLAGER,
	TN_MOB_VILLAGER_FARMER,
	TN_MOB_VILLAGER_LIBRARIAN,
	TN_MOB_VILLAGER_PRIEST,
	TN_MOB_VILLAGER_SMITH,
	TN_MOB_VILLAGER_BUTCHER,
	TN_MOB_ENDERDRAGON_ENDEREYES,
	TN__BLUR__MISC_GLINT,
	TN_ITEM_BOOK,
	TN_MISC_PARTICLEFIELD,

	// TU9
	TN_MISC_TUNNEL,
	TN_MOB_ENDERDRAGON_BEAM,
	TN_GUI_ITEMS,
	TN_TERRAIN,
	TN_MISC_MAPICONS,

	// TU12
	TN_MOB_WITHER_SKELETON,

	// TU14
	TN_TILE_ENDER_CHEST,
	TN_ART_KZ,
	TN_MOB_WOLF_TAME,
	TN_MOB_WOLF_COLLAR,
	TN_PARTICLES,
	TN_MOB_ZOMBIE_VILLAGER,

	TN_ITEM_LEASHKNOT,

	TN_MISC_BEACON_BEAM,

	TN_MOB_BAT,

	TN_MOB_DONKEY,
	TN_MOB_HORSE_BLACK,
	TN_MOB_HORSE_BROWN,
	TN_MOB_HORSE_CHESTNUT,
	TN_MOB_HORSE_CREAMY,
	TN_MOB_HORSE_DARKBROWN,
	TN_MOB_HORSE_GRAY,
	TN_MOB_HORSE_MARKINGS_BLACKDOTS,
	TN_MOB_HORSE_MARKINGS_WHITE,
	TN_MOB_HORSE_MARKINGS_WHITEDOTS,
	TN_MOB_HORSE_MARKINGS_WHITEFIELD,
	TN_MOB_HORSE_SKELETON,
	TN_MOB_HORSE_WHITE,
	TN_MOB_HORSE_ZOMBIE,
	TN_MOB_MULE,
	TN_MOB_HORSE_ARMOR_DIAMOND,
	TN_MOB_HORSE_ARMOR_GOLD,
	TN_MOB_HORSE_ARMOR_IRON,

	TN_MOB_WITCH,

	TN_MOB_WITHER,
	TN_MOB_WITHER_ARMOR,
	TN_MOB_WITHER_INVULNERABLE,

	TN_TILE_TRAP_CHEST,
	TN_TILE_LARGE_TRAP_CHEST,
	//TN_TILE_XMAS_CHEST,
	//TN_TILE_LARGE_XMAS_CHEST,

#ifdef _LARGE_WORLDS
	TN_MISC_ADDITIONALMAPICONS,
#endif

	// TU17
	TN_DEFAULT_FONT,
	// TN_ALT_FONT, // Not in TU yet

	TN_COUNT // Why is this here?
};

// This is for any TU textures that aren't part of our enum indexed preload set
const wchar_t *TUImagePaths[] =
{
	L"font/Default",
	L"font/Mojangles_7",
	L"font/Mojangles_11",

	// TU12
	L"armor/cloth_1.png",
	L"armor/cloth_1_b.png",
	L"armor/cloth_2.png",
	L"armor/cloth_2_b.png",

	//

	nullptr
};

bool Textures::IsTUImage(TEXTURE_NAME texId, const wstring& name)
{
	int i = 0;
	if(texId < TN_COUNT)
	{
		while(TUImages[i] < TN_COUNT)
		{
			if(texId == TUImages[i])
			{
				return true;
			}
			i++;
		}
	}
	i = 0;
	while(TUImagePaths[i])
	{
		if(name.compare(TUImagePaths[i])==0)
		{
			return true;
		}
		i++;
	}
	return false;
}

TEXTURE_NAME OriginalImages[] =
{
	TN_MOB_CHAR,
	TN_MOB_CHAR1,
	TN_MOB_CHAR2,
	TN_MOB_CHAR3,
	TN_MOB_CHAR4,
	TN_MOB_CHAR5,
	TN_MOB_CHAR6,
	TN_MOB_CHAR7,

	TN_MISC_MAPBG,

	TN_COUNT
};

const wchar_t *OriginalImagesPaths[] =
{
	L"misc/watercolor.png",

	nullptr
};

bool Textures::IsOriginalImage(TEXTURE_NAME texId, const wstring& name)
{
	int i = 0;
	if(texId < TN_COUNT)
	{
		while(OriginalImages[i] < TN_COUNT)
		{
			if(texId == OriginalImages[i])
			{
				return true;
			}
			i++;
		}
	}
	i = 0;
	while(OriginalImagesPaths[i])
	{
		if(name.compare(OriginalImagesPaths[i])==0)
		{
			return true;
		}
		i++;
	}
	return false;
}

