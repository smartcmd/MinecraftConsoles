#include "stdafx.h"
#include "UI.h"
#include "UIComponent_Panorama.h"
#include "Minecraft.h"
#include "MultiPlayerLevel.h"
#include "../../../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../../../Minecraft.World/net.minecraft.world.level.storage.h"

UIComponent_Panorama::UIComponent_Panorama(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bShowingDay = true;

	while(!m_hasTickedOnce) tick();
}

wstring UIComponent_Panorama::getMoviePath()
{
	switch( m_parentLayer->getViewport() )
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		m_bSplitscreen = true;
		return L"PanoramaSplit";
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		m_bSplitscreen = false;
		return L"Panorama";
		break;
	}
}

void UIComponent_Panorama::tick()
{
	if(!hasMovie()) return;

	Minecraft *pMinecraft = Minecraft::GetInstance();
	EnterCriticalSection(&pMinecraft->m_setLevelCS);
	if(pMinecraft->level!=nullptr)
	{
		int64_t i64TimeOfDay =0;
		// are we in the Nether? - Leave the time as 0 if we are, so we show daylight
		if(pMinecraft->level->dimension->id==0)
		{
			i64TimeOfDay = pMinecraft->level->getLevelData()->getGameTime() % 24000;
		}

		if(i64TimeOfDay>14000)
		{
			setPanorama(false);
		}
		else
		{
			setPanorama(true);
		}
	}
	else
	{
		setPanorama(true);
	}
	LeaveCriticalSection(&pMinecraft->m_setLevelCS);

	UIScene::tick();
}

void UIComponent_Panorama::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	bool specialViewport =	(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_TOP) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT) ||
		(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT);
	if(m_bSplitscreen && specialViewport)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			yPos = static_cast<S32>(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			xPos = static_cast<S32>(ui.getScreenWidth() / 2);
			break;
		}
		ui.setupRenderPosition(xPos, yPos);

		S32 tileXStart = 0;
		S32 tileYStart = 0;
		S32 tileWidth = width;
		S32 tileHeight = height;

		if((viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT) || (viewport == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT))
		{
			tileHeight = static_cast<S32>(ui.getScreenHeight());
		}
		else
		{
			tileWidth = static_cast<S32>(ui.getScreenWidth());
			tileYStart = static_cast<S32>(ui.getScreenHeight() / 2);
		}

		F32 scaleW = static_cast<F32>(tileXStart + tileWidth) / static_cast<F32>(m_movieWidth);
		F32 scaleH = static_cast<F32>(tileYStart + tileHeight) / static_cast<F32>(m_movieHeight);
		F32 scale = (scaleW > scaleH) ? scaleW : scaleH;
		if(scale < 1.0f) scale = 1.0f;

		IggyPlayerSetDisplaySize( getMovie(), static_cast<S32>(m_movieWidth * scale), static_cast<S32>(m_movieHeight * scale) );

		IggyPlayerDrawTilesStart ( getMovie() );

		m_renderWidth = tileWidth;
		m_renderHeight = tileHeight;
		IggyPlayerDrawTile ( getMovie() ,
			tileXStart ,
			tileYStart ,
			tileXStart + tileWidth ,
			tileYStart + tileHeight ,
			0 );
		IggyPlayerDrawTilesEnd ( getMovie() );
	}
	else
	{
		if(m_bIsReloading) return;
		if(!m_hasTickedOnce || !getMovie()) return;
		ui.setupRenderPosition(0, 0);
		IggyPlayerSetDisplaySize( getMovie(), static_cast<S32>(ui.getScreenWidth()), static_cast<S32>(ui.getScreenHeight()) );
		IggyPlayerDraw( getMovie() );
	}
}

void UIComponent_Panorama::setPanorama(bool isDay)
{
	if(isDay != m_bShowingDay)
	{
		m_bShowingDay = isDay;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_boolean;
		value[0].boolval = isDay;

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcShowPanoramaDay , 1 , value );
	}
}
