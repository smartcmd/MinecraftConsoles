#include "stdafx.h"
#include "StitchSlot.h"
#include "Texture.h"
#include "TextureHolder.h"
#include "TextureManager.h"
#include "StitchedTexture.h"
#include "Stitcher.h"

void Stitcher::_init(const wstring &name, int maxWidth, int maxHeight, bool forcePowerOfTwo, int forcedScale)
{
	this->name = name;
	this->maxWidth = maxWidth;
	this->maxHeight = maxHeight;
	this->forcePowerOfTwo = forcePowerOfTwo;
	this->forcedScale = forcedScale;

	// 4J init
	storageX = 0;
	storageY = 0;
	stitchedTexture = NULL;
}

Stitcher::Stitcher(const wstring &name, int maxWidth, int maxHeight, bool forcePowerOfTwo)
{
	_init(name, maxWidth, maxHeight, forcePowerOfTwo, 0);
}

Stitcher::Stitcher(const wstring &name, int maxWidth, int maxHeight, bool forcePowerOfTwo, int forcedScale)
{
	_init(name, maxWidth, maxHeight, forcePowerOfTwo, forcedScale);
}

int Stitcher::getWidth()
{
	return storageX;
}

int Stitcher::getHeight()
{
	return storageY;
}

void Stitcher::addTexture(TextureHolder *textureHolder)
{
	if (forcedScale > 0)
	{
		textureHolder->setForcedScale(forcedScale);
	}
	texturesToBeStitched.insert(textureHolder);
}

Texture *Stitcher::constructTexture(bool mipmap)
{
	if (forcePowerOfTwo)
	{
		storageX = smallestEncompassingPowerOfTwo(storageX);
		storageY = smallestEncompassingPowerOfTwo(storageY);
	}

	stitchedTexture = TextureManager::getInstance()->createTexture(name, Texture::TM_DYNAMIC, storageX, storageY, Texture::TFMT_RGBA, mipmap);
	stitchedTexture->fill(stitchedTexture->getRect(), 0xffff0000);

	vector<StitchSlot *> *slots = gatherAreas();
	for (int index = 0; index < slots->size(); index++)
	{
		StitchSlot *slot = slots->at(index);
		TextureHolder *textureHolder = slot->getHolder();
		stitchedTexture->blit(slot->getX(), slot->getY(), textureHolder->getTexture(), textureHolder->isRotated());
	}
	delete slots;
	TextureManager::getInstance()->registerName(name, stitchedTexture);

	return stitchedTexture;
}

void Stitcher::stitch()
{
	//TextureHolder[] textureHolders = texturesToBeStitched.toArray(new TextureHolder[texturesToBeStitched.size()]);
	//Arrays.sort(textureHolders);

	stitchedTexture = NULL;

	//for (int i = 0; i < textureHolders.length; i++)
	for( TextureHolder *textureHolder : texturesToBeStitched )
	{
		if (!addToStorage(textureHolder))
		{
			app.DebugPrintf("Stitcher exception!\n");
#ifndef _CONTENT_PACKAGE
			__debugbreak();
#endif
			//throw new StitcherException(textureHolder);
		}
	}
}

vector<StitchSlot *> *Stitcher::gatherAreas()
{
	vector<StitchSlot *> *result = new vector<StitchSlot *>();

	//for (StitchSlot slot : storage)
	for( StitchSlot *slot : storage )
	{
		if ( slot )
			slot->collectAssignments(result);
	}

	return result;
}

// Based on: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
int Stitcher::smallestEncompassingPowerOfTwo(int input)
{
	int result = input - 1;
	result |= result >> 1;
	result |= result >> 2;
	result |= result >> 4;
	result |= result >> 8;
	result |= result >> 16;
	return result + 1;
}

bool Stitcher::addToStorage(TextureHolder *textureHolder)
{
	for (int i = 0; i < storage.size(); i++)
	{
		if (storage.at(i)->add(textureHolder))
		{
			return true;
		}

		// Try rotated
		textureHolder->rotate();
		if (storage.at(i)->add(textureHolder))
		{
			return true;
		}

		// Undo rotation
		textureHolder->rotate();
	}

	return expand(textureHolder);
}

/**
* Expand the current storage to take in account the new texture.
* This should only be called if it didn't fit anywhere.
*
* @param textureHolder
* @return Boolean indicating if it could accommodate for the growth
*/
bool Stitcher::expand(TextureHolder *textureHolder)
{
	int minDistance = min(textureHolder->getHeight(), textureHolder->getWidth());
	bool firstAddition = storageX == 0 && storageY == 0;

	// It couldn't fit, decide which direction to grow to
	bool growOnX;
	if (forcePowerOfTwo)
	{
		int xCurrentSize = smallestEncompassingPowerOfTwo(storageX);
		int yCurrentSize = smallestEncompassingPowerOfTwo(storageY);
		int xNewSize = smallestEncompassingPowerOfTwo(storageX + minDistance);
		int yNewSize = smallestEncompassingPowerOfTwo(storageY + minDistance);

		bool xCanGrow = xNewSize <= maxWidth;
		bool yCanGrow = yNewSize <= maxHeight;

		if (!xCanGrow && !yCanGrow)
		{
			return false;
		}

		// Even if the smallest side fits the larger might not >.>
		int maxDistance = max(textureHolder->getHeight(), textureHolder->getWidth());
		// TODO: This seems wrong ...
		if (firstAddition && !xCanGrow && !(smallestEncompassingPowerOfTwo(storageY + maxDistance) <= maxHeight))
		{
			return false;
		}

		bool xWillGrow = xCurrentSize != xNewSize;
		bool yWillGrow = yCurrentSize != yNewSize;

		if (xWillGrow ^ yWillGrow)
		{
			// Either grows
			//only pick X if it can grow AND it wanted to grow
			// if !xCanGrow then yCanGrow


			growOnX = xWillGrow && xCanGrow;
		}
		else
		{
			// Both or Neither grow -- smallest side wins
			growOnX = xCanGrow && xCurrentSize <= yCurrentSize;
		}
	}
	else
	{
		// We need to figure out to either expand
		bool xCanGrow = (storageX + minDistance) <= maxWidth;
		bool yCanGrow = (storageY + minDistance) <= maxHeight;

		if (!xCanGrow && !yCanGrow)
		{
			return false;
		}

		// Prefer growing on X when its: first addition *or* its the smaller of the two sides
		growOnX = (firstAddition || storageX <= storageY) && xCanGrow;
	}

	StitchSlot *slot;
	if (growOnX)
	{
		if (textureHolder->getWidth() > textureHolder->getHeight())
		{
			textureHolder->rotate();
		}

		// Grow the 'Y' when it has no size yet
		if (storageY == 0)
		{
			storageY = textureHolder->getHeight();
		}

		int newSlotWidth = textureHolder->getWidth();
		// 4J Stu - If we are expanding the texture, then allocate the full powerOfTwo size that we are going to eventually create
		if (forcePowerOfTwo)
		{
			newSlotWidth = smallestEncompassingPowerOfTwo(storageX + newSlotWidth) - storageX;
		}
		slot = new StitchSlot(storageX, 0, newSlotWidth, storageY);
		//storageX += textureHolder->getWidth();
		storageX += newSlotWidth;
	}
	else
	{
		int newSlotHeight = textureHolder->getHeight();
		// 4J Stu - If we are expanding the texture, then allocate the full powerOfTwo size that we are going to eventually create
		if (forcePowerOfTwo)
		{
			newSlotHeight = smallestEncompassingPowerOfTwo(storageY + newSlotHeight) - storageY;
		}

		// grow on Y
		slot = new StitchSlot(0, storageY, storageX, newSlotHeight);
		//storageY += textureHolder->getHeight();
		storageY += newSlotHeight;
	}

	slot->add(textureHolder);
	storage.push_back(slot);

	return true;
}