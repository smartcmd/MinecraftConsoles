#pragma once
#include "Textures.h"

typedef arrayWithLength<_TEXTURE_NAME> textureNameArray;
class ResourceLocation
{
private:
	textureNameArray m_texture;
	wstring m_path;
	bool m_preloaded;

public:
	ResourceLocation()
	{
		m_preloaded = false;
		m_path = L"";
	}

	ResourceLocation(_TEXTURE_NAME texture)
	{
		m_texture = textureNameArray(1);
		m_texture[0] = texture;
		m_preloaded = true;
	}

	ResourceLocation(wstring path)
	{
		m_path = path;
		m_preloaded = false;
	}

	ResourceLocation(intArray textures)
	{
		m_texture = textureNameArray(textures.length);
		for(unsigned int i = 0; i < textures.length; ++i)
		{
			m_texture[i] = static_cast<_TEXTURE_NAME>(textures[i]);
		}
		m_preloaded = true;
	}

	~ResourceLocation()
	{
		delete m_texture.data;
	}

	_TEXTURE_NAME getTexture()
	{
		return m_texture[0];
	}

	_TEXTURE_NAME getTexture(int idx)
	{
		return m_texture[idx];
	}

	int getTextureCount()
	{
		return m_texture.length;
	}

	wstring getPath()
	{
		return m_path;
	}

	bool isPreloaded()
	{
		return m_preloaded;
	}
};