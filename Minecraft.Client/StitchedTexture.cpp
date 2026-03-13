#include "stdafx.h"
#include "Texture.h"
#include "../Minecraft.World/StringHelpers.h"
#include "ClockTexture.h"
#include "CompassTexture.h"
#include "StitchedTexture.h"
#include "TextureManager.h"

StitchedTexture *StitchedTexture::create(const wstring &name)
{
	// TODO: Generalize?
	if (name.compare(L"clock") == 0)
	{
		return new ClockTexture();
	}
	else if (name.compare(L"compass") == 0)
	{
		return new CompassTexture();
	}
	else
	{
		return new StitchedTexture(name,name);
	}
}

StitchedTexture::StitchedTexture(const wstring &name, const wstring &filename) : name(name)
{
	// 4J Initialisers
	source = nullptr;
	rotated = false;
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	u0 = 0.0f;
	u1 = 0.0f;
	v0 = 0.0f;
	v1 = 0.0f;
	widthTranslation = 0.0f;
	heightTranslation = 0.0f;
	frame = 0;
	subFrame = 0;
	frameOverride = nullptr;
	flags = 0;
	frames = nullptr;
	m_fileName = filename;
}

void StitchedTexture::freeFrameTextures()
{
	if ( frames )
	{
		for (auto& frame : *frames)
		{
			if ( frame )
			{
				TextureManager::getInstance()->unregisterTexture(L"", frame);
				delete frame;
			}
		}
		delete frames;
	}
}

StitchedTexture::~StitchedTexture()
{
	for(auto& frame : *frames)
	{
		if ( frame )
			delete frame;
	}
	delete frames;
}

void StitchedTexture::initUVs(float U0, float V0, float U1, float V1)
{
	u0 = U0;
	u1 = U1;
	v0 = V0;
	v1 = V1;
}

void StitchedTexture::init(Texture *source, vector<Texture *> *frames, int x, int y, int width, int height, bool rotated)
{
	this->source = source;
	this->frames = frames;
	frame = -1; // Force an update of animated textures
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->rotated = rotated;

	float marginX = 0.0f; //0.01f / source->getWidth();
	float marginY = 0.0f; //0.01f / source->getHeight();

	this->u0 = x / static_cast<float>(source->getWidth()) + marginX;
	this->u1 = (x + width) / static_cast<float>(source->getWidth()) - marginX;
	this->v0 = y / static_cast<float>(source->getHeight()) + marginY;
	this->v1 = (y + height) / static_cast<float>(source->getHeight()) - marginY;

#ifndef _CONTENT_PACKAGE
	bool addBreakpoint = false;
	if(addBreakpoint)
	{
		printf("\nTreeTop\n");
		printf("u0 = %f\n", u0);
		printf("u1 = %f\n", u1);
		printf("v0 = %f\n", v0);
		printf("v1 = %f\n", v1);
		printf("\n\n");
	}
#endif

	this->widthTranslation = width / static_cast<float>(SharedConstants::WORLD_RESOLUTION);
	this->heightTranslation = height / static_cast<float>(SharedConstants::WORLD_RESOLUTION);
}

void StitchedTexture::replaceWith(StitchedTexture *texture)
{
	init(texture->source, texture->frames, texture->x, texture->y, texture->width, texture->height, texture->rotated);
}

int StitchedTexture::getX() const
{
	return x;
}

int StitchedTexture::getY() const
{
	return y;
}

int StitchedTexture::getWidth() const
{
	return width;
}

int StitchedTexture::getHeight() const
{
	return height;
}

static const float UVAdjust = (1.0f/16.0f)/256.0f;

float StitchedTexture::getU0(bool adjust/*=false*/) const
{
	return adjust ? ( u0 + UVAdjust ) : u0;
}

float StitchedTexture::getU1(bool adjust/*=false*/) const
{
	return adjust ? ( u1 - UVAdjust ) : u1;
}

float StitchedTexture::getU(double offset, bool adjust/*=false*/) const
{
	float diff = getU1(adjust) - getU0(adjust);
	return getU0(adjust) + (diff * (static_cast<float>(offset) / SharedConstants::WORLD_RESOLUTION));
}

float StitchedTexture::getV0(bool adjust/*=false*/) const
{
	return adjust ? ( v0 + UVAdjust ) : v0;
}

float StitchedTexture::getV1(bool adjust/*=false*/) const
{
	return adjust ? ( v1 - UVAdjust ) : v1;
}

float StitchedTexture::getV(double offset, bool adjust/*=false*/) const
{
	float diff = getV1(adjust) - getV0(adjust);
	return getV0(adjust) + (diff * (static_cast<float>(offset) / SharedConstants::WORLD_RESOLUTION));
}

wstring StitchedTexture::getName() const
{
	return name;
}

int StitchedTexture::getSourceWidth() const
{
	return source->getWidth();
}

int StitchedTexture::getSourceHeight() const
{
	return source->getHeight();
}

void StitchedTexture::cycleFrames()
{
	if (frameOverride != nullptr)
	{
		pair<int, int> current = frameOverride->at(frame);
		subFrame++;
		if (subFrame >= current.second)
		{
			int oldFrame = current.first;
			frame = (frame + 1) % frameOverride->size();
			subFrame = 0;

			current = frameOverride->at(frame);
			int newFrame = current.first;
			if (oldFrame != newFrame && newFrame >= 0 && newFrame < frames->size())
			{
				source->blit(x, y, frames->at(newFrame), rotated);
			}
		}

	}
	else
	{
		int oldFrame = frame;
		frame = (frame + 1) % frames->size();

		if (oldFrame != frame)
		{
			source->blit(x, y, frames->at(this->frame), rotated);
		}
	}
}

Texture *StitchedTexture::getSource()
{
	return source;
}

Texture *StitchedTexture::getFrame(int i)
{
	return frames->at(0);
}

int StitchedTexture::getFrames()
{
	return frames?frames->size():0;
}

/**
* Loads animation frames from a file with the syntax, <code>
* 0,1,2,3,
* 4*10,5*10,
* 4*10,3,2,1,
* 0
* </code> or similar
*
* @param bufferedReader
*/
void StitchedTexture::loadAnimationFrames(BufferedReader *bufferedReader)
{
	if(frameOverride != nullptr)
	{
		delete frameOverride;
		frameOverride = nullptr;
	}
	frame = 0;
	subFrame = 0;

	intPairVector *results = new intPairVector();

	//try {
	wstring line = bufferedReader->readLine();
	while (!line.empty())
	{
		line = trimString(line);
		if (line.length() > 0)
		{
			std::vector<std::wstring> tokens = stringSplit(line, L',');
			//for (String token : tokens)
			for( const auto& token : tokens)
			{
				int multiPos = token.find_first_of('*');
				if (multiPos > 0)
				{
					int frame = _fromString<int>(token.substr(0, multiPos));
					int count = _fromString<int>(token.substr(multiPos + 1));
					results->emplace_back(frame, count);
				}
				else
				{
					int tokenVal = _fromString<int>(token);
					results->emplace_back(tokenVal, 1);
				}
			}
		}
		line = bufferedReader->readLine();
	}
	//} catch (Exception e) {
	//	System.err.println("Failed to read animation info for " + name + ": " + e.getMessage());
	//}

	if (!results->empty() && results->size() < (SharedConstants::TICKS_PER_SECOND * 30))
	{
		frameOverride = results;
	}
	else
	{
		delete results;
	}
}

void StitchedTexture::loadAnimationFrames(const wstring &string)
{
	if(frameOverride != nullptr)
	{
		delete frameOverride;
		frameOverride = nullptr;
	}
	frame = 0;
	subFrame = 0;

	intPairVector *results = new intPairVector();

	std::vector<std::wstring> tokens = stringSplit(trimString(string), L',');

	wstring token;
	for(auto& it : tokens)
	{
		token = trimString(it);
		int multiPos = token.find_first_of('*');
		if (multiPos > 0)
		{
			int frame = _fromString<int>(token.substr(0, multiPos));
			int count = _fromString<int>(token.substr(multiPos + 1));
			results->emplace_back(frame, count);
		}
		else if(!token.empty())
		{
			int tokenVal = _fromString<int>(token);
			results->emplace_back(tokenVal, 1);
		}
	}

	if (!results->empty() && results->size() < (SharedConstants::TICKS_PER_SECOND * 30))
	{
		frameOverride = results;
	}
	else
	{
		delete results;
	}
}

void StitchedTexture::setFlags(int flags)
{
	this->flags = flags;
}

int StitchedTexture::getFlags() const
{
	return this->flags;
}

bool StitchedTexture::hasOwnData()
{
	return true;
}