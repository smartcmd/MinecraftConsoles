#include "stdafx.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.locale.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "Tile.h"

wstring Tile::TILE_DESCRIPTION_PREFIX = L"Tile."; 

const float Tile::INDESTRUCTIBLE_DESTROY_TIME = -1.0f;

Tile::SoundType *Tile::SOUND_NORMAL = NULL;
Tile::SoundType *Tile::SOUND_WOOD = NULL;
Tile::SoundType *Tile::SOUND_GRAVEL = NULL;
Tile::SoundType *Tile::SOUND_GRASS = NULL; 
Tile::SoundType *Tile::SOUND_STONE = NULL;
Tile::SoundType *Tile::SOUND_METAL = NULL;
Tile::SoundType *Tile::SOUND_GLASS = NULL;
Tile::SoundType *Tile::SOUND_CLOTH = NULL;
Tile::SoundType *Tile::SOUND_SAND = NULL;
Tile::SoundType *Tile::SOUND_SNOW = NULL;
Tile::SoundType *Tile::SOUND_LADDER = NULL;
Tile::SoundType *Tile::SOUND_ANVIL = NULL;

bool Tile::solid[TILE_NUM_COUNT];
int Tile::lightBlock[TILE_NUM_COUNT];
bool Tile::transculent[TILE_NUM_COUNT];
int Tile::lightEmission[TILE_NUM_COUNT];
unsigned char Tile::_sendTileData[TILE_NUM_COUNT];		// 4J changed - was bool, now bitfield to indicate which bits are important to be sent
bool Tile::mipmapEnable[TILE_NUM_COUNT];
bool Tile::propagate[TILE_NUM_COUNT];

Tile **Tile::tiles = NULL;

Tile *Tile::rock = NULL;
GrassTile *Tile::grass = NULL;
Tile *Tile::dirt = NULL;
Tile *Tile::stoneBrick = NULL;
Tile *Tile::wood = NULL;
Tile *Tile::sapling = NULL;
Tile *Tile::unbreakable = NULL;
LiquidTile *Tile::water = NULL;
Tile *Tile::calmWater = NULL;
LiquidTile *Tile::lava = NULL;
Tile *Tile::calmLava = NULL;
Tile *Tile::sand = NULL;
Tile *Tile::gravel = NULL;
Tile *Tile::goldOre = NULL;
Tile *Tile::ironOre = NULL;
Tile *Tile::coalOre = NULL;
Tile *Tile::treeTrunk = NULL;
LeafTile *Tile::leaves = NULL;
Tile *Tile::sponge = NULL;
Tile *Tile::glass = NULL;
Tile *Tile::lapisOre = NULL;
Tile *Tile::lapisBlock = NULL;
Tile *Tile::dispenser = NULL;
Tile *Tile::sandStone = NULL;
Tile *Tile::musicBlock = NULL;
Tile *Tile::bed = NULL;
Tile *Tile::goldenRail = NULL;
Tile *Tile::detectorRail = NULL;
PistonBaseTile *Tile::pistonStickyBase = NULL;
Tile *Tile::web = NULL;
TallGrass *Tile::tallgrass = NULL;
DeadBushTile *Tile::deadBush = NULL;
PistonBaseTile *Tile::pistonBase = NULL;
PistonExtensionTile *Tile::pistonExtension = NULL;
Tile *Tile::cloth = NULL;
PistonMovingPiece *Tile::pistonMovingPiece = NULL;
Bush *Tile::flower = NULL;
Bush *Tile::rose = NULL;
Bush *Tile::mushroom1 = NULL;
Bush *Tile::mushroom2 = NULL;
Tile *Tile::goldBlock = NULL;
Tile *Tile::ironBlock = NULL;
HalfSlabTile *Tile::stoneSlab = NULL;
HalfSlabTile *Tile::stoneSlabHalf = NULL;
Tile *Tile::redBrick = NULL;
Tile *Tile::tnt = NULL;
Tile *Tile::bookshelf = NULL;
Tile *Tile::mossStone = NULL;
Tile *Tile::obsidian = NULL;
Tile *Tile::torch = NULL;
FireTile *Tile::fire = NULL;
Tile *Tile::mobSpawner = NULL;
Tile *Tile::stairs_wood = NULL;
ChestTile *Tile::chest = NULL;
RedStoneDustTile *Tile::redStoneDust = NULL;
Tile *Tile::diamondOre = NULL;
Tile *Tile::diamondBlock = NULL;
Tile *Tile::workBench = NULL;
Tile *Tile::crops = NULL;
Tile *Tile::farmland = NULL;
Tile *Tile::furnace = NULL;
Tile *Tile::furnace_lit = NULL;
Tile *Tile::sign = NULL;
Tile *Tile::door_wood = NULL;
Tile *Tile::ladder = NULL;
Tile *Tile::rail = NULL;
Tile *Tile::stairs_stone = NULL;
Tile *Tile::wallSign = NULL;
Tile *Tile::lever = NULL;
Tile *Tile::pressurePlate_stone = NULL;
Tile *Tile::door_iron = NULL;
Tile *Tile::pressurePlate_wood = NULL;
Tile *Tile::redStoneOre = NULL;
Tile *Tile::redStoneOre_lit = NULL;
Tile *Tile::notGate_off = NULL;
Tile *Tile::notGate_on = NULL;
Tile *Tile::button = NULL;
Tile *Tile::topSnow = NULL;
Tile *Tile::ice = NULL;
Tile *Tile::snow = NULL;
Tile *Tile::cactus = NULL;
Tile *Tile::clay = NULL;
Tile *Tile::reeds = NULL;
Tile *Tile::recordPlayer = NULL;
Tile *Tile::fence = NULL;
Tile *Tile::pumpkin = NULL;
Tile *Tile::hellRock = NULL;
Tile *Tile::hellSand = NULL;
Tile *Tile::lightGem = NULL;
PortalTile *Tile::portalTile = NULL;
Tile *Tile::litPumpkin = NULL;
Tile *Tile::cake = NULL;
RepeaterTile *Tile::diode_off = NULL;
RepeaterTile *Tile::diode_on = NULL;
Tile *Tile::aprilFoolsJoke = NULL;
Tile *Tile::trapdoor = NULL;

Tile *Tile::monsterStoneEgg = NULL;
Tile *Tile::stoneBrickSmooth = NULL;
Tile *Tile::hugeMushroom1 = NULL;
Tile *Tile::hugeMushroom2 = NULL;
Tile *Tile::ironFence = NULL;
Tile *Tile::thinGlass = NULL;
Tile *Tile::melon = NULL;
Tile *Tile::pumpkinStem = NULL;
Tile *Tile::melonStem = NULL;
Tile *Tile::vine = NULL;
Tile *Tile::fenceGate = NULL;
Tile *Tile::stairs_bricks = NULL;
Tile *Tile::stairs_stoneBrickSmooth = NULL;

MycelTile *Tile::mycel = NULL;
Tile *Tile::waterLily = NULL;
Tile *Tile::netherBrick = NULL;
Tile *Tile::netherFence = NULL;
Tile *Tile::stairs_netherBricks = NULL;
Tile *Tile::netherStalk = NULL;
Tile *Tile::enchantTable = NULL;
Tile *Tile::brewingStand = NULL;
CauldronTile *Tile::cauldron = NULL;
Tile *Tile::endPortalTile = NULL;
Tile *Tile::endPortalFrameTile = NULL;
Tile *Tile::whiteStone = NULL;
Tile *Tile::dragonEgg = NULL;
Tile *Tile::redstoneLight = NULL;
Tile *Tile::redstoneLight_lit = NULL;

// TU9
Tile *Tile::stairs_sandstone = NULL;
Tile *Tile::woodStairsDark = NULL;
Tile *Tile::woodStairsBirch = NULL;
Tile *Tile::woodStairsJungle = NULL;

Tile *Tile::button_wood = NULL;
HalfSlabTile *Tile::woodSlab = NULL;
HalfSlabTile *Tile::woodSlabHalf = NULL;

Tile *Tile::emeraldOre = NULL;
Tile *Tile::enderChest = NULL;
TripWireSourceTile *Tile::tripWireSource = NULL;
Tile *Tile::tripWire = NULL;
Tile *Tile::emeraldBlock = NULL;
 

Tile *Tile::cocoa = NULL;
Tile *Tile::skull = NULL;

Tile *Tile::cobbleWall = NULL;
Tile *Tile::flowerPot = NULL;
Tile *Tile::carrots = NULL;
Tile *Tile::potatoes = NULL;
Tile *Tile::anvil = NULL;
Tile *Tile::netherQuartz = NULL;
Tile *Tile::quartzBlock = NULL;
Tile *Tile::stairs_quartz = NULL;

Tile *Tile::woolCarpet = NULL;

DWORD Tile::tlsIdxShape = TlsAlloc();

Tile::ThreadStorage::ThreadStorage()
{
	xx0 = yy0 = zz0 = xx1 = yy1 = zz1 = 0.0;
	tileId = 0;
}

void Tile::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	TlsSetValue(Tile::tlsIdxShape, tls);
}

void Tile::ReleaseThreadStorage()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	delete tls;
}

void Tile::staticCtor()
{
	Tile::SOUND_NORMAL = new Tile::SoundType(eMaterialSoundType_STONE, 1, 1);
	Tile::SOUND_WOOD = new Tile::SoundType(eMaterialSoundType_WOOD, 1, 1);
	Tile::SOUND_GRAVEL = new Tile::SoundType(eMaterialSoundType_GRAVEL, 1, 1);
	Tile::SOUND_GRASS = new Tile::SoundType(eMaterialSoundType_GRASS, 1, 1);
	Tile::SOUND_STONE = new Tile::SoundType(eMaterialSoundType_STONE, 1, 1);
	Tile::SOUND_METAL = new Tile::SoundType(eMaterialSoundType_STONE, 1, 1.5f);
	Tile::SOUND_GLASS = new Tile::SoundType(eMaterialSoundType_STONE, 1, 1, eSoundType_RANDOM_GLASS,eSoundType_STEP_STONE);
	Tile::SOUND_CLOTH = new Tile::SoundType(eMaterialSoundType_CLOTH, 1, 1);
	Tile::SOUND_SAND = new Tile::SoundType(eMaterialSoundType_SAND, 1, 1);
	Tile::SOUND_SNOW = new Tile::SoundType(eMaterialSoundType_SNOW, 1, 1);
	Tile::SOUND_LADDER = new Tile::SoundType(eMaterialSoundType_LADDER, 1, 1,eSoundType_DIG_WOOD);
	Tile::SOUND_ANVIL = new Tile::SoundType(eMaterialSoundType_ANVIL, 0.3f, 1, eSoundType_DIG_STONE, eSoundType_RANDOM_ANVIL_LAND);

	Tile::tiles = new Tile *[TILE_NUM_COUNT];
	memset( tiles, 0, sizeof( Tile *)*TILE_NUM_COUNT );

	Tile::rail = (new RailTile(66, false))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_iron)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"rail")->setDescriptionId(IDS_TILE_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_RAIL)->disableMipmap();
	Tile::goldenRail = (new RailTile(27, true))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_gold)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"goldenRail")->setDescriptionId(IDS_TILE_GOLDEN_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_POWEREDRAIL)->disableMipmap();
	Tile::detectorRail = (new DetectorRailTile(28))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_detector)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"detectorRail")->setDescriptionId(IDS_TILE_DETECTOR_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_DETECTORRAIL)->disableMipmap();

	Tile::goldBlock = (new MetalTile(41))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_gold)->setDestroyTime(3.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"blockGold")->setDescriptionId(IDS_TILE_BLOCK_GOLD)->setUseDescriptionId(IDS_DESC_BLOCK_GOLD);
	Tile::ironBlock = (new MetalTile(42))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_iron)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"blockIron")->setDescriptionId(IDS_TILE_BLOCK_IRON)->setUseDescriptionId(IDS_DESC_BLOCK_IRON);
	Tile::lapisBlock = (new Tile(22, Material::stone))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_lapis)->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"blockLapis")->setDescriptionId(IDS_TILE_BLOCK_LAPIS)->setUseDescriptionId(IDS_DESC_BLOCK_LAPIS);
	Tile::musicBlock = (new MusicTile(25))					->setDestroyTime(0.8f)->setTextureName(L"musicBlock")->setDescriptionId(IDS_TILE_MUSIC_BLOCK)->sendTileData()->setUseDescriptionId(IDS_DESC_NOTEBLOCK);
	Tile::diamondBlock = (new MetalTile(57))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_diamond)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"blockDiamond")->setDescriptionId(IDS_TILE_BLOCK_DIAMOND)->setUseDescriptionId(IDS_DESC_BLOCK_DIAMOND);

	Tile::goldOre = (new OreTile(14))		->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreGold")->setDescriptionId(IDS_TILE_ORE_GOLD)->setUseDescriptionId(IDS_DESC_ORE_GOLD);
	Tile::ironOre = (new OreTile(15))		->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreIron")->setDescriptionId(IDS_TILE_ORE_IRON)->setUseDescriptionId(IDS_DESC_ORE_IRON);
	Tile::coalOre = (new OreTile(16))		->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreCoal")->setDescriptionId(IDS_TILE_ORE_COAL)->setUseDescriptionId(IDS_DESC_ORE_COAL);
	Tile::lapisOre = (new OreTile(21))		->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreLapis")->setDescriptionId(IDS_TILE_ORE_LAPIS)->setUseDescriptionId(IDS_DESC_ORE_LAPIS);
	Tile::diamondOre = (new OreTile(56))	->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreDiamond")->setDescriptionId(IDS_TILE_ORE_DIAMOND)->setUseDescriptionId(IDS_DESC_ORE_DIAMOND);


	Tile::rock = (new StoneTile(1))								->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"stone")->setDescriptionId(IDS_TILE_STONE)->setUseDescriptionId(IDS_DESC_STONE);
	Tile::grass = (GrassTile *) (new GrassTile(2))				->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"grass")->setDescriptionId(IDS_TILE_GRASS)->setUseDescriptionId(IDS_DESC_GRASS);
	Tile::dirt = (new DirtTile(3))								->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_GRAVEL)->setTextureName(L"dirt")->setDescriptionId(IDS_TILE_DIRT)->setUseDescriptionId(IDS_DESC_DIRT);
	Tile::sapling = (new Sapling(6))							->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"sapling")->setDescriptionId(IDS_TILE_SAPLING)->sendTileData()->setUseDescriptionId(IDS_DESC_SAPLING)->disableMipmap();
	Tile::unbreakable = (new Tile(7, Material::stone))			->setIndestructible()->setExplodeable(6000000)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"bedrock")->setDescriptionId(IDS_TILE_BEDROCK)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_BEDROCK);
	Tile::water = (LiquidTile *)(new LiquidTileDynamic(8, Material::water))	->setDestroyTime(100.0f)->setLightBlock(3)->setTextureName(L"water")->setDescriptionId(IDS_TILE_WATER)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_WATER);
	Tile::calmWater = (new LiquidTileStatic(9, Material::water))->setDestroyTime(100.0f)->setLightBlock(3)->setTextureName(L"water")->setDescriptionId(IDS_TILE_WATER)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_WATER);
	Tile::lava = (LiquidTile *)(new LiquidTileDynamic(10, Material::lava))	->setDestroyTime(00.0f)->setLightEmission(1.0f)->setLightBlock(255)->setTextureName(L"lava")->setDescriptionId(IDS_TILE_LAVA)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_LAVA);
	Tile::calmLava = (new LiquidTileStatic(11, Material::lava))	->setDestroyTime(100.0f)->setLightEmission(1.0f)->setLightBlock(255)->setTextureName(L"lava")->setDescriptionId(IDS_TILE_LAVA)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_LAVA);
	Tile::sand = (new HeavyTile(12))							->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_SAND)->setTextureName(L"sand")->setDescriptionId(IDS_TILE_SAND)->setUseDescriptionId(IDS_DESC_SAND);
	Tile::gravel = (new GravelTile(13))							->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setTextureName(L"gravel")->setDescriptionId(IDS_TILE_GRAVEL)->setUseDescriptionId(IDS_DESC_GRAVEL);
	Tile::treeTrunk = (new TreeTile(17))->setDestroyTime(2.0f)	->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"log")->setDescriptionId(IDS_TILE_LOG)->sendTileData()->setUseDescriptionId(IDS_DESC_LOG);
	// 4J - for leaves, have specified that only the data bits that encode the type of leaf are important to be sent
	Tile::leaves = (LeafTile *)(new LeafTile(18))				->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"leaves")->setDescriptionId(IDS_TILE_LEAVES)->sendTileData(LeafTile::LEAF_TYPE_MASK)->setUseDescriptionId(IDS_DESC_LEAVES);
	Tile::sponge = (new Sponge(19))								->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"sponge")->setDescriptionId(IDS_TILE_SPONGE)->setUseDescriptionId(IDS_DESC_SPONGE);
	Tile::glass = (new GlassTile(20, Material::glass, false))	->setDestroyTime(0.3f)->setSoundType(Tile::SOUND_GLASS)->setTextureName(L"glass")->setDescriptionId(IDS_TILE_GLASS)->setUseDescriptionId(IDS_DESC_GLASS);
	Tile::dispenser = (new DispenserTile(23))					->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"dispenser")->setDescriptionId(IDS_TILE_DISPENSER)->sendTileData()->setUseDescriptionId(IDS_DESC_DISPENSER);
	
	//Tile::wood = (new Tile(5, 4, Material::wood))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_structwoodstuff,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_WOOD)->setDescriptionId(IDS_TILE_WOOD)->sendTileData()->setUseDescriptionId(IDS_DESC_WOODENPLANKS);
	Tile::wood = (new WoodTile(5))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_structwoodstuff,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"wood")->setDescriptionId(IDS_TILE_OAKWOOD_PLANKS)->sendTileData()->setUseDescriptionId(IDS_DESC_WOODENPLANKS);
	Tile::sandStone = (new SandStoneTile(24))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_sand)->setSoundType(Tile::SOUND_STONE)->setDestroyTime(0.8f)->sendTileData()->setTextureName(L"sandStone")->setDescriptionId(IDS_TILE_SANDSTONE)->setUseDescriptionId(IDS_DESC_SANDSTONE)->sendTileData();
	Tile::stoneBrick = (new Tile(4, Material::stone))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"stonebrick")->setDescriptionId(IDS_TILE_STONE_BRICK)->setUseDescriptionId(IDS_DESC_STONE_BRICK);
	Tile::redBrick = (new Tile(45, Material::stone))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_brick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"brick")->setDescriptionId(IDS_TILE_BRICK)->setUseDescriptionId(IDS_DESC_BRICK);
	Tile::clay = (new ClayTile(82))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_clay)->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setTextureName(L"clay")->setDescriptionId(IDS_TILE_CLAY)->setUseDescriptionId(IDS_DESC_CLAY_TILE);
	Tile::snow = (new SnowTile(80))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_snow)->setDestroyTime(0.2f)->setSoundType(Tile::SOUND_CLOTH)->setTextureName(L"snow")->setDescriptionId(IDS_TILE_SNOW)->setUseDescriptionId(IDS_DESC_SNOW);
	
	Tile::torch = (new TorchTile(50))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_wood)->setDestroyTime(0.0f)->setLightEmission(15 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"torch")->setDescriptionId(IDS_TILE_TORCH)->sendTileData()->setUseDescriptionId(IDS_DESC_TORCH)->disableMipmap();
	Tile::litPumpkin = (new PumpkinTile(91, true))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_pumpkin)->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setLightEmission(1.0f)->setTextureName(L"litpumpkin")->setDescriptionId(IDS_TILE_LIT_PUMPKIN)->sendTileData()->setUseDescriptionId(IDS_DESC_JACKOLANTERN);
	Tile::lightGem = (new LightGemTile(89, Material::glass))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_glowstone)->setDestroyTime(0.3f)->setSoundType(Tile::SOUND_GLASS)->setLightEmission(1.0f)->setTextureName(L"lightgem")->setDescriptionId(IDS_TILE_LIGHT_GEM)->setUseDescriptionId(IDS_DESC_GLOWSTONE);

	Tile::trapdoor = (new TrapDoorTile(96, Material::wood))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_door,	Item::eMaterial_trap)->setDestroyTime(3.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"trapdoor")->setDescriptionId(IDS_TILE_TRAPDOOR)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_TRAPDOOR);

	Tile::bed = (new BedTile(26))													->setDestroyTime(0.2f)->setTextureName(L"bed")->setDescriptionId(IDS_TILE_BED)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_BED);
	Tile::pistonStickyBase = (PistonBaseTile *)(new PistonBaseTile(29, true))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_piston,	Item::eMaterial_stickypiston)->setTextureName(L"pistonStickyBase")->setDescriptionId(IDS_TILE_PISTON_STICK_BASE)->setUseDescriptionId(IDS_DESC_STICKY_PISTON)->sendTileData();
	Tile::web = (new WebTile(30))													->setLightBlock(1)->setDestroyTime(4.0f)->setTextureName(L"web")->setDescriptionId(IDS_TILE_WEB)->setUseDescriptionId(IDS_DESC_WEB);
	Tile::tallgrass = (TallGrass *)(new TallGrass(31))								->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"tallgrass")->setDescriptionId(IDS_TILE_TALL_GRASS)->setUseDescriptionId(IDS_DESC_TALL_GRASS)->disableMipmap();
	Tile::deadBush = (DeadBushTile *)(new DeadBushTile(32))							->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"deadbush")->setDescriptionId(IDS_TILE_DEAD_BUSH)->setUseDescriptionId(IDS_DESC_DEAD_BUSH)->disableMipmap();
	Tile::pistonBase = (PistonBaseTile *)(new PistonBaseTile(33,false))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_piston,	Item::eMaterial_piston)->setTextureName(L"pistonBase")->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(IDS_DESC_PISTON)->sendTileData();
	Tile::pistonExtension = (PistonExtensionTile *)(new PistonExtensionTile(34))	->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(-1)->sendTileData();

	Tile::cloth = (new ClothTile())												->setBaseItemTypeAndMaterial(Item::eBaseItemType_cloth,	Item::eMaterial_cloth)->setDestroyTime(0.8f)->setSoundType(Tile::SOUND_CLOTH)->setTextureName(L"cloth")->setDescriptionId(IDS_TILE_CLOTH)->sendTileData()->setUseDescriptionId(IDS_DESC_WOOL);
	Tile::pistonMovingPiece = (PistonMovingPiece *)(new PistonMovingPiece(36))	->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(-1);

	Tile::pressurePlate_stone = (Tile *)(new PressurePlateTile(70, L"stone", Material::stone, PressurePlateTile::mobs))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_stone)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"pressurePlate")->setDescriptionId(IDS_TILE_PRESSURE_PLATE)->sendTileData()->setUseDescriptionId(IDS_DESC_PRESSUREPLATE);
	Tile::pressurePlate_wood = (new PressurePlateTile(72, L"wood", Material::wood, PressurePlateTile::everything))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_wood)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"pressurePlate")->setDescriptionId(IDS_TILE_PRESSURE_PLATE)->sendTileData()->setUseDescriptionId(IDS_DESC_PRESSUREPLATE);

	Tile::flower = (Bush *) (new Bush(37))								->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"flower")->setDescriptionId(IDS_TILE_FLOWER)->setUseDescriptionId(IDS_DESC_FLOWER)->disableMipmap();
	Tile::rose = (Bush *) (new Bush(38))								->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"rose")->setDescriptionId(IDS_TILE_ROSE)->setUseDescriptionId(IDS_DESC_FLOWER)->disableMipmap();
	Tile::mushroom1 = (Bush *) (new Mushroom(39, L"mushroom_brown"))	->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setLightEmission(2 / 16.0f)->setTextureName(L"mushroom")->setDescriptionId(IDS_TILE_MUSHROOM)->setUseDescriptionId(IDS_DESC_MUSHROOM)->disableMipmap();
	Tile::mushroom2 = (Bush *) (new Mushroom(40, L"mushroom_red"))		->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"mushroom")->setDescriptionId(IDS_TILE_MUSHROOM)->setUseDescriptionId(IDS_DESC_MUSHROOM)->disableMipmap();

// 	Tile::stoneSlab = (new StoneSlabTile(43, true))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_slab,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_SLAB);
// 	Tile::stoneSlabHalf = (new StoneSlabTile(44, false))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_halfslab,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_HALFSLAB);

	
	Tile::tnt = (new TntTile(46))						->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"tnt")->setDescriptionId(IDS_TILE_TNT)->setUseDescriptionId(IDS_DESC_TNT);
	Tile::bookshelf = (new BookshelfTile(47))			->setDestroyTime(1.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"bookshelf")->setDescriptionId(IDS_TILE_BOOKSHELF)->setUseDescriptionId(IDS_DESC_BOOKSHELF);
	Tile::mossStone = (new Tile(48, Material::stone))	->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"stoneMoss")->setDescriptionId(IDS_TILE_STONE_MOSS)->setUseDescriptionId(IDS_DESC_MOSS_STONE);
	// 4J - change of destroy time from 10.0f -> 50.0f for obsidian brought forward from 1.2.3
	Tile::obsidian = (new ObsidianTile(49))				->setDestroyTime(50.0f)->setExplodeable(2000)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"obsidian")->setDescriptionId(IDS_TILE_OBSIDIAN)->setUseDescriptionId(IDS_DESC_OBSIDIAN);
	

	Tile::fire = (FireTile *) ((new FireTile(51))				->setDestroyTime(0.0f)->setLightEmission(1.0f)->setSoundType(Tile::SOUND_WOOD))->setTextureName(L"fire")->setDescriptionId(IDS_TILE_FIRE)->setNotCollectStatistics()->setUseDescriptionId(-1);
	Tile::mobSpawner = (new MobSpawnerTile(52))					->setDestroyTime(5.0f)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"mobSpawner")->setDescriptionId(IDS_TILE_MOB_SPAWNER)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_MOB_SPAWNER);
	
	Tile::chest = (ChestTile *)(new ChestTile(54))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_chest,	Item::eMaterial_ender)->setDestroyTime(2.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"chest")->setDescriptionId(IDS_TILE_CHEST)->sendTileData()->setUseDescriptionId(IDS_DESC_CHEST);
	Tile::redStoneDust = (RedStoneDustTile *)(new RedStoneDustTile(55))				->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_NORMAL)->setTextureName(L"redstoneDust")->setDescriptionId(IDS_TILE_REDSTONE_DUST)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONE_DUST);
	Tile::workBench = (new WorkbenchTile(58))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_wood)->setDestroyTime(2.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"workbench")->setDescriptionId(IDS_TILE_WORKBENCH)->setUseDescriptionId(IDS_DESC_CRAFTINGTABLE);
	Tile::crops = (new CropTile(59))							->setTextureName(L"crops")->setDescriptionId(IDS_TILE_CROPS)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_CROPS)->disableMipmap();
	Tile::farmland = (new FarmTile(60))							->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setTextureName(L"farmland")->setDescriptionId(IDS_TILE_FARMLAND)->setUseDescriptionId(IDS_DESC_FARMLAND)->sendTileData();
	Tile::furnace = (new FurnaceTile(61, false))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_stone)->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"furnace")->setDescriptionId(IDS_TILE_FURNACE)->sendTileData()->setUseDescriptionId(IDS_DESC_FURNACE);
	Tile::furnace_lit = (new FurnaceTile(62, true))				->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setLightEmission(14 / 16.0f)->setTextureName(L"furnace")->setDescriptionId(IDS_TILE_FURNACE)->sendTileData()->setUseDescriptionId(IDS_DESC_FURNACE);
	Tile::sign = (new SignTile(63, eTYPE_SIGNTILEENTITY, true))->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"sign")->setDescriptionId(IDS_TILE_SIGN)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_SIGN);
	Tile::door_wood = (new DoorTile(64, Material::wood))		->setDestroyTime(3.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"doorWood")->setDescriptionId(IDS_TILE_DOOR_WOOD)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Tile::ladder = (new LadderTile(65))							->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"ladder")->setDescriptionId(IDS_TILE_LADDER)->sendTileData()->setUseDescriptionId(IDS_DESC_LADDER)->disableMipmap();
	Tile::wallSign = (new SignTile(68, eTYPE_SIGNTILEENTITY, false))->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"sign")->setDescriptionId(IDS_TILE_SIGN)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_SIGN);
	Tile::lever = (new LeverTile(69))							->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"lever")->setDescriptionId(IDS_TILE_LEVER)->sendTileData()->setUseDescriptionId(IDS_DESC_LEVER);
	Tile::door_iron = (new DoorTile(71, Material::metal))		->setDestroyTime(5.0f)->setSoundType(Tile::SOUND_METAL)->setTextureName(L"doorIron")->setDescriptionId(IDS_TILE_DOOR_IRON)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_DOOR_IRON);
	Tile::redStoneOre = (new RedStoneOreTile(73,false))			->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreRedstone")->setDescriptionId(IDS_TILE_ORE_REDSTONE)->sendTileData()->setUseDescriptionId(IDS_DESC_ORE_REDSTONE);
	Tile::redStoneOre_lit = (new RedStoneOreTile(74, true))		->setLightEmission(10 / 16.0f)->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"oreRedstone")->setDescriptionId(IDS_TILE_ORE_REDSTONE)->sendTileData()->setUseDescriptionId(IDS_DESC_ORE_REDSTONE);
	Tile::notGate_off = (new NotGateTile(75, false))			->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"notGate")->setDescriptionId(IDS_TILE_NOT_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONETORCH)->disableMipmap();
	Tile::notGate_on = (new NotGateTile(76, true))				->setDestroyTime(0.0f)->setLightEmission(8 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"notGate")->setDescriptionId(IDS_TILE_NOT_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONETORCH)->disableMipmap();
	Tile::button = (new ButtonTile(77,false))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_button,	Item::eMaterial_stone)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"button")->setDescriptionId(IDS_TILE_BUTTON)->sendTileData()->setUseDescriptionId(IDS_DESC_BUTTON);
	Tile::topSnow = (new TopSnowTile(78))						->setDestroyTime(0.1f)->setSoundType(Tile::SOUND_CLOTH)->setTextureName(L"snow")->setDescriptionId(IDS_TILE_SNOW)->setUseDescriptionId(IDS_DESC_TOP_SNOW)->sendTileData()->setLightBlock(0);
	Tile::ice = (new IceTile(79))								->setDestroyTime(0.5f)->setLightBlock(3)->setSoundType(Tile::SOUND_GLASS)->setTextureName(L"ice")->setDescriptionId(IDS_TILE_ICE)->setUseDescriptionId(IDS_DESC_ICE);
	Tile::cactus = (new CactusTile(81))							->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_CLOTH)->setTextureName(L"cactus")->setDescriptionId(IDS_TILE_CACTUS)->setUseDescriptionId(IDS_DESC_CACTUS)->disableMipmap();
	Tile::reeds = (new ReedTile(83))							->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setTextureName(L"reeds")->setDescriptionId(IDS_TILE_REEDS)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_REEDS)->disableMipmap();
	Tile::recordPlayer = (new RecordPlayerTile(84))				->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"jukebox")->setDescriptionId(IDS_TILE_JUKEBOX)->sendTileData()->setUseDescriptionId(IDS_DESC_JUKEBOX);
	Tile::fence = (new FenceTile(85, L"wood", Material::wood))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"fence")->setDescriptionId(IDS_TILE_FENCE)->setUseDescriptionId(IDS_DESC_FENCE);

	Tile::pumpkin = (new PumpkinTile(86, false))	->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"pumpkin")->setDescriptionId(IDS_TILE_PUMPKIN)->sendTileData()->setUseDescriptionId(IDS_DESC_PUMPKIN);

	Tile::hellRock = (new HellStoneTile(87))			->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"hellrock")->setDescriptionId(IDS_TILE_HELL_ROCK)->setUseDescriptionId(IDS_DESC_HELL_ROCK);
	Tile::hellSand = (new HellSandTile(88))				->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_SAND)->setTextureName(L"hellsand")->setDescriptionId(IDS_TILE_HELL_SAND)->setUseDescriptionId(IDS_DESC_HELL_SAND);
	Tile::portalTile = (PortalTile *) ((new PortalTile(90))		->setDestroyTime(-1)->setSoundType(Tile::SOUND_GLASS)->setLightEmission(0.75f))->setTextureName(L"portal")->setDescriptionId(IDS_TILE_PORTAL)->setUseDescriptionId(IDS_DESC_PORTAL);
	Tile::cake = (new CakeTile(92))						->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_CLOTH)->setTextureName(L"cake")->setDescriptionId(IDS_TILE_CAKE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_CAKE);

	// TODO Copy the translations from IDS_ITEM_DIODE to IDS_TILE_DIODE
	Tile::diode_off = (RepeaterTile *)(new DiodeTile(93, false))	->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"diode")->setDescriptionId(IDS_ITEM_DIODE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER)->disableMipmap();
	Tile::diode_on = (RepeaterTile *)(new DiodeTile(94, true))		->setDestroyTime(0.0f)->setLightEmission(10 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"diode")->setDescriptionId(IDS_ITEM_DIODE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER)->disableMipmap();
	Tile::aprilFoolsJoke = (new LockedChestTile(95))				->setDestroyTime(0.0f)->setLightEmission(16 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"lockedchest")->setDescriptionId(IDS_TILE_LOCKED_CHEST)->setTicking(true)->sendTileData()->setUseDescriptionId(-1);

    Tile::monsterStoneEgg = (new StoneMonsterTile(97))									->setDestroyTime(0.75f)->setTextureName(L"monsterStoneEgg")->setDescriptionId(IDS_TILE_STONE_SILVERFISH)->setUseDescriptionId(IDS_DESC_STONE_SILVERFISH);
    Tile::stoneBrickSmooth = (new SmoothStoneBrickTile(98))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_stoneSmooth)->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setTextureName(L"stonebricksmooth")->setDescriptionId(IDS_TILE_STONE_BRICK_SMOOTH)->setUseDescriptionId(IDS_DESC_STONE_BRICK_SMOOTH);
    Tile::hugeMushroom1 = (new HugeMushroomTile(99, Material::wood, 0))					->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setTextureName(L"mushroom")->setDescriptionId(IDS_TILE_HUGE_MUSHROOM_1)->setUseDescriptionId(IDS_DESC_MUSHROOM)->sendTileData();
    Tile::hugeMushroom2 = (new HugeMushroomTile(100, Material::wood, 1))				->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setTextureName(L"mushroom")->setDescriptionId(IDS_TILE_HUGE_MUSHROOM_2)->setUseDescriptionId(IDS_DESC_MUSHROOM)->sendTileData();
    Tile::ironFence = (new ThinFenceTile(101, L"fenceIron", L"fenceIron", Material::metal, true))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,		Item::eMaterial_iron)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setTextureName(L"fenceIron")->setDescriptionId(IDS_TILE_IRON_FENCE)->setUseDescriptionId(IDS_DESC_IRON_FENCE);
    Tile::thinGlass = (new ThinFenceTile(102, L"glass", L"thinglass_top", Material::glass, false))	->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setTextureName(L"thinGlass")->setDescriptionId(IDS_TILE_THIN_GLASS)->setUseDescriptionId(IDS_DESC_THIN_GLASS);
    Tile::melon = (new MelonTile(103))													->setDestroyTime(1.0f)->setSoundType(SOUND_WOOD)->setTextureName(L"melon")->setDescriptionId(IDS_TILE_MELON)->setUseDescriptionId(IDS_DESC_MELON_BLOCK);
    Tile::pumpkinStem = (new StemTile(104, Tile::pumpkin))								->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setTextureName(L"pumpkinStem")->setDescriptionId(IDS_TILE_PUMPKIN_STEM)->sendTileData();
    Tile::melonStem = (new StemTile(105, Tile::melon))									->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setTextureName(L"pumpkinStem")->setDescriptionId(IDS_TILE_MELON_STEM)->sendTileData();
    Tile::vine = (new VineTile(106))->setDestroyTime(0.2f)								->setSoundType(SOUND_GRASS)->setTextureName(L"vine")->setDescriptionId(IDS_TILE_VINE)->setUseDescriptionId(IDS_DESC_VINE)->sendTileData();
    Tile::fenceGate = (new FenceGateTile(107))											->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setTextureName(L"fenceGate")->setDescriptionId(IDS_TILE_FENCE_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_FENCE_GATE);


	Tile::stairs_wood =				(new StairTile(53, Tile::wood,0))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_wood)		->setTextureName(L"stairsWood")->setDescriptionId(IDS_TILE_STAIRS_WOOD)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::stairs_stone =			(new StairTile(67, Tile::stoneBrick,0))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_stone)		->setTextureName(L"stairsStone")->setDescriptionId(IDS_TILE_STAIRS_STONE)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::stairs_bricks =			(new StairTile(108, Tile::redBrick,0))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_brick)		->setTextureName(L"stairsBrick")->setDescriptionId(IDS_TILE_STAIRS_BRICKS)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::stairs_stoneBrickSmooth = (new StairTile(109, Tile::stoneBrickSmooth,0))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_stoneSmooth)->setTextureName(L"stairsStoneBrickSmooth")->setDescriptionId(IDS_TILE_STAIRS_STONE_BRICKS_SMOOTH)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);

    Tile::mycel =					(MycelTile *)(new MycelTile(110))					->setDestroyTime(0.6f)->setSoundType(SOUND_GRASS)->setTextureName(L"mycel")->setDescriptionId(IDS_TILE_MYCEL)->setUseDescriptionId(IDS_DESC_MYCEL);
    Tile::waterLily =				(new WaterlilyTile(111))							->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setTextureName(L"waterlily")->setDescriptionId(IDS_TILE_WATERLILY)->setUseDescriptionId(IDS_DESC_WATERLILY);
    Tile::netherBrick =				(new Tile(112, Material::stone))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_netherbrick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setTextureName(L"netherBrick")->setDescriptionId(IDS_TILE_NETHERBRICK)->setUseDescriptionId(IDS_DESC_NETHERBRICK);
    Tile::netherFence =				(new FenceTile(113, L"netherBrick", Material::stone))->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,		Item::eMaterial_netherbrick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setTextureName(L"netherFence")->setDescriptionId(IDS_TILE_NETHERFENCE)->setUseDescriptionId(IDS_DESC_NETHERFENCE);
    Tile::stairs_netherBricks =		(new StairTile(114, Tile::netherBrick,0))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_netherbrick)->setTextureName(L"stairsNetherBrick")->setDescriptionId(IDS_TILE_STAIRS_NETHERBRICK)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
    Tile::netherStalk =				(new NetherStalkTile(115))							->setTextureName(L"netherStalk")->setDescriptionId(IDS_TILE_NETHERSTALK)->sendTileData()->setUseDescriptionId(IDS_DESC_NETHERSTALK);
    Tile::enchantTable =			(new EnchantmentTableTile(116))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_magic)->setDestroyTime(5.0f)->setExplodeable(2000)->setTextureName(L"enchantmentTable")->setDescriptionId(IDS_TILE_ENCHANTMENTTABLE)->setUseDescriptionId(IDS_DESC_ENCHANTMENTTABLE);
    Tile::brewingStand =			(new BrewingStandTile(117))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_blaze)->setDestroyTime(0.5f)->setLightEmission(2 / 16.0f)->setTextureName(L"brewingStand")->setDescriptionId(IDS_TILE_BREWINGSTAND)->sendTileData()->setUseDescriptionId(IDS_DESC_BREWING_STAND);
    Tile::cauldron =				(CauldronTile *)(new CauldronTile(118))			->setDestroyTime(2.0f)->setTextureName(L"cauldron")->setDescriptionId(IDS_TILE_CAULDRON)->sendTileData()->setUseDescriptionId(IDS_DESC_CAULDRON);
    Tile::endPortalTile =			(new TheEndPortal(119, Material::portal))			->setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME)->setExplodeable(6000000)->setDescriptionId(IDS_TILE_END_PORTAL)->setUseDescriptionId(IDS_DESC_END_PORTAL);
    Tile::endPortalFrameTile =		(new TheEndPortalFrameTile(120))					->setSoundType(SOUND_GLASS)->setLightEmission(2 / 16.0f)->setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME)->setTextureName(L"endPortalFrame")->setDescriptionId(IDS_TILE_ENDPORTALFRAME)->sendTileData()->setExplodeable(6000000)->setUseDescriptionId(IDS_DESC_ENDPORTALFRAME);
    Tile::whiteStone =				(new Tile(121, Material::stone))		->setDestroyTime(3.0f)->setExplodeable(15)->setSoundType(SOUND_STONE)->setTextureName(L"whiteStone")->setDescriptionId(IDS_TILE_WHITESTONE)->setUseDescriptionId(IDS_DESC_WHITESTONE);
    Tile::dragonEgg =				(new EggTile(122))						->setDestroyTime(3.0f)->setExplodeable(15)->setSoundType(SOUND_STONE)->setLightEmission(2.0f / 16.0f)->setTextureName(L"dragonEgg")->setDescriptionId(IDS_TILE_DRAGONEGG)->setUseDescriptionId(IDS_DESC_DRAGONEGG);
	Tile::redstoneLight =			(new RedlightTile(123, false))			->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setTextureName(L"redstoneLight")->setDescriptionId(IDS_TILE_REDSTONE_LIGHT)->setUseDescriptionId(IDS_DESC_REDSTONE_LIGHT);
	Tile::redstoneLight_lit =		(new RedlightTile(124, true))			->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setTextureName(L"redstoneLight")->setDescriptionId(IDS_TILE_REDSTONE_LIGHT)->setUseDescriptionId(IDS_DESC_REDSTONE_LIGHT);

	// TU9
	Tile::stairs_sandstone =		(new StairTile(128, Tile::sandStone,0))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_sand)	->setTextureName(L"stairsSandstone")->setDescriptionId(IDS_TILE_STAIRS_SANDSTONE)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::woodStairsDark =			(new StairTile(134, Tile::wood, TreeTile::DARK_TRUNK))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_sprucewood)->setTextureName(L"stairsWoodSpruce")->setDescriptionId(IDS_TILE_STAIRS_SPRUCEWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::woodStairsBirch =			(new StairTile(135, Tile::wood, TreeTile::BIRCH_TRUNK))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_birchwood)->setTextureName(L"stairsWoodBirch")->setDescriptionId(IDS_TILE_STAIRS_BIRCHWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::woodStairsJungle =		(new StairTile(136, Tile::wood, TreeTile::JUNGLE_TRUNK))->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_junglewood)->setTextureName(L"stairsWoodJungle")->setDescriptionId(IDS_TILE_STAIRS_JUNGLEWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::button_wood =				(new ButtonTile(143, true))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_button,	Item::eMaterial_wood)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setTextureName(L"button")->setDescriptionId(IDS_TILE_BUTTON)->sendTileData()->setUseDescriptionId(IDS_DESC_BUTTON);

	Tile::woodSlab =		(HalfSlabTile *) (new WoodSlabTile(Tile::woodSlab_Id, true))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_slab,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setTextureName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Tile::woodSlabHalf =	(HalfSlabTile *) (new WoodSlabTile(Tile::woodSlabHalf_Id, false))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_halfslab,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setTextureName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Tile::stoneSlab =		(HalfSlabTile *) (new StoneSlabTile(Tile::stoneSlab_Id, true))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_slab,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_SLAB);
	Tile::stoneSlabHalf =	(HalfSlabTile *) (new StoneSlabTile(Tile::stoneSlabHalf_Id, false))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_halfslab,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setTextureName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_HALFSLAB);

	Tile::emeraldOre =		(new OreTile(129))										->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setTextureName(L"oreEmerald")->setDescriptionId(IDS_TILE_EMERALDORE)->setUseDescriptionId(IDS_DESC_EMERALDORE);
	Tile::enderChest =		(new EnderChestTile(130))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_chest,	Item::eMaterial_ender)->setDestroyTime(22.5f)->setExplodeable(1000)->setSoundType(SOUND_STONE)->setTextureName(L"enderChest")->sendTileData()->setLightEmission(.5f)->setDescriptionId(IDS_TILE_ENDERCHEST)->setUseDescriptionId(IDS_DESC_ENDERCHEST);
	Tile::tripWireSource =	(TripWireSourceTile *)( new TripWireSourceTile(131) )	->setTextureName(L"tripWireSource")->sendTileData()->setDescriptionId(IDS_TILE_TRIPWIRE_SOURCE)->setUseDescriptionId(IDS_DESC_TRIPWIRE_SOURCE);
	Tile::tripWire =		(new TripWireTile(132))									->setTextureName(L"tripWire")->sendTileData()->setDescriptionId(IDS_TILE_TRIPWIRE)->setUseDescriptionId(IDS_DESC_TRIPWIRE);
	Tile::emeraldBlock =	(new MetalTile(133))									->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_emerald)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setTextureName(L"blockEmerald")->setDescriptionId(IDS_TILE_EMERALDBLOCK)->setUseDescriptionId(IDS_DESC_EMERALDBLOCK);
    

	Tile::cocoa =			(new CocoaTile(127))		->setDestroyTime(0.2f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setTextureName(L"cocoa")->sendTileData()->setDescriptionId(IDS_TILE_COCOA)->setUseDescriptionId(IDS_DESC_COCOA);
	Tile::skull =			(new SkullTile(144))		->setDestroyTime(1.0f)->setSoundType(SOUND_STONE)->setTextureName(L"skull")->setDescriptionId(IDS_TILE_SKULL)->setUseDescriptionId(IDS_DESC_SKULL);

	Tile::cobbleWall =		(new WallTile(139, Tile::stoneBrick))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,	Item::eMaterial_stone)->setTextureName(L"cobbleWall")->setDescriptionId(IDS_TILE_COBBLESTONE_WALL)->setUseDescriptionId(IDS_DESC_COBBLESTONE_WALL);
	Tile::flowerPot =		(new FlowerPotTile(140))				->setDestroyTime(0.0f)->setSoundType(SOUND_NORMAL)->setTextureName(L"flowerPot")->setDescriptionId(IDS_TILE_FLOWERPOT)->setUseDescriptionId(IDS_DESC_FLOWERPOT);
	Tile::carrots =			(new CarrotTile(141))					->setTextureName(L"carrots")->setDescriptionId(IDS_TILE_CARROTS)->setUseDescriptionId(IDS_DESC_CARROTS)->disableMipmap();
	Tile::potatoes =		(new PotatoTile(142))					->setTextureName(L"potatoes")->setDescriptionId(IDS_TILE_POTATOES)->setUseDescriptionId(IDS_DESC_POTATO)->disableMipmap();
	Tile::anvil =			(new AnvilTile(145))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_device, Item::eMaterial_iron)->setDestroyTime(5.0f)->setSoundType(SOUND_ANVIL)->setExplodeable(2000)->setTextureName(L"anvil")->sendTileData()->setDescriptionId(IDS_TILE_ANVIL)->setUseDescriptionId(IDS_DESC_ANVIL);
	Tile::netherQuartz =	(new OreTile(153))						->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setTextureName(L"netherquartz")->setDescriptionId(IDS_TILE_NETHER_QUARTZ)->setUseDescriptionId(IDS_DESC_NETHER_QUARTZ_ORE);
	Tile::quartzBlock =		(new QuartzBlockTile(155))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_quartz)->setSoundType(SOUND_STONE)->setDestroyTime(0.8f)->setTextureName(L"quartzBlock")->setDescriptionId(IDS_TILE_QUARTZ_BLOCK)->setUseDescriptionId(IDS_DESC_QUARTZ_BLOCK);
	Tile::stairs_quartz =	(new StairTile(156, Tile::quartzBlock, QuartzBlockTile::TYPE_DEFAULT))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_quartz)->setTextureName(L"stairsQuartz")->setDescriptionId(IDS_TILE_STAIRS_QUARTZ)->setUseDescriptionId(IDS_DESC_STAIRS);

	Tile::woolCarpet =		(new WoolCarpetTile(171))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_carpet,	Item::eMaterial_cloth)->setDestroyTime(0.1f)->setSoundType(SOUND_CLOTH)->setTextureName(L"woolCarpet")->setLightBlock(0)->setDescriptionId(IDS_TILE_CARPET)->setUseDescriptionId(IDS_DESC_CARPET);

	// Special cases for certain items since they can have different icons
	Item::items[Tile::cloth_Id]				= ( new ClothTileItem(Tile::cloth_Id- 256) )->setTextureName(L"cloth")->setDescriptionId(IDS_TILE_CLOTH)->setUseDescriptionId(IDS_DESC_WOOL);
	Item::items[Tile::woolCarpet_Id]		= ( new ClothTileItem(Tile::woolCarpet_Id - 256))->setTextureName(L"woolCarpet")->setDescriptionId(IDS_TILE_CARPET)->setUseDescriptionId(IDS_DESC_CARPET);
	Item::items[Tile::treeTrunk_Id]			= ( new TreeTileItem(Tile::treeTrunk_Id - 256, treeTrunk) )->setTextureName(L"log")->setDescriptionId(IDS_TILE_LOG)->setUseDescriptionId(IDS_DESC_LOG);
	Item::items[Tile::wood_Id]				= ( new MultiTextureTileItem(Tile::wood_Id - 256, Tile::wood, (int *)WoodTile::WOOD_NAMES, 4))->setTextureName(L"wood")->setDescriptionId(IDS_TILE_OAKWOOD_PLANKS)->setUseDescriptionId(IDS_DESC_LOG); //  <- TODO
	Item::items[Tile::monsterStoneEgg_Id]	= ( new StoneMonsterTileItem(Tile::monsterStoneEgg_Id - 256))->setTextureName(L"monsterStoneEgg")->setDescriptionId(IDS_TILE_STONE_SILVERFISH)->setUseDescriptionId(IDS_DESC_STONE_SILVERFISH); // 4J - Brought forward from post-1.2 to fix stacking problem
	Item::items[Tile::stoneBrickSmooth_Id]	= ( new SmoothStoneBrickTileItem(Tile::stoneBrickSmooth_Id - 256, stoneBrickSmooth))->setTextureName(L"stonebricksmooth")->setDescriptionId(IDS_TILE_STONE_BRICK_SMOOTH);
	Item::items[Tile::sandStone_Id]			= ( new MultiTextureTileItem(sandStone_Id - 256, sandStone, SandStoneTile::SANDSTONE_NAMES, SandStoneTile::SANDSTONE_BLOCK_NAMES) )->setTextureName(L"sandStone")->setDescriptionId(IDS_TILE_SANDSTONE)->setUseDescriptionId(IDS_DESC_SANDSTONE);
	Item::items[Tile::quartzBlock_Id]		= ( new MultiTextureTileItem(quartzBlock_Id - 256, quartzBlock, QuartzBlockTile::BLOCK_NAMES, QuartzBlockTile::QUARTZ_BLOCK_NAMES) )->setTextureName(L"quartzBlock")->setDescriptionId(IDS_TILE_QUARTZ_BLOCK)->setUseDescriptionId(IDS_DESC_QUARTZ_BLOCK);
	Item::items[Tile::stoneSlabHalf_Id]		= ( new StoneSlabTileItem(Tile::stoneSlabHalf_Id - 256, Tile::stoneSlabHalf,	Tile::stoneSlab, false) )->setTextureName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_HALFSLAB);
	Item::items[Tile::stoneSlab_Id]			= ( new StoneSlabTileItem(Tile::stoneSlab_Id - 256,		Tile::stoneSlabHalf,	Tile::stoneSlab, true))->setTextureName(L"stoneSlab")->setDescriptionId(IDS_DESC_STONESLAB)->setUseDescriptionId(IDS_DESC_SLAB);
	Item::items[Tile::woodSlabHalf_Id]		= ( new StoneSlabTileItem(Tile::woodSlabHalf_Id - 256,	Tile::woodSlabHalf,		Tile::woodSlab, false))->setTextureName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Item::items[Tile::woodSlab_Id]			= ( new StoneSlabTileItem(Tile::woodSlab_Id - 256,		Tile::woodSlabHalf,		Tile::woodSlab, true))->setTextureName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Item::items[Tile::sapling_Id]			= ( new SaplingTileItem(Tile::sapling_Id - 256) )->setTextureName(L"sapling")->setDescriptionId(IDS_TILE_SAPLING)->setUseDescriptionId(IDS_DESC_SAPLING);
	Item::items[Tile::leaves_Id]			= ( new LeafTileItem(Tile::leaves_Id - 256) )->setTextureName(L"leaves")->setDescriptionId(IDS_TILE_LEAVES)->setUseDescriptionId(IDS_DESC_LEAVES);
	Item::items[Tile::vine_Id]				= ( new ColoredTileItem(Tile::vine_Id - 256, false))->setDescriptionId(IDS_TILE_VINE)->setUseDescriptionId(IDS_DESC_VINE);
	int idsData[3] = {IDS_TILE_SHRUB, IDS_TILE_GRASS, IDS_TILE_FERN};
	intArray ids = intArray(idsData, 3);
	Item::items[Tile::tallgrass_Id]			= ((ColoredTileItem *)(new ColoredTileItem(Tile::tallgrass_Id - 256, true))->setDescriptionId(IDS_TILE_TALL_GRASS))->setDescriptionPostfixes(ids);
	Item::items[Tile::waterLily_Id]			= ( new WaterLilyTileItem(Tile::waterLily_Id - 256));
	Item::items[Tile::pistonBase_Id]		= ( new PistonTileItem(Tile::pistonBase_Id - 256) )->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(IDS_DESC_PISTON);
	Item::items[Tile::pistonStickyBase_Id]	= ( new PistonTileItem(Tile::pistonStickyBase_Id - 256) )->setDescriptionId(IDS_TILE_PISTON_STICK_BASE)->setUseDescriptionId(IDS_DESC_STICKY_PISTON);
	Item::items[Tile::cobbleWall_Id]		= ( new MultiTextureTileItem(cobbleWall_Id - 256, cobbleWall, (int *)WallTile::COBBLE_NAMES, 2) )->setDescriptionId(IDS_TILE_COBBLESTONE_WALL)->setUseDescriptionId(IDS_DESC_COBBLESTONE_WALL);
	Item::items[Tile::anvil_Id]				= ( new AnvilTileItem(anvil) )->setDescriptionId(IDS_TILE_ANVIL)->setUseDescriptionId(IDS_DESC_ANVIL);


	for (int i = 0; i < 256; i++)
	{
		if ( Tile::tiles[i] != NULL )
		{
			if( Item::items[i] == NULL)
			{
				Item::items[i] = new TileItem(i - 256);
				Tile::tiles[i]->init();
			}

            bool propagate = false;
            if (i > 0 && Tile::tiles[i]->getRenderShape() == Tile::SHAPE_STAIRS) propagate = true;
            if (i > 0 && dynamic_cast<HalfSlabTile *>(Tile::tiles[i]) != NULL)
			{
                propagate = true;
            }
            if (i == Tile::farmland_Id) propagate = true;
			if (Tile::transculent[i])
			{
				propagate = true;
			}
			if (Tile::lightBlock[i] == 0)
			{
				propagate = true;
			}
            Tile::propagate[i] = propagate;
		}
	}
	Tile::transculent[0] = true;

	Stats::buildItemStats();

	// */
}

// 4J - added for common ctor code
void Tile::_init(int id, Material *material, bool isSolidRender)
{
	destroySpeed = 0.0f;
	explosionResistance = 0.0f;
	isInventoryItem = true;
	collectStatistics = true;

	// 4J Stu - Removed these in favour of TLS versions
	//xx0 = yy0 = zz0 = xx1 = yy1 = zz1 = 0;

	soundType = Tile::SOUND_NORMAL;
	gravity = 1.0f;
	friction = 0.6f;
	_isTicking = false;
	_isEntityTile = false;

	/*	4J - TODO
	if (Tile.tiles[id] != null)
	{
	throw new IllegalArgumentException("Slot " + id + " is already occupied by " + Tile.tiles[id] + " when adding " + this);
	}
	*/
	this->material = material;
	Tile::tiles[id] = this;
	this->id = id;
	updateDefaultShape();
	// 4J - note these used to call isSolidRender(), but that always calls Tile::isSolidRender in C++ so have added as a parameter that can be varied from
	// derived ctors
	solid[id] = isSolidRender;
	lightBlock[id] = isSolidRender ? 255 : 0;
	transculent[id] = !material->blocksLight();
	mipmapEnable[id] = true;	// 4J added
	m_textureName = L"";
}

Tile::Tile(int id, Material *material, bool isSolidRender)
{
	_init(id,material, isSolidRender);
	m_iMaterial=Item::eMaterial_undefined;
	m_iBaseItemType=Item::eBaseItemType_undefined;
	icon = NULL;
}

Tile *Tile::sendTileData(unsigned char importantMask/*=15*/)
{
	Tile::_sendTileData[id] = importantMask;		// 4J - changed was bool, now bitfield to indicate which bits are important to be sent. Default behaviour with this method is all 4 bits
	return this;
}

void Tile::init()
{
}



// 4J-PB - adding so we can class different items together for the new crafting menu
// so pickaxe_stone would get tagged with pickaxe and stone
Tile *Tile::setBaseItemTypeAndMaterial(int iType,int iMaterial)
{
	this->m_iBaseItemType = iType;
	this->m_iMaterial = iMaterial;
	return this;
}

int Tile::getBaseItemType()
{
	return this->m_iBaseItemType;
}

int Tile::getMaterial()
{
	return this->m_iMaterial;
}


Tile *Tile::setSoundType(const SoundType *soundType)
{
	this->soundType = soundType;
	return this;
}

Tile *Tile::setLightBlock(int i)
{
	lightBlock[id] = i;
	return this;
}

Tile *Tile::setLightEmission(float f)
{
	Tile::lightEmission[id] = (int) (Level::MAX_BRIGHTNESS * f);
	return this;
}

Tile *Tile::setExplodeable(float explosionResistance)
{
	this->explosionResistance = explosionResistance * 3;
	return this;
}

bool Tile::isSolidBlockingTile(int t)
{
	Tile *tile = Tile::tiles[t];
	if (tile == NULL) return false;
	return tile->material->isSolidBlocking() && tile->isCubeShaped();
}

bool Tile::isCubeShaped()
{
	return true;
}

bool Tile::isPathfindable(LevelSource *level, int x, int y, int z)
{
	return !material->blocksMotion();
}

int Tile::getRenderShape()
{
	return SHAPE_BLOCK;
}

Tile *Tile::setDestroyTime(float destroySpeed)
{
	this->destroySpeed = destroySpeed;
	if (explosionResistance < destroySpeed * 5) explosionResistance = destroySpeed * 5;
	return this;
}

Tile *Tile::setIndestructible()
{
	setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME);
	return this;
}

float Tile::getDestroySpeed(Level *level, int x, int y, int z)
{
	return destroySpeed;
}

Tile *Tile::setTicking(bool tick)
{
	_isTicking = tick;
	return this;
}

bool Tile::isTicking()
{
	return _isTicking;
}

bool Tile::isEntityTile()
{
	return _isEntityTile;
}

Tile *Tile::disableMipmap()
{
	mipmapEnable[id] = false;
	return this;
}

void Tile::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	tls->xx0 = x0;
	tls->yy0 = y0;
	tls->zz0 = z0;
	tls->xx1 = x1;
	tls->yy1 = y1;
	tls->zz1 = z1;
	tls->tileId = this->id;
	
	//this->xx0 = x0;
	//this->yy0 = y0;
	//this->zz0 = z0;
	//this->xx1 = x1;
	//this->yy1 = y1;
	//this->zz1 = z1;
}

float Tile::getBrightness(LevelSource *level, int x, int y, int z)
{
	// Lighting fix brought forward from ~1.5 here - used to use the lightEmission level for this tile rather than getting the for the passed in x/y/z coords
	return level->getBrightness(x, y, z, Tile::lightEmission[level->getTile(x,y,z)]);
}

// 4J - brought forward from 1.8.2
int Tile::getLightColor(LevelSource *level, int x, int y, int z, int tileId/*=-1*/)
{
	// Lighting fix brought forward from ~1.5 here - used to use the lightEmission level for this tile rather than getting the for the passed in x/y/z coords
	if( tileId == -1 )
	{
	    return level->getLightColor(x, y, z, Tile::lightEmission[level->getTile(x,y,z)], -1);
	}
	else
	{
		return level->getLightColor(x, y, z, Tile::lightEmission[tileId], tileId);
	}
}

bool Tile::isFaceVisible(Level *level, int x, int y, int z, int f)
{
	if (f == 0) y--;
	if (f == 1) y++;
	if (f == 2) z--;
	if (f == 3) z++;
	if (f == 4) x--;
	if (f == 5) x++;
	return !level->isSolidRenderTile(x, y, z);
}

bool Tile::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	if (face == 0 && tls->yy0 > 0) return true;
	if (face == 1 && tls->yy1 < 1) return true;
	if (face == 2 && tls->zz0 > 0) return true;
	if (face == 3 && tls->zz1 < 1) return true;
	if (face == 4 && tls->xx0 > 0) return true;
	if (face == 5 && tls->xx1 < 1) return true;
	return (!level->isSolidRenderTile(x, y, z));
}

// AP - added this function so we can generate the faceFlags for a block in a single fast function
int Tile::getFaceFlags(LevelSource *level, int x, int y, int z)
{
	int faceFlags = 0;

	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();

	if( tls->yy0 > 0 || (!level->isSolidRenderTile(x, y - 1, z))) faceFlags |= 0x01;
	if( tls->yy1 < 1 || (!level->isSolidRenderTile(x, y + 1, z))) faceFlags |= 0x02;
	if( tls->zz0 > 0 || (!level->isSolidRenderTile(x, y, z - 1))) faceFlags |= 0x04;
	if( tls->zz1 < 1 || (!level->isSolidRenderTile(x, y, z + 1))) faceFlags |= 0x08;
	if( tls->xx0 > 0 || (!level->isSolidRenderTile(x - 1, y, z))) faceFlags |= 0x10;
	if( tls->xx1 < 1 || (!level->isSolidRenderTile(x + 1, y, z))) faceFlags |= 0x20;

	return faceFlags;
}

bool Tile::isSolidFace(LevelSource *level, int x, int y, int z, int face)
{
	return (level->getMaterial(x, y, z)->isSolid());
}

Icon *Tile::getTexture(LevelSource *level, int x, int y, int z, int face)
{
	// 4J - addition here to make rendering big blocks of leaves more efficient. Normally leaves never consider themselves as solid, so
	// blocks of leaves will have all sides of each block completely visible. Changing to consider as solid if this block is surrounded by
	// other leaves (or solid things). This is paired with another change in  Level::isSolidRenderTile/Region::isSolidRenderTile which makes things solid
	// code-wise (ie for determining visible sides of neighbouring blocks). This change just makes the texture a solid one (tex + 1) which
	// we already have in the texture map for doing non-fancy graphics. Note: this tile-specific code is here rather than making some new virtual
	// method in the tiles, for the sake of efficiency - I don't imagine we'll be doing much more of this sort of thing

	int tileId = level->getTile(x, y, z);
	int tileData = level->getData(x, y, z);

	if( tileId == Tile::leaves_Id )
	{
		bool opaque = true;

		int axo[6] = { 1,-1, 0, 0, 0, 0};
		int ayo[6] = { 0, 0, 1,-1, 0, 0};
		int azo[6] = { 0, 0, 0, 0, 1,-1};
		for( int i = 0; (i < 6) && opaque; i++ )
		{
			int t = level->getTile(x + axo[i], y + ayo[i] , z + azo[i]);
			if( ( t != Tile::leaves_Id ) && ( ( Tile::tiles[t] == NULL ) || !Tile::tiles[t]->isSolidRender() ) )
			{
				opaque = false;
			}
		}

		Icon *icon = NULL;
		if(opaque)
		{
			Tile::leaves->setFancy(false);
			icon = getTexture(face, tileData);
			Tile::leaves->setFancy(true);
		}
		else
		{
			icon = getTexture(face, tileData);
		}
		return icon;
	}
	return getTexture(face, tileData);
}

Icon *Tile::getTexture(int face, int data)
{
	return icon;
}

Icon *Tile::getTexture(int face)
{
	return getTexture(face, 0);
}

AABB *Tile::getTileAABB(Level *level, int x, int y, int z)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1);
}

void Tile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source) 
{
	AABB *aabb = getAABB(level, x, y, z);
	if (aabb != NULL && box->intersects(aabb)) boxes->push_back(aabb);
}

AABB *Tile::getAABB(Level *level, int x, int y, int z)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1);
}

bool Tile::isSolidRender(bool isServerLevel)
{
	return true;
}

bool Tile::mayPick(int data, bool liquid)
{
	return mayPick();
}

bool Tile::mayPick()
{
	return true;
}

void Tile::tick(Level *level, int x, int y, int z, Random *random)
{
}

void Tile::animateTick(Level *level, int x, int y, int z, Random *random)
{
}

void Tile::destroy(Level *level, int x, int y, int z, int data)
{
}

void Tile::neighborChanged(Level *level, int x, int y, int z, int type)
{
}

void Tile::addLights(Level *level, int x, int y, int z)
{
}

int Tile::getTickDelay()
{
	return 10;
}

void Tile::onPlace(Level *level, int x, int y, int z)
{
}

void Tile::onRemove(Level *level, int x, int y, int z, int id, int data)
{
}

int Tile::getResourceCount(Random *random)
{
	return 1;
}

int Tile::getResource(int data, Random *random, int playerBonusLevel)
{
	return id;
}

float Tile::getDestroyProgress(shared_ptr<Player> player, Level *level, int x, int y, int z)
{
	float destroySpeed = getDestroySpeed(level, x, y, z);
	if (destroySpeed < 0) return 0;
	if (!player->canDestroy(this)) return 1 / destroySpeed / 100.0f;
	return (player->getDestroySpeed(this) / destroySpeed) / 30;
}

void Tile::spawnResources(Level *level, int x, int y, int z, int data, int playerBonusLevel)
{
	spawnResources(level, x, y, z, data, 1, playerBonusLevel);
}

void Tile::spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel)
{
	if (level->isClientSide) return;
	int count = getResourceCountForLootBonus(playerBonusLevel, level->random);
	for (int i = 0; i < count; i++)
	{
		if (level->random->nextFloat() > odds) continue;
		int type = getResource(data, level->random, playerBonusLevel);
		if (type <= 0) continue;

		popResource(level, x, y, z, shared_ptr<ItemInstance>( new ItemInstance(type, 1, getSpawnResourcesAuxValue(data) ) ) );
	}
}

void Tile::popResource(Level *level, int x, int y, int z, shared_ptr<ItemInstance> itemInstance)
{
	if( level->isClientSide ) return;

	float s = 0.7f;
	double xo = level->random->nextFloat() * s + (1 - s) * 0.5;
	double yo = level->random->nextFloat() * s + (1 - s) * 0.5;
	double zo = level->random->nextFloat() * s + (1 - s) * 0.5;
	shared_ptr<ItemEntity> item = shared_ptr<ItemEntity>( new ItemEntity(level, x + xo, y + yo, z + zo, itemInstance ) );
	item->throwTime = 10;
	level->addEntity(item);
}

// Brought forward for TU7
void Tile::popExperience(Level *level, int x, int y, int z, int amount)
{
	if (!level->isClientSide)
	{
		while (amount > 0)
		{
			int newCount = ExperienceOrb::getExperienceValue(amount);
			amount -= newCount;
			level->addEntity(shared_ptr<ExperienceOrb>( new ExperienceOrb(level, x + .5, y + .5, z + .5, newCount)));
		}
	}
}

int Tile::getSpawnResourcesAuxValue(int data)
{
	return 0;
}

float Tile::getExplosionResistance(shared_ptr<Entity> source)
{
	return explosionResistance / 5.0f;
}

HitResult *Tile::clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b)
{
	updateShape(level, xt, yt, zt);

	a = a->add(-xt, -yt, -zt);
	b = b->add(-xt, -yt, -zt);

	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	Vec3 *xh0 = a->clipX(b, tls->xx0);
	Vec3 *xh1 = a->clipX(b, tls->xx1);

	Vec3 *yh0 = a->clipY(b, tls->yy0);
	Vec3 *yh1 = a->clipY(b, tls->yy1);

	Vec3 *zh0 = a->clipZ(b, tls->zz0);
	Vec3 *zh1 = a->clipZ(b, tls->zz1);

	Vec3 *closest = NULL;

	if (containsX(xh0) && (closest == NULL || a->distanceToSqr(xh0) < a->distanceToSqr(closest))) closest = xh0;
	if (containsX(xh1) && (closest == NULL || a->distanceToSqr(xh1) < a->distanceToSqr(closest))) closest = xh1;
	if (containsY(yh0) && (closest == NULL || a->distanceToSqr(yh0) < a->distanceToSqr(closest))) closest = yh0;
	if (containsY(yh1) && (closest == NULL || a->distanceToSqr(yh1) < a->distanceToSqr(closest))) closest = yh1;
	if (containsZ(zh0) && (closest == NULL || a->distanceToSqr(zh0) < a->distanceToSqr(closest))) closest = zh0;
	if (containsZ(zh1) && (closest == NULL || a->distanceToSqr(zh1) < a->distanceToSqr(closest))) closest = zh1;

	if (closest == NULL) return NULL;

	int face = -1;

	if (closest == xh0) face = Facing::WEST;
	if (closest == xh1) face = Facing::EAST;
	if (closest == yh0) face = Facing::DOWN;
	if (closest == yh1) face = Facing::UP;
	if (closest == zh0) face = Facing::NORTH;
	if (closest == zh1) face = Facing::SOUTH;

	return new HitResult(xt, yt, zt, face, closest->add(xt, yt, zt));
}

bool Tile::containsX(Vec3 *v)
{
	if( v == NULL) return false;
	
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->y >= tls->yy0 && v->y <= tls->yy1 && v->z >= tls->zz0 && v->z <= tls->zz1;
}

bool Tile::containsY(Vec3 *v)
{
	if( v == NULL) return false;

	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->x >= tls->xx0 && v->x <= tls->xx1 && v->z >= tls->zz0 && v->z <= tls->zz1;
}

bool Tile::containsZ(Vec3 *v)
{
	if( v == NULL) return false;

	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->x >= tls->xx0 && v->x <= tls->xx1 && v->y >= tls->yy0 && v->y <= tls->yy1;
}

void Tile::wasExploded(Level *level, int x, int y, int z)
{
}

int Tile::getRenderLayer()
{
	return 0;
}

bool Tile::mayPlace(Level *level, int x, int y, int z, int face)
{
	return mayPlace(level, x, y, z);
}

bool Tile::mayPlace(Level *level, int x, int y, int z)
{
	int t = level->getTile(x, y, z);
	return t == 0 || Tile::tiles[t]->material->isReplaceable();
}

// 4J-PB - Adding a TestUse for tooltip display
bool Tile::TestUse()
{
	return false;
}

bool Tile::TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
	return false;
}

bool Tile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	return false;
}

void Tile::stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
}

int Tile::getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue)
{
	return itemValue;
}

void Tile::prepareRender(Level *level, int x, int y, int z)
{
}

void Tile::attack(Level *level, int x, int y, int z, shared_ptr<Player> player)
{
}

void Tile::handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current)
{
}

void Tile::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
}

double Tile::getShapeX0()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->xx0;
}

double Tile::getShapeX1()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->xx1;
}

double Tile::getShapeY0()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->yy0;
}

double Tile::getShapeY1()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->yy1;
}

double Tile::getShapeZ0()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->zz0;
}

double Tile::getShapeZ1()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(Tile::tlsIdxShape);
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->zz1;
}

int Tile::getColor() const
{
	return 0xffffff;
}

int Tile::getColor(int auxData)
{
	return 0xffffff;
}

int Tile::getColor(LevelSource *level, int x, int y, int z)
{
	return 0xffffff;
}

int Tile::getColor(LevelSource *level, int x, int y, int z, int data)
{
	return 0xffffff;
}

bool Tile::getSignal(LevelSource *level, int x, int y, int z)
{
	return false;
}

bool Tile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return false;
}

bool Tile::isSignalSource()
{
	return false;
}

void Tile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
}

bool Tile::getDirectSignal(Level *level, int x, int y, int z, int dir)
{
	return false;
}

void Tile::updateDefaultShape()
{
	setShape(0,0,0,1,1,1);
}

void Tile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	// 4J Stu - Special case - only record a crop destroy if is fully grown
	if( id==Tile::crops_Id )
	{
		if( Tile::crops->getResource(data, NULL, 0) > 0 )
			player->awardStat(
				GenericStats::blocksMined(id),
				GenericStats::param_blocksMined(id,data,1)
				);
	}
	else if (id == Tile::potatoes_Id)
	{
		if (Tile::potatoes->getResource(data, NULL, 0) > 0)
			player->awardStat(
				GenericStats::blocksMined(id),
				GenericStats::param_blocksMined(id,data,1)
				);
	}
	else if (id == Tile::carrots_Id)
	{
		if (Tile::potatoes->getResource(data, NULL, 0) > 0)
			player->awardStat(
				GenericStats::blocksMined(id),
				GenericStats::param_blocksMined(id,data,1)
				);
	}
	else
	{
		player->awardStat(
			GenericStats::blocksMined(id),
			GenericStats::param_blocksMined(id,data,1)
			);
	}
	player->awardStat(GenericStats::totalBlocksMined(), GenericStats::param_noArgs());	// 4J : WESTY : Added for other award.
	player->causeFoodExhaustion(FoodConstants::EXHAUSTION_MINE);

	if( id == Tile::treeTrunk_Id )
		player->awardStat(GenericStats::mineWood(), GenericStats::param_noArgs());


    if (isSilkTouchable() && EnchantmentHelper::hasSilkTouch(player->inventory))
	{
        shared_ptr<ItemInstance> item = getSilkTouchItemInstance(data);
        if (item != NULL)
		{
            popResource(level, x, y, z, item);
        }
    }
	else
	{
        int playerBonusLevel = EnchantmentHelper::getDiggingLootBonus(player->inventory);
        spawnResources(level, x, y, z, data, playerBonusLevel);
    }
}

bool Tile::isSilkTouchable()
{
	return isCubeShaped() && !_isEntityTile;
}

shared_ptr<ItemInstance> Tile::getSilkTouchItemInstance(int data)
{
    int popData = 0;
    if (id >= 0 && id < Item::items.length && Item::items[id]->isStackedByData())
	{
        popData = data;
    }
    return shared_ptr<ItemInstance>(new ItemInstance(id, 1, popData));
}

int Tile::getResourceCountForLootBonus(int bonusLevel, Random *random)
{
	return getResourceCount(random);
}

bool Tile::canSurvive(Level *level, int x, int y, int z)
{
	return true;
}

void Tile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by)
{
}

void Tile::finalizePlacement(Level *level, int x, int y, int z, int data)
{
}

Tile *Tile::setDescriptionId(unsigned int id)
{
	this->descriptionId = id;
	return this;
}

wstring Tile::getName()
{
	return L"";//I18n::get(getDescriptionId() + L".name");
}

unsigned int Tile::getDescriptionId(int iData /*= -1*/)
{
	return descriptionId;
}

Tile *Tile::setUseDescriptionId(unsigned int id)
{
	this->useDescriptionId = id;
	return this;
}

unsigned int Tile::getUseDescriptionId()
{
	return useDescriptionId;
}

void Tile::triggerEvent(Level *level, int x, int y, int z, int b0, int b1)
{
}

bool Tile::isCollectStatistics()
{
	return collectStatistics;
}

Tile *Tile::setNotCollectStatistics()
{
	collectStatistics = false;
	return this;
}

int Tile::getPistonPushReaction()
{
	return material->getPushReaction();
}

// 4J - brought forward from 1.8.2
float Tile::getShadeBrightness(LevelSource *level, int x, int y, int z)
{
	return level->isSolidBlockingTile(x, y, z) ? 0.2f : 1.0f;
}

void Tile::fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance)
{
}

int Tile::cloneTileId(Level *level, int x, int y, int z)
{
	return id;
}

int Tile::cloneTileData(Level *level, int x, int y, int z)
{
	return getSpawnResourcesAuxValue(level->getData(x, y, z));
}

void Tile::playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player)
{
}

void Tile::onRemoving(Level *level, int x, int y, int z, int data)
{
}

void Tile::handleRain(Level *level, int x, int y, int z)
{
}

 void Tile::levelTimeChanged(Level *level, __int64 delta, __int64 newTime)
{
}

void Tile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(m_textureName);
}

wstring Tile::getTileItemIconName()
{
	return L"";
}

Tile *Tile::setTextureName(const wstring &name)
{
	m_textureName = name;
	return this;
}

Tile::SoundType::SoundType(eMATERIALSOUND_TYPE eMaterialSound, float volume, float pitch, int iBreakSound, int iPlaceSound)
{
	this->eMaterialSound = eMaterialSound;
	if( iBreakSound>-1 )
	{
		this->iBreakSound = iBreakSound;
	}
	else
	{
		switch(eMaterialSound)
		{
		case eMaterialSoundType_STONE:
			this->iBreakSound=eSoundType_DIG_STONE;
			break;
		case eMaterialSoundType_WOOD:
			this->iBreakSound=eSoundType_DIG_WOOD;
			break;
		case eMaterialSoundType_GRAVEL:
			this->iBreakSound=eSoundType_DIG_GRAVEL;
			break;
		case eMaterialSoundType_GRASS:
			this->iBreakSound=eSoundType_DIG_GRASS;
			break;
		case eMaterialSoundType_METAL:
			this->iBreakSound=eSoundType_DIG_STONE;
			break;
		case eMaterialSoundType_GLASS:
			this->iBreakSound=eSoundType_RANDOM_GLASS;
			break;
		case eMaterialSoundType_CLOTH:
			this->iBreakSound=eSoundType_DIG_CLOTH;
			break;
		case eMaterialSoundType_SAND:
			this->iBreakSound=eSoundType_DIG_SAND;
			break;
		case eMaterialSoundType_SNOW:
			this->iBreakSound=eSoundType_DIG_SNOW;
			break;
		case eMaterialSoundType_LADDER:
			this->iBreakSound=eSoundType_DIG_WOOD;
			break;
		default:
			app.DebugPrintf("NO BREAK SOUND!\n");
			this->iBreakSound=-1;
			break;
		}
		//this->breakSound = L"step." + this->name;
	}

	if( iPlaceSound>-1 )
	{
		this->iPlaceSound = iPlaceSound;
	}
	else
	{
		this->iPlaceSound = this->iBreakSound;
	}

	switch(eMaterialSound)
	{
	case eMaterialSoundType_STONE:
		this->iStepSound=eSoundType_STEP_STONE;
		break;
	case eMaterialSoundType_WOOD:
		this->iStepSound=eSoundType_STEP_WOOD;
		break;
	case eMaterialSoundType_GRAVEL:
		this->iStepSound=eSoundType_STEP_GRAVEL;
		break;
	case eMaterialSoundType_GRASS:
		this->iStepSound=eSoundType_STEP_GRASS;
		break;
	case eMaterialSoundType_METAL:
		this->iStepSound=eSoundType_STEP_METAL;
		break;
	case eMaterialSoundType_CLOTH:
		this->iStepSound=eSoundType_STEP_CLOTH;
		break;
	case eMaterialSoundType_SAND:
		this->iStepSound=eSoundType_STEP_SAND;
		break;
	case eMaterialSoundType_SNOW:
		this->iStepSound=eSoundType_STEP_SNOW;
		break;
	case eMaterialSoundType_LADDER:
		this->iStepSound=eSoundType_STEP_LADDER;
		break;
	default:
		app.DebugPrintf("NO STEP SOUND!\n");

		this->iStepSound=-1;
		break;

	}

	//this->stepSound = L"step." + this->name;
	this->volume = volume;
	this->pitch = pitch;
}

float Tile::SoundType::getVolume() const
{ 
	return volume; 
}
float Tile::SoundType::getPitch() const
{ 
	return pitch; 
}
//wstring getBreakSound() const { return breakSound; }
//wstring getStepSound()	const { return stepSound; }
int Tile::SoundType::getBreakSound() const
{ 
	return iBreakSound; 
}
int Tile::SoundType::getStepSound()	const
{ 
	return iStepSound; 
}
int Tile::SoundType::getPlaceSound() const
{ 
	return iPlaceSound; 
}


/*
	4J: These are necessary on the PS3.
		(and 4 and Vita).
*/
#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
const int Tile::rock_Id;
const int Tile::grass_Id;
const int Tile::dirt_Id;
const int Tile::stoneBrick_Id;
const int Tile::wood_Id;
const int Tile::sapling_Id;
const int Tile::unbreakable_Id;
const int Tile::water_Id;
const int Tile::calmWater_Id;
const int Tile::lava_Id;
const int Tile::calmLava_Id;
const int Tile::sand_Id;
const int Tile::gravel_Id;
const int Tile::goldOre_Id;
const int Tile::ironOre_Id;
const int Tile::coalOre_Id;
const int Tile::treeTrunk_Id;
const int Tile::leaves_Id;
const int Tile::sponge_Id;
const int Tile::glass_Id;
const int Tile::lapisOre_Id;
const int Tile::lapisBlock_Id;
const int Tile::dispenser_Id;
const int Tile::sandStone_Id;
const int Tile::musicBlock_Id;
const int Tile::bed_Id;
const int Tile::goldenRail_Id;
const int Tile::detectorRail_Id;
const int Tile::pistonStickyBase_Id;
const int Tile::web_Id;
const int Tile::tallgrass_Id;
const int Tile::deadBush_Id;
const int Tile::pistonBase_Id;
const int Tile::pistonExtensionPiece_Id;
const int Tile::cloth_Id;
const int Tile::pistonMovingPiece_Id;
const int Tile::flower_Id;
const int Tile::rose_Id;
const int Tile::mushroom1_Id;
const int Tile::mushroom2_Id;
const int Tile::goldBlock_Id;
const int Tile::ironBlock_Id;
const int Tile::stoneSlab_Id;
const int Tile::stoneSlabHalf_Id;
const int Tile::redBrick_Id;
const int Tile::tnt_Id;
const int Tile::bookshelf_Id;
const int Tile::mossStone_Id;
const int Tile::obsidian_Id;
const int Tile::torch_Id;
const int Tile::fire_Id;
const int Tile::mobSpawner_Id;
const int Tile::stairs_wood_Id;
const int Tile::chest_Id;
const int Tile::redStoneDust_Id;
const int Tile::diamondOre_Id;
const int Tile::diamondBlock_Id;
const int Tile::workBench_Id;
const int Tile::crops_Id;
const int Tile::farmland_Id;
const int Tile::furnace_Id;
const int Tile::furnace_lit_Id;
const int Tile::sign_Id;
const int Tile::door_wood_Id;
const int Tile::ladder_Id;
const int Tile::rail_Id;
const int Tile::stairs_stone_Id;
const int Tile::wallSign_Id;
const int Tile::lever_Id;
const int Tile::pressurePlate_stone_Id;
const int Tile::door_iron_Id;
const int Tile::pressurePlate_wood_Id;
const int Tile::redStoneOre_Id;
const int Tile::redStoneOre_lit_Id;
const int Tile::notGate_off_Id;
const int Tile::notGate_on_Id;
const int Tile::button_stone_Id;
const int Tile::topSnow_Id;
const int Tile::ice_Id;
const int Tile::snow_Id;
const int Tile::cactus_Id;
const int Tile::clay_Id;
const int Tile::reeds_Id;
const int Tile::recordPlayer_Id;
const int Tile::fence_Id;
const int Tile::pumpkin_Id;
const int Tile::hellRock_Id;
const int Tile::hellSand_Id;
const int Tile::lightGem_Id;
const int Tile::portalTile_Id;
const int Tile::litPumpkin_Id;
const int Tile::cake_Id;
const int Tile::diode_off_Id;
const int Tile::diode_on_Id;
const int Tile::aprilFoolsJoke_Id;
const int Tile::trapdoor_Id;
const int Tile::monsterStoneEgg_Id;
const int Tile::stoneBrickSmooth_Id;
const int Tile::hugeMushroom1_Id;
const int Tile::hugeMushroom2_Id;
const int Tile::ironFence_Id;
const int Tile::thinGlass_Id;
const int Tile::melon_Id;
const int Tile::pumpkinStem_Id;
const int Tile::melonStem_Id;
const int Tile::vine_Id;
const int Tile::fenceGate_Id;
const int Tile::stairs_bricks_Id;
const int Tile::stairs_stoneBrickSmooth_Id;
const int Tile::mycel_Id;
const int Tile::waterLily_Id;
const int Tile::netherBrick_Id;
const int Tile::netherFence_Id;
const int Tile::stairs_netherBricks_Id;
const int Tile::netherStalk_Id;
const int Tile::enchantTable_Id;
const int Tile::brewingStand_Id;
const int Tile::cauldron_Id;
const int Tile::endPortalTile_Id;
const int Tile::endPortalFrameTile_Id;
const int Tile::whiteStone_Id;
const int Tile::dragonEgg_Id;
const int Tile::redstoneLight_Id;
const int Tile::redstoneLight_lit_Id;
const int Tile::woodSlab_Id;
const int Tile::woodSlabHalf_Id;
const int Tile::cocoa_Id;
const int Tile::stairs_sandstone_Id;
const int Tile::stairs_sprucewood_Id;
const int Tile::stairs_birchwood_Id;
const int Tile::stairs_junglewood_Id;
const int Tile::emeraldOre_Id;
const int Tile::enderChest_Id;
const int Tile::tripWireSource_Id;
const int Tile::tripWire_Id;
const int Tile::emeraldBlock_Id;
const int Tile::cobbleWall_Id;
const int Tile::flowerPot_Id;
const int Tile::carrots_Id;
const int Tile::potatoes_Id;
const int Tile::anvil_Id;
const int Tile::button_wood_Id;
const int Tile::skull_Id;
const int Tile::netherQuartz_Id;
const int Tile::quartzBlock_Id;
const int Tile::stairs_quartz_Id;
const int Tile::woolCarpet_Id;
#endif
