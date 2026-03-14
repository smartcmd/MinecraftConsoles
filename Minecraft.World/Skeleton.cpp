#include "stdafx.h"
#include "net.minecraft.world.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.goal.target.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.damagesource.h"
#include "SharedConstants.h"
#include "Skeleton.h"
#include "..\Minecraft.Client\Textures.h"
#include "SoundTypes.h"

Skeleton::Skeleton(Level *level) : Monster( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	bowGoal = new RangedAttackGoal(this, this, 1.0, SharedConstants::TICKS_PER_SECOND * 1, SharedConstants::TICKS_PER_SECOND * 3, 15);
	meleeGoal = new MeleeAttackGoal(this, eTYPE_PLAYER, 1.2, false);

	goalSelector.addGoal(1, new FloatGoal(this));
	goalSelector.addGoal(2, new RestrictSunGoal(this));
	goalSelector.addGoal(3, new FleeSunGoal(this, 1.0));
	goalSelector.addGoal(5, new RandomStrollGoal(this, 1.0));
	goalSelector.addGoal(6, new LookAtPlayerGoal(this, typeid(Player), 8));
	goalSelector.addGoal(6, new RandomLookAroundGoal(this));

	targetSelector.addGoal(1, new HurtByTargetGoal(this, false));
	targetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), 0, true));

	if (level != nullptr && !level->isClientSide) reassessWeaponGoal();
}

Skeleton::~Skeleton()
{
	delete bowGoal;
	delete meleeGoal;
}

void Skeleton::registerAttributes()
{
	Monster::registerAttributes();

	getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25f);
}

void Skeleton::defineSynchedData()
{
	Monster::defineSynchedData();

	entityData->define(DATA_TYPE_ID, static_cast<byte>(TYPE_DEFAULT));
}

bool Skeleton::useNewAi()
{
	return true;
}

int Skeleton::getAmbientSound() 
{
	return eSoundType_MOB_SKELETON_AMBIENT;
}

int Skeleton::getHurtSound()
{
	return eSoundType_MOB_SKELETON_HURT;
}

int Skeleton::getDeathSound() 
{
	return eSoundType_MOB_SKELETON_DEATH;
}

void Skeleton::playStepSound(int xt, int yt, int zt, int t)
{
	playSound(eSoundType_MOB_SKELETON_STEP, 0.15f, 1);
}

bool Skeleton::doHurtTarget(shared_ptr<Entity> target)
{
	if (Monster::doHurtTarget(target))
	{
		if ( (getSkeletonType() == TYPE_WITHER) && target->instanceof(eTYPE_LIVINGENTITY) )
		{
			dynamic_pointer_cast<LivingEntity>(target)->addEffect(new MobEffectInstance(MobEffect::wither->id, SharedConstants::TICKS_PER_SECOND * 10));
		}
		return true;
	}
	return false;
}

MobType Skeleton::getMobType() 
{
	return UNDEAD;
}

void Skeleton::aiStep()
{
	if (level->isDay() && !level->isClientSide)
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
	if (level->isClientSide)
	{
		if (getSkeletonType() == TYPE_WITHER)
		{
			setSize(0.6f * 1.2f, 1.8f * 1.3f);
		}
	}

	Monster::aiStep();
}

void Skeleton::rideTick()
{
	Monster::rideTick();

	if ( riding != nullptr && riding->instanceof(eTYPE_PATHFINDER_MOB) )
	{
		yBodyRot = dynamic_pointer_cast<PathfinderMob>(riding)->yBodyRot;
	}

}

void Skeleton::die(DamageSource *source)
{
	Monster::die(source);
	
	if ( source->getDirectEntity() != nullptr && source->getDirectEntity()->instanceof(eTYPE_ARROW) && source->getEntity() != nullptr && source->getEntity()->instanceof(eTYPE_PLAYER) )
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>( source->getEntity() );

		double xd = player->x - x;
		double zd = player->z - z;
		if (xd * xd + zd * zd >= 50 * 50)
		{
			player->awardStat(GenericStats::snipeSkeleton(), GenericStats::param_snipeSkeleton());
		}
	}
}

int Skeleton::getDeathLoot() 
{
	return Item::arrow->id;
}

void Skeleton::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	if (getSkeletonType() == TYPE_WITHER)
	{
		// drop some arrows
		int count = random->nextInt(3 + playerBonusLevel) - 1;
		for (int i = 0; i < count; i++)
		{
			spawnAtLocation(Item::coal_Id, 1);
		}
	}
	else
	{
		// drop some arrows
		int count = random->nextInt(3 + playerBonusLevel);
		for (int i = 0; i < count; i++)
		{
			spawnAtLocation(Item::arrow_Id, 1);
		}
	}

	// and some bones
	int count = random->nextInt(3 + playerBonusLevel);
	for (int i = 0; i < count; i++)
	{
		spawnAtLocation(Item::bone->id, 1);
	}
}

void Skeleton::dropRareDeathLoot(int rareLootLevel)
{
	if (getSkeletonType() == TYPE_WITHER)
	{
		spawnAtLocation(std::make_shared<ItemInstance>(Item::skull_Id, 1, SkullTileEntity::TYPE_WITHER), 0);
	}
}

void Skeleton::populateDefaultEquipmentSlots()
{
	Monster::populateDefaultEquipmentSlots();

	setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::bow));
}

MobGroupData *Skeleton::finalizeMobSpawn(MobGroupData *groupData, int extraData /*= 0*/) // 4J Added extraData param
{
	groupData = Monster::finalizeMobSpawn(groupData);

	if ( dynamic_cast<HellDimension *>(level->dimension) != nullptr && getRandom()->nextInt(5) > 0)
	{
		goalSelector.addGoal(4, meleeGoal, false);

		setSkeletonType(TYPE_WITHER);
		setEquippedSlot(SLOT_WEAPON, std::make_shared<ItemInstance>(Item::sword_stone));
		getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4);
	}
	else
	{
		goalSelector.addGoal(4, bowGoal, false);

		populateDefaultEquipmentSlots();
		populateDefaultEquipmentEnchantments();
	}

	setCanPickUpLoot(random->nextFloat() < MAX_PICKUP_LOOT_CHANCE * level->getDifficulty(x, y, z));

	if (getCarried(SLOT_HELM) == nullptr)
	{
		if (Calendar::GetMonth() + 1 == 10 && Calendar::GetDayOfMonth() == 31 && random->nextFloat() < 0.25f)
		{
			// Halloween! OooOOo! 25% of all skeletons/zombies can wear pumpkins on their heads.
			setEquippedSlot(SLOT_HELM, std::make_shared<ItemInstance>(random->nextFloat() < 0.1f ? Tile::litPumpkin : Tile::pumpkin));
			dropChances[SLOT_HELM] = 0;
		}
	}
	return groupData;
}

void Skeleton::reassessWeaponGoal()
{
	goalSelector.removeGoal(meleeGoal);
	goalSelector.removeGoal(bowGoal);

	shared_ptr<ItemInstance> carried = getCarriedItem();

	if (carried != nullptr && carried->id == Item::bow_Id)
	{
		goalSelector.addGoal(4, bowGoal, false);
	}
	else
	{
		goalSelector.addGoal(4, meleeGoal, false);
	}
}

void Skeleton::performRangedAttack(shared_ptr<LivingEntity> target, float power)
{
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, dynamic_pointer_cast<LivingEntity>(shared_from_this()), target, 1.60f, 14 - (level->difficulty * 4));
	int damageBonus = EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowBonus->id, getCarriedItem());
	int knockbackBonus = EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowKnockback->id, getCarriedItem());

	arrow->setBaseDamage(power * 2.0f + (random->nextGaussian() * 0.25f + (level->difficulty * 0.11f)));

	if (damageBonus > 0)
	{
		arrow->setBaseDamage(arrow->getBaseDamage() + static_cast<double>(damageBonus) * .5 + .5);
	}
	if (knockbackBonus > 0)
	{
		arrow->setKnockback(knockbackBonus);
	}
	if (EnchantmentHelper::getEnchantmentLevel(Enchantment::arrowFire->id, getCarriedItem()) > 0 || getSkeletonType() == TYPE_WITHER)
	{
		arrow->setOnFire(100);
	}

	playSound(eSoundType_RANDOM_BOW, 1.0f, 1 / (getRandom()->nextFloat() * 0.4f + 0.8f));
	level->addEntity(arrow);
}

int Skeleton::getSkeletonType()
{
	return (int) entityData->getByte(DATA_TYPE_ID);
}

void Skeleton::setSkeletonType(int type)
{
	entityData->set(DATA_TYPE_ID, static_cast<byte>(type));

	fireImmune = type == TYPE_WITHER;
	if (type == TYPE_WITHER)
	{
		setSize(0.6f * 1.2f, 1.8f * 1.3f);
	}
	else
	{
		setSize(0.6f, 1.8f);
	}
}

void Skeleton::readAdditionalSaveData(CompoundTag *tag)
{
	Monster::readAdditionalSaveData(tag);

	if (tag->contains(L"SkeletonType"))
	{
		int value = tag->getByte(L"SkeletonType");
		setSkeletonType(value);
	}

	reassessWeaponGoal();
}

void Skeleton::addAdditonalSaveData(CompoundTag *entityTag)
{
	Monster::addAdditonalSaveData(entityTag);
	entityTag->putByte(L"SkeletonType", static_cast<byte>(getSkeletonType()));
}

void Skeleton::setEquippedSlot(int slot, shared_ptr<ItemInstance> item)
{
	Monster::setEquippedSlot(slot, item);

	if (!level->isClientSide && slot == SLOT_WEAPON)
	{
		reassessWeaponGoal();
	}
}

double Skeleton::getRidingHeight()
{
	return Monster::getRidingHeight() - .5;
}