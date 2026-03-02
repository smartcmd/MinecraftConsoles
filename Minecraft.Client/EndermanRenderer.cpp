#include "stdafx.h"
#include "EndermanRenderer.h"
#include "EndermanModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"

EndermanRenderer::EndermanRenderer() : MobRenderer(new EndermanModel(), 0.5f)
{
	model = (EndermanModel *) MobRenderer::model;
	this->setArmor(model);
}

void EndermanRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Boat rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderMan> mob = dynamic_pointer_cast<EnderMan>(_mob);

	model->carrying = mob->getCarryingTile() > 0;
	model->creepy = mob->isCreepy();

	if (mob->isCreepy())
	{
		double d = 0.02;
		x += random.nextGaussian() * d;
		z += random.nextGaussian() * d;
	}

	MobRenderer::render(mob, x, y, z, rot, a);
}

void EndermanRenderer::additionalRendering(shared_ptr<Mob> _mob, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Boat rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderMan> mob = dynamic_pointer_cast<EnderMan>(_mob);

	MobRenderer::additionalRendering(_mob, a);

	if (mob->getCarryingTile() > 0)
	{
		glEnable(GL_RESCALE_NORMAL);
		glPushMatrix();

		float s = 8 / 16.0f;
		glTranslatef(-0 / 16.0f, 11 / 16.0f, -12 / 16.0f);
		s *= 1.00f;
		glRotatef(20, 1, 0, 0);
		glRotatef(45, 0, 1, 0);
		glScalef(s, -s, s);


		if (SharedConstants::TEXTURE_LIGHTING)
		{
			int col = mob->getLightColor(a);
			int u = col % 65536;
			int v = col / 65536;

			glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
			glColor4f(1, 1, 1, 1);
		}

		glColor4f(1, 1, 1, 1);
		bindTexture(TN_TERRAIN);	// 4J was L"/terrain.png"
		tileRenderer->renderTile(Tile::tiles[mob->getCarryingTile()], mob->getCarryingData(), 1);
		glPopMatrix();
		glDisable(GL_RESCALE_NORMAL);
	}
}

int EndermanRenderer::prepareArmor(shared_ptr<Mob> _mob, int layer, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Boat rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<EnderMan> mob = dynamic_pointer_cast<EnderMan>(_mob);

	if (layer != 0) return -1;

	bindTexture(TN_MOB_ENDERMAN_EYES);	// 4J was L"/mob/enderman_eyes.png"
	float br = 1;
	glEnable(GL_BLEND);
	// 4J Stu - We probably don't need to do this on 360 either (as we force it back on the renderer)
	// However we do want it off for other platforms that don't force it on in the render lib CBuff handling
	// Several texture packs have fully transparent bits that break if this is off
#ifdef _XBOX
    glDisable(GL_ALPHA_TEST);
#endif
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_LIGHTING);

	if (mob->isInvisible())	glDepthMask(false);
    else					glDepthMask(true);

	if (SharedConstants::TEXTURE_LIGHTING)
	{
		int col = 0xf0f0;
		int u = col % 65536;
		int v = col / 65536;

		glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
		glColor4f(1, 1, 1, 1);
	}

	glEnable(GL_LIGHTING);
	glColor4f(1, 1, 1, br);
	return 1;
}