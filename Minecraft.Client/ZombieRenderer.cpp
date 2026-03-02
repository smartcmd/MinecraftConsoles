#include "stdafx.h"
#include "ZombieModel.h"
#include "VillagerZombieModel.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "ZombieRenderer.h"

ZombieRenderer::ZombieRenderer() : HumanoidMobRenderer(new ZombieModel(), .5f, 1.0f)
{
	modelVersion = 1;
	defaultModel = humanoidModel;
	villagerModel = new VillagerZombieModel();

	defaultArmorParts1 = NULL;
	defaultArmorParts2 = NULL;

	villagerArmorParts1 = NULL;
	villagerArmorParts2 = NULL;

	createArmorParts();
}

void ZombieRenderer::createArmorParts()
{
	delete armorParts1;
	delete armorParts2;

	armorParts1 = new ZombieModel(1.0f, true);
	armorParts2 = new ZombieModel(0.5f, true);

	defaultArmorParts1 = armorParts1;
	defaultArmorParts2 = armorParts2;

	villagerArmorParts1 = new VillagerZombieModel(1.0f, 0, true);
	villagerArmorParts2 = new VillagerZombieModel(0.5f, 0, true);
}

int ZombieRenderer::prepareArmor(shared_ptr<Mob> _mob, int layer, float a)
{
	shared_ptr<Zombie> mob = dynamic_pointer_cast<Zombie>(_mob);
	swapArmor(mob);
	return HumanoidMobRenderer::prepareArmor(_mob, layer, a);
}

void ZombieRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	shared_ptr<Zombie> mob = dynamic_pointer_cast<Zombie>(_mob);
	swapArmor(mob);
	HumanoidMobRenderer::render(_mob, x, y, z, rot, a);
}

void ZombieRenderer::additionalRendering(shared_ptr<Mob> _mob, float a)
{
	shared_ptr<Zombie> mob = dynamic_pointer_cast<Zombie>(_mob);
	swapArmor(mob);
	HumanoidMobRenderer::additionalRendering(_mob, a);
}

void ZombieRenderer::swapArmor(shared_ptr<Zombie> mob)
{
	if (mob->isVillager())
	{
		//if (modelVersion != villagerModel->version())
		//{
		//	villagerModel = new VillagerZombieModel();
		//	modelVersion = villagerModel->version();
		//	villagerArmorParts1 = new VillagerZombieModel(1.0f, 0, true);
		//	villagerArmorParts2 = new VillagerZombieModel(0.5f, 0, true);
		//}
		model = villagerModel;
		armorParts1 = villagerArmorParts1;
		armorParts2 = villagerArmorParts2;
	}
	else
	{
		model = defaultModel;
		armorParts1 = defaultArmorParts1;
		armorParts2 = defaultArmorParts2;
	}

	humanoidModel = (HumanoidModel *) model;
}

void ZombieRenderer::setupRotations(shared_ptr<Mob> _mob, float bob, float bodyRot, float a)
{
	shared_ptr<Zombie> mob = dynamic_pointer_cast<Zombie>(_mob);
	if (mob->isConverting())
	{
		bodyRot += (float) (cos(mob->tickCount * 3.25) * PI * .25f);
	}
	HumanoidMobRenderer::setupRotations(mob, bob, bodyRot, a);
}