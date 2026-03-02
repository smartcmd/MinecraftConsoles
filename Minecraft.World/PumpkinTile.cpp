#include "stdafx.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "PumpkinTile.h"
#include "Mob.h"
#include "SnowMan.h"
#include "MobCategory.h"

const wstring PumpkinTile::TEXTURE_FACE = L"pumpkin_face";
const wstring PumpkinTile::TEXTURE_LANTERN = L"pumpkin_jack";

PumpkinTile::PumpkinTile(int id, bool lit) : DirectionalTile(id, Material::vegetable)
{
    iconTop = NULL;
	iconFace = NULL;
    setTicking(true);
    this->lit = lit;
}

Icon *PumpkinTile::getTexture(int face, int data)
{
    if (face == Facing::UP) return iconTop;
    if (face == Facing::DOWN) return iconTop;

    if (data == DIR_NORTH && face == Facing::NORTH) return iconFace;
    if (data == DIR_EAST && face == Facing::EAST) return iconFace;
    if (data == DIR_SOUTH && face == Facing::SOUTH) return iconFace;
    if (data == DIR_WEST && face == Facing::WEST) return iconFace;

    else return icon;
}

void PumpkinTile::onPlace(Level *level, int x, int y, int z)
{
	Tile::onPlace(level, x, y, z);
    if (level->getTile(x, y - 1, z) == Tile::snow_Id && level->getTile(x, y - 2, z) == Tile::snow_Id)
	{
        if (!level->isClientSide)
		{
			// 4J - added limit of number of snowmen that can be spawned
			if( level->canCreateMore( eTYPE_SNOWMAN, Level::eSpawnType_Egg) )
			{
				level->setTileNoUpdate(x, y, z, 0);
				level->setTileNoUpdate(x, y - 1, z, 0);
				level->setTileNoUpdate(x, y - 2, z, 0);
				shared_ptr<SnowMan> snowMan = shared_ptr<SnowMan>(new SnowMan(level));
				snowMan->moveTo(x + 0.5, y - 1.95, z + 0.5, 0, 0);
				level->addEntity(snowMan);

                level->tileUpdated(x, y, z, 0);
                level->tileUpdated(x, y - 1, z, 0);
                level->tileUpdated(x, y - 2, z, 0);
			}
			else
			{
				// If we can't spawn it, at least give the resources back
				Tile::spawnResources(level, x, y, z, level->getData(x, y, z), 0);
				Tile::tiles[Tile::snow_Id]->spawnResources(level, x, y - 1, z, level->getData(x, y - 1, z), 0);
				Tile::tiles[Tile::snow_Id]->spawnResources(level, x, y - 2, z, level->getData(x, y - 2, z), 0);
				level->setTile(x, y, z, 0);
				level->setTile(x, y - 1, z, 0);
				level->setTile(x, y - 2, z, 0);
			}
        }
        for (int i = 0; i < 120; i++)
		{
            level->addParticle(eParticleType_snowshovel, x + level->random->nextDouble(), y - 2 + level->random->nextDouble() * 2.5, z + level->random->nextDouble(), 0, 0, 0);
        }
	}
	else if (level->getTile(x, y - 1, z) == Tile::ironBlock_Id && level->getTile(x, y - 2, z) == Tile::ironBlock_Id)
	{
		bool xArms = level->getTile(x - 1, y - 1, z) == Tile::ironBlock_Id && level->getTile(x + 1, y - 1, z) == Tile::ironBlock_Id;
		bool zArms = level->getTile(x, y - 1, z - 1) == Tile::ironBlock_Id && level->getTile(x, y - 1, z + 1) == Tile::ironBlock_Id;
		if (xArms || zArms)
		{
			if (!level->isClientSide)
			{
				// 4J - added limit of number of golems that can be spawned
				if( level->canCreateMore( eTYPE_VILLAGERGOLEM, Level::eSpawnType_Egg) )
				{
					level->setTileNoUpdate(x, y, z, 0);
					level->setTileNoUpdate(x, y - 1, z, 0);
					level->setTileNoUpdate(x, y - 2, z, 0);
					if (xArms)
					{
						level->setTileNoUpdate(x - 1, y - 1, z, 0);
						level->setTileNoUpdate(x + 1, y - 1, z, 0);
					}
					else
					{
						level->setTileNoUpdate(x, y - 1, z - 1, 0);
						level->setTileNoUpdate(x, y - 1, z + 1, 0);
					}

					shared_ptr<VillagerGolem> villagerGolem = shared_ptr<VillagerGolem>(new VillagerGolem(level));
					villagerGolem->setPlayerCreated(true);
					villagerGolem->moveTo(x + 0.5, y - 1.95, z + 0.5, 0, 0);
					level->addEntity(villagerGolem);

					for (int i = 0; i < 120; i++)
					{
						level->addParticle(eParticleType_snowballpoof, x + level->random->nextDouble(), y - 2 + level->random->nextDouble() * 3.9, z + level->random->nextDouble(), 0, 0, 0);
					}

					level->tileUpdated(x, y, z, 0);
					level->tileUpdated(x, y - 1, z, 0);
					level->tileUpdated(x, y - 2, z, 0);
					if (xArms)
					{
						level->tileUpdated(x - 1, y - 1, z, 0);
						level->tileUpdated(x + 1, y - 1, z, 0);
					}
					else
					{
						level->tileUpdated(x, y - 1, z - 1, 0);
						level->tileUpdated(x, y - 1, z + 1, 0);
					}
				}
				else
				{
					// If we can't spawn it, at least give the resources back
					Tile::spawnResources(level, x, y, z, level->getData(x, y, z), 0);
					Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x, y - 1, z, level->getData(x, y - 1, z), 0);
					Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x, y - 2, z, level->getData(x, y - 2, z), 0);
					level->setTile(x, y, z, 0);
					level->setTile(x, y - 1, z, 0);
					level->setTile(x, y - 2, z, 0);

					if(xArms)
					{
						Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x - 1, y - 1, z, level->getData(x - 1, y - 1, z), 0);
						Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x + 1, y - 1, z, level->getData(x + 1, y - 1, z), 0);
						level->setTile(x - 1, y - 1, z, 0);
						level->setTile(x + 1, y - 1, z, 0);
					}
					else
					{
						Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x, y - 1, z - 1, level->getData(x, y - 1, z - 1), 0);
						Tile::tiles[Tile::ironBlock_Id]->spawnResources(level, x, y - 1, z + 1, level->getData(x, y - 1, z + 1), 0);
						level->setTile(x, y - 1, z - 1, 0);
						level->setTile(x, y - 1, z + 1, 0);
					}
				}
			}
		}
	}
}

bool PumpkinTile::mayPlace(Level *level, int x, int y, int z)
{
    int t = level->getTile(x, y, z);
    return (t == 0 || Tile::tiles[t]->material->isReplaceable()) && level->isTopSolidBlocking(x, y - 1, z);

}

void PumpkinTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
    int dir = Mth::floor(by->yRot * 4 / (360) + 2.5) & 3;
    level->setData(x, y, z, dir);
}

void PumpkinTile::registerIcons(IconRegister *iconRegister)
{
	iconFace = iconRegister->registerIcon(lit ? TEXTURE_LANTERN : TEXTURE_FACE);
	iconTop = iconRegister->registerIcon(L"pumpkin_top");
	icon = iconRegister->registerIcon(L"pumpkin_side");
}
