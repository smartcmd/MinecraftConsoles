#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.phys.h"
#include "EnderEyeItem.h"
#include "SoundTypes.h"
#include "LevelData.h"

EnderEyeItem::EnderEyeItem(int id) : Item(id)
{
}

bool EnderEyeItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	int targetType = level->getTile(x, y, z);
	int targetData = level->getData(x, y, z);

	if (player->mayBuild(x, y, z) && targetType == Tile::endPortalFrameTile_Id && !TheEndPortalFrameTile::hasEye(targetData))
	{
		if(bTestUseOnOnly) return true;
		if (level->isClientSide) return true;
		level->setData(x, y, z, targetData + TheEndPortalFrameTile::EYE_BIT);
		instance->count--;

		for (int i = 0; i < 16; i++)
		{
			double xp = x + (5.0f + random->nextFloat() * 6.0f) / 16.0f;
			double yp = y + 13.0f / 16.0f;
			double zp = z + (5.0f + random->nextFloat() * 6.0f) / 16.0f;
			double xa = 0;
			double ya = 0;
			double za = 0;

			level->addParticle(eParticleType_smoke, xp, yp, zp, xa, ya, za);
		}

		// scan if the circle is complete
		int direction = targetData & 3;

		// find borders
		int min = 0;
		int max = 0;
		bool firstFound = false;
		bool valid = true;
		int rightHandDirection = Direction::DIRECTION_CLOCKWISE[direction];
		for (int offset = -2; offset <= 2; offset++)
		{
			int testX = x + Direction::STEP_X[rightHandDirection] * offset;
			int testZ = z + Direction::STEP_Z[rightHandDirection] * offset;

			int tile = level->getTile(testX, y, testZ);
			if (tile == Tile::endPortalFrameTile->id)
			{
				int data = level->getData(testX, y, testZ);
				if (!TheEndPortalFrameTile::hasEye(data))
				{
					valid = false;
					break;
				}
				max = offset;
				if (!firstFound)
				{
					min = offset;
					firstFound = true;
				}
			}
		}

		// got a full frame?
		if (valid && max == min + 2)
		{

			// check if other edge is valid
			for (int offset = min; offset <= max; offset++)
			{
				int testX = x + Direction::STEP_X[rightHandDirection] * offset;
				int testZ = z + Direction::STEP_Z[rightHandDirection] * offset;
				testX += Direction::STEP_X[direction] * 4;
				testZ += Direction::STEP_Z[direction] * 4;

				int tile = level->getTile(testX, y, testZ);
				int data = level->getData(testX, y, testZ);
				if (tile != Tile::endPortalFrameTile_Id || !TheEndPortalFrameTile::hasEye(data))
				{
					valid = false;
					break;
				}
			}
			// check if edges on the sides are valid
			for (int side = (min - 1); side <= (max + 1); side += 4)
			{
				for (int offset = 1; offset <= 3; offset++)
				{
					int testX = x + Direction::STEP_X[rightHandDirection] * side;
					int testZ = z + Direction::STEP_Z[rightHandDirection] * side;
					testX += Direction::STEP_X[direction] * offset;
					testZ += Direction::STEP_Z[direction] * offset;

					int tile = level->getTile(testX, y, testZ);
					int data = level->getData(testX, y, testZ);
					if (tile != Tile::endPortalFrameTile_Id || !TheEndPortalFrameTile::hasEye(data))
					{
						valid = false;
						break;
					}
				}
			}
			if (valid)
			{

				// fill portal
				for (int px = min; px <= max; px++)
				{
					for (int pz = 1; pz <= 3; pz++)
					{
						int targetX = x + Direction::STEP_X[rightHandDirection] * px;
						int targetZ = z + Direction::STEP_Z[rightHandDirection] * px;
						targetX += Direction::STEP_X[direction] * pz;
						targetZ += Direction::STEP_Z[direction] * pz;

						level->setTile(targetX, y, targetZ, Tile::endPortalTile_Id);
					}
				}
			}
		}

		return true;
	}
	return false;
}

bool EnderEyeItem::TestUse(Level *level, shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, false);
	if (hr != NULL && hr->type == HitResult::TILE)
	{
		int tile = level->getTile(hr->x, hr->y, hr->z);
		delete hr;
		if (tile == Tile::endPortalFrameTile_Id)
		{
			return false;
		}
	}
	else if( hr != NULL )
	{
		delete hr;
	}

	//if (!level->isClientSide)
	{
		if((level->dimension->id==LevelData::DIMENSION_OVERWORLD) && level->getLevelData()->getHasStronghold())
		{
			return true;
		}
		else
		{
// 			int x,z;			
// 			if(app.GetTerrainFeaturePosition(eTerrainFeature_Stronghold,&x,&z))
// 			{
// 				level->getLevelData()->setXStronghold(x);
// 				level->getLevelData()->setZStronghold(z);
// 				level->getLevelData()->setHasStronghold();
// 
// 				app.DebugPrintf("=== FOUND stronghold in terrain features list\n");
// 				
// 				app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_StrongholdPosition);
// 			}
// 			else
			{
				// can't find the stronghold position in the terrain feature list. Do we have to run a post-process?
				app.DebugPrintf("=== Can't find stronghold in terrain features list\n");
			}

		}
// 		TilePos *nearestMapFeature = level->findNearestMapFeature(LargeFeature::STRONGHOLD, (int) player->x, (int) player->y, (int) player->z);
// 		if (nearestMapFeature != NULL)
// 		{
// 			delete nearestMapFeature;
// 			return true;
// 		}
	}
	return false;
}

shared_ptr<ItemInstance> EnderEyeItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	HitResult *hr = getPlayerPOVHitResult(level, player, false);
	if (hr != NULL && hr->type == HitResult::TILE)
	{
		int tile = level->getTile(hr->x, hr->y, hr->z);
		delete hr;
		if (tile == Tile::endPortalFrameTile_Id)
		{
			return instance;
		}
	}
	else if( hr != NULL )
	{
		delete hr;
	}

	if (!level->isClientSide)
	{
		if((level->dimension->id==LevelData::DIMENSION_OVERWORLD) && level->getLevelData()->getHasStronghold())
		{
			shared_ptr<EyeOfEnderSignal> eyeOfEnderSignal = shared_ptr<EyeOfEnderSignal>( new EyeOfEnderSignal(level, player->x, player->y + 1.62 - player->heightOffset, player->z) );
			eyeOfEnderSignal->signalTo(level->getLevelData()->getXStronghold()<<4, player->y + 1.62 - player->heightOffset, level->getLevelData()->getZStronghold()<<4);
			level->addEntity(eyeOfEnderSignal);

			level->playSound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
			level->levelEvent(nullptr, LevelEvent::SOUND_LAUNCH, (int) player->x, (int) player->y, (int) player->z, 0);
			if (!player->abilities.instabuild)
			{
				instance->count--;
			}
		}

		/*TilePos *nearestMapFeature = level->findNearestMapFeature(LargeFeature::STRONGHOLD, (int) player->x, (int) player->y, (int) player->z);
		if (nearestMapFeature != NULL)
		{
			shared_ptr<EyeOfEnderSignal> eyeOfEnderSignal = shared_ptr<EyeOfEnderSignal>( new EyeOfEnderSignal(level, player->x, player->y + 1.62 - player->heightOffset, player->z) );
			eyeOfEnderSignal->signalTo(nearestMapFeature->x, nearestMapFeature->y, nearestMapFeature->z);
			delete nearestMapFeature;
			level->addEntity(eyeOfEnderSignal);

			level->playSound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
			level->levelEvent(NULL, LevelEvent::SOUND_LAUNCH, (int) player->x, (int) player->y, (int) player->z, 0);
			if (!player->abilities.instabuild)
			{
				instance->count--;
			}
		}*/
	}
	return instance;
}