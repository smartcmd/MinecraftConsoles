#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/ByteBuffer.h"
#include "Minecraft.h"
#include "LevelRenderer.h"
#include "EntityRenderDispatcher.h"
#include "Stitcher.h"
#include "StitchSlot.h"
#include "StitchedTexture.h"
#include "Texture.h"
#include "TextureHolder.h"
#include "TextureManager.h"
#include "TexturePack.h"
#include "TexturePackRepository.h"
#include "PreStitchedTextureMap.h"
#include "SimpleIcon.h"
#include "CompassTexture.h"
#include "ClockTexture.h"

const wstring PreStitchedTextureMap::NAME_MISSING_TEXTURE = L"missingno";

PreStitchedTextureMap::PreStitchedTextureMap(int type, const wstring &name, const wstring &path, BufferedImage *missingTexture, bool mipmap) : iconType(type), name(name), path(path), extension(L".png")
{
	this->missingTexture = missingTexture;

	// 4J Initialisers
	missingPosition = nullptr;
	stitchResult = nullptr;

	m_mipMap = mipmap;
	missingPosition = static_cast<StitchedTexture *>(new SimpleIcon(NAME_MISSING_TEXTURE, NAME_MISSING_TEXTURE, 0, 0, 1, 1));
}

void PreStitchedTextureMap::stitch()
{
	// Animated StitchedTextures store a vector of textures for each frame of the animation. Free any pre-existing ones here.
	for(StitchedTexture *animatedStitchedTexture : animatedTextures)
	{
		animatedStitchedTexture->freeFrameTextures();
	}

	loadUVs();

	if (iconType == Icon::TYPE_TERRAIN)
	{
		//for (Tile tile : Tile.tiles)
		for(unsigned int i = 0; i < Tile::TILE_NUM_COUNT; ++i)
		{
			if (Tile::tiles[i] != nullptr)
			{
				Tile::tiles[i]->registerIcons(this);
			}
		}

		Minecraft::GetInstance()->levelRenderer->registerTextures(this);
		EntityRenderDispatcher::instance->registerTerrainTextures(this);
	}

	//for (Item item : Item.items)
	for(unsigned int i = 0; i < Item::ITEM_NUM_COUNT; ++i)
	{
		Item *item = Item::items[i];
		if (item != nullptr && item->getIconType() == iconType)
		{
			item->registerIcons(this);
		}
	}

	// Collection bucket for multiple frames per texture
	unordered_map<TextureHolder *, vector<Texture *> * > textures; // = new HashMap<TextureHolder, List<Texture>>();

	Stitcher *stitcher = TextureManager::getInstance()->createStitcher(name);

	animatedTextures.clear();

	// Create the final image
	wstring filename = name + extension;

	TexturePack *texturePack = Minecraft::GetInstance()->skins->getSelected();
	//try {
	int mode = Texture::TM_DYNAMIC;
	int clamp = Texture::WM_WRAP; // 4J Stu - Don't clamp as it causes issues with how we signal non-mipmmapped textures to the pixel shader //Texture::WM_CLAMP;
	int minFilter = Texture::TFLT_NEAREST;
	int magFilter = Texture::TFLT_NEAREST;

	MemSect(32);
	wstring drive = L"";

	// 4J-PB - need to check for BD patched files
#ifdef __PS3__
	const char *pchName=wstringtofilename(filename);
	if(app.GetBootedFromDiscPatch() && app.IsFileInPatchList(pchName))
	{
		if(texturePack->hasFile(L"res/" + filename,false))
		{
			drive = texturePack->getPath(true,pchName);
		}
		else
		{
			drive = Minecraft::GetInstance()->skins->getDefault()->getPath(true,pchName);
			texturePack = Minecraft::GetInstance()->skins->getDefault();
		}
	}
	else
#endif
	if(texturePack->hasFile(L"res/" + filename,false))
	{
		drive = texturePack->getPath(true);
	}
	else
	{
		drive = Minecraft::GetInstance()->skins->getDefault()->getPath(true);
		texturePack = Minecraft::GetInstance()->skins->getDefault();
	}

	//BufferedImage *image = new BufferedImage(texturePack->getResource(L"/" + filename),false,true,drive); //ImageIO::read(texturePack->getResource(L"/" + filename));
	BufferedImage *image = texturePack->getImageResource(filename, false, true, drive);
	MemSect(0);
	int height = image->getHeight();
	int width = image->getWidth();

	if(stitchResult != nullptr)
	{
		TextureManager::getInstance()->unregisterTexture(name, stitchResult);
		delete stitchResult;
	}
	stitchResult = TextureManager::getInstance()->createTexture(name, Texture::TM_DYNAMIC, width, height, Texture::TFMT_RGBA, m_mipMap);
	stitchResult->transferFromImage(image);
	delete image;
	TextureManager::getInstance()->registerName(name, stitchResult);
	//stitchResult = stitcher->constructTexture(m_mipMap);

	for(auto & it : texturesByName)
	{
		auto *preStitched = static_cast<StitchedTexture *>(it.second);

		int x = preStitched->getU0() * stitchResult->getWidth();
		int y = preStitched->getV0() * stitchResult->getHeight();
		int width = (preStitched->getU1() * stitchResult->getWidth()) - x;
		int height = (preStitched->getV1() * stitchResult->getHeight()) - y;

		preStitched->init(stitchResult, nullptr, x, y, width, height, false);
	}

	MemSect(52);
	for(auto& it : texturesByName)
	{
		auto *preStitched = static_cast<StitchedTexture *>(it.second);

		makeTextureAnimated(texturePack, preStitched);
	}
	MemSect(0);
	//missingPosition = (StitchedTexture *)texturesByName.find(NAME_MISSING_TEXTURE)->second;

	stitchResult->writeAsPNG(L"debug.stitched_" + name + L".png");
	stitchResult->updateOnGPU();


#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita so we mark which icons actually require it
	DWORD *data = (DWORD*) this->getStitchedTexture()->getData()->getBuffer();
	int Width = this->getStitchedTexture()->getWidth();
	int Height = this->getStitchedTexture()->getHeight();
	for( auto it = texturesByName.begin(); it != texturesByName.end(); ++it)
	{
		StitchedTexture *preStitched = (StitchedTexture *)it->second;

		bool Found = false;
		int u0 = preStitched->getU0() * Width;
		int u1 = preStitched->getU1() * Width;
		int v0 = preStitched->getV0() * Height;
		int v1 = preStitched->getV1() * Height;

		// check all the texels for this icon. If ANY are transparent we mark it as 'cut out'
		for( int v = v0;v < v1; v+= 1 )
		{
			for( int u = u0;u < u1; u+= 1 )
			{
				// is this texel alpha value < 0.1
				if( (data[v * Width + u] & 0xff000000) < 0x20000000 )
				{
					// this texel is transparent. Mark the icon as such and bail
					preStitched->setFlags(Icon::IS_ALPHA_CUT_OUT);
					Found = true;
					break;
				}
			}

			if( Found )
			{
				// move onto the next icon
				break;
			}
		}
	}
#endif
}

void PreStitchedTextureMap::makeTextureAnimated(TexturePack *texturePack, StitchedTexture *tex)
{
	if(!tex->hasOwnData())
	{
		animatedTextures.push_back(tex);
		return;
	}

	wstring textureFileName = tex->m_fileName;

	wstring animString = texturePack->getAnimationString(textureFileName, path, true);

	if(!animString.empty())
	{
		wstring filename = path + textureFileName + extension;

		// TODO: [EB] Put the frames into a proper object, not this inside out hack
		vector<Texture *> *frames = TextureManager::getInstance()->createTextures(filename, m_mipMap);
		if (frames == nullptr || frames->empty())
		{
			return; // Couldn't load a texture, skip it
		}

		Texture *first = frames->at(0);

#ifndef _CONTENT_PACKAGE
		if(first->getWidth() != tex->getWidth() || first->getHeight() != tex->getHeight())
		{
			app.DebugPrintf("%ls - first w - %d, h - %d, tex w - %d, h - %d\n",textureFileName.c_str(),first->getWidth(),tex->getWidth(),first->getHeight(),tex->getHeight());
			//__debugbreak();
		}
#endif

		tex->init(stitchResult, frames, tex->getX(), tex->getY(), first->getWidth(), first->getHeight(), false);

		if (frames->size() > 1)
		{
			animatedTextures.push_back(tex);

			tex->loadAnimationFrames(animString);
		}
	}
}

StitchedTexture *PreStitchedTextureMap::getTexture(const wstring &name)
{
#ifndef _CONTENT_PACKAGE
	app.DebugPrintf("Not implemented!\n");
	__debugbreak();
#endif
	return nullptr;
#if 0
	StitchedTexture *result = texturesByName.find(name)->second;
	if (result == nullptr) result = missingPosition;
	return result;
#endif
}

void PreStitchedTextureMap::cycleAnimationFrames()
{
	for(StitchedTexture* texture : animatedTextures)
	{
		texture->cycleFrames();
	}
}

Texture *PreStitchedTextureMap::getStitchedTexture()
{
	return stitchResult;
}

// 4J Stu - register is a reserved keyword in C++
Icon *PreStitchedTextureMap::registerIcon(const wstring &name)
{
	Icon *result = nullptr;
	if (name.empty())
	{
		app.DebugPrintf("Don't register nullptr\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		result = missingPosition;
		//new RuntimeException("Don't register null!").printStackTrace();
	}

    auto it = texturesByName.find(name);
    if(it != texturesByName.end()) result = it->second;

	if (result == nullptr)
	{
#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("Could not find uv data for icon %ls\n", name.c_str() );
		__debugbreak();
#endif
		result = missingPosition;
	}

	return result;
}

int PreStitchedTextureMap::getIconType()
{
	return iconType;
}

Icon *PreStitchedTextureMap::getMissingIcon()
{
	return missingPosition;
}

#define ADD_ICON(row, column, name) (texturesByName[name] =	new SimpleIcon(name,name,horizRatio*column,vertRatio*row,horizRatio*(column+1),vertRatio*(row+1)));
#define ADD_ICON_WITH_NAME(row, column, name, filename) (texturesByName[name] =	new SimpleIcon(name,filename,horizRatio*column,vertRatio*row,horizRatio*(column+1),vertRatio*(row+1)));
#define ADD_ICON_SIZE(row, column, name, height, width) (texturesByName[name] =	new SimpleIcon(name,name,horizRatio*column,vertRatio*row,horizRatio*(column+width),vertRatio*(row+height)));

void PreStitchedTextureMap::loadUVs()
{
	if(!texturesByName.empty())
	{
		// 4J Stu - We only need to populate this once at the moment as we have hardcoded positions for each texture
		// If we ever load that dynamically, be aware that the Icon objects could currently be being used by the
		// GameRenderer::runUpdate thread
		return;
	}

	for(auto& it : texturesByName)
	{
		delete it.second;
	}
	texturesByName.clear();

	if(iconType != Icon::TYPE_TERRAIN)
	{
		float horizRatio = 1.0f/16.0f;
		float vertRatio = 1.0f/16.0f;

		ADD_ICON(0,		0,	L"helmetCloth")
		ADD_ICON(0,		1,	L"helmetChain")
		ADD_ICON(0,		2,	L"helmetIron")
		ADD_ICON(0,		3,	L"helmetDiamond")
		ADD_ICON(0,		4,	L"helmetGold")
		ADD_ICON(0,		5,	L"flintAndSteel")
		ADD_ICON(0,		6,	L"flint")
		ADD_ICON(0,		7,	L"coal")
		ADD_ICON(0,		8,	L"string")
		ADD_ICON(0,		9,	L"seeds")
		ADD_ICON(0,		10,	L"apple")
		ADD_ICON(0,		11,	L"appleGold")
		ADD_ICON(0,		12,	L"egg")
		ADD_ICON(0,		13,	L"sugar")
		ADD_ICON(0,		14,	L"snowball")
		ADD_ICON(0,		15,	L"slot_empty_helmet")

		ADD_ICON(1,		0,	L"chestplateCloth")
		ADD_ICON(1,		1,	L"chestplateChain")
		ADD_ICON(1,		2,	L"chestplateIron")
		ADD_ICON(1,		3,	L"chestplateDiamond")
		ADD_ICON(1,		4,	L"chestplateGold")
		ADD_ICON(1,		5,	L"bow")
		ADD_ICON(1,		6,	L"brick")
		ADD_ICON(1,		7,	L"ingotIron")
		ADD_ICON(1,		8,	L"feather")
		ADD_ICON(1,		9,	L"wheat")
		ADD_ICON(1,		10,	L"painting")
		ADD_ICON(1,		11,	L"reeds")
		ADD_ICON(1,		12,	L"bone")
		ADD_ICON(1,		13,	L"cake")
		ADD_ICON(1,		14,	L"slimeball")
		ADD_ICON(1,		15,	L"slot_empty_chestplate")

		ADD_ICON(2,		0,	L"leggingsCloth")
		ADD_ICON(2,		1,	L"leggingsChain")
		ADD_ICON(2,		2,	L"leggingsIron")
		ADD_ICON(2,		3,	L"leggingsDiamond")
		ADD_ICON(2,		4,	L"leggingsGold")
		ADD_ICON(2,		5,	L"arrow")
		ADD_ICON(2,		6,	L"quiver")
		ADD_ICON(2,		7,	L"ingotGold")
		ADD_ICON(2,		8,	L"sulphur")
		ADD_ICON(2,		9,	L"bread")
		ADD_ICON(2,		10,	L"sign")
		ADD_ICON(2,		11,	L"doorWood")
		ADD_ICON(2,		12,	L"doorIron")
		ADD_ICON(2,		13,	L"bed")
		ADD_ICON(2,		14,	L"fireball")
		ADD_ICON(2,		15,	L"slot_empty_leggings")

		ADD_ICON(3,		0,	L"bootsCloth")
		ADD_ICON(3,		1,	L"bootsChain")
		ADD_ICON(3,		2,	L"bootsIron")
		ADD_ICON(3,		3,	L"bootsDiamond")
		ADD_ICON(3,		4,	L"bootsGold")
		ADD_ICON(3,		5,	L"stick")
		ADD_ICON(3,		6,	L"compass")
		ADD_ICON(3,		7,	L"diamond")
		ADD_ICON(3,		8,	L"redstone")
		ADD_ICON(3,		9,	L"clay")
		ADD_ICON(3,		10,	L"paper")
		ADD_ICON(3,		11,	L"book")
		ADD_ICON(3,		12,	L"map")
		ADD_ICON(3,		13,	L"seeds_pumpkin")
		ADD_ICON(3,		14,	L"seeds_melon")
		ADD_ICON(3,		15,	L"slot_empty_boots")

		ADD_ICON(4,		0,	L"swordWood")
		ADD_ICON(4,		1,	L"swordStone")
		ADD_ICON(4,		2,	L"swordIron")
		ADD_ICON(4,		3,	L"swordDiamond")
		ADD_ICON(4,		4,	L"swordGold")
		ADD_ICON(4,		5,	L"fishingRod_uncast")
		ADD_ICON(4,		6,	L"clock")
		ADD_ICON(4,		7,	L"bowl")
		ADD_ICON(4,		8,	L"mushroomStew")
		ADD_ICON(4,		9,	L"yellowDust")
		ADD_ICON(4,		10,	L"bucket")
		ADD_ICON(4,		11,	L"bucketWater")
		ADD_ICON(4,		12,	L"bucketLava")
		ADD_ICON(4,		13,	L"milk")
		ADD_ICON(4,		14,	L"dyePowder_black")
		ADD_ICON(4,		15,	L"dyePowder_gray")

		ADD_ICON(5,		0,	L"shovelWood")
		ADD_ICON(5,		1,	L"shovelStone")
		ADD_ICON(5,		2,	L"shovelIron")
		ADD_ICON(5,		3,	L"shovelDiamond")
		ADD_ICON(5,		4,	L"shovelGold")
		ADD_ICON(5,		5,	L"fishingRod_cast")
		ADD_ICON(5,		6,	L"diode")
		ADD_ICON(5,		7,	L"porkchopRaw")
		ADD_ICON(5,		8,	L"porkchopCooked")
		ADD_ICON(5,		9,	L"fishRaw")
		ADD_ICON(5,		10,	L"fishCooked")
		ADD_ICON(5,		11,	L"rottenFlesh")
		ADD_ICON(5,		12,	L"cookie")
		ADD_ICON(5,		13,	L"shears")
		ADD_ICON(5,		14,	L"dyePowder_red")
		ADD_ICON(5,		15,	L"dyePowder_pink")

		ADD_ICON(6,		0,	L"pickaxeWood")
		ADD_ICON(6,		1,	L"pickaxeStone")
		ADD_ICON(6,		2,	L"pickaxeIron")
		ADD_ICON(6,		3,	L"pickaxeDiamond")
		ADD_ICON(6,		4,	L"pickaxeGold")
		ADD_ICON(6,		5,	L"bow_pull_0")
		ADD_ICON(6,		6,	L"carrotOnAStick")
		ADD_ICON(6,		7,	L"leather")
		ADD_ICON(6,		8,	L"saddle")
		ADD_ICON(6,		9,	L"beefRaw")
		ADD_ICON(6,		10,	L"beefCooked")
		ADD_ICON(6,		11,	L"enderPearl")
		ADD_ICON(6,		12,	L"blazeRod")
		ADD_ICON(6,		13,	L"melon")
		ADD_ICON(6,		14,	L"dyePowder_green")
		ADD_ICON(6,		15,	L"dyePowder_lime")

		ADD_ICON(7,		0,	L"hatchetWood")
		ADD_ICON(7,		1,	L"hatchetStone")
		ADD_ICON(7,		2,	L"hatchetIron")
		ADD_ICON(7,		3,	L"hatchetDiamond")
		ADD_ICON(7,		4,	L"hatchetGold")
		ADD_ICON(7,		5,	L"bow_pull_1")
		ADD_ICON(7,		6,	L"potatoBaked")
		ADD_ICON(7,		7,	L"potato")
		ADD_ICON(7,		8,	L"carrots")
		ADD_ICON(7,		9,	L"chickenRaw")
		ADD_ICON(7,		10,	L"chickenCooked")
		ADD_ICON(7,		11,	L"ghastTear")
		ADD_ICON(7,		12,	L"goldNugget")
		ADD_ICON(7,		13,	L"netherStalkSeeds")
		ADD_ICON(7,		14,	L"dyePowder_brown")
		ADD_ICON(7,		15,	L"dyePowder_yellow")

		ADD_ICON(8,		0,	L"hoeWood")
		ADD_ICON(8,		1,	L"hoeStone")
		ADD_ICON(8,		2,	L"hoeIron")
		ADD_ICON(8,		3,	L"hoeDiamond")
		ADD_ICON(8,		4,	L"hoeGold")
		ADD_ICON(8,		5,	L"bow_pull_2")
		ADD_ICON(8,		6,	L"potatoPoisonous")
		ADD_ICON(8,		7,	L"minecart")
		ADD_ICON(8,		8,	L"boat")
		ADD_ICON(8,		9,	L"speckledMelon")
		ADD_ICON(8,		10,	L"fermentedSpiderEye")
		ADD_ICON(8,		11,	L"spiderEye")
		ADD_ICON(8,		12,	L"potion")
		ADD_ICON(8,		12,	L"glassBottle") // Same as potion
		ADD_ICON(8,		13,	L"potion_contents")
		ADD_ICON(8,		14,	L"dyePowder_blue")
		ADD_ICON(8,		15,	L"dyePowder_light_blue")

		ADD_ICON(9,		0,	L"helmetCloth_overlay")
		//ADD_ICON(9,		1,	L"unused")
		ADD_ICON(9,		2,	L"iron_horse_armor")
		ADD_ICON(9,		3,	L"diamond_horse_armor")
		ADD_ICON(9,		4,	L"gold_horse_armor")
		ADD_ICON(9,		5,	L"comparator")
		ADD_ICON(9,		6,	L"carrotGolden")
		ADD_ICON(9,		7,	L"minecart_chest")
		ADD_ICON(9,		8,	L"pumpkinPie")
		ADD_ICON(9,		9,	L"monsterPlacer")
		ADD_ICON(9,		10,	L"potion_splash")
		ADD_ICON(9,		11,	L"eyeOfEnder")
		ADD_ICON(9,		12,	L"cauldron")
		ADD_ICON(9,		13,	L"blazePowder")
		ADD_ICON(9,		14,	L"dyePowder_purple")
		ADD_ICON(9,		15,	L"dyePowder_magenta")

		ADD_ICON(10,	0,	L"chestplateCloth_overlay")
		//ADD_ICON(10,	1,	L"unused")
		//ADD_ICON(10,	2,	L"unused")
		ADD_ICON(10,	3,	L"name_tag")
		ADD_ICON(10,	4,	L"lead")
		ADD_ICON(10,	5,	L"netherbrick")
		//ADD_ICON(10,	6,	L"unused")
		ADD_ICON(10,	7,	L"minecart_furnace")
		ADD_ICON(10,	8,	L"charcoal")
		ADD_ICON(10,	9,	L"monsterPlacer_overlay")
		ADD_ICON(10,	10,	L"ruby")
		ADD_ICON(10,	11,	L"expBottle")
		ADD_ICON(10,	12,	L"brewingStand")
		ADD_ICON(10,	13,	L"magmaCream")
		ADD_ICON(10,	14,	L"dyePowder_cyan")
		ADD_ICON(10,	15,	L"dyePowder_orange")

		ADD_ICON(11,	0,	L"leggingsCloth_overlay")
		//ADD_ICON(11,	1,	L"unused")
		//ADD_ICON(11,	2,	L"unused")
		//ADD_ICON(11,	3,	L"unused")
		//ADD_ICON(11,	4,	L"unused")
		//ADD_ICON(11,	5,	L"unused")
		//ADD_ICON(11,	6,	L"unused")
		ADD_ICON(11,	7,	L"minecart_hopper")
		ADD_ICON(11,	8,	L"hopper")
		ADD_ICON(11,	9,	L"nether_star")
		ADD_ICON(11,	10,	L"emerald")
		ADD_ICON(11,	11,	L"writingBook")
		ADD_ICON(11,	12,	L"writtenBook")
		ADD_ICON(11,	13,	L"flowerPot")
		ADD_ICON(11,	14,	L"dyePowder_silver")
		ADD_ICON(11,	15,	L"dyePowder_white")

		ADD_ICON(12,	0,	L"bootsCloth_overlay")
		//ADD_ICON(12,	1,	L"unused")
		//ADD_ICON(12,	2,	L"unused")
		//ADD_ICON(12,	3,	L"unused")
		//ADD_ICON(12,	4,	L"unused")
		//ADD_ICON(12,	5,	L"unused")
		//ADD_ICON(12,	6,	L"unused")
		ADD_ICON(12,	7,	L"minecart_tnt")
		//ADD_ICON(12,	8,	L"unused")
		ADD_ICON(12,	9,	L"fireworks")
		ADD_ICON(12,	10,	L"fireworks_charge")
		ADD_ICON(12,	11,	L"fireworks_charge_overlay")
		ADD_ICON(12,	12,	L"netherquartz")
		ADD_ICON(12,	13,	L"map_empty")
		ADD_ICON(12,	14,	L"frame")
		ADD_ICON(12,	15,	L"enchantedBook")

		ADD_ICON(14,	0,	L"skull_skeleton")
		ADD_ICON(14,	1,	L"skull_wither")
		ADD_ICON(14,	2,	L"skull_zombie")
		ADD_ICON(14,	3,	L"skull_char")
		ADD_ICON(14,	4,	L"skull_creeper")
		//ADD_ICON(14,	5,	L"unused")
		//ADD_ICON(14,	6,	L"unused")
		ADD_ICON_WITH_NAME(14,	7,	L"compassP0", L"compass") // 4J Added
		ADD_ICON_WITH_NAME(14,	8,	L"compassP1", L"compass") // 4J Added
		ADD_ICON_WITH_NAME(14,	9,	L"compassP2", L"compass") // 4J Added
		ADD_ICON_WITH_NAME(14,	10,	L"compassP3", L"compass") // 4J Added
		ADD_ICON_WITH_NAME(14,	11,	L"clockP0", L"clock") // 4J Added
		ADD_ICON_WITH_NAME(14,	12,	L"clockP1", L"clock") // 4J Added
		ADD_ICON_WITH_NAME(14,	13,	L"clockP2", L"clock") // 4J Added
		ADD_ICON_WITH_NAME(14,	14,	L"clockP3", L"clock") // 4J Added
		ADD_ICON(14,	15,	L"dragonFireball")

		ADD_ICON(15,		0,	L"record_13")
		ADD_ICON(15,		1,	L"record_cat")
		ADD_ICON(15,		2,	L"record_blocks")
		ADD_ICON(15,		3,	L"record_chirp")
		ADD_ICON(15,		4,	L"record_far")
		ADD_ICON(15,		5,	L"record_mall")
		ADD_ICON(15,		6,	L"record_mellohi")
		ADD_ICON(15,		7,	L"record_stal")
		ADD_ICON(15,		8,	L"record_strad")
		ADD_ICON(15,		9,	L"record_ward")
		ADD_ICON(15,		10,	L"record_11")
		ADD_ICON(15,		11,	L"record_where are we now")

		// Special cases
		ClockTexture *dataClock = new ClockTexture();
		Icon *oldClock = texturesByName[L"clock"];
		dataClock->initUVs(oldClock->getU0(), oldClock->getV0(), oldClock->getU1(), oldClock->getV1() );
		delete oldClock;
		texturesByName[L"clock"] = dataClock;

		ClockTexture *clock = new ClockTexture(0, dataClock);
		oldClock = texturesByName[L"clockP0"];
		clock->initUVs(oldClock->getU0(), oldClock->getV0(), oldClock->getU1(), oldClock->getV1() );
		delete oldClock;
		texturesByName[L"clockP0"] = clock;

		clock = new ClockTexture(1, dataClock);
		oldClock = texturesByName[L"clockP1"];
		clock->initUVs(oldClock->getU0(), oldClock->getV0(), oldClock->getU1(), oldClock->getV1() );
		delete oldClock;
		texturesByName[L"clockP1"] = clock;

		clock = new ClockTexture(2, dataClock);
		oldClock = texturesByName[L"clockP2"];
		clock->initUVs(oldClock->getU0(), oldClock->getV0(), oldClock->getU1(), oldClock->getV1() );
		delete oldClock;
		texturesByName[L"clockP2"] = clock;

		clock = new ClockTexture(3, dataClock);
		oldClock = texturesByName[L"clockP3"];
		clock->initUVs(oldClock->getU0(), oldClock->getV0(), oldClock->getU1(), oldClock->getV1() );
		delete oldClock;
		texturesByName[L"clockP3"] = clock;

		CompassTexture *dataCompass = new CompassTexture();
		Icon *oldCompass = texturesByName[L"compass"];
		dataCompass->initUVs(oldCompass->getU0(), oldCompass->getV0(), oldCompass->getU1(), oldCompass->getV1() );
		delete oldCompass;
		texturesByName[L"compass"] = dataCompass;

		CompassTexture *compass = new CompassTexture(0, dataCompass);
		oldCompass = texturesByName[L"compassP0"];
		compass->initUVs(oldCompass->getU0(), oldCompass->getV0(), oldCompass->getU1(), oldCompass->getV1() );
		delete oldCompass;
		texturesByName[L"compassP0"] = compass;

		compass = new CompassTexture(1, dataCompass);
		oldCompass = texturesByName[L"compassP1"];
		compass->initUVs(oldCompass->getU0(), oldCompass->getV0(), oldCompass->getU1(), oldCompass->getV1() );
		delete oldCompass;
		texturesByName[L"compassP1"] = compass;

		compass = new CompassTexture(2, dataCompass);
		oldCompass = texturesByName[L"compassP2"];
		compass->initUVs(oldCompass->getU0(), oldCompass->getV0(), oldCompass->getU1(), oldCompass->getV1() );
		delete oldCompass;
		texturesByName[L"compassP2"] = compass;

		compass = new CompassTexture(3, dataCompass);
		oldCompass = texturesByName[L"compassP3"];
		compass->initUVs(oldCompass->getU0(), oldCompass->getV0(), oldCompass->getU1(), oldCompass->getV1() );
		delete oldCompass;
		texturesByName[L"compassP3"] = compass;
	}
	else
	{
		float horizRatio = 1.0f/16.0f;
		float vertRatio = 1.0f/32.0f;

		ADD_ICON(0,		0,	L"grass_top")
		texturesByName[L"grass_top"]->setFlags(Icon::IS_GRASS_TOP);			// 4J added for faster determination of texture type in tesselation
		ADD_ICON(0,		1,	L"stone")
		ADD_ICON(0,		2,	L"dirt")
		ADD_ICON(0,		3,	L"grass_side")
		texturesByName[L"grass_side"]->setFlags(Icon::IS_GRASS_SIDE);		// 4J added for faster determination of texture type in tesselation
		ADD_ICON(0,		4,	L"planks_oak")
		ADD_ICON(0,		5,	L"stoneslab_side")
		ADD_ICON(0,		6,	L"stoneslab_top")
		ADD_ICON(0,		7,	L"brick")
		ADD_ICON(0,		8,	L"tnt_side")
		ADD_ICON(0,		9,	L"tnt_top")
		ADD_ICON(0,		10,	L"tnt_bottom")
		ADD_ICON(0,		11,	L"web")
		ADD_ICON(0,		12,	L"flower_rose")
		ADD_ICON(0,		13,	L"flower_dandelion")
		ADD_ICON(0,		14,	L"portal")
		ADD_ICON(0,		15,	L"sapling")

		ADD_ICON(1,		0,	L"cobblestone");
		ADD_ICON(1,		1,	L"bedrock");
		ADD_ICON(1,		2,	L"sand");
		ADD_ICON(1,		3,	L"gravel");
		ADD_ICON(1,		4,	L"log_oak");
		ADD_ICON(1,		5,	L"log_oak_top");
		ADD_ICON(1,		6,	L"iron_block");
		ADD_ICON(1,		7,	L"gold_block");
		ADD_ICON(1,		8,	L"diamond_block");
		ADD_ICON(1,		9,	L"emerald_block");
		ADD_ICON(1,		10,	L"redstone_block");
		ADD_ICON(1,		11,	L"dropper_front_horizontal");
		ADD_ICON(1,		12,	L"mushroom_red");
		ADD_ICON(1,		13,	L"mushroom_brown");
		ADD_ICON(1,		14,	L"sapling_jungle");
		ADD_ICON(1,		15,	L"fire_0");

		ADD_ICON(2,		0,	L"gold_ore");
		ADD_ICON(2,		1,	L"iron_ore");
		ADD_ICON(2,		2,	L"coal_ore");
		ADD_ICON(2,		3,	L"bookshelf");
		ADD_ICON(2,		4,	L"cobblestone_mossy");
		ADD_ICON(2,		5,	L"obsidian");
		ADD_ICON(2,		6,	L"grass_side_overlay");
		ADD_ICON(2,		7,	L"tallgrass");
		ADD_ICON(2,		8,	L"dispenser_front_vertical");
		ADD_ICON(2,		9,	L"beacon");
		ADD_ICON(2,		10,	L"dropper_front_vertical");
		ADD_ICON(2,		11,	L"workbench_top");
		ADD_ICON(2,		12,	L"furnace_front");
		ADD_ICON(2,		13,	L"furnace_side");
		ADD_ICON(2,		14,	L"dispenser_front");
		ADD_ICON(2,		15,	L"fire_1");

		ADD_ICON(3,		0,	L"sponge");
		ADD_ICON(3,		1,	L"glass");
		ADD_ICON(3,		2,	L"diamond_ore");
		ADD_ICON(3,		3,	L"redstone_ore");
		ADD_ICON(3,		4,	L"leaves");
		ADD_ICON(3,		5,	L"leaves_opaque");
		ADD_ICON(3,		6,	L"stonebrick");
		ADD_ICON(3,		7,	L"deadbush");
		ADD_ICON(3,		8,	L"fern");
		ADD_ICON(3,		9,	L"daylight_detector_top");
		ADD_ICON(3,		10,	L"daylight_detector_side");
		ADD_ICON(3,		11,	L"workbench_side");
		ADD_ICON(3,		12,	L"workbench_front");
		ADD_ICON(3,		13,	L"furnace_front_lit");
		ADD_ICON(3,		14,	L"furnace_top");
		ADD_ICON(3,		15,	L"sapling_spruce");

		ADD_ICON(4,		0,	L"wool_colored_white");
		ADD_ICON(4,		1,	L"mob_spawner");
		ADD_ICON(4,		2,	L"snow");
		ADD_ICON(4,		3,	L"ice");
		ADD_ICON(4,		4,	L"snow_side");
		ADD_ICON(4,		5,	L"cactus_top");
		ADD_ICON(4,		6,	L"cactus_side");
		ADD_ICON(4,		7,	L"cactus_bottom");
		ADD_ICON(4,		8,	L"clay");
		ADD_ICON(4,		9,	L"reeds");
		ADD_ICON(4,		10,	L"jukebox_side");
		ADD_ICON(4,		11,	L"jukebox_top");
		ADD_ICON(4,		12,	L"waterlily");
		ADD_ICON(4,		13,	L"mycel_side");
		ADD_ICON(4,		14,	L"mycel_top");
		ADD_ICON(4,		15,	L"sapling_birch");

		ADD_ICON(5,		0,	L"torch_on");
		ADD_ICON(5,		1,	L"door_wood_upper");
		ADD_ICON(5,		2,	L"door_iron_upper");
		ADD_ICON(5,		3,	L"ladder");
		ADD_ICON(5,		4,	L"trapdoor");
		ADD_ICON(5,		5,	L"iron_bars");
		ADD_ICON(5,		6,	L"farmland_wet");
		ADD_ICON(5,		7,	L"farmland_dry");
		ADD_ICON(5,		8,	L"crops_0");
		ADD_ICON(5,		9,	L"crops_1");
		ADD_ICON(5,		10,	L"crops_2");
		ADD_ICON(5,		11,	L"crops_3");
		ADD_ICON(5,		12,	L"crops_4");
		ADD_ICON(5,		13,	L"crops_5");
		ADD_ICON(5,		14,	L"crops_6");
		ADD_ICON(5,		15,	L"crops_7");

		ADD_ICON(6,		0,	L"lever");
		ADD_ICON(6,		1,	L"door_wood_lower");
		ADD_ICON(6,		2,	L"door_iron_lower");
		ADD_ICON(6,		3,	L"redstone_torch_on");
		ADD_ICON(6,		4,	L"stonebrick_mossy");
		ADD_ICON(6,		5,	L"stonebrick_cracked");
		ADD_ICON(6,		6,	L"pumpkin_top");
		ADD_ICON(6,		7,	L"netherrack");
		ADD_ICON(6,		8,	L"soul_sand");
		ADD_ICON(6,		9,	L"glowstone");
		ADD_ICON(6,		10,	L"piston_top_sticky");
		ADD_ICON(6,		11,	L"piston_top");
		ADD_ICON(6,		12,	L"piston_side");
		ADD_ICON(6,		13,	L"piston_bottom");
		ADD_ICON(6,		14,	L"piston_inner_top");
		ADD_ICON(6,		15,	L"stem_straight");

		ADD_ICON(7,		0,	L"rail_normal_turned");
		ADD_ICON(7,		1,	L"wool_colored_black");
		ADD_ICON(7,		2,	L"wool_colored_gray");
		ADD_ICON(7,		3,	L"redstone_torch_off");
		ADD_ICON(7,		4,	L"log_spruce");
		ADD_ICON(7,		5,	L"log_birch");
		ADD_ICON(7,		6,	L"pumpkin_side");
		ADD_ICON(7,		7,	L"pumpkin_face_off");
		ADD_ICON(7,		8,	L"pumpkin_face_on");
		ADD_ICON(7,		9,	L"cake_top");
		ADD_ICON(7,		10,	L"cake_side");
		ADD_ICON(7,		11,	L"cake_inner");
		ADD_ICON(7,		12,	L"cake_bottom");
		ADD_ICON(7,		13,	L"mushroom_block_skin_red");
		ADD_ICON(7,		14,	L"mushroom_block_skin_brown");
		ADD_ICON(7,		15,	L"stem_bent");

		ADD_ICON(8,		0,	L"rail_normal");
		ADD_ICON(8,		1,	L"wool_colored_red");
		ADD_ICON(8,		2,	L"wool_colored_pink");
		ADD_ICON(8,		3,	L"repeater_off");
		ADD_ICON(8,		4,	L"leaves_spruce");
		ADD_ICON(8,		5,	L"leaves_spruce_opaque");
		ADD_ICON(8,		6,	L"bed_feet_top");
		ADD_ICON(8,		7,	L"bed_head_top");
		ADD_ICON(8,		8,	L"melon_side");
		ADD_ICON(8,		9,	L"melon_top");
		ADD_ICON(8,		10,	L"cauldron_top");
		ADD_ICON(8,		11,	L"cauldron_inner");
		//ADD_ICON(8,		12,	L"unused");
		ADD_ICON(8,		13,	L"mushroom_block_skin_stem");
		ADD_ICON(8,		14,	L"mushroom_block_inside");
		ADD_ICON(8,		15,	L"vine");

		ADD_ICON(9,		0,	L"lapis_block");
		ADD_ICON(9,		1,	L"wool_colored_green");
		ADD_ICON(9,		2,	L"wool_colored_lime");
		ADD_ICON(9,		3,	L"repeater_on");
		ADD_ICON(9,		4,	L"glass_pane_top");
		ADD_ICON(9,		5,	L"bed_feet_end");
		ADD_ICON(9,		6,	L"bed_feet_side");
		ADD_ICON(9,		7,	L"bed_head_side");
		ADD_ICON(9,		8,	L"bed_head_end");
		ADD_ICON(9,		9,	L"log_jungle");
		ADD_ICON(9,		10,	L"cauldron_side");
		ADD_ICON(9,		11,	L"cauldron_bottom");
		ADD_ICON(9,		12,	L"brewing_stand_base");
		ADD_ICON(9,		13,	L"brewing_stand");
		ADD_ICON(9,		14,	L"endframe_top");
		ADD_ICON(9,		15,	L"endframe_side");

		ADD_ICON(10,	0,	L"lapis_ore");
		ADD_ICON(10,	1,	L"wool_colored_brown");
		ADD_ICON(10,	2,	L"wool_colored_yellow");
		ADD_ICON(10,	3,	L"rail_golden");
		ADD_ICON(10,	4,	L"redstone_dust_cross");
		ADD_ICON(10,	5,	L"redstone_dust_line");
		ADD_ICON(10,	6,	L"enchantment_top");
		ADD_ICON(10,	7,	L"dragon_egg");
		ADD_ICON(10,	8,	L"cocoa_2");
		ADD_ICON(10,	9,	L"cocoa_1");
		ADD_ICON(10,	10,	L"cocoa_0");
		ADD_ICON(10,	11,	L"emerald_ore");
		ADD_ICON(10,	12,	L"trip_wire_source");
		ADD_ICON(10,	13,	L"trip_wire");
		ADD_ICON(10,	14,	L"endframe_eye");
		ADD_ICON(10,	15,	L"end_stone");

		ADD_ICON(11,	0,	L"sandstone_top");
		ADD_ICON(11,	1,	L"wool_colored_blue");
		ADD_ICON(11,	2,	L"wool_colored_light_blue");
		ADD_ICON(11,	3,	L"rail_golden_powered");
		ADD_ICON(11,	4,	L"redstone_dust_cross_overlay");
		ADD_ICON(11,	5,	L"redstone_dust_line_overlay");
		ADD_ICON(11,	6,	L"enchantment_side");
		ADD_ICON(11,	7,	L"enchantment_bottom");
		ADD_ICON(11,	8,	L"command_block");
		ADD_ICON(11,	9,	L"itemframe_back");
		ADD_ICON(11,	10,	L"flower_pot");
		ADD_ICON(11,	11,	L"comparator_off");
		ADD_ICON(11,	12,	L"comparator_on");
		ADD_ICON(11,	13,	L"rail_activator");
		ADD_ICON(11,	14,	L"rail_activator_powered");
		ADD_ICON(11,	15,	L"quartz_ore");

		ADD_ICON(12,	0,	L"sandstone_side");
		ADD_ICON(12,	1,	L"wool_colored_purple");
		ADD_ICON(12,	2,	L"wool_colored_magenta");
		ADD_ICON(12,	3,	L"detectorRail");
		ADD_ICON(12,	4,	L"leaves_jungle");
		ADD_ICON(12,	5,	L"leaves_jungle_opaque");
		ADD_ICON(12,	6,	L"planks_spruce");
		ADD_ICON(12,	7,	L"planks_jungle");
		ADD_ICON(12,	8,	L"carrots_stage_0");
		ADD_ICON(12,	9,	L"carrots_stage_1");
		ADD_ICON(12,	10,	L"carrots_stage_2");
		ADD_ICON(12,	11,	L"carrots_stage_3");
		//ADD_ICON(12,	12,	L"unused");
		ADD_ICON(12,	13,	L"water");
		ADD_ICON_SIZE(12,14,L"water_flow",2,2);

		ADD_ICON(13,	0,	L"sandstone_bottom");
		ADD_ICON(13,	1,	L"wool_colored_cyan");
		ADD_ICON(13,	2,	L"wool_colored_orange");
		ADD_ICON(13,	3,	L"redstoneLight");
		ADD_ICON(13,	4,	L"redstoneLight_lit");
		ADD_ICON(13,	5,	L"stonebrick_carved");
		ADD_ICON(13,	6,	L"planks_birch");
		ADD_ICON(13,	7,	L"anvil_base");
		ADD_ICON(13,	8,	L"anvil_top_damaged_1");
		ADD_ICON(13,	9,	L"quartz_block_chiseled_top");
		ADD_ICON(13,	10,	L"quartz_block_lines_top");
		ADD_ICON(13,	11,	L"quartz_block_top");
		ADD_ICON(13,	12,	L"hopper_outside");
		ADD_ICON(13,	13,	L"detectorRail_on");

		ADD_ICON(14,	0,	L"nether_brick");
		ADD_ICON(14,	1,	L"wool_colored_silver");
		ADD_ICON(14,	2,	L"nether_wart_stage_0");
		ADD_ICON(14,	3,	L"nether_wart_stage_1");
		ADD_ICON(14,	4,	L"nether_wart_stage_2");
		ADD_ICON(14,	5,	L"sandstone_carved");
		ADD_ICON(14,	6,	L"sandstone_smooth");
		ADD_ICON(14,	7,	L"anvil_top");
		ADD_ICON(14,	8,	L"anvil_top_damaged_2");
		ADD_ICON(14,	9,	L"quartz_block_chiseled");
		ADD_ICON(14,	10,	L"quartz_block_lines");
		ADD_ICON(14,	11,	L"quartz_block_side");
		ADD_ICON(14,	12,	L"hopper_inside");
		ADD_ICON(14,	13,	L"lava");
		ADD_ICON_SIZE(14,14,L"lava_flow",2,2);

		ADD_ICON(15,	0,	L"destroy_0");
		ADD_ICON(15,	1,	L"destroy_1");
		ADD_ICON(15,	2,	L"destroy_2");
		ADD_ICON(15,	3,	L"destroy_3");
		ADD_ICON(15,	4,	L"destroy_4");
		ADD_ICON(15,	5,	L"destroy_5");
		ADD_ICON(15,	6,	L"destroy_6");
		ADD_ICON(15,	7,	L"destroy_7");
		ADD_ICON(15,	8,	L"destroy_8");
		ADD_ICON(15,	9,	L"destroy_9");
		ADD_ICON(15,	10,	L"hay_block_side");
		ADD_ICON(15,	11,	L"quartz_block_bottom");
		ADD_ICON(15,	12,	L"hopper_top");
		ADD_ICON(15,	13,	L"hay_block_top");

		ADD_ICON(16,	0,	L"coal_block");
		ADD_ICON(16,	1,	L"hardened_clay");
		ADD_ICON(16,	2,	L"noteblock");
		//ADD_ICON(16,	3,	L"unused");
		//ADD_ICON(16,	4,	L"unused");
		//ADD_ICON(16,	5,	L"unused");
		//ADD_ICON(16,	6,	L"unused");
		//ADD_ICON(16,	7,	L"unused");
		//ADD_ICON(16,	8,	L"unused");
		ADD_ICON(16,	9,	L"potatoes_stage_0");
		ADD_ICON(16,	10,	L"potatoes_stage_1");
		ADD_ICON(16,	11,	L"potatoes_stage_2");
		ADD_ICON(16,	12,	L"potatoes_stage_3");
		ADD_ICON(16,	13,	L"log_spruce_top");
		ADD_ICON(16,	14,	L"log_jungle_top");
		ADD_ICON(16,	15,	L"log_birch_top");

		ADD_ICON(17,	0,	L"hardened_clay_stained_black");
		ADD_ICON(17,	1,	L"hardened_clay_stained_blue");
		ADD_ICON(17,	2,	L"hardened_clay_stained_brown");
		ADD_ICON(17,	3,	L"hardened_clay_stained_cyan");
		ADD_ICON(17,	4,	L"hardened_clay_stained_gray");
		ADD_ICON(17,	5,	L"hardened_clay_stained_green");
		ADD_ICON(17,	6,	L"hardened_clay_stained_light_blue");
		ADD_ICON(17,	7,	L"hardened_clay_stained_lime");
		ADD_ICON(17,	8,	L"hardened_clay_stained_magenta");
		ADD_ICON(17,	9,	L"hardened_clay_stained_orange");
		ADD_ICON(17,	10,	L"hardened_clay_stained_pink");
		ADD_ICON(17,	11,	L"hardened_clay_stained_purple");
		ADD_ICON(17,	12,	L"hardened_clay_stained_red");
		ADD_ICON(17,	13,	L"hardened_clay_stained_silver");
		ADD_ICON(17,	14,	L"hardened_clay_stained_white");
		ADD_ICON(17,	15,	L"hardened_clay_stained_yellow");

		ADD_ICON(18,	0,	L"glass_black");
		ADD_ICON(18,	1,	L"glass_blue");
		ADD_ICON(18,	2,	L"glass_brown");
		ADD_ICON(18,	3,	L"glass_cyan");
		ADD_ICON(18,	4,	L"glass_gray");
		ADD_ICON(18,	5,	L"glass_green");
		ADD_ICON(18,	6,	L"glass_light_blue");
		ADD_ICON(18,	7,	L"glass_lime");
		ADD_ICON(18,	8,	L"glass_magenta");
		ADD_ICON(18,	9,	L"glass_orange");
		ADD_ICON(18,	10,	L"glass_pink");
		ADD_ICON(18,	11,	L"glass_purple");
		ADD_ICON(18,	12,	L"glass_red");
		ADD_ICON(18,	13,	L"glass_silver");
		ADD_ICON(18,	14,	L"glass_white");
		ADD_ICON(18,	15,	L"glass_yellow");

		ADD_ICON(19,	0,	L"glass_pane_top_black");
		ADD_ICON(19,	1,	L"glass_pane_top_blue");
		ADD_ICON(19,	2,	L"glass_pane_top_brown");
		ADD_ICON(19,	3,	L"glass_pane_top_cyan");
		ADD_ICON(19,	4,	L"glass_pane_top_gray");
		ADD_ICON(19,	5,	L"glass_pane_top_green");
		ADD_ICON(19,	6,	L"glass_pane_top_light_blue");
		ADD_ICON(19,	7,	L"glass_pane_top_lime");
		ADD_ICON(19,	8,	L"glass_pane_top_magenta");
		ADD_ICON(19,	9,	L"glass_pane_top_orange");
		ADD_ICON(19,	10,	L"glass_pane_top_pink");
		ADD_ICON(19,	11,	L"glass_pane_top_purple");
		ADD_ICON(19,	12,	L"glass_pane_top_red");
		ADD_ICON(19,	13,	L"glass_pane_top_silver");
		ADD_ICON(19,	14,	L"glass_pane_top_white");
		ADD_ICON(19,	15,	L"glass_pane_top_yellow");
	}
}
