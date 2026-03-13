#include "stdafx.h"
#include "../Minecraft.World/net.minecraft.world.entity.npc.h"
#include "VillagerModel.h"
#include "VillagerRenderer.h"

ResourceLocation VillagerRenderer::VILLAGER_LOCATION = ResourceLocation(TN_MOB_VILLAGER_VILLAGER);
ResourceLocation VillagerRenderer::VILLAGER_FARMER_LOCATION = ResourceLocation(TN_MOB_VILLAGER_FARMER);
ResourceLocation VillagerRenderer::VILLAGER_LIBRARIAN_LOCATION = ResourceLocation(TN_MOB_VILLAGER_LIBRARIAN);
ResourceLocation VillagerRenderer::VILLAGER_PRIEST_LOCATION = ResourceLocation(TN_MOB_VILLAGER_PRIEST);
ResourceLocation VillagerRenderer::VILLAGER_SMITH_LOCATION = ResourceLocation(TN_MOB_VILLAGER_SMITH);
ResourceLocation VillagerRenderer::VILLAGER_BUTCHER_LOCATION = ResourceLocation(TN_MOB_VILLAGER_BUTCHER);

VillagerRenderer::VillagerRenderer() : MobRenderer(new VillagerModel(0), 0.5f)
{
	villagerModel = static_cast<VillagerModel *>(model);
}

int VillagerRenderer::prepareArmor(shared_ptr<LivingEntity> villager, int layer, float a)
{
	return -1;
}

void VillagerRenderer::render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a)
{
	MobRenderer::render(mob, x, y, z, rot, a);
}

ResourceLocation *VillagerRenderer::getTextureLocation(shared_ptr<Entity> _mob)
{
	shared_ptr<Villager> mob = dynamic_pointer_cast<Villager>(_mob);

	switch (mob->getProfession())
	{
	case Villager::PROFESSION_FARMER:
		return &VILLAGER_FARMER_LOCATION;
	case Villager::PROFESSION_LIBRARIAN:
		return &VILLAGER_LIBRARIAN_LOCATION;
	case Villager::PROFESSION_PRIEST:
		return &VILLAGER_PRIEST_LOCATION;
	case Villager::PROFESSION_SMITH:
		return &VILLAGER_SMITH_LOCATION;
	case Villager::PROFESSION_BUTCHER:
		return &VILLAGER_BUTCHER_LOCATION;
	default:
		return &VILLAGER_LOCATION;
	}
}

void VillagerRenderer::additionalRendering(shared_ptr<LivingEntity> mob, float a)
{
	MobRenderer::additionalRendering(mob, a);
}

void VillagerRenderer::scale(shared_ptr<LivingEntity> _mob, float a)
{
	// 4J - original version used generics and thus had an input parameter of type Blaze rather than shared_ptr<Entity>  we have here - 
	// do some casting around instead
	shared_ptr<Villager> mob = dynamic_pointer_cast<Villager>(_mob);
	float s = 15 / 16.0f;
	if (mob->getAge() < 0)
	{
		s *= 0.5;
		shadowRadius = 0.25f;
	} else shadowRadius = 0.5f;
	glScalef(s, s, s);
}