#include "stdafx.h"
#include "FallingTileRenderer.h"
#include "TileRenderer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "EntityRenderDispatcher.h"

FallingTileRenderer::FallingTileRenderer() : EntityRenderer()
{
	tileRenderer = new TileRenderer();
	this->shadowRadius = 0.5f;
}

void FallingTileRenderer::render(shared_ptr<Entity> _tile, double x, double y, double z, float rot, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<FallingTile> tile = dynamic_pointer_cast<FallingTile>(_tile);
	glPushMatrix();
	glTranslatef((float) x, (float) y, (float) z);

	bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
	Tile *tt = Tile::tiles[tile->tile];

	Level *level = tile->getLevel();

	glDisable(GL_LIGHTING);
	glColor4f(1, 1, 1, 1);	// 4J added - this wouldn't be needed in real opengl as the block render has vertex colours and so this isn't use, but our pretend gl always modulates with this
	if (tt == Tile::anvil && tt->getRenderShape() == Tile::SHAPE_ANVIL)
	{
		tileRenderer->level = level;
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		t->offset(-Mth::floor(tile->x) - 0.5f, -Mth::floor(tile->y) - 0.5f, -Mth::floor(tile->z) - 0.5f);
		tileRenderer->tesselateAnvilInWorld((AnvilTile *) tt, Mth::floor(tile->x), Mth::floor(tile->y), Mth::floor(tile->z), tile->data);
		t->offset(0, 0, 0);
		t->end();
	}
	else if (tt == Tile::dragonEgg)
	{
		tileRenderer->level = level;
		Tesselator *t = Tesselator::getInstance();
		t->begin();
		t->offset(-Mth::floor(tile->x) - 0.5f, -Mth::floor(tile->y) - 0.5f, -Mth::floor(tile->z) - 0.5f);
		tileRenderer->tesselateInWorld(tt, Mth::floor(tile->x), Mth::floor(tile->y), Mth::floor(tile->z));
		t->offset(0, 0, 0);
		t->end();
	}
	else if( tt != NULL )
	{
		tileRenderer->setShape(tt);
		tileRenderer->renderBlock(tt, level, Mth::floor(tile->x), Mth::floor(tile->y), Mth::floor(tile->z), tile->data);
	}
	glEnable(GL_LIGHTING);
	glPopMatrix();
}