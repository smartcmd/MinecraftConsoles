#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
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
#include "TextureMap.h"

const wstring TextureMap::NAME_MISSING_TEXTURE = L"missingno";

TextureMap::TextureMap(int type, const wstring &name, const wstring &path, BufferedImage *missingTexture, bool mipmap) : iconType(type), name(name), path(path), extension(L".png")
{
	this->missingTexture = missingTexture;

	// 4J Initialisers
	missingPosition = nullptr;
	stitchResult = nullptr;

	m_mipMap = mipmap;
}

void TextureMap::stitch()
{
	texturesToRegister.clear();

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

	for(auto& it : texturesByName)
	{
		delete it.second;
	}
	texturesByName.clear();
	animatedTextures.clear();

	// Prep missing texture -- anything that has no resources will get pointed at this one
	Texture *missingTex = TextureManager::getInstance()->createTexture(NAME_MISSING_TEXTURE, Texture::TM_CONTAINER, missingTexture->getWidth(), missingTexture->getHeight(), Texture::WM_CLAMP, Texture::TFMT_RGBA, Texture::TFLT_NEAREST, Texture::TFLT_NEAREST, m_mipMap, missingTexture);
	TextureHolder *missingHolder = new TextureHolder(missingTex);

	stitcher->addTexture(missingHolder);
	vector<Texture *> *missingVec = new vector<Texture *>();
	missingVec->push_back(missingTex);
	textures.insert( unordered_map<TextureHolder *, vector<Texture *> * >::value_type( missingHolder, missingVec ));

	// Extract frames from textures and add them to the stitchers
	//for (final String name : texturesToRegister.keySet())
	for(auto& it : texturesToRegister)
	{
		wstring name = it.first;

		wstring filename = path + name + extension;

		// TODO: [EB] Put the frames into a proper object, not this inside out hack
		vector<Texture *> *frames = TextureManager::getInstance()->createTextures(filename, m_mipMap);

		if (frames == nullptr || frames->empty())
		{
			continue; // Couldn't load a texture, skip it
		}

		TextureHolder *holder = new TextureHolder(frames->at(0));
		stitcher->addTexture(holder);

		// Store frames
		textures.insert( unordered_map<TextureHolder *, vector<Texture *> * >::value_type( holder, frames ) );
	}

	// Stitch!
	//try {
		stitcher->stitch();
	//} catch (StitcherException e) {
	//	throw e;
		// TODO: [EB] Retry mechanism
	//}

	// Create the final image
	stitchResult = stitcher->constructTexture(m_mipMap);

	// Extract all the final positions and store them
    auto areas = stitcher->gatherAreas();
    //for (StitchSlot slot : stitcher.gatherAreas())
	for(auto& slot : *areas)
	{
		TextureHolder *textureHolder = slot->getHolder();

		Texture *texture = textureHolder->getTexture();
		wstring textureName = texture->getName();

		vector<Texture *> *frames = textures.find(textureHolder)->second;

		StitchedTexture *stored = nullptr;

        auto itTex = texturesToRegister.find(textureName);
        if(itTex != texturesToRegister.end() ) stored = itTex->second;

		// [EB]: What is this code for? debug warnings for when during transition?
		bool missing = false;
		if (stored)
		{
			missing = true;
			stored = StitchedTexture::create(textureName);

			if (textureName.compare(NAME_MISSING_TEXTURE)!=0)
			{
				//Minecraft::getInstance()->getLogger().warning("Couldn't find premade icon for " + textureName + " doing " + name);
#ifndef _CONTENT_PACKAGE
				wprintf(L"Couldn't find premade icon for %ls doing %ls\n", textureName.c_str(), name.c_str() );
#endif
			}
		}

		stored->init(stitchResult, frames, slot->getX(), slot->getY(), textureHolder->getTexture()->getWidth(), textureHolder->getTexture()->getHeight(), textureHolder->isRotated());

		texturesByName.insert( stringStitchedTextureMap::value_type(textureName, stored) );
		if (!missing) texturesToRegister.erase(textureName);

		if (frames->size() > 1)
		{
			animatedTextures.push_back(stored);

			wstring animationDefinitionFile = textureName + L".txt";

			TexturePack *texturePack = Minecraft::GetInstance()->skins->getSelected();
			bool requiresFallback = !texturePack->hasFile(L"\\" + textureName + L".png", false);
				InputStream *fileStream = texturePack->getResource(L"\\" + path + animationDefinitionFile, requiresFallback);

				//Minecraft::getInstance()->getLogger().info("Found animation info for: " + animationDefinitionFile);
#ifndef _CONTENT_PACKAGE
				wprintf(L"Found animation info for: %ls\n", animationDefinitionFile.c_str() );
#endif
				InputStreamReader isr(fileStream);
				BufferedReader br(&isr);
				stored->loadAnimationFrames(&br);
				delete fileStream;
		}
	}
	delete areas;

	missingPosition = texturesByName.find(NAME_MISSING_TEXTURE)->second;

	//for (StitchedTexture texture : texturesToRegister.values())
	for(auto& it : texturesToRegister)
	{
		StitchedTexture *texture = it.second;
		texture->replaceWith(missingPosition);
	}

	stitchResult->writeAsPNG(L"debug.stitched_" + name + L".png");
	stitchResult->updateOnGPU();
}

StitchedTexture *TextureMap::getTexture(const wstring &name)
{
	StitchedTexture *result = texturesByName.find(name)->second;
	if (result == nullptr) result = missingPosition;
	return result;
}

void TextureMap::cycleAnimationFrames()
{
	//for (StitchedTexture texture : animatedTextures)
	for(auto& texture : animatedTextures)
	{
		if ( texture )
			texture->cycleFrames();
	}
}

Texture *TextureMap::getStitchedTexture()
{
	return stitchResult;
}

// 4J Stu - register is a reserved keyword in C++
Icon *TextureMap::registerIcon(const wstring &name)
{
	if (name.empty())
	{
		app.DebugPrintf("Don't register nullptr\n");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
		//new RuntimeException("Don't register null!").printStackTrace();
	}

	// TODO: [EB]: Why do we allow multiple registrations?
	StitchedTexture *result = nullptr;
    auto it = texturesToRegister.find(name);
    if(it != texturesToRegister.end()) result = it->second;

	if (result == nullptr)
	{
		result = StitchedTexture::create(name);
		texturesToRegister.insert( stringStitchedTextureMap::value_type(name, result) );
	}

	return result;
}

int TextureMap::getIconType()
{
	return iconType;
}

Icon *TextureMap::getMissingIcon()
{
	return missingPosition;
}