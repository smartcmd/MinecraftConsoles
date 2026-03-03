#pragma once

using namespace std;

#include "Container.h"
#include "UseAnim.h"
#include "Rarity.h"

class MapItem;
class Mob;
class Player;
class Random;
class Level;
class ShearsItem;
class PotionItem;
class HitResult;
class IconRegister;
class Icon;
class ArmorItem;
class BowItem;
class FishingRodItem;
class EnchantedBookItem;
class EmptyMapItem;

#define ITEM_ICON_COLUMNS 16


class Item : public enable_shared_from_this<Item>
{
protected:
	//static const UUID BASE_ATTACK_DAMAGE_UUID;

public:
	static const int ITEM_NUM_COUNT = 32000;

	static void staticCtor();
	static void staticInit();

	// 4J-PB - added for new crafting menu
	enum
	{
		eMaterial_undefined=0,
		eMaterial_wood,
		eMaterial_stone,
		eMaterial_iron,
		eMaterial_gold,
		eMaterial_diamond,
		eMaterial_cloth,
		eMaterial_chain, // 4J Stu - It's available in creative in 1.8
		eMaterial_detector,
		eMaterial_lapis,
		eMaterial_music,
		eMaterial_dye,
		eMaterial_sand,
		eMaterial_brick,
		eMaterial_clay,
		eMaterial_snow,
		eMaterial_bow,
		eMaterial_arrow,
		eMaterial_compass,
		eMaterial_clock,
		eMaterial_map,
		eMaterial_pumpkin,
		eMaterial_glowstone,
		eMaterial_water,
		eMaterial_trap,
		eMaterial_flintandsteel,
		eMaterial_shears,
		eMaterial_piston,
		eMaterial_stickypiston,
		eMaterial_gate,
		eMaterial_stoneSmooth,
		eMaterial_netherbrick,
		eMaterial_ender,
		eMaterial_glass,
		eMaterial_blaze,
		eMaterial_magic,
		eMaterial_melon,
		eMaterial_setfire,
		eMaterial_sprucewood,
		eMaterial_birchwood,
		eMaterial_junglewood,
		eMaterial_emerald,
		eMaterial_quartz,
		eMaterial_apple,
		eMaterial_carrot,
		eMaterial_redstone,
		eMaterial_coal,
		eMaterial_paper,
		eMaterial_book,
		eMaterial_bookshelf,
		eMaterial_wheat,

	}
	eMaterial;

	enum
	{
		eBaseItemType_undefined=0,
		eBaseItemType_sword,
		eBaseItemType_shovel,
		eBaseItemType_pickaxe,
		eBaseItemType_hatchet,
		eBaseItemType_hoe,
		eBaseItemType_door,
		eBaseItemType_helmet,
		eBaseItemType_chestplate,
		eBaseItemType_leggings,
		eBaseItemType_boots,
		eBaseItemType_ingot,
		eBaseItemType_rail,
		eBaseItemType_block,
		eBaseItemType_pressureplate,
		eBaseItemType_stairs,
		eBaseItemType_cloth,
		eBaseItemType_dyepowder,
		eBaseItemType_structwoodstuff,
		eBaseItemType_structblock,
		eBaseItemType_slab,
		eBaseItemType_halfslab,
		eBaseItemType_torch,
		eBaseItemType_bow,
		eBaseItemType_pockettool,
		eBaseItemType_utensil,
		eBaseItemType_piston,
		eBaseItemType_devicetool,
		eBaseItemType_fence,
		eBaseItemType_device,
		eBaseItemType_treasure,
		eBaseItemType_seed,
		eBaseItemType_HangingItem,
		eBaseItemType_button,
		eBaseItemType_chest,
		eBaseItemType_rod,
		eBaseItemType_giltFruit,
		eBaseItemType_carpet,
		eBaseItemType_clay,
		eBaseItemType_glass,
		eBaseItemType_redstoneContainer,
		eBaseItemType_fireworks,
		eBaseItemType_lever,
		eBaseItemType_paper,
		eBaseItemType_MAXTYPES,
	}
	eBaseItemType;

protected:
	static const int ICON_COLUMNS = ITEM_ICON_COLUMNS;
	static wstring ICON_DESCRIPTION_PREFIX; // 4J Stu - Was const but we have to static initialise it outside of this class

public:

	class Tier
	{
	public:
		static const Tier *WOOD; //
		static const Tier *STONE; //
		static const Tier *IRON; //
		static const Tier *DIAMOND; //
		static const Tier *GOLD;

	private:
		const int level;
		const int uses;
		const float speed;
		const float damage;
		const int enchantmentValue;

		// 4J Stu - Had to make this public but was protected
		// We shouldn't be creating these except the static initialisation
	public:
		Tier(int level, int uses, float speed, float damage, int enchantmentValue);

	public:
		int getUses() const;
		float getSpeed() const;
		float getAttackDamageBonus() const;
		int getLevel() const;
		int getEnchantmentValue() const;
		int getTierItemId() const;
	};

protected:
	static Random *random;

private:
	static const int MAX_STACK_SIZE = Container::LARGE_MAX_STACK_SIZE;

public:
	static ItemArray items;

	static Item *shovel_iron;
	static Item *pickAxe_iron;
	static Item *hatchet_iron;
	static Item *flintAndSteel;
	static Item *apple;
	static BowItem *bow;
	static Item *arrow;
	static Item *coal;
	static Item *diamond;
	static Item *ironIngot;
	static Item *goldIngot;
	static Item *sword_iron;

	static Item *sword_wood;
	static Item *shovel_wood;
	static Item *pickAxe_wood;
	static Item *hatchet_wood;

	static Item *sword_stone;
	static Item *shovel_stone;
	static Item *pickAxe_stone;
	static Item *hatchet_stone;

	static Item *sword_diamond;
	static Item *shovel_diamond;
	static Item *pickAxe_diamond;
	static Item *hatchet_diamond;

	static Item *stick;
	static Item *bowl;
	static Item *mushroomStew;

	static Item *sword_gold;
	static Item *shovel_gold;
	static Item *pickAxe_gold;
	static Item *hatchet_gold;

	static Item *string;
	static Item *feather;
	static Item *gunpowder;

	static Item *hoe_wood;
	static Item *hoe_stone;
	static Item *hoe_iron;
	static Item *hoe_diamond;
	static Item *hoe_gold;

	static Item *seeds_wheat;
	static Item *wheat;
	static Item *bread;

	static ArmorItem *helmet_leather;
	static ArmorItem *chestplate_leather;
	static ArmorItem *leggings_leather;
	static ArmorItem *boots_leather;

	static ArmorItem *helmet_chain;
	static ArmorItem *chestplate_chain;
	static ArmorItem *leggings_chain;
	static ArmorItem *boots_chain;

	static ArmorItem *helmet_iron;
	static ArmorItem *chestplate_iron;
	static ArmorItem *leggings_iron;
	static ArmorItem *boots_iron;

	static ArmorItem *helmet_diamond;
	static ArmorItem *chestplate_diamond;
	static ArmorItem *leggings_diamond;
	static ArmorItem *boots_diamond;

	static ArmorItem *helmet_gold;
	static ArmorItem *chestplate_gold;
	static ArmorItem *leggings_gold;
	static ArmorItem *boots_gold;

	static Item *flint;
	static Item *porkChop_raw;
	static Item *porkChop_cooked;
	static Item *painting;

	static Item *apple_gold;

	static Item *sign;
	static Item *door_wood;

	static Item *bucket_empty;
	static Item *bucket_water;
	static Item *bucket_lava;

	static Item *minecart;
	static Item *saddle;
	static Item *door_iron;
	static Item *redStone;
	static Item *snowBall;

	static Item *boat;

	static Item *leather;
	static Item *bucket_milk;
	static Item *brick;
	static Item *clay;
	static Item *reeds;
	static Item *paper;
	static Item *book;
	static Item *slimeBall;
	static Item *minecart_chest;
	static Item *minecart_furnace;
	static Item *egg;
	static Item *compass;
	static FishingRodItem *fishingRod;
	static Item *clock;
	static Item *yellowDust;
	static Item *fish_raw;
	static Item *fish_cooked;

	static Item *dye_powder;
	static Item *bone;
	static Item *sugar;
	static Item *cake;

	static Item *bed;

	static Item *repeater;
	static Item *cookie;

	static MapItem *map;

	static ShearsItem *shears;

	static Item *melon;

	static Item *seeds_pumpkin;
	static Item *seeds_melon;

	static Item *beef_raw;
	static Item *beef_cooked;
	static Item *chicken_raw;
	static Item *chicken_cooked;
	static Item *rotten_flesh;

	static Item *enderPearl;

	static Item *blazeRod;
	static Item *ghastTear;
	static Item *goldNugget;

	static Item *netherwart_seeds;

	static PotionItem *potion;
	static Item *glassBottle;

	static Item *spiderEye;
	static Item *fermentedSpiderEye;

	static Item *blazePowder;
	static Item *magmaCream;

	static Item *brewingStand;
	static Item *cauldron;
	static Item *eyeOfEnder;
	static Item *speckledMelon;

	static Item *spawnEgg;

	static Item *expBottle;

	static Item *skull;

	static Item *record_01;
	static Item *record_02;
	static Item *record_03;
	static Item *record_04;
	static Item *record_05;
	static Item *record_06;
	static Item *record_07;
	static Item *record_08;
	static Item *record_09;
	static Item *record_10;
	static Item *record_11;
	static Item *record_12;

	// TU9
	static Item *fireball;
	static Item *frame;

	// TU14
	//static Item writingBook;
	//static Item writtenBook;

	static Item *emerald;

	static Item *flowerPot;

	static Item *carrots;
	static Item *potato;
	static Item *potatoBaked;
	static Item *potatoPoisonous;

	static EmptyMapItem *emptyMap;

	static Item *carrotGolden;

	static Item *carrotOnAStick;
	static Item *netherStar;
	static Item *pumpkinPie;

	static Item *fireworks;
	static Item *fireworksCharge;
	static Item *netherQuartz;

	static Item *comparator;
	static Item *netherbrick;
	static EnchantedBookItem *enchantedBook;
	static Item *minecart_tnt;
	static Item *minecart_hopper;

	static Item *horseArmorMetal;
	static Item *horseArmorGold;
	static Item *horseArmorDiamond;
	static Item *lead;
	static Item *nameTag;


	static const int shovel_iron_Id			= 256;
	static const int pickAxe_iron_Id		= 257;
	static const int hatchet_iron_Id		= 258;
	static const int flintAndSteel_Id		= 259;
	static const int apple_Id				= 260;
	static const int bow_Id					= 261;
	static const int arrow_Id				= 262;
	static const int coal_Id				= 263;
	static const int diamond_Id				= 264;
	static const int ironIngot_Id			= 265;
	static const int goldIngot_Id			= 266;
	static const int sword_iron_Id			= 267;
	static const int sword_wood_Id			= 268;
	static const int shovel_wood_Id			= 269;
	static const int pickAxe_wood_Id		= 270;
	static const int hatchet_wood_Id		= 271;
	static const int sword_stone_Id			= 272;
	static const int shovel_stone_Id		= 273;
	static const int pickAxe_stone_Id		= 274;
	static const int hatchet_stone_Id		= 275;
	static const int sword_diamond_Id		= 276;
	static const int shovel_diamond_Id		= 277;
	static const int pickAxe_diamond_Id		= 278;
	static const int hatchet_diamond_Id		= 279;
	static const int stick_Id				= 280;
	static const int bowl_Id				= 281;
	static const int mushroomStew_Id		= 282;
	static const int sword_gold_Id			= 283;
	static const int shovel_gold_Id			= 284;
	static const int pickAxe_gold_Id		= 285;
	static const int hatchet_gold_Id		= 286;
	static const int string_Id				= 287;
	static const int feather_Id				= 288;
	static const int gunpowder_Id			= 289;
	static const int hoe_wood_Id			= 290;
	static const int hoe_stone_Id			= 291;
	static const int hoe_iron_Id			= 292;
	static const int hoe_diamond_Id			= 293;
	static const int hoe_gold_Id			= 294;
	static const int seeds_wheat_Id			= 295;
	static const int wheat_Id				= 296;
	static const int bread_Id				= 297;

	static const int helmet_leather_Id		= 298;
	static const int chestplate_leather_Id	= 299;
	static const int leggings_leather_Id	= 300;
	static const int boots_leather_Id		= 301;

	static const int helmet_chain_Id		= 302;
	static const int chestplate_chain_Id	= 303;
	static const int leggings_chain_Id		= 304;
	static const int boots_chain_Id			= 305;

	static const int helmet_iron_Id			= 306;
	static const int chestplate_iron_Id		= 307;
	static const int leggings_iron_Id		= 308;
	static const int boots_iron_Id			= 309;

	static const int helmet_diamond_Id		= 310;
	static const int chestplate_diamond_Id	= 311;
	static const int leggings_diamond_Id	= 312;
	static const int boots_diamond_Id		= 313;

	static const int helmet_gold_Id			= 314;
	static const int chestplate_gold_Id		= 315;
	static const int leggings_gold_Id		= 316;
	static const int boots_gold_Id			= 317;

	static const int flint_Id				= 318;
	static const int porkChop_raw_Id		= 319;
	static const int porkChop_cooked_Id		= 320;
	static const int painting_Id			= 321;
	static const int apple_gold_Id			= 322;
	static const int sign_Id				= 323;
	static const int door_wood_Id			= 324;
	static const int bucket_empty_Id		= 325;
	static const int bucket_water_Id		= 326;
	static const int bucket_lava_Id			= 327;
	static const int minecart_Id			= 328;
	static const int saddle_Id				= 329;
	static const int door_iron_Id			= 330;
	static const int redStone_Id			= 331;
	static const int snowBall_Id			= 332;
	static const int boat_Id				= 333;
	static const int leather_Id				= 334;
	static const int bucket_milk_Id			= 335;
	static const int brick_Id				= 336;
	static const int clay_Id				= 337;
	static const int reeds_Id				= 338;
	static const int paper_Id				= 339;
	static const int book_Id				= 340;
	static const int slimeBall_Id			= 341;
	static const int minecart_chest_Id		= 342;
	static const int minecart_furnace_Id	= 343;
	static const int egg_Id					= 344;
	static const int compass_Id				= 345;
	static const int fishingRod_Id			= 346;
	static const int clock_Id				= 347;
	static const int yellowDust_Id			= 348;
	static const int fish_raw_Id			= 349;
	static const int fish_cooked_Id			= 350;
	static const int dye_powder_Id			= 351;
	static const int bone_Id				= 352;
	static const int sugar_Id				= 353;
	static const int cake_Id				= 354;
	static const int bed_Id					= 355;
	static const int repeater_Id			= 356;
	static const int cookie_Id				= 357;
	static const int map_Id					= 358;

	// 1.7.3
	static const int shears_Id				= 359;

	// 1.8.2
	static const int melon_Id				= 360;
	static const int seeds_pumpkin_Id		= 361;
	static const int seeds_melon_Id			= 362;
	static const int beef_raw_Id			= 363;
	static const int beef_cooked_Id			= 364;
	static const int chicken_raw_Id			= 365;
	static const int chicken_cooked_Id		= 366;
	static const int rotten_flesh_Id		= 367;
	static const int enderPearl_Id			= 368;

	// 1.0.1
	static const int blazeRod_Id			= 369;
	static const int ghastTear_Id			= 370;
	static const int goldNugget_Id			= 371;
	static const int netherwart_seeds_Id	= 372;
	static const int potion_Id				= 373;
	static const int glassBottle_Id			= 374;
	static const int spiderEye_Id			= 375;
	static const int fermentedSpiderEye_Id	= 376;
	static const int blazePowder_Id			= 377;
	static const int magmaCream_Id			= 378;
	static const int brewingStand_Id		= 379;
	static const int cauldron_Id			= 380;
	static const int eyeOfEnder_Id			= 381;
	static const int speckledMelon_Id		= 382;

	// 1.1
	static const int spawnEgg_Id		= 383;

	static const int expBottle_Id			 = 384;

	// TU 12
	static const int skull_Id				= 397;

	static const int record_01_Id			= 2256;
	static const int record_02_Id			= 2257;
	static const int record_03_Id			= 2258;
	static const int record_04_Id			= 2259;
	static const int record_05_Id			= 2260;
	static const int record_06_Id			= 2261;
	static const int record_07_Id			= 2262;
	static const int record_09_Id			= 2263;
	static const int record_10_Id		    = 2264;
	static const int record_11_Id		    = 2265;
	static const int record_12_Id			= 2266;

	// 4J-PB - this one isn't playable in the PC game, but is fine in ours
	static const int record_08_Id			= 2267;

	// TU9
	static const int fireball_Id			 = 385;
	static const int itemFrame_Id			 = 389;

	// TU14
	//static const int writingBook_Id			 = 130;
	//static const int writtenBook_Id			 = 131;

	static const int emerald_Id				 = 388;

	static const int flowerPot_Id = 390;

	static const int carrots_Id = 391;
	static const int potato_Id = 392;
	static const int potatoBaked_Id = 393;
	static const int potatoPoisonous_Id = 394;

	static const int emptyMap_Id = 395;

	static const int carrotGolden_Id = 396;

	static const int carrotOnAStick_Id = 398;
	static const int netherStar_Id = 399;
	static const int pumpkinPie_Id = 400;

	static const int fireworks_Id = 401;
	static const int fireworksCharge_Id = 402;

	static const int enchantedBook_Id = 403;

	static const int comparator_Id = 404;
	static const int netherbrick_Id = 405;
	static const int netherQuartz_Id = 406;
	static const int minecart_tnt_Id = 407;
	static const int minecart_hopper_Id = 408;

	static const int horseArmorMetal_Id = 417;
	static const int horseArmorGold_Id = 418;
	static const int horseArmorDiamond_Id = 419;
	static const int lead_Id = 420;
	static const int nameTag_Id = 421;

public:
	const int id;

protected:
	int maxStackSize;

private:
	int maxDamage;

protected:
	Icon *icon;
	// 4J-PB - added for new crafting menu
	int m_iBaseItemType;
	int m_iMaterial;
	bool m_handEquipped;
	bool m_isStackedByData;

private:
	Item *craftingRemainingItem;
	wstring potionBrewingFormula;

	// 4J Stu - A value from strings.h, that is the name of the item
	unsigned int descriptionId;

	// 4J Stu - A value from strings.h that says what this does
	unsigned int useDescriptionId;

	wstring m_textureName;

protected:
	Item(int id);

public:
	// 4J Using per-item textures now
	Item *setIconName(const wstring &name);
	wstring getIconName();
	Item *setMaxStackSize(int max);
	Item *setBaseItemTypeAndMaterial(int iType,int iMaterial);
	int getBaseItemType();
	int getMaterial();

	virtual int getIconType();
	virtual Icon *getIcon(int auxValue);
	Icon *getIcon(shared_ptr<ItemInstance> itemInstance);

	virtual bool useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly=false);
	virtual float getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile);
	virtual bool TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual shared_ptr<ItemInstance> useTimeDepleted(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual int getMaxStackSize() const;
	virtual int getLevelDataForAuxValue(int auxValue);
	bool isStackedByData();

protected:
	Item *setStackedByData(bool isStackedByData);

public:
	int getMaxDamage();

protected:
	Item *setMaxDamage(int maxDamage);

public:
	bool canBeDepleted();

	/**
	* Returns true when the item was used to deal more than default damage
	*
	* @param itemInstance
	* @param mob
	* @param attacker
	* @return
	*/
	virtual bool hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker);

	/**
	* Returns true when the item was used to mine more efficiently
	*
	* @param itemInstance
	* @param tile
	* @param x
	* @param yf
	* @param z
	* @param owner
	* @return
	*/
	virtual bool mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner);
	virtual int getAttackDamage(shared_ptr<Entity> entity);
	virtual bool canDestroySpecial(Tile *tile);
	virtual bool interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob);
	Item *handEquipped();
	virtual bool isHandEquipped();
	virtual bool isMirroredArt();
	Item *setDescriptionId(unsigned int id);
	LPCWSTR getDescription();
	LPCWSTR getDescription(shared_ptr<ItemInstance> instance);
	virtual unsigned int getDescriptionId(int iData = -1);
	virtual unsigned int getDescriptionId(shared_ptr<ItemInstance> instance);
	Item *setUseDescriptionId(unsigned int id);
	virtual unsigned int getUseDescriptionId();
	virtual unsigned int getUseDescriptionId(shared_ptr<ItemInstance> instance);
	Item *setCraftingRemainingItem(Item *craftingRemainingItem);
	virtual bool shouldMoveCraftingResultToInventory(shared_ptr<ItemInstance> instance);
	virtual bool shouldOverrideMultiplayerNBT();
	Item *getCraftingRemainingItem();
	bool hasCraftingRemainingItem();
	std::wstring getName();
	virtual int getColor(shared_ptr<ItemInstance> item, int spriteLayer);
	virtual void inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected);
	virtual void onCraftedBy(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player);
	virtual bool isComplex();

	virtual UseAnim getUseAnimation(shared_ptr<ItemInstance> itemInstance);
	virtual int getUseDuration(shared_ptr<ItemInstance> itemInstance);
	virtual void releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft);

protected:
	virtual Item *setPotionBrewingFormula(const wstring &potionBrewingFormula);

public:
	virtual wstring getPotionBrewingFormula();
	virtual bool hasPotionBrewingFormula();
	virtual void appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced);
	virtual wstring getHoverName(shared_ptr<ItemInstance> itemInstance);
	virtual bool isFoil(shared_ptr<ItemInstance> itemInstance);
	virtual const Rarity *getRarity(shared_ptr<ItemInstance> itemInstance);
	virtual bool isEnchantable(shared_ptr<ItemInstance> itemInstance);

protected:
	HitResult *getPlayerPOVHitResult(Level *level, shared_ptr<Player> player, bool alsoPickLiquid);

public:
	virtual int getEnchantmentValue();
	virtual bool hasMultipleSpriteLayers();
	virtual Icon *getLayerIcon(int auxValue, int spriteLayer);
	virtual bool mayBePlacedInAdventureMode();
	virtual bool isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem);
	virtual void registerIcons(IconRegister *iconRegister);
	virtual attrAttrModMap *getDefaultAttributeModifiers();
};
