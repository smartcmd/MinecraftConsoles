#include "stdafx.h"
#include "Minecraft.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "MultiplayerLocalPlayer.h"
#include "..\Minecraft.World\JavaMath.h"
#include "Texture.h"
#include "CompassTexture.h"

CompassTexture *CompassTexture::instance = NULL;

CompassTexture::CompassTexture() : StitchedTexture(L"compass",L"compass")
{
	instance = this;

	m_dataTexture = NULL;
	m_iPad = XUSER_INDEX_ANY;

	rot = rota = 0.0;
}

CompassTexture::CompassTexture(int iPad, CompassTexture *dataTexture) : StitchedTexture(L"compass",L"compass")
{
	m_dataTexture = dataTexture;
	m_iPad = iPad;

	rot = rota = 0.0;
}

void CompassTexture::cycleFrames()
{
	Minecraft *mc = Minecraft::GetInstance();

	if (m_iPad >= 0 && m_iPad < XUSER_MAX_COUNT && mc->level != NULL && mc->localplayers[m_iPad] != NULL)
	{
		updateFromPosition(mc->localplayers[m_iPad]->level, mc->localplayers[m_iPad]->x, mc->localplayers[m_iPad]->z, mc->localplayers[m_iPad]->yRot, false, false);
	}
	else
	{
		frame = 1;
		updateFromPosition(NULL, 0, 0, 0, false, true);
	}
}

void CompassTexture::updateFromPosition(Level *level, double x, double z, double yRot, bool noNeedle, bool instant)
{
	double rott = 0;
	if (level != NULL && !noNeedle)
	{
		Pos *spawnPos = level->getSharedSpawnPos();
		double xa = spawnPos->x - x;
		double za = spawnPos->z - z;
		delete spawnPos;
		yRot = (int)yRot % 360;
		rott = -((yRot - 90) * PI / 180 - atan2(za, xa));
		if (!level->dimension->isNaturalDimension())
		{
			rott = Math::random() * PI * 2;
		}
	}

	if (instant)
	{
		rot = rott;
	}
	else
	{
		double rotd = rott - rot;
		while (rotd < -PI)
			rotd += PI * 2;
		while (rotd >= PI)
			rotd -= PI * 2;
		if (rotd < -1) rotd = -1;
		if (rotd > 1) rotd = 1;
		rota += rotd * 0.1;
		rota *= 0.8;
		rot += rota;
	}

	// 4J Stu - We share data with another texture
	if(m_dataTexture != NULL)
	{
		int newFrame = (int) (((rot / (PI * 2)) + 1.0) * m_dataTexture->frames->size()) % m_dataTexture->frames->size();
		while (newFrame < 0)
		{
			newFrame = (newFrame + m_dataTexture->frames->size()) % m_dataTexture->frames->size();
		}
		if (newFrame != frame)
		{
			frame = newFrame;
			m_dataTexture->source->blit(this->x, this->y, m_dataTexture->frames->at(this->frame), rotated);
		}
	}
	else
	{
		int newFrame = (int) (((rot / (PI * 2)) + 1.0) * frames->size()) % frames->size();
		while (newFrame < 0)
		{
			newFrame = (newFrame + frames->size()) % frames->size();
		}
		if (newFrame != frame)
		{
			frame = newFrame;
			source->blit(this->x, this->y, frames->at(this->frame), rotated);
		}
	}
}

int CompassTexture::getSourceWidth() const
{
	return source->getWidth();
}

int CompassTexture::getSourceHeight() const
{
	return source->getHeight();
}

int CompassTexture::getFrames()
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

void CompassTexture::freeFrameTextures()
{
	if(m_dataTexture == NULL)
	{
		StitchedTexture::freeFrameTextures();
	}
}

bool CompassTexture::hasOwnData()
{
	return m_dataTexture == NULL;
}