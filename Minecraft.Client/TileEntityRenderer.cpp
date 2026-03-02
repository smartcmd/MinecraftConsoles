#include "stdafx.h"
#include "TileEntityRenderer.h"
#include "TileEntityRenderDispatcher.h"

void TileEntityRenderer::bindTexture(ResourceLocation *location)
{
    Textures *t = tileEntityRenderDispatcher->textures;
    if(t != NULL) t->bind(t->loadTexture(location->getTexture()));
}

void TileEntityRenderer::bindTexture(const wstring& urlTexture, ResourceLocation *location)
{
    Textures *t = tileEntityRenderDispatcher->textures;
    if(t != NULL) t->bind(t->loadHttpTexture(urlTexture, location->getTexture()));
}

Level *TileEntityRenderer::getLevel()
{
	return tileEntityRenderDispatcher->level;
}

void TileEntityRenderer::init(TileEntityRenderDispatcher *tileEntityRenderDispatcher)
{
	this->tileEntityRenderDispatcher = tileEntityRenderDispatcher;
}

Font *TileEntityRenderer::getFont()
{
	return tileEntityRenderDispatcher->getFont();
}