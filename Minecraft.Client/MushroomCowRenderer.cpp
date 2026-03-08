#include "stdafx.h"
#include "ModelPart.h"
#include "MushroomCowRenderer.h"
#include "TextureAtlas.h"
#include "QuadrupedModel.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"

ResourceLocation MushroomCowRenderer::MOOSHROOM_LOCATION = ResourceLocation(TN_MOB_RED_COW);

MushroomCowRenderer::MushroomCowRenderer(Model *model, float shadow) : MobRenderer(model, shadow)
{
}

void MushroomCowRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - original version used generics and thus had an input parameter of type MushroomCow rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	//shared_ptr<MushroomCow> mob = dynamic_pointer_cast<MushroomCow>(_mob);

	// 4J Stu - No need to do the cast, just pass through as-is
	MobRenderer::render(_mob, x, y, z, rot, a);
}

void MushroomCowRenderer::additionalRendering(shared_ptr<LivingEntity> _mob, float a)
{
	// 4J - original version used generics and thus had an input parameter of type MushroomCow rather than shared_ptr<Mob>  we have here - 
	// do some casting around instead
	shared_ptr<MushroomCow> mob = dynamic_pointer_cast<MushroomCow>(_mob);
	MobRenderer::additionalRendering(mob, a);
	if (mob->isBaby()) return;
	bindTexture(&TextureAtlas::LOCATION_BLOCKS); // 4J was "/terrain.png"
	glEnable(GL_CULL_FACE);
	glPushMatrix();
	glScalef(1, -1, 1);
	glTranslatef(0.2f, 0.4f, 0.5f);
	glRotatef(42, 0, 1, 0);
	tileRenderer->renderTile(Tile::mushroom_red, 0, 1);
	glTranslatef(0.1f, 0, -0.6f);
	glRotatef(42, 0, 1, 0);
	tileRenderer->renderTile(Tile::mushroom_red, 0, 1);
	glPopMatrix();

	glPushMatrix();
	((QuadrupedModel *) model)->head->translateTo(1 / 16.0f);
	glScalef(1, -1, 1);
	glTranslatef(0, 0.75f, -0.2f);
	glRotatef(12, 0, 1, 0);
	tileRenderer->renderTile(Tile::mushroom_red, 0, 1);
	glPopMatrix();

	glDisable(GL_CULL_FACE);
}

ResourceLocation *MushroomCowRenderer::getTextureLocation(shared_ptr<Entity> mob)
{
    return &MOOSHROOM_LOCATION;
}