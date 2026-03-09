#include "stdafx.h"
#include "TileEntityRenderDispatcher.h"
#include "TileEntityRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\PistonPieceEntity.h"
#include "SignRenderer.h"
#include "MobSpawnerRenderer.h"
#include "PistonPieceRenderer.h"
#include "ChestRenderer.h"
#include "EnchantTableRenderer.h"
#include "TheEndPortalRenderer.h"
#include "SkullTileRenderer.h"
#include "EnderChestRenderer.h"
#include "BeaconRenderer.h"

TileEntityRenderDispatcher *TileEntityRenderDispatcher::instance = nullptr;
double TileEntityRenderDispatcher::xOff = 0;
double TileEntityRenderDispatcher::yOff = 0;
double TileEntityRenderDispatcher::zOff = 0;

void TileEntityRenderDispatcher::staticCtor()
{
	instance = new TileEntityRenderDispatcher();
}

TileEntityRenderDispatcher::TileEntityRenderDispatcher()
{
	// 4J -a dded
	font = nullptr;
	textures = nullptr;
	level = nullptr;
	cameraEntity = nullptr;
	playerRotY = 0.0f;
	playerRotX = 0.0f;;
	xPlayer = yPlayer = zPlayer = 0;

	glEnable(GL_LIGHTING);
	renderers[eTYPE_SIGNTILEENTITY] = new SignRenderer();
	renderers[eTYPE_MOBSPAWNERTILEENTITY] = new MobSpawnerRenderer();
	renderers[eTYPE_PISTONPIECEENTITY] = new PistonPieceRenderer();
	renderers[eTYPE_CHESTTILEENTITY] = new ChestRenderer();
	renderers[eTYPE_ENDERCHESTTILEENTITY] = new EnderChestRenderer();
	renderers[eTYPE_ENCHANTMENTTABLEENTITY] = new EnchantTableRenderer();
	renderers[eTYPE_THEENDPORTALTILEENTITY] = new TheEndPortalRenderer();
	renderers[eTYPE_SKULLTILEENTITY] = new SkullTileRenderer();
	renderers[eTYPE_FURNACETILEENTITY] = nullptr;
	renderers[eTYPE_BEACONTILEENTITY] = new BeaconRenderer();
	glDisable(GL_LIGHTING);

	for(auto& renderer : renderers)
	{
		if(renderer.second) renderer.second->init(this);
	}
}

TileEntityRenderer *TileEntityRenderDispatcher::getRenderer(eINSTANCEOF e)
{
	TileEntityRenderer *r = nullptr;
	//TileEntityRenderer *r = renderers[e];
    auto it = renderers.find(e); // 4J Stu - The .at and [] accessors insert elements if they don't exist

    if( it == renderers.end() )
	{
		return nullptr;
	}

/* 4J - not doing this hierarchical search anymore. We need to explicitly add renderers for any eINSTANCEOF type that we want to be able to render
	if (it == renderers.end() && e != TileEntity::_class)
	{
		r = getRenderer(dynamic_cast<TileEntity::Class *>( e->getSuperclass() ));
		// 4J - added condition here to only add if a valid renderer found
		if( r ) renderers.insert( classToTileRendererMap::value_type( e, r ) );
		//assert(false);
	}
	else if(it != renderers.end() && e != TileEntity::_class)
		r = (*it).second;
		*/

	return it->second;
}

bool TileEntityRenderDispatcher::hasRenderer(shared_ptr<TileEntity> e)
{
	return getRenderer(e) != nullptr;
}

TileEntityRenderer *TileEntityRenderDispatcher::getRenderer(shared_ptr<TileEntity> e)
{
	if (e == nullptr) return nullptr;
	return getRenderer(e->GetType());
}

void TileEntityRenderDispatcher::prepare(Level *level, Textures *textures, Font *font, shared_ptr<LivingEntity> player, float a)
{
	if( this->level != level )
	{
		setLevel( level );
	}
	this->textures = textures;
	cameraEntity = player;
	this->font = font;

	playerRotY = player->yRotO + (player->yRot - player->yRotO) * a;
	playerRotX = player->xRotO + (player->xRot - player->xRotO) * a;

	xPlayer = player->xOld + (player->x - player->xOld) * a;
	yPlayer = player->yOld + (player->y - player->yOld) * a;
	zPlayer = player->zOld + (player->z - player->zOld) * a;
}

void TileEntityRenderDispatcher::render(shared_ptr<TileEntity> e, float a, bool setColor/*=true*/)
{
	if (e->distanceToSqr(xPlayer, yPlayer, zPlayer) < e->getViewDistance())
	{
		// 4J - changes brought forward from 1.8.2
		if (SharedConstants::TEXTURE_LIGHTING)
		{
			int col = level->getLightColor(e->x, e->y, e->z, 0);
			int u = col % 65536;
			int v = col / 65536;
			glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
			glColor4f(1, 1, 1, 1);
		}
		else
		{
			float br = level->getBrightness(e->x, e->y, e->z);
			glColor4f(br, br, br, 1);
		}
		render(e, e->x - xOff, e->y - yOff, e->z - zOff, a, setColor);
	}
}

void TileEntityRenderDispatcher::render(shared_ptr<TileEntity> entity, double x, double y, double z, float a, bool setColor/*=true*/, float alpha, bool useCompiled)
{
	TileEntityRenderer *renderer = getRenderer(entity);
	if (renderer != nullptr)
	{
		renderer->render(entity, x, y, z, a, setColor, alpha, useCompiled);
	}
}

void TileEntityRenderDispatcher::setLevel(Level *level)
{
	this->level = level;

	for(auto& renderer : renderers)
	{
		if(renderer.second) renderer.second->onNewLevel(level);
	}
}

double TileEntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
	double xd = x - xPlayer;
	double yd = y - yPlayer;
	double zd = z - zPlayer;
	return xd * xd + yd * yd + zd * zd;
}

Font *TileEntityRenderDispatcher::getFont()
{
	return font;
}
