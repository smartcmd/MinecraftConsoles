#include "stdafx.h"
#include "Minecraft.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "MultiplayerLocalPlayer.h"
#include "..\Minecraft.World\JavaMath.h"
#include "Texture.h"
#include "ClockTexture.h"

ClockTexture::ClockTexture() : StitchedTexture(L"clock", L"clock")
{
	rot = rota = 0.0;
	m_dataTexture = NULL;
	m_iPad = XUSER_INDEX_ANY;
}

ClockTexture::ClockTexture(int iPad, ClockTexture *dataTexture) : StitchedTexture(L"clock", L"clock")
{
	rot = rota = 0.0;
	m_dataTexture = dataTexture;
	m_iPad = iPad;
}

void ClockTexture::cycleFrames()
{

	Minecraft *mc = Minecraft::GetInstance();

	double rott = 0;
	if (m_iPad >= 0 && m_iPad < XUSER_MAX_COUNT && mc->level != NULL && mc->localplayers[m_iPad] != NULL)
	{
		float time = mc->localplayers[m_iPad]->level->getTimeOfDay(1);
		rott = time;
		if (!mc->localplayers[m_iPad]->level->dimension->isNaturalDimension())
		{
			rott = Math::random();
		}
	}
	else
	{
		// 4J Stu - For the static version, pretend we are already on a frame other than 0
		frame = 1;
	}

	double rotd = rott - rot;
	while (rotd < -.5)
		rotd += 1.0;
	while (rotd >= .5)
		rotd -= 1.0;
	if (rotd < -1) rotd = -1;
	if (rotd > 1) rotd = 1;
	rota += rotd * 0.1;
	rota *= 0.8;

	rot += rota;
	
	// 4J Stu - We share data with another texture
	if(m_dataTexture != NULL)
	{
		int newFrame = (int) ((rot + 1.0) * m_dataTexture->frames->size()) % m_dataTexture->frames->size();
		while (newFrame < 0)
		{
			newFrame = (newFrame + m_dataTexture->frames->size()) % m_dataTexture->frames->size();
		}
		if (newFrame != frame)
		{
			frame = newFrame;
			m_dataTexture->source->blit(x, y, m_dataTexture->frames->at(this->frame), rotated);
		}
	}
	else
	{
		int newFrame = (int) ((rot + 1.0) * frames->size()) % frames->size();
		while (newFrame < 0)
		{
			newFrame = (newFrame + frames->size()) % frames->size();
		}
		if (newFrame != frame)
		{
			frame = newFrame;
			source->blit(x, y, frames->at(this->frame), rotated);
		}
	}
}

int ClockTexture::getSourceWidth() const
{
	return source->getWidth();
}

int ClockTexture::getSourceHeight() const
{
	return source->getHeight();
}

int ClockTexture::getFrames()
{
	if(m_dataTexture == NULL)
	{
		return StitchedTexture::getFrames();
	}
	else
	{
		return m_dataTexture->getFrames();
	}
}

void ClockTexture::freeFrameTextures()
{
	if(m_dataTexture == NULL)
	{
		StitchedTexture::freeFrameTextures();
	}
}

bool ClockTexture::hasOwnData()
{
	return m_dataTexture == NULL;
}