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
#include "net.minecraft.world.level.redstone.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "Tile.h"

wstring Tile::TILE_DESCRIPTION_PREFIX = L"Tile.";

const float Tile::INDESTRUCTIBLE_DESTROY_TIME = -1.0f;

Tile::SoundType *Tile::SOUND_NORMAL = nullptr;
Tile::SoundType *Tile::SOUND_WOOD = nullptr;
Tile::SoundType *Tile::SOUND_GRAVEL = nullptr;
Tile::SoundType *Tile::SOUND_GRASS = nullptr;
Tile::SoundType *Tile::SOUND_STONE = nullptr;
Tile::SoundType *Tile::SOUND_METAL = nullptr;
Tile::SoundType *Tile::SOUND_GLASS = nullptr;
Tile::SoundType *Tile::SOUND_CLOTH = nullptr;
Tile::SoundType *Tile::SOUND_SAND = nullptr;
Tile::SoundType *Tile::SOUND_SNOW = nullptr;
Tile::SoundType *Tile::SOUND_LADDER = nullptr;
Tile::SoundType *Tile::SOUND_ANVIL = nullptr;

bool Tile::solid[TILE_NUM_COUNT];
int Tile::lightBlock[TILE_NUM_COUNT];
bool Tile::transculent[TILE_NUM_COUNT];
int Tile::lightEmission[TILE_NUM_COUNT];
unsigned char Tile::_sendTileData[TILE_NUM_COUNT];		// 4J changed - was bool, now bitfield to indicate which bits are important to be sent
bool Tile::mipmapEnable[TILE_NUM_COUNT];
bool Tile::propagate[TILE_NUM_COUNT];

Tile **Tile::tiles = nullptr;

Tile *Tile::stone = nullptr;
GrassTile *Tile::grass = nullptr;
Tile *Tile::dirt = nullptr;
Tile *Tile::cobblestone = nullptr;
Tile *Tile::wood = nullptr;
Tile *Tile::sapling = nullptr;
Tile *Tile::unbreakable = nullptr;
LiquidTile *Tile::water = nullptr;
Tile *Tile::calmWater = nullptr;
LiquidTile *Tile::lava = nullptr;
Tile *Tile::calmLava = nullptr;
Tile *Tile::sand = nullptr;
Tile *Tile::gravel = nullptr;
Tile *Tile::goldOre = nullptr;
Tile *Tile::ironOre = nullptr;
Tile *Tile::coalOre = nullptr;
Tile *Tile::treeTrunk = nullptr;
LeafTile *Tile::leaves = nullptr;
Tile *Tile::sponge = nullptr;
Tile *Tile::glass = nullptr;
Tile *Tile::lapisOre = nullptr;
Tile *Tile::lapisBlock = nullptr;
Tile *Tile::dispenser = nullptr;
Tile *Tile::sandStone = nullptr;
Tile *Tile::noteblock = nullptr;
Tile *Tile::bed = nullptr;
Tile *Tile::goldenRail = nullptr;
Tile *Tile::detectorRail = nullptr;
PistonBaseTile *Tile::pistonStickyBase = nullptr;
Tile *Tile::web = nullptr;
TallGrass *Tile::tallgrass = nullptr;
DeadBushTile *Tile::deadBush = nullptr;
PistonBaseTile *Tile::pistonBase = nullptr;
PistonExtensionTile *Tile::pistonExtension = nullptr;
Tile *Tile::wool = nullptr;
PistonMovingPiece *Tile::pistonMovingPiece = nullptr;
Bush *Tile::flower = nullptr;
Bush *Tile::rose = nullptr;
Bush *Tile::mushroom_brown = nullptr;
Bush *Tile::mushroom_red = nullptr;
Tile *Tile::goldBlock = nullptr;
Tile *Tile::ironBlock = nullptr;
HalfSlabTile *Tile::stoneSlab = nullptr;
HalfSlabTile *Tile::stoneSlabHalf = nullptr;
Tile *Tile::redBrick = nullptr;
Tile *Tile::tnt = nullptr;
Tile *Tile::bookshelf = nullptr;
Tile *Tile::mossyCobblestone = nullptr;
Tile *Tile::obsidian = nullptr;
Tile *Tile::torch = nullptr;
FireTile *Tile::fire = nullptr;
Tile *Tile::mobSpawner = nullptr;
Tile *Tile::stairs_wood = nullptr;
ChestTile *Tile::chest = nullptr;
RedStoneDustTile *Tile::redStoneDust = nullptr;
Tile *Tile::diamondOre = nullptr;
Tile *Tile::diamondBlock = nullptr;
Tile *Tile::workBench = nullptr;
Tile *Tile::wheat = nullptr;
Tile *Tile::farmland = nullptr;
Tile *Tile::furnace = nullptr;
Tile *Tile::furnace_lit = nullptr;
Tile *Tile::sign = nullptr;
Tile *Tile::door_wood = nullptr;
Tile *Tile::ladder = nullptr;
Tile *Tile::rail = nullptr;
Tile *Tile::stairs_stone = nullptr;
Tile *Tile::wallSign = nullptr;
Tile *Tile::lever = nullptr;
Tile *Tile::pressurePlate_stone = nullptr;
Tile *Tile::door_iron = nullptr;
Tile *Tile::pressurePlate_wood = nullptr;
Tile *Tile::redStoneOre = nullptr;
Tile *Tile::redStoneOre_lit = nullptr;
Tile *Tile::redstoneTorch_off = nullptr;
Tile *Tile::redstoneTorch_on = nullptr;
Tile *Tile::button = nullptr;
Tile *Tile::topSnow = nullptr;
Tile *Tile::ice = nullptr;
Tile *Tile::snow = nullptr;
Tile *Tile::cactus = nullptr;
Tile *Tile::clay = nullptr;
Tile *Tile::reeds = nullptr;
Tile *Tile::jukebox = nullptr;
Tile *Tile::fence = nullptr;
Tile *Tile::pumpkin = nullptr;
Tile *Tile::netherRack = nullptr;
Tile *Tile::soulsand = nullptr;
Tile *Tile::glowstone = nullptr;
PortalTile *Tile::portalTile = nullptr;
Tile *Tile::litPumpkin = nullptr;
Tile *Tile::cake = nullptr;
RepeaterTile *Tile::diode_off = nullptr;
RepeaterTile *Tile::diode_on = nullptr;
Tile *Tile::stained_glass = nullptr;
Tile *Tile::trapdoor = nullptr;

Tile *Tile::monsterStoneEgg = nullptr;
Tile *Tile::stoneBrick = nullptr;
Tile *Tile::hugeMushroom_brown = nullptr;
Tile *Tile::hugeMushroom_red = nullptr;
Tile *Tile::ironFence = nullptr;
Tile *Tile::thinGlass = nullptr;
Tile *Tile::melon = nullptr;
Tile *Tile::pumpkinStem = nullptr;
Tile *Tile::melonStem = nullptr;
Tile *Tile::vine = nullptr;
Tile *Tile::fenceGate = nullptr;
Tile *Tile::stairs_bricks = nullptr;
Tile *Tile::stairs_stoneBrickSmooth = nullptr;

MycelTile *Tile::mycel = nullptr;
Tile *Tile::waterLily = nullptr;
Tile *Tile::netherBrick = nullptr;
Tile *Tile::netherFence = nullptr;
Tile *Tile::stairs_netherBricks = nullptr;
Tile *Tile::netherStalk = nullptr;
Tile *Tile::enchantTable = nullptr;
Tile *Tile::brewingStand = nullptr;
CauldronTile *Tile::cauldron = nullptr;
Tile *Tile::endPortalTile = nullptr;
Tile *Tile::endPortalFrameTile = nullptr;
Tile *Tile::endStone = nullptr;
Tile *Tile::dragonEgg = nullptr;
Tile *Tile::redstoneLight = nullptr;
Tile *Tile::redstoneLight_lit = nullptr;

// TU9
Tile *Tile::stairs_sandstone = nullptr;
Tile *Tile::woodStairsDark = nullptr;
Tile *Tile::woodStairsBirch = nullptr;
Tile *Tile::woodStairsJungle = nullptr;
Tile *Tile::commandBlock = nullptr;
BeaconTile *Tile::beacon = nullptr;
Tile *Tile::button_wood = nullptr;
HalfSlabTile *Tile::woodSlab = nullptr;
HalfSlabTile *Tile::woodSlabHalf = nullptr;

Tile *Tile::emeraldOre = nullptr;
Tile *Tile::enderChest = nullptr;
TripWireSourceTile *Tile::tripWireSource = nullptr;
Tile *Tile::tripWire = nullptr;
Tile *Tile::emeraldBlock = nullptr;


Tile *Tile::cocoa = nullptr;
Tile *Tile::skull = nullptr;

Tile *Tile::cobbleWall = nullptr;
Tile *Tile::flowerPot = nullptr;
Tile *Tile::carrots = nullptr;
Tile *Tile::potatoes = nullptr;
Tile *Tile::anvil = nullptr;
Tile *Tile::chest_trap = nullptr;
Tile *Tile::weightedPlate_light = nullptr;
Tile *Tile::weightedPlate_heavy = nullptr;
ComparatorTile *Tile::comparator_off = nullptr;
ComparatorTile *Tile::comparator_on = nullptr;

DaylightDetectorTile *Tile::daylightDetector = nullptr;
Tile *Tile::redstoneBlock = nullptr;

Tile *Tile::netherQuartz = nullptr;
HopperTile *Tile::hopper = nullptr;
Tile *Tile::quartzBlock = nullptr;
Tile *Tile::stairs_quartz = nullptr;
Tile *Tile::activatorRail = nullptr;
Tile *Tile::dropper = nullptr;
Tile *Tile::clayHardened_colored = nullptr;
Tile *Tile::stained_glass_pane = nullptr;

Tile *Tile::hayBlock = nullptr;
Tile *Tile::woolCarpet = nullptr;
Tile *Tile::clayHardened = nullptr;
Tile *Tile::coalBlock = nullptr;

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
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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

	Tile::stone = (new StoneTile(1))										->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"stone")->setDescriptionId(IDS_TILE_STONE)->setUseDescriptionId(IDS_DESC_STONE);
	Tile::grass = static_cast<GrassTile *>((new GrassTile(2))->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"grass")->setDescriptionId(IDS_TILE_GRASS)->setUseDescriptionId(IDS_DESC_GRASS));
	Tile::dirt = (new DirtTile(3))											->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_GRAVEL)->setIconName(L"dirt")->setDescriptionId(IDS_TILE_DIRT)->setUseDescriptionId(IDS_DESC_DIRT);
	Tile::cobblestone = (new Tile(4, Material::stone))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"cobblestone")->setDescriptionId(IDS_TILE_STONE_BRICK)->setUseDescriptionId(IDS_DESC_STONE_BRICK);
	Tile::wood = (new WoodTile(5))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_structwoodstuff,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"planks")->setDescriptionId(IDS_TILE_OAKWOOD_PLANKS)->sendTileData()->setUseDescriptionId(IDS_DESC_WOODENPLANKS);
	Tile::sapling = (new Sapling(6))										->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"sapling")->setDescriptionId(IDS_TILE_SAPLING)->sendTileData()->setUseDescriptionId(IDS_DESC_SAPLING)->disableMipmap();
	Tile::unbreakable = (new Tile(7, Material::stone))						->setIndestructible()->setExplodeable(6000000)->setSoundType(Tile::SOUND_STONE)->setIconName(L"bedrock")->setDescriptionId(IDS_TILE_BEDROCK)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_BEDROCK);
	Tile::water = static_cast<LiquidTile *>((new LiquidTileDynamic(8, Material::water))->setDestroyTime(100.0f)->setLightBlock(3)->setIconName(L"water_flow")->setDescriptionId(IDS_TILE_WATER)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_WATER));
	Tile::calmWater = (new LiquidTileStatic(9, Material::water))			->setDestroyTime(100.0f)->setLightBlock(3)->setIconName(L"water_still")->setDescriptionId(IDS_TILE_WATER)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_WATER);
	Tile::lava = static_cast<LiquidTile *>((new LiquidTileDynamic(10, Material::lava))->setDestroyTime(00.0f)->setLightEmission(1.0f)->setLightBlock(255)->setIconName(L"lava_flow")->setDescriptionId(IDS_TILE_LAVA)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_LAVA));

	Tile::calmLava = (new LiquidTileStatic(11, Material::lava))	->setDestroyTime(100.0f)->setLightEmission(1.0f)->setLightBlock(255)->setIconName(L"lava_still")->setDescriptionId(IDS_TILE_LAVA)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_LAVA);
	Tile::sand = (new HeavyTile(12))							->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_SAND)->setIconName(L"sand")->setDescriptionId(IDS_TILE_SAND)->setUseDescriptionId(IDS_DESC_SAND);
	Tile::gravel = (new GravelTile(13))							->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setIconName(L"gravel")->setDescriptionId(IDS_TILE_GRAVEL)->setUseDescriptionId(IDS_DESC_GRAVEL);
	Tile::goldOre = (new OreTile(14))							->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"gold_ore")->setDescriptionId(IDS_TILE_ORE_GOLD)->setUseDescriptionId(IDS_DESC_ORE_GOLD);
	Tile::ironOre = (new OreTile(15))							->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"iron_ore")->setDescriptionId(IDS_TILE_ORE_IRON)->setUseDescriptionId(IDS_DESC_ORE_IRON);
	Tile::coalOre = (new OreTile(16))							->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"coal_ore")->setDescriptionId(IDS_TILE_ORE_COAL)->setUseDescriptionId(IDS_DESC_ORE_COAL);
	Tile::treeTrunk = (new TreeTile(17))->setDestroyTime(2.0f)	->setSoundType(Tile::SOUND_WOOD)->setIconName(L"log")->setDescriptionId(IDS_TILE_LOG)->sendTileData()->setUseDescriptionId(IDS_DESC_LOG);
	// 4J - for leaves, have specified that only the data bits that encode the type of leaf are important to be sent
	Tile::leaves = static_cast<LeafTile *>((new LeafTile(18))->setDestroyTime(0.2f)->setLightBlock(1)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"leaves")->setDescriptionId(IDS_TILE_LEAVES)->sendTileData(LeafTile::LEAF_TYPE_MASK)->setUseDescriptionId(IDS_DESC_LEAVES));
	Tile::sponge = (new Sponge(19))								->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"sponge")->setDescriptionId(IDS_TILE_SPONGE)->setUseDescriptionId(IDS_DESC_SPONGE);
	Tile::glass = (new GlassTile(20, Material::glass, false))	->setDestroyTime(0.3f)->setSoundType(Tile::SOUND_GLASS)->setIconName(L"glass")->setDescriptionId(IDS_TILE_GLASS)->setUseDescriptionId(IDS_DESC_GLASS);

	Tile::lapisOre = (new OreTile(21))											->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"lapis_ore")->setDescriptionId(IDS_TILE_ORE_LAPIS)->setUseDescriptionId(IDS_DESC_ORE_LAPIS);
	Tile::lapisBlock = (new Tile(22, Material::stone))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_lapis)->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"lapis_block")->setDescriptionId(IDS_TILE_BLOCK_LAPIS)->setUseDescriptionId(IDS_DESC_BLOCK_LAPIS);
	Tile::dispenser = (new DispenserTile(23))									->setBaseItemTypeAndMaterial(Item::eBaseItemType_redstoneContainer,	Item::eMaterial_undefined)->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setIconName(L"dispenser")->setDescriptionId(IDS_TILE_DISPENSER)->sendTileData()->setUseDescriptionId(IDS_DESC_DISPENSER);
	Tile::sandStone = (new SandStoneTile(24))									->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_sand)->setSoundType(Tile::SOUND_STONE)->setDestroyTime(0.8f)->sendTileData()->setIconName(L"sandstone")->setDescriptionId(IDS_TILE_SANDSTONE)->setUseDescriptionId(IDS_DESC_SANDSTONE)->sendTileData();
	Tile::noteblock = (new NoteBlockTile(25))									->setDestroyTime(0.8f)->setIconName(L"noteblock")->setDescriptionId(IDS_TILE_MUSIC_BLOCK)->sendTileData()->setUseDescriptionId(IDS_DESC_NOTEBLOCK);
	Tile::bed = (new BedTile(26))												->setDestroyTime(0.2f)->setIconName(L"bed")->setDescriptionId(IDS_TILE_BED)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_BED);
	Tile::goldenRail = (new PoweredRailTile(27))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_gold)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setIconName(L"rail_golden")->setDescriptionId(IDS_TILE_GOLDEN_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_POWEREDRAIL)->disableMipmap();
	Tile::detectorRail = (new DetectorRailTile(28))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_detector)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setIconName(L"rail_detector")->setDescriptionId(IDS_TILE_DETECTOR_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_DETECTORRAIL)->disableMipmap();
	Tile::pistonStickyBase = static_cast<PistonBaseTile *>((new PistonBaseTile(29, true))->setBaseItemTypeAndMaterial(Item::eBaseItemType_piston, Item::eMaterial_stickypiston)->setIconName(L"pistonStickyBase")->setDescriptionId(IDS_TILE_PISTON_STICK_BASE)->setUseDescriptionId(IDS_DESC_STICKY_PISTON)->sendTileData());
	Tile::web = (new WebTile(30))												->setLightBlock(1)->setDestroyTime(4.0f)->setIconName(L"web")->setDescriptionId(IDS_TILE_WEB)->setUseDescriptionId(IDS_DESC_WEB);

	Tile::tallgrass = static_cast<TallGrass *>((new TallGrass(31))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"tallgrass")->setDescriptionId(IDS_TILE_TALL_GRASS)->setUseDescriptionId(IDS_DESC_TALL_GRASS)->disableMipmap());
	Tile::deadBush = static_cast<DeadBushTile *>((new DeadBushTile(32))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"deadbush")->setDescriptionId(IDS_TILE_DEAD_BUSH)->setUseDescriptionId(IDS_DESC_DEAD_BUSH)->disableMipmap());
	Tile::pistonBase = static_cast<PistonBaseTile *>((new PistonBaseTile(33, false))->setBaseItemTypeAndMaterial(Item::eBaseItemType_piston, Item::eMaterial_piston)->setIconName(L"pistonBase")->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(IDS_DESC_PISTON)->sendTileData());
	Tile::pistonExtension = static_cast<PistonExtensionTile *>((new PistonExtensionTile(34))->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(-1)->sendTileData());
	Tile::wool = (new ColoredTile(35, Material::cloth))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_cloth,	Item::eMaterial_cloth)->setDestroyTime(0.8f)->setSoundType(Tile::SOUND_CLOTH)->setIconName(L"wool_colored")->setDescriptionId(IDS_TILE_CLOTH)->sendTileData()->setUseDescriptionId(IDS_DESC_WOOL);
	Tile::pistonMovingPiece = static_cast<PistonMovingPiece *>((new PistonMovingPiece(36))->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(-1));
	Tile::flower = static_cast<Bush *>((new Bush(37))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"flower_dandelion")->setDescriptionId(IDS_TILE_FLOWER)->setUseDescriptionId(IDS_DESC_FLOWER)->disableMipmap());
	Tile::rose = static_cast<Bush *>((new Bush(38))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"flower_rose")->setDescriptionId(IDS_TILE_ROSE)->setUseDescriptionId(IDS_DESC_FLOWER)->disableMipmap());
	Tile::mushroom_brown = static_cast<Bush *>((new Mushroom(39))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setLightEmission(2 / 16.0f)->setIconName(L"mushroom_brown")->setDescriptionId(IDS_TILE_MUSHROOM)->setUseDescriptionId(IDS_DESC_MUSHROOM)->disableMipmap());
	Tile::mushroom_red = static_cast<Bush *>((new Mushroom(40))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"mushroom_red")->setDescriptionId(IDS_TILE_MUSHROOM)->setUseDescriptionId(IDS_DESC_MUSHROOM)->disableMipmap());

	Tile::goldBlock = (new MetalTile(41))														->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_gold)->setDestroyTime(3.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setIconName(L"gold_block")->setDescriptionId(IDS_TILE_BLOCK_GOLD)->setUseDescriptionId(IDS_DESC_BLOCK_GOLD);
	Tile::ironBlock = (new MetalTile(42))														->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_iron)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setIconName(L"iron_block")->setDescriptionId(IDS_TILE_BLOCK_IRON)->setUseDescriptionId(IDS_DESC_BLOCK_IRON);
	Tile::stoneSlab = static_cast<HalfSlabTile *>((new StoneSlabTile(Tile::stoneSlab_Id, true))->setBaseItemTypeAndMaterial(Item::eBaseItemType_slab, Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_SLAB));
	Tile::stoneSlabHalf = static_cast<HalfSlabTile *>((new StoneSlabTile(Tile::stoneSlabHalf_Id, false))->setBaseItemTypeAndMaterial(Item::eBaseItemType_halfslab, Item::eMaterial_stone)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_HALFSLAB));
	Tile::redBrick = (new Tile(45, Material::stone))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_brick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"brick")->setDescriptionId(IDS_TILE_BRICK)->setUseDescriptionId(IDS_DESC_BRICK);
	Tile::tnt = (new TntTile(46))																->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"tnt")->setDescriptionId(IDS_TILE_TNT)->setUseDescriptionId(IDS_DESC_TNT);
	Tile::bookshelf = (new BookshelfTile(47))													->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_bookshelf)->setDestroyTime(1.5f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"bookshelf")->setDescriptionId(IDS_TILE_BOOKSHELF)->setUseDescriptionId(IDS_DESC_BOOKSHELF);
	Tile::mossyCobblestone = (new Tile(48, Material::stone))									->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"cobblestone_mossy")->setDescriptionId(IDS_TILE_STONE_MOSS)->setUseDescriptionId(IDS_DESC_MOSS_STONE);
	Tile::obsidian = (new ObsidianTile(49))														->setDestroyTime(50.0f)->setExplodeable(2000)->setSoundType(Tile::SOUND_STONE)->setIconName(L"obsidian")->setDescriptionId(IDS_TILE_OBSIDIAN)->setUseDescriptionId(IDS_DESC_OBSIDIAN);
	Tile::torch = (new TorchTile(50))															->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_wood)->setDestroyTime(0.0f)->setLightEmission(15 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"torch_on")->setDescriptionId(IDS_TILE_TORCH)->setUseDescriptionId(IDS_DESC_TORCH)->disableMipmap();

	Tile::fire = static_cast<FireTile *>(((new FireTile(51))->setDestroyTime(0.0f)->setLightEmission(1.0f)->setSoundType(Tile::SOUND_WOOD))->setIconName(L"fire")->setDescriptionId(IDS_TILE_FIRE)->setNotCollectStatistics()->setUseDescriptionId(-1));
	Tile::mobSpawner = (new MobSpawnerTile(52))								->setDestroyTime(5.0f)->setSoundType(Tile::SOUND_METAL)->setIconName(L"mob_spawner")->setDescriptionId(IDS_TILE_MOB_SPAWNER)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_MOB_SPAWNER);
	Tile::stairs_wood =	 (new StairTile(53, Tile::wood,0))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_wood)		->setIconName(L"stairsWood")->setDescriptionId(IDS_TILE_STAIRS_WOOD)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::chest = static_cast<ChestTile *>((new ChestTile(54, ChestTile::TYPE_BASIC))->setBaseItemTypeAndMaterial(Item::eBaseItemType_chest, Item::eMaterial_wood)->setDestroyTime(2.5f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"chest")->setDescriptionId(IDS_TILE_CHEST)->sendTileData()->setUseDescriptionId(IDS_DESC_CHEST));
	Tile::redStoneDust = static_cast<RedStoneDustTile *>((new RedStoneDustTile(55))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_NORMAL)->setIconName(L"redstone_dust")->setDescriptionId(IDS_TILE_REDSTONE_DUST)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONE_DUST));
	Tile::diamondOre = (new OreTile(56))									->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"diamond_ore")->setDescriptionId(IDS_TILE_ORE_DIAMOND)->setUseDescriptionId(IDS_DESC_ORE_DIAMOND);
	Tile::diamondBlock = (new MetalTile(57))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_diamond)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_METAL)->setIconName(L"diamond_block")->setDescriptionId(IDS_TILE_BLOCK_DIAMOND)->setUseDescriptionId(IDS_DESC_BLOCK_DIAMOND);
	Tile::workBench = (new WorkbenchTile(58))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_wood)->setDestroyTime(2.5f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"crafting_table")->setDescriptionId(IDS_TILE_WORKBENCH)->setUseDescriptionId(IDS_DESC_CRAFTINGTABLE);
	Tile::wheat = (new CropTile(59))										->setIconName(L"wheat")->setDescriptionId(IDS_TILE_CROPS)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_CROPS)->disableMipmap();
	Tile::farmland = (new FarmTile(60))										->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setIconName(L"farmland")->setDescriptionId(IDS_TILE_FARMLAND)->setUseDescriptionId(IDS_DESC_FARMLAND)->sendTileData();

	Tile::furnace = (new FurnaceTile(61, false))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_stone)->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setIconName(L"furnace")->setDescriptionId(IDS_TILE_FURNACE)->sendTileData()->setUseDescriptionId(IDS_DESC_FURNACE);
	Tile::furnace_lit = (new FurnaceTile(62, true))							->setDestroyTime(3.5f)->setSoundType(Tile::SOUND_STONE)->setLightEmission(14 / 16.0f)->setIconName(L"furnace")->setDescriptionId(IDS_TILE_FURNACE)->sendTileData()->setUseDescriptionId(IDS_DESC_FURNACE);
	Tile::sign = (new SignTile(63, eTYPE_SIGNTILEENTITY, true))				->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"sign")->setDescriptionId(IDS_TILE_SIGN)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_SIGN);
	Tile::door_wood = (new DoorTile(64, Material::wood))					->setDestroyTime(3.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"door_wood")->setDescriptionId(IDS_TILE_DOOR_WOOD)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Tile::ladder = (new LadderTile(65))										->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_LADDER)->setIconName(L"ladder")->setDescriptionId(IDS_TILE_LADDER)->sendTileData()->setUseDescriptionId(IDS_DESC_LADDER)->disableMipmap();
	Tile::rail = (new RailTile(66))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_rail,	Item::eMaterial_iron)->setDestroyTime(0.7f)->setSoundType(Tile::SOUND_METAL)->setIconName(L"rail_normal")->setDescriptionId(IDS_TILE_RAIL)->sendTileData()->setUseDescriptionId(IDS_DESC_RAIL)->disableMipmap();
	Tile::stairs_stone =(new StairTile(67, Tile::cobblestone,0))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_stone)		->setIconName(L"stairsStone")->setDescriptionId(IDS_TILE_STAIRS_STONE)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::wallSign = (new SignTile(68, eTYPE_SIGNTILEENTITY, false))		->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"sign")->setDescriptionId(IDS_TILE_SIGN)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_SIGN);
	Tile::lever = (new LeverTile(69))										->setBaseItemTypeAndMaterial(Item::eBaseItemType_lever,	Item::eMaterial_wood)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"lever")->setDescriptionId(IDS_TILE_LEVER)->sendTileData()->setUseDescriptionId(IDS_DESC_LEVER);
	Tile::pressurePlate_stone = (Tile *)(new PressurePlateTile(70, L"stone", Material::stone, PressurePlateTile::mobs))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_stone)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_STONE)->setDescriptionId(IDS_TILE_PRESSURE_PLATE)->sendTileData()->setUseDescriptionId(IDS_DESC_PRESSUREPLATE);

	Tile::door_iron = (new DoorTile(71, Material::metal))		->setDestroyTime(5.0f)->setSoundType(Tile::SOUND_METAL)->setIconName(L"door_iron")->setDescriptionId(IDS_TILE_DOOR_IRON)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_DOOR_IRON);
	Tile::pressurePlate_wood = (new PressurePlateTile(72, L"planks_oak", Material::wood, PressurePlateTile::everything))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_wood)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setDescriptionId(IDS_TILE_PRESSURE_PLATE)->sendTileData()->setUseDescriptionId(IDS_DESC_PRESSUREPLATE);
	Tile::redStoneOre = (new RedStoneOreTile(73,false))			->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"redstone_ore")->setDescriptionId(IDS_TILE_ORE_REDSTONE)->sendTileData()->setUseDescriptionId(IDS_DESC_ORE_REDSTONE);
	Tile::redStoneOre_lit = (new RedStoneOreTile(74, true))		->setLightEmission(10 / 16.0f)->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_STONE)->setIconName(L"redstone_ore")->setDescriptionId(IDS_TILE_ORE_REDSTONE)->sendTileData()->setUseDescriptionId(IDS_DESC_ORE_REDSTONE);
	Tile::redstoneTorch_off = (new NotGateTile(75, false))		->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"redstone_torch_off")->setDescriptionId(IDS_TILE_NOT_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONETORCH)->disableMipmap();
	Tile::redstoneTorch_on = (new NotGateTile(76, true))		->setDestroyTime(0.0f)->setLightEmission(8 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"redstone_torch_on")->setDescriptionId(IDS_TILE_NOT_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONETORCH)->disableMipmap();
	Tile::button = (new StoneButtonTile(77))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_button,	Item::eMaterial_stone)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_STONE)->setIconName(L"button")->setDescriptionId(IDS_TILE_BUTTON)->sendTileData()->setUseDescriptionId(IDS_DESC_BUTTON);
	Tile::topSnow = (new TopSnowTile(78))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_snow)->setDestroyTime(0.1f)->setSoundType(Tile::SOUND_SNOW)->setIconName(L"snow")->setDescriptionId(IDS_TILE_SNOW)->setUseDescriptionId(IDS_DESC_TOP_SNOW)->sendTileData()->setLightBlock(0);
	Tile::ice = (new IceTile(79))								->setDestroyTime(0.5f)->setLightBlock(3)->setSoundType(Tile::SOUND_GLASS)->setIconName(L"ice")->setDescriptionId(IDS_TILE_ICE)->setUseDescriptionId(IDS_DESC_ICE);
	Tile::snow = (new SnowTile(80))								->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_snow)->setDestroyTime(0.2f)->setSoundType(Tile::SOUND_CLOTH)->setIconName(L"snow")->setDescriptionId(IDS_TILE_SNOW)->setUseDescriptionId(IDS_DESC_SNOW);

	Tile::cactus = (new CactusTile(81))									->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_CLOTH)->setIconName(L"cactus")->setDescriptionId(IDS_TILE_CACTUS)->setUseDescriptionId(IDS_DESC_CACTUS)->disableMipmap();
	Tile::clay = (new ClayTile(82))										->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_clay)->setDestroyTime(0.6f)->setSoundType(Tile::SOUND_GRAVEL)->setIconName(L"clay")->setDescriptionId(IDS_TILE_CLAY)->setUseDescriptionId(IDS_DESC_CLAY_TILE);
	Tile::reeds = (new ReedTile(83))									->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_GRASS)->setIconName(L"reeds")->setDescriptionId(IDS_TILE_REEDS)->setNotCollectStatistics()->setUseDescriptionId(IDS_DESC_REEDS)->disableMipmap();
	Tile::jukebox = (new JukeboxTile(84))								->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(Tile::SOUND_STONE)->setIconName(L"jukebox")->setDescriptionId(IDS_TILE_JUKEBOX)->sendTileData()->setUseDescriptionId(IDS_DESC_JUKEBOX);
	Tile::fence = (new FenceTile(85, L"planks_oak", Material::wood))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,	Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(Tile::SOUND_WOOD)->setDescriptionId(IDS_TILE_FENCE)->setUseDescriptionId(IDS_DESC_FENCE);
	Tile::pumpkin = (new PumpkinTile(86, false))						->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"pumpkin")->setDescriptionId(IDS_TILE_PUMPKIN)->sendTileData()->setUseDescriptionId(IDS_DESC_PUMPKIN);
	Tile::netherRack = (new NetherrackTile(87))							->setDestroyTime(0.4f)->setSoundType(Tile::SOUND_STONE)->setIconName(L"netherrack")->setDescriptionId(IDS_TILE_HELL_ROCK)->setUseDescriptionId(IDS_DESC_HELL_ROCK);
	Tile::soulsand = (new SoulSandTile(88))								->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_SAND)->setIconName(L"soul_sand")->setDescriptionId(IDS_TILE_HELL_SAND)->setUseDescriptionId(IDS_DESC_HELL_SAND);
	Tile::glowstone = (new Glowstonetile(89, Material::glass))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_glowstone)->setDestroyTime(0.3f)->setSoundType(Tile::SOUND_GLASS)->setLightEmission(1.0f)->setIconName(L"glowstone")->setDescriptionId(IDS_TILE_LIGHT_GEM)->setUseDescriptionId(IDS_DESC_GLOWSTONE);
	Tile::portalTile = static_cast<PortalTile *>(((new PortalTile(90))->setDestroyTime(-1)->setSoundType(Tile::SOUND_GLASS)->setLightEmission(0.75f))->setIconName(L"portal")->setDescriptionId(IDS_TILE_PORTAL)->setUseDescriptionId(IDS_DESC_PORTAL));

	Tile::litPumpkin = (new PumpkinTile(91, true))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_torch,	Item::eMaterial_pumpkin)->setDestroyTime(1.0f)->setSoundType(Tile::SOUND_WOOD)->setLightEmission(1.0f)->setIconName(L"pumpkin")->setDescriptionId(IDS_TILE_LIT_PUMPKIN)->sendTileData()->setUseDescriptionId(IDS_DESC_JACKOLANTERN);
	Tile::cake = (new CakeTile(92))									->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_CLOTH)->setIconName(L"cake")->setDescriptionId(IDS_TILE_CAKE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_CAKE);
	Tile::diode_off = static_cast<RepeaterTile *>((new RepeaterTile(93, false))->setDestroyTime(0.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"repeater_off")->setDescriptionId(IDS_ITEM_DIODE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER)->disableMipmap());
	Tile::diode_on = static_cast<RepeaterTile *>((new RepeaterTile(94, true))->setDestroyTime(0.0f)->setLightEmission(10 / 16.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"repeater_on")->setDescriptionId(IDS_ITEM_DIODE)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER)->disableMipmap());
	Tile::stained_glass = (new StainedGlassBlock(95, Material::glass))->setBaseItemTypeAndMaterial(Item::eBaseItemType_glass,	Item::eMaterial_glass)->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setIconName(L"glass")->setDescriptionId(IDS_TILE_STAINED_GLASS)->setUseDescriptionId(IDS_DESC_STAINED_GLASS);
	Tile::trapdoor = (new TrapDoorTile(96, Material::wood))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_door,	Item::eMaterial_trap)->setDestroyTime(3.0f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"trapdoor")->setDescriptionId(IDS_TILE_TRAPDOOR)->setNotCollectStatistics()->sendTileData()->setUseDescriptionId(IDS_DESC_TRAPDOOR);
	Tile::monsterStoneEgg = (new StoneMonsterTile(97))				->setDestroyTime(0.75f)->setIconName(L"monsterStoneEgg")->setDescriptionId(IDS_TILE_STONE_SILVERFISH)->setUseDescriptionId(IDS_DESC_STONE_SILVERFISH);
	Tile::stoneBrick = (new SmoothStoneBrickTile(98))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_stoneSmooth)->setDestroyTime(1.5f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setIconName(L"stonebrick")->setDescriptionId(IDS_TILE_STONE_BRICK_SMOOTH)->setUseDescriptionId(IDS_DESC_STONE_BRICK_SMOOTH);
	Tile::hugeMushroom_brown = (new HugeMushroomTile(99, Material::wood, HugeMushroomTile::MUSHROOM_TYPE_BROWN))		->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setIconName(L"mushroom_block")->setDescriptionId(IDS_TILE_HUGE_MUSHROOM_1)->setUseDescriptionId(IDS_DESC_MUSHROOM)->sendTileData();
	Tile::hugeMushroom_red = (new HugeMushroomTile(100, Material::wood, HugeMushroomTile::MUSHROOM_TYPE_RED))			->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setIconName(L"mushroom_block")->setDescriptionId(IDS_TILE_HUGE_MUSHROOM_2)->setUseDescriptionId(IDS_DESC_MUSHROOM)->sendTileData();


	Tile::ironFence = (new ThinFenceTile(101, L"iron_bars", L"iron_bars", Material::metal, true))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,		Item::eMaterial_iron)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setDescriptionId(IDS_TILE_IRON_FENCE)->setUseDescriptionId(IDS_DESC_IRON_FENCE);
	Tile::thinGlass = (new ThinFenceTile(102, L"glass", L"glass_pane_top", Material::glass, false))	->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setDescriptionId(IDS_TILE_THIN_GLASS)->setUseDescriptionId(IDS_DESC_THIN_GLASS);
	Tile::melon = (new MelonTile(103))											->setDestroyTime(1.0f)->setSoundType(SOUND_WOOD)->setIconName(L"melon")->setDescriptionId(IDS_TILE_MELON)->setUseDescriptionId(IDS_DESC_MELON_BLOCK);
	Tile::pumpkinStem = (new StemTile(104, Tile::pumpkin))						->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setIconName(L"pumpkin_stem")->setDescriptionId(IDS_TILE_PUMPKIN_STEM)->sendTileData();
	Tile::melonStem = (new StemTile(105, Tile::melon))							->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setIconName(L"melon_stem")->setDescriptionId(IDS_TILE_MELON_STEM)->sendTileData();
	Tile::vine = (new VineTile(106))->setDestroyTime(0.2f)						->setSoundType(SOUND_GRASS)->setIconName(L"vine")->setDescriptionId(IDS_TILE_VINE)->setUseDescriptionId(IDS_DESC_VINE)->sendTileData();
	Tile::fenceGate = (new FenceGateTile(107))									->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setIconName(L"fenceGate")->setDescriptionId(IDS_TILE_FENCE_GATE)->sendTileData()->setUseDescriptionId(IDS_DESC_FENCE_GATE);
	Tile::stairs_bricks = (new StairTile(108, Tile::redBrick,0))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_brick)		->setIconName(L"stairsBrick")->setDescriptionId(IDS_TILE_STAIRS_BRICKS)				->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::stairs_stoneBrickSmooth = (new StairTile(109, Tile::stoneBrick,0))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_stoneSmooth)->setIconName(L"stairsStoneBrickSmooth")->setDescriptionId(IDS_TILE_STAIRS_STONE_BRICKS_SMOOTH)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::mycel = static_cast<MycelTile *>((new MycelTile(110))->setDestroyTime(0.6f)->setSoundType(SOUND_GRASS)->setIconName(L"mycelium")->setDescriptionId(IDS_TILE_MYCEL)->setUseDescriptionId(IDS_DESC_MYCEL));

	Tile::waterLily = (new WaterlilyTile(111))									->setDestroyTime(0.0f)->setSoundType(SOUND_GRASS)->setIconName(L"waterlily")->setDescriptionId(IDS_TILE_WATERLILY)->setUseDescriptionId(IDS_DESC_WATERLILY);
	Tile::netherBrick = (new Tile(112, Material::stone))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_netherbrick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setIconName(L"nether_brick")->setDescriptionId(IDS_TILE_NETHERBRICK)->setUseDescriptionId(IDS_DESC_NETHERBRICK);
	Tile::netherFence = (new FenceTile(113, L"nether_brick", Material::stone))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,		Item::eMaterial_netherbrick)->setDestroyTime(2.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setDescriptionId(IDS_TILE_NETHERFENCE)->setUseDescriptionId(IDS_DESC_NETHERFENCE);
	Tile::stairs_netherBricks = (new StairTile(114, Tile::netherBrick,0))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_netherbrick)->setIconName(L"stairsNetherBrick")->setDescriptionId(IDS_TILE_STAIRS_NETHERBRICK)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::netherStalk = (new NetherWartTile(115))								->setIconName(L"nether_wart")->setDescriptionId(IDS_TILE_NETHERSTALK)->sendTileData()->setUseDescriptionId(IDS_DESC_NETHERSTALK);
	Tile::enchantTable = (new EnchantmentTableTile(116))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_magic)->setDestroyTime(5.0f)->setExplodeable(2000)->setIconName(L"enchanting_table")->setDescriptionId(IDS_TILE_ENCHANTMENTTABLE)->setUseDescriptionId(IDS_DESC_ENCHANTMENTTABLE);
	Tile::brewingStand = (new BrewingStandTile(117))							->setBaseItemTypeAndMaterial(Item::eBaseItemType_device,	Item::eMaterial_blaze)->setDestroyTime(0.5f)->setLightEmission(2 / 16.0f)->setIconName(L"brewing_stand")->setDescriptionId(IDS_TILE_BREWINGSTAND)->sendTileData()->setUseDescriptionId(IDS_DESC_BREWING_STAND);
	Tile::cauldron = static_cast<CauldronTile *>((new CauldronTile(118))->setDestroyTime(2.0f)->setIconName(L"cauldron")->setDescriptionId(IDS_TILE_CAULDRON)->sendTileData()->setUseDescriptionId(IDS_DESC_CAULDRON));
	Tile::endPortalTile = (new TheEndPortal(119, Material::portal))				->setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME)->setExplodeable(6000000)->setDescriptionId(IDS_TILE_END_PORTAL)->setUseDescriptionId(IDS_DESC_END_PORTAL);
	Tile::endPortalFrameTile = (new TheEndPortalFrameTile(120))					->setSoundType(SOUND_GLASS)->setLightEmission(2 / 16.0f)->setDestroyTime(INDESTRUCTIBLE_DESTROY_TIME)->setIconName(L"endframe")->setDescriptionId(IDS_TILE_ENDPORTALFRAME)->sendTileData()->setExplodeable(6000000)->setUseDescriptionId(IDS_DESC_ENDPORTALFRAME);

	Tile::endStone = (new Tile(121, Material::stone))										->setDestroyTime(3.0f)->setExplodeable(15)->setSoundType(SOUND_STONE)->setIconName(L"end_stone")->setDescriptionId(IDS_TILE_WHITESTONE)->setUseDescriptionId(IDS_DESC_WHITESTONE);
	Tile::dragonEgg = (new EggTile(122))													->setDestroyTime(3.0f)->setExplodeable(15)->setSoundType(SOUND_STONE)->setLightEmission(2.0f / 16.0f)->setIconName(L"dragon_egg")->setDescriptionId(IDS_TILE_DRAGONEGG)->setUseDescriptionId(IDS_DESC_DRAGONEGG);
	Tile::redstoneLight = (new RedlightTile(123, false))									->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setIconName(L"redstone_lamp_off")->setDescriptionId(IDS_TILE_REDSTONE_LIGHT)->setUseDescriptionId(IDS_DESC_REDSTONE_LIGHT);
	Tile::redstoneLight_lit = (new RedlightTile(124, true))									->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setIconName(L"redstone_lamp_on")->setDescriptionId(IDS_TILE_REDSTONE_LIGHT)->setUseDescriptionId(IDS_DESC_REDSTONE_LIGHT);
	Tile::woodSlab = static_cast<HalfSlabTile *>((new WoodSlabTile(Tile::woodSlab_Id, true))->setBaseItemTypeAndMaterial(Item::eBaseItemType_slab, Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setIconName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB));
	Tile::woodSlabHalf = static_cast<HalfSlabTile *>((new WoodSlabTile(Tile::woodSlabHalf_Id, false))->setBaseItemTypeAndMaterial(Item::eBaseItemType_halfslab, Item::eMaterial_wood)->setDestroyTime(2.0f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setIconName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB));
	Tile::cocoa = (new CocoaTile(127))														->setDestroyTime(0.2f)->setExplodeable(5)->setSoundType(SOUND_WOOD)->setIconName(L"cocoa")->sendTileData()->setDescriptionId(IDS_TILE_COCOA)->setUseDescriptionId(IDS_DESC_COCOA);
	Tile::stairs_sandstone = (new StairTile(128, Tile::sandStone,0))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_sand)	->setIconName(L"stairsSandstone")->setDescriptionId(IDS_TILE_STAIRS_SANDSTONE)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::emeraldOre = (new OreTile(129))													->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setIconName(L"emerald_ore")->setDescriptionId(IDS_TILE_EMERALDORE)->setUseDescriptionId(IDS_DESC_EMERALDORE);
	Tile::enderChest = (new EnderChestTile(130))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_chest,	Item::eMaterial_ender)->setDestroyTime(22.5f)->setExplodeable(1000)->setSoundType(SOUND_STONE)->setIconName(L"enderChest")->sendTileData()->setLightEmission(.5f)->setDescriptionId(IDS_TILE_ENDERCHEST)->setUseDescriptionId(IDS_DESC_ENDERCHEST);


	Tile::tripWireSource =	static_cast<TripWireSourceTile *>((new TripWireSourceTile(131))->setBaseItemTypeAndMaterial(Item::eBaseItemType_lever, Item::eMaterial_undefined)->setIconName(L"trip_wire_source")->sendTileData()->setDescriptionId(IDS_TILE_TRIPWIRE_SOURCE)->setUseDescriptionId(IDS_DESC_TRIPWIRE_SOURCE));
	Tile::tripWire =		(new TripWireTile(132))									->setIconName(L"trip_wire")->sendTileData()->setDescriptionId(IDS_TILE_TRIPWIRE)->setUseDescriptionId(IDS_DESC_TRIPWIRE);
	Tile::emeraldBlock =	(new MetalTile(133))									->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_emerald)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setIconName(L"emerald_block")->setDescriptionId(IDS_TILE_EMERALDBLOCK)->setUseDescriptionId(IDS_DESC_EMERALDBLOCK);
	Tile::woodStairsDark =	(new StairTile(134, Tile::wood, TreeTile::DARK_TRUNK))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_sprucewood)->setIconName(L"stairsWoodSpruce")->setDescriptionId(IDS_TILE_STAIRS_SPRUCEWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::woodStairsBirch =	(new StairTile(135, Tile::wood, TreeTile::BIRCH_TRUNK))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_birchwood)->setIconName(L"stairsWoodBirch")->setDescriptionId(IDS_TILE_STAIRS_BIRCHWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::woodStairsJungle =(new StairTile(136, Tile::wood, TreeTile::JUNGLE_TRUNK))->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_junglewood)->setIconName(L"stairsWoodJungle")->setDescriptionId(IDS_TILE_STAIRS_JUNGLEWOOD)	->sendTileData()->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::commandBlock = (new CommandBlock(137))									->setIndestructible()->setExplodeable(6000000)->setIconName(L"command_block")->setDescriptionId(IDS_TILE_COMMAND_BLOCK)->setUseDescriptionId(IDS_DESC_COMMAND_BLOCK);
	Tile::beacon = static_cast<BeaconTile *>((new BeaconTile(138))->setLightEmission(1.0f)->setIconName(L"beacon")->setDescriptionId(IDS_TILE_BEACON)->setUseDescriptionId(IDS_DESC_BEACON));
	Tile::cobbleWall =		(new WallTile(139, Tile::stoneBrick))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_fence,	Item::eMaterial_stone)->setIconName(L"cobbleWall")->setDescriptionId(IDS_TILE_COBBLESTONE_WALL)->setUseDescriptionId(IDS_DESC_COBBLESTONE_WALL);
	Tile::flowerPot =		(new FlowerPotTile(140))								->setDestroyTime(0.0f)->setSoundType(SOUND_NORMAL)->setIconName(L"flower_pot")->setDescriptionId(IDS_TILE_FLOWERPOT)->setUseDescriptionId(IDS_DESC_FLOWERPOT);

	Tile::carrots =			(new CarrotTile(141))					->setIconName(L"carrots")->setDescriptionId(IDS_TILE_CARROTS)->setUseDescriptionId(IDS_DESC_CARROTS)->disableMipmap();
	Tile::potatoes =		(new PotatoTile(142))					->setIconName(L"potatoes")->setDescriptionId(IDS_TILE_POTATOES)->setUseDescriptionId(IDS_DESC_POTATO)->disableMipmap();
	Tile::button_wood =		(new WoodButtonTile(143))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_button,	Item::eMaterial_wood)->setDestroyTime(0.5f)->setSoundType(Tile::SOUND_WOOD)->setIconName(L"button")->setDescriptionId(IDS_TILE_BUTTON)->sendTileData()->setUseDescriptionId(IDS_DESC_BUTTON);
	Tile::skull =			(new SkullTile(144))					->setDestroyTime(1.0f)->setSoundType(SOUND_STONE)->setIconName(L"skull")->setDescriptionId(IDS_TILE_SKULL)->setUseDescriptionId(IDS_DESC_SKULL);
	Tile::anvil =			(new AnvilTile(145))					->setBaseItemTypeAndMaterial(Item::eBaseItemType_device, Item::eMaterial_iron)->setDestroyTime(5.0f)->setSoundType(SOUND_ANVIL)->setExplodeable(2000)->setIconName(L"anvil")->sendTileData()->setDescriptionId(IDS_TILE_ANVIL)->setUseDescriptionId(IDS_DESC_ANVIL);
	Tile::chest_trap = (new ChestTile(146, ChestTile::TYPE_TRAP))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_chest,	Item::eMaterial_trap)->setDestroyTime(2.5f)->setSoundType(SOUND_WOOD)->setDescriptionId(IDS_TILE_CHEST_TRAP)->setUseDescriptionId(IDS_DESC_CHEST_TRAP);
	Tile::weightedPlate_light = (new WeightedPressurePlateTile(147, L"gold_block", Material::metal, Redstone::SIGNAL_MAX))		->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_gold)->setDestroyTime(0.5f)->setSoundType(SOUND_WOOD)->setDescriptionId(IDS_TILE_WEIGHTED_PLATE_LIGHT)->setUseDescriptionId(IDS_DESC_WEIGHTED_PLATE_LIGHT);
	Tile::weightedPlate_heavy = (new WeightedPressurePlateTile(148, L"iron_block", Material::metal, Redstone::SIGNAL_MAX * 10))->setBaseItemTypeAndMaterial(Item::eBaseItemType_pressureplate,	Item::eMaterial_iron)->setDestroyTime(0.5f)->setSoundType(SOUND_WOOD)->setDescriptionId(IDS_TILE_WEIGHTED_PLATE_HEAVY)->setUseDescriptionId(IDS_DESC_WEIGHTED_PLATE_HEAVY);
	Tile::comparator_off = static_cast<ComparatorTile *>((new ComparatorTile(149, false))->setDestroyTime(0.0f)->setSoundType(SOUND_WOOD)->setIconName(L"comparator_off")->setDescriptionId(IDS_TILE_COMPARATOR)->setUseDescriptionId(IDS_DESC_COMPARATOR));
	Tile::comparator_on = static_cast<ComparatorTile *>((new ComparatorTile(150, true))->setDestroyTime(0.0f)->setLightEmission(10 / 16.0f)->setSoundType(SOUND_WOOD)->setIconName(L"comparator_on")->setDescriptionId(IDS_TILE_COMPARATOR)->setUseDescriptionId(IDS_DESC_COMPARATOR));

	Tile::daylightDetector = static_cast<DaylightDetectorTile *>((new DaylightDetectorTile(151))->setDestroyTime(0.2f)->setSoundType(SOUND_WOOD)->setIconName(L"daylight_detector")->setDescriptionId(IDS_TILE_DAYLIGHT_DETECTOR)->setUseDescriptionId(IDS_DESC_DAYLIGHT_DETECTOR));
	Tile::redstoneBlock = (new PoweredMetalTile(152))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_redstone)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_METAL)->setIconName(L"redstone_block")->setDescriptionId(IDS_TILE_REDSTONE_BLOCK)->setUseDescriptionId(IDS_DESC_REDSTONE_BLOCK);
	Tile::netherQuartz =	(new OreTile(153))								->setDestroyTime(3.0f)->setExplodeable(5)->setSoundType(SOUND_STONE)->setIconName(L"quartz_ore")->setDescriptionId(IDS_TILE_NETHER_QUARTZ)->setUseDescriptionId(IDS_DESC_NETHER_QUARTZ_ORE);
	Tile::hopper = static_cast<HopperTile *>((new HopperTile(154))->setBaseItemTypeAndMaterial(Item::eBaseItemType_redstoneContainer, Item::eMaterial_undefined)->setDestroyTime(3.0f)->setExplodeable(8)->setSoundType(SOUND_WOOD)->setIconName(L"hopper")->setDescriptionId(IDS_TILE_HOPPER)->setUseDescriptionId(IDS_DESC_HOPPER));
	Tile::quartzBlock =		(new QuartzBlockTile(155))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_structblock,	Item::eMaterial_quartz)->setSoundType(SOUND_STONE)->setDestroyTime(0.8f)->setIconName(L"quartz_block")->setDescriptionId(IDS_TILE_QUARTZ_BLOCK)->setUseDescriptionId(IDS_DESC_QUARTZ_BLOCK);
	Tile::stairs_quartz =	(new StairTile(156, Tile::quartzBlock, QuartzBlockTile::TYPE_DEFAULT))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_stairs,	Item::eMaterial_quartz)->setIconName(L"stairsQuartz")->setDescriptionId(IDS_TILE_STAIRS_QUARTZ)->setUseDescriptionId(IDS_DESC_STAIRS);
	Tile::activatorRail = (new PoweredRailTile(157))						->setDestroyTime(0.7f)->setSoundType(SOUND_METAL)->setIconName(L"rail_activator")->setDescriptionId(IDS_TILE_ACTIVATOR_RAIL)->setUseDescriptionId(IDS_DESC_ACTIVATOR_RAIL);
	Tile::dropper = (new DropperTile(158))									->setBaseItemTypeAndMaterial(Item::eBaseItemType_redstoneContainer,	Item::eMaterial_undefined)->setDestroyTime(3.5f)->setSoundType(SOUND_STONE)->setIconName(L"dropper")->setDescriptionId(IDS_TILE_DROPPER)->setUseDescriptionId(IDS_DESC_DROPPER);
	Tile::clayHardened_colored = (new ColoredTile(159, Material::stone))	->setBaseItemTypeAndMaterial(Item::eBaseItemType_clay,	Item::eMaterial_clay)->setDestroyTime(1.25f)->setExplodeable(7)->setSoundType(SOUND_STONE)->setIconName(L"hardened_clay_stained")->setDescriptionId(IDS_TILE_STAINED_CLAY)->setUseDescriptionId(IDS_DESC_STAINED_CLAY);
	Tile::stained_glass_pane = (new StainedGlassPaneBlock(160))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_glass,	Item::eMaterial_glass)->setDestroyTime(0.3f)->setSoundType(SOUND_GLASS)->setIconName(L"glass")->setDescriptionId(IDS_TILE_STAINED_GLASS_PANE)->setUseDescriptionId(IDS_DESC_STAINED_GLASS_PANE);

	Tile::hayBlock = (new HayBlockTile(170))						->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_wheat)->setDestroyTime(0.5f)->setSoundType(SOUND_GRASS)->setIconName(L"hay_block")->setDescriptionId(IDS_TILE_HAY)->setUseDescriptionId(IDS_DESC_HAY);
	Tile::woolCarpet =		(new WoolCarpetTile(171))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_carpet,	Item::eMaterial_cloth)->setDestroyTime(0.1f)->setSoundType(SOUND_CLOTH)->setIconName(L"woolCarpet")->setLightBlock(0)->setDescriptionId(IDS_TILE_CARPET)->setUseDescriptionId(IDS_DESC_CARPET);
	Tile::clayHardened = (new Tile(172, Material::stone))			->setBaseItemTypeAndMaterial(Item::eBaseItemType_clay,	Item::eMaterial_clay)->setDestroyTime(1.25f)->setExplodeable(7)->setSoundType(SOUND_STONE)->setIconName(L"hardened_clay")->setDescriptionId(IDS_TILE_HARDENED_CLAY)->setUseDescriptionId(IDS_DESC_HARDENED_CLAY);
	Tile::coalBlock = (new Tile(173, Material::stone))				->setBaseItemTypeAndMaterial(Item::eBaseItemType_block,	Item::eMaterial_coal)->setDestroyTime(5.0f)->setExplodeable(10)->setSoundType(SOUND_STONE)->setIconName(L"coal_block")->setDescriptionId(IDS_TILE_COAL)->setUseDescriptionId(IDS_DESC_COAL_BLOCK);


	// Special cases for certain items since they can have different icons
	Item::items[wool_Id]				= ( new WoolTileItem(Tile::wool_Id- 256) )->setIconName(L"cloth")->setDescriptionId(IDS_TILE_CLOTH)->setUseDescriptionId(IDS_DESC_WOOL);
	Item::items[clayHardened_colored_Id]= ( new WoolTileItem(Tile::clayHardened_colored_Id - 256))->setIconName(L"clayHardenedStained")->setDescriptionId(IDS_TILE_STAINED_CLAY)->setUseDescriptionId(IDS_DESC_STAINED_CLAY);
	Item::items[stained_glass_Id]		= ( new WoolTileItem(Tile::stained_glass_Id - 256))->setIconName(L"stainedGlass")->setDescriptionId(IDS_TILE_STAINED_GLASS)->setUseDescriptionId(IDS_DESC_STAINED_GLASS);
	Item::items[stained_glass_pane_Id]	= ( new WoolTileItem(Tile::stained_glass_pane_Id - 256))->setIconName(L"stainedGlassPane")->setDescriptionId(IDS_TILE_STAINED_GLASS_PANE)->setUseDescriptionId(IDS_DESC_STAINED_GLASS_PANE);
	Item::items[woolCarpet_Id]			= ( new WoolTileItem(Tile::woolCarpet_Id - 256))->setIconName(L"woolCarpet")->setDescriptionId(IDS_TILE_CARPET)->setUseDescriptionId(IDS_DESC_CARPET);
	Item::items[treeTrunk_Id]			= ( new MultiTextureTileItem(Tile::treeTrunk_Id - 256, treeTrunk, (int *)TreeTile::TREE_NAMES, 4) )->setIconName(L"log")->setDescriptionId(IDS_TILE_LOG)->setUseDescriptionId(IDS_DESC_LOG);
	Item::items[wood_Id]				= ( new MultiTextureTileItem(Tile::wood_Id - 256, Tile::wood, (int *)WoodTile::WOOD_NAMES, 4, IDS_TILE_PLANKS))->setIconName(L"wood")->setDescriptionId(IDS_TILE_OAKWOOD_PLANKS)->setUseDescriptionId(IDS_DESC_LOG); //  <- TODO
	Item::items[monsterStoneEgg_Id]		= ( new MultiTextureTileItem(Tile::monsterStoneEgg_Id - 256, monsterStoneEgg, (int *)StoneMonsterTile::STONE_MONSTER_NAMES, 3))->setIconName(L"monsterStoneEgg")->setDescriptionId(IDS_TILE_STONE_SILVERFISH)->setUseDescriptionId(IDS_DESC_STONE_SILVERFISH); // 4J - Brought forward from post-1.2 to fix stacking problem
	Item::items[stoneBrick_Id]			= ( new MultiTextureTileItem(Tile::stoneBrick_Id - 256, stoneBrick,(int *)SmoothStoneBrickTile::SMOOTH_STONE_BRICK_NAMES, 4))->setIconName(L"stonebricksmooth")->setDescriptionId(IDS_TILE_STONE_BRICK_SMOOTH);
	Item::items[sandStone_Id]			= ( new MultiTextureTileItem(sandStone_Id - 256, sandStone, SandStoneTile::SANDSTONE_NAMES, SandStoneTile::SANDSTONE_BLOCK_NAMES) )->setIconName(L"sandStone")->setDescriptionId(IDS_TILE_SANDSTONE)->setUseDescriptionId(IDS_DESC_SANDSTONE);
	Item::items[quartzBlock_Id]			= ( new MultiTextureTileItem(quartzBlock_Id - 256, quartzBlock, QuartzBlockTile::BLOCK_NAMES, QuartzBlockTile::QUARTZ_BLOCK_NAMES) )->setIconName(L"quartzBlock")->setDescriptionId(IDS_TILE_QUARTZ_BLOCK)->setUseDescriptionId(IDS_DESC_QUARTZ_BLOCK);
	Item::items[stoneSlabHalf_Id]		= ( new StoneSlabTileItem(Tile::stoneSlabHalf_Id - 256, Tile::stoneSlabHalf,	Tile::stoneSlab, false) )->setIconName(L"stoneSlab")->setDescriptionId(IDS_TILE_STONESLAB)->setUseDescriptionId(IDS_DESC_HALFSLAB);
	Item::items[stoneSlab_Id]			= ( new StoneSlabTileItem(Tile::stoneSlab_Id - 256,		Tile::stoneSlabHalf,	Tile::stoneSlab, true))->setIconName(L"stoneSlab")->setDescriptionId(IDS_DESC_STONESLAB)->setUseDescriptionId(IDS_DESC_SLAB);
	Item::items[woodSlabHalf_Id]		= ( new StoneSlabTileItem(Tile::woodSlabHalf_Id - 256,	Tile::woodSlabHalf,		Tile::woodSlab, false))->setIconName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Item::items[woodSlab_Id]			= ( new StoneSlabTileItem(Tile::woodSlab_Id - 256,		Tile::woodSlabHalf,		Tile::woodSlab, true))->setIconName(L"woodSlab")->setDescriptionId(IDS_DESC_WOODSLAB)->setUseDescriptionId(IDS_DESC_WOODSLAB);
	Item::items[sapling_Id]				= ( new MultiTextureTileItem(Tile::sapling_Id - 256, Tile::sapling, Sapling::SAPLING_NAMES, 4) )->setIconName(L"sapling")->setDescriptionId(IDS_TILE_SAPLING)->setUseDescriptionId(IDS_DESC_SAPLING);
	Item::items[leaves_Id]				= ( new LeafTileItem(Tile::leaves_Id - 256) )->setIconName(L"leaves")->setDescriptionId(IDS_TILE_LEAVES)->setUseDescriptionId(IDS_DESC_LEAVES);
	Item::items[vine_Id]				= ( new ColoredTileItem(Tile::vine_Id - 256, false))->setDescriptionId(IDS_TILE_VINE)->setUseDescriptionId(IDS_DESC_VINE);
	int idsData[3] = {IDS_TILE_SHRUB, IDS_TILE_TALL_GRASS, IDS_TILE_FERN};
	intArray ids = intArray(idsData, 3);
	Item::items[tallgrass_Id]			= static_cast<ColoredTileItem *>((new ColoredTileItem(Tile::tallgrass_Id - 256, true))->setDescriptionId(IDS_TILE_TALL_GRASS))->setDescriptionPostfixes(ids);
	Item::items[topSnow_Id]				= ( new SnowItem(topSnow_Id - 256, topSnow) );
	Item::items[waterLily_Id]			= ( new WaterLilyTileItem(Tile::waterLily_Id - 256));
	Item::items[pistonBase_Id]			= ( new PistonTileItem(Tile::pistonBase_Id - 256) )->setDescriptionId(IDS_TILE_PISTON_BASE)->setUseDescriptionId(IDS_DESC_PISTON);
	Item::items[pistonStickyBase_Id]	= ( new PistonTileItem(Tile::pistonStickyBase_Id - 256) )->setDescriptionId(IDS_TILE_PISTON_STICK_BASE)->setUseDescriptionId(IDS_DESC_STICKY_PISTON);
	Item::items[cobbleWall_Id]			= ( new MultiTextureTileItem(cobbleWall_Id - 256, cobbleWall, (int *)WallTile::COBBLE_NAMES, 2) )->setDescriptionId(IDS_TILE_COBBLESTONE_WALL)->setUseDescriptionId(IDS_DESC_COBBLESTONE_WALL);
	Item::items[anvil_Id]				= ( new AnvilTileItem(anvil) )->setDescriptionId(IDS_TILE_ANVIL)->setUseDescriptionId(IDS_DESC_ANVIL);


	for (int i = 0; i < 256; i++)
	{
		if ( Tile::tiles[i] != nullptr )
		{
			if( Item::items[i] == nullptr)
			{
				Item::items[i] = new TileItem(i - 256);
				Tile::tiles[i]->init();
			}

			bool propagate = false;
			if (i > 0 && Tile::tiles[i]->getRenderShape() == Tile::SHAPE_STAIRS) propagate = true;
			if (i > 0 && dynamic_cast<HalfSlabTile *>(Tile::tiles[i]) != nullptr)
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
	iconName = L"";
}

Tile::Tile(int id, Material *material, bool isSolidRender)
{
	_init(id,material, isSolidRender);
	m_iMaterial=Item::eMaterial_undefined;
	m_iBaseItemType=Item::eBaseItemType_undefined;
	icon = nullptr;
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
	Tile::lightEmission[id] = static_cast<int>(Level::MAX_BRIGHTNESS * f);
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
	if (tile == nullptr) return false;
	return tile->material->isSolidBlocking() && tile->isCubeShaped() && !tile->isSignalSource();
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
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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
	return level->getBrightness(x, y, z, lightEmission[level->getTile(x,y,z)]);
}

// 4J - brought forward from 1.8.2
int Tile::getLightColor(LevelSource *level, int x, int y, int z, int tileId/*=-1*/)
{
	// Lighting fix brought forward from ~1.5 here - used to use the lightEmission level for this tile rather than getting the for the passed in x/y/z coords
	if( tileId == -1 )
	{
		return level->getLightColor(x, y, z, lightEmission[level->getTile(x,y,z)], -1);
	}
	else
	{
		return level->getLightColor(x, y, z, lightEmission[tileId], tileId);
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
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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

	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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
			if( ( t != Tile::leaves_Id ) && ( ( Tile::tiles[t] == nullptr ) || !Tile::tiles[t]->isSolidRender() ) )
			{
				opaque = false;
			}
		}

		Icon *icon = nullptr;
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
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return AABB::newTemp(x + tls->xx0, y + tls->yy0, z + tls->zz0, x + tls->xx1, y + tls->yy1, z + tls->zz1);
}

void Tile::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
	AABB *aabb = getAABB(level, x, y, z);
	if (aabb != nullptr && box->intersects(aabb)) boxes->push_back(aabb);
}

AABB *Tile::getAABB(Level *level, int x, int y, int z)
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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

int Tile::getTickDelay(Level *level)
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
	if (!player->canDestroy(this))
	{
		return player->getDestroySpeed(this, false) / destroySpeed / 100.0f;
	}
	return (player->getDestroySpeed(this, true) / destroySpeed) / 30;
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

		popResource(level, x, y, z, std::make_shared<ItemInstance>(type, 1, getSpawnResourcesAuxValue(data)));
	}
}

void Tile::popResource(Level *level, int x, int y, int z, shared_ptr<ItemInstance> itemInstance)
{
	if( level->isClientSide || !level->getGameRules()->getBoolean(GameRules::RULE_DOTILEDROPS) ) return;

	float s = 0.7f;
	double xo = level->random->nextFloat() * s + (1 - s) * 0.5;
	double yo = level->random->nextFloat() * s + (1 - s) * 0.5;
	double zo = level->random->nextFloat() * s + (1 - s) * 0.5;
	shared_ptr<ItemEntity> item = std::make_shared<ItemEntity>(level, x + xo, y + yo, z + zo, itemInstance);
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
			level->addEntity(std::make_shared<ExperienceOrb>(level, x + .5, y + .5, z + .5, newCount));
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

	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	Vec3 *xh0 = a->clipX(b, tls->xx0);
	Vec3 *xh1 = a->clipX(b, tls->xx1);

	Vec3 *yh0 = a->clipY(b, tls->yy0);
	Vec3 *yh1 = a->clipY(b, tls->yy1);

	Vec3 *zh0 = a->clipZ(b, tls->zz0);
	Vec3 *zh1 = a->clipZ(b, tls->zz1);

	Vec3 *closest = nullptr;

	if (containsX(xh0) && (closest == nullptr || a->distanceToSqr(xh0) < a->distanceToSqr(closest))) closest = xh0;
	if (containsX(xh1) && (closest == nullptr || a->distanceToSqr(xh1) < a->distanceToSqr(closest))) closest = xh1;
	if (containsY(yh0) && (closest == nullptr || a->distanceToSqr(yh0) < a->distanceToSqr(closest))) closest = yh0;
	if (containsY(yh1) && (closest == nullptr || a->distanceToSqr(yh1) < a->distanceToSqr(closest))) closest = yh1;
	if (containsZ(zh0) && (closest == nullptr || a->distanceToSqr(zh0) < a->distanceToSqr(closest))) closest = zh0;
	if (containsZ(zh1) && (closest == nullptr || a->distanceToSqr(zh1) < a->distanceToSqr(closest))) closest = zh1;

	if (closest == nullptr) return nullptr;

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
	if( v == nullptr) return false;

	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->y >= tls->yy0 && v->y <= tls->yy1 && v->z >= tls->zz0 && v->z <= tls->zz1;
}

bool Tile::containsY(Vec3 *v)
{
	if( v == nullptr) return false;

	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->x >= tls->xx0 && v->x <= tls->xx1 && v->z >= tls->zz0 && v->z <= tls->zz1;
}

bool Tile::containsZ(Vec3 *v)
{
	if( v == nullptr) return false;

	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return v->x >= tls->xx0 && v->x <= tls->xx1 && v->y >= tls->yy0 && v->y <= tls->yy1;
}

void Tile::wasExploded(Level *level, int x, int y, int z, Explosion *explosion)
{
}

bool Tile::mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<ItemInstance> item)
{
	return mayPlace(level, x, y, z, face);
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
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
}

double Tile::getShapeX0()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->xx0;
}

double Tile::getShapeX1()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->xx1;
}

double Tile::getShapeY0()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->yy0;
}

double Tile::getShapeY1()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->yy1;
}

double Tile::getShapeZ0()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
	// 4J Stu - Added this so that the TLS shape is correct for this tile
	if(tls->tileId != this->id) updateDefaultShape();
	return tls->zz0;
}

double Tile::getShapeZ1()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(Tile::tlsIdxShape));
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

int Tile::getSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return Redstone::SIGNAL_NONE;
}

bool Tile::isSignalSource()
{
	return false;
}

void Tile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
}

int Tile::getDirectSignal(LevelSource *level, int x, int y, int z, int dir)
{
	return Redstone::SIGNAL_NONE;
}

void Tile::updateDefaultShape()
{
	setShape(0,0,0,1,1,1);
}

void Tile::playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data)
{
	// 4J Stu - Special case - only record a crop destroy if is fully grown
	if( id==Tile::wheat_Id )
	{
		if( Tile::wheat->getResource(data, nullptr, 0) > 0 )
			player->awardStat(
			GenericStats::blocksMined(id),
			GenericStats::param_blocksMined(id,data,1)
			);
	}
	else if (id == Tile::potatoes_Id)
	{
		if (Tile::potatoes->getResource(data, nullptr, 0) > 0)
			player->awardStat(
			GenericStats::blocksMined(id),
			GenericStats::param_blocksMined(id,data,1)
			);
	}
	else if (id == Tile::carrots_Id)
	{
		if (Tile::potatoes->getResource(data, nullptr, 0) > 0)
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


	if (isSilkTouchable() && EnchantmentHelper::hasSilkTouch(player))
	{
		shared_ptr<ItemInstance> item = getSilkTouchItemInstance(data);
		if (item != nullptr)
		{
			popResource(level, x, y, z, item);
		}
	}
	else
	{
		int playerBonusLevel = EnchantmentHelper::getDiggingLootBonus(player);
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
	return std::make_shared<ItemInstance>(id, 1, popData);
}

int Tile::getResourceCountForLootBonus(int bonusLevel, Random *random)
{
	return getResourceCount(random);
}

bool Tile::canSurvive(Level *level, int x, int y, int z)
{
	return true;
}

void Tile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
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

bool Tile::triggerEvent(Level *level, int x, int y, int z, int b0, int b1)
{
	return false;
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

void Tile::levelTimeChanged(Level *level, int64_t delta, int64_t newTime)
{
}

bool Tile::useOwnCloneData()
{
	return false;
}

bool Tile::canInstantlyTick()
{
	return true;
}

bool Tile::dropFromExplosion(Explosion *explosion)
{
	return true;
}

bool Tile::isMatching(int id)
{
	return this->id == id;
}

bool Tile::isMatching(int tileIdA, int tileIdB)
{
	if (tileIdA == tileIdB)
	{
		return true;
	}
	if (tileIdA == 0 || tileIdB == 0 || tiles[tileIdA] == nullptr || tiles[tileIdB] == nullptr)
	{
		return false;
	}
	return tiles[tileIdA]->isMatching(tileIdB);
}

bool Tile::hasAnalogOutputSignal()
{
	return false;
}

int Tile::getAnalogOutputSignal(Level *level, int x, int y, int z, int dir)
{
	return Redstone::SIGNAL_NONE;
}

Tile *Tile::setIconName(const wstring &iconName)
{
	this->iconName = iconName;
	return this;
}

wstring Tile::getIconName()
{
	return iconName.empty() ? L"MISSING_ICON_TILE_" + std::to_wstring(id) + L"_" + std::to_wstring(descriptionId) : iconName;
}

void Tile::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(getIconName());
}

wstring Tile::getTileItemIconName()
{
	return L"";
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
const int Tile::stone_Id;
const int Tile::grass_Id;
const int Tile::dirt_Id;
//				4
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
//				25
const int Tile::bed_Id;
const int Tile::goldenRail_Id;
const int Tile::detectorRail_Id;
const int Tile::pistonStickyBase_Id;
const int Tile::web_Id;
const int Tile::tallgrass_Id;
const int Tile::deadBush_Id;
const int Tile::pistonBase_Id;
const int Tile::pistonExtensionPiece_Id;
const int Tile::wool_Id;
const int Tile::pistonMovingPiece_Id;
const int Tile::flower_Id;
const int Tile::rose_Id;
const int Tile::mushroom_brown_Id;
const int Tile::mushroom_red_Id;
const int Tile::goldBlock_Id;
const int Tile::ironBlock_Id;
const int Tile::stoneSlab_Id;
const int Tile::stoneSlabHalf_Id;
const int Tile::redBrick_Id;
const int Tile::tnt_Id;
const int Tile::bookshelf_Id;
const int Tile::mossyCobblestone_Id;
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
const int Tile::wheat_Id;
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
const int Tile::redstoneTorch_off_Id;
const int Tile::redstoneTorch_on_Id;
const int Tile::button_stone_Id;
const int Tile::topSnow_Id;
const int Tile::ice_Id;
const int Tile::snow_Id;
const int Tile::cactus_Id;
const int Tile::clay_Id;
const int Tile::reeds_Id;
const int Tile::jukebox_Id;
const int Tile::fence_Id;
const int Tile::pumpkin_Id;
const int Tile::netherRack_Id;
const int Tile::soulsand_Id;
const int Tile::glowstone_Id;
const int Tile::portalTile_Id;
const int Tile::litPumpkin_Id;
const int Tile::cake_Id;
const int Tile::diode_off_Id;
const int Tile::diode_on_Id;
const int Tile::stained_glass_Id;
const int Tile::trapdoor_Id;
const int Tile::monsterStoneEgg_Id;
const int Tile::stoneBrick_Id;
const int Tile::hugeMushroom_brown_Id;
const int Tile::hugeMushroom_red_Id;
const int Tile::ironFence_Id;
const int Tile::thinGlass_Id;
const int Tile::melon_Id;
const int Tile::pumpkinStem_Id;
const int Tile::melonStem_Id;
const int Tile::vine_Id;
const int Tile::fenceGate_Id;
const int Tile::stairs_bricks_Id;
const int Tile::stairs_stoneBrick_Id;
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
const int Tile::endStone_Id;
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
