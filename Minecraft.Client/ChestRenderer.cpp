#include "stdafx.h"
#include "ChestRenderer.h"
#include "ChestModel.h"
#include "LargeChestModel.h"
#include "ModelPart.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

ChestRenderer::ChestRenderer()
{
	chestModel = new ChestModel();
	largeChestModel = new LargeChestModel();
}

ChestRenderer::~ChestRenderer()
{
	delete chestModel;
	delete largeChestModel;
}

void ChestRenderer::render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J Convert as we aren't using a templated class
	shared_ptr<ChestTileEntity> chest = dynamic_pointer_cast<ChestTileEntity>(_chest);

	int data;

	if (!chest->hasLevel())
	{
		data = 0;
	}
	else
	{
		Tile *tile = chest->getTile();
		data = chest->getData();

		if (tile != NULL && data == 0)
		{
			((ChestTile *) tile)->recalcLockDir(chest->getLevel(), chest->x, chest->y, chest->z);
			data = chest->getData();
		}

		chest->checkNeighbors();
	}
	if (chest->n.lock() != NULL || chest->w.lock() != NULL) return;


	ChestModel *model;
	if (chest->e.lock() != NULL || chest->s.lock() != NULL)
	{
		model = largeChestModel;
		bindTexture(TN_TILE_LARGE_CHEST); // 4J Was "/item/largechest.png"
	}
	else
	{
		model = chestModel;
		bindTexture(TN_TILE_CHEST); // 4J Was "/item/chest.png"
	}

	glPushMatrix();
	glEnable(GL_RESCALE_NORMAL);
	//if( setColor ) glColor4f(1, 1, 1, 1);
	if( setColor ) glColor4f(1, 1, 1, alpha);
	glTranslatef((float) x, (float) y + 1, (float) z + 1);
	glScalef(1, -1, -1);

	glTranslatef(0.5f, 0.5f, 0.5f);
	int rot = 0;
	if (data == 2) rot = 180;
	if (data == 3) rot = 0;
	if (data == 4) rot = 90;
	if (data == 5) rot = -90;

	if (data == 2 && chest->e.lock() != NULL)
	{
		glTranslatef(1, 0, 0);
	}
	if (data == 5 && chest->s.lock() != NULL)
	{
		glTranslatef(0, 0, -1);
	}
	glRotatef(rot, 0, 1, 0);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	float open = chest->oOpenness + (chest->openness - chest->oOpenness) * a;
	if (chest->n.lock() != NULL)
	{
		float open2 = chest->n.lock()->oOpenness + (chest->n.lock()->openness - chest->n.lock()->oOpenness) * a;
		if (open2 > open) open = open2;
	}
	if (chest->w.lock() != NULL)
	{
		float open2 = chest->w.lock()->oOpenness + (chest->w.lock()->openness - chest->w.lock()->oOpenness) * a;
		if (open2 > open) open = open2;
	}

	open = 1 - open;
	open = 1 - open * open * open;

	model->lid->xRot = -(open * PI / 2);
	model->render(useCompiled);
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if( setColor ) glColor4f(1, 1, 1, 1);
}
