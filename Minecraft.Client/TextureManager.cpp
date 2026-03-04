#include "stdafx.h"
#include "Stitcher.h"
#include "Texture.h"
#include "TexturePack.h"
#include "TexturePackRepository.h"
#include "Minecraft.h"
#include "TextureManager.h"
#include "..\Minecraft.World\StringHelpers.h"

TextureManager *TextureManager::instance = NULL;

void TextureManager::createInstance()
{
	instance = new TextureManager();
}

TextureManager *TextureManager::getInstance()
{
	return instance;
}

TextureManager::TextureManager()
{
	nextID = 0;
}

int TextureManager::createTextureID()
{
	return nextID++;
}

Texture *TextureManager::getTexture(const wstring &name)
{
	if (stringToIDMap.find(name) != stringToIDMap.end())
	{
		return idToTextureMap.find(stringToIDMap.find(name)->second)->second;
	}

	return NULL;
}

void TextureManager::registerName(const wstring &name, Texture *texture)
{
	stringToIDMap.insert( stringIntMap::value_type( name, texture->getManagerId() ) );

	if (idToTextureMap.find(texture->getManagerId()) == idToTextureMap.end())
	{
		idToTextureMap.insert( intTextureMap::value_type( texture->getManagerId(), texture) );
	}
}

void TextureManager::registerTexture(Texture *texture)
{
	for(AUTO_VAR(it, idToTextureMap.begin()); it != idToTextureMap.end(); ++it)
	{
		if(it->second == texture)
		{
			//Minecraft.getInstance().getLogger().warning("TextureManager.registerTexture called, but this texture has " + "already been registered. ignoring.");
			app.DebugPrintf("TextureManager.registerTexture called, but this texture has already been registered. ignoring.");
			return;
		}
	}

	idToTextureMap.insert( intTextureMap::value_type( texture->getManagerId(), texture ) );
}

void TextureManager::unregisterTexture(const wstring &name, Texture *texture)
{
	AUTO_VAR(it, idToTextureMap.find(texture->getManagerId()));
	if(it != idToTextureMap.end()) idToTextureMap.erase(it);

	AUTO_VAR(it2, stringToIDMap.find(name));
	if(it2 != stringToIDMap.end()) stringToIDMap.erase(it2);
}

Stitcher *TextureManager::createStitcher(const wstring &name)
{
	int maxTextureSize = Minecraft::maxSupportedTextureSize();

	return new Stitcher(name, maxTextureSize, maxTextureSize, true);
}

vector<Texture *> *TextureManager::createTextures(const wstring &filename, bool mipmap)
{
	vector<Texture *> *result = new vector<Texture *>();
	TexturePack *texturePack = Minecraft::GetInstance()->skins->getSelected();
	//try {
	int mode = Texture::TM_CONTAINER; // Most important -- so it doesn't get uploaded to videoram
	int clamp = Texture::WM_WRAP; // 4J Stu - Don't clamp as it causes issues with how we signal non-mipmmapped textures to the pixel shader //Texture::WM_CLAMP;
	int format = Texture::TFMT_RGBA;
	int minFilter = Texture::TFLT_NEAREST;
	int magFilter = Texture::TFLT_NEAREST;

	MemSect(32);
	wstring drive = L"";


	if(texturePack->hasFile(L"res/" + filename,false))
	{
		drive = texturePack->getPath(true);
	}
	else
	{
#ifdef __PS3__
		if(app.GetBootedFromDiscPatch())
		{
			const char *pchTextureName=wstringtofilename(filename);
			char *pchUsrDir = app.GetBDUsrDirPath(pchTextureName);
			wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));
			drive= wstr + L"\\Common\\res\\TitleUpdate\\";
		}
		else
#endif		
		{
			drive = Minecraft::GetInstance()->skins->getDefault()->getPath(true);
		}
	}

	//BufferedImage *image = new BufferedImage(texturePack->getResource(L"/" + filename),false,true,drive); //ImageIO::read(texturePack->getResource(L"/" + filename));

	BufferedImage *image = texturePack->getImageResource(filename, false, true, drive);
	MemSect(0);
	int height = image->getHeight();
	int width = image->getWidth();

	wstring texName = getTextureNameFromPath(filename);

	if (isAnimation(filename, texturePack))
	{
		// TODO: Read this information from the animation file later
		int frameWidth = width;
		int frameHeight = width;

		// This could end as 0 frames
		int frameCount = height / frameWidth;
		for (int i = 0; i < frameCount; i++)
		{
			BufferedImage *subImage = image->getSubimage(0, frameHeight * i, frameWidth, frameHeight);
			Texture *texture = createTexture(texName, mode, frameWidth, frameHeight, clamp, format, minFilter, magFilter, mipmap || image->getData(1) != NULL, subImage);
			delete subImage;
			result->push_back(texture);
		}
	}
	else
	{
		// TODO: Remove this hack -- fix proper rotation support (needed for 'off-aspect textures')
		if (width == height)
		{
			result->push_back(createTexture(texName, mode, width, height, clamp, format, minFilter, magFilter, mipmap || image->getData(1) != NULL, image));
		}
		else
		{
			//Minecraft.getInstance().getLogger().warning("TextureManager.createTexture: Skipping " + filename + " because of broken aspect ratio and not animation");
#ifndef _CONTENT_PACKAGE
			wprintf(L"TextureManager.createTexture: Skipping %ls because of broken aspect ratio and not animation\n", filename.c_str());
#endif
		}
	}
	delete image;


	//return result;
	//} catch (FileNotFoundException e) {
	//	Minecraft.getInstance().getLogger().warning("TextureManager.createTexture called for file " + filename + ", but that file does not exist. Ignoring.");
	//} catch (IOException e) {
	//	Minecraft.getInstance().getLogger().warning("TextureManager.createTexture encountered an IOException when " + "trying to read file " + filename + ". Ignoring.");
	//}
	return result;
}

wstring TextureManager::getTextureNameFromPath(const wstring &filename)
{
	File file(filename);
	return file.getName().substr(0, file.getName().find_last_of(L'.'));
}

bool TextureManager::isAnimation(const wstring &filename, TexturePack *texturePack)
{
	wstring dataFileName = L"/" + filename.substr(0, filename.find_last_of(L'.')) + L".txt";
	bool hasOriginalImage = texturePack->hasFile(L"/" + filename, false);
	return Minecraft::GetInstance()->skins->getSelected()->hasFile(dataFileName, !hasOriginalImage);
}

Texture *TextureManager::createTexture(const wstring &name, int mode, int width, int height, int wrap, int format, int minFilter, int magFilter, bool mipmap, BufferedImage *image)
{
	Texture *newTex = new Texture(name, mode, width, height, wrap, format, minFilter, magFilter, image, mipmap);
	registerTexture(newTex);
	return newTex;
}

Texture *TextureManager::createTexture(const wstring &name, int mode, int width, int height, int format, bool mipmap)
{
	// 4J Stu - Don't clamp as it causes issues with how we signal non-mipmmapped textures to the pixel shader
	//return createTexture(name, mode, width, height, Texture::WM_CLAMP, format, Texture::TFLT_NEAREST, Texture::TFLT_NEAREST, mipmap, NULL);
	return createTexture(name, mode, width, height, Texture::WM_WRAP, format, Texture::TFLT_NEAREST, Texture::TFLT_NEAREST, mipmap, NULL);
}