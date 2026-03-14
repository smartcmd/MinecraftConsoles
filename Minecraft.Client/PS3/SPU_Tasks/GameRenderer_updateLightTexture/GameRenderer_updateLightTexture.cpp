/* SCE CONFIDENTIAL
PlayStation(R)3 Programmer Tool Runtime Library 430.001
* Copyright (C) 2007 Sony Computer Entertainment Inc.
* All Rights Reserved.
*/

/* common headers */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <spu_printf.h>
#include <cell/dma.h>
#include <cell/spurs/job_queue.h>

#include "../Common/DmaData.h"

#include "GameRenderer_updateLightTexture.h"

static const bool sc_verbose = false;

CellSpursJobContext2* g_pSpursJobContext;
	



void updateLightTexture(GameRenderer_updateLightTexture_dataIn& dataIn)
{
	// 4J - we've added separate light textures for all dimensions, and this loop to update them all here
	for(int j = 0; j < 3; j++ )
	{
		int lightPixels[16*16];
		GameRenderer_updateLightTexture_dataIn::LevelData& levelDataIn = dataIn.m_levelData[j];
		if (levelDataIn.m_valid == false) continue;
		for (int i = 0; i < 256; i++)
		{
			float darken = levelDataIn.m_skyDarken * 0.95f + 0.05f;
			float sky = levelDataIn.m_brightnessRamp[i / 16] * darken;
			float block = levelDataIn.m_brightnessRamp[i % 16] * (dataIn.blr * 0.1f + 1.5f);

			if (levelDataIn.m_lightningBoltTime > 0)
			{
				sky = levelDataIn.m_brightnessRamp[i / 16];
			}

			float rs = sky * (levelDataIn.m_skyDarken * 0.65f + 0.35f);
			float gs = sky * (levelDataIn.m_skyDarken * 0.65f + 0.35f);
			float bs = sky;

	/*
	* float dr = darken; dr = dr - dr * dr; System.out.println(dr); if (dr > 0) {
	* gs += dr * 0.5f; rs += dr; }
	*/

			// rs = gs = bs;

			float rb = block;
			float gb = block * ((block * 0.6f + 0.4f) * 0.6f + 0.4f);
			float bb = block * ((block * block) * 0.6f + 0.4f);

			float _r = (rs + rb);
			float _g = (gs + gb);
			float _b = (bs + bb);

			_r = _r * 0.96f + 0.03f;
			_g = _g * 0.96f + 0.03f;
			_b = _b * 0.96f + 0.03f;

			if (levelDataIn.m_dimensionID == 1)
			{
				_r = (0.22f + rb * 0.75f);
				_g = (0.28f + gb * 0.75f);
				_b = (0.25f + bb * 0.75f);
			}

			float brightness = 0.0f;	// 4J - TODO - was mc->options->gamma;
			if (_r > 1) _r = 1;
			if (_g > 1) _g = 1;
			if (_b > 1) _b = 1;

			float ir = 1 - _r;
			float ig = 1 - _g;
			float ib = 1 - _b;
			ir = 1 - (ir * ir * ir * ir);
			ig = 1 - (ig * ig * ig * ig);
			ib = 1 - (ib * ib * ib * ib);
			_r = _r * (1 - brightness) + ir * brightness;
			_g = _g * (1 - brightness) + ig * brightness;
			_b = _b * (1 - brightness) + ib * brightness;


			_r = _r * 0.96f + 0.03f;
			_g = _g * 0.96f + 0.03f;
			_b = _b * 0.96f + 0.03f;


			if (_r > 1) _r = 1;
			if (_g > 1) _g = 1;
			if (_b > 1) _b = 1;
			if (_r < 0) _r = 0;
			if (_g < 0) _g = 0;
			if (_b < 0) _b = 0;

			int a = 255;
			int r = (int) (_r * 255);
			int g = (int) (_g * 255);
			int b = (int) (_b * 255);

			lightPixels[i] = r << 24 | g << 16 | b << 8 | a;
		}
		DmaData_SPU::putAndWait(lightPixels, (uintptr_t)levelDataIn.m_lightPixelsOutput, 16*16*sizeof(int));
	}
}









void cellSpursJobQueueMain(CellSpursJobContext2 *pContext, CellSpursJob256 *pJob)
{
// 	CellSpursTaskId idTask = cellSpursGetTaskId();
	unsigned int idSpu = cellSpursGetCurrentSpuId();

	if(sc_verbose)
		spu_printf("GameRenderer_updateLightTexture [SPU#%u] start\n", idSpu);

	g_pSpursJobContext = pContext;

	uint32_t eaDataIn = pJob->workArea.userData[0];						
 
 	GameRenderer_updateLightTexture_dataIn dataIn;
 	DmaData_SPU::getAndWait(&dataIn, eaDataIn, sizeof(GameRenderer_updateLightTexture_dataIn));
	updateLightTexture(dataIn);

	if(sc_verbose)
	 	spu_printf("GameRenderer_updateLightTexture [SPU#%u] exit\n", idSpu);
}

