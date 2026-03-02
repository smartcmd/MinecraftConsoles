#include "stdafx.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "net.minecraft.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.h"
#include "MonsterPlacerItem.h"
#include "Difficulty.h"


MonsterPlacerItem::MonsterPlacerItem(int id) : Item(id)
{
	setMaxStackSize(16); // 4J-PB brought forward. It is 64 on PC, but we'll never be able to place that many
	setStackedByData(true);
	overlay = NULL;
}

wstring MonsterPlacerItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
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

int MonsterPlacerItem::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	AUTO_VAR(it, EntityIO::idsSpawnableInCreative.find(item->getAuxValue()));
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

bool MonsterPlacerItem::hasMultipleSpriteLayers()
{
	return true;
}

Icon *MonsterPlacerItem::getLayerIcon(int auxValue, int spriteLayer)
{
	if (spriteLayer > 0)
	{
		return overlay;
	}
	return Item::getLayerIcon(auxValue, spriteLayer);
}

// 4J-PB - added for dispenser
shared_ptr<Entity> MonsterPlacerItem::canSpawn(int iAuxVal, Level *level, int *piResult)
{
	shared_ptr<Entity> newEntity = EntityIO::newById(iAuxVal, level);
	if (newEntity != NULL)
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
		default:
			if( (newEntity->GetType() & eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) == eTYPE_ANIMALS_SPAWN_LIMIT_CHECK)
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
			else if( (newEntity->GetType() & eTYPE_MONSTER) == eTYPE_MONSTER)
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
			break;
		}

		if(canSpawn)
		{
			return newEntity;
		}
	}

	return nullptr;
}

bool MonsterPlacerItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	if (level->isClientSide)
	{
		return true;
	}

	int tile = level->getTile(x, y, z);

#ifndef _CONTENT_PACKAGE
	if(app.DebugSettingsOn() && tile == Tile::mobSpawner_Id)
	{
		// 4J Stu - Force adding this as a tile update
		level->setTile(x,y,z,0);
		level->setTile(x,y,z,Tile::mobSpawner_Id);
		shared_ptr<MobSpawnerTileEntity> mste = dynamic_pointer_cast<MobSpawnerTileEntity>( level->getTileEntity(x,y,z) );
		if(mste != NULL)
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
	// 4J-PB - missing parentheses added
	if (face == Facing::UP && (tile == Tile::fence_Id || tile == Tile::netherFence_Id))
	{
		// special case
		yOff = .5;
	}

	int iResult=0;
	bool spawned = spawnMobAt(level, itemInstance->getAuxValue(), x + .5, y + yOff, z + .5, &iResult) != NULL;

	if(bTestUseOnOnly)
	{
		return spawned;
	}

	if (spawned)
	{		
		if (!player->abilities.instabuild)
		{
			itemInstance->count--;
		}		
	}
	else
	{
		// some negative sound effect?
		//level->levelEvent(LevelEvent::SOUND_CLICK_FAIL, x, y, z, 0);
		switch(iResult)
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

	return true;
}

shared_ptr<Entity> MonsterPlacerItem::spawnMobAt(Level *level, int mobId, double x, double y, double z, int *piResult)
{
	if (EntityIO::idsSpawnableInCreative.find(mobId) == EntityIO::idsSpawnableInCreative.end())
	{
		return nullptr;
	}

	shared_ptr<Entity> newEntity = nullptr;

	for (int i = 0; i < SPAWN_COUNT; i++)
	{
		newEntity = canSpawn(mobId, level, piResult);

		shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(newEntity);
		if (mob)
		{
			newEntity->moveTo(x, y, z, Mth::wrapDegrees(level->random->nextFloat() * 360), 0);
			newEntity->setDespawnProtected();		// 4J added, default to being protected against despawning (has to be done after initial position is set)
			mob->yHeadRot = mob->yRot;
			mob->yBodyRot = mob->yRot;

			mob->finalizeMobSpawn();
			level->addEntity(newEntity);
			mob->playAmbientSound();
		}
	}

	return newEntity;
}

void MonsterPlacerItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);
	overlay = iconRegister->registerIcon(L"monsterPlacer_overlay");
}
