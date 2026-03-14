#include "stdafx.h"
#include "Tile_SPU.h"
#include "ChunkRebuildData.h"
#include "GrassTile_SPU.h"
#include "HalfSlabTile_SPU.h"
#include "WoodSlabTile_SPU.h"
#include "StoneSlabTile_SPU.h"
#include "ChestTile_SPU.h"
#include "ThinFenceTile_SPU.h"
#include "FenceTile_SPU.h"
#include "StairTile_SPU.h"
#include "DirtTile_SPU.h"
#include "DoorTile_SPU.h"
#include "PressurePlateTile_SPU.h"
#include "FarmTile_SPU.h"
#include "Bush_SPU.h"
#include "TallGrass_SPU.h"
#include "SandStoneTile_SPU.h"
#include "WoodTile_SPU.h"
#include "TreeTile_SPU.h"
#include "LeafTile_SPU.h"
#include "CropTile_SPU.h"
#include "ReedTile_SPU.h"
#include "TorchTile_SPU.h"
#include "Mushroom_SPU.h"
#include "FurnaceTile_SPU.h"
#include "WebTile_SPU.h"
#include "LiquidTile_SPU.h"
#include "FireTile_SPU.h"
#include "Sapling_SPU.h"
#include "GlassTile_SPU.h"
#include "IceTile_SPU.h"
#include "PortalTile_SPU.h"
#include "DispenserTile_SPU.h"
#include "RailTile_SPU.h"
#include "DetectorRailTile_SPU.h"
#include "TntTile_SPU.h"
#include "BookshelfTile_SPU.h"
#include "WorkbenchTile_SPU.h"
#include "SignTile_SPU.h"
#include "LadderTile_SPU.h"
#include "ButtonTile_SPU.h"
#include "TopSnowTile_SPU.h"
#include "CactusTile_SPU.h"
#include "RecordPlayerTile_SPU.h"
#include "PumpkinTile_SPU.h"
#include "CakeTile_SPU.h"
#include "TrapDoorTile_SPU.h"
#include "StoneMonsterTile_SPU.h"
#include "SmoothStoneBrickTile_SPU.h"
#include "HugeMushroomTile_SPU.h"
#include "MelonTile_SPU.h"
#include "StemTile_SPU.h"
#include "VineTile_SPU.h"
#include "MycelTile_SPU.h"
#include "WaterLilyTile_SPU.h"
#include "NetherStalkTile_SPU.h"
#include "EnchantmentTableTile_SPU.h"
#include "BrewingStandTile_SPU.h"
#include "DiodeTile_SPU.h"
#include "RedStoneDustTile_SPU.h"
#include "FenceGateTile_SPU.h"
#include "BedTile_SPU.h"
#include "PistonBaseTile_SPU.h"
#include "PistonExtensionTile_SPU.h"
#include "PistonMovingPiece_SPU.h"
#include "LeverTile_SPU.h"
#include "CauldronTile_SPU.h"
#include "TheEndPortal_SPU.h"
#include "TheEndPortalFrameTile_SPU.h"
#include "EggTile_SPU.h"
#include "CocoaTile_SPU.h"
#include "RedlightTile_SPU.h"
#include "ClothTile_SPU.h"
#include "SkullTile_SPU.h"
#include "MobSpawnerTile_SPU.h"
#include "EnderChestTile_SPU.h"
#include "TripWireSourceTile_SPU.h"
#include "TripWireTile_SPU.h"
#include "WallTile_SPU.h"
#include "FlowerPotTile_SPU.h"
#include "CarrotTile_SPU.h"
#include "PotatoTile_SPU.h"
#include "QuartzBlockTile_SPU.h"
#include "WoolCarpetTile_SPU.h"

#ifdef SN_TARGET_PS3_SPU
#include "../Common/spu_assert.h"
#endif

#include <assert.h>
#include <new>
#include "AnvilTile_SPU.h"

TileData_SPU* Tile_SPU::ms_pTileData = nullptr;

Tile_SPU Tile_SPU::m_tiles[256];



int Tile_SPU::getRenderShape()
{
	return SHAPE_BLOCK;
}


void Tile_SPU::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	ms_pTileData->xx0[id] = x0;
	ms_pTileData->yy0[id] = y0;
	ms_pTileData->zz0[id] = z0;
	ms_pTileData->xx1[id] = x1;
	ms_pTileData->yy1[id] = y1;
	ms_pTileData->zz1[id] = z1;
}

float Tile_SPU::getBrightness(ChunkRebuildData *level, int x, int y, int z)
{
	return level->getBrightness(x, y, z, ms_pTileData->lightEmission[id]);
}
// 
// // 4J - brought forward from 1.8.2
int Tile_SPU::getLightColor(ChunkRebuildData *level, int x, int y, int z)
{
	int tileID = level->getTile(x, y, z);
	return level->getLightColor(x, y, z, ms_pTileData->lightEmission[tileID]);
}
// 
// bool Tile_SPU::isFaceVisible(Level *level, int x, int y, int z, int f)
// {
// 	if (f == 0) y--;
// 	if (f == 1) y++;
// 	if (f == 2) z--;
// 	if (f == 3) z++;
// 	if (f == 4) x--;
// 	if (f == 5) x++;
// 	return !level->isSolidRenderTile(x, y, z);
// }
// 
bool Tile_SPU::shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
	if (face == 0 && getShapeY0() > 0) return true;
	if (face == 1 && getShapeY1() < 1) return true;
	if (face == 2 && getShapeZ0() > 0) return true;
	if (face == 3 && getShapeZ1() < 1) return true;
	if (face == 4 && getShapeX0() > 0) return true;
	if (face == 5 && getShapeX1() < 1) return true;
	return (!level->isSolidRenderTile(x, y, z));
}
// 
bool Tile_SPU::isSolidFace(ChunkRebuildData *level, int x, int y, int z, int face)
{
	return (level->getMaterial(x, y, z)->isSolid());
}

Icon_SPU *Tile_SPU::getTexture(ChunkRebuildData *level, int x, int y, int z, int face)
{
	// 4J - addition here to make rendering big blocks of leaves more efficient. Normally leaves never consider themselves as solid, so
	// blocks of leaves will have all sides of each block completely visible. Changing to consider as solid if this block is surrounded by
	// other leaves. This is paired with another change in  Level::isSolidRenderTile/Region::isSolidRenderTile which makes things solid
	// code-wise (ie for determining visible sides of neighbouring blocks). This change just makes the texture a solid one (tex + 1) which
	// we already have in the texture map for doing non-fancy graphics. Note: this tile-specific code is here rather than making some new virtual
	// method in the tiles, for the sake of efficiency - I don't imagine we'll be doing much more of this sort of thing

	int tileId = level->getTile(x, y, z);
	int tileData = level->getData(x, y, z);

	if( tileId == Tile_SPU::leaves_Id )
	{
		bool opaque = true;
		int axo[6] = { 1,-1, 0, 0, 0, 0};
		int ayo[6] = { 0, 0, 1,-1, 0, 0};
		int azo[6] = { 0, 0, 0, 0, 1,-1};
		for( int i = 0; (i < 6) && opaque; i++ )
		{
			int t = level->getTile(x + axo[i], y + ayo[i] , z + azo[i]);
			if( ( t != Tile_SPU::leaves_Id ) && ( ( Tile_SPU::m_tiles[t].id == -1) || !Tile_SPU::m_tiles[t].isSolidRender() ) )
			{
				opaque = false;
			}
		}


		Icon_SPU *icon = nullptr;
		if(opaque)
		{
			LeafTile_SPU::setFancy(false);
			icon = getTexture(face, tileData);
			LeafTile_SPU::setFancy(true);
		}
		else
		{
			icon = getTexture(face, tileData);
		}
		return icon;
	}
	return getTexture(face, tileData);
}
// 
Icon_SPU *Tile_SPU::getTexture(int face, int data)
{
	return &ms_pTileData->iconData[id];
}
// 
Icon_SPU *Tile_SPU::getTexture(int face)
{
	return getTexture(face, 0);
}
// 
// AABB *Tile_SPU::getTileAABB(Level *level, int x, int y, int z)
// {
// 	return AABB::newTemp(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
// }
// 
// void Tile_SPU::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, Entity *source) 
// {
// 	AABB *aabb = getAABB(level, x, y, z);
// 	if (aabb != nullptr && box->intersects(aabb)) boxes->push_back(aabb);
// }
// 
// void Tile_SPU::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes)
// {
// 	AABB *aabb = getAABB(level, x, y, z);
// 	if (aabb != nullptr && box->intersects(aabb)) boxes->push_back(aabb);
// }
// 
// AABB *Tile_SPU::getAABB(Level *level, int x, int y, int z)
// {
// 	return AABB::newTemp(x + xx0, y + yy0, z + zz0, x + xx1, y + yy1, z + zz1);
// }
// 
 bool Tile_SPU::isSolidRender(bool isServerLevel)
 {
 	return true;
 }


// bool Tile_SPU::mayPick(int data, bool liquid)
// {
// 	return mayPick();
// }
// 
// bool Tile_SPU::mayPick()
// {
// 	return true;
// }
// 
// void Tile_SPU::tick(Level *level, int x, int y, int z, Random *random)
// {
// }
// 
// void Tile_SPU::animateTick(Level *level, int x, int y, int z, Random *random)
// {
// }
// 
// void Tile_SPU::destroy(Level *level, int x, int y, int z, int data)
// {
// }
// 
// void Tile_SPU::neighborChanged(Level *level, int x, int y, int z, int type)
// {
// }
// 
// void Tile_SPU::addLights(Level *level, int x, int y, int z)
// {
// }
// 
// int Tile_SPU::getTickDelay()
// {
// 	return 10;
// }
// 
// void Tile_SPU::onPlace(Level *level, int x, int y, int z)
// {
// }
// 
// void Tile_SPU::onRemove(Level *level, int x, int y, int z)
// {
// }
// 
// int Tile_SPU::getResourceCount(Random *random)
// {
// 	return 1;
// }
// 
// int Tile_SPU::getResource(int data, Random *random, int playerBonusLevel)
// {
// 	return id;
// }
// 
// float Tile_SPU::getDestroyProgress(shared_ptr<Player> player)
// {
// 	if (destroySpeed < 0) return 0;
// 	if (!player->canDestroy(this)) return 1 / destroySpeed / 100.0f;
// 	return (player->getDestroySpeed(this) / destroySpeed) / 30;
// }
// 
// void Tile_SPU::spawnResources(Level *level, int x, int y, int z, int data, int playerBonusLevel)
// {
// 	spawnResources(level, x, y, z, data, 1, playerBonusLevel);
// }
// 
// void Tile_SPU::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
// {
// 	if (level->isClientSide) return;
// 	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
// 	for (int i = 0; i < count; i++)
// 	{
// 		if (level->random->nextFloat() > odds) continue;
// 		int type = getResource(data, level->random, playerBonusLevel);
// 		if (type <= 0) continue;
// 
// 		popResource(level, x, y, z, shared_ptr<ItemInstance>( new ItemInstance(type, 1, getSpawnResourcesAuxValue(data) ) ) );
// 	}
// }
// 
// void Tile_SPU::popResource(Level *level, int x, int y, int z, shared_ptr<ItemInstance> itemInstance)
// {
// 	if( level->isClientSide ) return;
// 
// 	float s = 0.7f;
// 	double xo = level->random->nextFloat() * s + (1 - s) * 0.5;
// 	double yo = level->random->nextFloat() * s + (1 - s) * 0.5;
// 	double zo = level->random->nextFloat() * s + (1 - s) * 0.5;
// 	shared_ptr<ItemEntity> item = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, itemInstance ) );
// 	item->throwTime = 10;
// 	level->addEntity(item);
// }
// 
// // Brought forward for TU7
// void Tile_SPU::popExperience(Level *level, int x, int y, int z, int amount)
// {
// 	if (!level->isClientSide)
// 	{
// 		while (amount > 0)
// 		{
// 			int newCount = ExperienceOrb::getExperienceValue(amount);
// 			amount -= newCount;
// 			level->addEntity(shared_ptr<ExperienceOrb>( new ExperienceOrb(level, x + .5, y + .5, z + .5, newCount)));
// 		}
// 	}
// }
// 
// int Tile_SPU::getSpawnResourcesAuxValue(int data)
// {
// 	return 0;
// }
// 
// float Tile_SPU::getExplosionResistance(shared_ptr<Entity> source)
// {
// 	return explosionResistance / 5.0f;
// }
// 
// HitResult *Tile_SPU::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
// {
// 	EnterCriticalSection(&m_csShape);
// 	updateShape(level, xt, yt, zt);
// 
// 	a = a->add(-xt, -yt, -zt);
// 	b = b->add(-xt, -yt, -zt);
// 
// 	Vec3 *xh0 = a->clipX(b, xx0);
// 	Vec3 *xh1 = a->clipX(b, xx1);
// 
// 	Vec3 *yh0 = a->clipY(b, yy0);
// 	Vec3 *yh1 = a->clipY(b, yy1);
// 
// 	Vec3 *zh0 = a->clipZ(b, zz0);
// 	Vec3 *zh1 = a->clipZ(b, zz1);
// 
// 	Vec3 *closest = nullptr;
// 
// 	if (containsX(xh0) && (closest == nullptr || a->distanceTo(xh0) < a->distanceTo(closest))) closest = xh0;
// 	if (containsX(xh1) && (closest == nullptr || a->distanceTo(xh1) < a->distanceTo(closest))) closest = xh1;
// 	if (containsY(yh0) && (closest == nullptr || a->distanceTo(yh0) < a->distanceTo(closest))) closest = yh0;
// 	if (containsY(yh1) && (closest == nullptr || a->distanceTo(yh1) < a->distanceTo(closest))) closest = yh1;
// 	if (containsZ(zh0) && (closest == nullptr || a->distanceTo(zh0) < a->distanceTo(closest))) closest = zh0;
// 	if (containsZ(zh1) && (closest == nullptr || a->distanceTo(zh1) < a->distanceTo(closest))) closest = zh1;
// 
// 	LeaveCriticalSection(&m_csShape);
// 
// 	if (closest == nullptr) return nullptr;
// 
// 	int face = -1;
// 
// 	if (closest == xh0) face = 4;
// 	if (closest == xh1) face = 5;
// 	if (closest == yh0) face = 0;
// 	if (closest == yh1) face = 1;
// 	if (closest == zh0) face = 2;
// 	if (closest == zh1) face = 3;
// 
// 	return new HitResult(xt, yt, zt, face, closest->add(xt, yt, zt));
// }
// 
// bool Tile_SPU::containsX(Vec3 *v)
// {
// 	if( v == nullptr) return false;
// 	return v->y >= yy0 && v->y <= yy1 && v->z >= zz0 && v->z <= zz1;
// }
// 
// bool Tile_SPU::containsY(Vec3 *v)
// {
// 	if( v == nullptr) return false;
// 	return v->x >= xx0 && v->x <= xx1 && v->z >= zz0 && v->z <= zz1;
// }
// 
// bool Tile_SPU::containsZ(Vec3 *v)
// {
// 	if( v == nullptr) return false;
// 	return v->x >= xx0 && v->x <= xx1 && v->y >= yy0 && v->y <= yy1;
// }
// 
// void Tile_SPU::wasExploded(Level *level, int x, int y, int z)
// {
// }
// 
int Tile_SPU::getRenderLayer()
{
	return 0;
}
// 
// bool Tile_SPU::mayPlace(Level *level, int x, int y, int z, int face)
// {
// 	return mayPlace(level, x, y, z);
// }
// 
// bool Tile_SPU::mayPlace(Level *level, int x, int y, int z)
// {
// 	int t = level->getTile(x, y, z);
// 	return t == 0 || Tile_SPU::tiles[t]->material->isReplaceable();
// }
// 
// // 4J-PB - Adding a TestUse for tooltip display
// bool Tile_SPU::TestUse()
// {
// 	return false;
// }
// 
// bool Tile_SPU::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
// {
// 	return false;
// }
// 
// bool Tile_SPU::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
// {
// 	return false;
// }
// 
// void Tile_SPU::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
// {
// }
// 
// void Tile_SPU::setPlacedOnFace(Level *level, int x, int y, int z, int face)
// {
// }
// 
// void Tile_SPU::prepareRender(Level *level, int x, int y, int z)
// {
// }
// 
// void Tile_SPU::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
// {
// }
// 
// void Tile_SPU::handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current)
// {
// }
// 
void Tile_SPU::updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData, TileEntity* forceEntity) // 4J added forceData, forceEntity param
{
}
// 

int Tile_SPU::getColor(ChunkRebuildData *level, int x, int y, int z)
{
	return 0xffffff;
}

// 
// int Tile_SPU::getColor(LevelSource *level, int x, int y, int z, int data)
// {
// 	return 0xffffff;
// }
// 
// bool Tile_SPU::getSignal(LevelSource *level, int x, int y, int z)
// {
// 	return false;
// }
// 
// bool Tile_SPU::getSignal(LevelSource *level, int x, int y, int z, int dir)
// {
// 	return false;
// }
// 
// bool Tile_SPU::isSignalSource()
// {
// 	return false;
// }
// 
// void Tile_SPU::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
// {
// }
// 
// bool Tile_SPU::getDirectSignal(Level *level, int x, int y, int z, int dir)
// {
// 	return false;
// }
// 
void Tile_SPU::updateDefaultShape()
{
}
// 
// void Tile_SPU::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
// {
// 	// 4J Stu - Special case - only record a crop destroy if is fully grown
// 	if(id==Tile_SPU::crops_Id)
// 	{
// 		if( Tile_SPU::crops->getResource(data, nullptr, 0) > 0 )
// 			player->awardStat(Stats::blocksMined[id], 1);
// 	}
// 	else
// 	{
// 		player->awardStat(Stats::blocksMined[id], 1);
// 	}
// 	player->awardStat(Stats::totalBlocksMined, 1);	// 4J : WESTY : Added for other award.
// 	player->causeFoodExhaustion(FoodConstants::EXHAUSTION_MINE);
// 
// 	if( id == Tile_SPU::treeTrunk_Id )
// 		player->awardStat(Achievements::mineWood);
// 
// 
//     if (isCubeShaped() && !isEntityTile[id] && EnchantmentHelper::hasSilkTouch(player->inventory))
// 	{
//         shared_ptr<ItemInstance> item = getSilkTouchItemInstance(data);
//         if (item != nullptr)
// 		{
//             popResource(level, x, y, z, item);
//         }
//     }
// 	else
// 	{
//         int playerBonusLevel = EnchantmentHelper::getDiggingLootBonus(player->inventory);
//         spawnResources(level, x, y, z, data, playerBonusLevel);
//     }
// }
// 
// shared_ptr<ItemInstance> Tile_SPU::getSilkTouchItemInstance(int data)
// {
//     int popData = 0;
//     if (id >= 0 && id < Item::items.length && Item::items[id]->isStackedByData())
// 	{
//         popData = data;
//     }
//     return shared_ptr<ItemInstance>(new ItemInstance(id, 1, popData));
// }
// 
// int Tile_SPU::getResourceCountForLootBonus(int bonusLevel, Random *random)
// {
// 	return getResourceCount(random);
// }
// 
// bool Tile_SPU::canSurvive(Level *level, int x, int y, int z)
// {
// 	return true;
// }
// 
// void Tile_SPU::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
// {
// }
// 
// Tile *Tile_SPU::setDescriptionId(unsigned int id)
// {
// 	this->descriptionId = id;
// 	return this;
// }
// 
// wstring Tile_SPU::getName()
// {
// 	return I18n::get(getDescriptionId() + L".name");
// }
// 
// unsigned int Tile_SPU::getDescriptionId(int iData /*= -1*/)
// {
// 	return descriptionId;
// }
// 
// Tile *Tile_SPU::setUseDescriptionId(unsigned int id)
// {
// 	this->useDescriptionId = id;
// 	return this;
// }
// 
// unsigned int Tile_SPU::getUseDescriptionId()
// {
// 	return useDescriptionId;
// }
// 
// void Tile_SPU::triggerEvent(Level *level, int x, int y, int z, int b0, int b1)
// {
// }
// 
// bool Tile_SPU::isCollectStatistics()
// {
// 	return collectStatistics;
// }
// 
// Tile *Tile_SPU::setNotCollectStatistics()
// {
// 	collectStatistics = false;
// 	return this;
// }
// 
// int Tile_SPU::getPistonPushReaction()
// {
// 	return material->getPushReaction();
// }
// 
// // 4J - brought forward from 1.8.2
float Tile_SPU::getShadeBrightness(ChunkRebuildData *level, int x, int y, int z)
{
	return level->isSolidBlockingTile(x, y, z) ? 0.2f : 1.0f;
}

Tile_SPU* Tile_SPU::createFromID( int tileID )
{
	if(tileID == 0)
		return nullptr;

	if(m_tiles[tileID].id != -1)
		return &m_tiles[tileID];
	
#ifndef SN_TARGET_PS3_SPU
	app.DebugPrintf("missing tile ID %d\n", tileID);
#else
	spu_print("missing tile ID %d\n", tileID);
#endif	
	return &m_tiles[1];

}

Material_SPU* Tile_SPU::getMaterial()
{
	int matID = ms_pTileData->materialIDs[id]; 
	return &ms_pTileData->materials[matID];
}

// 
// void Tile_SPU::fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance)
// {
// }
// 
// void Tile_SPU::registerIcons(IconRegister *iconRegister)
// {
// 	icon = iconRegister->registerIcon(m_textureName);
// }
// 
// wstring Tile_SPU::getTileItemIconName()
// {
// 	return L"";
// }
// 
// Tile *Tile_SPU::setTextureName(const wstring &name)
// {
// 	m_textureName = name;
// 	return this;
// }



void Tile_SPU::initTilePointers()
{

#define  CREATE_TILE_TYPE(index, className) new (&m_tiles[index]) className(index);


	CREATE_TILE_TYPE(grass_Id, GrassTile_SPU);
	CREATE_TILE_TYPE(stoneSlab_Id, StoneSlabTile_SPU);
	CREATE_TILE_TYPE(stoneSlabHalf_Id, StoneSlabTile_SPU);
	CREATE_TILE_TYPE(woodSlab_Id, WoodSlabTile_SPU);
	CREATE_TILE_TYPE(woodSlabHalf_Id, WoodSlabTile_SPU);

	CREATE_TILE_TYPE(chest_Id, ChestTile_SPU);

	CREATE_TILE_TYPE(ironFence_Id, ThinFenceTile_SPU);
	CREATE_TILE_TYPE(thinGlass_Id, ThinFenceTile_SPU);

	CREATE_TILE_TYPE(fence_Id, FenceTile_SPU);
	CREATE_TILE_TYPE(netherFence_Id, FenceTile_SPU);

	CREATE_TILE_TYPE(stairs_wood_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_stone_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_bricks_Id, StairTile_SPU);	
	CREATE_TILE_TYPE(stairs_stoneBrickSmooth_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_netherBricks_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_sandstone_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_sprucewood_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_birchwood_Id, StairTile_SPU);
	CREATE_TILE_TYPE(stairs_junglewood_Id, StairTile_SPU);

	CREATE_TILE_TYPE(dirt_Id, DirtTile_SPU);

	CREATE_TILE_TYPE(door_iron_Id, DoorTile_SPU);
	CREATE_TILE_TYPE(door_wood_Id, DoorTile_SPU);

	CREATE_TILE_TYPE(pressurePlate_stone_Id, PressurePlateTile_SPU);
	CREATE_TILE_TYPE(pressurePlate_wood_Id, PressurePlateTile_SPU);

	CREATE_TILE_TYPE(farmland_Id, FarmTile_SPU);

	CREATE_TILE_TYPE(flower_Id, Bush_SPU);
	CREATE_TILE_TYPE(rose_Id, Bush_SPU);
	CREATE_TILE_TYPE(deadBush_Id, Bush_SPU); // DeadBushTile

	CREATE_TILE_TYPE(tallgrass_Id, TallGrass_SPU);

	CREATE_TILE_TYPE(sandStone_Id, SandStoneTile_SPU);

	CREATE_TILE_TYPE(wood_Id, WoodTile_SPU);

	CREATE_TILE_TYPE(treeTrunk_Id, TreeTile_SPU);

	CREATE_TILE_TYPE(leaves_Id, LeafTile_SPU);

	CREATE_TILE_TYPE(crops_Id, CropTile_SPU);

	CREATE_TILE_TYPE(reeds_Id, ReedTile_SPU);

	CREATE_TILE_TYPE(torch_Id, TorchTile_SPU);
	CREATE_TILE_TYPE(notGate_off_Id, TorchTile_SPU); // TorchTile->NotGateTile
	CREATE_TILE_TYPE(notGate_on_Id, TorchTile_SPU);	// TorchTile->NotGateTile

	CREATE_TILE_TYPE(mushroom1_Id, Mushroom_SPU);
	CREATE_TILE_TYPE(mushroom2_Id, Mushroom_SPU);

	CREATE_TILE_TYPE(mobSpawner_Id, MobSpawnerTile_SPU);
	CREATE_TILE_TYPE(musicBlock_Id, EntityTile_SPU);	// MusicTile->EntityTile

	CREATE_TILE_TYPE(furnace_Id, FurnaceTile_SPU);
	CREATE_TILE_TYPE(furnace_lit_Id, FurnaceTile_SPU);

	CREATE_TILE_TYPE(web_Id, WebTile_SPU);

	CREATE_TILE_TYPE(water_Id, LiquidTile_SPU);
	CREATE_TILE_TYPE(lava_Id, LiquidTile_SPU);
	CREATE_TILE_TYPE(calmLava_Id, LiquidTile_SPU);		// LiquidTileStatic
	CREATE_TILE_TYPE(calmWater_Id, LiquidTile_SPU);		// LiquidTileStatic

	CREATE_TILE_TYPE(fire_Id, FireTile_SPU);

	CREATE_TILE_TYPE(sapling_Id, Sapling_SPU);

	CREATE_TILE_TYPE(glass_Id, GlassTile_SPU);

	CREATE_TILE_TYPE(ice_Id, IceTile_SPU);

	CREATE_TILE_TYPE(portalTile_Id, PortalTile_SPU);

	CREATE_TILE_TYPE(dispenser_Id, DispenserTile_SPU);

	CREATE_TILE_TYPE(rail_Id, RailTile_SPU);
	CREATE_TILE_TYPE(goldenRail_Id, RailTile_SPU);

	CREATE_TILE_TYPE(detectorRail_Id, DetectorRailTile_SPU);

	CREATE_TILE_TYPE(tnt_Id, TntTile_SPU);

	CREATE_TILE_TYPE(bookshelf_Id, BookshelfTile_SPU);

	CREATE_TILE_TYPE(workBench_Id, WorkbenchTile_SPU);

	CREATE_TILE_TYPE(sign_Id, SignTile_SPU);
	CREATE_TILE_TYPE(wallSign_Id, SignTile_SPU);

	CREATE_TILE_TYPE(ladder_Id, LadderTile_SPU);

	CREATE_TILE_TYPE(button_stone_Id, ButtonTile_SPU);
	CREATE_TILE_TYPE(button_wood_Id, ButtonTile_SPU);

	CREATE_TILE_TYPE(topSnow_Id, TopSnowTile_SPU);

	CREATE_TILE_TYPE(cactus_Id, CactusTile_SPU);

	CREATE_TILE_TYPE(recordPlayer_Id, RecordPlayerTile_SPU);

	CREATE_TILE_TYPE(pumpkin_Id, PumpkinTile_SPU);
	CREATE_TILE_TYPE(litPumpkin_Id, PumpkinTile_SPU);

	CREATE_TILE_TYPE(cake_Id, CakeTile_SPU);

	CREATE_TILE_TYPE(trapdoor_Id, TrapDoorTile_SPU);

	CREATE_TILE_TYPE(monsterStoneEgg_Id, StoneMonsterTile_SPU);

	CREATE_TILE_TYPE(stoneBrickSmooth_Id, SmoothStoneBrickTile_SPU);

	CREATE_TILE_TYPE(hugeMushroom1_Id, HugeMushroomTile_SPU);
	CREATE_TILE_TYPE(hugeMushroom2_Id, HugeMushroomTile_SPU);

	CREATE_TILE_TYPE(melon_Id, MelonTile_SPU);

	CREATE_TILE_TYPE(melonStem_Id, StemTile_SPU);
	CREATE_TILE_TYPE(pumpkinStem_Id, StemTile_SPU);

	CREATE_TILE_TYPE(vine_Id, VineTile_SPU);

	CREATE_TILE_TYPE(mycel_Id, MycelTile_SPU);

	CREATE_TILE_TYPE(waterLily_Id, WaterlilyTile_SPU);

	CREATE_TILE_TYPE(netherStalk_Id, NetherStalkTile_SPU);

	CREATE_TILE_TYPE(enchantTable_Id, EnchantmentTableTile_SPU);

	CREATE_TILE_TYPE(brewingStand_Id, BrewingStandTile_SPU);

	CREATE_TILE_TYPE(diode_on_Id, DiodeTile_SPU);
	CREATE_TILE_TYPE(diode_off_Id, DiodeTile_SPU);

	CREATE_TILE_TYPE(redStoneDust_Id, RedStoneDustTile_SPU);

	CREATE_TILE_TYPE(fenceGate_Id, FenceGateTile_SPU);

	CREATE_TILE_TYPE(bed_Id, BedTile_SPU);

	CREATE_TILE_TYPE(pistonBase_Id, PistonBaseTile_SPU);
	CREATE_TILE_TYPE(pistonStickyBase_Id, PistonBaseTile_SPU);

	CREATE_TILE_TYPE(pistonExtensionPiece_Id, PistonExtensionTile_SPU);

	CREATE_TILE_TYPE(pistonMovingPiece_Id, PistonMovingPiece_SPU);

	CREATE_TILE_TYPE(lever_Id, LeverTile_SPU);

	CREATE_TILE_TYPE(cauldron_Id, CauldronTile_SPU);

	CREATE_TILE_TYPE(endPortalTile_Id, TheEndPortal_SPU);

	CREATE_TILE_TYPE(endPortalFrameTile_Id, TheEndPortalFrameTile_SPU);

	CREATE_TILE_TYPE(dragonEgg_Id, EggTile_SPU);

	CREATE_TILE_TYPE(cocoa_Id, CocoaTile_SPU);

	CREATE_TILE_TYPE(redstoneLight_Id, RedlightTile_SPU);
	CREATE_TILE_TYPE(redstoneLight_lit_Id, RedlightTile_SPU);

	CREATE_TILE_TYPE(skull_Id, SkullTile_SPU);

		// these tile types don't have any additional code that we need.
	CREATE_TILE_TYPE(stoneBrick_Id, Tile_SPU);		// Tile
	CREATE_TILE_TYPE(lapisBlock_Id, Tile_SPU);
	CREATE_TILE_TYPE(redBrick_Id, Tile_SPU);
	CREATE_TILE_TYPE(mossStone_Id, Tile_SPU);
	CREATE_TILE_TYPE(netherBrick_Id, Tile_SPU);
	CREATE_TILE_TYPE(whiteStone_Id, Tile_SPU);
	CREATE_TILE_TYPE(unbreakable_Id, Tile_SPU);
	CREATE_TILE_TYPE(sponge_Id, Tile_SPU);
	CREATE_TILE_TYPE(rock_Id, Tile_SPU);			// StoneTile
	CREATE_TILE_TYPE(obsidian_Id, Tile_SPU);		// StoneTile->ObsidianTile
	CREATE_TILE_TYPE(sand_Id, Tile_SPU);			// HeavyTile
	CREATE_TILE_TYPE(gravel_Id, Tile_SPU);			// GravelTile
	CREATE_TILE_TYPE(goldOre_Id, Tile_SPU);		// OreTile
	CREATE_TILE_TYPE(ironOre_Id, Tile_SPU); 		// OreTile
	CREATE_TILE_TYPE(coalOre_Id, Tile_SPU); 		// OreTile
	CREATE_TILE_TYPE(lapisOre_Id, Tile_SPU); 		// OreTile	
	CREATE_TILE_TYPE(diamondOre_Id, Tile_SPU); 	// OreTile
	CREATE_TILE_TYPE(clay_Id, Tile_SPU);			// ClayTile
	CREATE_TILE_TYPE(redStoneOre_Id, Tile_SPU);	// RedStoneOreTile
	CREATE_TILE_TYPE(redStoneOre_lit_Id, Tile_SPU);	// RedStoneOreTile
	CREATE_TILE_TYPE(goldBlock_Id, Tile_SPU);		// MetalTile
	CREATE_TILE_TYPE(ironBlock_Id, Tile_SPU);		// MetalTile
	CREATE_TILE_TYPE(diamondBlock_Id, Tile_SPU);	// MetalTile
	CREATE_TILE_TYPE(snow_Id, Tile_SPU);			// SnowTile
	CREATE_TILE_TYPE(hellRock_Id, Tile_SPU);		// HellStoneTile
	CREATE_TILE_TYPE(hellSand_Id, Tile_SPU);		// HellSandTile
	CREATE_TILE_TYPE(lightGem_Id, Tile_SPU);		// LightGemTile
	CREATE_TILE_TYPE(aprilFoolsJoke_Id, Tile_SPU); // LockedChestTile

	CREATE_TILE_TYPE(cloth_Id, ClothTile_SPU); // wool


	CREATE_TILE_TYPE(emeraldOre_Id, Tile_SPU);		// OreTile
	CREATE_TILE_TYPE(enderChest_Id, EnderChestTile_SPU);
	CREATE_TILE_TYPE(tripWireSource_Id, TripWireSourceTile_SPU);
	CREATE_TILE_TYPE(tripWire_Id, TripWireTile_SPU);
// 
	CREATE_TILE_TYPE(emeraldBlock_Id, Tile_SPU);	// MetalTile
	CREATE_TILE_TYPE(cobbleWall_Id, WallTile_SPU);
	CREATE_TILE_TYPE(flowerPot_Id, FlowerPotTile_SPU);
	CREATE_TILE_TYPE(carrots_Id, CarrotTile_SPU);
	CREATE_TILE_TYPE(potatoes_Id, PotatoTile_SPU);
 	CREATE_TILE_TYPE(anvil_Id, AnvilTile_SPU);
 	CREATE_TILE_TYPE(netherQuartz_Id, Tile_SPU);		// OreTile
 	CREATE_TILE_TYPE(quartzBlock_Id, QuartzBlockTile_SPU);
	CREATE_TILE_TYPE(stairs_quartz_Id, StairTile_SPU);
 	CREATE_TILE_TYPE(woolCarpet_Id, WoolCarpetTile_SPU);

};