#include "stdafx.h"
#include "ItemRenderer.h"
#include "tileRenderer.h"
#include "entityRenderDispatcher.h"
//#include "ItemFrame"
#include "ItemFrameRenderer.h"

#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\net.minecraft.world.entity.Item.h"
#include "..\Minecraft.World\net.minecraft.world.Item.h"
#include "..\Minecraft.World\net.minecraft.world.Item.alchemy.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "Minecraft.h"
#include "..\Minecraft.World\Item.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.h"
#include "CompassTexture.h"
#include "Minimap.h"

void ItemFrameRenderer::registerTerrainTextures(IconRegister *iconRegister)
{
	backTexture = iconRegister->registerIcon(L"itemframe_back");
}

void ItemFrameRenderer::render(shared_ptr<Entity>  _itemframe, double x, double y, double z, float rot, float a) 
{
	// 4J - original version used generics and thus had an input parameter of type EnderCrystal rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<ItemFrame> itemFrame = dynamic_pointer_cast<ItemFrame>(_itemframe);

	glPushMatrix();
	float xOffs = (float) (itemFrame->x - x) - 0.5f;
	float yOffs = (float) (itemFrame->y - y) - 0.5f;
	float zOffs = (float) (itemFrame->z - z) - 0.5f;

	int xt = itemFrame->xTile + Direction::STEP_X[itemFrame->dir];
	int yt = itemFrame->yTile;
	int zt = itemFrame->zTile + Direction::STEP_Z[itemFrame->dir];

	glTranslatef((float) xt - xOffs, (float) yt - yOffs, (float) zt - zOffs);

	drawFrame(itemFrame);
	drawItem(itemFrame);

	glPopMatrix();
}


void ItemFrameRenderer::drawFrame(shared_ptr<ItemFrame> itemFrame) 
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	glPushMatrix();
	entityRenderDispatcher->textures->bindTexture(TN_TERRAIN);
	glRotatef(itemFrame->yRot, 0, 1, 0);

	Tile *wood = Tile::wood;
	float depth = 1.0f / 16.0f;
	float width = 12.0f / 16.0f;
	float widthHalf = width / 2.0f;

	// Back
	glPushMatrix();

	tileRenderer->setFixedShape(0, 0.5f - widthHalf + 1.0f / 16.0f, 0.5f - widthHalf + 1.0f / 16.0f, depth * .5f, 0.5f + widthHalf - 1.0f / 16.0f, 0.5f + widthHalf - 1.0f / 16.0f);
	tileRenderer->setFixedTexture(backTexture);
	tileRenderer->renderTile(wood, 0, 1);
	tileRenderer->clearFixedTexture();
	tileRenderer->clearFixedShape();
	glPopMatrix();

	tileRenderer->setFixedTexture(Tile::wood->getTexture(Facing::UP, TreeTile::BIRCH_TRUNK));

	// Bottom
	glPushMatrix();
	tileRenderer->setFixedShape(0, 0.5f - widthHalf, 0.5f - widthHalf, depth + 0.0001f, depth + 0.5f - widthHalf, 0.5f + widthHalf);
	tileRenderer->renderTile(wood, 0, 1);
	glPopMatrix();

	// Top
	glPushMatrix();
	tileRenderer->setFixedShape(0, 0.5f + widthHalf - depth, 0.5f - widthHalf, depth + 0.0001f, 0.5f + widthHalf, 0.5f + widthHalf);
	tileRenderer->renderTile(wood, 0, 1);
	glPopMatrix();

	// Right
	glPushMatrix();
	tileRenderer->setFixedShape(0, 0.5f - widthHalf, 0.5f - widthHalf, depth, 0.5f + widthHalf, depth + 0.5f - widthHalf);
	tileRenderer->renderTile(wood, 0, 1);
	glPopMatrix();

	// Left
	glPushMatrix();
	tileRenderer->setFixedShape(0, 0.5f - widthHalf, 0.5f + widthHalf - depth, depth, 0.5f + widthHalf, 0.5f + widthHalf);
	tileRenderer->renderTile(wood, 0, 1);
	glPopMatrix();

	tileRenderer->clearFixedShape();
	tileRenderer->clearFixedTexture();

	glPopMatrix();
}

void ItemFrameRenderer::drawItem(shared_ptr<ItemFrame> entity) 
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	shared_ptr<ItemInstance> instance = entity->getItem();
	if (instance == NULL) return;

	shared_ptr<ItemEntity> itemEntity = shared_ptr<ItemEntity>(new ItemEntity(entity->level, 0, 0, 0, instance));
	itemEntity->getItem()->count = 1;
	itemEntity->bobOffs = 0;

	glPushMatrix();

	glTranslatef((-7.25f / 16.0f) * Direction::STEP_X[entity->dir], -0.18f, (-7.25f / 16.0f) * Direction::STEP_Z[entity->dir]);
	glRotatef(180 + entity->yRot, 0, 1, 0);
	glRotatef(-90 * entity->getRotation(), 0, 0, 1);

	switch (entity->getRotation()) 
	{
	case 1:
		glTranslatef(-0.16f, -0.16f, 0);
		break;
	case 2:
		glTranslatef(0, -0.32f, 0);
		break;
	case 3:
		glTranslatef(0.16f, -0.16f, 0);
		break;
	}

	if (itemEntity->getItem()->getItem() == Item::map) 
	{
		entityRenderDispatcher->textures->bindTexture(TN_MISC_MAPBG);
		Tesselator *t = Tesselator::getInstance();

		glRotatef(180, 0, 1, 0);
		glRotatef(180, 0, 0, 1);
		glScalef(1.0f / 256.0f, 1.0f / 256.0f, 1.0f / 256.0f);
		glTranslatef(-65, -107, -3);
		glNormal3f(0, 0, -1);
		t->begin();
		int vo = 7;
		t->vertexUV(0 - vo, 128 + vo, 0, 0, 1);
		t->vertexUV(128 + vo, 128 + vo, 0, 1, 1);
		t->vertexUV(128 + vo, 0 - vo, 0, 1, 0);
		t->vertexUV(0 - vo, 0 - vo, 0, 0, 0);
		t->end();

 		shared_ptr<MapItemSavedData> data = Item::map->getSavedData(itemEntity->getItem(), entity->level);
 		if (data != NULL) 
 		{
			entityRenderDispatcher->itemInHandRenderer->minimap->render(nullptr, entityRenderDispatcher->textures, data, entity->entityId);
 		}
	} 
	else 
	{
		if (itemEntity->getItem()->getItem() == Item::compass)
		{
			CompassTexture *ct = CompassTexture::instance;
			double compassRot = ct->rot;
			double compassRotA = ct->rota;
			ct->rot = 0;
			ct->rota = 0;
			ct->updateFromPosition(entity->level, entity->x, entity->z, Mth::wrapDegrees( (float)(180 + entity->dir * 90) ), false, true);
			ct->rot = compassRot;
			ct->rota = compassRotA;
		}

		EntityRenderDispatcher::instance->render(itemEntity, 0, 0, 0, 0, 0, true);

		if (itemEntity->getItem()->getItem() == Item::compass)
		{
			CompassTexture *ct = CompassTexture::instance;
			ct->cycleFrames();
		}
	}
	
	glPopMatrix();
}

