#include "stdafx.h"

#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.item.crafting.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.chunk.storage.h"
#include "net.minecraft.world.level.levelgen.structure.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.effect.h"

#include "Minecraft.World.h"
#include "../Minecraft.Client/ServerLevel.h"

#ifdef _DURANGO
#include "DurangoStats.h"
#else
#include "CommonStats.h"
#endif

void MinecraftWorld_RunStaticCtors()
{
	// The ordering of these static ctors can be important. If they are within statement blocks then
	// DO NOT CHANGE the ordering - 4J Stu

	Packet::staticCtor();

	{
		MaterialColor::staticCtor();
		Material::staticCtor();
		Tile::staticCtor();
		HatchetItem::staticCtor();
		PickaxeItem::staticCtor();
		ShovelItem::staticCtor();
		BlockReplacements::staticCtor();
		Biome::staticCtor();
		MobEffect::staticCtor();
		Item::staticCtor();
		FurnaceRecipes::staticCtor();
		Recipes::staticCtor();	
#ifdef _DURANGO
		GenericStats::setInstance(new DurangoStats());
#else
		GenericStats::setInstance(new CommonStats());
		Stats::staticCtor();
#endif
		//Achievements::staticCtor(); // 4J Stu - This is now called from within the Stats::staticCtor()
		TileEntity::staticCtor();
		EntityIO::staticCtor();
		MobCategory::staticCtor();

		Item::staticInit();
		LevelChunk::staticCtor();

		LevelType::staticCtor();

		{
			StructureFeatureIO::staticCtor();

			MineShaftPieces::staticCtor();
			StrongholdFeature::staticCtor();
			VillagePieces::Smithy::staticCtor();
			VillageFeature::staticCtor();
			RandomScatteredLargeFeature::staticCtor();
		}
	}
	EnderMan::staticCtor();
	PotionBrewing::staticCtor();
	Enchantment::staticCtor();

	SharedConstants::staticCtor();

	ServerLevel::staticCtor();
	SparseLightStorage::staticCtor();
	CompressedTileStorage::staticCtor();
	SparseDataStorage::staticCtor();
	McRegionChunkStorage::staticCtor();
	Villager::staticCtor();
	GameType::staticCtor();
	BeaconTileEntity::staticCtor();
}
