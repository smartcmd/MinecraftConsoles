#include "stdafx.h"
#include "BookModel.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"
#include "../Minecraft.World/Mth.h"
#include "EnchantTableRenderer.h"

ResourceLocation EnchantTableRenderer::BOOK_LOCATION = ResourceLocation(TN_ITEM_BOOK);

EnchantTableRenderer::EnchantTableRenderer()
{
	bookModel = new BookModel();
}

EnchantTableRenderer::~EnchantTableRenderer()
{
	delete bookModel;
}

void EnchantTableRenderer::render(shared_ptr<TileEntity> _table, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled)
{
	// 4J Convert as we aren't using a templated class
	shared_ptr<EnchantmentTableEntity> table = dynamic_pointer_cast<EnchantmentTableEntity>(_table);

#ifdef __PSVITA__
	// AP - the book pages are made with 0 depth so the front and back polys are at the same location. This can cause z-fighting if culling is disabled which can sometimes happen
	// depending on what object was last seen so make sure culling is always enabled. Should this be a problem for other platforms?
	glEnable(GL_CULL_FACE);
#endif

	glPushMatrix();
	glTranslatef((float) x + 0.5f, (float) y + 12 / 16.0f, (float) z + 0.5f);

	float tt = table->time + a;

	glTranslatef(0, 0.1f + sin(tt * 0.1f) * 0.01f, 0);
	float orot = (table->rot - table->oRot);
	while (orot >= PI)
		orot -= PI * 2;
	while (orot < -PI)
		orot += PI * 2;

	float yRot = table->oRot + orot * a;

	glRotatef(-yRot * 180 / PI, 0, 1, 0);
	glRotatef(80, 0, 0, 1);
	bindTexture(&BOOK_LOCATION); // 4J was "/item/book.png"

	float ff1 = table->oFlip + (table->flip - table->oFlip) * a + 0.25f;
	float ff2 = table->oFlip + (table->flip - table->oFlip) * a + 0.75f;
	ff1 = (ff1 - Mth::fastFloor(ff1)) * 1.6f - 0.3f;
	ff2 = (ff2 - Mth::fastFloor(ff2)) * 1.6f - 0.3f;

	if (ff1 < 0) ff1 = 0;
	if (ff2 < 0) ff2 = 0;
	if (ff1 > 1) ff1 = 1;
	if (ff2 > 1) ff2 = 1;

	float o = table->oOpen + (table->open - table->oOpen) * a;
	glEnable(GL_CULL_FACE);
	bookModel->render(nullptr, tt, ff1, ff2, o, 0, 1 / 16.0f,true);
	glPopMatrix();
}
