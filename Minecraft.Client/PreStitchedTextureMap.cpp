#include "stdafx.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\ByteBuffer.h"
#include "..\Minecraft.World\ArmorItem.h"
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
	missingPosition = NULL;
	stitchResult = NULL;

	m_mipMap = mipmap;
	missingPosition = (StitchedTexture *)(new SimpleIcon(NAME_MISSING_TEXTURE,NAME_MISSING_TEXTURE,0,0,1,1));
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
			if (Tile::tiles[i] != NULL)
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
		if (item != NULL && item->getIconType() == iconType)
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
	texturePack->generateStitched(texturesByName); // add fallback so custom ones work?

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

	if(stitchResult != NULL)
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
		if (frames == NULL || frames->empty())
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
	return NULL;
#if 0
	StitchedTexture *result = texturesByName.find(name)->second;
	if (result == NULL) result = missingPosition;
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
	Icon *result = NULL;
	if (name.empty())
	{
		app.DebugPrintf("Don't register NULL\n");
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
#define ADD_OBJ_ICON(row, column, tile) (texturesByName[tile->getIconName()] =	new SimpleIcon(tile->getIconName(),tile->getIconName(),horizRatio*column,vertRatio*row,horizRatio*(column+1),vertRatio*(row+1)));
#define ADD_COL_TILE_ICON(row, column, tile, id) (texturesByName[((ColoredTile*)tile)->getColoredIconName(DyePowderItem::id)] =	new SimpleIcon(((ColoredTile*)tile)->getColoredIconName(DyePowderItem::id),((ColoredTile*)tile)->getColoredIconName(DyePowderItem::id),horizRatio*column,vertRatio*row,horizRatio*(column+1),vertRatio*(row+1)));
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

		ADD_OBJ_ICON(0,		0,	Item::helmet_leather)
		ADD_OBJ_ICON(0,		1,	Item::helmet_chain)
		ADD_OBJ_ICON(0,		2,	Item::helmet_iron)
		ADD_OBJ_ICON(0,		3,	Item::helmet_diamond)
		ADD_OBJ_ICON(0,		4,	Item::helmet_gold)
		ADD_OBJ_ICON(0,		5,	Item::flintAndSteel)
		ADD_OBJ_ICON(0,		6,	Item::flint)
		ADD_OBJ_ICON(0,		7,	Item::coal)
		ADD_OBJ_ICON(0,		8,	Item::string)
		ADD_OBJ_ICON(0,		9,	Item::seeds_wheat)
		ADD_OBJ_ICON(0,		10,	Item::apple)
		ADD_OBJ_ICON(0,		11,	Item::apple_gold)
		ADD_OBJ_ICON(0,		12,	Item::egg)
		ADD_OBJ_ICON(0,		13,	Item::sugar)
		ADD_OBJ_ICON(0,		14,	Item::snowBall)
		ADD_ICON(0,		15, ((ArmorItem*)Item::boots_iron)->TEXTURE_EMPTY_SLOTS[0])

		ADD_OBJ_ICON(1,		0,	Item::chestplate_leather)
		ADD_OBJ_ICON(1,		1,	Item::chestplate_chain)
		ADD_OBJ_ICON(1,		2,	Item::chestplate_iron)
		ADD_OBJ_ICON(1,		3,	Item::chestplate_diamond)
		ADD_OBJ_ICON(1,		4,	Item::chestplate_gold)
		ADD_OBJ_ICON(1,		5,	Item::bow)
		ADD_OBJ_ICON(1,		6,	Item::brick)
		ADD_OBJ_ICON(1,		7,	Item::ironIngot)
		ADD_ICON(1,		8,	L"feather")
		ADD_OBJ_ICON(1,		9,	Item::wheat)
		ADD_ICON(1,		10,	L"painting")
		ADD_OBJ_ICON(1,		11,	Item::reeds)
		ADD_OBJ_ICON(1,		12,	Item::bone)
		ADD_OBJ_ICON(1,		13,	Item::cake)
		ADD_OBJ_ICON(1,		14,	Item::slimeBall)
		ADD_ICON(1, 15, ((ArmorItem*)Item::boots_iron)->TEXTURE_EMPTY_SLOTS[1]) //	L"empty_armor_slot_chestplate")

		ADD_OBJ_ICON(2, 0,	Item::leggings_leather)
		ADD_OBJ_ICON(2, 1,	Item::leggings_chain)
		ADD_OBJ_ICON(2, 2,	Item::leggings_iron)
		ADD_OBJ_ICON(2,	3,	Item::leggings_diamond)
		ADD_OBJ_ICON(2,	4,	Item::leggings_gold)
		ADD_OBJ_ICON(2,		5,	Item::arrow)
		ADD_ICON(2,		6,	L"quiver")
		ADD_OBJ_ICON(2,		7,	Item::goldIngot)
		ADD_OBJ_ICON(2,		8,	Item::gunpowder)
		ADD_OBJ_ICON(2,		9,	Item::bread)
		ADD_OBJ_ICON(2,		10,	Item::sign)
		ADD_OBJ_ICON(2,		11,	Item::door_wood)
		ADD_OBJ_ICON(2,		12,	Item::door_iron)
		ADD_OBJ_ICON(2,	13,	Item::bed)
		ADD_OBJ_ICON(2,	14,	Item::fireball)
		ADD_ICON(2,		15, ((ArmorItem*)Item::boots_iron)->TEXTURE_EMPTY_SLOTS[2])

		ADD_OBJ_ICON(3,	0,	Item::boots_leather)
		ADD_OBJ_ICON(3,	1,	Item::boots_chain)
		ADD_OBJ_ICON(3,	2,	Item::boots_iron)
		ADD_OBJ_ICON(3,	3,	Item::boots_diamond)
		ADD_OBJ_ICON(3,	4,	Item::boots_gold)
		ADD_OBJ_ICON(3,	5,	Item::stick)
		ADD_ICON(3,		6,	L"compass")
		ADD_OBJ_ICON(3, 7,	Item::diamond)
		ADD_OBJ_ICON(3, 8,	Item::redStone)
		ADD_OBJ_ICON(3, 9,	Item::clay)
		ADD_OBJ_ICON(3, 10, Item::paper)
		ADD_OBJ_ICON(3, 11, Item::book)
		ADD_ICON(3,		12,	L"filled_map")
		ADD_OBJ_ICON(3,		13,	Item::seeds_pumpkin)
		ADD_OBJ_ICON(3,		14,	Item::seeds_melon)
		ADD_ICON(3,		15, ((ArmorItem*)Item::boots_iron)->TEXTURE_EMPTY_SLOTS[3])

		ADD_OBJ_ICON(4,	0,	Item::sword_wood)
		ADD_OBJ_ICON(4, 1,	Item::sword_stone)
		ADD_OBJ_ICON(4, 2,	Item::sword_iron)
		ADD_OBJ_ICON(4,	3,	Item::sword_diamond)
		ADD_OBJ_ICON(4,	4,	Item::sword_gold)
		ADD_OBJ_ICON(4,		5,	Item::fishingRod)
		ADD_ICON(4,		6,	L"clock")
		ADD_OBJ_ICON(4,	7,	Item::bowl)
		ADD_OBJ_ICON(4,	8,	Item::mushroomStew)
		ADD_OBJ_ICON(4,	9,	Item::yellowDust)
		ADD_OBJ_ICON(4, 10, Item::bucket_empty)
		ADD_OBJ_ICON(4, 11, Item::bucket_water)
		ADD_OBJ_ICON(4, 12, Item::bucket_lava)
		ADD_OBJ_ICON(4,	13,	Item::bucket_milk)
		ADD_ICON(4,		14,	L"black_dye")
		ADD_ICON(4,		15,	L"gray_dye")

		ADD_OBJ_ICON(5,		0, Item::shovel_wood)
		ADD_OBJ_ICON(5,		1, Item::shovel_stone)
		ADD_OBJ_ICON(5,		2, Item::shovel_iron)
		ADD_OBJ_ICON(5,		3, Item::shovel_diamond)
		ADD_OBJ_ICON(5,		4, Item::shovel_gold)
		ADD_ICON(5,		5,	L"fishing_rod_cast")
		ADD_OBJ_ICON(5,		6,	Item::repeater)
		ADD_OBJ_ICON(5,		7,	Item::porkChop_raw)
		ADD_OBJ_ICON(5,		8,	Item::porkChop_cooked)
		ADD_OBJ_ICON(5,		9,	Item::fish_raw)
		ADD_OBJ_ICON(5,		10,	Item::fish_cooked)
		ADD_OBJ_ICON(5,		11,	Item::rotten_flesh)
		ADD_OBJ_ICON(5,		12,	Item::cookie)
		ADD_OBJ_ICON(5,		13, Item::shears)
		ADD_ICON(5,		14,	L"red_dye")
		ADD_ICON(5,		15,	L"pink_dye")

		ADD_OBJ_ICON(6, 0, Item::pickAxe_wood)
		ADD_OBJ_ICON(6, 1, Item::pickAxe_stone)
		ADD_OBJ_ICON(6, 2, Item::pickAxe_iron)
		ADD_OBJ_ICON(6, 3, Item::pickAxe_diamond)
		ADD_OBJ_ICON(6,		4,	Item::pickAxe_gold)
		ADD_ICON(6,		5,	L"bow_pull_0")
		ADD_OBJ_ICON(6,		6,	Item::carrotOnAStick)
		ADD_OBJ_ICON(6, 7,	Item::leather)
		ADD_OBJ_ICON(6, 8,	Item::saddle)
		ADD_OBJ_ICON(6, 9,	Item::beef_raw)
		ADD_OBJ_ICON(6,	10,	Item::beef_cooked)
		ADD_OBJ_ICON(6,	11,	Item::enderPearl)
		ADD_OBJ_ICON(6,	12,	Item::blazeRod)
		ADD_OBJ_ICON(6,	13,	Item::melon)
		ADD_ICON(6,		14,	L"green_dye")
		ADD_ICON(6,		15,	L"lime_dye")

		ADD_OBJ_ICON(7, 0,	Item::hatchet_wood)
		ADD_OBJ_ICON(7, 1,	Item::hatchet_stone)
		ADD_OBJ_ICON(7, 2,	Item::hatchet_iron)
		ADD_OBJ_ICON(7, 3,	Item::hatchet_diamond)
		ADD_OBJ_ICON(7,	4,	Item::hatchet_gold)
		ADD_ICON(7,		5,	L"bow_pull_1")
		ADD_OBJ_ICON(7,		6,	Item::potatoBaked)
		ADD_OBJ_ICON(7, 7,	Item::potato)
		ADD_OBJ_ICON(7, 8,	Item::carrots)
		ADD_OBJ_ICON(7, 9,	Item::chicken_raw)
		ADD_OBJ_ICON(7, 10, Item::chicken_cooked)
		ADD_OBJ_ICON(7, 11, Item::ghastTear)
		ADD_OBJ_ICON(7, 12, Item::goldNugget)
		ADD_OBJ_ICON(7,	13,	Item::netherwart_seeds)
		ADD_ICON(7,		14,	L"brown_dye")
		ADD_ICON(7,		15,	L"yellow_dye")

		ADD_OBJ_ICON(8, 0, Item::hoe_wood)
		ADD_OBJ_ICON(8, 1, Item::hoe_stone)
		ADD_OBJ_ICON(8, 2, Item::hoe_iron)
		ADD_OBJ_ICON(8, 3, Item::hoe_diamond)
		ADD_OBJ_ICON(8,		4,	Item::hoe_gold)
		ADD_ICON(8,		5,	L"bow_pull_2")
		ADD_OBJ_ICON(8,	6,	Item::potatoPoisonous)
		ADD_OBJ_ICON(8,	7,	Item::minecart)
		ADD_OBJ_ICON(8,	8,	Item::boat)
		ADD_OBJ_ICON(8,	9,	Item::speckledMelon)
		ADD_OBJ_ICON(8,	10,	Item::fermentedSpiderEye)
		ADD_OBJ_ICON(8,	11,	Item::spiderEye)
		ADD_OBJ_ICON(8,	12,	Item::potion)
		ADD_OBJ_ICON(8,	12,	Item::glassBottle) // Same as potion
		ADD_ICON(8,		13,		((PotionItem*)Item::potion)->CONTENTS_ICON)
		ADD_ICON(8,		14,		L"blue_dye")
		ADD_ICON(8,		15,		L"light_blue_dye")

		ADD_ICON(9,		0,	((ArmorItem*)Item::boots_iron)->LEATHER_OVERLAYS[0])
		//ADD_ICON(9,		1,	L"unused")
		ADD_OBJ_ICON(9,	2,	Item::horseArmorMetal)
		ADD_OBJ_ICON(9,	3,	Item::horseArmorDiamond)
		ADD_OBJ_ICON(9,	4,	Item::horseArmorGold)
		ADD_OBJ_ICON(9,	5,	Item::comparator)
		ADD_OBJ_ICON(9,	6,	Item::carrotGolden)
		ADD_OBJ_ICON(9,	7,	Item::minecart_chest)
		ADD_OBJ_ICON(9,	8,	Item::pumpkinPie)
		ADD_OBJ_ICON(9,	9,	Item::spawnEgg)
		ADD_ICON(9,		10,		((PotionItem*)Item::potion)->THROWABLE_ICON)
		ADD_OBJ_ICON(9,	11,	Item::eyeOfEnder)
		ADD_OBJ_ICON(9,	12,	Item::cauldron)
		ADD_OBJ_ICON(9,	13, Item::blazePowder)
		ADD_ICON(9,		14,		L"purple_dye")
		ADD_ICON(9,		15,		L"magenta_dye")

		ADD_ICON(10,	0,		((ArmorItem*)Item::boots_iron)->LEATHER_OVERLAYS[1])
		//ADD_ICON(10,	1,	L"unused")
		//ADD_ICON(10,	2,	L"unused")
		ADD_OBJ_ICON(10, 3,	Item::nameTag)
		ADD_OBJ_ICON(10, 4,	Item::lead)
		ADD_OBJ_ICON(10, 5,	Item::netherbrick)
		//ADD_ICON(10,	6,	L"unused")
		ADD_OBJ_ICON(10,	7,	Item::minecart_furnace)
		ADD_ICON(10,	8,	L"charcoal")
		ADD_ICON(10,	9,	L"spawn_egg_overlay")
		ADD_ICON(10,	10,	L"ruby")
		ADD_OBJ_ICON(10, 11,	Item::expBottle)
		ADD_OBJ_ICON(10, 12,	Item::brewingStand)
		ADD_OBJ_ICON(10, 13,	Item::magmaCream)
		ADD_ICON(10,	14,	L"cyan_dye")
		ADD_ICON(10,	15,	L"orange_dye")

		ADD_ICON(11,	0,		((ArmorItem*)Item::boots_iron)->LEATHER_OVERLAYS[2])
		//ADD_ICON(11,	1,	L"unused")
		//ADD_ICON(11,	2,	L"unused")
		//ADD_ICON(11,	3,	L"unused")
		//ADD_ICON(11,	4,	L"unused")
		//ADD_ICON(11,	5,	L"unused")
		//ADD_ICON(11,	6,	L"unused")
		ADD_OBJ_ICON(11,	7,	Item::minecart_hopper)
		ADD_ICON(11,	8,		L"hopper")
		ADD_OBJ_ICON(11,	9,	Item::netherStar)
		ADD_OBJ_ICON(11,	10,	Item::emerald)
		ADD_ICON(11,	11,		L"writable_book")
		ADD_ICON(11,	12,		L"written_book")
		ADD_OBJ_ICON(11,	13,	Item::flowerPot)
		ADD_ICON(11,	14,		L"light_gray_dye")
		ADD_ICON(11,	15,		L"white_dye")

		ADD_ICON(12,	0,		((ArmorItem*)Item::boots_iron)->LEATHER_OVERLAYS[3])
		//ADD_ICON(12,	1,	L"unused")
		//ADD_ICON(12,	2,	L"unused")
		//ADD_ICON(12,	3,	L"unused")
		//ADD_ICON(12,	4,	L"unused")
		//ADD_ICON(12,	5,	L"unused")
		//ADD_ICON(12,	6,	L"unused")
		ADD_OBJ_ICON(12,	7,	Item::minecart_tnt)
		//ADD_ICON(12,	8,	L"unused")
		ADD_OBJ_ICON(12,	9,	Item::fireworks)
		ADD_OBJ_ICON(12,	10,	Item::fireworksCharge)
		ADD_ICON(12,	11,	L"fireworks_charge_overlay")
		ADD_OBJ_ICON(12,	12,	Item::netherQuartz)
		ADD_ICON(12,	13,	L"map")
		ADD_OBJ_ICON(12,	14,	Item::frame)
		ADD_OBJ_ICON(12,	15,	Item::enchantedBook)

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

		ADD_ICON(15,		0,	L"music_disc_13")
		ADD_ICON(15,		1,	L"music_disc_cat")
		ADD_ICON(15,		2,	L"music_disc_blocks")
		ADD_ICON(15,		3,	L"music_disc_chirp")
		ADD_ICON(15,		4,	L"music_disc_far")
		ADD_ICON(15,		5,	L"music_disc_mall")
		ADD_ICON(15,		6,	L"music_disc_mellohi")
		ADD_ICON(15,		7,	L"music_disc_stal")
		ADD_ICON(15,		8,	L"music_disc_strad")
		ADD_ICON(15,		9,	L"music_disc_ward")
		ADD_ICON(15,		10,	L"music_disc_11")
		ADD_ICON(15,		11,	L"music_disc_where are we now")

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

		ADD_ICON(0,		0,	L"grass_block_top")
		texturesByName[L"grass_block_top"]->setFlags(Icon::IS_GRASS_TOP);			// 4J added for faster determination of texture type in tesselation
		ADD_ICON(0,		1,	L"stone")
		ADD_ICON(0,		2,	L"dirt")
		ADD_ICON(0,		3,	L"grass_block_side")
		texturesByName[L"grass_block_side"]->setFlags(Icon::IS_GRASS_SIDE);		// 4J added for faster determination of texture type in tesselation
		ADD_ICON(0,		4,	L"oak_planks")
		ADD_ICON(0,		5,	L"smooth_stone_slab_side")
		ADD_ICON(0,		6,	L"smooth_stone")
		ADD_OBJ_ICON(0,		7,	Tile::redBrick)
		ADD_ICON(0,		8,	L"tnt_side")
		ADD_ICON(0,		9,	L"tnt_top")
		ADD_ICON(0,		10,	L"tnt_bottom")
		ADD_OBJ_ICON(0,		11,	Tile::web)
		ADD_OBJ_ICON(0,		12,	Tile::rose)
		ADD_OBJ_ICON(0,		13,	Tile::flower)
		ADD_ICON(0,		14,	L"portal")
		ADD_ICON(0,		15,	L"oak_sapling")

		ADD_ICON(1,		0,	L"cobblestone");
		ADD_ICON(1,		1,	L"bedrock");
		ADD_ICON(1,		2,	L"sand");
		ADD_ICON(1,		3,	L"gravel");
		ADD_ICON(1,		4,	L"oak_log");
		ADD_ICON(1,		5,	L"oak_log_top");
		ADD_ICON(1,		6,	L"iron_block");
		ADD_ICON(1,		7,	L"gold_block");
		ADD_ICON(1,		8,	L"diamond_block");
		ADD_ICON(1,		9,	L"emerald_block");
		ADD_ICON(1,		10,	L"redstone_block");
		ADD_ICON(1,		11,	L"dropper_front");
		ADD_OBJ_ICON(1,		12, Tile::mushroom_red);
		ADD_OBJ_ICON(1,		13, Tile::mushroom_brown);
		ADD_ICON(1,		14,	L"jungle_sapling");
		ADD_ICON(1,		15,	L"fire_0");

		ADD_ICON(2,		0,	L"gold_ore");
		ADD_ICON(2,		1,	L"iron_ore");
		ADD_ICON(2,		2,	L"coal_ore");
		ADD_ICON(2,		3,	L"bookshelf");
		ADD_OBJ_ICON(2,		4,	Tile::mossyCobblestone);
		ADD_OBJ_ICON(2,		5,	Tile::obsidian);
		ADD_ICON(2,		6,	L"grass_block_side_overlay");
		ADD_OBJ_ICON(2,		7,	Tile::tallgrass);
		ADD_ICON(2,		8,	L"dispenser_front_vertical");
		ADD_ICON(2,		9,	L"beacon");
		ADD_ICON(2,		10,	L"dropper_front_vertical");
		ADD_ICON(2,		11,	L"crafting_table_top");
		ADD_ICON(2,		12,	L"furnace_front");
		ADD_ICON(2,		13,	L"furnace_side");
		ADD_ICON(2,		14,	L"dispenser_front");
		ADD_ICON(2,		15,	L"fire_1");

		ADD_ICON(3,		0,	L"sponge");
		ADD_ICON(3,		1,	L"glass");
		ADD_OBJ_ICON(3,		2,	Tile::diamondOre);
		ADD_OBJ_ICON(3,		3,	Tile::redStoneOre);
		ADD_ICON(3,		4,	L"oak_leaves");
		ADD_ICON(3,		5,	L"oak_leaves_opaque");
		ADD_OBJ_ICON(3,		6,	Tile::stoneBrick);
		ADD_OBJ_ICON(3,		7,	Tile::deadBush);
		ADD_ICON(3,		8,	L"fern");
		ADD_ICON(3,		9,	L"daylight_detector_top");
		ADD_ICON(3,		10,	L"daylight_detector_side");
		ADD_ICON(3,		11,	L"crafting_table_side");
		ADD_ICON(3,		12,	L"crafting_table_front");
		ADD_ICON(3,		13,	L"furnace_front_on");
		ADD_ICON(3,		14,	L"furnace_top");
		ADD_ICON(3,		15,	L"spruce_sapling");

		ADD_COL_TILE_ICON(4,	0,	Tile::wool, WHITE);
		ADD_OBJ_ICON(4,		1,	Tile::mobSpawner);
		ADD_ICON(4,		2,	L"snow");
		ADD_ICON(4,		3,	L"ice");
		ADD_ICON(4,		4,	L"grass_block_snow");
		ADD_ICON(4,		5,	L"cactus_top");
		ADD_ICON(4,		6,	L"cactus_side");
		ADD_ICON(4,		7,	L"cactus_bottom");
		ADD_ICON(4,		8,	L"clay");
		ADD_OBJ_ICON(4,		9,	Tile::reeds);
		ADD_ICON(4,		10,	L"jukebox_side");
		ADD_ICON(4,		11,	L"jukebox_top");
		ADD_OBJ_ICON(4,		12,	Tile::waterLily);
		ADD_ICON(4,		13,	L"mycelium_side");
		ADD_ICON(4,		14,	L"mycelium_top");
		ADD_ICON(4,		15,	L"birch_sapling");

		ADD_OBJ_ICON(5,		0,	Tile::torch);
		ADD_ICON(5,		1,	L"oak_door_top");
		ADD_ICON(5,		2,	L"iron_door_top");
		ADD_OBJ_ICON(5,		3,	Tile::ladder);
		ADD_OBJ_ICON(5,		4,	Tile::trapdoor);
		ADD_ICON(5,		5,	L"iron_bars");
		ADD_ICON(5,		6,	L"farmland_moist");
		ADD_ICON(5,		7,	L"farmland");
		ADD_ICON(5,		8,	L"wheat_stage0");
		ADD_ICON(5,		9,	L"wheat_stage1");
		ADD_ICON(5,		10,	L"wheat_stage2");
		ADD_ICON(5,		11,	L"wheat_stage3");
		ADD_ICON(5,		12,	L"wheat_stage4");
		ADD_ICON(5,		13,	L"wheat_stage5");
		ADD_ICON(5,		14,	L"wheat_stage6");
		ADD_ICON(5,		15,	L"wheat_stage7");

		ADD_ICON(6,		0,	L"lever");
		ADD_ICON(6,		1,	L"oak_door_bottom");
		ADD_ICON(6,		2,	L"iron_door_bottom");
		ADD_OBJ_ICON(6,		3,	Tile::redstoneTorch_on);
		ADD_ICON(6,		4,	L"mossy_stone_bricks");
		ADD_ICON(6,		5,	L"cracked_stone_bricks");
		ADD_ICON(6,		6,	L"pumpkin_top");
		ADD_OBJ_ICON(6,		7,	Tile::netherRack);
		ADD_ICON(6,		8,	L"soul_sand");
		ADD_ICON(6,		9,	L"glowstone");
		ADD_ICON(6,		10,	L"piston_top_sticky");
		ADD_ICON(6,		11,	L"piston_top");
		ADD_ICON(6,		12,	L"piston_side");
		ADD_ICON(6,		13,	L"piston_bottom");
		ADD_ICON(6,		14,	L"piston_inner");
		ADD_ICON(6,		15,	L"pumpkin_stem_disconnected");

		ADD_ICON(7,		0,	L"rail_corner");
		ADD_COL_TILE_ICON(7,		1,	Tile::wool, BLACK);
		ADD_COL_TILE_ICON(7,		2,	Tile::wool, GRAY);
		ADD_OBJ_ICON(7,		3,	Tile::redstoneTorch_off);
		ADD_ICON(7,		4,	L"spruce_log");
		ADD_ICON(7,		5,	L"birch_log");
		ADD_ICON(7,		6,	L"pumpkin_side");
		ADD_ICON(7,		7,	L"carved_pumpkin");
		ADD_ICON(7,		8,	L"jack_o_lantern");
		ADD_ICON(7,		9,	L"cake_top");
		ADD_ICON(7,		10,	L"cake_side");
		ADD_ICON(7,		11,	L"cake_inner");
		ADD_ICON(7,		12,	L"cake_bottom");
		ADD_ICON(7,		13,	L"red_mushroom_block");
		ADD_ICON(7,		14,	L"brown_mushroom_block");
		ADD_ICON(7,		15,	L"attached_pumpkin_stem");

		ADD_ICON(8,		0,	L"rail");
		ADD_COL_TILE_ICON(8,		1,	Tile::wool, RED);
		ADD_COL_TILE_ICON(8,		2,	Tile::wool, PINK);
		ADD_OBJ_ICON(8,		3,	Tile::diode_off);
		ADD_ICON(8,		4,	L"spruce_leaves");
		ADD_ICON(8,		5,	L"spruce_leaves_opaque");
		ADD_ICON(8,		6,	L"bed_feet_top");
		ADD_ICON(8,		7,	L"bed_head_top");
		ADD_ICON(8,		8,	L"melon_side");
		ADD_ICON(8,		9,	L"melon_top");
		ADD_ICON(8,		10,	L"cauldron_top");
		ADD_ICON(8,		11,	L"cauldron_inner");
		//ADD_ICON(8,		12,	L"unused");
		ADD_ICON(8,		13,	L"mushroom_stem");
		ADD_ICON(8,		14,	L"mushroom_block_inside");
		ADD_ICON(8,		15,	L"vine");

		ADD_ICON(9,		0,	L"lapis_block");
		ADD_COL_TILE_ICON(9,		1, Tile::wool, GREEN);
		ADD_COL_TILE_ICON(9,		2, Tile::wool, LIME);
		ADD_OBJ_ICON(9,	3,	Tile::diode_on);
		ADD_ICON(9,		4,	L"glass_pane_top");
		ADD_ICON(9,		5,	L"bed_feet_end");
		ADD_ICON(9,		6,	L"bed_feet_side");
		ADD_ICON(9,		7,	L"bed_head_side");
		ADD_ICON(9,		8,	L"bed_head_end");
		ADD_ICON(9,		9,	L"jungle_log");
		ADD_ICON(9,		10,	L"cauldron_side");
		ADD_ICON(9,		11,	L"cauldron_bottom");
		ADD_ICON(9,		12,	L"brewing_stand_base");
		ADD_ICON(9,		13,	L"brewing_stand");
		ADD_ICON(9,		14,	L"end_portal_frame_top");
		ADD_ICON(9,		15,	L"end_portal_frame_side");

		ADD_ICON(10,	0,	L"lapis_ore");
		ADD_COL_TILE_ICON(10,	1, Tile::wool, BROWN);
		ADD_COL_TILE_ICON(10,	2, Tile::wool, YELLOW);
		ADD_OBJ_ICON(10,	3,	Tile::goldenRail);
		ADD_ICON(10,	4,	L"redstone_dust_cross");
		ADD_ICON(10,	5,	L"redstone_dust_line");
		ADD_ICON(10,	6,	L"enchanting_table_top");
		ADD_ICON(10,	7,	L"dragon_egg");
		ADD_ICON(10,	8,	L"cocoa_stage2");
		ADD_ICON(10,	9,	L"cocoa_stage1");
		ADD_ICON(10,	10,	L"cocoa_stage0");
		ADD_ICON(10,	11,	L"emerald_ore");
		ADD_OBJ_ICON(10,	12,	Tile::tripWireSource);
		ADD_OBJ_ICON(10,	13, Tile::tripWire);
		ADD_ICON(10,	14,	L"end_portal_frame_eye");
		ADD_ICON(10,	15,	L"end_stone");

		ADD_ICON(11,	0,	L"sandstone_top");
		ADD_COL_TILE_ICON(11,	1,	Tile::wool, BLUE);
		ADD_COL_TILE_ICON(11,	2,	Tile::wool, LIGHT_BLUE);
		ADD_ICON(11,	3,	L"powered_rail_on");
		ADD_ICON(11,	4,	L"redstone_dust_cross_overlay");
		ADD_ICON(11,	5,	L"redstone_dust_line_overlay");
		ADD_ICON(11,	6,	L"enchanting_table_side");
		ADD_ICON(11,	7,	L"enchanting_table_bottom");
		ADD_ICON(11,	8,	L"command_block");
		ADD_ICON(11,	9,	L"itemframe_back");
		ADD_ICON(11,	10,	L"flower_pot");
		ADD_ICON(11,	11,	L"comparator");
		ADD_ICON(11,	12,	L"comparator_on");
		ADD_OBJ_ICON(11,	13,	Tile::activatorRail);
		ADD_ICON(11,	14,	L"activator_rail_on");
		ADD_OBJ_ICON(11,	15,	Tile::netherQuartz);

		ADD_ICON(12,	0,	L"sandstone");
		ADD_COL_TILE_ICON(12,	1, Tile::wool, PURPLE);
		ADD_COL_TILE_ICON(12,	2, Tile::wool, MAGENTA);
		ADD_OBJ_ICON(12,	3,	Tile::detectorRail);
		ADD_ICON(12,	4,	L"jungle_leaves");
		ADD_ICON(12,	5,	L"jungle_leaves_opaque");
		ADD_ICON(12,	6,	L"spruce_planks");
		ADD_ICON(12,	7,	L"jungle_planks");
		ADD_ICON(12,	8,	L"carrots_stage0");
		ADD_ICON(12,	9,	L"carrots_stage1");
		ADD_ICON(12,	10,	L"carrots_stage2");
		ADD_ICON(12,	11,	L"carrots_stage3");
		//ADD_ICON(12,	12,	L"unused");
		ADD_ICON(12,	13,	L"water");
		ADD_ICON_SIZE(12,14,L"water_flow",2,2);

		ADD_ICON(13,	0,	L"sandstone_bottom");
		ADD_COL_TILE_ICON(13,	1,	Tile::wool,	CYAN);
		ADD_COL_TILE_ICON(13,	2,	Tile::wool, ORANGE);
		ADD_OBJ_ICON(13,	3,	Tile::redstoneLight);
		ADD_OBJ_ICON(13,	4,	Tile::redstoneLight_lit);
		ADD_ICON(13,	5,	L"chiseled_stone_bricks");
		ADD_ICON(13,	6,	L"birch_planks");
		ADD_ICON(13,	7,	L"anvil");
		ADD_ICON(13,	8,	L"chipped_anvil_top");
		ADD_ICON(13,	9,	L"chiseled_quartz_block_top");
		ADD_ICON(13,	10,	L"quartz_pillar_top");
		ADD_ICON(13,	11,	L"quartz_block_top");
		ADD_ICON(13,	12,	L"hopper_outside");
		ADD_ICON(13,	13,	L"detector_rail_on");

		ADD_OBJ_ICON(14,	0,	Tile::netherBrick);
		ADD_COL_TILE_ICON(14,	1, Tile::wool, SILVER); // light gray
		ADD_ICON(14,	2,	L"nether_wart_stage0");
		ADD_ICON(14,	3,	L"nether_wart_stage1");
		ADD_ICON(14,	4,	L"nether_wart_stage2");
		ADD_ICON(14,	5,	L"chiseled_sandstone");
		ADD_ICON(14,	6,	L"cut_sandstone");
		ADD_ICON(14,	7,	L"anvil_top");
		ADD_ICON(14,	8,	L"damaged_anvil_top");
		ADD_ICON(14,	9,	L"chiseled_quartz_block");
		ADD_ICON(14,	10,	L"quartz_pillar");
		ADD_ICON(14,	11,	L"quartz_block_side");
		ADD_ICON(14,	12,	L"hopper_inside");
		ADD_ICON(14,	13,	L"lava");
		ADD_ICON_SIZE(14,14,L"lava_flow",2,2);

		ADD_ICON(15,	0,	L"destroy_stage_0");
		ADD_ICON(15,	1,	L"destroy_stage_1");
		ADD_ICON(15,	2,	L"destroy_stage_2");
		ADD_ICON(15,	3,	L"destroy_stage_3");
		ADD_ICON(15,	4,	L"destroy_stage_4");
		ADD_ICON(15,	5,	L"destroy_stage_5");
		ADD_ICON(15,	6,	L"destroy_stage_6");
		ADD_ICON(15,	7,	L"destroy_stage_7");
		ADD_ICON(15,	8,	L"destroy_stage_8");
		ADD_ICON(15,	9,	L"destroy_stage_9");
		ADD_ICON(15,	10,	L"hay_block_side");
		ADD_ICON(15,	11,	L"quartz_block_bottom");
		ADD_ICON(15,	12,	L"hopper_top");
		ADD_ICON(15,	13,	L"hay_block_top");

		ADD_ICON(16,	0,	L"coal_block");
		ADD_OBJ_ICON(16,	1,	Tile::clayHardened);
		ADD_OBJ_ICON(16,	2,	Tile::noteblock);
		//ADD_ICON(16,	3,	L"unused");
		//ADD_ICON(16,	4,	L"unused");
		//ADD_ICON(16,	5,	L"unused");
		//ADD_ICON(16,	6,	L"unused");
		//ADD_ICON(16,	7,	L"unused");
		//ADD_ICON(16,	8,	L"unused");
		ADD_ICON(16,	9,	L"potatoes_stage0");
		ADD_ICON(16,	10,	L"potatoes_stage1");
		ADD_ICON(16,	11,	L"potatoes_stage2");
		ADD_ICON(16,	12,	L"potatoes_stage3");
		ADD_ICON(16,	13,	L"spruce_log_top");
		ADD_ICON(16,	14,	L"jungle_log_top");
		ADD_ICON(16,	15,	L"birch_log_top");

		ADD_COL_TILE_ICON(17,	0,	Tile::clayHardened, BLACK);
		ADD_COL_TILE_ICON(17,	1,	Tile::clayHardened, BLUE);
		ADD_COL_TILE_ICON(17,	2,	Tile::clayHardened, BROWN);
		ADD_COL_TILE_ICON(17,	3,	Tile::clayHardened, CYAN);
		ADD_COL_TILE_ICON(17,	4,	Tile::clayHardened, GRAY);
		ADD_COL_TILE_ICON(17,	5,	Tile::clayHardened, GREEN);
		ADD_COL_TILE_ICON(17,	6,	Tile::clayHardened, LIGHT_BLUE);
		ADD_COL_TILE_ICON(17,	7,	Tile::clayHardened, LIME);
		ADD_COL_TILE_ICON(17,	8,	Tile::clayHardened, MAGENTA);
		ADD_COL_TILE_ICON(17,	9,	Tile::clayHardened, ORANGE);
		ADD_COL_TILE_ICON(17,	10, Tile::clayHardened, PINK);
		ADD_COL_TILE_ICON(17,	11, Tile::clayHardened, PURPLE);
		ADD_COL_TILE_ICON(17,	12, Tile::clayHardened, RED);
		ADD_COL_TILE_ICON(17,	13, Tile::clayHardened, SILVER);
		ADD_COL_TILE_ICON(17,	14, Tile::clayHardened, WHITE);
		ADD_COL_TILE_ICON(17,	15, Tile::clayHardened, YELLOW);

		ADD_COL_TILE_ICON(18,	0,	Tile::stained_glass, BLACK);//L"glass_black");
		ADD_COL_TILE_ICON(18,	1,	Tile::stained_glass, BLUE);//L"glass_blue");
		ADD_COL_TILE_ICON(18,	2,	Tile::stained_glass, BROWN);//L"glass_brown");
		ADD_COL_TILE_ICON(18,	3,	Tile::stained_glass, CYAN);//L"glass_cyan");
		ADD_COL_TILE_ICON(18,	4,	Tile::stained_glass, GRAY);//L"glass_gray");
		ADD_COL_TILE_ICON(18,	5,	Tile::stained_glass, GREEN);//L"glass_green");
		ADD_COL_TILE_ICON(18,	6,	Tile::stained_glass, LIGHT_BLUE);//L"glass_light_blue");
		ADD_COL_TILE_ICON(18,	7,	Tile::stained_glass, LIME);//L"glass_lime");
		ADD_COL_TILE_ICON(18,	8,	Tile::stained_glass, MAGENTA);//L"glass_magenta");
		ADD_COL_TILE_ICON(18,	9,	Tile::stained_glass, ORANGE);//L"glass_orange");
		ADD_COL_TILE_ICON(18,	10, Tile::stained_glass, PINK);//L"glass_pink");
		ADD_COL_TILE_ICON(18,	11, Tile::stained_glass, PURPLE);//L"glass_purple");
		ADD_COL_TILE_ICON(18,	12, Tile::stained_glass, RED);//L"glass_red");
		ADD_COL_TILE_ICON(18,	13, Tile::stained_glass, SILVER);//L"glass_silver");
		ADD_COL_TILE_ICON(18,	14, Tile::stained_glass, WHITE);//L"glass_white");
		ADD_COL_TILE_ICON(18,	15,	Tile::stained_glass, YELLOW);//L"glass_yellow");

		ADD_ICON(19,	0,	L"black_stained_glass_pane_top");
		ADD_ICON(19,	1,	L"blue_stained_glass_pane_top");
		ADD_ICON(19,	2,	L"brown_stained_glass_pane_top");
		ADD_ICON(19,	3,	L"cyan_stained_glass_pane_top");
		ADD_ICON(19,	4,	L"gray_stained_glass_pane_top");
		ADD_ICON(19,	5,	L"green_stained_glass_pane_top");
		ADD_ICON(19,	6,	L"light_blue_stained_glass_pane_top");
		ADD_ICON(19,	7,	L"lime_stained_glass_pane_top");
		ADD_ICON(19,	8,	L"magenta_stained_glass_pane_top");
		ADD_ICON(19,	9,	L"orange_stained_glass_pane_top");
		ADD_ICON(19,	10, L"pink_stained_glass_pane_top");
		ADD_ICON(19,	11, L"purple_stained_glass_pane_top");
		ADD_ICON(19,	12, L"red_stained_glass_pane_top");
		ADD_ICON(19,	13, L"light_gray_stained_glass_pane_top");
		ADD_ICON(19,	14, L"white_stained_glass_pane_top");
		ADD_ICON(19,	15, L"yellow_stained_glass_pane_top");
	}
}
