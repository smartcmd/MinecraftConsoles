#pragma once

// Shared split-screen UI helpers to avoid duplicating viewport math
// across HUD, Chat, Tooltips, and container menus.

// Compute the raw viewport rectangle for a given viewport type.
inline void GetViewportRect(F32 screenW, F32 screenH, C4JRender::eViewportType viewport,
	F32 &originX, F32 &originY, F32 &viewW, F32 &viewH)
{
	originX = originY = 0;
	viewW = screenW;
	viewH = screenH;
	switch(viewport)
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		viewH = screenH * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		originY = screenH * 0.5f; viewH = screenH * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		viewW = screenW * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		originX = screenW * 0.5f; viewW = screenW * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		viewW = screenW * 0.5f; viewH = screenH * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		originX = screenW * 0.5f; viewW = screenW * 0.5f; viewH = screenH * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		originY = screenH * 0.5f; viewW = screenW * 0.5f; viewH = screenH * 0.5f; break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		originX = screenW * 0.5f; originY = screenH * 0.5f;
		viewW = screenW * 0.5f; viewH = screenH * 0.5f; break;
	default: break;
	}
}

// Fit a 16:9 rectangle inside the given dimensions.
inline void Fit16x9(F32 viewW, F32 viewH, S32 &fitW, S32 &fitH, S32 &offsetX, S32 &offsetY)
{
	const F32 kAspect = 16.0f / 9.0f;
	if(viewW / viewH > kAspect)
	{
		fitH = (S32)viewH;
		fitW = (S32)(viewH * kAspect);
	}
	else
	{
		fitW = (S32)viewW;
		fitH = (S32)(viewW / kAspect);
	}
	offsetX = (S32)((viewW - fitW) * 0.5f);
	offsetY = (S32)((viewH - fitH) * 0.5f);
}

// Convenience: just fit 16:9 dimensions, ignore offsets.
inline void Fit16x9(S32 &width, S32 &height)
{
	S32 offX, offY;
	Fit16x9((F32)width, (F32)height, width, height, offX, offY);
}

// Compute the uniform scale and tileYStart for split-screen tile rendering.
// Used by HUD, Chat, and Tooltips to scale the SWF movie to cover the viewport tile.
inline void ComputeTileScale(S32 tileWidth, S32 tileHeight, S32 movieWidth, S32 movieHeight,
	bool needsYTile, F32 &outScale, S32 &outTileYStart)
{
	F32 scaleW = (F32)tileWidth / (F32)movieWidth;
	F32 scaleH = (F32)tileHeight / (F32)movieHeight;
	F32 scale = (scaleW > scaleH) ? scaleW : scaleH;
	if(scale < 1.0f) scale = 1.0f;

	outTileYStart = 0;
	if(needsYTile)
	{
		S32 dispH = (S32)(movieHeight * scale);
		outTileYStart = dispH - tileHeight;
		if(outTileYStart < 0) outTileYStart = 0;
		scaleH = (F32)(outTileYStart + tileHeight) / (F32)movieHeight;
		scale = (scaleW > scaleH) ? scaleW : scaleH;
		if(scale < 1.0f) scale = 1.0f;
	}

	outScale = scale;
}

// Compute the render offset to center split-screen SWF content in the viewport.
// Used by Chat and Tooltips (HUD uses repositionHud instead).
inline void ComputeSplitContentOffset(C4JRender::eViewportType viewport, S32 movieWidth, S32 movieHeight,
	F32 scale, S32 tileWidth, S32 tileHeight, S32 tileYStart,
	S32 &outXOffset, S32 &outYOffset)
{
	S32 contentCenterX, contentCenterY;
	if(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT || viewport == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT)
	{
		contentCenterX = (S32)(movieWidth * scale / 4);
		contentCenterY = (S32)(movieHeight * scale / 2);
	}
	else if(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_TOP || viewport == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM)
	{
		contentCenterX = (S32)(movieWidth * scale / 2);
		contentCenterY = (S32)(movieHeight * scale * 3 / 4);
	}
	else
	{
		contentCenterX = (S32)(movieWidth * scale / 4);
		contentCenterY = (S32)(movieHeight * scale * 3 / 4);
	}

	outXOffset = 0;
	outYOffset = 0;
	if(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT || viewport == C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT || viewport == C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT)
		outXOffset = -(tileWidth / 2 - contentCenterX);
	if(viewport == C4JRender::VIEWPORT_TYPE_SPLIT_TOP || viewport == C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT || viewport == C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT)
		outYOffset = -(tileHeight / 2 - (contentCenterY - tileYStart));
}
