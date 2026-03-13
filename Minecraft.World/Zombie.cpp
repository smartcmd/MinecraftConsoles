#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.player.h"
#include "Zombie.h"
#include "GenericStats.h"
#include "../Minecraft.Client/Textures.h"
#include "net.minecraft.world.entity.h"
#include "JavaMath.h"
#include "SoundTypes.h"

 Attribute *Zombie::SPAWN_REINFORCEMENTS_CHANCE = (new RangedAttribute(eAttributeId_ZOMBIE_SPAWNREINFORCEMENTS, 0, 0, 1));
 AttributeModifier *Zombie::SPEED_MODIFIER_BABY = new AttributeModifier(eModifierId_MOB_ZOMBIE_BABYSPEED, 0.5f, AttributeModifier::OPERATION_MULTIPLY_BASE);

const float Zombie::ZOMBIE_LEADER_CHANCE = 0.05f;


Zombie::Zombie(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	villagerConversionTime = 0;

	getNavigation()->setCanOpenDoors(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new BreakDoorGoal(this));
	goalSelector.addGoal(2, new MeleeAttackGoal(this, eTYPE_PLAYER, 1.0, false));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGER, 1.0, true));
	goalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, 1.0));
	goalSelector.addGoal(5, new MoveThroughVillageGoal(this, 1.0, false));
	goalSelector.addGoal(6, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Villager), 0, false));
}

void Zombie::registerAttributes()
{
	Monster::registerAttributes();

	// 4J Stu - Don't make it so far!
	getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(40);

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23f);
	getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(3);

	getAttributes()->registerAttribute(SPAWN_REINFORCEMENTS_CHANCE)->setBaseValue(random->nextDouble() * 0.10f);
}

void Zombie::defineSynchedData()
{
	Monster::defineSynchedData();

	getEntityData()->define(DATA_BABY_ID, static_cast<byte>(0));
	getEntityData()->define(DATA_VILLAGER_ID, static_cast<byte>(0));
	getEntityData()->define(DATA_CONVERTING_ID, static_cast<byte>(0));
}

int Zombie::getArmorValue()
{
	int value = Monster::getArmorValue() + 2;
	if (value > 20) value = 20;
	return value;
}

bool Zombie::useNewAi()
{
	return true;
}

bool Zombie::isBaby()
{
	return getEntityData()->getByte(DATA_BABY_ID) == static_cast<byte>(1);
}

void Zombie::setBaby(bool baby)
{
	getEntityData()->set(DATA_BABY_ID, (byte) (baby ? 1 : 0));
	updateSize(baby);

	if (level != nullptr && !level->isClientSide)
	{
		AttributeInstance *speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
		speed->removeModifier(SPEED_MODIFIER_BABY);
		if (baby)
		{
			speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_BABY));
		}
	}
}

bool Zombie::isVillager()
{
	return getEntityData()->getByte(DATA_VILLAGER_ID) == static_cast<byte>(1);
}

void Zombie::setVillager(bool villager)
{
	getEntityData()->set(DATA_VILLAGER_ID, static_cast<byte>(villager ? 1 : 0));
}

void Zombie::aiStep()
{
	if (level->isDay() && !level->isClientSide && !isBaby())
	{
		float br = getBrightness(1);
		if (br > 0.5f && random->nextFloat() * 30 < (br - 0.4f) * 2 && level->canSeeSky(Mth::floor(x), static_cast<int>(floor(y + 0.5)), Mth::floor(z)))
		{
			bool burn = true;

			shared_ptr<ItemInstance> helmet = getCarried(SLOT_HELM);
			if (helmet != nullptr)
			{
				if (helmet->isDamageableItem())
				{
					helmet->setAuxValue(helmet->getDamageValue() + random->nextInt(2));
					if (helmet->getDamageValue() >= helmet->getMaxDamage())
					{
						breakItem(helmet);
						setEquippedSlot(SLOT_HELM, nullptr);
					}
				}

				burn = false;
			}

			if (burn)
			{
				setOnFire(8);
			}
		}
	}
	Monster::aiStep();
}

bool Zombie::hurt(DamageSource *source, float dmg)
{
	if (Monster::hurt(source, dmg))
	{
		shared_ptr<LivingEntity> target = getTarget();
		if ( (target == nullptr) && getAttackTarget() != nullptr && getAttackTarget()->instanceof(eTYPE_LIVINGENTITY) )	target = dynamic_pointer_cast<LivingEntity>( getAttackTarget() );
		if ( (target == nullptr) && source->getEntity() != nullptr && source->getEntity()->instanceof(eTYPE_LIVINGENTITY) )	target = dynamic_pointer_cast<LivingEntity>( source->getEntity() );

		if ( (target != nullptr) && level->difficulty >= Difficulty::HARD && random->nextFloat() < getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->getValue())
		{
			int x = Mth::floor(this->x);
			int y = Mth::floor(this->y);
			int z = Mth::floor(this->z);
			shared_ptr<Zombie> reinforcement = std::make_shared<Zombie>(level);

			for (int i = 0; i < REINFORCEMENT_ATTEMPTS; i++)
			{
				int xt = x + Mth::nextInt(random, REINFORCEMENT_RANGE_MIN, REINFORCEMENT_RANGE_MAX) * Mth::nextInt(random, -1, 1);
				int yt = y + Mth::nextInt(random, REINFORCEMENT_RANGE_MIN, REINFORCEMENT_RANGE_MAX) * Mth::nextInt(random, -1, 1);
				int zt = z + Mth::nextInt(random, REINFORCEMENT_RANGE_MIN, REINFORCEMENT_RANGE_MAX) * Mth::nextInt(random, -1, 1);

				if (level->isTopSolidBlocking(xt, yt - 1, zt) && level->getRawBrightness(xt, yt, zt) < 10)
				{
					reinforcement->setPos(xt, yt, zt);

					if (level->isUnobstructed(reinforcement->bb) && level->getCubes(reinforcement, reinforcement->bb)->empty() && !level->containsAnyLiquid(reinforcement->bb))
					{
						level->addEntity(reinforcement);
						reinforcement->setTarget(target);
						reinforcement->finalizeMobSpawn(nullptr);

						getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->addModifier(new AttributeModifier(-0.05f, AttributeModifier::OPERATION_ADDITION));
						reinforcement->getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->addModifier(new AttributeModifier(-0.05f, AttributeModifier::OPERATION_ADDITION));
						break;
					}
				}
			}
		}

		return true;
	}

	return false;
}

void Zombie::tick()
{
	if (!level->isClientSide && isConverting())
	{
		int amount = getConversionProgress();

		villagerConversionTime -= amount;

		if (villagerConversionTime <= 0)
		{
			finishConversion();
		}
	}

	Monster::tick();

	if (level->isClientSide)
	{
		updateSize(isBaby());
	}
}

bool Zombie::doHurtTarget(shared_ptr<Entity> target)
{
	bool result = Monster::doHurtTarget(target);

	if (result)
	{
		if (getCarriedItem() == nullptr && isOnFire() && random->nextFloat() < level->difficulty * 0.3f)
		{
			target->setOnFire(2 * level->difficulty);
		}
	}

	return result;
}

void Zombie::updateSize(bool isBaby)
{
	float scale = isBaby ? 0.5f : 1.0f;
	setSize(0.6f, 1.8f * scale);
}

int Zombie::getAmbientSound()
{
	return eSoundType_MOB_ZOMBIE_AMBIENT;
}

int Zombie::getHurtSound()
{
	return eSoundType_MOB_ZOMBIE_HURT;
}

int Zombie::getDeathSound()
{
	return eSoundType_MOB_ZOMBIE_DEATH;
}    

int Zombie::getDeathLoot()
{
	return Item::rotten_flesh_Id;
}

void Zombie::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_ZOMBIE_STEP, 0.15f, 1);
}

MobType Zombie::getMobType()
{
	return UNDEAD;
}

void Zombie::dropRareDeathLoot(int rareLootLevel)
{
	switch (random->nextInt(3))
	{
	case 0:
		spawnAtLocation(Item::ironIngot_Id, 1);
		break;
	case 1:
		spawnAtLocation(Item::carrots_Id, 1);
		break;
	case 2:
		spawnAtLocation(Item::potato_Id, 1);
		break;
	}
}

void Zombie::populateDefaultEquipmentSlots()
{
	Monster::populateDefaultEquipmentSlots();

	if (random->nextFloat() < (level->difficulty == Difficulty::HARD ? 0.05f : 0.01f))
	{
		int rand = random->nextInt(3);
		if (rand == 0)
		{
			setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::sword_iron));
		}
		else
		{
			setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::shovel_iron));
		}
	}
}

void Zombie::addAdditonalSaveData(CompoundTag *tag)
{
	Monster::addAdditonalSaveData(tag);

	if (isBaby()) tag->putBoolean(L"IsBaby", true);
	if (isVillager()) tag->putBoolean(L"IsVillager", true);
	tag->putInt(L"ConversionTime", isConverting() ? villagerConversionTime : -1);
}

void Zombie::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);

	if (tag->getBoolean(L"IsBaby")) setBaby(true);
	if (tag->getBoolean(L"IsVillager")) setVillager(true);
	if (tag->contains(L"ConversionTime") && tag->getInt(L"ConversionTime") > -1) startConverting(tag->getInt(L"ConversionTime"));
}

void Zombie::killed(shared_ptr<LivingEntity> mob)
{
	Monster::killed(mob);

	if ( level->difficulty >= Difficulty::NORMAL && (mob->GetType() == eTYPE_VILLAGER) ) // 4J-JEV: Villager isn't a non-terminal class, no need to instanceof.
	{
		if (level->difficulty == Difficulty::NORMAL && random->nextBoolean()) return;

		shared_ptr<Zombie> zombie = std::make_shared<Zombie>(level);
		zombie->copyPosition(mob);
		level->removeEntity(mob);
		zombie->finalizeMobSpawn(nullptr);
		zombie->setVillager(true);
		if (mob->isBaby()) zombie->setBaby(true);
		level->addEntity(zombie);

		level->levelEvent(nullptr, LevelEvent::SOUND_ZOMBIE_INFECTED, static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), 0);
	}
}

MobGroupData *Zombie::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = Monster::finalizeMobSpawn(groupData);
	float difficulty = level->getDifficulty(x, y, z);

	setCanPickUpLoot(random->nextFloat() < MAX_PICKUP_LOOT_CHANCE * difficulty);

	if (groupData == nullptr)
	{
		groupData = new ZombieGroupData(level->random->nextFloat() < 0.05f, level->random->nextFloat() < 0.05f);
	}

	if ( dynamic_cast<ZombieGroupData *>( groupData ) != nullptr)
	{
		ZombieGroupData *zombieData = static_cast<ZombieGroupData *>(groupData);

		if (zombieData->isVillager)
		{
			setVillager(true);
		}

		if (zombieData->isBaby)
		{
			setBaby(true);
		}
	}

	populateDefaultEquipmentSlots();
	populateDefaultEquipmentEnchantments();

	if (getCarried(SLOT_HELM) == nullptr)
	{
		// [EB]: We have this code in quite some places, shouldn't we set
		// something like this globally?
		if (Calendar::GetMonth() + 1 == 10 && Calendar::GetDayOfMonth() == 31 && random->nextFloat() < 0.25f)
		{
			// Halloween! OooOOo! 25% of all skeletons/zombies can wear
			// pumpkins on their heads.
			setEquippedSlot(SLOT_HELM, std::make_shared<ItemInstance>(random->nextFloat() < 0.1f ? Tile::litPumpkin : Tile::pumpkin));
			dropChances[SLOT_HELM] = 0;
		}
	}

	getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->addModifier(new AttributeModifier(random->nextDouble() * 0.05f, AttributeModifier::OPERATION_ADDITION));

	// 4J Stu - Take this out, it's not good and nobody will notice. Also not great for performance.
	//getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->addModifier(new AttributeModifier(random->nextDouble() * 1.50f, AttributeModifier::OPERATION_MULTIPLY_TOTAL));

	if (random->nextFloat() < difficulty * ZOMBIE_LEADER_CHANCE)
	{
		getAttribute(SPAWN_REINFORCEMENTS_CHANCE)->addModifier(new AttributeModifier(random->nextDouble() * 0.25f + 0.50f, AttributeModifier::OPERATION_ADDITION));
		getAttribute(SharedMonsterAttributes::MAX_HEALTH)->addModifier(new AttributeModifier(random->nextDouble() * 3.0f + 1.0f, AttributeModifier::OPERATION_MULTIPLY_TOTAL));
	}

	return groupData;
}

bool Zombie::mobInteract(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->getSelectedItem();

	if (item != nullptr && item->getItem() == Item::apple_gold && item->getAuxValue() == 0 && isVillager() && hasEffect(MobEffect::weakness))
	{
		if (!player->abilities.instabuild) item->count--;
		if (item->count <= 0)
		{
			player->inventory->setItem(player->inventory->selected, nullptr);
		}

		if (!level->isClientSide)
		{
			startConverting(random->nextInt(VILLAGER_CONVERSION_WAIT_MAX - VILLAGER_CONVERSION_WAIT_MIN + 1) + VILLAGER_CONVERSION_WAIT_MIN);

			// 4J-JEV, award achievement here, as it is impractical to award when the zombie is actually cured.
			player->awardStat(GenericStats::zombieDoctor(),GenericStats::param_zombieDoctor());
		}

		return true;
	}

	return false;
}

void Zombie::startConverting(int time)
{
	villagerConversionTime = time;
	getEntityData()->set(DATA_CONVERTING_ID, static_cast<byte>(1));

	removeEffect(MobEffect::weakness->id);
	addEffect(new MobEffectInstance(MobEffect::damageBoost->id, time, min(level->difficulty - 1, 0)));

	level->broadcastEntityEvent(shared_from_this(), EntityEvent::ZOMBIE_CONVERTING);
}

void Zombie::handleEntityEvent(byte id)
{
	if (id == EntityEvent::ZOMBIE_CONVERTING)
	{
		level->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_MOB_ZOMBIE_REMEDY, 1 + random->nextFloat(), random->nextFloat() * 0.7f + 0.3f, false);
	}
	else
	{
		Monster::handleEntityEvent(id);
	}
}

bool Zombie::removeWhenFarAway()
{
	return !isConverting();
}

bool Zombie::isConverting()
{
	return getEntityData()->getByte(DATA_CONVERTING_ID) == static_cast<byte>(1);
}

void Zombie::finishConversion()
{
	shared_ptr<Villager> villager = std::make_shared<Villager>(level);
	villager->copyPosition(shared_from_this());
	villager->finalizeMobSpawn(nullptr);
	villager->setRewardPlayersInVillage();
	if (isBaby()) villager->setAge(-20 * 60 * 20);
	level->removeEntity(shared_from_this());
	level->addEntity(villager);

	villager->addEffect(new MobEffectInstance(MobEffect::confusion->id, SharedConstants::TICKS_PER_SECOND * 10, 0));
	level->levelEvent(nullptr, LevelEvent::SOUND_ZOMBIE_CONVERTED, static_cast<int>(x), static_cast<int>(y), static_cast<int>(z), 0);
}

int Zombie::getConversionProgress()
{
	int amount = 1;

	if (random->nextFloat() < 0.01f)
	{
		int specialBlocksCount = 0;

		for (int xx = static_cast<int>(x) - SPECIAL_BLOCK_RADIUS; xx < static_cast<int>(x) + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; xx++)
		{
			for (int yy = static_cast<int>(y) - SPECIAL_BLOCK_RADIUS; yy < static_cast<int>(y) + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; yy++)
			{
				for (int zz = static_cast<int>(z) - SPECIAL_BLOCK_RADIUS; zz < static_cast<int>(z) + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; zz++)
				{
					int tile = level->getTile(xx, yy, zz);

					if (tile == Tile::ironFence_Id || tile == Tile::bed_Id)
					{
						if (random->nextFloat() < 0.3f) amount++;
						specialBlocksCount++;
					}
				}
			}
		}
	}
	return amount;
}

Zombie::ZombieGroupData::ZombieGroupData(bool baby, bool villager)
{
	isBaby = baby;
	isVillager = villager;
}
