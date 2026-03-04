#pragma once
// #include "Material.h"
// #include "Vec3.h"
// #include "Definitions.h"
// #include "SoundTypes.h"
// using namespace std;
#include "Icon_SPU.h"
#include "Material_SPU.h"
#include <stddef.h>

class ChunkRebuildData;
class GrassTile_SPU;
class LeafTile;
class TallGrass;
class DeadBushTile;
class FireTile;
class PortalTile;
class MycelTile;
class PistonExtensionTile;
class PistonMovingPiece;
class StoneTile;
class stoneBrick;
class Bush;
class StairTile;
class LiquidTile;
class PistonBaseTile;
class ChestTile;
class RedStoneDustTile;
class RepeaterTile;
class CauldronTile;
class TripWireSourceTile;
class Random;
class HitResult;
class Level;

class Player;
class LevelSource;
class Mob;
class TileEntity;
class HalfSlabTile;
class IconRegister;

class TileData_SPU
{
public:

	class Bitfield256
	{
		unsigned int m_data[8];

		void setBit(unsigned char i){ m_data[i>>5] |= (1<<(i%32)); }
		void clearBit(unsigned char i) { m_data[i>>5] &= ~(1<<(i%32)); }
	public:
		bool operator[](unsigned char i) { return (m_data[i>>5] & 1<<(i%32)) != 0; }
		void set(unsigned char i, bool val){ (val) ? setBit(i) : clearBit(i); }
	};
	Bitfield256	mipmapEnable;
	Bitfield256	solid;
	Bitfield256	transculent;
	Bitfield256	_sendTileData;
	Bitfield256	propagate;
	Bitfield256	signalSource;
	Bitfield256	cubeShaped;


	char		lightBlock[256];
	char		lightEmission[256];
	char		materialIDs[256];
	Material_SPU	materials[Material_SPU::num_Ids];

	float xx0[256];
	float yy0[256];
	float zz0[256];
	float xx1[256];
	float yy1[256];
	float zz1[256];

	int			iconTexWidth;
	int			iconTexHeight;
	Icon_SPU iconData[256];


	Icon_SPU grass_iconTop;
	Icon_SPU grass_iconSnowSide;
	Icon_SPU grass_iconSideOverlay;

	Icon_SPU ironFence_EdgeTexture;
	Icon_SPU thinGlass_EdgeTexture;

	Icon_SPU farmTile_Wet;
	Icon_SPU farmTile_Dry;

	Icon_SPU doorTile_Icons[8];

	Icon_SPU tallGrass_Icons[3];

	Icon_SPU sandStone_icons[3];
	Icon_SPU sandStone_iconTop;
	Icon_SPU sandStone_iconBottom;

	Icon_SPU woodTile_icons[4];

	Icon_SPU treeTile_icons[4];
	Icon_SPU treeTile_iconTop;

	Icon_SPU leafTile_icons[2][4];	
	bool leafTile_allowSame;
	int	leafTile_fancyTextureSet;

	Icon_SPU cropTile_icons[8];

	Icon_SPU furnaceTile_iconTop;
	Icon_SPU furnaceTile_iconFront;
	Icon_SPU furnaceTile_iconFront_lit;

	Icon_SPU liquidTile_iconWaterStill;
	Icon_SPU liquidTile_iconWaterFlow;
	Icon_SPU liquidTile_iconLavaStill;
	Icon_SPU liquidTile_iconLavaFlow;

	Icon_SPU fireTile_icons[2];
	Icon_SPU sapling_icons[4];

	bool glassTile_allowSame;
	bool iceTile_allowSame;

	Icon_SPU dispenserTile_iconTop;
	Icon_SPU dispenserTile_iconFront;
	Icon_SPU dispenserTile_iconFrontVertical;

	Icon_SPU railTile_iconTurn;
	Icon_SPU railTile_iconTurnGolden;

	Icon_SPU detectorRailTile_icons[2];

	Icon_SPU tntTile_iconBottom;
	Icon_SPU tntTile_iconTop;

	Icon_SPU workBench_iconTop;
	Icon_SPU workBench_iconFront;
	
	Icon_SPU cactusTile_iconTop;
	Icon_SPU cactusTile_iconBottom;

	Icon_SPU recordPlayer_iconTop;

	Icon_SPU pumpkinTile_iconTop;
	Icon_SPU pumpkinTile_iconFace;
	Icon_SPU pumpkinTile_iconFaceLit;

	Icon_SPU cakeTile_iconTop;
	Icon_SPU cakeTile_iconBottom;
	Icon_SPU cakeTile_iconInner;

	Icon_SPU smoothStoneBrick_icons[4];

	Icon_SPU hugeMushroom_icons[2];
	Icon_SPU hugeMushroom_iconStem;
	Icon_SPU hugeMushroom_iconInside;

	Icon_SPU melonTile_iconTop;

	Icon_SPU stemTile_iconAngled;

	Icon_SPU mycelTile_iconTop;
	Icon_SPU mycelTile_iconSnowSide;

	Icon_SPU netherStalk_icons[3];

	Icon_SPU enchantmentTable_iconTop;
	Icon_SPU enchantmentTable_iconBottom;

	Icon_SPU brewingStand_iconBase;

	Icon_SPU redStoneDust_iconCross;
	Icon_SPU redStoneDust_iconLine;
	Icon_SPU redStoneDust_iconCrossOver;
	Icon_SPU redStoneDust_iconLineOver;

	Icon_SPU clothTile_icons[16];	

	Icon_SPU stoneSlab_iconSide;

	Icon_SPU carrot_icons[4];
	Icon_SPU potato_icons[4];
	Icon_SPU anvil_icons[3];


	Icon_SPU quartzBlock_icons[5];
	Icon_SPU quartzBlock_iconChiseledTop;
	Icon_SPU quartzBlock_iconLinesTop;
	Icon_SPU quartzBlock_iconTop;
	Icon_SPU quartzBlock_iconBottom;


	int anvilPart; // normally part of the AnvilTile class

	unsigned int stemTile_minColour;
	unsigned int stemTile_maxColour;
	unsigned int waterLily_colour;

	unsigned int foliageColor_evergreenColor;
	unsigned int foliageColor_birchColor;


protected:
//     float destroySpeed;
//     float explosionResistance;
//     bool isInventoryItem;
//     bool collectStatistics;
// 	int m_iMaterial;
// 	int m_iBaseItemType;


public:
//     const SoundType *soundType;
// 
//     float gravity;
//    Material *material;
//     float friction;

private:
//     unsigned int descriptionId;
//     unsigned int useDescriptionId; // 4J Added
// 
// 	wstring m_textureName;

protected:
// 	Icon *icon;

public:

    int getRenderShape();

	void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
    float getBrightness(ChunkRebuildData *level, int x, int y, int z);
	int getLightColor(ChunkRebuildData *level, int x, int y, int z);		// 4J - brought forward from 1.8.2
    bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face);

public:
    Icon_SPU *getTexture(ChunkRebuildData *level, int x, int y, int z, int face);
    Icon_SPU *getTexture(int face, int data);
    Icon_SPU *getTexture(int face);
	public:
    void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = NULL);	// 4J added forceData, forceEntity param
	double getShapeX0();
	double getShapeX1();
	double getShapeY0();
	double getShapeY1();
	double getShapeZ0();
	double getShapeZ1();
// 	int getColor(int auxData);
    int getColor(ChunkRebuildData *level, int x, int y, int z);
	float getShadeBrightness(ChunkRebuildData *level, int x, int y, int z);	// 4J - brought forward from 1.8.2
	bool hasMaterialLiquid() { return false; } // material->isLiquid()
};

class Tile_SPU
{
public:
	static const int SHAPE_INVISIBLE = -1;
	static const int SHAPE_BLOCK = 0;
	static const int SHAPE_CROSS_TEXTURE = 1;
	static const int SHAPE_TORCH = 2;
	static const int SHAPE_FIRE = 3;
	static const int SHAPE_WATER = 4;
	static const int SHAPE_RED_DUST = 5;
	static const int SHAPE_ROWS = 6;
	static const int SHAPE_DOOR = 7;
	static const int SHAPE_LADDER = 8;
	static const int SHAPE_RAIL = 9;
	static const int SHAPE_STAIRS = 10;
	static const int SHAPE_FENCE = 11;
	static const int SHAPE_LEVER = 12;
	static const int SHAPE_CACTUS = 13;
	static const int SHAPE_BED = 14;
	static const int SHAPE_DIODE = 15;
	static const int SHAPE_PISTON_BASE = 16;
	static const int SHAPE_PISTON_EXTENSION = 17;
	static const int SHAPE_IRON_FENCE = 18;
	static const int SHAPE_STEM = 19;
	static const int SHAPE_VINE = 20;
	static const int SHAPE_FENCE_GATE = 21;
	static const int SHAPE_ENTITYTILE_ANIMATED = 22;
	static const int SHAPE_LILYPAD = 23;
	static const int SHAPE_CAULDRON = 24;
	static const int SHAPE_BREWING_STAND = 25;
	static const int SHAPE_PORTAL_FRAME = 26;
	static const int SHAPE_EGG = 27;
	static const int SHAPE_COCOA = 28;
	static const int SHAPE_TRIPWIRE_SOURCE = 29;
	static const int SHAPE_TRIPWIRE = 30;
	static const int SHAPE_TREE = 31;
	static const int SHAPE_WALL = 32;
	static const int SHAPE_FLOWER_POT = 33;
	static const int SHAPE_BEACON = 34;
	static const int SHAPE_ANVIL = 35;
	static const int SHAPE_QUARTZ = 39;

	// 4J - this array of simple constants made so the compiler can optimise references to Ids that were previous of the form Tile_SPU::<whatever>->id, and are now simply Tile_SPU::whatever_Id
	static const int rock_Id = 1;
	static const int grass_Id = 2;
	static const int dirt_Id = 3;
	static const int stoneBrick_Id = 4;
	static const int wood_Id = 5;
	static const int sapling_Id = 6;
	static const int unbreakable_Id = 7;
	static const int water_Id = 8;
	static const int calmWater_Id = 9;
	static const int lava_Id = 10;
	static const int calmLava_Id = 11;
	static const int sand_Id = 12;
	static const int gravel_Id = 13;
	static const int goldOre_Id = 14;
	static const int ironOre_Id = 15;
	static const int coalOre_Id = 16;
	static const int treeTrunk_Id = 17;
	static const int leaves_Id = 18;
	static const int sponge_Id = 19;
	static const int glass_Id = 20;
	static const int lapisOre_Id = 21;
	static const int lapisBlock_Id = 22;
	static const int dispenser_Id = 23;
	static const int sandStone_Id = 24;
	static const int musicBlock_Id = 25;
	static const int bed_Id = 26;
	static const int goldenRail_Id = 27;
	static const int detectorRail_Id = 28;
	static const int pistonStickyBase_Id = 29;
	static const int web_Id = 30;
	static const int tallgrass_Id = 31;
	static const int deadBush_Id = 32;
	static const int pistonBase_Id = 33;
	static const int pistonExtensionPiece_Id = 34;
	static const int cloth_Id = 35;
	static const int pistonMovingPiece_Id = 36;
	static const int flower_Id = 37;
	static const int rose_Id = 38;
	static const int mushroom1_Id = 39;
	static const int mushroom2_Id = 40;
	static const int goldBlock_Id = 41;
	static const int ironBlock_Id = 42;
	static const int stoneSlab_Id = 43;
	static const int stoneSlabHalf_Id = 44;
	static const int redBrick_Id = 45;
	static const int tnt_Id = 46;
	static const int bookshelf_Id = 47;
	static const int mossStone_Id = 48;
	static const int obsidian_Id = 49;
	static const int torch_Id = 50;
	static const int fire_Id = 51;
	static const int mobSpawner_Id = 52;
	static const int stairs_wood_Id = 53;
	static const int chest_Id = 54;
	static const int redStoneDust_Id = 55;
	static const int diamondOre_Id = 56;
	static const int diamondBlock_Id = 57;
	static const int workBench_Id = 58;
	static const int crops_Id = 59;
	static const int farmland_Id = 60;
	static const int furnace_Id = 61;
	static const int furnace_lit_Id = 62;
	static const int sign_Id = 63;
	static const int door_wood_Id = 64;
	static const int ladder_Id = 65;
	static const int rail_Id = 66;
	static const int stairs_stone_Id = 67;
	static const int wallSign_Id = 68;
	static const int lever_Id = 69;
	static const int pressurePlate_stone_Id = 70;
	static const int door_iron_Id = 71;
	static const int pressurePlate_wood_Id = 72;
	static const int redStoneOre_Id = 73;
	static const int redStoneOre_lit_Id = 74;
	static const int notGate_off_Id = 75;
	static const int notGate_on_Id = 76;
	static const int button_stone_Id = 77;
	static const int topSnow_Id = 78;
	static const int ice_Id = 79;
	static const int snow_Id = 80;
	static const int cactus_Id = 81;
	static const int clay_Id = 82;
	static const int reeds_Id = 83;
	static const int recordPlayer_Id = 84;
	static const int fence_Id = 85;
	static const int pumpkin_Id = 86;
	static const int hellRock_Id = 87;
	static const int hellSand_Id = 88;
	static const int lightGem_Id = 89;
	static const int portalTile_Id = 90;
	static const int litPumpkin_Id = 91;
	static const int cake_Id = 92;
	static const int diode_off_Id = 93;
	static const int diode_on_Id = 94;
	static const int aprilFoolsJoke_Id = 95;
	static const int trapdoor_Id = 96;

	static const int monsterStoneEgg_Id = 97;
	static const int stoneBrickSmooth_Id = 98;
	static const int hugeMushroom1_Id = 99;
	static const int hugeMushroom2_Id = 100;
	static const int ironFence_Id = 101;
	static const int thinGlass_Id = 102;
	static const int melon_Id = 103;
	static const int pumpkinStem_Id = 104;
	static const int melonStem_Id = 105;
	static const int vine_Id = 106;
	static const int fenceGate_Id = 107;
	static const int stairs_bricks_Id = 108;
	static const int stairs_stoneBrickSmooth_Id = 109;

	static const int mycel_Id = 110;
	static const int waterLily_Id = 111;
	static const int netherBrick_Id = 112;
	static const int netherFence_Id = 113;
	static const int stairs_netherBricks_Id = 114;
	static const int netherStalk_Id = 115;
	static const int enchantTable_Id = 116;
	static const int brewingStand_Id = 117;
	static const int cauldron_Id = 118;
	static const int endPortalTile_Id = 119;
	static const int endPortalFrameTile_Id = 120;
	static const int whiteStone_Id = 121;
	static const int dragonEgg_Id = 122;
	static const int redstoneLight_Id = 123;
	static const int redstoneLight_lit_Id = 124;


	static const int woodSlab_Id = 125;
	static const int woodSlabHalf_Id = 126;
	static const int cocoa_Id = 127;
	static const int stairs_sandstone_Id = 128;
	static const int stairs_sprucewood_Id = 134;
	static const int stairs_birchwood_Id = 135;
	static const int stairs_junglewood_Id = 136;
	static const int emeraldOre_Id = 129;
	static const int enderChest_Id = 130;
	static const int tripWireSource_Id = 131;
	static const int tripWire_Id = 132;
	static const int emeraldBlock_Id = 133;

	static const int cobbleWall_Id = 139;
	static const int flowerPot_Id = 140;
	static const int carrots_Id = 141;
	static const int potatoes_Id = 142;
	static const int anvil_Id = 145;
	static const int button_wood_Id = 143;
	static const int skull_Id = 144;
	static const int netherQuartz_Id = 153;
	static const int quartzBlock_Id = 155;
	static const int stairs_quartz_Id = 156;
	static const int woolCarpet_Id = 171;

	static Tile_SPU m_tiles[256];

	Tile_SPU() { id = -1;}
	Tile_SPU(int tileID) { id = tileID; }
	virtual ~Tile_SPU() {}

	double getShapeX0() { return ms_pTileData->xx0[id]; }
	double getShapeX1() { return ms_pTileData->xx1[id]; }
	double getShapeY0() { return ms_pTileData->yy0[id]; }
	double getShapeY1() { return ms_pTileData->yy1[id]; }
	double getShapeZ0() { return ms_pTileData->zz0[id]; }
	double getShapeZ1() { return ms_pTileData->zz1[id]; }
	Material_SPU* getMaterial();

	virtual void updateShape(ChunkRebuildData *level, int x, int y, int z, int forceData = -1, TileEntity* forceEntity = NULL);	// 4J added forceData, forceEntity param
	virtual void updateDefaultShape();
	virtual void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
	virtual float getBrightness(ChunkRebuildData *level, int x, int y, int z);
	virtual int getLightColor(ChunkRebuildData *level, int x, int y, int z);		// 4J - brought forward from 1.8.2
	virtual bool shouldRenderFace(ChunkRebuildData *level, int x, int y, int z, int face);
	virtual int getRenderShape();
	virtual int getColor(ChunkRebuildData *level, int x, int y, int z);
	virtual float getShadeBrightness(ChunkRebuildData *level, int x, int y, int z);	// 4J - brought forward from 1.8.2
	virtual bool isSolidRender(bool isServerLevel = false);
	virtual bool isSolidFace(ChunkRebuildData *level, int x, int y, int z, int face);
	virtual int getRenderLayer();
	virtual Icon_SPU *getTexture(ChunkRebuildData *level, int x, int y, int z, int face);
	virtual Icon_SPU *getTexture(int face, int data);
	virtual Icon_SPU *getTexture(int face);

	bool isSignalSource()	{ return ms_pTileData->signalSource[id]; }
	bool isCubeShaped()		{ return ms_pTileData->cubeShaped[id]; }

	Icon_SPU* icon() { return &ms_pTileData->iconData[id]; }


	int id;
	static TileData_SPU* ms_pTileData;

	static void initTilePointers();
	static Tile_SPU* createFromID(int tileID);

};

class TileRef_SPU
{
	// because of the way this ref stuff works, we need all the tile classes to be of the same size, 
	// so make sure no data is added to child classes.
	Tile_SPU* m_pTile;
public:
	TileRef_SPU(int tileID) 		{	m_pTile = Tile_SPU::createFromID(tileID);	}
	virtual ~TileRef_SPU()			{ }
	Tile_SPU* operator->() const	{ return m_pTile; }
	Tile_SPU* getPtr() { return m_pTile; }
};

//class stoneBrick : public Tile {};