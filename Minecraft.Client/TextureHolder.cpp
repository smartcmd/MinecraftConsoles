#include "stdafx.h"
#include "Stitcher.h"
#include "Texture.h"
#include "../Minecraft.World/StringHelpers.h"
#include "TextureHolder.h"

TextureHolder::TextureHolder(Texture *texture)
{
	scale = 1.0f;

	this->texture = texture;
	this->width = texture->getWidth();
	this->height = texture->getHeight();

	this->rotated = smallestFittingMinTexel(height) > smallestFittingMinTexel(width);
}

Texture *TextureHolder::getTexture()
{
	return texture;
}

int TextureHolder::getWidth() const
{
	return rotated ? smallestFittingMinTexel((int) (height * scale)) : smallestFittingMinTexel((int) (width * scale));
}

int TextureHolder::getHeight() const
{
	return rotated ? smallestFittingMinTexel((int) (width * scale)) : smallestFittingMinTexel((int) (height * scale));
}

void TextureHolder::rotate()
{
	rotated = !rotated;
}

bool TextureHolder::isRotated()
{
	return rotated;
}

int TextureHolder::smallestFittingMinTexel(int input) const
{
	return ((input >> Stitcher::MAX_MIPLEVEL) + ((input & (Stitcher::MIN_TEXEL - 1)) == 0 ? 0 : 1)) << Stitcher::MAX_MIPLEVEL;
}

void TextureHolder::setForcedScale(int targetSize)
{
	if (width <= targetSize || height <= targetSize)
	{
		return;
	}

	scale = (float) targetSize / min(width, height);
}

//@Override
wstring TextureHolder::toString()
{
	return L"TextureHolder{width=" + std::to_wstring(width) +	L", height=" + std::to_wstring(height) + L'}';
}

int TextureHolder::compareTo(const TextureHolder *other) const
{
	int result = 0;
	if (this->getHeight() == other->getHeight())
	{
		if (this->getWidth() == other->getWidth())
		{
			if (texture->getName().empty())
			{
				return other->texture->getName().empty() ? 0 : -1;
			}
			return texture->getName().compare(other->texture->getName());
		}
		result = this->getWidth() < other->getWidth() ? 1 : -1;
	}
	else
	{
		result = this->getHeight() < other->getHeight() ? 1 : -1;
	}
	return result;
}