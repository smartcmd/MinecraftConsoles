#pragma once
#include "Material.h"
#include "Vec3.h"
#include "Definitions.h"
#include "SoundTypes.h"
using namespace std;

class GrassTile;
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
class Icon;
class IconRegister;

class ChunkRebuildData;

class Tile
{
	// 4J Stu - Stair tile accesses the protected members of a Tile object passed in
	friend class StairTile;
	friend class ChunkRebuildData;
	friend class WallTile;

protected:
	// 4J added so we can have separate shapes for different threads
	class ThreadStorage
	{
	public:
		double xx0, yy0, zz0, xx1, yy1, zz1;
		int tileId;
		ThreadStorage();
	};
	static DWORD tlsIdxShape;
public:	
	// Each new thread that needs to use Vec3 pools will need to call one of the following 2 functions, to either create its own
	// local storage, or share the default storage already allocated by the main thread
	static void CreateNewThreadStorage();
	static void ReleaseThreadStorage();

public:	
    static const int TILE_NUM_COUNT = 4096;
    static const int TILE_NUM_MASK = 0xfff; // 4096 - 1
    static const int TILE_NUM_SHIFT = 12; // 4096 is 12 bits

private:
	// 4J Stu - Was const but had to change it so that we can initialise it in TileStaticInit
	static wstring TILE_DESCRIPTION_PREFIX;
protected:
	static const float INDESTRUCTIBLE_DESTROY_TIME;

public:

	class SoundType
	{
	public:
//         wstring name;
// 		wstring breakSound;
// 		wstring stepSound;
		eMATERIALSOUND_TYPE eMaterialSound;
		int iBreakSound,iStepSound,iPlaceSound;
        float volume;
        float pitch;

        SoundType(eMATERIALSOUND_TYPE eMaterialSound, float volume, float pitch, int iBreakSound = -1, int iPlaceSound = -1);
	
		float getVolume() const;
		float getPitch() const;
		//wstring getBreakSound() const { return breakSound; }
		//wstring getStepSound()	const { return stepSound; }
		int getBreakSound() const;
		int getStepSound()	const;
		int getPlaceSound() const;
    };

    static SoundType *SOUND_NORMAL;
    static SoundType *SOUND_WOOD;
    static SoundType *SOUND_GRAVEL;
    static SoundType *SOUND_GRASS; 
    static SoundType *SOUND_STONE;
    static SoundType *SOUND_METAL;
    static SoundType *SOUND_GLASS;
    static SoundType *SOUND_CLOTH;
    static SoundType *SOUND_SAND;
	static SoundType *SOUND_SNOW;
	static SoundType *SOUND_LADDER;
	static SoundType *SOUND_ANVIL;

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

    static  Tile **tiles;

	static	bool	mipmapEnable[TILE_NUM_COUNT];
    static  bool	solid[TILE_NUM_COUNT];
    static  int		lightBlock[TILE_NUM_COUNT];
    static  bool	transculent[TILE_NUM_COUNT];
    static  int		lightEmission[TILE_NUM_COUNT];
    static  unsigned char _sendTileData[TILE_NUM_COUNT];		// 4J - was bool, changed to bitfield so we can indicate which bits are important to be sent
	static  bool	propagate[TILE_NUM_COUNT];

	// 4J - this array of simple constants made so the compiler can optimise references to Ids that were previous of the form Tile::<whatever>->id, and are now simply Tile::whatever_Id
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


    static Tile *rock;
    static GrassTile *grass;
    static Tile *dirt;
    static Tile *stoneBrick;
    static Tile *wood;
    static Tile *sapling;
    static Tile *unbreakable;
    static LiquidTile *water;
    static Tile *calmWater;
    static LiquidTile *lava;
    static Tile *calmLava;
    static Tile *sand;
    static Tile *gravel;
    static Tile *goldOre;
    static Tile *ironOre;
    static Tile *coalOre;
    static Tile *treeTrunk;
    static LeafTile *leaves;
    static Tile *sponge;
    static Tile *glass;
    static Tile *lapisOre;
    static Tile *lapisBlock;
    static Tile *dispenser;
    static Tile *sandStone;
    static Tile *musicBlock;
    static Tile *bed;
    static Tile *goldenRail;
    static Tile *detectorRail;
	static PistonBaseTile *pistonStickyBase;
    static Tile *web;
    static TallGrass *tallgrass;
    static DeadBushTile *deadBush;
    static PistonBaseTile *pistonBase;
    static PistonExtensionTile *pistonExtension;
    static Tile *cloth;
    static PistonMovingPiece *pistonMovingPiece;
    static Bush *flower;
    static Bush *rose;
    static Bush *mushroom1;
    static Bush *mushroom2;
    static Tile *goldBlock;
    static Tile *ironBlock;
//     static Tile *stoneSlab;
//     static Tile *stoneSlabHalf;
    static Tile *redBrick;
    static Tile *tnt;
    static Tile *bookshelf;
    static Tile *mossStone;
    static Tile *obsidian;
    static Tile *torch;
    static FireTile *fire;
    static Tile *mobSpawner;
    static Tile *stairs_wood;
    static ChestTile *chest;
    static RedStoneDustTile *redStoneDust;
    static Tile *diamondOre;
    static Tile *diamondBlock;
    static Tile *workBench;
    static Tile *crops;
    static Tile *farmland;
    static Tile *furnace;
    static Tile *furnace_lit;
    static Tile *sign;
    static Tile *door_wood;
    static Tile *ladder;
    static Tile *rail;
    static Tile *stairs_stone;
    static Tile *wallSign;
    static Tile *lever;
    static Tile *pressurePlate_stone;
    static Tile *door_iron;
    static Tile *pressurePlate_wood;
    static Tile *redStoneOre;
    static Tile *redStoneOre_lit;
    static Tile *notGate_off;
    static Tile *notGate_on;
    static Tile *button;
    static Tile *topSnow;
    static Tile *ice;
    static Tile *snow;
    static Tile *cactus;
    static Tile *clay;
    static Tile *reeds;
    static Tile *recordPlayer;
    static Tile *fence;
    static Tile *pumpkin;
    static Tile *hellRock;
    static Tile *hellSand;
    static Tile *lightGem;
    static PortalTile *portalTile;
    static Tile *litPumpkin;
    static Tile *cake;
    static RepeaterTile *diode_off;
    static RepeaterTile *diode_on;
    static Tile *aprilFoolsJoke;
    static Tile *trapdoor;

	static Tile *monsterStoneEgg;
	static Tile *stoneBrickSmooth;
	static Tile *hugeMushroom1;
	static Tile *hugeMushroom2;
	static Tile *ironFence;
	static Tile *thinGlass;
	static Tile *melon;
	static Tile *pumpkinStem;
	static Tile *melonStem;
	static Tile *vine;
	static Tile *fenceGate;
	static Tile *stairs_bricks;
	static Tile *stairs_stoneBrickSmooth;

    static MycelTile *mycel;
    static Tile *waterLily;
    static Tile *netherBrick;
    static Tile *netherFence;
    static Tile *stairs_netherBricks;
    static Tile *netherStalk;
    static Tile *enchantTable;
    static Tile *brewingStand;
    static CauldronTile *cauldron;
    static Tile *endPortalTile;
    static Tile *endPortalFrameTile;
    static Tile *whiteStone;
    static Tile *dragonEgg;
	static Tile *redstoneLight;
	static Tile *redstoneLight_lit;

	static Tile *stairs_sandstone;
	static Tile *woodStairsDark;
	static Tile *woodStairsBirch;
	static Tile *woodStairsJungle;
	static Tile *button_wood;
	static HalfSlabTile *woodSlab;
	static HalfSlabTile *woodSlabHalf;
	static HalfSlabTile *stoneSlab;
	static HalfSlabTile *stoneSlabHalf;
	static Tile *emeraldOre;
	static Tile *enderChest;
	static TripWireSourceTile *tripWireSource;
	static Tile *tripWire;
	static Tile *emeraldBlock;
    
	static Tile *cocoa;
	static Tile *skull;

	static Tile *cobbleWall;
	static Tile *flowerPot;
	static Tile *carrots;
	static Tile *potatoes;
	static Tile *anvil;	
	static Tile *netherQuartz;
	static Tile *quartzBlock;
	static Tile *stairs_quartz;

	static Tile *woolCarpet;

	static void staticCtor();

    int id;
protected:
    float destroySpeed;
    float explosionResistance;
    bool isInventoryItem;
    bool collectStatistics;
	bool _isTicking;
	bool _isEntityTile;
	int m_iMaterial;
	int m_iBaseItemType;

	// 4J Stu - Removed this in favour of a TLS version
    //double xx0, yy0, zz0, xx1, yy1, zz1;

public:
    const SoundType *soundType;

    float gravity;
    Material *material;
    float friction;

private:
    unsigned int descriptionId;
    unsigned int useDescriptionId; // 4J Added

	wstring m_textureName;

protected:
	Icon *icon;

protected:
	void _init(int id, Material *material, bool isSolidRender);
    Tile(int id, Material *material, bool isSolidRender = true);
	virtual ~Tile() {}
protected:
    virtual Tile *sendTileData(unsigned char importantMask=15);		// 4J - added importantMask to indicate which bits in the data are important
protected:
    virtual void init();
    virtual Tile *setSoundType(const SoundType *soundType);
    virtual Tile *setLightBlock(int i);
    virtual Tile *setLightEmission(float f);
    virtual Tile *setExplodeable(float explosionResistance);
	Tile *setBaseItemTypeAndMaterial(int iType,int iMaterial);
public:
	static bool isSolidBlockingTile(int t);
    virtual bool isCubeShaped();
	virtual bool isPathfindable(LevelSource *level, int x, int y, int z);
    virtual int getRenderShape();
	// 4J-PB added
	int getBaseItemType();
	int getMaterial();
protected:
	virtual Tile *setDestroyTime(float destroySpeed);
	virtual Tile *setIndestructible();
public:
	virtual float getDestroySpeed(Level *level, int x, int y, int z);
protected:
    virtual Tile *setTicking(bool tick);
	virtual Tile *disableMipmap();
public:
	virtual bool isTicking();
	virtual bool isEntityTile();
	virtual void setShape(float x0, float y0, float z0, float x1, float y1, float z1);
    virtual float getBrightness(LevelSource *level, int x, int y, int z);
	virtual int getLightColor(LevelSource *level, int x, int y, int z, int tileId=-1);		// 4J - brought forward from 1.8.2
    static bool isFaceVisible(Level *level, int x, int y, int z, int f);
    virtual bool shouldRenderFace(LevelSource *level, int x, int y, int z, int face);
    virtual bool isSolidFace(LevelSource *level, int x, int y, int z, int face);
    virtual Icon *getTexture(LevelSource *level, int x, int y, int z, int face);
    virtual Icon *getTexture(int face, int data);
    virtual Icon *getTexture(int face);
    virtual AABB *getTileAABB(Level *level, int x, int y, int z);
	virtual void addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source);
    virtual AABB *getAABB(Level *level, int x, int y, int z);
    virtual bool isSolidRender(bool isServerLevel = false);							// 4J - Added isServerLevel param
    virtual bool mayPick(int data, bool liquid);
    virtual bool mayPick();
    virtual void tick(Level *level, int x, int y, int z, Random *random);
    virtual void animateTick(Level *level, int x, int y, int z, Random *random);
    virtual void destroy(Level *level, int x, int y, int z, int data);
    virtual void neighborChanged(Level *level, int x, int y, int z, int type);
    virtual void addLights(Level *level, int x, int y, int z);
    virtual int getTickDelay();
    virtual void onPlace(Level *level, int x, int y, int z);
    virtual void onRemove(Level *level, int x, int y, int z, int id, int data);
    virtual int getResourceCount(Random *random);
    virtual int getResource(int data, Random *random, int playerBonusLevel);
    virtual float getDestroyProgress(shared_ptr<Player> player, Level *level, int x, int y, int z);
    virtual void spawnResources(Level *level, int x, int y, int z, int data, int playerBonusLevel);
    virtual void spawnResources(Level *level, int x, int y, int z, int data, float odds, int playerBonusLevel);
protected:
	virtual void popResource(Level *level, int x, int y, int z, shared_ptr<ItemInstance> itemInstance);
	virtual void popExperience(Level *level, int x, int y, int z, int amount);

public:
	virtual int getSpawnResourcesAuxValue(int data);
    virtual float getExplosionResistance(shared_ptr<Entity> source);
    virtual HitResult *clip(Level *level, int xt, int yt, int zt, Vec3 *a, Vec3 *b);
private:
    virtual bool containsX(Vec3 *v);
    virtual bool containsY(Vec3 *v);
    virtual bool containsZ(Vec3 *v);
public:
    virtual void wasExploded(Level *level, int x, int y, int z);
    virtual int getRenderLayer();
    virtual bool mayPlace(Level *level, int x, int y, int z, int face);
    virtual bool mayPlace(Level *level, int x, int y, int z);
	virtual bool TestUse();
	virtual bool TestUse(Level *level, int x, int y, int z, shared_ptr<Player> player);
	virtual bool use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly = false); // 4J added soundOnly param
    virtual void stepOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
	virtual int getPlacedOnFaceDataValue(Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, int itemValue);
    virtual void prepareRender(Level *level, int x, int y, int z);
    virtual void attack(Level *level, int x, int y, int z, shared_ptr<Player> player);
    virtual void handleEntityInside(Level *level, int x, int y, int z, shared_ptr<Entity> e, Vec3 *current);
    virtual void updateShape(LevelSource *level, int x, int y, int z, int forceData = -1, shared_ptr<TileEntity> forceEntity = shared_ptr<TileEntity>());	// 4J added forceData, forceEntity param
	virtual double getShapeX0();
	virtual double getShapeX1();
	virtual double getShapeY0();
	virtual double getShapeY1();
	virtual double getShapeZ0();
	virtual double getShapeZ1();
    virtual int getColor() const;
	virtual int getColor(int auxData);
    virtual int getColor(LevelSource *level, int x, int y, int z);
	virtual int getColor(LevelSource *level, int x, int y, int z, int data);		// 4J added
    virtual bool getSignal(LevelSource *level, int x, int y, int z);
    virtual bool getSignal(LevelSource *level, int x, int y, int z, int dir);
    virtual bool isSignalSource();
    virtual void entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity);
    virtual bool getDirectSignal(Level *level, int x, int y, int z, int dir);
    virtual void updateDefaultShape();
    virtual void playerDestroy(Level *level, shared_ptr<Player> player, int x, int y, int z, int data);
    virtual bool canSurvive(Level *level, int x, int y, int z);
protected:
	virtual bool isSilkTouchable();
	virtual shared_ptr<ItemInstance> getSilkTouchItemInstance(int data);
public:
	virtual int getResourceCountForLootBonus(int bonusLevel, Random *random);
    virtual void setPlacedBy(Level *level, int x, int y, int z, shared_ptr<Mob> by);
	virtual void finalizePlacement(Level *level, int x, int y, int z, int data);
    virtual Tile *setDescriptionId(unsigned int id);
    virtual wstring getName();
    virtual unsigned int getDescriptionId(int iData = -1);
    virtual Tile *setUseDescriptionId(unsigned int id); // 4J Added
    virtual unsigned int getUseDescriptionId(); // 4J Added
    virtual void triggerEvent(Level *level, int x, int y, int z, int b0, int b1);
    virtual bool isCollectStatistics();

	// 4J Added so we can check before we try to add a tile to the tick list if it's actually going to do seomthing
	// Default to true (it's also checking a bool array) and just override when we need to be able to say no
	virtual bool shouldTileTick(Level *level, int x,int y,int z) { return true; }
protected:
    virtual Tile *setNotCollectStatistics();
public:
	virtual int getPistonPushReaction();
	virtual float getShadeBrightness(LevelSource *level, int x, int y, int z);	// 4J - brought forward from 1.8.2
	virtual void fallOn(Level *level, int x, int y, int z, shared_ptr<Entity> entity, float fallDistance);
	virtual int cloneTileId(Level *level, int x, int y, int z);
	virtual int cloneTileData(Level *level, int x, int y, int z);
	virtual void playerWillDestroy(Level *level, int x, int y, int z, int data, shared_ptr<Player> player);
	virtual void onRemoving(Level *level, int x, int y, int z, int data);
	virtual void handleRain(Level *level, int x, int y, int z);
	virtual void levelTimeChanged(Level *level, __int64 delta, __int64 newTime);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual wstring getTileItemIconName();	
	// 4J Using per-item textures now
	Tile *setTextureName(const wstring &name);
	// AP - added this function so we can generate the faceFlags for a block in a single fast function
	int getFaceFlags(LevelSource *level, int x, int y, int z);
};

class stoneBrick : public Tile {};
