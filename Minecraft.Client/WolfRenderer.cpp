#include "stdafx.h"
#include "WolfRenderer.h"
#include "MultiPlayerLocalPlayer.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"

ResourceLocation *WolfRenderer::WOLF_LOCATION = new ResourceLocation(TN_MOB_WOLF);
ResourceLocation *WolfRenderer::WOLF_TAME_LOCATION = new ResourceLocation(TN_MOB_WOLF_TAME);
ResourceLocation *WolfRenderer::WOLF_ANGRY_LOCATION = new ResourceLocation(TN_MOB_WOLF_ANGRY);
ResourceLocation *WolfRenderer::WOLF_COLLAR_LOCATION = new ResourceLocation(TN_MOB_WOLF_COLLAR);

WolfRenderer::WolfRenderer(Model *model, Model *armor, float shadow) : MobRenderer(model, shadow)
{
	setArmor(armor);
}

float WolfRenderer::getBob(shared_ptr<LivingEntity> _mob, float a)
{
	// 4J - dynamic cast required because we aren't using templates/generics in our version
	shared_ptr<Wolf> mob = dynamic_pointer_cast<Wolf>(_mob);

	return mob->getTailAngle();
}

int WolfRenderer::prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a)
{
	if (mob->isInvisibleTo(Minecraft::GetInstance()->player)) return -1; // 4J-JEV: Todo, merge with java fix in '1.7.5'.

	shared_ptr<Wolf> wolf = dynamic_pointer_cast<Wolf>(mob);
	if (layer == 0 && wolf->isWet())
	{
		float brightness = wolf->getBrightness(a) * wolf->getWetShade(a);
		bindTexture(WOLF_LOCATION);
		glColor3f(brightness, brightness, brightness);

		return 1;
	}
	if (layer == 1 && wolf->isTame())
	{
		bindTexture(WOLF_COLLAR_LOCATION);
		float brightness = SharedConstants::TEXTURE_LIGHTING ? 1 : wolf->getBrightness(a);
		int color = wolf->getCollarColor();
		glColor3f(brightness * Sheep::COLOR[color][0], brightness * Sheep::COLOR[color][1], brightness * Sheep::COLOR[color][2]);

		return 1;
	}
	return -1;
}

ResourceLocation *WolfRenderer::getTextureLocation(shared_ptr<Entity> _mob)
{
	shared_ptr<Wolf> mob = dynamic_pointer_cast<Wolf>(_mob);
	if (mob->isTame())
	{
		return WOLF_TAME_LOCATION;
	}
	if (mob->isAngry())
	{
		return WOLF_ANGRY_LOCATION;
	}
	return WOLF_LOCATION;
}
