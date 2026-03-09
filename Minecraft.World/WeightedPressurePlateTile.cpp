#include "stdafx.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.item.h"
#include "Entity.h"
#include "WeightedPressurePlateTile.h"

WeightedPressurePlateTile::WeightedPressurePlateTile(int id, const wstring &tex, Material *material, int maxWeight) : BasePressurePlateTile(id, tex, material)
{
	this->maxWeight = maxWeight;

	// 4J Stu - Move this from base class to use virtual function
	updateShape(getDataForSignal(Redstone::SIGNAL_MAX));
}

int WeightedPressurePlateTile::getSignalStrength(Level *level, int x, int y, int z)
{
	int weightOfEntities = level->getEntitiesOfClass(typeid(Entity), getSensitiveAABB(x, y, z))->size();
	int count = min(weightOfEntities, maxWeight);

	if (count <= 0)
	{
		return 0;
	}
	else
	{
		float pct = min(maxWeight, count) / static_cast<float>(maxWeight);
		return Mth::ceil(pct * Redstone::SIGNAL_MAX);
	}
}

int WeightedPressurePlateTile::getSignalForData(int data)
{
	return data;
}

int WeightedPressurePlateTile::getDataForSignal(int signal)
{
	return signal;
}

int WeightedPressurePlateTile::getTickDelay(Level *level)
{
	return SharedConstants::TICKS_PER_SECOND / 2;
}