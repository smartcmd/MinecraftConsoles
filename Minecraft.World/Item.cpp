#include "stdafx.h"

#include "net.minecraft.locale.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.food.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.stats.h"
#include "MapItem.h"
#include "Item.h"
#include "HangingEntityItem.h"
#include "HtmlString.h"

typedef Item::Tier _Tier;

//const UUID Item::BASE_ATTACK_DAMAGE_UUID = UUID::fromString(L"CB3F55D3-645C-4F38-A497-9C13A33DB5CF");

wstring Item::ICON_DESCRIPTION_PREFIX = L"item.";

const _Tier *_Tier::WOOD = new _Tier(0, 59, 2, 0, 15); //
const _Tier *_Tier::STONE = new _Tier(1, 131, 4, 1, 5); //
const _Tier *_Tier::IRON = new _Tier(2, 250, 6, 2, 14); //
const _Tier *_Tier::DIAMOND = new _Tier(3, 1561, 8, 3, 10); //
const _Tier *_Tier::GOLD = new _Tier(0, 32, 12, 0, 22);

Random *Item::random = new Random();

ItemArray Item::items = ItemArray( ITEM_NUM_COUNT );

Item *Item::shovel_iron = NULL;
Item *Item::pickAxe_iron = NULL;
Item *Item::hatchet_iron = NULL;
Item *Item::flintAndSteel = NULL;
Item *Item::apple = NULL;
BowItem *Item::bow = NULL;
Item *Item::arrow = NULL;
Item *Item::coal = NULL;
Item *Item::diamond = NULL;
Item *Item::ironIngot = NULL;
Item *Item::goldIngot = NULL;
Item *Item::sword_iron = NULL;

Item *Item::sword_wood = NULL;
Item *Item::shovel_wood = NULL;
Item *Item::pickAxe_wood = NULL;
Item *Item::hatchet_wood = NULL;

Item *Item::sword_stone = NULL;
Item *Item::shovel_stone = NULL;
Item *Item::pickAxe_stone = NULL;
Item *Item::hatchet_stone = NULL;

Item *Item::sword_diamond = NULL;
Item *Item::shovel_diamond = NULL;
Item *Item::pickAxe_diamond = NULL;
Item *Item::hatchet_diamond = NULL;

Item *Item::stick = NULL;
Item *Item::bowl = NULL;
Item *Item::mushroomStew = NULL;

Item *Item::sword_gold = NULL;
Item *Item::shovel_gold = NULL;
Item *Item::pickAxe_gold = NULL;
Item *Item::hatchet_gold = NULL;

Item *Item::string = NULL;
Item *Item::feather = NULL;
Item *Item::gunpowder = NULL;

Item *Item::hoe_wood = NULL;
Item *Item::hoe_stone = NULL;
Item *Item::hoe_iron = NULL;
Item *Item::hoe_diamond = NULL;
Item *Item::hoe_gold = NULL;

Item *Item::seeds_wheat = NULL;
Item *Item::wheat = NULL;
Item *Item::bread = NULL;

ArmorItem *Item::helmet_leather = NULL;
ArmorItem *Item::chestplate_leather = NULL;
ArmorItem *Item::leggings_leather = NULL;
ArmorItem *Item::boots_leather = NULL;

ArmorItem *Item::helmet_chain = NULL;
ArmorItem *Item::chestplate_chain = NULL;
ArmorItem *Item::leggings_chain = NULL;
ArmorItem *Item::boots_chain = NULL;

ArmorItem *Item::helmet_iron = NULL;
ArmorItem *Item::chestplate_iron = NULL;
ArmorItem *Item::leggings_iron = NULL;
ArmorItem *Item::boots_iron = NULL;

ArmorItem *Item::helmet_diamond = NULL;
ArmorItem *Item::chestplate_diamond = NULL;
ArmorItem *Item::leggings_diamond = NULL;
ArmorItem *Item::boots_diamond = NULL;

ArmorItem *Item::helmet_gold = NULL;
ArmorItem *Item::chestplate_gold = NULL;
ArmorItem *Item::leggings_gold = NULL;
ArmorItem *Item::boots_gold = NULL;

Item *Item::flint = NULL;
Item *Item::porkChop_raw = NULL;
Item *Item::porkChop_cooked = NULL;
Item *Item::painting = NULL;

Item *Item::apple_gold = NULL;

Item *Item::sign = NULL;
Item *Item::door_wood = NULL;

Item *Item::bucket_empty = NULL;
Item *Item::bucket_water = NULL;
Item *Item::bucket_lava = NULL;

Item *Item::minecart = NULL;
Item *Item::saddle = NULL;
Item *Item::door_iron = NULL;
Item *Item::redStone = NULL;
Item *Item::snowBall = NULL;

Item *Item::boat = NULL;

Item *Item::leather = NULL;
Item *Item::bucket_milk = NULL;
Item *Item::brick = NULL;
Item *Item::clay = NULL;
Item *Item::reeds = NULL;
Item *Item::paper = NULL;
Item *Item::book = NULL;
Item *Item::slimeBall = NULL;
Item *Item::minecart_chest = NULL;
Item *Item::minecart_furnace = NULL;
Item *Item::egg = NULL;
Item *Item::compass = NULL;
FishingRodItem *Item::fishingRod = NULL;
Item *Item::clock = NULL;
Item *Item::yellowDust = NULL;
Item *Item::fish_raw = NULL;
Item *Item::fish_cooked = NULL;

Item *Item::dye_powder = NULL;
Item *Item::bone = NULL;
Item *Item::sugar = NULL;
Item *Item::cake = NULL;

Item *Item::bed = NULL;

Item *Item::repeater = NULL;
Item *Item::cookie = NULL;

MapItem *Item::map = NULL;

Item *Item::record_01 = NULL;
Item *Item::record_02 = NULL;
Item *Item::record_03 = NULL;
Item *Item::record_04 = NULL;
Item *Item::record_05 = NULL;
Item *Item::record_06 = NULL;
Item *Item::record_07 = NULL;
Item *Item::record_08 = NULL;
Item *Item::record_09 = NULL;
Item *Item::record_10 = NULL;
Item *Item::record_11 = NULL;
Item *Item::record_12 = NULL;

ShearsItem *Item::shears = NULL;

Item *Item::melon = NULL;

Item *Item::seeds_pumpkin = NULL;
Item *Item::seeds_melon = NULL;

Item *Item::beef_raw = NULL;
Item *Item::beef_cooked = NULL;
Item *Item::chicken_raw = NULL;
Item *Item::chicken_cooked = NULL;
Item *Item::rotten_flesh = NULL;

Item *Item::enderPearl = NULL;

Item *Item::blazeRod = NULL;
Item *Item::ghastTear = NULL;
Item *Item::goldNugget = NULL;
Item *Item::netherwart_seeds = NULL;
PotionItem *Item::potion = NULL;
Item *Item::glassBottle = NULL;
Item *Item::spiderEye = NULL;
Item *Item::fermentedSpiderEye = NULL;
Item *Item::blazePowder = NULL;
Item *Item::magmaCream = NULL;
Item *Item::brewingStand = NULL;
Item *Item::cauldron = NULL;
Item *Item::eyeOfEnder = NULL;
Item *Item::speckledMelon = NULL;

Item *Item::spawnEgg = NULL;

Item *Item::expBottle = NULL;

// TU9
Item *Item::fireball = NULL;
Item *Item::frame = NULL;

Item *Item::skull = NULL;


// TU14
//Item *Item::writingBook = NULL;
//Item *Item::writtenBook = NULL;

Item *Item::emerald = NULL;

Item *Item::flowerPot = NULL;

Item *Item::carrots = NULL;
Item *Item::potato = NULL;
Item *Item::potatoBaked = NULL;
Item *Item::potatoPoisonous = NULL;

EmptyMapItem *Item::emptyMap = NULL;

Item *Item::carrotGolden = NULL;

Item *Item::carrotOnAStick = NULL;
Item *Item::netherStar = NULL;
Item *Item::pumpkinPie = NULL;
Item *Item::fireworks = NULL;
Item *Item::fireworksCharge = NULL;

EnchantedBookItem *Item::enchantedBook = NULL;

Item *Item::comparator = NULL;
Item *Item::netherbrick = NULL;
Item *Item::netherQuartz = NULL;
Item *Item::minecart_tnt = NULL;
Item *Item::minecart_hopper = NULL;

Item *Item::horseArmorMetal = NULL;
Item *Item::horseArmorGold = NULL;
Item *Item::horseArmorDiamond = NULL;
Item *Item::lead = NULL;
Item *Item::nameTag = NULL;


void Item::staticCtor()
{
	Item::sword_wood		= ( new WeaponItem(12, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_wood)		->setIconName(L"swordWood")->setDescriptionId(IDS_ITEM_SWORD_WOOD)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::sword_stone		= ( new WeaponItem(16, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_stone)	->setIconName(L"swordStone")->setDescriptionId(IDS_ITEM_SWORD_STONE)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::sword_iron		= ( new WeaponItem(11, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_iron)		->setIconName(L"swordIron")->setDescriptionId(IDS_ITEM_SWORD_IRON)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::sword_diamond		= ( new WeaponItem(20, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_diamond)	->setIconName(L"swordDiamond")->setDescriptionId(IDS_ITEM_SWORD_DIAMOND)->setUseDescriptionId(IDS_DESC_SWORD);
	Item::sword_gold		= ( new WeaponItem(27, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_sword,	eMaterial_gold)		->setIconName(L"swordGold")->setDescriptionId(IDS_ITEM_SWORD_GOLD)->setUseDescriptionId(IDS_DESC_SWORD);

	Item::shovel_wood		= ( new ShovelItem(13, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_wood)		->setIconName(L"shovelWood")->setDescriptionId(IDS_ITEM_SHOVEL_WOOD)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::shovel_stone		= ( new ShovelItem(17, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_stone)	->setIconName(L"shovelStone")->setDescriptionId(IDS_ITEM_SHOVEL_STONE)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::shovel_iron		= ( new ShovelItem(0, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_iron)		->setIconName(L"shovelIron")->setDescriptionId(IDS_ITEM_SHOVEL_IRON)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::shovel_diamond	= ( new ShovelItem(21, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_diamond)	->setIconName(L"shovelDiamond")->setDescriptionId(IDS_ITEM_SHOVEL_DIAMOND)->setUseDescriptionId(IDS_DESC_SHOVEL);
	Item::shovel_gold		= ( new ShovelItem(28, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_shovel,	eMaterial_gold)		->setIconName(L"shovelGold")->setDescriptionId(IDS_ITEM_SHOVEL_GOLD)->setUseDescriptionId(IDS_DESC_SHOVEL);

	Item::pickAxe_wood		= ( new PickaxeItem(14, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_wood)		->setIconName(L"pickaxeWood")->setDescriptionId(IDS_ITEM_PICKAXE_WOOD)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::pickAxe_stone		= ( new PickaxeItem(18, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_stone)	->setIconName(L"pickaxeStone")->setDescriptionId(IDS_ITEM_PICKAXE_STONE)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::pickAxe_iron		= ( new PickaxeItem(1, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_iron)		->setIconName(L"pickaxeIron")->setDescriptionId(IDS_ITEM_PICKAXE_IRON)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::pickAxe_diamond	= ( new PickaxeItem(22, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_diamond)	->setIconName(L"pickaxeDiamond")->setDescriptionId(IDS_ITEM_PICKAXE_DIAMOND)->setUseDescriptionId(IDS_DESC_PICKAXE);
	Item::pickAxe_gold		= ( new PickaxeItem(29, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_pickaxe,	eMaterial_gold)		->setIconName(L"pickaxeGold")->setDescriptionId(IDS_ITEM_PICKAXE_GOLD)->setUseDescriptionId(IDS_DESC_PICKAXE);

	Item::hatchet_wood		= ( new HatchetItem(15, _Tier::WOOD) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_wood)		->setIconName(L"hatchetWood")->setDescriptionId(IDS_ITEM_HATCHET_WOOD)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::hatchet_stone		= ( new HatchetItem(19, _Tier::STONE) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_stone)	->setIconName(L"hatchetStone")->setDescriptionId(IDS_ITEM_HATCHET_STONE)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::hatchet_iron		= ( new HatchetItem(2, _Tier::IRON) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_iron)		->setIconName(L"hatchetIron")->setDescriptionId(IDS_ITEM_HATCHET_IRON)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::hatchet_diamond	= ( new HatchetItem(23, _Tier::DIAMOND) )	->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_diamond)	->setIconName(L"hatchetDiamond")->setDescriptionId(IDS_ITEM_HATCHET_DIAMOND)->setUseDescriptionId(IDS_DESC_HATCHET);
	Item::hatchet_gold		= ( new HatchetItem(30, _Tier::GOLD) )		->setBaseItemTypeAndMaterial(eBaseItemType_hatchet,	eMaterial_gold)		->setIconName(L"hatchetGold")->setDescriptionId(IDS_ITEM_HATCHET_GOLD)->setUseDescriptionId(IDS_DESC_HATCHET);

	Item::hoe_wood			= ( new HoeItem(34, _Tier::WOOD) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_wood)		->setIconName(L"hoeWood")->setDescriptionId(IDS_ITEM_HOE_WOOD)->setUseDescriptionId(IDS_DESC_HOE);
	Item::hoe_stone			= ( new HoeItem(35, _Tier::STONE) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_stone)	->setIconName(L"hoeStone")->setDescriptionId(IDS_ITEM_HOE_STONE)->setUseDescriptionId(IDS_DESC_HOE);
	Item::hoe_iron			= ( new HoeItem(36, _Tier::IRON) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_iron)		->setIconName(L"hoeIron")->setDescriptionId(IDS_ITEM_HOE_IRON)->setUseDescriptionId(IDS_DESC_HOE);
	Item::hoe_diamond		= ( new HoeItem(37, _Tier::DIAMOND) )		->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_diamond)	->setIconName(L"hoeDiamond")->setDescriptionId(IDS_ITEM_HOE_DIAMOND)->setUseDescriptionId(IDS_DESC_HOE);
	Item::hoe_gold			= ( new HoeItem(38, _Tier::GOLD) )			->setBaseItemTypeAndMaterial(eBaseItemType_hoe,	eMaterial_gold)		->setIconName(L"hoeGold")->setDescriptionId(IDS_ITEM_HOE_GOLD)->setUseDescriptionId(IDS_DESC_HOE);

	Item::door_wood			= ( new DoorItem(68, Material::wood) )		->setBaseItemTypeAndMaterial(eBaseItemType_door,	eMaterial_wood)->setIconName(L"doorWood")->setDescriptionId(IDS_ITEM_DOOR_WOOD)->setUseDescriptionId(IDS_DESC_DOOR_WOOD);
	Item::door_iron			= ( new DoorItem(74, Material::metal) )		->setBaseItemTypeAndMaterial(eBaseItemType_door,	eMaterial_iron)->setIconName(L"doorIron")->setDescriptionId(IDS_ITEM_DOOR_IRON)->setUseDescriptionId(IDS_DESC_DOOR_IRON);

	Item::helmet_leather		= (ArmorItem *) ( ( new ArmorItem(42, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_HEAD) )			->setBaseItemTypeAndMaterial(eBaseItemType_helmet,	eMaterial_cloth)		->setIconName(L"helmetCloth")->setDescriptionId(IDS_ITEM_HELMET_CLOTH)->setUseDescriptionId(IDS_DESC_HELMET_LEATHER) );
	Item::helmet_iron		= (ArmorItem *) ( ( new ArmorItem(50, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_HEAD) )			->setBaseItemTypeAndMaterial(eBaseItemType_helmet,	eMaterial_iron)			->setIconName(L"helmetIron")->setDescriptionId(IDS_ITEM_HELMET_IRON)->setUseDescriptionId(IDS_DESC_HELMET_IRON) );
	Item::helmet_diamond	= (ArmorItem *) ( ( new ArmorItem(54, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_HEAD) )			->setBaseItemTypeAndMaterial(eBaseItemType_helmet,	eMaterial_diamond)		->setIconName(L"helmetDiamond")->setDescriptionId(IDS_ITEM_HELMET_DIAMOND)->setUseDescriptionId(IDS_DESC_HELMET_DIAMOND) );
	Item::helmet_gold		= (ArmorItem *) ( ( new ArmorItem(58, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_HEAD) )			->setBaseItemTypeAndMaterial(eBaseItemType_helmet,	eMaterial_gold)			->setIconName(L"helmetGold")->setDescriptionId(IDS_ITEM_HELMET_GOLD)->setUseDescriptionId(IDS_DESC_HELMET_GOLD) );

	Item::chestplate_leather	= (ArmorItem *) ( ( new ArmorItem(43, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_TORSO) )			->setBaseItemTypeAndMaterial(eBaseItemType_chestplate,	eMaterial_cloth)	->setIconName(L"chestplateCloth")->setDescriptionId(IDS_ITEM_CHESTPLATE_CLOTH)->setUseDescriptionId(IDS_DESC_CHESTPLATE_LEATHER) );
	Item::chestplate_iron		= (ArmorItem *) ( ( new ArmorItem(51, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_TORSO) )			->setBaseItemTypeAndMaterial(eBaseItemType_chestplate,	eMaterial_iron)		->setIconName(L"chestplateIron")->setDescriptionId(IDS_ITEM_CHESTPLATE_IRON)->setUseDescriptionId(IDS_DESC_CHESTPLATE_IRON) );
	Item::chestplate_diamond	= (ArmorItem *) ( ( new ArmorItem(55, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_TORSO) )		->setBaseItemTypeAndMaterial(eBaseItemType_chestplate,	eMaterial_diamond)	->setIconName(L"chestplateDiamond")->setDescriptionId(IDS_ITEM_CHESTPLATE_DIAMOND)->setUseDescriptionId(IDS_DESC_CHESTPLATE_DIAMOND) );
	Item::chestplate_gold		= (ArmorItem *) ( ( new ArmorItem(59, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_TORSO) )			->setBaseItemTypeAndMaterial(eBaseItemType_chestplate,	eMaterial_gold)		->setIconName(L"chestplateGold")->setDescriptionId(IDS_ITEM_CHESTPLATE_GOLD)->setUseDescriptionId(IDS_DESC_CHESTPLATE_GOLD) );

	Item::leggings_leather	= (ArmorItem *) ( ( new ArmorItem(44, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_LEGS) )			->setBaseItemTypeAndMaterial(eBaseItemType_leggings,	eMaterial_cloth)	->setIconName(L"leggingsCloth")->setDescriptionId(IDS_ITEM_LEGGINGS_CLOTH)->setUseDescriptionId(IDS_DESC_LEGGINGS_LEATHER) );
	Item::leggings_iron		= (ArmorItem *) ( ( new ArmorItem(52, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_LEGS) )			->setBaseItemTypeAndMaterial(eBaseItemType_leggings,	eMaterial_iron)		->setIconName(L"leggingsIron")->setDescriptionId(IDS_ITEM_LEGGINGS_IRON)->setUseDescriptionId(IDS_DESC_LEGGINGS_IRON) );
	Item::leggings_diamond	= (ArmorItem *) ( ( new ArmorItem(56, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_LEGS) )			->setBaseItemTypeAndMaterial(eBaseItemType_leggings,	eMaterial_diamond)	->setIconName(L"leggingsDiamond")->setDescriptionId(IDS_ITEM_LEGGINGS_DIAMOND)->setUseDescriptionId(IDS_DESC_LEGGINGS_DIAMOND) );
	Item::leggings_gold		= (ArmorItem *) ( ( new ArmorItem(60, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_LEGS) )			->setBaseItemTypeAndMaterial(eBaseItemType_leggings,	eMaterial_gold)		->setIconName(L"leggingsGold")->setDescriptionId(IDS_ITEM_LEGGINGS_GOLD)->setUseDescriptionId(IDS_DESC_LEGGINGS_GOLD) );

	Item::helmet_chain		= (ArmorItem *) ( ( new ArmorItem(46, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_HEAD) )			->setBaseItemTypeAndMaterial(eBaseItemType_helmet,		eMaterial_chain)	->setIconName(L"helmetChain")->setDescriptionId(IDS_ITEM_HELMET_CHAIN)->setUseDescriptionId(IDS_DESC_HELMET_CHAIN) );
	Item::chestplate_chain	= (ArmorItem *) ( ( new ArmorItem(47, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_TORSO) )			->setBaseItemTypeAndMaterial(eBaseItemType_chestplate,	eMaterial_chain)	->setIconName(L"chestplateChain")->setDescriptionId(IDS_ITEM_CHESTPLATE_CHAIN)->setUseDescriptionId(IDS_DESC_CHESTPLATE_CHAIN) );
	Item::leggings_chain	= (ArmorItem *) ( ( new ArmorItem(48, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_LEGS) )			->setBaseItemTypeAndMaterial(eBaseItemType_leggings,	eMaterial_chain)	->setIconName(L"leggingsChain")->setDescriptionId(IDS_ITEM_LEGGINGS_CHAIN)->setUseDescriptionId(IDS_DESC_LEGGINGS_CHAIN) );
	Item::boots_chain		= (ArmorItem *) ( ( new ArmorItem(49, ArmorItem::ArmorMaterial::CHAIN, 1, ArmorItem::SLOT_FEET) )			->setBaseItemTypeAndMaterial(eBaseItemType_boots,		eMaterial_chain)	->setIconName(L"bootsChain")->setDescriptionId(IDS_ITEM_BOOTS_CHAIN)->setUseDescriptionId(IDS_DESC_BOOTS_CHAIN) );

	Item::boots_leather		= (ArmorItem *) ( ( new ArmorItem(45, ArmorItem::ArmorMaterial::CLOTH, 0, ArmorItem::SLOT_FEET) )				->setBaseItemTypeAndMaterial(eBaseItemType_boots,	eMaterial_cloth)		->setIconName(L"bootsCloth")->setDescriptionId(IDS_ITEM_BOOTS_CLOTH)->setUseDescriptionId(IDS_DESC_BOOTS_LEATHER) );
	Item::boots_iron		= (ArmorItem *) ( ( new ArmorItem(53, ArmorItem::ArmorMaterial::IRON, 2, ArmorItem::SLOT_FEET) )				->setBaseItemTypeAndMaterial(eBaseItemType_boots,	eMaterial_iron)			->setIconName(L"bootsIron")->setDescriptionId(IDS_ITEM_BOOTS_IRON)->setUseDescriptionId(IDS_DESC_BOOTS_IRON) );
	Item::boots_diamond		= (ArmorItem *) ( ( new ArmorItem(57, ArmorItem::ArmorMaterial::DIAMOND, 3, ArmorItem::SLOT_FEET) )				->setBaseItemTypeAndMaterial(eBaseItemType_boots,	eMaterial_diamond)		->setIconName(L"bootsDiamond")->setDescriptionId(IDS_ITEM_BOOTS_DIAMOND)->setUseDescriptionId(IDS_DESC_BOOTS_DIAMOND) );
	Item::boots_gold		= (ArmorItem *) ( ( new ArmorItem(61, ArmorItem::ArmorMaterial::GOLD, 4, ArmorItem::SLOT_FEET) )				->setBaseItemTypeAndMaterial(eBaseItemType_boots,	eMaterial_gold)			->setIconName(L"bootsGold")->setDescriptionId(IDS_ITEM_BOOTS_GOLD)->setUseDescriptionId(IDS_DESC_BOOTS_GOLD) );

	Item::ironIngot = ( new Item(9) )->setIconName(L"ingotIron")					->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_iron)->setDescriptionId(IDS_ITEM_INGOT_IRON)->setUseDescriptionId(IDS_DESC_INGOT);
	Item::goldIngot = ( new Item(10) )->setIconName(L"ingotGold")					->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_gold)->setDescriptionId(IDS_ITEM_INGOT_GOLD)->setUseDescriptionId(IDS_DESC_INGOT);


	// 4J-PB - todo - add materials and base types to the ones below
	Item::bucket_empty		= ( new BucketItem(69, 0) )					->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_water)->setIconName(L"bucket")->setDescriptionId(IDS_ITEM_BUCKET)->setUseDescriptionId(IDS_DESC_BUCKET)->setMaxStackSize(16);
	Item::bowl = ( new Item(25) )										->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_wood)->setIconName(L"bowl")->setDescriptionId(IDS_ITEM_BOWL)->setUseDescriptionId(IDS_DESC_BOWL)->setMaxStackSize(64);

	Item::bucket_water		= ( new BucketItem(70, Tile::water_Id) )	->setIconName(L"bucketWater")->setDescriptionId(IDS_ITEM_BUCKET_WATER)->setCraftingRemainingItem(Item::bucket_empty)->setUseDescriptionId(IDS_DESC_BUCKET_WATER);
	Item::bucket_lava		= ( new BucketItem(71, Tile::lava_Id) )		->setIconName(L"bucketLava")->setDescriptionId(IDS_ITEM_BUCKET_LAVA)->setCraftingRemainingItem(Item::bucket_empty)->setUseDescriptionId(IDS_DESC_BUCKET_LAVA);
	Item::bucket_milk				= ( new MilkBucketItem(79) )->setIconName(L"milk")->setDescriptionId(IDS_ITEM_BUCKET_MILK)->setCraftingRemainingItem(Item::bucket_empty)->setUseDescriptionId(IDS_DESC_BUCKET_MILK);

	Item::bow = (BowItem *)( new BowItem(5) )										->setIconName(L"bow")->setBaseItemTypeAndMaterial(eBaseItemType_bow,	eMaterial_bow)		->setDescriptionId(IDS_ITEM_BOW)->setUseDescriptionId(IDS_DESC_BOW);
	Item::arrow = ( new Item(6) )													->setIconName(L"arrow")->setBaseItemTypeAndMaterial(eBaseItemType_bow,	eMaterial_arrow)	->setDescriptionId(IDS_ITEM_ARROW)->setUseDescriptionId(IDS_DESC_ARROW);

	Item::compass = ( new CompassItem(89) )											->setIconName(L"compass")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_compass)		->setDescriptionId(IDS_ITEM_COMPASS)->setUseDescriptionId(IDS_DESC_COMPASS);
	Item::clock = ( new ClockItem(91) )												->setIconName(L"clock")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_clock)		->setDescriptionId(IDS_ITEM_CLOCK)->setUseDescriptionId(IDS_DESC_CLOCK);
	Item::map = (MapItem *) ( new MapItem(102) )									->setIconName(L"map")->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_map)		->setDescriptionId(IDS_ITEM_MAP)->setUseDescriptionId(IDS_DESC_MAP);

	Item::flintAndSteel = ( new FlintAndSteelItem(3) )								->setIconName(L"flintAndSteel")->setBaseItemTypeAndMaterial(eBaseItemType_devicetool,	eMaterial_flintandsteel)->setDescriptionId(IDS_ITEM_FLINT_AND_STEEL)->setUseDescriptionId(IDS_DESC_FLINTANDSTEEL);
	Item::apple = ( new FoodItem(4, 4, FoodConstants::FOOD_SATURATION_LOW, false) )	->setIconName(L"apple")->setDescriptionId(IDS_ITEM_APPLE)->setUseDescriptionId(IDS_DESC_APPLE);
	Item::coal = ( new CoalItem(7) )												->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_coal)->setIconName(L"coal")->setDescriptionId(IDS_ITEM_COAL)->setUseDescriptionId(IDS_DESC_COAL);
	Item::diamond = ( new Item(8) )													->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_diamond)->setIconName(L"diamond")->setDescriptionId(IDS_ITEM_DIAMOND)->setUseDescriptionId(IDS_DESC_DIAMONDS);
	Item::stick = ( new Item(24) )													->setIconName(L"stick")->handEquipped()->setDescriptionId(IDS_ITEM_STICK)->setUseDescriptionId(IDS_DESC_STICK);
	Item::mushroomStew = ( new BowlFoodItem(26, 6) )								->setIconName(L"mushroomStew")->setDescriptionId(IDS_ITEM_MUSHROOM_STEW)->setUseDescriptionId(IDS_DESC_MUSHROOMSTEW);

	Item::string = ( new TilePlanterItem(31, Tile::tripWire) )						->setIconName(L"string")->setDescriptionId(IDS_ITEM_STRING)->setUseDescriptionId(IDS_DESC_STRING);
	Item::feather = ( new Item(32) )												->setIconName(L"feather")->setDescriptionId(IDS_ITEM_FEATHER)->setUseDescriptionId(IDS_DESC_FEATHER);
	Item::gunpowder = ( new Item(33) )												->setIconName(L"sulphur")->setDescriptionId(IDS_ITEM_SULPHUR)->setUseDescriptionId(IDS_DESC_SULPHUR)->setPotionBrewingFormula(PotionBrewing::MOD_GUNPOWDER);


	Item::seeds_wheat = ( new SeedItem(39, Tile::wheat_Id, Tile::farmland_Id) )			->setIconName(L"seeds")->setDescriptionId(IDS_ITEM_WHEAT_SEEDS)->setUseDescriptionId(IDS_DESC_WHEAT_SEEDS);
	Item::wheat = ( new Item(40) )														->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_wheat)->setIconName(L"wheat")->setDescriptionId(IDS_ITEM_WHEAT)->setUseDescriptionId(IDS_DESC_WHEAT);
	Item::bread = ( new FoodItem(41, 5, FoodConstants::FOOD_SATURATION_NORMAL, false) )	->setIconName(L"bread")->setDescriptionId(IDS_ITEM_BREAD)->setUseDescriptionId(IDS_DESC_BREAD);


	Item::flint = ( new Item(62) )																->setIconName(L"flint")->setDescriptionId(IDS_ITEM_FLINT)->setUseDescriptionId(IDS_DESC_FLINT);
	Item::porkChop_raw = ( new FoodItem(63, 3, FoodConstants::FOOD_SATURATION_LOW, true) )		->setIconName(L"porkchopRaw")->setDescriptionId(IDS_ITEM_PORKCHOP_RAW)->setUseDescriptionId(IDS_DESC_PORKCHOP_RAW);
	Item::porkChop_cooked = ( new FoodItem(64, 8, FoodConstants::FOOD_SATURATION_GOOD, true) )	->setIconName(L"porkchopCooked")->setDescriptionId(IDS_ITEM_PORKCHOP_COOKED)->setUseDescriptionId(IDS_DESC_PORKCHOP_COOKED);
	Item::painting = ( new HangingEntityItem(65,eTYPE_PAINTING) )								->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem,	eMaterial_cloth)->setIconName(L"painting")->setDescriptionId(IDS_ITEM_PAINTING)->setUseDescriptionId(IDS_DESC_PICTURE);

	Item::apple_gold = ( new GoldenAppleItem(66, 4, FoodConstants::FOOD_SATURATION_SUPERNATURAL, false) )->setCanAlwaysEat()->setEatEffect(MobEffect::regeneration->id, 5, 1, 1.0f)
																										->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,eMaterial_apple)->setIconName(L"appleGold")->setDescriptionId(IDS_ITEM_APPLE_GOLD);//->setUseDescriptionId(IDS_DESC_GOLDENAPPLE);

	Item::sign = ( new SignItem(67) )															->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem, eMaterial_wood)->setIconName(L"sign")->setDescriptionId(IDS_ITEM_SIGN)->setUseDescriptionId(IDS_DESC_SIGN);



	Item::minecart = ( new MinecartItem(72, Minecart::TYPE_RIDEABLE) )		->setIconName(L"minecart")->setDescriptionId(IDS_ITEM_MINECART)->setUseDescriptionId(IDS_DESC_MINECART);
	Item::saddle = ( new SaddleItem(73) )								->setIconName(L"saddle")->setDescriptionId(IDS_ITEM_SADDLE)->setUseDescriptionId(IDS_DESC_SADDLE);
	Item::redStone = ( new RedStoneItem(75) )							->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_redstone)->setIconName(L"redstone")->setDescriptionId(IDS_ITEM_REDSTONE)->setUseDescriptionId(IDS_DESC_REDSTONE_DUST)->setPotionBrewingFormula(PotionBrewing::MOD_REDSTONE);
	Item::snowBall = ( new SnowballItem(76) )							->setIconName(L"snowball")->setDescriptionId(IDS_ITEM_SNOWBALL)->setUseDescriptionId(IDS_DESC_SNOWBALL);

	Item::boat = ( new BoatItem(77) )									->setIconName(L"boat")->setDescriptionId(IDS_ITEM_BOAT)->setUseDescriptionId(IDS_DESC_BOAT);

	Item::leather = ( new Item(78) )									->setIconName(L"leather")->setDescriptionId(IDS_ITEM_LEATHER)->setUseDescriptionId(IDS_DESC_LEATHER);
	Item::brick = ( new Item(80) )										->setIconName(L"brick")->setDescriptionId(IDS_ITEM_BRICK)->setUseDescriptionId(IDS_DESC_BRICK);
	Item::clay = ( new Item(81) )										->setIconName(L"clay")->setDescriptionId(IDS_ITEM_CLAY)->setUseDescriptionId(IDS_DESC_CLAY);
	Item::reeds = ( new TilePlanterItem(82, Tile::reeds) )				->setIconName(L"reeds")->setDescriptionId(IDS_ITEM_REEDS)->setUseDescriptionId(IDS_DESC_REEDS);
	Item::paper = ( new Item(83) )										->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_paper)->setIconName(L"paper")->setDescriptionId(IDS_ITEM_PAPER)->setUseDescriptionId(IDS_DESC_PAPER);
	Item::book = ( new BookItem(84) )									->setBaseItemTypeAndMaterial(Item::eBaseItemType_paper, Item::eMaterial_book)->setIconName(L"book")->setDescriptionId(IDS_ITEM_BOOK)->setUseDescriptionId(IDS_DESC_BOOK);
	Item::slimeBall = ( new Item(85) )									->setIconName(L"slimeball")->setDescriptionId(IDS_ITEM_SLIMEBALL)->setUseDescriptionId(IDS_DESC_SLIMEBALL);
	Item::minecart_chest = ( new MinecartItem(86, Minecart::TYPE_CHEST) )	->setIconName(L"minecart_chest")->setDescriptionId(IDS_ITEM_MINECART_CHEST)->setUseDescriptionId(IDS_DESC_MINECARTWITHCHEST);
	Item::minecart_furnace = ( new MinecartItem(87, Minecart::TYPE_FURNACE) )->setIconName(L"minecart_furnace")->setDescriptionId(IDS_ITEM_MINECART_FURNACE)->setUseDescriptionId(IDS_DESC_MINECARTWITHFURNACE);
	Item::egg = ( new EggItem(88) )										->setIconName(L"egg")->setDescriptionId(IDS_ITEM_EGG)->setUseDescriptionId(IDS_DESC_EGG);
	Item::fishingRod = (FishingRodItem *)( new FishingRodItem(90) )	->setBaseItemTypeAndMaterial(eBaseItemType_rod, eMaterial_wood)->setIconName(L"fishingRod")->setDescriptionId(IDS_ITEM_FISHING_ROD)->setUseDescriptionId(IDS_DESC_FISHINGROD);
	Item::yellowDust = ( new Item(92) )									->setIconName(L"yellowDust")->setDescriptionId(IDS_ITEM_YELLOW_DUST)->setUseDescriptionId(IDS_DESC_YELLOW_DUST)->setPotionBrewingFormula(PotionBrewing::MOD_GLOWSTONE);
	Item::fish_raw = ( new FoodItem(93, 2, FoodConstants::FOOD_SATURATION_LOW, false) )			->setIconName(L"fishRaw")->setDescriptionId(IDS_ITEM_FISH_RAW)->setUseDescriptionId(IDS_DESC_FISH_RAW);
	Item::fish_cooked = ( new FoodItem(94, 5, FoodConstants::FOOD_SATURATION_NORMAL, false) )	->setIconName(L"fishCooked")->setDescriptionId(IDS_ITEM_FISH_COOKED)->setUseDescriptionId(IDS_DESC_FISH_COOKED);

	Item::dye_powder = ( new DyePowderItem(95) )			->setBaseItemTypeAndMaterial(eBaseItemType_dyepowder,	eMaterial_dye)->setIconName(L"dyePowder")->setDescriptionId(IDS_ITEM_DYE_POWDER)->setUseDescriptionId(-1);

	Item::bone = ( new Item(96) )										->setIconName(L"bone")->setDescriptionId(IDS_ITEM_BONE)->handEquipped()->setUseDescriptionId(IDS_DESC_BONE);
	Item::sugar = ( new Item(97) )										->setIconName(L"sugar")->setDescriptionId(IDS_ITEM_SUGAR)->setUseDescriptionId(IDS_DESC_SUGAR)->setPotionBrewingFormula(PotionBrewing::MOD_SUGAR);
	// 4J-PB  - changing the cake to be stackable - Jens ok'ed this 23/10/12
	//Item::cake = ( new TilePlanterItem(98, Tile::cake) )->setMaxStackSize(1)->setIcon(13, 1)->setDescriptionId(IDS_ITEM_CAKE)->setUseDescriptionId(IDS_DESC_CAKE);
	Item::cake = ( new TilePlanterItem(98, Tile::cake) )				->setIconName(L"cake")->setDescriptionId(IDS_ITEM_CAKE)->setUseDescriptionId(IDS_DESC_CAKE);

	Item::bed = ( new BedItem(99) )										->setMaxStackSize(1)->setIconName(L"bed")->setDescriptionId(IDS_ITEM_BED)->setUseDescriptionId(IDS_DESC_BED);

	Item::repeater = ( new TilePlanterItem(100, (Tile *)Tile::diode_off) )			->setIconName(L"diode")->setDescriptionId(IDS_ITEM_DIODE)->setUseDescriptionId(IDS_DESC_REDSTONEREPEATER);
	Item::cookie = ( new FoodItem(101, 2, FoodConstants::FOOD_SATURATION_POOR, false) )	->setIconName(L"cookie")->setDescriptionId(IDS_ITEM_COOKIE)->setUseDescriptionId(IDS_DESC_COOKIE);


	Item::shears = (ShearsItem *)( new ShearsItem(103) )								->setIconName(L"shears")->setBaseItemTypeAndMaterial(eBaseItemType_devicetool,	eMaterial_shears)->setDescriptionId(IDS_ITEM_SHEARS)->setUseDescriptionId(IDS_DESC_SHEARS);

	Item::melon = (new FoodItem(104, 2, FoodConstants::FOOD_SATURATION_LOW, false))		->setIconName(L"melon")->setDescriptionId(IDS_ITEM_MELON_SLICE)->setUseDescriptionId(IDS_DESC_MELON_SLICE);

	Item::seeds_pumpkin = (new SeedItem(105, Tile::pumpkinStem_Id, Tile::farmland_Id))	->setIconName(L"seeds_pumpkin")->setBaseItemTypeAndMaterial(eBaseItemType_seed,	eMaterial_pumpkin)->setDescriptionId(IDS_ITEM_PUMPKIN_SEEDS)->setUseDescriptionId(IDS_DESC_PUMPKIN_SEEDS);
	Item::seeds_melon = (new SeedItem(106, Tile::melonStem_Id, Tile::farmland_Id))		->setIconName(L"seeds_melon")->setBaseItemTypeAndMaterial(eBaseItemType_seed,	eMaterial_melon)->setDescriptionId(IDS_ITEM_MELON_SEEDS)->setUseDescriptionId(IDS_DESC_MELON_SEEDS);

	Item::beef_raw = (new FoodItem(107, 3, FoodConstants::FOOD_SATURATION_LOW, true))	->setIconName(L"beefRaw")->setDescriptionId(IDS_ITEM_BEEF_RAW)->setUseDescriptionId(IDS_DESC_BEEF_RAW);
	Item::beef_cooked = (new FoodItem(108, 8, FoodConstants::FOOD_SATURATION_GOOD, true))->setIconName(L"beefCooked")->setDescriptionId(IDS_ITEM_BEEF_COOKED)->setUseDescriptionId(IDS_DESC_BEEF_COOKED);
	Item::chicken_raw = (new FoodItem(109, 2, FoodConstants::FOOD_SATURATION_LOW, true))->setEatEffect(MobEffect::hunger->id, 30, 0, .3f)->setIconName(L"chickenRaw")->setDescriptionId(IDS_ITEM_CHICKEN_RAW)->setUseDescriptionId(IDS_DESC_CHICKEN_RAW);
	Item::chicken_cooked = (new FoodItem(110, 6, FoodConstants::FOOD_SATURATION_NORMAL, true))->setIconName(L"chickenCooked")->setDescriptionId(IDS_ITEM_CHICKEN_COOKED)->setUseDescriptionId(IDS_DESC_CHICKEN_COOKED);
	Item::rotten_flesh = (new FoodItem(111, 4, FoodConstants::FOOD_SATURATION_POOR, true))->setEatEffect(MobEffect::hunger->id, 30, 0, .8f)->setIconName(L"rottenFlesh")->setDescriptionId(IDS_ITEM_ROTTEN_FLESH)->setUseDescriptionId(IDS_DESC_ROTTEN_FLESH);

	Item::enderPearl =			(new EnderpearlItem(112))											->setIconName(L"enderPearl")->setDescriptionId(IDS_ITEM_ENDER_PEARL)->setUseDescriptionId(IDS_DESC_ENDER_PEARL);

	Item::blazeRod =			(new Item(113)	)													->setIconName(L"blazeRod")->setDescriptionId(IDS_ITEM_BLAZE_ROD)->setUseDescriptionId(IDS_DESC_BLAZE_ROD)->handEquipped();
	Item::ghastTear =			(new Item(114) )													->setIconName(L"ghastTear")->setDescriptionId(IDS_ITEM_GHAST_TEAR)->setUseDescriptionId(IDS_DESC_GHAST_TEAR)->setPotionBrewingFormula(PotionBrewing::MOD_GHASTTEARS);
	Item::goldNugget =			(new Item(115) )													->setBaseItemTypeAndMaterial(eBaseItemType_treasure,	eMaterial_gold)->setIconName(L"goldNugget")->setDescriptionId(IDS_ITEM_GOLD_NUGGET)->setUseDescriptionId(IDS_DESC_GOLD_NUGGET);

	Item::netherwart_seeds =	(new SeedItem(116, Tile::netherStalk_Id, Tile::soulsand_Id) )		->setIconName(L"netherStalkSeeds")->setDescriptionId(IDS_ITEM_NETHER_STALK_SEEDS)->setUseDescriptionId(IDS_DESC_NETHER_STALK_SEEDS)->setPotionBrewingFormula(PotionBrewing::MOD_NETHERWART);

	Item::potion =				(PotionItem *) ( ( new PotionItem(117) )							->setIconName(L"potion")->setDescriptionId(IDS_ITEM_POTION)->setUseDescriptionId(IDS_DESC_POTION)  );
	Item::glassBottle =			(new BottleItem(118) )												->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_glass)->setIconName(L"glassBottle")->setDescriptionId(IDS_ITEM_GLASS_BOTTLE)->setUseDescriptionId(IDS_DESC_GLASS_BOTTLE);

	Item::spiderEye =			(new FoodItem(119, 2, FoodConstants::FOOD_SATURATION_GOOD, false) )	->setEatEffect(MobEffect::poison->id, 5, 0, 1.0f)->setIconName(L"spiderEye")->setDescriptionId(IDS_ITEM_SPIDER_EYE)->setUseDescriptionId(IDS_DESC_SPIDER_EYE)->setPotionBrewingFormula(PotionBrewing::MOD_SPIDEREYE);
	Item::fermentedSpiderEye =	(new Item(120) )													->setIconName(L"fermentedSpiderEye")->setDescriptionId(IDS_ITEM_FERMENTED_SPIDER_EYE)->setUseDescriptionId(IDS_DESC_FERMENTED_SPIDER_EYE)->setPotionBrewingFormula(PotionBrewing::MOD_FERMENTEDEYE);

	Item::blazePowder =			(new Item(121) )													->setIconName(L"blazePowder")->setDescriptionId(IDS_ITEM_BLAZE_POWDER)->setUseDescriptionId(IDS_DESC_BLAZE_POWDER)->setPotionBrewingFormula(PotionBrewing::MOD_BLAZEPOWDER);
	Item::magmaCream =			(new Item(122) )													->setIconName(L"magmaCream")->setDescriptionId(IDS_ITEM_MAGMA_CREAM)->setUseDescriptionId(IDS_DESC_MAGMA_CREAM)->setPotionBrewingFormula(PotionBrewing::MOD_MAGMACREAM);

	Item::brewingStand =		(new TilePlanterItem(123, Tile::brewingStand) )						->setBaseItemTypeAndMaterial(eBaseItemType_device,	eMaterial_blaze)->setIconName(L"brewingStand")->setDescriptionId(IDS_ITEM_BREWING_STAND)->setUseDescriptionId(IDS_DESC_BREWING_STAND);
	Item::cauldron =			(new TilePlanterItem(124, Tile::cauldron) )							->setBaseItemTypeAndMaterial(eBaseItemType_utensil,	eMaterial_iron)->setIconName(L"cauldron")->setDescriptionId(IDS_ITEM_CAULDRON)->setUseDescriptionId(IDS_DESC_CAULDRON);
	Item::eyeOfEnder =			(new EnderEyeItem(125) )											->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_ender)->setIconName(L"eyeOfEnder")->setDescriptionId(IDS_ITEM_EYE_OF_ENDER)->setUseDescriptionId(IDS_DESC_EYE_OF_ENDER);
	Item::speckledMelon =		(new Item(126) )													->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,	eMaterial_melon)->setIconName(L"speckledMelon")->setDescriptionId(IDS_ITEM_SPECKLED_MELON)->setUseDescriptionId(IDS_DESC_SPECKLED_MELON)->setPotionBrewingFormula(PotionBrewing::MOD_SPECKLEDMELON);

	Item::spawnEgg =			(new SpawnEggItem(127))												->setIconName(L"monsterPlacer")->setDescriptionId(IDS_ITEM_MONSTER_SPAWNER)->setUseDescriptionId(IDS_DESC_MONSTER_SPAWNER);

	// 4J Stu - Brought this forward
	Item::expBottle =			(new ExperienceItem(128))											->setIconName(L"expBottle")->setDescriptionId(IDS_ITEM_EXP_BOTTLE)->setUseDescriptionId(IDS_DESC_EXP_BOTTLE);

	Item::record_01 =			( new RecordingItem(2000, L"13") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_01)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_02 =			( new RecordingItem(2001, L"cat") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_02)->setUseDescriptionId(IDS_DESC_RECORD);

	// 4J - new records brought forward from 1.2.3
	Item::record_03 =			( new RecordingItem(2002, L"blocks") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_03)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_04 =			( new RecordingItem(2003, L"chirp") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_04)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_05 =			( new RecordingItem(2004, L"far") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_05)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_06 =			( new RecordingItem(2005, L"mall") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_06)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_07 =			( new RecordingItem(2006, L"mellohi") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_07)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_09 =			( new RecordingItem(2007, L"stal") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_08)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_10 =			( new RecordingItem(2008, L"strad") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_09)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_11 =			( new RecordingItem(2009, L"ward") )								->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_10)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_12 =			( new RecordingItem(2010, L"11") )									->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_11)->setUseDescriptionId(IDS_DESC_RECORD);
	Item::record_08 =			( new RecordingItem(2011, L"where are we now") )					->setIconName(L"record")->setDescriptionId(IDS_ITEM_RECORD_12)->setUseDescriptionId(IDS_DESC_RECORD);

	// TU9
	// putting the fire charge in as a torch, so that it stacks without being near the middle of the selection boxes
	Item::fireball =			(new FireChargeItem(129))											->setBaseItemTypeAndMaterial(eBaseItemType_torch,	eMaterial_setfire)->setIconName(L"fireball")->setDescriptionId(IDS_ITEM_FIREBALL)->setUseDescriptionId(IDS_DESC_FIREBALL);
	Item::frame =				(new HangingEntityItem(133,eTYPE_ITEM_FRAME))						->setBaseItemTypeAndMaterial(eBaseItemType_HangingItem,	eMaterial_glass)->setIconName(L"frame")->setDescriptionId(IDS_ITEM_ITEMFRAME)->setUseDescriptionId(IDS_DESC_ITEMFRAME);


	// TU12
	Item::skull =				(new SkullItem(141))												->setIconName(L"skull")->setDescriptionId(IDS_ITEM_SKULL)->setUseDescriptionId(IDS_DESC_SKULL);

	// TU14
	//Item::writingBook = (new WritingBookItem(130))->setIcon(11, 11)->setDescriptionId("writingBook");
	//Item::writtenBook = (new WrittenBookItem(131))->setIcon(12, 11)->setDescriptionId("writtenBook");

	Item::emerald =				(new Item(132))														->setBaseItemTypeAndMaterial(eBaseItemType_treasure, eMaterial_emerald)->setIconName(L"emerald")->setDescriptionId(IDS_ITEM_EMERALD)->setUseDescriptionId(IDS_DESC_EMERALD);

	Item::flowerPot = (new TilePlanterItem(134, Tile::flowerPot))									->setIconName(L"flowerPot")->setDescriptionId(IDS_FLOWERPOT)->setUseDescriptionId(IDS_DESC_FLOWERPOT);

	Item::carrots = (new SeedFoodItem(135, 4, FoodConstants::FOOD_SATURATION_NORMAL, Tile::carrots_Id, Tile::farmland_Id))	->setIconName(L"carrots")->setDescriptionId(IDS_CARROTS)->setUseDescriptionId(IDS_DESC_CARROTS);
	Item::potato = (new SeedFoodItem(136, 1, FoodConstants::FOOD_SATURATION_LOW, Tile::potatoes_Id, Tile::farmland_Id))		->setIconName(L"potato")->setDescriptionId(IDS_POTATO)->setUseDescriptionId(IDS_DESC_POTATO);
	Item::potatoBaked = (new FoodItem(137, 6, FoodConstants::FOOD_SATURATION_NORMAL, false))								->setIconName(L"potatoBaked")->setDescriptionId(IDS_ITEM_POTATO_BAKED)->setUseDescriptionId(IDS_DESC_POTATO_BAKED);
	Item::potatoPoisonous = (new FoodItem(138, 2, FoodConstants::FOOD_SATURATION_LOW, false))								->setEatEffect(MobEffect::poison->id, 5, 0, .6f)->setIconName(L"potatoPoisonous")->setDescriptionId(IDS_ITEM_POTATO_POISONOUS)->setUseDescriptionId(IDS_DESC_POTATO_POISONOUS);

	Item::emptyMap = (EmptyMapItem *) (new EmptyMapItem(139))->setIconName(L"map_empty")->setDescriptionId(IDS_ITEM_MAP_EMPTY)->setUseDescriptionId(IDS_DESC_MAP_EMPTY);

	Item::carrotGolden = (new FoodItem(140, 6, FoodConstants::FOOD_SATURATION_SUPERNATURAL, false))			->setBaseItemTypeAndMaterial(eBaseItemType_giltFruit,	eMaterial_carrot)->setIconName(L"carrotGolden")->setPotionBrewingFormula(PotionBrewing::MOD_GOLDENCARROT)->setDescriptionId(IDS_ITEM_CARROT_GOLDEN)->setUseDescriptionId(IDS_DESC_CARROT_GOLDEN);

	Item::carrotOnAStick = (new CarrotOnAStickItem(142))													->setBaseItemTypeAndMaterial(eBaseItemType_rod, eMaterial_carrot)->setIconName(L"carrotOnAStick")->setDescriptionId(IDS_ITEM_CARROT_ON_A_STICK)->setUseDescriptionId(IDS_DESC_CARROT_ON_A_STICK);
	Item::netherStar = (new SimpleFoiledItem(143))													->setIconName(L"nether_star")->setDescriptionId(IDS_NETHER_STAR)->setUseDescriptionId(IDS_DESC_NETHER_STAR);
	Item::pumpkinPie = (new FoodItem(144, 8, FoodConstants::FOOD_SATURATION_LOW, false))			->setIconName(L"pumpkinPie")->setDescriptionId(IDS_ITEM_PUMPKIN_PIE)->setUseDescriptionId(IDS_DESC_PUMPKIN_PIE);
	Item::fireworks = (new FireworksItem(145))														->setBaseItemTypeAndMaterial(Item::eBaseItemType_fireworks,	Item::eMaterial_undefined)->setIconName(L"fireworks")->setDescriptionId(IDS_FIREWORKS)->setUseDescriptionId(IDS_DESC_FIREWORKS);
	Item::fireworksCharge = (new FireworksChargeItem(146))											->setBaseItemTypeAndMaterial(Item::eBaseItemType_fireworks,	Item::eMaterial_undefined)->setIconName(L"fireworks_charge")->setDescriptionId(IDS_FIREWORKS_CHARGE)->setUseDescriptionId(IDS_DESC_FIREWORKS_CHARGE);
	EnchantedBookItem::enchantedBook = (EnchantedBookItem *)(new EnchantedBookItem(147))			->setMaxStackSize(1)->setIconName(L"enchantedBook")->setDescriptionId(IDS_ITEM_ENCHANTED_BOOK)->setUseDescriptionId(IDS_DESC_ENCHANTED_BOOK);
	Item::comparator = (new TilePlanterItem(148, Tile::comparator_off))								->setIconName(L"comparator")->setDescriptionId(IDS_ITEM_COMPARATOR)->setUseDescriptionId(IDS_DESC_COMPARATOR);
	Item::netherbrick =	(new Item(149))																->setIconName(L"netherbrick")->setDescriptionId(IDS_ITEM_NETHERBRICK)->setUseDescriptionId(IDS_DESC_ITEM_NETHERBRICK);
	Item::netherQuartz = (new Item(150))															->setIconName(L"netherquartz")->setDescriptionId(IDS_ITEM_NETHER_QUARTZ)->setUseDescriptionId(IDS_DESC_NETHER_QUARTZ);
	Item::minecart_tnt = (new MinecartItem(151, Minecart::TYPE_TNT))								->setIconName(L"minecart_tnt")->setDescriptionId(IDS_ITEM_MINECART_TNT)->setUseDescriptionId(IDS_DESC_MINECART_TNT);
	Item::minecart_hopper = (new MinecartItem(152, Minecart::TYPE_HOPPER))							->setIconName(L"minecart_hopper")->setDescriptionId(IDS_ITEM_MINECART_HOPPER)->setUseDescriptionId(IDS_DESC_MINECART_HOPPER);

	Item::horseArmorMetal = (new Item(161))															->setIconName(L"iron_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_IRON_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_IRON_HORSE_ARMOR);
	Item::horseArmorGold = (new Item(162))															->setIconName(L"gold_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_GOLD_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_GOLD_HORSE_ARMOR);
	Item::horseArmorDiamond = (new Item(163))														->setIconName(L"diamond_horse_armor")->setMaxStackSize(1)->setDescriptionId(IDS_ITEM_DIAMOND_HORSE_ARMOR)->setUseDescriptionId(IDS_DESC_DIAMOND_HORSE_ARMOR);
	Item::lead = (new LeashItem(164))																->setBaseItemTypeAndMaterial(eBaseItemType_pockettool,	eMaterial_undefined)->setIconName(L"lead")->setDescriptionId(IDS_ITEM_LEAD)->setUseDescriptionId(IDS_DESC_LEAD);
	Item::nameTag = (new NameTagItem(165))															->setIconName(L"name_tag")->setDescriptionId(IDS_ITEM_NAME_TAG)->setUseDescriptionId(IDS_DESC_NAME_TAG);}


// 4J Stu - We need to do this after the staticCtor AND after staticCtors for other class
// eg Recipes
void Item::staticInit()
{
	Stats::buildItemStats();
}


_Tier::Tier(int level, int uses, float speed, float damage, int enchantmentValue) :
level( level ),
	uses( uses ),
	speed( speed ),
	damage( damage ),
	enchantmentValue( enchantmentValue )
{
}


int _Tier::getUses() const
{
	return uses;
}

float _Tier::getSpeed() const
{
	return speed;
}

float _Tier::getAttackDamageBonus() const
{
	return damage;
}

int _Tier::getLevel() const
{
	return level;
}

int _Tier::getEnchantmentValue() const
{
	return enchantmentValue;
}

int _Tier::getTierItemId() const
{
	if (this == Tier::WOOD)
	{
		return Tile::wood_Id;
	}
	else if (this == Tier::STONE)
	{
		return Tile::cobblestone_Id;
	}
	else if (this == Tier::GOLD)
	{
		return Item::goldIngot_Id;
	}
	else if (this == Tier::IRON)
	{
		return Item::ironIngot_Id;
	}
	else if (this == Tier::DIAMOND)
	{
		return Item::diamond_Id;
	}
	return 0;
}

Item::Item(int id) : id( 256 + id )
{
	maxStackSize = Item::MAX_STACK_SIZE;
	maxDamage = 0;
	icon = NULL;
	m_handEquipped = false;
	m_isStackedByData = false;

	craftingRemainingItem = NULL;
	potionBrewingFormula = L"";

	m_iMaterial=eMaterial_undefined;
	m_iBaseItemType=eBaseItemType_undefined;
	m_textureName = L"";

	// TODO Init this string
	//string descriptionId;

	//this->id = 256 + id;
	if (items[256 + id] != NULL)
	{
		app.DebugPrintf("CONFLICT @ %d" , id);
	}

	items[256 + id] = this;
}

// 4J-PB - adding so we can class different items together for the new crafting menu
// so pickaxe_stone would get tagged with pickaxe and stone
Item *Item::setBaseItemTypeAndMaterial(int iType,int iMaterial)
{
	this->m_iBaseItemType = iType;
	this->m_iMaterial = iMaterial;
	return this;
}

int Item::getBaseItemType()
{
	return this->m_iBaseItemType;
}

int Item::getMaterial()
{
	return this->m_iMaterial;
}

Item *Item::setIconName(const wstring &name)
{
	m_textureName = name;

	return this;
}

wstring Item::getIconName()
{
	return m_textureName;
}

Item *Item::setMaxStackSize(int max)
{
	maxStackSize = max;
	return this;
}

int Item::getIconType()
{
	return Icon::TYPE_ITEM;
}

Icon *Item::getIcon(int auxValue)
{
	return icon;
}

Icon *Item::getIcon(shared_ptr<ItemInstance> itemInstance)
{
	return getIcon(itemInstance->getAuxValue());
}

bool Item::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	return false;
}

float Item::getDestroySpeed(shared_ptr<ItemInstance> itemInstance, Tile *tile)
{
	return 1;
}

bool Item::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return false;
}

shared_ptr<ItemInstance> Item::use(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return itemInstance;
}

shared_ptr<ItemInstance> Item::useTimeDepleted(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return itemInstance;
}

int Item::getMaxStackSize() const
{
	return maxStackSize;
}

int Item::getLevelDataForAuxValue(int auxValue)
{
	return 0;
}

bool Item::isStackedByData()
{
	return m_isStackedByData;
}


Item *Item::setStackedByData(bool isStackedByData)
{
	this->m_isStackedByData = isStackedByData;
	return this;
}


int Item::getMaxDamage()
{
	return maxDamage;
}


Item *Item::setMaxDamage(int maxDamage)
{
	this->maxDamage = maxDamage;
	return this;
}


bool Item::canBeDepleted()
{
	return maxDamage > 0 && !m_isStackedByData;
}

/**
* Returns true when the item was used to deal more than default damage
*
* @param itemInstance
* @param mob
* @param attacker
* @return
*/
bool Item::hurtEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<LivingEntity> mob, shared_ptr<LivingEntity> attacker)
{
	return false;
}

/**
* Returns true when the item was used to mine more efficiently
*
* @param itemInstance
* @param tile
* @param x
* @param y
* @param z
* @param owner
* @return
*/
bool Item::mineBlock(shared_ptr<ItemInstance> itemInstance, Level *level, int tile, int x, int y, int z, shared_ptr<LivingEntity> owner)
{
	return false;
}

int Item::getAttackDamage(shared_ptr<Entity> entity)
{
	return 1;
}

bool Item::canDestroySpecial(Tile *tile)
{
	return false;
}

bool Item::interactEnemy(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, shared_ptr<LivingEntity> mob)
{
	return false;
}

Item *Item::handEquipped()
{
	m_handEquipped = true;
	return this;
}

bool Item::isHandEquipped()
{
	return m_handEquipped;
}

bool Item::isMirroredArt()
{
	return false;
}

Item *Item::setDescriptionId(unsigned int id)
{
	this->descriptionId = id;
	return this;
}

LPCWSTR Item::getDescription()
{
	return app.GetString(getDescriptionId());
	//return I18n::get(getDescriptionId());
}

LPCWSTR Item::getDescription(shared_ptr<ItemInstance> instance)
{
	return app.GetString(getDescriptionId(instance));
	//return I18n::get(getDescriptionId(instance));
}

unsigned int Item::getDescriptionId(int iData /*= -1*/)
{
	return descriptionId;
}

unsigned int Item::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	return descriptionId;
}

Item *Item::setUseDescriptionId(unsigned int id)
{
	this->useDescriptionId = id;
	return this;
}

unsigned int Item::getUseDescriptionId()
{
	return useDescriptionId;
}

unsigned int Item::getUseDescriptionId(shared_ptr<ItemInstance> instance)
{
	return useDescriptionId;
}

Item *Item::setCraftingRemainingItem(Item *craftingRemainingItem)
{
	this->craftingRemainingItem = craftingRemainingItem;
	return this;
}

bool Item::shouldMoveCraftingResultToInventory(shared_ptr<ItemInstance> instance)
{
	// Default is good for the vast majority of items
	return true;
}

bool Item::shouldOverrideMultiplayerNBT()
{
	return true;
}

Item *Item::getCraftingRemainingItem()
{
	return craftingRemainingItem;
}

bool Item::hasCraftingRemainingItem()
{
	return craftingRemainingItem != NULL;
}

wstring Item::getName()
{
	return L"";//I18n::get(getDescriptionId() + L".name");
}

int Item::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	return 0xffffff;
}

void Item::inventoryTick(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Entity> owner, int slot, bool selected) {
}

void Item::onCraftedBy(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
}

bool Item::isComplex()
{
	return false;
}

UseAnim Item::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_none;
}

int Item::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 0;
}

void Item::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
}

Item *Item::setPotionBrewingFormula(const wstring &potionBrewingFormula)
{
	this->potionBrewingFormula = potionBrewingFormula;
	return this;
}

wstring Item::getPotionBrewingFormula()
{
	return potionBrewingFormula;
}

bool Item::hasPotionBrewingFormula()
{
	return !potionBrewingFormula.empty();
}

void Item::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
}

wstring Item::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	//String elementName = ("" + Language.getInstance().getElementName(getDescription(itemInstance))).trim();
	//return elementName;
	return app.GetString(getDescriptionId(itemInstance));
}

bool Item::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->isEnchanted()) return true;
	return false;
}

const Rarity *Item::getRarity(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->isEnchanted()) return Rarity::rare;
	return Rarity::common;
}

bool Item::isEnchantable(shared_ptr<ItemInstance> itemInstance)
{
	return getMaxStackSize() == 1 && canBeDepleted();
}

HitResult *Item::getPlayerPOVHitResult(Level *level, shared_ptr<Player> player, bool alsoPickLiquid)
{
	float a = 1;

	float xRot = player->xRotO + (player->xRot - player->xRotO) * a;
	float yRot = player->yRotO + (player->yRot - player->yRotO) * a;


	double x = player->xo + (player->x - player->xo) * a;
	double y = player->yo + (player->y - player->yo) * a + 1.62 - player->heightOffset;
	double z = player->zo + (player->z - player->zo) * a;

	Vec3 *from = Vec3::newTemp(x, y, z);

	float yCos = (float) cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = (float) sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = (float) -cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = (float) sin(-xRot * Mth::RAD_TO_GRAD);

	float xa = ySin * xCos;
	float ya = xSin;
	float za = yCos * xCos;

	double range = 5;
	Vec3 *to = from->add(xa * range, ya * range, za * range);
	return level->clip(from, to, alsoPickLiquid, !alsoPickLiquid);
}

int Item::getEnchantmentValue()
{
	return 0;
}

bool Item::hasMultipleSpriteLayers()
{
	return false;
}

Icon *Item::getLayerIcon(int auxValue, int spriteLayer)
{
	return getIcon(auxValue);
}

bool Item::mayBePlacedInAdventureMode()
{
	return true;
}

bool Item::isValidRepairItem(shared_ptr<ItemInstance> source, shared_ptr<ItemInstance> repairItem)
{
	return false;
}

void Item::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(m_textureName);
}

attrAttrModMap *Item::getDefaultAttributeModifiers()
{
	return new attrAttrModMap();
}


/*
	4J: These are necesary on the PS3.
		(and 4 and Vita).
*/
#if (defined __PS3__ || defined __ORBIS__ || defined __PSVITA__)
const int Item::shovel_iron_Id		;
const int Item::pickAxe_iron_Id		;
const int Item::hatchet_iron_Id		;
const int Item::flintAndSteel_Id	;
const int Item::apple_Id			;
const int Item::bow_Id				;
const int Item::arrow_Id			;
const int Item::coal_Id				;
const int Item::diamond_Id			;
const int Item::ironIngot_Id		;
const int Item::goldIngot_Id		;
const int Item::sword_iron_Id		;
const int Item::sword_wood_Id		;
const int Item::shovel_wood_Id		;
const int Item::pickAxe_wood_Id		;
const int Item::hatchet_wood_Id		;
const int Item::sword_stone_Id		;
const int Item::shovel_stone_Id		;
const int Item::pickAxe_stone_Id	;
const int Item::hatchet_stone_Id	;
const int Item::sword_diamond_Id	;
const int Item::shovel_diamond_Id	;
const int Item::pickAxe_diamond_Id	;
const int Item::hatchet_diamond_Id	;
const int Item::stick_Id			;
const int Item::bowl_Id				;
const int Item::mushroomStew_Id		;
const int Item::sword_gold_Id		;
const int Item::shovel_gold_Id		;
const int Item::pickAxe_gold_Id		;
const int Item::hatchet_gold_Id		;
const int Item::string_Id			;
const int Item::feather_Id			;
const int Item::gunpowder_Id		;
const int Item::hoe_wood_Id			;
const int Item::hoe_stone_Id		;
const int Item::hoe_iron_Id			;
const int Item::hoe_diamond_Id		;
const int Item::hoe_gold_Id			;
const int Item::seeds_wheat_Id		;
const int Item::wheat_Id			;
const int Item::bread_Id			;
const int Item::helmet_leather_Id		;
const int Item::chestplate_leather_Id	;
const int Item::leggings_leather_Id	;
const int Item::boots_leather_Id		;
const int Item::helmet_chain_Id		;
const int Item::chestplate_chain_Id	;
const int Item::leggings_chain_Id	;
const int Item::boots_chain_Id		;
const int Item::helmet_iron_Id		;
const int Item::chestplate_iron_Id	;
const int Item::leggings_iron_Id	;
const int Item::boots_iron_Id		;
const int Item::helmet_diamond_Id	;
const int Item::chestplate_diamond_Id;
const int Item::leggings_diamond_Id	;
const int Item::boots_diamond_Id	;
const int Item::helmet_gold_Id		;
const int Item::chestplate_gold_Id	;
const int Item::leggings_gold_Id	;
const int Item::boots_gold_Id		;
const int Item::flint_Id			;
const int Item::porkChop_raw_Id		;
const int Item::porkChop_cooked_Id	;
const int Item::painting_Id			;
const int Item::apple_gold_Id		;
const int Item::sign_Id				;
const int Item::door_wood_Id			;
const int Item::bucket_empty_Id		;
const int Item::bucket_water_Id		;
const int Item::bucket_lava_Id		;
const int Item::minecart_Id			;
const int Item::saddle_Id			;
const int Item::door_iron_Id			;
const int Item::redStone_Id			;
const int Item::snowBall_Id			;
const int Item::boat_Id				;
const int Item::leather_Id			;
const int Item::bucket_milk_Id				;
const int Item::brick_Id				;
const int Item::clay_Id				;
const int Item::reeds_Id				;
const int Item::paper_Id				;
const int Item::book_Id				;
const int Item::slimeBall_Id			;
const int Item::minecart_chest_Id	;
const int Item::minecart_furnace_Id	;
const int Item::egg_Id				;
const int Item::compass_Id			;
const int Item::fishingRod_Id		;
const int Item::clock_Id				;
const int Item::yellowDust_Id		;
const int Item::fish_raw_Id			;
const int Item::fish_cooked_Id		;
const int Item::dye_powder_Id		;
const int Item::bone_Id				;
const int Item::sugar_Id				;
const int Item::cake_Id				;
const int Item::bed_Id				;
const int Item::repeater_Id				;
const int Item::cookie_Id			;
const int Item::map_Id				;
const int Item::shears_Id			;
const int Item::melon_Id				;
const int Item::seeds_pumpkin_Id		;
const int Item::seeds_melon_Id		;
const int Item::beef_raw_Id			;
const int Item::beef_cooked_Id		;
const int Item::chicken_raw_Id		;
const int Item::chicken_cooked_Id	;
const int Item::rotten_flesh_Id		;
const int Item::enderPearl_Id		;
const int Item::blazeRod_Id			;
const int Item::ghastTear_Id			;
const int Item::goldNugget_Id		;
const int Item::netherwart_seeds_Id;
const int Item::potion_Id			;
const int Item::glassBottle_Id		;
const int Item::spiderEye_Id			;
const int Item::fermentedSpiderEye_Id;
const int Item::blazePowder_Id		;
const int Item::magmaCream_Id		;
const int Item::brewingStand_Id		;
const int Item::cauldron_Id			;
const int Item::eyeOfEnder_Id		;
const int Item::speckledMelon_Id		;
const int Item::spawnEgg_Id;
const int Item::expBottle_Id			;
const int Item::skull_Id				;
const int Item::record_01_Id			;
const int Item::record_02_Id			;
const int Item::record_03_Id			;
const int Item::record_04_Id			;
const int Item::record_05_Id			;
const int Item::record_06_Id			;
const int Item::record_07_Id			;
const int Item::record_09_Id			;
const int Item::record_10_Id		    ;
const int Item::record_11_Id		    ;
const int Item::record_12_Id			;
const int Item::record_08_Id			;
const int Item::fireball_Id			;
const int Item::itemFrame_Id			;
const int Item::netherbrick_Id		;
const int Item::emerald_Id			;
const int Item::flowerPot_Id			;
const int Item::carrots_Id			;
const int Item::potato_Id			;
const int Item::potatoBaked_Id		;
const int Item::potatoPoisonous_Id	;
const int Item::carrotGolden_Id		;
const int Item::carrotOnAStick_Id	;
const int Item::pumpkinPie_Id		;
const int Item::enchantedBook_Id		;
const int Item::netherQuartz_Id		;
#endif
