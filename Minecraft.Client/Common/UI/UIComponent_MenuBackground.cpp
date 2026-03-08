#include "stdafx.h"
#include "UI.h"
#include "UIComponent_MenuBackground.h"

UIComponent_MenuBackground::UIComponent_MenuBackground(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	m_bSplitscreen = false; 
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
}

wstring UIComponent_MenuBackground::getMoviePath()
{	switch( m_parentLayer->getViewport() )
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
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		m_bSplitscreen = false;
		break;
	}

	// We use the fullscreen one even in splitscreen, just draw different parts of it
	return L"MenuBackground";
}

void UIComponent_MenuBackground::render(S32 width, S32 height, C4JRender::eViewportType viewport)
{
	if(m_bSplitscreen)
	{
		S32 xPos = 0;
		S32 yPos = 0;
		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			xPos = (S32)(ui.getScreenWidth() / 2);
			yPos = (S32)(ui.getScreenHeight() / 2);
			break;
		}
		ui.setupRenderPosition(xPos, yPos);

		S32 tileXStart = 0;
		S32 tileYStart = 0;
		S32 tileWidth = width;
		S32 tileHeight = height;

		switch( viewport )
		{
		case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
			tileHeight = (S32)(ui.getScreenHeight());
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
			tileWidth = (S32)(ui.getScreenWidth());
			tileYStart = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
			tileWidth = (S32)(ui.getScreenWidth());
			tileYStart = (S32)(ui.getScreenHeight() / 2);
			break;
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
			tileYStart = (S32)(ui.getScreenHeight() / 2);
			break;
		}

		F32 scaleW = (F32)(tileXStart + tileWidth) / (F32)m_movieWidth;
		F32 scaleH = (F32)(tileYStart + tileHeight) / (F32)m_movieHeight;
		F32 scale = (scaleW > scaleH) ? scaleW : scaleH;
		if(scale < 1.0f) scale = 1.0f;

		IggyPlayerSetDisplaySize( getMovie(), (S32)(m_movieWidth * scale), (S32)(m_movieHeight * scale) );

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
		IggyPlayerSetDisplaySize( getMovie(), (S32)ui.getScreenWidth(), (S32)ui.getScreenHeight() );
		IggyPlayerDraw( getMovie() );
	}
}