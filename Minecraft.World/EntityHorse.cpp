#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.phys.h"
#include "..\Minecraft.Client\Textures.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "BasicTypeContainers.h"
#include "EntityHorse.h"

const wstring EntityHorse::TEX_FOLDER = L"mob/horse/";

const EntitySelector *EntityHorse::PARENT_HORSE_SELECTOR = new HorseEntitySelector();

Attribute *EntityHorse::JUMP_STRENGTH = (new RangedAttribute(eAttributeId_HORSE_JUMPSTRENGTH, .7, 0, 2.0))->setSyncable(true);

wstring EntityHorse::ARMOR_TEXTURES[EntityHorse::ARMORS] = {L"", L"armor/horse_armor_iron.png", L"armor/horse_armor_gold.png", L"armor/horse_armor_diamond.png"};
int EntityHorse::ARMOR_TEXTURES_ID[EntityHorse::ARMORS] = {-1, TN_MOB_HORSE_ARMOR_IRON, TN_MOB_HORSE_ARMOR_GOLD, TN_MOB_HORSE_ARMOR_DIAMOND };
wstring EntityHorse::ARMOR_HASHES[EntityHorse::ARMORS] = {L"", L"meo", L"goo", L"dio"};
int EntityHorse::ARMOR_PROTECTION[EntityHorse::ARMORS] = {0, 5, 7, 11};

wstring EntityHorse::VARIANT_TEXTURES[EntityHorse::VARIANTS] = {L"horse_white.png", L"horse_creamy.png", L"horse_chestnut.png", L"horse_brown.png",  L"horse_black.png", L"horse_gray.png", L"horse_darkbrown.png"};
int EntityHorse::VARIANT_TEXTURES_ID[EntityHorse::VARIANTS] = {TN_MOB_HORSE_WHITE, TN_MOB_HORSE_CREAMY, TN_MOB_HORSE_CHESTNUT, TN_MOB_HORSE_BROWN,  TN_MOB_HORSE_BLACK, TN_MOB_HORSE_GRAY, TN_MOB_HORSE_DARKBROWN};

wstring EntityHorse::VARIANT_HASHES[EntityHorse::VARIANTS] = {L"hwh", L"hcr", L"hch", L"hbr", L"hbl", L"hgr", L"hdb"};

wstring EntityHorse::MARKING_TEXTURES[EntityHorse::MARKINGS] = {L"", L"horse_markings_white.png", L"horse_markings_whitefield.png", L"horse_markings_whitedots.png", L"horse_markings_blackdots.png"};
int EntityHorse::MARKING_TEXTURES_ID[EntityHorse::MARKINGS] = {-1, TN_MOB_HORSE_MARKINGS_WHITE, TN_MOB_HORSE_MARKINGS_WHITEFIELD, TN_MOB_HORSE_MARKINGS_WHITEDOTS, TN_MOB_HORSE_MARKINGS_BLACKDOTS};
wstring EntityHorse::MARKING_HASHES[EntityHorse::MARKINGS] = {L"", L"wo_", L"wmo", L"wdo", L"bdo"};

bool HorseEntitySelector::matches(shared_ptr<Entity> entity) const
{
	return entity->instanceof(eTYPE_HORSE) && dynamic_pointer_cast<EntityHorse>(entity)->isBred();
}

EntityHorse::EntityHorse(Level *level) : Animal(level)
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	countEating = 0;
	mouthCounter = 0;
	standCounter = 0;
	tailCounter = 0;
	sprintCounter = 0;
	isEntityJumping = false;
	inventory = nullptr;
	hasReproduced = false;
	temper = 0;
	playerJumpPendingScale = 0.0f;
	allowStandSliding = false;
	eatAnim = eatAnimO = 0.0f;
	standAnim = standAnimO = 0.0f;
	mouthAnim = mouthAnimO = 0.0f;
	gallopSoundCounter = 0;

	layerTextureHashName = L"";

	layerTextureLayers = intArray(3);
	for(unsigned int i = 0; i < 3; ++i)
	{
		layerTextureLayers[i] = -1;
	}

	setSize(1.4f, 1.6f);
	fireImmune = false;
	setChestedHorse(false);

	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 1.2));
	goalSelector.addGoal(1, new RunAroundLikeCrazyGoal(this, 1.2));
	goalSelector.addGoal(2, new BreedGoal(this, 1.0));
	goalSelector.addGoal(4, new FollowParentGoal(this, 1.0));
	goalSelector.addGoal(6, new RandomStrollGoal(this, .7));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));

	createInventory();
}

EntityHorse::~EntityHorse()
{
	delete [] layerTextureLayers.data;
}

void EntityHorse::defineSynchedData()
{
	Animal::defineSynchedData();
	entityData->define(DATA_ID_HORSE_FLAGS, 0);
	entityData->define(DATA_ID_TYPE, static_cast<byte>(0));
	entityData->define(DATA_ID_TYPE_VARIANT, 0);
	entityData->define(DATA_ID_OWNER_NAME, L"");
	entityData->define(DATA_ID_ARMOR, 0);
}

void EntityHorse::setType(int i)
{
	entityData->set(DATA_ID_TYPE, static_cast<byte>(i));
	clearLayeredTextureInfo();
}

int EntityHorse::getType()
{
	return entityData->getByte(DATA_ID_TYPE);
}

void EntityHorse::setVariant(int i)
{
	entityData->set(DATA_ID_TYPE_VARIANT, i);
	clearLayeredTextureInfo();
}

int EntityHorse::getVariant()
{
	return entityData->getInteger(DATA_ID_TYPE_VARIANT);
}

wstring EntityHorse::getAName()
{
	if (hasCustomName()) return getCustomName();
#ifdef _DEBUG
	int type = getType();
	switch (type)
	{
	default:
	case TYPE_HORSE:
		return L"entity.horse.name";
	case TYPE_DONKEY:
		return L"entity.donkey.name";
	case TYPE_MULE:
		return L"entity.mule.name";
	case TYPE_SKELETON:
		return L"entity.skeletonhorse.name";
	case TYPE_UNDEAD:
		return L"entity.zombiehorse.name";
	}
#else
	return L"";
#endif
}

bool EntityHorse::getHorseFlag(int flag)
{
	return (entityData->getInteger(DATA_ID_HORSE_FLAGS) & flag) != 0;
}

void EntityHorse::setHorseFlag(int flag, bool value)
{
	int current = entityData->getInteger(DATA_ID_HORSE_FLAGS);
	if (value)
	{
		entityData->set(DATA_ID_HORSE_FLAGS, current | flag);
	}
	else
	{
		entityData->set(DATA_ID_HORSE_FLAGS, current & ~flag);
	}
}

bool EntityHorse::isAdult()
{
	return !isBaby();
}

bool EntityHorse::isTamed()
{
	return getHorseFlag(FLAG_TAME);
}

bool EntityHorse::isRidable()
{
	return isAdult();
}

wstring EntityHorse::getOwnerName()
{
	return entityData->getString(DATA_ID_OWNER_NAME);
}

void EntityHorse::setOwner(const wstring &par1Str)
{
	entityData->set(DATA_ID_OWNER_NAME, par1Str);
}

float EntityHorse::getFoalScale()
{
	int age = getAge();
	if (age >= 0)
	{
		return 1.0f;
	}
	return .5f + static_cast<float>(BABY_START_AGE - age) / static_cast<float>(BABY_START_AGE) * .5f;
}


void EntityHorse::updateSize(bool isBaby)
{
	if (isBaby)
	{
		internalSetSize(getFoalScale());
	}
	else
	{
		internalSetSize(1.0f);
	}
}

bool EntityHorse::getIsJumping()
{
	return isEntityJumping;
}

void EntityHorse::setTamed(bool flag)
{
	setHorseFlag(FLAG_TAME, flag);
}

void EntityHorse::setIsJumping(bool flag)
{
	isEntityJumping = flag;
}


bool EntityHorse::canBeLeashed()
{
	return !isUndead() && Animal::canBeLeashed();
}

void EntityHorse::onLeashDistance(float distanceToLeashHolder)
{
	if (distanceToLeashHolder > 6 && isEating())
	{
		setEating(false);
	}
}

bool EntityHorse::isChestedHorse()
{
	return getHorseFlag(FLAG_CHESTED);
}

int EntityHorse::getArmorType()
{
	return entityData->getInteger(DATA_ID_ARMOR);
}

int EntityHorse::getArmorTypeForItem(shared_ptr<ItemInstance> armorItem)
{
	if (armorItem == nullptr)
	{
		return ARMOR_NONE;
	}
	if (armorItem->id == Item::horseArmorMetal_Id)
	{
		return ARMOR_IRON;
	}
	else if (armorItem->id == Item::horseArmorGold_Id)
	{
		return ARMOR_GOLD;
	}
	else if (armorItem->id == Item::horseArmorDiamond_Id)
	{
		return ARMOR_DIAMOND;
	}
	return ARMOR_NONE;
}

bool EntityHorse::isEating()
{
	return getHorseFlag(FLAG_EATING);
}

bool EntityHorse::isStanding()
{
	return getHorseFlag(FLAG_STANDING);
}

bool EntityHorse::isBred()
{
	return getHorseFlag(FLAG_BRED);
}

bool EntityHorse::getHasReproduced()
{
	return hasReproduced;
}

void EntityHorse::setArmorType(int i)
{
	entityData->set(DATA_ID_ARMOR, i);
	clearLayeredTextureInfo();
}

void EntityHorse::setBred(bool flag)
{
	setHorseFlag(FLAG_BRED, flag);

}

void EntityHorse::setChestedHorse(bool flag)
{
	setHorseFlag(FLAG_CHESTED, flag);
}

void EntityHorse::setReproduced(bool flag)
{
	hasReproduced = flag;
}

void EntityHorse::setSaddled(bool flag)
{
	setHorseFlag(FLAG_SADDLE, flag);
}

int EntityHorse::getTemper()
{
	return temper;
}

void EntityHorse::setTemper(int temper)
{
	this->temper = temper;
}

int EntityHorse::modifyTemper(int amount)
{
	int temper = Mth::clamp(getTemper() + amount, 0, getMaxTemper());

	setTemper(temper);
	return temper;
}


bool EntityHorse::hurt(DamageSource *damagesource, float dmg)
{
	// 4J: Protect owned horses from untrusted players
	if (isTamed())
	{
		shared_ptr<Entity> entity = damagesource->getDirectEntity();
		if (entity != nullptr && entity->instanceof(eTYPE_PLAYER))
		{
			shared_ptr<Player> attacker = dynamic_pointer_cast<Player>(entity);
			attacker->canHarmPlayer(getOwnerName());
		}
	}

	shared_ptr<Entity> attacker = damagesource->getEntity();
	if (rider.lock() != nullptr && (rider.lock() == (attacker) ))
	{
		return false;
	}

	return Animal::hurt(damagesource, dmg);
}


int EntityHorse::getArmorValue()
{
	return ARMOR_PROTECTION[getArmorType()];
}


bool EntityHorse::isPushable()
{
	return rider.lock() == nullptr;
}

// TODO: [EB]: Explain why this is being done - what side effect does getBiome have?
bool EntityHorse::checkSpawningBiome()
{
	int x = Mth::floor(this->x);
	int z = Mth::floor(this->z);

	level->getBiome(x, z);
	return true;
}

/**
* Drops a chest block if the horse is bagged
*/
void EntityHorse::dropBags()
{
	if (level->isClientSide || !isChestedHorse())
	{
		return;
	}

	spawnAtLocation(Tile::chest_Id, 1);
	setChestedHorse(false);
}

void EntityHorse::eatingHorse()
{
	openMouth();
	level->playEntitySound(shared_from_this(), eSoundType_EATING, 1.0f, 1.0f + (random->nextFloat() - random->nextFloat()) * 0.2f);
}

/**
* Changed to adjust fall damage for riders
*/
void EntityHorse::causeFallDamage(float fallDistance)
{

	if (fallDistance > 1)
	{
		playSound(eSoundType_MOB_HORSE_LAND, .4f, 1);
	}

	int dmg = Mth::ceil(fallDistance * .5f - 3.0f);
	if (dmg <= 0) return;

	hurt(DamageSource::fall, dmg);

	if (rider.lock() != nullptr)
	{
		rider.lock()->hurt(DamageSource::fall, dmg);
	}

	int id = level->getTile(Mth::floor(x), Mth::floor(y - 0.2 - yRotO), Mth::floor(z));
	if (id > 0)
	{
		const Tile::SoundType *stepsound = Tile::tiles[id]->soundType;
		level->playEntitySound(shared_from_this(), stepsound->getStepSound(), stepsound->getVolume() * 0.5f, stepsound->getPitch() * 0.75f);
	}
}


/**
* Different inventory sizes depending on the kind of horse
*
* @return
*/
int EntityHorse::getInventorySize()
{
	int type = getType();
	if (isChestedHorse() && (type == TYPE_DONKEY || type == TYPE_MULE))
	{
		return INV_BASE_COUNT + INV_DONKEY_CHEST_COUNT;
	}
	return INV_BASE_COUNT;
}

void EntityHorse::createInventory()
{
	shared_ptr<AnimalChest> old = inventory;
	inventory = std::make_shared<AnimalChest>(L"HorseChest", getInventorySize());
	inventory->setCustomName(getAName());
	if (old != nullptr)
	{
		old->removeListener(this);

		int max = min(old->getContainerSize(), inventory->getContainerSize());
		for (int slot = 0; slot < max; slot++)
		{
			shared_ptr<ItemInstance> item = old->getItem(slot);
			if (item != nullptr)
			{
				inventory->setItem(slot, item->copy());
			}
		}
		old = nullptr;
	}
	inventory->addListener(this);
	updateEquipment();
}

void EntityHorse::updateEquipment()
{
	if (level && !level->isClientSide)
	{
		setSaddled(inventory->getItem(INV_SLOT_SADDLE) != nullptr);
		if (canWearArmor())
		{
			setArmorType(getArmorTypeForItem(inventory->getItem(INV_SLOT_ARMOR)));
		}
	}
}

void EntityHorse::containerChanged()
{
	int armorType = getArmorType();
	bool saddled = isSaddled();
	updateEquipment();
	if (tickCount > 20)
	{
		if (armorType == ARMOR_NONE && armorType != getArmorType())
		{
			playSound(eSoundType_MOB_HORSE_ARMOR, .5f, 1);
		}
		if (!saddled && isSaddled())
		{
			playSound(eSoundType_MOB_HORSE_LEATHER, .5f, 1);
		}
	}

}


bool EntityHorse::canSpawn()
{
	checkSpawningBiome();
	return Animal::canSpawn();
}

bool EntityHorse::removeWhenFarAway()
{
	if (isTamed())          return false;
	if (isSaddled())        return false;
	if (isLeashed())        return false;
	if (getArmorType() > 0) return false;
	return Animal::removeWhenFarAway();
}


shared_ptr<EntityHorse> EntityHorse::getClosestMommy(shared_ptr<Entity> baby, double searchRadius)
{
	double closestDistance = Double::MAX_VALUE;

	shared_ptr<Entity> mommy = nullptr;
	vector<shared_ptr<Entity> > *list = level->getEntities(baby, baby->bb->expand(searchRadius, searchRadius, searchRadius), PARENT_HORSE_SELECTOR);

	for( auto& horse : *list )
	{
		double distanceSquared = horse->distanceToSqr(baby->x, baby->y, baby->z);

		if (distanceSquared < closestDistance)
		{
			mommy = horse;
			closestDistance = distanceSquared;
		}
	}
	delete list;

	return dynamic_pointer_cast<EntityHorse>(mommy);
}

double EntityHorse::getCustomJump()
{
	return getAttribute(JUMP_STRENGTH)->getValue();
}

int EntityHorse::getDeathSound()
{
	openMouth();
	int type = getType();
	if (type == TYPE_UNDEAD)
	{
		return eSoundType_MOB_HORSE_ZOMBIE_DEATH; //"mob.horse.zombie.death";
	}
	if (type == TYPE_SKELETON)
	{
		return eSoundType_MOB_HORSE_SKELETON_DEATH; //"mob.horse.skeleton.death";
	}
	if (type == TYPE_DONKEY || type == TYPE_MULE)
	{
		return eSoundType_MOB_HORSE_DONKEY_DEATH; //"mob.horse.donkey.death";
	}
	return eSoundType_MOB_HORSE_DEATH; //"mob.horse.death";
}

int EntityHorse::getDeathLoot()
{
	bool flag = random->nextInt(4) == 0;

	int type = getType();
	if (type == TYPE_SKELETON)
	{
		return Item::bone_Id;
	}
	if (type == TYPE_UNDEAD)
	{
		if (flag)
		{
			return 0;
		}
		return Item::rotten_flesh_Id;
	}

	return Item::leather_Id;
}

int EntityHorse::getHurtSound()
{
	openMouth();
	{
		if (random->nextInt(3) == 0)
		{
			stand();
		}
	}
	int type = getType();
	if (type == TYPE_UNDEAD)
	{
		return eSoundType_MOB_HORSE_ZOMBIE_HIT; //"mob.horse.zombie.hit";
	}
	if (type == TYPE_SKELETON)
	{
		return eSoundType_MOB_HORSE_SKELETON_HIT; //"mob.horse.skeleton.hit";
	}
	if (type == TYPE_DONKEY || type == TYPE_MULE)
	{
		return eSoundType_MOB_HORSE_DONKEY_HIT; //"mob.horse.donkey.hit";
	}
	return eSoundType_MOB_HORSE_HIT; //"mob.horse.hit";
}

bool EntityHorse::isSaddled()
{
	return getHorseFlag(FLAG_SADDLE);
}


int EntityHorse::getAmbientSound()
{
	openMouth();
	if (random->nextInt(10) == 0 && !isImmobile())
	{
		stand();
	}
	int type = getType();
	if (type == TYPE_UNDEAD)
	{
		return eSoundType_MOB_HORSE_ZOMBIE_IDLE; //"mob.horse.zombie.idle";
	}
	if (type == TYPE_SKELETON)
	{
		return eSoundType_MOB_HORSE_SKELETON_IDLE; //"mob.horse.skeleton.idle";
	}
	if (type == TYPE_DONKEY || type == TYPE_MULE)
	{
		return eSoundType_MOB_HORSE_DONKEY_IDLE; //"mob.horse.donkey.idle";
	}
	return eSoundType_MOB_HORSE_IDLE; //"mob.horse.idle";
}

/**
* sound played when an untamed mount buckles rider
*/
int EntityHorse::getMadSound()
{
	openMouth();
	stand();
	int type = getType();
	if (type == TYPE_UNDEAD || type == TYPE_SKELETON)
	{
		return -1;
	}
	if (type == TYPE_DONKEY || type == TYPE_MULE)
	{
		return eSoundType_MOB_HORSE_DONKEY_ANGRY; //"mob.horse.donkey.angry";
	}
	return eSoundType_MOB_HORSE_ANGRY; //"mob.horse.angry";
}

void EntityHorse::playStepSound(int xt, int yt, int zt, int t)
{
	const Tile::SoundType *soundType = Tile::tiles[t]->soundType;
	if (level->getTile(xt, yt + 1, zt) == Tile::topSnow_Id)
	{
		soundType = Tile::topSnow->soundType;
	}
	if (!Tile::tiles[t]->material->isLiquid())
	{
		int type = getType();
		if (rider.lock() != nullptr && type != TYPE_DONKEY && type != TYPE_MULE)
		{
			gallopSoundCounter++;
			if (gallopSoundCounter > 5 && gallopSoundCounter % 3 == 0)
			{
				playSound(eSoundType_MOB_HORSE_GALLOP, soundType->getVolume() * 0.15f, soundType->getPitch());
				if (type == TYPE_HORSE && random->nextInt(10) == 0)
				{
					playSound(eSoundType_MOB_HORSE_BREATHE, soundType->getVolume() * 0.6f, soundType->getPitch());
				}
			}
			else if (gallopSoundCounter <= 5)
			{
				playSound(eSoundType_MOB_HORSE_WOOD, soundType->getVolume() * 0.15f, soundType->getPitch());
			}
		}
		else if (soundType == Tile::SOUND_WOOD)
		{
			playSound(eSoundType_MOB_HORSE_SOFT, soundType->getVolume() * 0.15f, soundType->getPitch());
		}
		else
		{
			playSound(eSoundType_MOB_HORSE_WOOD, soundType->getVolume() * 0.15f, soundType->getPitch());
		}
	}
}

void EntityHorse::registerAttributes()
{
	Animal::registerAttributes();

	getAttributes()->registerAttribute(JUMP_STRENGTH);

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(53);
	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.225f);
}

int EntityHorse::getMaxSpawnClusterSize()
{
	return 6;
}

/**
* How difficult is the creature to be tamed? the Higher the number, the
* more difficult
*/
int EntityHorse::getMaxTemper()
{
	return 100;
}

float EntityHorse::getSoundVolume()
{
	return 0.8f;
}


int EntityHorse::getAmbientSoundInterval()
{
	return 400;
}

bool EntityHorse::hasLayeredTextures()
{
	return getType() == TYPE_HORSE || getArmorType() > 0;
}

void EntityHorse::clearLayeredTextureInfo()
{
	layerTextureHashName = L"";
}

void EntityHorse::rebuildLayeredTextureInfo()
{
	layerTextureHashName = L"horse/";
	layerTextureLayers[0] = -1;
	layerTextureLayers[1] = -1;
	layerTextureLayers[2] = -1;

	int type = getType();
	int variant = getVariant();
	int armorIndex = 2;
	if (type == TYPE_HORSE)
	{
		int skin = variant & 0xFF;
		int markings = (variant & 0xFF00) >> 8;
		layerTextureLayers[0] = VARIANT_TEXTURES_ID[skin];
		layerTextureHashName += VARIANT_HASHES[skin];

		layerTextureLayers[1] = MARKING_TEXTURES_ID[markings];
		layerTextureHashName += MARKING_HASHES[markings];

		if(layerTextureLayers[1] == -1)
		{
			armorIndex = 1;
		}
	}
	else
	{
		layerTextureLayers[0] = -1;
		layerTextureHashName += L"_" + std::to_wstring(type) + L"_";
		armorIndex = 1;
	}

	int armor = getArmorType();
	layerTextureLayers[armorIndex] = ARMOR_TEXTURES_ID[armor];
	layerTextureHashName += ARMOR_HASHES[armor];
}

wstring EntityHorse::getLayeredTextureHashName()
{
	if (layerTextureHashName.empty())
	{
		rebuildLayeredTextureInfo();
	}
	return layerTextureHashName;
}

intArray EntityHorse::getLayeredTextureLayers()
{
	if (layerTextureHashName.empty())
	{
		rebuildLayeredTextureInfo();
	}
	return layerTextureLayers;
}

void EntityHorse::openInventory(shared_ptr<Player> player)
{
	if (!level->isClientSide && (rider.lock() == nullptr || rider.lock() == player) && isTamed())
	{
		inventory->setCustomName(getAName());
		player->openHorseInventory(dynamic_pointer_cast<EntityHorse>(shared_from_this()), inventory);
	}
}

bool EntityHorse::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> itemstack = player->inventory->getSelected();

	if (itemstack != nullptr && itemstack->id == Item::spawnEgg_Id)
	{
		return Animal::mobInteract(player);
	}

	if (!isTamed())
	{
		if (isUndead())
		{
			return false;
		}
	}

	if (isTamed() && isAdult() && player->isSneaking())
	{
		openInventory(player);
		return true;
	}

	if (isRidable() && rider.lock() != nullptr)
	{
		return Animal::mobInteract(player);
	}

	// consumables
	if (itemstack != nullptr)
	{
		bool itemUsed = false;

		if (canWearArmor())
		{
			int armorType = -1;

			if (itemstack->id == Item::horseArmorMetal_Id)
			{
				armorType = ARMOR_IRON;
			}
			else if (itemstack->id == Item::horseArmorGold_Id)
			{
				armorType = ARMOR_GOLD;
			}
			else if (itemstack->id == Item::horseArmorDiamond_Id)
			{
				armorType = ARMOR_DIAMOND;
			}

			if (armorType >= 0)
			{
				if (!isTamed())
				{
					makeMad();
					return true;
				}
				openInventory(player);
				return true;
			}
		}

		if (!itemUsed && !isUndead())
		{
			float _heal = 0;
			int _ageUp = 0;
			int temper = 0;

			if (itemstack->id == Item::wheat_Id)
			{
				_heal = 2;
				_ageUp = 60;
				temper = 3;
			}
			else if (itemstack->id == Item::sugar_Id)
			{
				_heal = 1;
				_ageUp = 30;
				temper = 3;
			}
			else if (itemstack->id == Item::bread_Id)
			{
				_heal = 7;
				_ageUp = 180;
				temper = 3;
			}
			else if (itemstack->id == Tile::hayBlock_Id)
			{
				_heal = 20;
				_ageUp = 180;
			}
			else if (itemstack->id == Item::apple_Id)
			{
				_heal = 3;
				_ageUp = 60;
				temper = 3;
			}
			else if (itemstack->id == Item::carrotGolden_Id)
			{
				_heal = 4;
				_ageUp = 60;
				temper = 5;
				if (isTamed() && getAge() == 0)
				{
					itemUsed = true;
					setInLove();
				}
			}
			else if (itemstack->id == Item::apple_gold_Id)
			{
				_heal = 10;
				_ageUp = 240;
				temper = 10;
				if (isTamed() && getAge() == 0)
				{
					itemUsed = true;
					setInLove();
				}
			}
			if (getHealth() < getMaxHealth() && _heal > 0)
			{
				heal(_heal);
				itemUsed = true;
			}
			if (!isAdult() && _ageUp > 0)
			{
				ageUp(_ageUp);
				itemUsed = true;
			}
			if (temper > 0 && (itemUsed || !isTamed()) && temper < getMaxTemper())
			{
				itemUsed = true;
				modifyTemper(temper);
			}
			if (itemUsed)
			{
				eatingHorse();
			}
		}

		if (!isTamed() && !itemUsed)
		{
			if (itemstack != nullptr && itemstack->interactEnemy(player, dynamic_pointer_cast<LivingEntity>(shared_from_this())))
			{
				return true;
			}
			makeMad();
			return true;
		}

		if (!itemUsed && canWearBags() && !isChestedHorse())
		{
			if (itemstack->id == Tile::chest_Id)
			{
				setChestedHorse(true);
				playSound(eSoundType_MOB_CHICKENPLOP, 1.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
				itemUsed = true;
				createInventory();
			}
		}

		if (!itemUsed && isRidable() && !isSaddled())
		{
			if (itemstack->id == Item::saddle_Id)
			{
				openInventory(player);
				return true;
			}
		}

		if (itemUsed)
		{
			if (!player->abilities.instabuild)
			{
				if (--itemstack->count == 0)
				{
					player->inventory->setItem(player->inventory->selected, nullptr);
				}
			}
			return true;
		}
	}

	if (isRidable() && rider.lock() == nullptr)
	{
		// for name tag items and such, we must call the item's interaction
		// method before riding
		if (itemstack != nullptr && itemstack->interactEnemy(player, dynamic_pointer_cast<LivingEntity>(shared_from_this())))
		{
			return true;
		}
		doPlayerRide(player);

		app.DebugPrintf("<EntityHorse::mobInteract> Horse speed: %f\n", static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));

		return true;
	}
	else
	{
		return Animal::mobInteract(player);
	}
}

void EntityHorse::doPlayerRide(shared_ptr<Player> player)
{
	player->yRot = yRot;
	player->xRot = xRot;
	setEating(false);
	setStanding(false);
	if (!level->isClientSide)
	{
		player->ride(shared_from_this());
	}
}

/**
* Can this horse be trapped in an amulet?
*/
bool EntityHorse::isAmuletHorse()
{
	return getType() == TYPE_SKELETON;
}

/**
* Can wear regular armor
*/
bool EntityHorse::canWearArmor()
{
	return getType() == TYPE_HORSE;
}

/**
* able to carry bags
*
* @return
*/
bool EntityHorse::canWearBags()
{
	int type = getType();
	return type == TYPE_MULE || type == TYPE_DONKEY;
}

bool EntityHorse::isImmobile()
{
	if (rider.lock() != nullptr && isSaddled())
	{
		return true;
	}
	return isEating() || isStanding();
}

/**
* Rare horse that can be transformed into Nightmares or Bathorses or give
* ghost horses on dead
*/
bool EntityHorse::isPureBreed()
{
	return getType() > 10 && getType() < 21;
}

/**
* Is this an Undead Horse?
*
* @return
*/
bool EntityHorse::isUndead()
{
	int type = getType();
	return type == TYPE_UNDEAD || type == TYPE_SKELETON;
}

bool EntityHorse::isSterile()
{
	return isUndead() || getType() == TYPE_MULE;
}


bool EntityHorse::isFood(shared_ptr<ItemInstance> itemInstance)
{
	// horses have their own food behaviors in mobInterract
	return false;
}

void EntityHorse::moveTail()
{
	tailCounter = 1;
}

int EntityHorse::nameYOffset()
{
	if (isAdult())
	{
		return -80;
	}
	else
	{
		return static_cast<int>(-5 - getFoalScale() * 80.0f);
	}
}

void EntityHorse::die(DamageSource *damagesource)
{
	Animal::die(damagesource);
	if (!level->isClientSide)
	{
		dropMyStuff();
	}
}

void EntityHorse::aiStep()
{
	if (random->nextInt(200) == 0)
	{
		moveTail();
	}

	Animal::aiStep();

	if (!level->isClientSide)
	{
		if (random->nextInt(900) == 0 && deathTime == 0)
		{
			heal(1);
		}

		if (!isEating() && rider.lock() == nullptr && random->nextInt(300) == 0)
		{
			if (level->getTile(Mth::floor(x), Mth::floor(y) - 1, Mth::floor(z)) == Tile::grass_Id)
			{
				setEating(true);
			}
		}

		if (isEating() && ++countEating > 50)
		{
			countEating = 0;
			setEating(false);
		}

		if (isBred() && !isAdult() && !isEating())
		{
			shared_ptr<EntityHorse> mommy = getClosestMommy(shared_from_this(), 16);
			if (mommy != nullptr && distanceToSqr(mommy) > 4.0)
			{
				Path *pathentity = level->findPath(shared_from_this(), mommy, 16.0f, true, false, false, true);
				setPath(pathentity);
			}

		}
	}
}

void EntityHorse::tick()
{
	Animal::tick();

	// if client-side data values have changed, rebuild texture info
	if (level->isClientSide && entityData->isDirty())
	{
		entityData->clearDirty();
		clearLayeredTextureInfo();
	}

	if (mouthCounter > 0 && ++mouthCounter > 30)
	{
		mouthCounter = 0;
		setHorseFlag(FLAG_OPEN_MOUTH, false);
	}

	if (!level->isClientSide)
	{
		if (standCounter > 0 && ++standCounter > 20)
		{
			standCounter = 0;
			setStanding(false);
		}
	}

	if (tailCounter > 0 && ++tailCounter > 8)
	{
		tailCounter = 0;
	}

	if (sprintCounter > 0)
	{
		++sprintCounter;

		if (sprintCounter > 300)
		{
			sprintCounter = 0;
		}
	}

	eatAnimO = eatAnim;
	if (isEating())
	{
		eatAnim += (1.0f - eatAnim) * .4f + .05f;
		if (eatAnim > 1)
		{
			eatAnim = 1;
		}
	}
	else
	{
		eatAnim += (.0f - eatAnim) * .4f - .05f;
		if (eatAnim < 0)
		{
			eatAnim = 0;
		}
	}
	standAnimO = standAnim;
	if (isStanding())
	{
		// standing is incompatible with eating, so lock eat anim
		eatAnimO = eatAnim = 0;
		standAnim += (1.0f - standAnim) * .4f + .05f;
		if (standAnim > 1)
		{
			standAnim = 1;
		}
	}
	else
	{
		allowStandSliding = false;
		// the animation falling back to ground is slower in the beginning
		standAnim += (.8f * standAnim * standAnim * standAnim - standAnim) * .6f - .05f;
		if (standAnim < 0)
		{
			standAnim = 0;
		}
	}
	mouthAnimO = mouthAnim;
	if (getHorseFlag(FLAG_OPEN_MOUTH))
	{
		mouthAnim += (1.0f - mouthAnim) * .7f + .05f;
		if (mouthAnim > 1)
		{
			mouthAnim = 1;
		}
	}
	else
	{
		mouthAnim += (.0f - mouthAnim) * .7f - .05f;
		if (mouthAnim < 0)
		{
			mouthAnim = 0;
		}
	}
}

void EntityHorse::openMouth()
{
	if (!level->isClientSide)
	{
		mouthCounter = 1;
		setHorseFlag(FLAG_OPEN_MOUTH, true);
	}
}

bool EntityHorse::isReadyForParenting()
{
	return rider.lock() == nullptr && riding == nullptr && isTamed() && isAdult() && !isSterile() && getHealth() >= getMaxHealth();
}

bool EntityHorse::renderName()
{
	return hasCustomName() && rider.lock() == nullptr;
}

bool EntityHorse::rideableEntity()
{
	return true;
}


void EntityHorse::setUsingItemFlag(bool flag)
{
	setHorseFlag(FLAG_EATING, flag);
}

void EntityHorse::setEating(bool state)
{
	setUsingItemFlag(state);
}

void EntityHorse::setStanding(bool state)
{
	if (state)
	{
		setEating(false);
	}
	setHorseFlag(FLAG_STANDING, state);
}

void EntityHorse::stand()
{
	if (!level->isClientSide)
	{
		standCounter = 1;
		setStanding(true);
	}
}

void EntityHorse::makeMad()
{
	stand();
	int ambient = getMadSound();
	playSound(ambient, getSoundVolume(), getVoicePitch());
}

void EntityHorse::dropMyStuff()
{
	dropInventory(shared_from_this(), inventory);
	dropBags();
}

void EntityHorse::dropInventory(shared_ptr<Entity> entity, shared_ptr<AnimalChest> animalchest)
{
	if (animalchest == nullptr || level->isClientSide) return;

	for (int i = 0; i < animalchest->getContainerSize(); i++)
	{
		shared_ptr<ItemInstance> itemstack = animalchest->getItem(i);
		if (itemstack == nullptr)
		{
			continue;
		}
		spawnAtLocation(itemstack, 0);
	}

}

bool EntityHorse::tameWithName(shared_ptr<Player> player)
{
	setOwner(player->getName());
	setTamed(true);
	return true;
}

/**
* Overridden method to add control to mounts, should be moved to
* EntityLiving
*/
void EntityHorse::travel(float xa, float ya)
{
	// If the entity is not ridden by Player, then execute the normal
	// Entityliving code
	if (rider.lock() == nullptr || !isSaddled())
	{
		footSize = .5f;
		flyingSpeed = .02f;
		Animal::travel(xa, ya);
		return;
	}

	yRotO = yRot = rider.lock()->yRot;
	xRot = rider.lock()->xRot * 0.5f;
	setRot(yRot, xRot);
	yHeadRot = yBodyRot = yRot;

	shared_ptr<LivingEntity> livingRider = dynamic_pointer_cast<LivingEntity>(rider.lock());
	xa = livingRider->xxa * .5f;
	ya = livingRider->yya;

	// move much slower backwards
	if (ya <= 0)
	{
		ya *= .25f;
		gallopSoundCounter = 0;
	}

	if (onGround && playerJumpPendingScale == 0 && isStanding() && !allowStandSliding)
	{
		xa = 0;
		ya = 0;
	}

	if (playerJumpPendingScale > 0 && !getIsJumping() && onGround)
	{
		yd = getCustomJump() * playerJumpPendingScale;
		if (hasEffect(MobEffect::jump))
		{
			yd += (getEffect(MobEffect::jump)->getAmplifier() + 1) * .1f;
		}

		setIsJumping(true);
		hasImpulse = true;

		if (ya > 0)
		{
			float sin = Mth::sin(yRot * PI / 180);
			float cos = Mth::cos(yRot * PI / 180);

			xd += -0.4f * sin * playerJumpPendingScale;
			zd += 0.4f * cos * playerJumpPendingScale;

			playSound(eSoundType_MOB_HORSE_JUMP, .4f, 1);
		}
		playerJumpPendingScale = 0;
	}

	footSize = 1;
	flyingSpeed = getSpeed() * .1f;
	if (!level->isClientSide)
	{
		setSpeed(static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue()));
		Animal::travel(xa, ya);
	}


	if (onGround)
	{
		// blood - fixes jump bug
		playerJumpPendingScale = 0;
		setIsJumping(false);
	}
	walkAnimSpeedO = walkAnimSpeed;
	double dx = x - xo;
	double dz = z - zo;
	float wst = Mth::sqrt(dx * dx + dz * dz) * 4.0f;
	if (wst > 1.0f)
	{
		wst = 1.0f;
	}

	walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;

}


void EntityHorse::addAdditonalSaveData(CompoundTag *tag)
{
	Animal::addAdditonalSaveData(tag);

	tag->putBoolean(L"EatingHaystack", isEating());
	tag->putBoolean(L"ChestedHorse", isChestedHorse());
	tag->putBoolean(L"HasReproduced", getHasReproduced());
	tag->putBoolean(L"Bred", isBred());
	tag->putInt(L"Type", getType());
	tag->putInt(L"Variant", getVariant());
	tag->putInt(L"Temper", getTemper());
	tag->putBoolean(L"Tame", isTamed());
	tag->putString(L"OwnerName", getOwnerName());

	if (isChestedHorse())
	{
		ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

		for (int i = INV_BASE_COUNT; i < inventory->getContainerSize(); i++)
		{
			shared_ptr<ItemInstance> stack = inventory->getItem(i);

			if (stack != nullptr)
			{
				CompoundTag *compoundTag = new CompoundTag();

				compoundTag->putByte(L"Slot", static_cast<byte>(i));

				stack->save(compoundTag);
				listTag->add(compoundTag);
			}
		}
		tag->put(L"Items", listTag);
	}

	if (inventory->getItem(INV_SLOT_ARMOR) != nullptr)
	{
		tag->put(L"ArmorItem", inventory->getItem(INV_SLOT_ARMOR)->save(new CompoundTag(L"ArmorItem")));
	}
	if (inventory->getItem(INV_SLOT_SADDLE) != nullptr)
	{
		tag->put(L"SaddleItem", inventory->getItem(INV_SLOT_SADDLE)->save(new CompoundTag(L"SaddleItem")));
	}
}


void EntityHorse::readAdditionalSaveData(CompoundTag *tag)
{
	Animal::readAdditionalSaveData(tag);
	setEating(tag->getBoolean(L"EatingHaystack"));
	setBred(tag->getBoolean(L"Bred"));
	setChestedHorse(tag->getBoolean(L"ChestedHorse"));
	setReproduced(tag->getBoolean(L"HasReproduced"));
	setType(tag->getInt(L"Type"));
	setVariant(tag->getInt(L"Variant"));
	setTemper(tag->getInt(L"Temper"));
	setTamed(tag->getBoolean(L"Tame"));
	if (tag->contains(L"OwnerName"))
	{
		setOwner(tag->getString(L"OwnerName"));
	}

	// 4J: This is for handling old save data, not needed on console
	/*AttributeInstance *oldSpeedAttribute = getAttributes()->getInstance(SharedMonsterAttributes::MOVEMENT_SPEED);

	if (oldSpeedAttribute != nullptr)
	{
		getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(oldSpeedAttribute->getBaseValue() * 0.25f);
	}*/

	if (isChestedHorse())
	{
		ListTag<CompoundTag> *nbttaglist = (ListTag<CompoundTag> *) tag->getList(L"Items");
		createInventory();

		for (int i = 0; i < nbttaglist->size(); i++)
		{
			CompoundTag *compoundTag = nbttaglist->get(i);
			int slot = compoundTag->getByte(L"Slot") & 0xFF;

			if (slot >= INV_BASE_COUNT && slot < inventory->getContainerSize())
			{
				inventory->setItem(slot, ItemInstance::fromTag(compoundTag));
			}
		}
	}

	if (tag->contains(L"ArmorItem"))
	{
		shared_ptr<ItemInstance> armor = ItemInstance::fromTag(tag->getCompound(L"ArmorItem"));
		if (armor != nullptr && isHorseArmor(armor->id))
		{
			inventory->setItem(INV_SLOT_ARMOR, armor);
		}
	}

	if (tag->contains(L"SaddleItem"))
	{
		shared_ptr<ItemInstance> saddleItem = ItemInstance::fromTag(tag->getCompound(L"SaddleItem"));
		if (saddleItem != nullptr && saddleItem->id == Item::saddle_Id)
		{
			inventory->setItem(INV_SLOT_SADDLE, saddleItem);
		}
	}
	else if (tag->getBoolean(L"Saddle"))
	{
		inventory->setItem(INV_SLOT_SADDLE, std::make_shared<ItemInstance>(Item::saddle));
	}
	updateEquipment();
}


bool EntityHorse::canMate(shared_ptr<Animal> partner)
{
	if (partner == shared_from_this()) return false;
	if (partner->GetType() != GetType()) return false;

	shared_ptr<EntityHorse> horsePartner = dynamic_pointer_cast<EntityHorse>(partner);

	if (!isReadyForParenting() || !horsePartner->isReadyForParenting())
	{
		return false;
	}
	int type = getType();
	int pType = horsePartner->getType();

	return type == pType || (type == TYPE_HORSE && pType == TYPE_DONKEY) || (type == TYPE_DONKEY && pType == TYPE_HORSE);
}


shared_ptr<AgableMob> EntityHorse::getBreedOffspring(shared_ptr<AgableMob> partner)
{
	shared_ptr<EntityHorse> horsePartner = dynamic_pointer_cast<EntityHorse>(partner);
	shared_ptr<EntityHorse> baby = std::make_shared<EntityHorse>(level);

	int type = getType();
	int partnerType = horsePartner->getType();
	int babyType = TYPE_HORSE;

	if (type == partnerType)
	{
		babyType = type;
	}
	else if (type == TYPE_HORSE && partnerType == TYPE_DONKEY || type == TYPE_DONKEY && partnerType == TYPE_HORSE)
	{
		babyType = TYPE_MULE;
	}

	// select skin and marking colors
	if (babyType == TYPE_HORSE)
	{
		int skinResult;
		int selectSkin = random->nextInt(9);
		if (selectSkin < 4)
		{
			skinResult = getVariant() & 0xff;
		}
		else if (selectSkin < 8)
		{
			skinResult = horsePartner->getVariant() & 0xff;
		}
		else
		{
			skinResult = random->nextInt(VARIANTS);
		}

		int selectMarking = random->nextInt(5);
		if (selectMarking < 4)
		{
			skinResult |= getVariant() & 0xff00;
		}
		else if (selectMarking < 8)
		{
			skinResult |= horsePartner->getVariant() & 0xff00;
		}
		else
		{
			skinResult |= (random->nextInt(MARKINGS) << 8) & 0xff00;
		}
		baby->setVariant(skinResult);
	}

	baby->setType(babyType);

	// generate stats from parents
	double maxHealth = getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getBaseValue() + partner->getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getBaseValue() + generateRandomMaxHealth();
	baby->getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(maxHealth / 3.0f);

	double jumpStrength = getAttribute(JUMP_STRENGTH)->getBaseValue() + partner->getAttribute(JUMP_STRENGTH)->getBaseValue() + generateRandomJumpStrength();
	baby->getAttribute(JUMP_STRENGTH)->setBaseValue(jumpStrength / 3.0f);

	double speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getBaseValue() + partner->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getBaseValue() + generateRandomSpeed();
	baby->getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(speed / 3.0f);

	return baby;
}

MobGroupData *EntityHorse::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = Animal::finalizeMobSpawn(groupData);

	int type = 0;
	int variant = 0;

	if ( dynamic_cast<HorseGroupData *>(groupData) != nullptr )
	{
		type = static_cast<HorseGroupData *>(groupData)->horseType;
		variant = static_cast<HorseGroupData *>(groupData)->horseVariant & 0xff | (random->nextInt(MARKINGS) << 8);
	}
	else
	{
		if(extraData != 0)
		{
			type = extraData - 1;
		}
		else if (random->nextInt(10) == 0)
		{
			type = TYPE_DONKEY;
		}
		else
		{
			type = TYPE_HORSE;
		}

		if(type == TYPE_HORSE)
		{
			int skin = random->nextInt(VARIANTS);
			int mark = random->nextInt(MARKINGS);
			variant = skin | (mark << 8);
		}
		groupData = new HorseGroupData(type, variant);
	}

	setType(type);
	setVariant(variant);

	if (random->nextInt(5) == 0)
	{
		setAge(AgableMob::BABY_START_AGE);
	}

	if (type == TYPE_SKELETON || type == TYPE_UNDEAD)
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(15);
		getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.2f);
	}
	else
	{
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(generateRandomMaxHealth());
		if (type == TYPE_HORSE)
		{
			getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(generateRandomSpeed());
		}
		else
		{
			getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.175f);
		}
	}
	if (type == TYPE_MULE || type == TYPE_DONKEY)
	{
		getAttribute(JUMP_STRENGTH)->setBaseValue(.5f);
	}
	else
	{
		getAttribute(JUMP_STRENGTH)->setBaseValue(generateRandomJumpStrength());
	}
	setHealth(getMaxHealth());

	return groupData;
}

float EntityHorse::getEatAnim(float a)
{
	return eatAnimO + (eatAnim - eatAnimO) * a;
}

float EntityHorse::getStandAnim(float a)
{
	return standAnimO + (standAnim - standAnimO) * a;
}

float EntityHorse::getMouthAnim(float a)
{
	return mouthAnimO + (mouthAnim - mouthAnimO) * a;
}

bool EntityHorse::useNewAi()
{
	return true;
}

void EntityHorse::onPlayerJump(int jumpAmount)
{
	if (isSaddled())
	{
		if (jumpAmount < 0)
		{
			jumpAmount = 0;
		}
		else
		{
			allowStandSliding = true;
			stand();
		}

		if (jumpAmount >= 90)
		{
			playerJumpPendingScale = 1.0f;
		}
		else
		{
			playerJumpPendingScale = .4f + .4f * static_cast<float>(jumpAmount) / 90.0f;
		}
	}
}

void EntityHorse::spawnTamingParticles(bool success)
{
	ePARTICLE_TYPE particle = success ? eParticleType_heart : eParticleType_smoke;

	for (int i = 0; i < 7; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		level->addParticle(particle, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + .5f + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
	}
}

void EntityHorse::handleEntityEvent(byte id)
{
	if (id == EntityEvent::TAMING_SUCCEEDED)
	{
		spawnTamingParticles(true);
	}
	else if (id == EntityEvent::TAMING_FAILED)
	{
		spawnTamingParticles(false);
	}
	else
	{
		Animal::handleEntityEvent(id);
	}
}

void EntityHorse::positionRider()
{
	Animal::positionRider();

	if (standAnimO > 0)
	{
		float sin = Mth::sin(yBodyRot * PI / 180);
		float cos = Mth::cos(yBodyRot * PI / 180);
		float dist = .7f * standAnimO;
		float height = .15f * standAnimO;

		rider.lock()->setPos(x + dist * sin, y + getRideHeight() + rider.lock()->getRidingHeight() + height, z - dist * cos);

		if ( rider.lock()->instanceof(eTYPE_LIVINGENTITY) )
		{
			shared_ptr<LivingEntity> livingRider = dynamic_pointer_cast<LivingEntity>(rider.lock());
			livingRider->yBodyRot = yBodyRot;
		}
	}
}

// Health is between 15 and 30
float EntityHorse::generateRandomMaxHealth()
{
	return 15.0f + random->nextInt(8) + random->nextInt(9);
}

double EntityHorse::generateRandomJumpStrength()
{
	return .4f + random->nextDouble() * .2 + random->nextDouble() * .2 + random->nextDouble() * .2;
}

double EntityHorse::generateRandomSpeed()
{
	double speed = (0.45f + random->nextDouble() * .3 + random->nextDouble() * .3 + random->nextDouble() * .3) * 0.25f;
	app.DebugPrintf("<EntityHorse::generateRandomSpeed> Speed: %f\n", speed);
	return speed;
}

EntityHorse::HorseGroupData::HorseGroupData(int type, int variant)
{
	horseType = type;
	horseVariant = variant;
}

bool EntityHorse::isHorseArmor(int itemId)
{
	return itemId == Item::horseArmorMetal_Id || itemId == Item::horseArmorGold_Id || itemId == Item::horseArmorDiamond_Id;
}

bool EntityHorse::onLadder()
{
	// prevent horses from climbing ladders
	return false;
}

shared_ptr<Player> EntityHorse::getOwner()
{
	return level->getPlayerByUUID(getOwnerName());
}