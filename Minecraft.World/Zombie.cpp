#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.player.h"
#include "Zombie.h"
#include "GenericStats.h"
#include "..\Minecraft.Client\Textures.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"

Zombie::Zombie(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_ZOMBIE;	// 4J was L"/mob/zombie.png";
	runSpeed = 0.23f;
	attackDamage = 4;

	villagerConversionTime = 0;
	
	registeredBBWidth = -1;
	registeredBBHeight = 0;

	setSize(bbWidth, bbHeight);

	getNavigation()->setCanOpenDoors(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new BreakDoorGoal(this));
	goalSelector.addGoal(2, new MeleeAttackGoal(this, eTYPE_PLAYER, runSpeed, false));
	goalSelector.addGoal(3, new MeleeAttackGoal(this, eTYPE_VILLAGER, runSpeed, true));
	goalSelector.addGoal(4, new MoveTowardsRestrictionGoal(this, runSpeed));
	goalSelector.addGoal(5, new MoveThroughVillageGoal(this, runSpeed, false));
	goalSelector.addGoal(6, new RandomStrollGoal(this, runSpeed));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(7, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 16, 0, true));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Villager), 16, 0, false));
}

void Zombie::defineSynchedData()
{
	Monster::defineSynchedData();

	getEntityData()->define(DATA_BABY_ID, (byte) 0);
	getEntityData()->define(DATA_VILLAGER_ID, (byte) 0);
	getEntityData()->define(DATA_CONVERTING_ID, (byte) 0);
}

float Zombie::getWalkingSpeedModifier()
{
    return Monster::getWalkingSpeedModifier() * (isBaby() ? 1.5f : 1.0f);
}

int Zombie::getTexture()
{
	return isVillager() ? TN_MOB_ZOMBIE_VILLAGER : TN_MOB_ZOMBIE;
}

int Zombie::getMaxHealth()
{
	return 20;
}

int Zombie::getArmorValue()
{
	return 2;
}

bool Zombie::useNewAi()
{
	return true;
}

bool Zombie::isBaby()
{
	return getEntityData()->getByte(DATA_BABY_ID) == (byte) 1;
}

void Zombie::setBaby(bool baby)
{
	getEntityData()->set(DATA_BABY_ID, (byte) 1);
	updateSize(isBaby());
}

bool Zombie::isVillager()
{
	return getEntityData()->getByte(DATA_VILLAGER_ID) == (byte) 1;
}

void Zombie::setVillager(bool villager)
{
	getEntityData()->set(DATA_VILLAGER_ID, (byte) (villager ? 1 : 0));
}

void Zombie::aiStep()
{
	if(level->isClientSide)
	{
		updateSize(isBaby());
	}
	else if (level->isDay() && !level->isClientSide && !isBaby())
	{
		float br = getBrightness(1);
		if (br > 0.5f && random->nextFloat() * 30 < (br - 0.4f) * 2 && level->canSeeSky( Mth::floor(x), Mth::floor(y), Mth::floor(z)))
		{
			setOnFire(8);
		}
	}
	Monster::aiStep();
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

MobType Zombie::getMobType()
{
	return UNDEAD;
}

void Zombie::dropRareDeathLoot(int rareLootLevel)
{
	switch (random->nextInt(3))
	{
/*	case 0:
		spawnAtLocation(Item::sword_iron_Id, 1);
		break;
	case 1:
		spawnAtLocation(Item::helmet_iron_Id, 1);
		break;
	case 2:
		spawnAtLocation(Item::ironIngot_Id, 1);
		break;
	case 3:
		spawnAtLocation(Item::shovel_iron_Id, 1);
		break;*/
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

void Zombie::killed(shared_ptr<Mob> mob)
{
	Monster::killed(mob);

	if (level->difficulty >= Difficulty::NORMAL && ((mob->GetType() & eTYPE_VILLAGER) == eTYPE_VILLAGER))
	{
		if( !level->canCreateMore( GetType(), Level::eSpawnType_Egg) ) return;
		if (level->difficulty == Difficulty::NORMAL && random->nextBoolean()) return;

		shared_ptr<Zombie> zombie = shared_ptr<Zombie>(new Zombie(level));
		zombie->copyPosition(mob);
		level->removeEntity(mob);
		zombie->finalizeMobSpawn();
		zombie->setVillager(true);
		if (mob->isBaby()) zombie->setBaby(true);
		level->addEntity(zombie);

		level->levelEvent(nullptr, LevelEvent::SOUND_ZOMBIE_INFECTED, (int) x, (int) y, (int) z, 0);
	}
}

void Zombie::finalizeMobSpawn()
{
	// 4J Stu - TODO TU15
#if 0
	canPickUpLoot = random->nextFloat() < CAN_PICK_UP_LOOT_CHANCES[level->difficulty];
#endif

	if (level->random->nextFloat() < 0.05f)
	{
		setVillager(true);
	}

	// 4J Stu - TODO TU15
#if 0
	populateDefaultEquipmentSlots();
	populateDefaultEquipmentEnchantments();

	if (getCarried(SLOT_HELM) == null)
	{
		Calendar cal = level.getCalendar();

		if (cal.get(Calendar.MONTH) + 1 == 10 && cal.get(Calendar.DAY_OF_MONTH) == 31 && random.nextFloat() < 0.25f) {
			// Halloween! OooOOo! 25% of all skeletons/zombies can wear
			// pumpkins on their heads.
			setEquippedSlot(SLOT_HELM, new ItemInstance(random.nextFloat() < 0.1f ? Tile.litPumpkin : Tile.pumpkin));
			dropChances[SLOT_HELM] = 0;
		}
	}
#endif
}

bool Zombie::interact(shared_ptr<Player> player)
{
	shared_ptr<ItemInstance> item = player->getSelectedItem();

	if (item != NULL && item->getItem() == Item::apple_gold && item->getAuxValue() == 0 && isVillager() && hasEffect(MobEffect::weakness))
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
	getEntityData()->set(DATA_CONVERTING_ID, (byte) 1);

	removeEffect(MobEffect::weakness->id);
	addEffect(new MobEffectInstance(MobEffect::damageBoost->id, time, min(level->difficulty - 1, 0)));

	level->broadcastEntityEvent(shared_from_this(), EntityEvent::ZOMBIE_CONVERTING);
}

void Zombie::handleEntityEvent(byte id)
{
	if (id == EntityEvent::ZOMBIE_CONVERTING)
	{
		level->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_MOB_ZOMBIE_REMEDY, 1 + random->nextFloat(), random->nextFloat() * 0.7f + 0.3f);//, false);
	}
	else
	{
		Monster::handleEntityEvent(id);
	}
}

bool Zombie::isConverting()
{
	return getEntityData()->getByte(DATA_CONVERTING_ID) == (byte) 1;
}

void Zombie::finishConversion()
{
	shared_ptr<Villager> villager = shared_ptr<Villager>(new Villager(level));
	villager->copyPosition(shared_from_this());
	villager->finalizeMobSpawn();
	villager->setRewardPlayersInVillage();
	if (isBaby()) villager->setAge(-20 * 60 * 20);
	level->removeEntity(shared_from_this());
	level->addEntity(villager);

	villager->addEffect(new MobEffectInstance(MobEffect::confusion->id, SharedConstants::TICKS_PER_SECOND * 10, 0));
	level->levelEvent(nullptr, LevelEvent::SOUND_ZOMBIE_CONVERTED, (int) x, (int) y, (int) z, 0);
}

int Zombie::getConversionProgress()
{
	int amount = 1;

	if (random->nextFloat() < 0.01f)
	{
		int specialBlocksCount = 0;

		for (int xx = (int) x - SPECIAL_BLOCK_RADIUS; xx < (int) x + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; xx++)
		{
			for (int yy = (int) y - SPECIAL_BLOCK_RADIUS; yy < (int) y + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; yy++)
			{
				for (int zz = (int) z - SPECIAL_BLOCK_RADIUS; zz < (int) z + SPECIAL_BLOCK_RADIUS && specialBlocksCount < MAX_SPECIAL_BLOCKS_COUNT; zz++)
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

void Zombie::updateSize(bool isBaby)
{
	internalSetSize(isBaby ? .5f : 1.0f);
}

void Zombie::setSize(float w, float h)
{
	bool inited = registeredBBWidth > 0;

	registeredBBWidth = w;
	registeredBBHeight = h;

	if (!inited)
	{
		internalSetSize(1.0f);
	}
}

void Zombie::internalSetSize(float scale)
{
	PathfinderMob::setSize(registeredBBWidth * scale, registeredBBHeight * scale);
}
