#include "stdafx.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.h"
#include "HitResult.h"
#include "SpawnEggItem.h"
#include "Difficulty.h"


SpawnEggItem::SpawnEggItem(int id) : Item(id)
{
	setMaxStackSize(16); // 4J-PB brought forward. It is 64 on PC, but we'll never be able to place that many
	setStackedByData(true);
	overlay = nullptr;
}

wstring SpawnEggItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	wstring elementName = getDescription();

	int nameId = EntityIO::getNameId(itemInstance->getAuxValue());
	if (nameId >= 0)
	{
		elementName = replaceAll(elementName,L"{*CREATURE*}",app.GetString(nameId));
		//elementName += " " + I18n.get("entity." + encodeId + ".name");
	}
	else
	{
		elementName = replaceAll(elementName,L"{*CREATURE*}",L"");
	}

	return elementName;
}

int SpawnEggItem::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
    auto it = EntityIO ::idsSpawnableInCreative.find(item->getAuxValue());
    if (it != EntityIO::idsSpawnableInCreative.end())
	{
		EntityIO::SpawnableMobInfo *spawnableMobInfo = it->second;
		if (spriteLayer == 0) {
			return Minecraft::GetInstance()->getColourTable()->getColor( spawnableMobInfo->eggColor1 );
		}
		return Minecraft::GetInstance()->getColourTable()->getColor( spawnableMobInfo->eggColor2 );
	}
	return 0xffffff;
}

bool SpawnEggItem::hasMultipleSpriteLayers()
{
	return true;
}

Icon *SpawnEggItem::getLayerIcon(int auxValue, int spriteLayer)
{
	if (spriteLayer > 0)
	{
		return overlay;
	}
	return Item::getLayerIcon(auxValue, spriteLayer);
}

// 4J-PB - added for dispenser
shared_ptr<Entity> SpawnEggItem::canSpawn(int iAuxVal, Level *level, int *piResult)
{
	shared_ptr<Entity> newEntity = EntityIO::newById(iAuxVal, level);
	if (newEntity != nullptr)
	{
		bool canSpawn = false;

		switch(newEntity->GetType())
		{
		case eTYPE_CHICKEN:
			if(level->canCreateMore( eTYPE_CHICKEN, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManyChickens;
			}
			break;
		case eTYPE_WOLF:
			if(level->canCreateMore( eTYPE_WOLF, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManyWolves;
			}
			break;
		case eTYPE_VILLAGER:
			if(level->canCreateMore( eTYPE_VILLAGER, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManyVillagers;
			}
			break;
		case eTYPE_MUSHROOMCOW:
			if(level->canCreateMore(eTYPE_MUSHROOMCOW, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManyMooshrooms;
			}
			break;
		case eTYPE_SQUID:
			if(level->canCreateMore( eTYPE_SQUID, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManySquid;
			}
			break;
		case eTYPE_BAT:
			if(level->canCreateMore( eTYPE_BAT, Level::eSpawnType_Egg) )
			{
				canSpawn = true;
			}
			else
			{
				*piResult=eSpawnResult_FailTooManyBats;
			}
			break;
		default:
			if ( eTYPE_FLAGSET(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK, newEntity->GetType()) )
			{
				if( level->canCreateMore( newEntity->GetType(), Level::eSpawnType_Egg ) )
				{
					canSpawn = true;
				}
				else
				{
					// different message for each animal

					*piResult=eSpawnResult_FailTooManyPigsCowsSheepCats;
				}
			}
			// 4J: Use eTYPE_ENEMY instead of monster (slimes and ghasts aren't monsters)
			else if(newEntity->instanceof(eTYPE_ENEMY))
			{
				// 4J-PB - check if the player is trying to spawn an enemy in peaceful mode
				if(level->difficulty==Difficulty::PEACEFUL)
				{
					*piResult=eSpawnResult_FailCantSpawnInPeaceful;
				}
				else if(level->canCreateMore( newEntity->GetType(), Level::eSpawnType_Egg) )
				{
					canSpawn = true;
				}
				else
				{
					*piResult=eSpawnResult_FailTooManyMonsters;
				}
			}
#ifndef _CONTENT_PACKAGE
			else if(app.DebugArtToolsOn())
			{
				canSpawn = true;
			}
#endif
			break;
		}

		if(canSpawn)
		{
			return newEntity;
		}
	}

	return nullptr;
}

bool SpawnEggItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (level->isClientSide)
	{
		return true;
	}

	int tile = level->getTile(x, y, z);

#ifndef _CONTENT_PACKAGE
	if(app.DebugArtToolsOn() && tile == Tile::mobSpawner_Id)
	{
		// 4J Stu - Force adding this as a tile update
		level->removeTile(x,y,z);
		level->setTileAndData(x,y,z,Tile::mobSpawner_Id, 0, Tile::UPDATE_ALL);
		shared_ptr<MobSpawnerTileEntity> mste = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x,y,z) );
		if(mste != nullptr)
		{
			mste->setEntityId( EntityIO::getEncodeId(itemInstance->getAuxValue()) );
			return true;
		}
	}
#endif

	x += Facing::STEP_X[face];
	y += Facing::STEP_Y[face];
	z += Facing::STEP_Z[face];

	double yOff = 0;
	if (face == Facing::UP && (Tile::tiles[tile] != nullptr && Tile::tiles[tile]->getRenderShape() == Tile::SHAPE_FENCE))
	{
		// special case
		yOff = .5;
	}

	int iResult=0;
	shared_ptr<Entity> result = spawnMobAt(level, itemInstance->getAuxValue(), x + .5, y + yOff, z + .5, &iResult);

	if(bTestUseOnOnly)
	{
		return result != nullptr;
	}

	if (result != nullptr)
	{
		// 4J-JEV: SetCustomName is a method for Mob not LivingEntity; so change instanceof to check for Mobs.
		if ( result->instanceof(eTYPE_MOB) && itemInstance->hasCustomHoverName() )
		{
			dynamic_pointer_cast<Mob>(result)->setCustomName(itemInstance->getHoverName());
		}
		if ( !player->abilities.instabuild )
		{
			itemInstance->count--;
		}
	}
	else
	{
		DisplaySpawnError(player, iResult);
	}

	return true;
}

shared_ptr<ItemInstance> SpawnEggItem::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	if (level->isClientSide) return itemInstance;

	HitResult *hr = getPlayerPOVHitResult(level, player, true);
	if (hr == nullptr)
	{
		delete hr;
		return itemInstance;
	}

	if (hr->type == HitResult::TILE)
	{
		int xt = hr->x;
		int yt = hr->y;
		int zt = hr->z;

		if (!level->mayInteract(player, xt, yt, zt,0))
		{
			delete hr;
			return itemInstance;
		}
		if (!player->mayUseItemAt(xt, yt, zt, hr->f, itemInstance)) return itemInstance;

		if (level->getMaterial(xt, yt, zt) == Material::water)
		{
			int iResult=0;
			shared_ptr<Entity> result = spawnMobAt(level, itemInstance->getAuxValue(), xt, yt, zt, &iResult);
			if (result != nullptr)
			{
				// 4J-JEV: SetCustomName is a method for Mob not LivingEntity; so change instanceof to check for Mobs.
				if ( result->instanceof(eTYPE_MOB) && itemInstance->hasCustomHoverName() )
				{
					dynamic_pointer_cast<Mob>(result)->setCustomName(itemInstance->getHoverName());
				}
				if (!player->abilities.instabuild)
				{
					itemInstance->count--;
				}
			}
			else
			{
				SpawnEggItem::DisplaySpawnError(player, iResult);
			}
		}
	}
	return itemInstance;
}

shared_ptr<Entity> SpawnEggItem::spawnMobAt(Level *level, int auxVal, double x, double y, double z, int *piResult)
{
	int mobId = auxVal;
	int extraData = 0;

	//4J Stu - Enable spawning specific entity sub-types
	mobId = auxVal & 0xFFF;
	extraData = auxVal >> 12;

	if (EntityIO::idsSpawnableInCreative.find(mobId) == EntityIO::idsSpawnableInCreative.end())
	{
		return nullptr;
	}

	shared_ptr<Entity> newEntity = nullptr;

	for (int i = 0; i < SPAWN_COUNT; i++)
	{
		newEntity = canSpawn(mobId, level, piResult);

		// 4J-JEV: DynCasting to Mob not LivingEntity; so change instanceof to check for Mobs.
		if ( newEntity != nullptr && newEntity->instanceof(eTYPE_MOB) )
		{
			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(newEntity);
			newEntity->moveTo(x, y, z, Mth::wrapDegrees(level->random->nextFloat() * 360), 0);
			newEntity->setDespawnProtected();		// 4J added, default to being protected against despawning (has to be done after initial position is set)
			mob->yHeadRot = mob->yRot;
			mob->yBodyRot = mob->yRot;

			mob->finalizeMobSpawn(nullptr, extraData);
			level->addEntity(newEntity);
			mob->playAmbientSound();
		}
	}

	return newEntity;
}

void SpawnEggItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);
	overlay = iconRegister->registerIcon(getIconName() + L"_overlay");
}

void SpawnEggItem::DisplaySpawnError(shared_ptr<Player> player, int result)
{
	// some negative sound effect?
	//level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);
	switch(result)
	{
	case eSpawnResult_FailTooManyPigsCowsSheepCats:
		player->displayClientMessage(IDS_MAX_PIGS_SHEEP_COWS_CATS_SPAWNED );
		break;
	case eSpawnResult_FailTooManyChickens:
		player->displayClientMessage(IDS_MAX_CHICKENS_SPAWNED );
		break;
	case eSpawnResult_FailTooManySquid:
		player->displayClientMessage(IDS_MAX_SQUID_SPAWNED );
		break;
	case eSpawnResult_FailTooManyBats:
		player->displayClientMessage(IDS_MAX_BATS_SPAWNED);
		break;
	case eSpawnResult_FailTooManyWolves:
		player->displayClientMessage(IDS_MAX_WOLVES_SPAWNED );
		break;
	case eSpawnResult_FailTooManyMooshrooms:
		player->displayClientMessage(IDS_MAX_MOOSHROOMS_SPAWNED );
		break;
	case eSpawnResult_FailTooManyMonsters:
		player->displayClientMessage(IDS_MAX_ENEMIES_SPAWNED );
		break;
	case eSpawnResult_FailTooManyVillagers:
		player->displayClientMessage(IDS_MAX_VILLAGERS_SPAWNED );
		break;
	case eSpawnResult_FailCantSpawnInPeaceful:
		player->displayClientMessage(IDS_CANT_SPAWN_IN_PEACEFUL );
		break;

	}
}