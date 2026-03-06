#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.redstone.h"
#include "PressurePlateTile.h"

PressurePlateTile::PressurePlateTile(int id, const wstring &tex, Material *material, Sensitivity sensitivity) : BasePressurePlateTile(id, tex, material)
{
	this->sensitivity = sensitivity;

	// 4J Stu - Move this from base class to use virtual function
	updateShape(getDataForSignal(Redstone::SIGNAL_MAX));
}

int PressurePlateTile::getDataForSignal(int signal)
{
	return signal > 0 ? 1 : 0;
}

int PressurePlateTile::getSignalForData(int data)
{
	return data == 1 ? Redstone::SIGNAL_MAX : 0;
}

int PressurePlateTile::getSignalStrength(Level *level, int x, int y, int z)
{
	vector< shared_ptr<Entity> > *entities = NULL;

	if (sensitivity == everything)		entities = level->getEntities(nullptr, getSensitiveAABB(x, y, z));
	else if (sensitivity == mobs)		entities = level->getEntitiesOfClass(typeid(LivingEntity), getSensitiveAABB(x, y, z));
	else if (sensitivity == players)	entities = level->getEntitiesOfClass(typeid(Player), getSensitiveAABB(x, y, z));
	else								__debugbreak(); // 4J-JEV: We're going to delete something at a random location.

	if (entities != NULL && !entities->empty())
	{
        for (auto& e : *entities)
        {
			if (!e->isIgnoringTileTriggers())
			{
				if (sensitivity != everything) delete entities;
				return Redstone::SIGNAL_MAX;
			}
		}
	}

	if (sensitivity != everything) delete entities;
	return Redstone::SIGNAL_NONE;
}