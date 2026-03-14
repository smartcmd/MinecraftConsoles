#include "stdafx.h"
#include "JavaMath.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.control.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.ai.sensing.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.h"
#include "../Minecraft.Client/ServerLevel.h"
#include "../Minecraft.Client/EntityTracker.h"
#include "com.mojang.nbt.h"
#include "Mob.h"
#include "../Minecraft.Client/Textures.h"
#include "SoundTypes.h"
#include "BasicTypeContainers.h"
#include "ParticleTypes.h"
#include "GenericStats.h"
#include "ItemEntity.h"

const float Mob::MAX_WEARING_ARMOR_CHANCE = 0.15f;
const float Mob::MAX_PICKUP_LOOT_CHANCE = 0.55f;
const float Mob::MAX_ENCHANTED_ARMOR_CHANCE = 0.50f;
const float Mob::MAX_ENCHANTED_WEAPON_CHANCE = 0.25f;

void Mob::_init()
{
	ambientSoundTime = 0;
	xpReward = 0;
	defaultLookAngle = 0.0f;
	lookingAt = nullptr;
	lookTime = 0;
	target = nullptr;
	sensing = nullptr;

	equipment = ItemInstanceArray(5);
	dropChances  = floatArray(5);
	for(unsigned int i = 0; i < 5; ++i)
	{
		equipment[i] = nullptr;
		dropChances[i] = 0.0f;
	}

	_canPickUpLoot = false;
	persistenceRequired = false;

	_isLeashed = false;
	leashHolder = nullptr;
	leashInfoTag = nullptr;
}

Mob::Mob( Level* level) : LivingEntity(level)
{
	MemSect(57);
	_init();
	MemSect(0);

	MemSect(58);
	// 4J Stu - We call this again in the derived classes, but need to do it here for some internal members
	registerAttributes();
	MemSect(0);

	lookControl = new LookControl(this);
	moveControl = new MoveControl(this);
	jumpControl = new JumpControl(this);
	bodyControl = new BodyControl(this);
	navigation = new PathNavigation(this, level);
	sensing = new Sensing(this);

	for (int i = 0; i < 5; i++)
	{
		dropChances[i] = 0.085f;
	}
}

Mob::~Mob()
{
	if(lookControl != nullptr) delete lookControl;
	if(moveControl != nullptr) delete moveControl;
	if(jumpControl != nullptr) delete jumpControl;
	if(bodyControl != nullptr) delete bodyControl;
	if(navigation != nullptr) delete navigation;
	if(sensing != nullptr) delete sensing;

	if(leashInfoTag != nullptr) delete leashInfoTag;

	if(equipment.data != nullptr) delete [] equipment.data;
	delete [] dropChances.data;
}

void Mob::registerAttributes()
{
	LivingEntity::registerAttributes();

	getAttributes()->registerAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(16);
}

LookControl *Mob::getLookControl()
{
	return lookControl;
}

MoveControl *Mob::getMoveControl()
{
	return moveControl;
}

JumpControl *Mob::getJumpControl()
{
	return jumpControl;
}

PathNavigation *Mob::getNavigation()
{
	return navigation;
}

Sensing *Mob::getSensing()
{
	return sensing;
}

shared_ptr<LivingEntity> Mob::getTarget()
{
	return target;
}

void Mob::setTarget(shared_ptr<LivingEntity> target)
{
	this->target = target;
}

bool Mob::canAttackType(eINSTANCEOF targetType)
{
	return !(targetType == eTYPE_CREEPER || targetType == eTYPE_GHAST);
}

// Called by eatTileGoal
void Mob::ate()
{
}

void Mob::defineSynchedData()
{
	LivingEntity::defineSynchedData();
	entityData->define(DATA_CUSTOM_NAME_VISIBLE, static_cast<byte>(0));
	entityData->define(DATA_CUSTOM_NAME, L"");
}

int Mob::getAmbientSoundInterval()
{
	return 20 * 4;
}

void Mob::playAmbientSound()
{
	MemSect(31);
	int ambient = getAmbientSound();
	if (ambient != -1)
	{
		playSound(ambient, getSoundVolume(), getVoicePitch());
	}
	MemSect(0);
}

void Mob::baseTick()
{
	LivingEntity::baseTick();

	if (isAlive() && random->nextInt(1000) < ambientSoundTime++)
	{
		ambientSoundTime = -getAmbientSoundInterval();

		playAmbientSound();
	}
}

int Mob::getExperienceReward(shared_ptr<Player> killedBy)
{
	if (xpReward > 0)
	{
		int result = xpReward;

		ItemInstanceArray slots = getEquipmentSlots();
		for (int i = 0; i < slots.length; i++)
		{
			if (slots[i] != nullptr && dropChances[i] <= 1)
			{
				result += 1 + random->nextInt(3);
			}
		}

		return result;
	}
	else
	{
		return xpReward;
	}
}
void Mob::spawnAnim()
{
	for (int i = 0; i < 20; i++)
	{
		double xa = random->nextGaussian() * 0.02;
		double ya = random->nextGaussian() * 0.02;
		double za = random->nextGaussian() * 0.02;
		double dd = 10;
		level->addParticle(eParticleType_explode, x + random->nextFloat() * bbWidth * 2 - bbWidth - xa * dd, y + random->nextFloat() * bbHeight - ya * dd, z + random->nextFloat() * bbWidth * 2 - bbWidth - za
			* dd, xa, ya, za);
	}
}

void Mob::tick()
{
	LivingEntity::tick();

	if (!level->isClientSide)
	{
		tickLeash();
	}
}

float Mob::tickHeadTurn(float yBodyRotT, float walkSpeed)
{
	if (useNewAi())
	{
		bodyControl->clientTick();
		return walkSpeed;
	}
	else
	{
		return LivingEntity::tickHeadTurn(yBodyRotT, walkSpeed);
	}
}

int Mob::getAmbientSound()
{
	return -1;
}

int Mob::getDeathLoot()
{
	return 0;
}

void Mob::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	int loot = getDeathLoot();
	if (loot > 0)
	{
		int count = random->nextInt(3);
		if (playerBonusLevel > 0)
		{
			count += random->nextInt(playerBonusLevel + 1);
		}
		for (int i = 0; i < count; i++)
			spawnAtLocation(loot, 1);
	}
}

void Mob::addAdditonalSaveData(CompoundTag *entityTag)
{
	LivingEntity::addAdditonalSaveData(entityTag);
	entityTag->putBoolean(L"CanPickUpLoot", canPickUpLoot());
	entityTag->putBoolean(L"PersistenceRequired", persistenceRequired);

	ListTag<CompoundTag> *gear = new ListTag<CompoundTag>();
	for (int i = 0; i < equipment.length; i++)
	{
		CompoundTag *tag = new CompoundTag();
		if (equipment[i] != nullptr) equipment[i]->save(tag);
		gear->add(tag);
	}
	entityTag->put(L"Equipment", gear);

	ListTag<FloatTag> *dropChanceList = new ListTag<FloatTag>();
	for (int i = 0; i < dropChances.length; i++)
	{
		dropChanceList->add(new FloatTag( std::to_wstring(i), dropChances[i]));
	}
	entityTag->put(L"DropChances", dropChanceList);
	entityTag->putString(L"CustomName", getCustomName());
	entityTag->putBoolean(L"CustomNameVisible", isCustomNameVisible());

	// leash info
	entityTag->putBoolean(L"Leashed", _isLeashed);
	if (leashHolder != nullptr)
	{
		CompoundTag *leashTag = new CompoundTag(L"Leash");
		if ( leashHolder->instanceof(eTYPE_LIVINGENTITY) )
		{
			// a walking, talking, leash holder
			leashTag->putString(L"UUID", leashHolder->getUUID());
		}
		else if ( leashHolder->instanceof(eTYPE_HANGING_ENTITY) )
		{
			// a fixed holder (that doesn't save itself)
			shared_ptr<HangingEntity> hangInThere = dynamic_pointer_cast<HangingEntity>(leashHolder);
			leashTag->putInt(L"X", hangInThere->xTile);
			leashTag->putInt(L"Y", hangInThere->yTile);
			leashTag->putInt(L"Z", hangInThere->zTile);
		}
		entityTag->put(L"Leash", leashTag);
	}
}

void Mob::readAdditionalSaveData(CompoundTag *tag)
{
	LivingEntity::readAdditionalSaveData(tag);

	setCanPickUpLoot(tag->getBoolean(L"CanPickUpLoot"));
	persistenceRequired = tag->getBoolean(L"PersistenceRequired");
	if (tag->contains(L"CustomName") && tag->getString(L"CustomName").length() > 0) setCustomName(tag->getString(L"CustomName"));
	setCustomNameVisible(tag->getBoolean(L"CustomNameVisible"));

	if (tag->contains(L"Equipment"))
	{
		ListTag<CompoundTag> *gear = (ListTag<CompoundTag> *) tag->getList(L"Equipment");

		for (int i = 0; i < equipment.length; i++)
		{
			equipment[i] = ItemInstance::fromTag(gear->get(i));
		}
	}

	if (tag->contains(L"DropChances"))
	{
		ListTag<FloatTag> *items = (ListTag<FloatTag> *) tag->getList(L"DropChances");
		for (int i = 0; i < items->size(); i++)
		{
			dropChances[i] = items->get(i)->data;
		}
	}

	_isLeashed = tag->getBoolean(L"Leashed");
	if (_isLeashed && tag->contains(L"Leash"))
	{
		leashInfoTag = static_cast<CompoundTag *>(tag->getCompound(L"Leash")->copy());
	}
}

void Mob::setYya(float yya)
{
	this->yya = yya;
}

void Mob::setSpeed(float speed)
{
	LivingEntity::setSpeed(speed);
	setYya(speed);
}

void Mob::aiStep()
{
	LivingEntity::aiStep();

	if (!level->isClientSide && canPickUpLoot() && !dead && level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
	{
		vector<shared_ptr<Entity> > *entities = level->getEntitiesOfClass(typeid(ItemEntity), bb->grow(1, 0, 1));
		for (auto& it : *entities)
		{
			shared_ptr<ItemEntity> entity = dynamic_pointer_cast<ItemEntity>(it);
			if (entity->removed || entity->getItem() == nullptr) continue;
			shared_ptr<ItemInstance> item = entity->getItem();
			int slot = getEquipmentSlotForItem(item);

			if (slot > -1)
			{
				bool replace = true;
				shared_ptr<ItemInstance> current = getCarried(slot);

				if (current != nullptr)
				{
					if (slot == SLOT_WEAPON)
					{
						WeaponItem *newWeapon = dynamic_cast<WeaponItem *>(item->getItem());
						WeaponItem *oldWeapon = dynamic_cast<WeaponItem *>(current->getItem());
						if ( newWeapon != nullptr && oldWeapon == nullptr)
						{
							replace = true;
						}
						else if (newWeapon != nullptr && oldWeapon != nullptr)
						{
							if (newWeapon->getTierDamage() == oldWeapon->getTierDamage())
							{
								replace = item->getAuxValue() > current->getAuxValue() || item->hasTag() && !current->hasTag();
							}
							else
							{
								replace = newWeapon->getTierDamage() > oldWeapon->getTierDamage();
							}
						}
						else
						{
							replace = false;
						}
					}
					else
					{
						ArmorItem *newArmor = dynamic_cast<ArmorItem *>(item->getItem());
						ArmorItem *oldArmor = dynamic_cast<ArmorItem *>(current->getItem());
						if (newArmor != nullptr && oldArmor == nullptr)
						{
							replace = true;
						}
						else if (newArmor != nullptr && oldArmor != nullptr)
						{
							if (newArmor->defense == oldArmor->defense)
							{
								replace = item->getAuxValue() > current->getAuxValue() || item->hasTag() && !current->hasTag();
							}
							else
							{
								replace = newArmor->defense > oldArmor->defense;
							}
						}
						else
						{
							replace = false;
						}
					}
				}

				if (replace)
				{
					if (current != nullptr && random->nextFloat() - 0.1f < dropChances[slot])
					{
						spawnAtLocation(current, 0);
					}

					setEquippedSlot(slot, item);
					dropChances[slot] = 2;
					persistenceRequired = true;
					take(entity, 1);
					entity->remove();
				}
			}
		}
		delete entities;
	}
}

bool Mob::useNewAi()
{
	return false;
}

bool Mob::removeWhenFarAway()
{
	return true;
}

void Mob::checkDespawn()
{
	if (persistenceRequired)
	{
		noActionTime = 0;
		return;
	}
	shared_ptr<Entity> player = level->getNearestPlayer(shared_from_this(), -1);
	if (player != nullptr)
	{
		double xd = player->x - x;
		double yd = player->y - y;
		double zd = player->z - z;
		double sd = xd * xd + yd * yd + zd * zd;

		if (removeWhenFarAway() && sd > 128 * 128)
		{
			remove();
		}

		if (noActionTime > 20 * 30 && random->nextInt(800) == 0 && sd > 32 * 32 && removeWhenFarAway())
		{
			remove();
		}
		else if (sd < 32 * 32)
		{
			noActionTime = 0;
		}
	}
}

void Mob::newServerAiStep()
{
	PIXBeginNamedEvent(0,"Tick target selector for %d",GetType());
	MemSect(51);
	noActionTime++;
	PIXBeginNamedEvent(0,"Check despawn");
	checkDespawn();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick sensing");
	sensing->tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick target selector");
	targetSelector.tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick goal selectors");
	goalSelector.tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick navigation");
	navigation->tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick server ai mob step");
	serverAiMobStep();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick move");
	moveControl->tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick look");
	lookControl->tick();
	PIXEndNamedEvent();
	PIXBeginNamedEvent(0,"Tick jump");
	jumpControl->tick();
	PIXEndNamedEvent();
	// Consider this for extra strolling if it is protected against despawning. We aren't interested in ones that aren't protected as the whole point of this
	// extra wandering is to potentially transition from protected to not protected.
	PIXBeginNamedEvent(0,"Consider extra wandering");
	considerForExtraWandering( isDespawnProtected() );
	PIXEndNamedEvent();
	MemSect(0);
	PIXEndNamedEvent();
}

void Mob::serverAiStep()
{
	LivingEntity::serverAiStep();

	xxa = 0;
	yya = 0;

	checkDespawn();

	float lookDistance = 8;
	if (random->nextFloat() < 0.02f)
	{
		shared_ptr<Player> player = level->getNearestPlayer(shared_from_this(), lookDistance);
		if (player != nullptr)
		{
			lookingAt = player;
			lookTime = 10 + random->nextInt(20);
		}
		else
		{
			yRotA = (random->nextFloat() - 0.5f) * 20;
		}
	}

	if (lookingAt != nullptr)
	{
		lookAt(lookingAt, 10.0f, static_cast<float>(getMaxHeadXRot()));
		if (lookTime-- <= 0 || lookingAt->removed || lookingAt->distanceToSqr(shared_from_this()) > lookDistance * lookDistance)
		{
			lookingAt = nullptr;
		}
	}
	else
	{
		if (random->nextFloat() < 0.05f)
		{
			yRotA = (random->nextFloat() - 0.5f) * 20;
		}
		yRot += yRotA;
		xRot = defaultLookAngle;
	}

	bool inWater = isInWater();
	bool inLava = isInLava();
	if (inWater || inLava) jumping = random->nextFloat() < 0.8f;
}

int Mob::getMaxHeadXRot()
{
	return 40;
}

void Mob::lookAt(shared_ptr<Entity> e, float yMax, float xMax)
{
	double xd = e->x - x;
	double yd;
	double zd = e->z - z;


	if ( e->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(e);
		yd = (mob->y + mob->getHeadHeight()) - (y + getHeadHeight());
	}
	else
	{
		yd = (e->bb->y0 + e->bb->y1) / 2 - (y + getHeadHeight());
	}

	double sd = Mth::sqrt(xd * xd + zd * zd);

	float yRotD = static_cast<float>(atan2(zd, xd) * 180 / PI) - 90;
	float xRotD = static_cast<float>(-(atan2(yd, sd) * 180 / PI));
	xRot = rotlerp(xRot, xRotD, xMax);
	yRot = rotlerp(yRot, yRotD, yMax);
}

bool Mob::isLookingAtAnEntity()
{
	return lookingAt != nullptr;
}

shared_ptr<Entity> Mob::getLookingAt()
{
	return lookingAt;
}

float Mob::rotlerp(float a, float b, float max)
{
	float diff = Mth::wrapDegrees(b - a);
	if (diff > max)
	{
		diff = max;
	}
	if (diff < -max)
	{
		diff = -max;
	}
	return a + diff;
}

bool Mob::canSpawn()
{
	// 4J - altered to use special containsAnyLiquid variant
	return level->isUnobstructed(bb) && level->getCubes(shared_from_this(), bb)->empty() && !level->containsAnyLiquid_NoLoad(bb);
}

float Mob::getSizeScale()
{
	return 1.0f;
}

float Mob::getHeadSizeScale()
{
	return 1.0f;
}

int Mob::getMaxSpawnClusterSize()
{
	return 4;
}

int Mob::getMaxFallDistance()
{
	if (getTarget() == nullptr) return 3;
	int sacrifice = static_cast<int>(getHealth() - (getMaxHealth() * 0.33f));
	sacrifice -= (3 - level->difficulty) * 4;
	if (sacrifice < 0) sacrifice = 0;
	return sacrifice + 3;
}

shared_ptr<ItemInstance> Mob::getCarriedItem()
{
	return equipment[SLOT_WEAPON];
}

shared_ptr<ItemInstance> Mob::getCarried(int slot)
{
	return equipment[slot];
}

shared_ptr<ItemInstance> Mob::getArmor(int pos)
{
	return equipment[pos + 1];
}

void Mob::setEquippedSlot(int slot, shared_ptr<ItemInstance> item)
{
	equipment[slot] = item;
}

ItemInstanceArray Mob::getEquipmentSlots()
{
	return equipment;
}

void Mob::dropEquipment(bool byPlayer, int playerBonusLevel)
{
	for (int slot = 0; slot < getEquipmentSlots().length; slot++)
	{
		shared_ptr<ItemInstance> item = getCarried(slot);
		bool preserve = dropChances[slot] > 1;

		if (item != nullptr && (byPlayer || preserve) && random->nextFloat() - playerBonusLevel * 0.01f < dropChances[slot])
		{
			if (!preserve && item->isDamageableItem())
			{
				int _max = max(item->getMaxDamage() - 25, 1);
				int damage = item->getMaxDamage() - random->nextInt(random->nextInt(_max) + 1);
				if (damage > _max) damage = _max;
				if (damage < 1) damage = 1;
				item->setAuxValue(damage);
			}
			spawnAtLocation(item, 0);
		}
	}
}

void Mob::populateDefaultEquipmentSlots()
{
	if (random->nextFloat() < MAX_WEARING_ARMOR_CHANCE * level->getDifficulty(x, y, z))
	{
		int armorType = random->nextInt(2);
		float partialChance = level->difficulty == Difficulty::HARD ? 0.1f : 0.25f;
		if (random->nextFloat() < 0.095f) armorType++;
		if (random->nextFloat() < 0.095f) armorType++;
		if (random->nextFloat() < 0.095f) armorType++;

		for (int i = 3; i >= 0; i--)
		{
			shared_ptr<ItemInstance> item = getArmor(i);
			if (i < 3 && random->nextFloat() < partialChance) break;
			if (item == nullptr)
			{
				Item *equip = getEquipmentForSlot(i + 1, armorType);
				if (equip != nullptr) setEquippedSlot(i + 1, std::make_shared<ItemInstance>(equip));
			}
		}
	}
}

int Mob::getEquipmentSlotForItem(shared_ptr<ItemInstance> item)
{
	if (item->id == Tile::pumpkin_Id || item->id == Item::skull_Id)
	{
		return SLOT_HELM;
	}

	ArmorItem *armorItem = dynamic_cast<ArmorItem *>(item->getItem());
	if (armorItem != nullptr)
	{
		switch (armorItem->slot)
		{
		case ArmorItem::SLOT_FEET:
			return SLOT_BOOTS;
		case ArmorItem::SLOT_LEGS:
			return SLOT_LEGGINGS;
		case ArmorItem::SLOT_TORSO:
			return SLOT_CHEST;
		case ArmorItem::SLOT_HEAD:
			return SLOT_HELM;
		}
	}

	return SLOT_WEAPON;
}

Item *Mob::getEquipmentForSlot(int slot, int type)
{
	switch (slot)
	{
	case SLOT_HELM:
		if (type == 0) return Item::helmet_leather;
		if (type == 1) return Item::helmet_gold;
		if (type == 2) return Item::helmet_chain;
		if (type == 3) return Item::helmet_iron;
		if (type == 4) return Item::helmet_diamond;
	case SLOT_CHEST:
		if (type == 0) return Item::chestplate_leather;
		if (type == 1) return Item::chestplate_gold;
		if (type == 2) return Item::chestplate_chain;
		if (type == 3) return Item::chestplate_iron;
		if (type == 4) return Item::chestplate_diamond;
	case SLOT_LEGGINGS:
		if (type == 0) return Item::leggings_leather;
		if (type == 1) return Item::leggings_gold;
		if (type == 2) return Item::leggings_chain;
		if (type == 3) return Item::leggings_iron;
		if (type == 4) return Item::leggings_diamond;
	case SLOT_BOOTS:
		if (type == 0) return Item::boots_leather;
		if (type == 1) return Item::boots_gold;
		if (type == 2) return Item::boots_chain;
		if (type == 3) return Item::boots_iron;
		if (type == 4) return Item::boots_diamond;
	}

	return nullptr;
}

void Mob::populateDefaultEquipmentEnchantments()
{
	float difficulty = level->getDifficulty(x, y, z);

	if (getCarriedItem() != nullptr && random->nextFloat() < MAX_ENCHANTED_WEAPON_CHANCE * difficulty) {
		EnchantmentHelper::enchantItem(random, getCarriedItem(), static_cast<int>(5 + difficulty * random->nextInt(18)));
	}

	for (int i = 0; i < 4; i++)
	{
		shared_ptr<ItemInstance> item = getArmor(i);
		if (item != nullptr && random->nextFloat() < MAX_ENCHANTED_ARMOR_CHANCE * difficulty)
		{
			EnchantmentHelper::enchantItem(random, item, static_cast<int>(5 + difficulty * random->nextInt(18)));
		}
	}
}

/**
* Added this method so mobs can handle their own spawn settings instead of
* hacking MobSpawner.java
*
* @param groupData
*            TODO
* @return TODO
*/
MobGroupData *Mob::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	// 4J Stu - Take this out, it's not great and nobody will notice. Also not great for performance.
	//getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->addModifier(new AttributeModifier(random->nextGaussian() * 0.05, AttributeModifier::OPERATION_MULTIPLY_BASE));

	return groupData;
}

void Mob::finalizeSpawnEggSpawn(int extraData)
{
}

bool Mob::canBeControlledByRider()
{
	return false;
}

wstring Mob::getAName()
{
	if (hasCustomName()) return getCustomName();
	return LivingEntity::getAName();
}

void Mob::setPersistenceRequired()
{
	persistenceRequired = true;
}

void Mob::setCustomName(const wstring &name)
{
	entityData->set(DATA_CUSTOM_NAME, name);
}

wstring Mob::getCustomName()
{
	return entityData->getString(DATA_CUSTOM_NAME);
}

bool Mob::hasCustomName()
{
	return entityData->getString(DATA_CUSTOM_NAME).length() > 0;
}

void Mob::setCustomNameVisible(bool visible)
{
	entityData->set(DATA_CUSTOM_NAME_VISIBLE, visible ? static_cast<byte>(1) : static_cast<byte>(0));
}

bool Mob::isCustomNameVisible()
{
	return entityData->getByte(DATA_CUSTOM_NAME_VISIBLE) == 1;
}

bool Mob::shouldShowName()
{
	return isCustomNameVisible();
}

void Mob::setDropChance(int slot, float pct)
{
	dropChances[slot] = pct;
}

bool Mob::canPickUpLoot()
{
	return _canPickUpLoot;
}

void Mob::setCanPickUpLoot(bool canPickUpLoot)
{
	_canPickUpLoot = canPickUpLoot;
}

bool Mob::isPersistenceRequired()
{
	return persistenceRequired;
}

bool Mob::interact(shared_ptr<Player> player)
{

	if (isLeashed() && getLeashHolder() == player)
	{
		dropLeash(true, !player->abilities.instabuild);
		return true;
	}

	shared_ptr<ItemInstance> itemstack = player->inventory->getSelected();
	if (itemstack != nullptr)
	{
		// it's inconvenient to have the leash code here, but it's because
		// the mob.interact(player) method has priority over
		// item.interact(mob)
		if (itemstack->id == Item::lead_Id)
		{
			if (canBeLeashed())
			{
				shared_ptr<TamableAnimal> tamableAnimal = nullptr;
				if (	shared_from_this()->instanceof(eTYPE_TAMABLE_ANIMAL)
					&&	(tamableAnimal = dynamic_pointer_cast<TamableAnimal>(shared_from_this()))->isTame() ) // 4J-JEV: excuse the assignment operator in here, don't want to dyn-cast if it's avoidable.
				{
					if (player->getUUID().compare(tamableAnimal->getOwnerUUID()) == 0)
					{
						setLeashedTo(player, true);
						itemstack->count--;
						return true;
					}
				}
				else
				{
					setLeashedTo(player, true);
					itemstack->count--;
					return true;
				}
			}
		}
	}

	if (mobInteract(player))
	{
		return true;
	}

	return LivingEntity::interact(player);
}

bool Mob::mobInteract(shared_ptr<Player> player)
{
	return false;
}

void Mob::tickLeash()
{
	if (leashInfoTag != nullptr)
	{
		restoreLeashFromSave();
	}
	if (!_isLeashed)
	{
		return;
	}

	if (leashHolder == nullptr || leashHolder->removed)
	{
		dropLeash(true, true);
		return;
	}
}

void Mob::dropLeash(bool synch, bool createItemDrop)
{
	if (_isLeashed)
	{
		_isLeashed = false;
		leashHolder = nullptr;
		if (!level->isClientSide && createItemDrop)
		{
			spawnAtLocation(Item::lead_Id, 1);
		}

		ServerLevel *serverLevel = dynamic_cast<ServerLevel *>(level);
		if (!level->isClientSide && synch && serverLevel != nullptr)
		{
			serverLevel->getTracker()->broadcast(shared_from_this(), std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::LEASH, shared_from_this(), nullptr));
		}
	}
}

bool Mob::canBeLeashed()
{
	return !isLeashed() && !shared_from_this()->instanceof(eTYPE_ENEMY);
}

bool Mob::isLeashed()
{
	return _isLeashed;
}

shared_ptr<Entity> Mob::getLeashHolder()
{
	return leashHolder;
}

void Mob::setLeashedTo(shared_ptr<Entity> holder, bool synch)
{
	_isLeashed = true;
	leashHolder = holder;

	ServerLevel *serverLevel = dynamic_cast<ServerLevel *>(level);
	if (!level->isClientSide && synch && serverLevel)
	{
		serverLevel->getTracker()->broadcast(shared_from_this(), std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::LEASH, shared_from_this(), leashHolder));
	}
}

void Mob::restoreLeashFromSave()
{
	// after being added to the world, attempt to recreate leash bond
	if (_isLeashed && leashInfoTag != nullptr)
	{
		if (leashInfoTag->contains(L"UUID"))
		{
			wstring leashUuid = leashInfoTag->getString(L"UUID");
			vector<shared_ptr<Entity> > *livingEnts = level->getEntitiesOfClass(typeid(LivingEntity), bb->grow(10, 10, 10));
			for(auto& livingEnt : *livingEnts)
			{
				shared_ptr<LivingEntity> le = dynamic_pointer_cast<LivingEntity>(livingEnt);
				if (le->getUUID().compare(leashUuid) == 0)
				{
					leashHolder = le;
					setLeashedTo(leashHolder, true);
					break;
				}
			}
			delete livingEnts;
		}
		else if (leashInfoTag->contains(L"X") && leashInfoTag->contains(L"Y") && leashInfoTag->contains(L"Z"))
		{
			int x = leashInfoTag->getInt(L"X");
			int y = leashInfoTag->getInt(L"Y");
			int z = leashInfoTag->getInt(L"Z");

			shared_ptr<LeashFenceKnotEntity> activeKnot = LeashFenceKnotEntity::findKnotAt(level, x, y, z);
			if (activeKnot == nullptr)
			{
				activeKnot = LeashFenceKnotEntity::createAndAddKnot(level, x, y, z);
			}
			leashHolder = activeKnot;
			setLeashedTo(leashHolder, true);
		}
		else
		{
			dropLeash(false, true);
		}
	}
	leashInfoTag = nullptr;
}

// 4J added so we can not render mobs before their chunks are loaded - to resolve bug 10327 :Gameplay: NPCs can spawn over chunks that have not yet been streamed and display jitter.
bool Mob::shouldRender(Vec3 *c)
{
	if( !level->reallyHasChunksAt( Mth::floor(bb->x0), Mth::floor(bb->y0), Mth::floor(bb->z0), Mth::floor(bb->x1), Mth::floor(bb->y1), Mth::floor(bb->z1)))
	{
		return false;
	}
	return Entity::shouldRender(c);
}

void Mob::setLevel(Level *level)
{
	Entity::setLevel(level);
	navigation->setLevel(level);
	goalSelector.setLevel(level);
	targetSelector.setLevel(level);
}