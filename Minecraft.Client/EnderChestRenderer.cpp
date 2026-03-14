#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "ModelPart.h"
#include "EnderChestRenderer.h"

ResourceLocation EnderChestRenderer::ENDER_CHEST_LOCATION = ResourceLocation(TN_TILE_ENDER_CHEST);

void EnderChestRenderer::render(shared_ptr<TileEntity>  _chest, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J Convert as we aren't using a templated class
	shared_ptr<EnderChestTileEntity> chest = dynamic_pointer_cast<EnderChestTileEntity>(_chest);

	int data = 0;

	if (chest->hasLevel())
	{
		data = chest->getData();
	}

	bindTexture(&ENDER_CHEST_LOCATION);

	glPushMatrix();
	glEnable(GL_RESCALE_NORMAL);
	//glColor4f(1, 1, 1, 1);
	if( setColor ) glColor4f(1, 1, 1, alpha);
	glTranslatef(static_cast<float>(x), static_cast<float>(y) + 1, static_cast<float>(z) + 1);
	glScalef(1, -1, -1);

	glTranslatef(0.5f, 0.5f, 0.5f);
	int rot = 0;
	if (data == 2) rot = 180;
	if (data == 3) rot = 0;
	if (data == 4) rot = 90;
	if (data == 5) rot = -90;

	glRotatef(rot, 0, 1, 0);
	glTranslatef(-0.5f, -0.5f, -0.5f);

	float open = chest->oOpenness + (chest->openness - chest->oOpenness) * a;
	open = 1 - open;
	open = 1 - open * open * open;

	chestModel.lid->xRot = -(open * PI / 2);
	chestModel.render(useCompiled);
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
	if( setColor ) glColor4f(1, 1, 1, 1);
}
