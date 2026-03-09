#include "stdafx.h"
#include "JavaMath.h"
#include "Mth.h"
#include "net.minecraft.network.packet.h"
#include "net.minecraft.world.level.h"
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
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.scores.h"
#include "com.mojang.nbt.h"
#include "LivingEntity.h"
#include "..\Minecraft.Client\Textures.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.Client\EntityTracker.h"
#include "SoundTypes.h"
#include "BasicTypeContainers.h"
#include "ParticleTypes.h"
#include "GenericStats.h"
#include "ItemEntity.h"

const double LivingEntity::MIN_MOVEMENT_DISTANCE = 0.005;

AttributeModifier *LivingEntity::SPEED_MODIFIER_SPRINTING = (new AttributeModifier(eModifierId_MOB_SPRINTING, 0.3f, AttributeModifier::OPERATION_MULTIPLY_TOTAL))->setSerialize(false);

void LivingEntity::_init()
{
	attributes = nullptr;
	combatTracker = new CombatTracker(this);
	lastEquipment = ItemInstanceArray(5);

	swinging = false;
	swingTime = 0;
	removeArrowTime = 0;
	lastHealth = 0.0f;

	hurtTime = 0;
	hurtDuration = 0;
	hurtDir = 0.0f;
	deathTime = 0;
	attackTime = 0;
	oAttackAnim = attackAnim = 0.0f;

	walkAnimSpeedO = 0.0f;
	walkAnimSpeed = 0.0f;
	walkAnimPos = 0.0f;
	invulnerableDuration = 20;
	oTilt = tilt = 0.0f;
	timeOffs = 0.0f;
	rotA = 0.0f;
	yBodyRot = yBodyRotO = 0.0f;
	yHeadRot = yHeadRotO = 0.0f;
	flyingSpeed = 0.02f;

	lastHurtByPlayer = nullptr;
	lastHurtByPlayerTime = 0;
	dead = false;
	noActionTime = 0;
	oRun = run = 0.0f;
	animStep = animStepO = 0.0f;
	rotOffs = 0.0f;
	deathScore = 0;
	lastHurt = 0.0f;
	jumping = false;

	xxa = 0.0f;
	yya = 0.0f;
	yRotA = 0.0f;
	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;

	effectsDirty = false;

	lastHurtByMob = nullptr;
	lastHurtByMobTimestamp = 0;
	lastHurtMob = nullptr;
	lastHurtMobTimestamp = 0;

	speed = 0.0f;
	noJumpDelay = 0;
	absorptionAmount = 0.0f;
}

LivingEntity::LivingEntity( Level* level) : Entity(level)
{
	MemSect(56);
	_init();
	MemSect(0);

	// 4J Stu - This will not call the correct derived function, so moving to each derived class
	//setHealth(0);
	//registerAttributes();

	blocksBuilding = true;

	rotA = static_cast<float>(Math::random() + 1) * 0.01f;
	setPos(x, y, z);
	timeOffs = static_cast<float>(Math::random()) * 12398;
	yRot = static_cast<float>(Math::random() * PI * 2);
	yHeadRot = yRot;

	footSize = 0.5f;
}

LivingEntity::~LivingEntity()
{
	for(auto& it : activeEffects)
	{
		delete it.second;
	}

	delete attributes;
	delete combatTracker;

	if(lastEquipment.data != nullptr) delete [] lastEquipment.data;
}

void LivingEntity::defineSynchedData()
{
	entityData->define(DATA_EFFECT_COLOR_ID, 0);
	entityData->define(DATA_EFFECT_AMBIENCE_ID, static_cast<byte>(0));
	entityData->define(DATA_ARROW_COUNT_ID, static_cast<byte>(0));
	entityData->define(DATA_HEALTH_ID, 1.0f);
}

void LivingEntity::registerAttributes()
{
	getAttributes()->registerAttribute(SharedMonsterAttributes::MAX_HEALTH);
	getAttributes()->registerAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE);
	getAttributes()->registerAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);

	if (!useNewAi())
	{
		getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.1f);
	}
}

void LivingEntity::checkFallDamage(double ya, bool onGround)
{
	if (!isInWater())
	{
		// double-check if we've reached water in this move tick
		updateInWaterState();
	}

	if (onGround && fallDistance > 0)
	{
		int xt = Mth::floor(x);
		int yt = Mth::floor(y - 0.2f - heightOffset);
		int zt = Mth::floor(z);
		int t = level->getTile(xt, yt, zt);
		if (t == 0)
		{
			int renderShape = level->getTileRenderShape(xt, yt - 1, zt);
			if (renderShape == Tile::SHAPE_FENCE || renderShape == Tile::SHAPE_WALL || renderShape == Tile::SHAPE_FENCE_GATE)
			{
				t = level->getTile(xt, yt - 1, zt);
			}
		}

		if (t > 0)
		{
			Tile::tiles[t]->fallOn(level, xt, yt, zt, shared_from_this(), fallDistance);
		}
	}

	Entity::checkFallDamage(ya, onGround);
}

bool LivingEntity::isWaterMob()
{
	return false;
}

void LivingEntity::baseTick()
{
	oAttackAnim = attackAnim;
	Entity::baseTick();

	if (isAlive() && isInWall())
	{
		hurt(DamageSource::inWall, 1);
	}

	if (isFireImmune() || level->isClientSide) clearFire();
	shared_ptr<Player> thisPlayer = dynamic_pointer_cast<Player>(shared_from_this());
	bool isInvulnerable = (thisPlayer != nullptr && thisPlayer->abilities.invulnerable);

	if (isAlive() && isUnderLiquid(Material::water))
	{
		if(!isWaterMob() && !hasEffect(MobEffect::waterBreathing->id) && !isInvulnerable)
		{
			setAirSupply(decreaseAirSupply(getAirSupply()));
			if (getAirSupply() == -20)
			{
				setAirSupply(0);
				if(canCreateParticles())
				{
					for (int i = 0; i < 8; i++)
					{
						float xo = random->nextFloat() - random->nextFloat();
						float yo = random->nextFloat() - random->nextFloat();
						float zo = random->nextFloat() - random->nextFloat();
						level->addParticle(eParticleType_bubble, x + xo, y + yo, z + zo, xd, yd, zd);
					}
				}
				hurt(DamageSource::drown, 2);
			}
		}

		clearFire();
		if ( !level->isClientSide && isRiding() && riding->instanceof(eTYPE_LIVINGENTITY) )
		{
			ride(nullptr);
		}
	}
	else
	{
		setAirSupply(TOTAL_AIR_SUPPLY);
	}

	oTilt = tilt;

	if (attackTime > 0) attackTime--;
	if (hurtTime > 0) hurtTime--;
	if (invulnerableTime > 0) invulnerableTime--;
	if (getHealth() <= 0)
	{
		tickDeath();
	}

	if (lastHurtByPlayerTime > 0) lastHurtByPlayerTime--;
	else
	{
		// Note - this used to just set to nullptr, but that has to create a new shared_ptr and free an old one, when generally this won't be doing anything at all. This
		// is the lightweight but ugly alternative
		if( lastHurtByPlayer )
		{
			lastHurtByPlayer.reset();
		}
	}
	if (lastHurtMob != nullptr && !lastHurtMob->isAlive())
	{
		lastHurtMob = nullptr;
	}

	// If lastHurtByMob is dead, remove it
	if (lastHurtByMob != nullptr && !lastHurtByMob->isAlive())
	{
		setLastHurtByMob(nullptr);
	}

	// Update effects
	tickEffects();

	animStepO = animStep;

	yBodyRotO = yBodyRot;
	yHeadRotO = yHeadRot;
	yRotO = yRot;
	xRotO = xRot;
}

bool LivingEntity::isBaby()
{
	return false;
}

void LivingEntity::tickDeath()
{
	deathTime++;
	if (deathTime == 20)
	{
		// 4J Stu - Added level->isClientSide check from 1.2 to fix XP orbs being created client side
		if(!level->isClientSide && (lastHurtByPlayerTime > 0 || isAlwaysExperienceDropper()) )
		{
			if (!isBaby() && level->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
			{
				int xpCount = this->getExperienceReward(lastHurtByPlayer);
				while (xpCount > 0)
				{
					int newCount = ExperienceOrb::getExperienceValue(xpCount);
					xpCount -= newCount;
					level->addEntity(std::make_shared<ExperienceOrb>(level, x, y, z, newCount));
				}
			}
		}

		remove();
		for (int i = 0; i < 20; i++)
		{
			double xa = random->nextGaussian() * 0.02;
			double ya = random->nextGaussian() * 0.02;
			double za = random->nextGaussian() * 0.02;
			level->addParticle(eParticleType_explode, x + random->nextFloat() * bbWidth * 2 - bbWidth, y + random->nextFloat() * bbHeight, z + random->nextFloat() * bbWidth * 2 - bbWidth, xa, ya, za);
		}
	}
}

int LivingEntity::decreaseAirSupply(int currentSupply)
{
	int oxygenBonus = EnchantmentHelper::getOxygenBonus(dynamic_pointer_cast<LivingEntity>(shared_from_this()));
	if (oxygenBonus > 0)
	{
		if (random->nextInt(oxygenBonus + 1) > 0)
		{
			// the oxygen bonus prevents us from drowning
			return currentSupply;
		}
	}
	if(instanceof(eTYPE_PLAYER))
	{
		app.DebugPrintf("++++++++++ %s: Player decreasing air supply to %d\n", level->isClientSide ? "CLIENT" : "SERVER", currentSupply - 1 );
	}
	return currentSupply - 1;
}

int LivingEntity::getExperienceReward(shared_ptr<Player> killedBy)
{
	return 0;
}

bool LivingEntity::isAlwaysExperienceDropper()
{
	return false;
}

Random *LivingEntity::getRandom()
{
	return random;
}

shared_ptr<LivingEntity> LivingEntity::getLastHurtByMob()
{
	return lastHurtByMob;
}

int LivingEntity::getLastHurtByMobTimestamp()
{
	return lastHurtByMobTimestamp;
}

void LivingEntity::setLastHurtByMob(shared_ptr<LivingEntity> target)
{
	lastHurtByMob = target;
	lastHurtByMobTimestamp = tickCount;
}

shared_ptr<LivingEntity> LivingEntity::getLastHurtMob()
{
	return lastHurtMob;
}

int LivingEntity::getLastHurtMobTimestamp()
{
	return lastHurtMobTimestamp;
}

void LivingEntity::setLastHurtMob(shared_ptr<Entity> target)
{
	if ( target->instanceof(eTYPE_LIVINGENTITY) )
	{
		lastHurtMob = dynamic_pointer_cast<LivingEntity>(target);
	}
	else
	{
		lastHurtMob = nullptr;
	}
	lastHurtMobTimestamp = tickCount;
}

int LivingEntity::getNoActionTime()
{
	return noActionTime;
}

void LivingEntity::addAdditonalSaveData(CompoundTag *entityTag)
{
	entityTag->putFloat(L"HealF", getHealth());
	entityTag->putShort(L"Health", static_cast<short>(ceil(getHealth())));
	entityTag->putShort(L"HurtTime", static_cast<short>(hurtTime));
	entityTag->putShort(L"DeathTime", static_cast<short>(deathTime));
	entityTag->putShort(L"AttackTime", static_cast<short>(attackTime));
	entityTag->putFloat(L"AbsorptionAmount", getAbsorptionAmount());

	ItemInstanceArray items = getEquipmentSlots();
	for (unsigned int i = 0; i < items.length; ++i)
	{
		shared_ptr<ItemInstance> item = items[i];
		if (item != nullptr)
		{
			attributes->removeItemModifiers(item);
		}
	}

	entityTag->put(L"Attributes", SharedMonsterAttributes::saveAttributes(getAttributes()));

	for (unsigned int i = 0; i < items.length; ++i)
	{
		shared_ptr<ItemInstance> item = items[i];
		if (item != nullptr)
		{
			attributes->addItemModifiers(item);
		}
	}

	if (!activeEffects.empty())
	{
		ListTag<CompoundTag> *listTag = new ListTag<CompoundTag>();

		for(auto & it : activeEffects)
		{
			MobEffectInstance *effect = it.second;
			listTag->add(effect->save(new CompoundTag()));
		}
		entityTag->put(L"ActiveEffects", listTag);
	}
}

void LivingEntity::readAdditionalSaveData(CompoundTag *tag)
{
	setAbsorptionAmount(tag->getFloat(L"AbsorptionAmount"));

	if (tag->contains(L"Attributes") && level != nullptr && !level->isClientSide)
	{
		SharedMonsterAttributes::loadAttributes(getAttributes(), (ListTag<CompoundTag> *) tag->getList(L"Attributes"));
	}

	if (tag->contains(L"ActiveEffects"))
	{
		ListTag<CompoundTag> *effects = (ListTag<CompoundTag> *) tag->getList(L"ActiveEffects");
		for (int i = 0; i < effects->size(); i++)
		{
			CompoundTag *effectTag = effects->get(i);
			MobEffectInstance *effect = MobEffectInstance::load(effectTag);
			activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( effect->getId(), effect ) );
		}
	}

	if (tag->contains(L"HealF"))
	{
		setHealth( tag->getFloat(L"HealF") );
	}
	else
	{
		Tag *healthTag = tag->get(L"Health");
		if (healthTag == nullptr)
		{
			setHealth(getMaxHealth());
		}
		else if (healthTag->getId() == Tag::TAG_Float)
		{
			setHealth(static_cast<FloatTag *>(healthTag)->data);
		}
		else if (healthTag->getId() == Tag::TAG_Short)
		{
			// pre-1.6 health
			setHealth((float) static_cast<ShortTag *>(healthTag)->data);
		}
	}

	hurtTime = tag->getShort(L"HurtTime");
	deathTime = tag->getShort(L"DeathTime");
	attackTime = tag->getShort(L"AttackTime");
}

void LivingEntity::tickEffects()
{
	bool removed = false;
    for (auto it = activeEffects.begin(); it != activeEffects.end();)
    {
		MobEffectInstance *effect = it->second;
		removed = false;
		if (!effect->tick(dynamic_pointer_cast<LivingEntity>(shared_from_this())))
		{
			if (!level->isClientSide)
			{
				it = activeEffects.erase( it );
				onEffectRemoved(effect);
				delete effect;
				removed = true;
			}
		}
		else if (effect->getDuration() % (SharedConstants::TICKS_PER_SECOND * 30) == 0)
		{
			// update effects every 30 seconds to synchronize client-side
			// timer
			onEffectUpdated(effect, false);
		}
		if(!removed)
		{
			++it;
		}
	}
	if (effectsDirty)
	{
		if (!level->isClientSide)
		{
			if (activeEffects.empty())
			{
				entityData->set(DATA_EFFECT_AMBIENCE_ID, static_cast<byte>(0));
				entityData->set(DATA_EFFECT_COLOR_ID, 0);
				setInvisible(false);
				setWeakened(false);
			}
			else
			{
				vector<MobEffectInstance *> values;
				for(auto& it : activeEffects)
				{
					values.push_back(it.second);
				}
				int colorValue = PotionBrewing::getColorValue(&values);
				entityData->set(DATA_EFFECT_AMBIENCE_ID, PotionBrewing::areAllEffectsAmbient(&values) ? static_cast<byte>(1) : static_cast<byte>(0));
				values.clear();
				entityData->set(DATA_EFFECT_COLOR_ID, colorValue);
				setInvisible(hasEffect(MobEffect::invisibility->id));
				setWeakened(hasEffect(MobEffect::weakness->id));
			}
		}
		effectsDirty = false;
	}
	int colorValue = entityData->getInteger(DATA_EFFECT_COLOR_ID);
	bool ambient = entityData->getByte(DATA_EFFECT_AMBIENCE_ID) > 0;

	if (colorValue > 0)
	{
		boolean doParticle = false;

		if (!isInvisible())
		{
			doParticle = random->nextBoolean();
		}
		else
		{
			// much fewer particles when invisible
			doParticle = random->nextInt(15) == 0;
		}

		if (ambient) doParticle &= random->nextInt(5) == 0;

		if (doParticle)
		{
			//                int colorValue = entityData.getInteger(DATA_EFFECT_COLOR_ID);
			if (colorValue > 0)
			{
				double red = static_cast<double>((colorValue >> 16) & 0xff) / 255.0;
				double green = static_cast<double>((colorValue >> 8) & 0xff) / 255.0;
				double blue = static_cast<double>((colorValue >> 0) & 0xff) / 255.0;

				level->addParticle(ambient? eParticleType_mobSpellAmbient : eParticleType_mobSpell, x + (random->nextDouble() - 0.5) * bbWidth, y + random->nextDouble() * bbHeight - heightOffset, z + (random->nextDouble() - 0.5) * bbWidth, red, green, blue);
			}
		}
	}
}

void LivingEntity::removeAllEffects()
{
    for (auto it = activeEffects.begin(); it != activeEffects.end();)
    {
		MobEffectInstance *effect = it->second;//activeEffects.get(effectId);

		if (!level->isClientSide)
		{
			it = activeEffects.erase(it);
			onEffectRemoved(effect);
			delete effect;
		}
		else
		{
			++it;
		}
	}
}

vector<MobEffectInstance *> *LivingEntity::getActiveEffects()
{
	vector<MobEffectInstance *> *active = new vector<MobEffectInstance *>();

	for(auto& it : activeEffects)
	{
		active->push_back(it.second);
	}

	return active;
}

bool LivingEntity::hasEffect(int id)
{
	return activeEffects.find(id) != activeEffects.end();;
}

bool LivingEntity::hasEffect(MobEffect *effect)
{
	return activeEffects.find(effect->id) != activeEffects.end();
}

MobEffectInstance *LivingEntity::getEffect(MobEffect *effect)
{
	MobEffectInstance *effectInst = nullptr;

    auto it = activeEffects.find(effect->id);
    if(it != activeEffects.end() ) effectInst = it->second;

	return effectInst;
}

void LivingEntity::addEffect(MobEffectInstance *newEffect)
{
	if (!canBeAffected(newEffect))
	{
		return;
	}

	if (activeEffects.find(newEffect->getId()) != activeEffects.end() )
	{
		// replace effect and update
		MobEffectInstance *effectInst = activeEffects.find(newEffect->getId())->second;
		effectInst->update(newEffect);
		onEffectUpdated(effectInst, true);
	}
	else
	{
		activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( newEffect->getId(), newEffect ) );
		onEffectAdded(newEffect);
	}
}

// 4J Added
void LivingEntity::addEffectNoUpdate(MobEffectInstance *newEffect)
{
	if (!canBeAffected(newEffect))
	{
		return;
	}

	if (activeEffects.find(newEffect->getId()) != activeEffects.end() )
	{
		// replace effect and update
		MobEffectInstance *effectInst = activeEffects.find(newEffect->getId())->second;
		effectInst->update(newEffect);
	}
	else
	{
		activeEffects.insert( unordered_map<int, MobEffectInstance *>::value_type( newEffect->getId(), newEffect ) );
	}
}

bool LivingEntity::canBeAffected(MobEffectInstance *newEffect)
{
	if (getMobType() == UNDEAD)
	{
		int id = newEffect->getId();
		if (id == MobEffect::regeneration->id || id == MobEffect::poison->id)
		{
			return false;
		}
	}

	return true;
}

bool LivingEntity::isInvertedHealAndHarm()
{
	return getMobType() == UNDEAD;
}

void LivingEntity::removeEffectNoUpdate(int effectId)
{
    auto it = activeEffects.find(effectId);
    if (it != activeEffects.end())
	{
		MobEffectInstance *effect = it->second;
		if(effect != nullptr)
		{
			delete effect;
		}
		activeEffects.erase(it);
	}
}

void LivingEntity::removeEffect(int effectId)
{
    auto it = activeEffects.find(effectId);
    if (it != activeEffects.end())
	{
		MobEffectInstance *effect = it->second;
		if(effect != nullptr)
		{
			onEffectRemoved(effect);
			delete effect;
		}
		activeEffects.erase(it);
	}
}

void LivingEntity::onEffectAdded(MobEffectInstance *effect)
{
	effectsDirty = true;
	if (!level->isClientSide) MobEffect::effects[effect->getId()]->addAttributeModifiers(dynamic_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(), effect->getAmplifier());
}

void LivingEntity::onEffectUpdated(MobEffectInstance *effect, bool doRefreshAttributes)
{
	effectsDirty = true;
	if (doRefreshAttributes && !level->isClientSide)
	{
		MobEffect::effects[effect->getId()]->removeAttributeModifiers(dynamic_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(), effect->getAmplifier());
		MobEffect::effects[effect->getId()]->addAttributeModifiers(dynamic_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(), effect->getAmplifier());
	}
}

void LivingEntity::onEffectRemoved(MobEffectInstance *effect)
{
	effectsDirty = true;
	if (!level->isClientSide) MobEffect::effects[effect->getId()]->removeAttributeModifiers(dynamic_pointer_cast<LivingEntity>(shared_from_this()), getAttributes(), effect->getAmplifier());
}

void LivingEntity::heal(float heal)
{
	float health = getHealth();
	if (health > 0)
	{
		setHealth(health + heal);
	}
}

float LivingEntity::getHealth()
{
	return entityData->getFloat(DATA_HEALTH_ID);
}

void LivingEntity::setHealth(float health)
{
	entityData->set(DATA_HEALTH_ID, Mth::clamp(health, 0.0f, getMaxHealth()));
}

bool LivingEntity::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;

	// 4J Stu - Reworked this function a bit to show hurt damage on the client before the server responds.
	// Fix for #8823 - Gameplay: Confirmation that a monster or animal has taken damage from an attack is highly delayed
	// 4J Stu - Change to the fix to only show damage when attacked, rather than collision damage
	// Fix for #10299 - When in corners, passive mobs may show that they are taking damage.
	// 4J Stu - Change to the fix for TU6, as source is never nullptr due to changes in 1.8.2 to what source actually is
	if (level->isClientSide && dynamic_cast<EntityDamageSource *>(source) == nullptr) return false;
	noActionTime = 0;
	if (getHealth() <= 0) return false;

	if ( source->isFire() && hasEffect(MobEffect::fireResistance) )
	{
		// 4J-JEV, for new achievement Stayin'Frosty, TODO merge with Java version.
		if ( this->instanceof(eTYPE_PLAYER) && (source == DamageSource::lava) ) // Only award when in lava (not any fire).
		{
			shared_ptr<Player> plr = dynamic_pointer_cast<Player>(shared_from_this());
			plr->awardStat(GenericStats::stayinFrosty(),GenericStats::param_stayinFrosty());
		}
		return false;
	}

	if ((source == DamageSource::anvil || source == DamageSource::fallingBlock) && getCarried(SLOT_HELM) != nullptr)
	{
		getCarried(SLOT_HELM)->hurtAndBreak(static_cast<int>(dmg * 4 + random->nextFloat() * dmg * 2.0f), dynamic_pointer_cast<LivingEntity>( shared_from_this() ));
		dmg *= 0.75f;
	}

	walkAnimSpeed = 1.5f;

	bool sound = true;
	if (invulnerableTime > invulnerableDuration / 2.0f)
	{
		if (dmg <= lastHurt) return false;
		if(!level->isClientSide) actuallyHurt(source, dmg - lastHurt);
		lastHurt = dmg;
		sound = false;
	}
	else
	{
		lastHurt = dmg;
		lastHealth = getHealth();
		invulnerableTime = invulnerableDuration;
		if (!level->isClientSide) actuallyHurt(source, dmg);
		hurtTime = hurtDuration = 10;
	}

	hurtDir = 0;

	shared_ptr<Entity> sourceEntity = source->getEntity();
	if (sourceEntity != nullptr)
	{
		if ( sourceEntity->instanceof(eTYPE_LIVINGENTITY) )
		{
			setLastHurtByMob(dynamic_pointer_cast<LivingEntity>(sourceEntity));
		}

		if ( sourceEntity->instanceof(eTYPE_PLAYER) )
		{
			lastHurtByPlayerTime = PLAYER_HURT_EXPERIENCE_TIME;
			lastHurtByPlayer = dynamic_pointer_cast<Player>(sourceEntity);
		}
		else if ( sourceEntity->instanceof(eTYPE_WOLF) )
		{
			shared_ptr<Wolf> w = dynamic_pointer_cast<Wolf>(sourceEntity);
			if (w->isTame())
			{
				lastHurtByPlayerTime = PLAYER_HURT_EXPERIENCE_TIME;
				lastHurtByPlayer = nullptr;
			}
		}
	}

	if (sound && level->isClientSide)
	{
		return false;
	}

	if (sound)
	{
		level->broadcastEntityEvent(shared_from_this(), EntityEvent::HURT);
		if (source != DamageSource::drown) markHurt();
		if (sourceEntity != nullptr)
		{
			double xd = sourceEntity->x - x;
			double zd = sourceEntity->z - z;
			while (xd * xd + zd * zd < 0.0001)
			{
				xd = (Math::random() - Math::random()) * 0.01;
				zd = (Math::random() - Math::random()) * 0.01;
			}
			hurtDir = static_cast<float>(atan2(zd, xd) * 180 / PI) - yRot;
			knockback(sourceEntity, dmg, xd, zd);
		}
		else
		{
			hurtDir = static_cast<float>((int)((Math::random() * 2) * 180)); // 4J This cast is the same as Java
		}
	}

	MemSect(31);
	if (getHealth() <= 0)
	{
		if (sound) playSound(getDeathSound(), getSoundVolume(), getVoicePitch());
		die(source);
	}
	else
	{
		if (sound) playSound(getHurtSound(), getSoundVolume(), getVoicePitch());
	}
	MemSect(0);

	return true;
}

void LivingEntity::breakItem(shared_ptr<ItemInstance> itemInstance)
{
	playSound(eSoundType_RANDOM_BREAK, 0.8f, 0.8f + level->random->nextFloat() * 0.4f);

	for (int i = 0; i < 5; i++)
	{
		Vec3 *d = Vec3::newTemp((random->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0);
		d->xRot(-xRot * PI / 180);
		d->yRot(-yRot * PI / 180);

		Vec3 *p = Vec3::newTemp((random->nextFloat() - 0.5) * 0.3, -random->nextFloat() * 0.6 - 0.3, 0.6);
		p->xRot(-xRot * PI / 180);
		p->yRot(-yRot * PI / 180);
		p = p->add(x, y + getHeadHeight(), z);
		level->addParticle(PARTICLE_ICONCRACK(itemInstance->getItem()->id,0), p->x, p->y, p->z, d->x, d->y + 0.05, d->z);
	}
}

void LivingEntity::die(DamageSource *source)
{
	shared_ptr<Entity> sourceEntity = source->getEntity();
	shared_ptr<LivingEntity> killer = getKillCredit();
	if (deathScore >= 0 && killer != nullptr) killer->awardKillScore(shared_from_this(), deathScore);

	if (sourceEntity != nullptr) sourceEntity->killed( dynamic_pointer_cast<LivingEntity>( shared_from_this() ) );

	dead = true;

	if (!level->isClientSide)
	{
		int playerBonus = 0;

		shared_ptr<Player> player = nullptr;
		if ( (sourceEntity != nullptr) && sourceEntity->instanceof(eTYPE_PLAYER) )
		{
			player = dynamic_pointer_cast<Player>(sourceEntity);
			playerBonus = EnchantmentHelper::getKillingLootBonus(dynamic_pointer_cast<LivingEntity>(player));
		}

		if (!isBaby() && level->getGameRules()->getBoolean(GameRules::RULE_DOMOBLOOT))
		{
			dropDeathLoot(lastHurtByPlayerTime > 0, playerBonus);
			dropEquipment(lastHurtByPlayerTime > 0, playerBonus);
			if (lastHurtByPlayerTime > 0)
			{
				int rareLoot = random->nextInt(200) - playerBonus;
				if (rareLoot < 5)
				{
					dropRareDeathLoot((rareLoot <= 0) ? 1 : 0);
				}
			}
		}

		// 4J-JEV, hook for Durango mobKill event.
		if (player != nullptr)
		{
			player->awardStat(GenericStats::killMob(),GenericStats::param_mobKill(player, dynamic_pointer_cast<Mob>(shared_from_this()), source));
		}
	}

	level->broadcastEntityEvent(shared_from_this(), EntityEvent::DEATH);
}

void LivingEntity::dropEquipment(bool byPlayer, int playerBonusLevel)
{
}

void LivingEntity::knockback(shared_ptr<Entity> source, float dmg, double xd, double zd)
{
	if (random->nextDouble() < getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->getValue())
	{
		return;
	}

	hasImpulse = true;
	float dd = Mth::sqrt(xd * xd + zd * zd);
	float pow = 0.4f;

	this->xd /= 2;
	yd /= 2;
	this->zd /= 2;

	this->xd -= xd / dd * pow;
	yd += pow;
	this->zd -= zd / dd * pow;

	if (yd > 0.4f) yd = 0.4f;
}

int LivingEntity::getHurtSound()
{
	return eSoundType_DAMAGE_HURT;
}

int LivingEntity::getDeathSound()
{
	return eSoundType_DAMAGE_HURT;
}

/**
* Drop extra rare loot. Only occurs roughly 5% of the time, rareRootLevel
* is set to 1 (otherwise 0) 1% of the time.
*
* @param rareLootLevel
*/
void LivingEntity::dropRareDeathLoot(int rareLootLevel)
{

}

void LivingEntity::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
}

bool LivingEntity::onLadder()
{
	int xt = Mth::floor(x);
	int yt = Mth::floor(bb->y0);
	int zt = Mth::floor(z);

	// 4J-PB - TU9 - add climbable vines
	int iTile = level->getTile(xt, yt, zt);
	return  (iTile== Tile::ladder_Id) || (iTile== Tile::vine_Id);
}

bool LivingEntity::isShootable()
{
	return true;
}

bool LivingEntity::isAlive()
{
	return !removed && getHealth() > 0;
}

void LivingEntity::causeFallDamage(float distance)
{
	Entity::causeFallDamage(distance);
	MobEffectInstance *jumpBoost = getEffect(MobEffect::jump);
	float padding = jumpBoost != nullptr ? jumpBoost->getAmplifier() + 1 : 0;

	int dmg = static_cast<int>(ceil(distance - 3 - padding));
	if (dmg > 0)
	{
		// 4J - new sounds here brought forward from 1.2.3
		if (dmg > 4)
		{
			playSound(eSoundType_DAMAGE_FALL_BIG, 1, 1);
		}
		else
		{
			playSound(eSoundType_DAMAGE_FALL_SMALL, 1, 1);
		}
		hurt(DamageSource::fall, dmg);

		int t = level->getTile( Mth::floor(x), Mth::floor(y - 0.2f - this->heightOffset), Mth::floor(z));
		if (t > 0)
		{
			const Tile::SoundType *soundType = Tile::tiles[t]->soundType;
			MemSect(31);
			playSound(soundType->getStepSound(), soundType->getVolume() * 0.5f, soundType->getPitch() * 0.75f);
			MemSect(0);
		}
	}
}

void LivingEntity::animateHurt()
{
	hurtTime = hurtDuration = 10;
	hurtDir = 0;
}

/**
* Fetches the mob's armor value, from 0 (no armor) to 20 (full armor)
*
* @return
*/
int LivingEntity::getArmorValue()
{
	int val = 0;
	ItemInstanceArray items = getEquipmentSlots();
	for (unsigned int i = 0; i < items.length; ++i)
	{
		shared_ptr<ItemInstance> item = items[i];
		if (item != nullptr && dynamic_cast<ArmorItem *>(item->getItem()) != nullptr)
		{
			int baseProtection = static_cast<ArmorItem *>(item->getItem())->defense;
			val += baseProtection;
		}
	}
	return val;
}

void LivingEntity::hurtArmor(float damage)
{
}

float LivingEntity::getDamageAfterArmorAbsorb(DamageSource *damageSource, float damage)
{
	if (!damageSource->isBypassArmor())
	{
		int absorb = 25 - getArmorValue();
		float v = (damage) * absorb;
		hurtArmor(damage);
		damage = v / 25;
	}
	return damage;
}

float LivingEntity::getDamageAfterMagicAbsorb(DamageSource *damageSource, float damage)
{
	// [EB]: Stupid hack :(
	if ( this->instanceof(eTYPE_ZOMBIE) )
	{
		damage = damage;
	}
	if (hasEffect(MobEffect::damageResistance) && damageSource != DamageSource::outOfWorld)
	{
		int absorbValue = (getEffect(MobEffect::damageResistance)->getAmplifier() + 1) * 5;
		int absorb = 25 - absorbValue;
		float v = (damage) * absorb;
		damage = v / 25;
	}

	if (damage <= 0) return 0;

	int enchantmentArmor = EnchantmentHelper::getDamageProtection(getEquipmentSlots(), damageSource);
	if (enchantmentArmor > 20)
	{
		enchantmentArmor = 20;
	}
	if (enchantmentArmor > 0 && enchantmentArmor <= 20)
	{
		int absorb = 25 - enchantmentArmor;
		float v = damage * absorb;
		damage = v / 25;
	}

	return damage;
}

void LivingEntity::actuallyHurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return;
	dmg = getDamageAfterArmorAbsorb(source, dmg);
	dmg = getDamageAfterMagicAbsorb(source, dmg);

	float originalDamage = dmg;
	dmg = max(dmg - getAbsorptionAmount(), 0.0f);
	setAbsorptionAmount(getAbsorptionAmount() - (originalDamage - dmg));
	if (dmg == 0) return;

	float oldHealth = getHealth();
	setHealth(oldHealth - dmg);
	getCombatTracker()->recordDamage(source, oldHealth, dmg);
	setAbsorptionAmount(getAbsorptionAmount() - dmg);
}

CombatTracker *LivingEntity::getCombatTracker()
{
	return combatTracker;
}

shared_ptr<LivingEntity> LivingEntity::getKillCredit()
{
	if (combatTracker->getKiller() != nullptr) return combatTracker->getKiller();
	if (lastHurtByPlayer != nullptr) return lastHurtByPlayer;
	if (lastHurtByMob != nullptr) return lastHurtByMob;
	return nullptr;
}

float LivingEntity::getMaxHealth()
{
	return static_cast<float>(getAttribute(SharedMonsterAttributes::MAX_HEALTH)->getValue());
}

int LivingEntity::getArrowCount()
{
	return entityData->getByte(DATA_ARROW_COUNT_ID);
}

void LivingEntity::setArrowCount(int count)
{
	entityData->set(DATA_ARROW_COUNT_ID, static_cast<byte>(count));
}

int LivingEntity::getCurrentSwingDuration()
{
	if (hasEffect(MobEffect::digSpeed))
	{
		return SWING_DURATION - (1 + getEffect(MobEffect::digSpeed)->getAmplifier()) * 1;
	}
	if (hasEffect(MobEffect::digSlowdown))
	{
		return SWING_DURATION + (1 + getEffect(MobEffect::digSlowdown)->getAmplifier()) * 2;
	}
	return SWING_DURATION;
}

void LivingEntity::swing()
{
	if (!swinging || swingTime >= getCurrentSwingDuration() / 2 || swingTime < 0)
	{
		swingTime = -1;
		swinging = true;

		if (dynamic_cast<ServerLevel *>(level) != nullptr)
		{
			static_cast<ServerLevel *>(level)->getTracker()->broadcast(shared_from_this(), std::make_shared<AnimatePacket>(shared_from_this(), AnimatePacket::SWING));
		}
	}
}

void LivingEntity::handleEntityEvent(byte id)
{
	if (id == EntityEvent::HURT)
	{
		walkAnimSpeed = 1.5f;

		invulnerableTime = invulnerableDuration;
		hurtTime = hurtDuration = 10;
		hurtDir = 0;

		MemSect(31);
		// 4J-PB -added because villagers have no sounds
		int iHurtSound=getHurtSound();
		if(iHurtSound!=-1)
		{
			playSound(iHurtSound, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		}
		MemSect(0);
		hurt(DamageSource::genericSource, 0);
	}
	else if (id == EntityEvent::DEATH)
	{
		MemSect(31);
		// 4J-PB -added because villagers have no sounds
		int iDeathSound=getDeathSound();
		if(iDeathSound!=-1)
		{
			playSound(iDeathSound, getSoundVolume(), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		}
		MemSect(0);
		setHealth(0);
		die(DamageSource::genericSource);
	}
	else
	{
		Entity::handleEntityEvent(id);
	}
}

void LivingEntity::outOfWorld()
{
	hurt(DamageSource::outOfWorld, 4);
}

void LivingEntity::updateSwingTime()
{
	int currentSwingDuration = getCurrentSwingDuration();
	if (swinging)
	{
		swingTime++;
		if (swingTime >= currentSwingDuration)
		{
			swingTime = 0;
			swinging = false;
		}
	}
	else
	{
		swingTime = 0;
	}

	attackAnim = swingTime / static_cast<float>(currentSwingDuration);
}

AttributeInstance *LivingEntity::getAttribute(Attribute *attribute)
{
	return getAttributes()->getInstance(attribute);
}

BaseAttributeMap *LivingEntity::getAttributes()
{
	if (attributes == nullptr)
	{
		attributes = new ServersideAttributeMap();
	}

	return attributes;
}

MobType LivingEntity::getMobType()
{
	return UNDEFINED;
}

void LivingEntity::setSprinting(bool value)
{
	Entity::setSprinting(value);

	AttributeInstance *speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
	if (speed->getModifier(eModifierId_MOB_SPRINTING) != nullptr)
	{
		speed->removeModifier(eModifierId_MOB_SPRINTING);
	}
	if (value)
	{
		speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_SPRINTING));
	}
}

float LivingEntity::getSoundVolume()
{
	return 1;
}

float LivingEntity::getVoicePitch()
{
	if (isBaby())
	{
		return (random->nextFloat() - random->nextFloat()) * 0.2f + 1.5f;

	}
	return (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f;
}

bool LivingEntity::isImmobile()
{
	return getHealth() <= 0;
}

void LivingEntity::teleportTo(double x, double y, double z)
{
	moveTo(x, y, z, yRot, xRot);
}

void LivingEntity::findStandUpPosition(shared_ptr<Entity> vehicle)
{
    const double vehicleX = vehicle->x;
    const double vehicleY = vehicle->bb->y0 + vehicle->bbHeight;
    const double vehicleZ = vehicle->z;
    double fallbackX = vehicleX;
    double fallbackY = vehicleY;
    double fallbackZ = vehicleZ;
    const double searchY = vehicleY;

    for (double xDiff = -1.5; xDiff < 2; xDiff += 1.5)
    {
        for (double zDiff = -1.5; zDiff < 2; zDiff += 1.5)
        {
            if (xDiff == 0 && zDiff == 0)
            {
                continue;
            }

            const int xToInt = static_cast<int>(vehicleX + xDiff);
            const int zToInt = static_cast<int>(vehicleZ + zDiff);
            AABB *boundingBox = bb->cloneMove(vehicleX + xDiff - x, searchY + 1 - y, vehicleZ + zDiff - z);

            if (level->getTileCubes(boundingBox, true)->empty())
            {
                if (level->isTopSolidBlocking(xToInt, static_cast<int>(searchY), zToInt))
                {
                    teleportTo(vehicleX + xDiff, searchY + 1, vehicleZ + zDiff);
                    return;
                }
                if (level->isTopSolidBlocking(xToInt, static_cast<int>(searchY) - 1, zToInt) ||
                    level->getMaterial(xToInt, static_cast<int>(searchY) - 1, zToInt) == Material::water)
                {
                    fallbackX = vehicleX + xDiff;
                    fallbackY = searchY + 1;
                    fallbackZ = vehicleZ + zDiff;
                }
            }
        }
    }

    teleportTo(fallbackX, fallbackY, fallbackZ);
}

bool LivingEntity::shouldShowName()
{
	return false;
}

Icon *LivingEntity::getItemInHandIcon(shared_ptr<ItemInstance> item, int layer)
{
	return item->getIcon();
}

void LivingEntity::jumpFromGround()
{
	yd = 0.42f;
	if (hasEffect(MobEffect::jump))
	{
		yd += (getEffect(MobEffect::jump)->getAmplifier() + 1) * .1f;
	}
	if (isSprinting())
	{
		float rr = yRot * Mth::RAD_TO_GRAD;

		xd -= Mth::sin(rr) * 0.2f;
		zd += Mth::cos(rr) * 0.2f;
	}
	this->hasImpulse = true;
}

void LivingEntity::travel(float xa, float ya)
{
#ifdef __PSVITA__
	// AP - dynamic_pointer_cast is a non-trivial call
	Player *thisPlayer = nullptr;
	if( this->instanceof(eTYPE_PLAYER) )
	{
		thisPlayer = (Player*) this;
	}
#else
	shared_ptr<Player> thisPlayer = dynamic_pointer_cast<Player>(shared_from_this());
#endif
	if (isInWater() && !(thisPlayer && thisPlayer->abilities.flying) )
	{
		double yo = y;
		moveRelative(xa, ya, useNewAi() ? 0.04f : 0.02f);
		move(xd, yd, zd);

		xd *= 0.80f;
		yd *= 0.80f;
		zd *= 0.80f;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6f - y + yo, zd))
		{
			yd = 0.3f;
		}
	}
	else if (isInLava() && !(thisPlayer && thisPlayer->abilities.flying) )
	{
		double yo = y;
		moveRelative(xa, ya, 0.02f);
		move(xd, yd, zd);
		xd *= 0.50f;
		yd *= 0.50f;
		zd *= 0.50f;
		yd -= 0.02;

		if (horizontalCollision && isFree(xd, yd + 0.6f - y + yo, zd))
		{
			yd = 0.3f;
		}
	}
	else
	{
		float friction = 0.91f;
		if (onGround)
		{
			friction = 0.6f * 0.91f;
			int t = level->getTile(Mth::floor(x), Mth::floor(bb->y0) - 1, Mth::floor(z));
			if (t > 0)
			{
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}

		float friction2 = (0.6f * 0.6f * 0.91f * 0.91f * 0.6f * 0.91f) / (friction * friction * friction);

		float speed;
		if (onGround)
		{
			speed = getSpeed() * friction2;
		}
		else
		{
			speed = flyingSpeed;
		}

		moveRelative(xa, ya, speed);

		friction = 0.91f;
		if (onGround)
		{
			friction = 0.6f * 0.91f;
			int t = level->getTile( Mth::floor(x), Mth::floor(bb->y0) - 1, Mth::floor(z));
			if (t > 0)
			{
				friction = Tile::tiles[t]->friction * 0.91f;
			}
		}
		if (onLadder())
		{
			float max = 0.15f;
			if (xd < -max) xd = -max;
			if (xd > max) xd = max;
			if (zd < -max) zd = -max;
			if (zd > max) zd = max;
			fallDistance = 0;
			if (yd < -0.15) yd = -0.15;
			bool playerSneaking = isSneaking() && this->instanceof(eTYPE_PLAYER);
			if (playerSneaking && yd < 0) yd = 0;
		}

		move(xd, yd, zd);

		if (horizontalCollision && onLadder())
		{
			yd = 0.2;
		}

		if (!level->isClientSide || (level->hasChunkAt(static_cast<int>(x), 0, static_cast<int>(z)) && level->getChunkAt(static_cast<int>(x), static_cast<int>(z))->loaded))
		{
			yd -= 0.08;
		}
		else if (y > 0)
		{
			yd = -0.1;
		}
		else
		{
			yd = 0;
		}

		yd *= 0.98f;
		xd *= friction;
		zd *= friction;
	}

	walkAnimSpeedO = walkAnimSpeed;
	double xxd = x - xo;
	double zzd = z - zo;
	float wst = Mth::sqrt(xxd * xxd + zzd * zzd) * 4;
	if (wst > 1) wst = 1;
	walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
	walkAnimPos += walkAnimSpeed;
}

// 4J - added for more accurate lighting of mobs. Takes a weighted average of all tiles touched by the bounding volume of the entity - the method in the Entity class (which used to be used for
// mobs too) simply gets a single tile's lighting value causing sudden changes of lighting values when entities go in and out of lit areas, for example when bobbing in the water.
int LivingEntity::getLightColor(float a)
{
	float accum[2] = {0,0};
	float totVol = ( bb->x1 - bb->x0 ) * ( bb->y1 - bb->y0 ) * ( bb->z1 - bb->z0 );
	int xmin = Mth::floor(bb->x0);
	int xmax = Mth::floor(bb->x1);
	int ymin = Mth::floor(bb->y0);
	int ymax = Mth::floor(bb->y1);
	int zmin = Mth::floor(bb->z0);
	int zmax = Mth::floor(bb->z1);
	for( int xt = xmin; xt <= xmax; xt++ )
		for( int yt = ymin; yt <= ymax; yt++ )
			for( int zt = zmin; zt <= zmax; zt++ )
			{
				float tilexmin = static_cast<float>(xt);
				float tilexmax = static_cast<float>(xt + 1);
				float tileymin = static_cast<float>(yt);
				float tileymax = static_cast<float>(yt + 1);
				float tilezmin = static_cast<float>(zt);
				float tilezmax = static_cast<float>(zt + 1);
				if( tilexmin < bb->x0 ) tilexmin = bb->x0;
				if( tilexmax > bb->x1 ) tilexmax = bb->x1;
				if( tileymin < bb->y0 ) tileymin = bb->y0;
				if( tileymax > bb->y1 ) tileymax = bb->y1;
				if( tilezmin < bb->z0 ) tilezmin = bb->z0;
				if( tilezmax > bb->z1 ) tilezmax = bb->z1;
				float tileVol = ( tilexmax - tilexmin ) * ( tileymax - tileymin ) * ( tilezmax - tilezmin );
				float frac = tileVol / totVol;
				int lc = level->getLightColor(xt, yt, zt, 0);
				accum[0] += frac * static_cast<float>(lc & 0xffff);
				accum[1] += frac * static_cast<float>(lc >> 16);
			}

			if( accum[0] > 240.0f ) accum[0] = 240.0f;
			if( accum[1] > 240.0f ) accum[1] = 240.0f;

			return ( static_cast<int>(accum[1])<<16) | static_cast<int>(accum[0]);
}

bool LivingEntity::useNewAi()
{
	return false;
}

float LivingEntity::getSpeed()
{
	if (useNewAi())
	{
		return speed;
	}
	else
	{
		return 0.1f;
	}
}

void LivingEntity::setSpeed(float speed)
{
	this->speed = speed;
}

bool LivingEntity::doHurtTarget(shared_ptr<Entity> target)
{
	setLastHurtMob(target);
	return false;
}

bool LivingEntity::isSleeping()
{
	return false;
}

void LivingEntity::tick()
{
	Entity::tick();

	if (!level->isClientSide)
	{
		int arrowCount = getArrowCount();
		if (arrowCount > 0)
		{
			if (removeArrowTime <= 0)
			{
				removeArrowTime = SharedConstants::TICKS_PER_SECOND * (30 - arrowCount);
			}
			removeArrowTime--;
			if (removeArrowTime <= 0)
			{
				setArrowCount(arrowCount - 1);
			}
		}

		for (int i = 0; i < 5; i++)
		{
			shared_ptr<ItemInstance> previous = lastEquipment[i];
			shared_ptr<ItemInstance> current = getCarried(i);

			if (!ItemInstance::matches(current, previous))
			{
				static_cast<ServerLevel *>(level)->getTracker()->broadcast(shared_from_this(), std::make_shared<SetEquippedItemPacket>(entityId, i, current));
				if (previous != nullptr) attributes->removeItemModifiers(previous);
				if (current != nullptr) attributes->addItemModifiers(current);
				lastEquipment[i] = current == nullptr ? nullptr : current->copy();
			}
		}
	}

	aiStep();

	double xd = x - xo;
	double zd = z - zo;

	float sideDist = xd * xd + zd * zd;

	float yBodyRotT = yBodyRot;

	float walkSpeed = 0;
	oRun = run;
	float tRun = 0;
	if (sideDist > 0.05f * 0.05f)
	{
		tRun = 1;
		walkSpeed = sqrt(sideDist) * 3;
		yBodyRotT = (static_cast<float>(atan2(zd, xd)) * 180 / (float) PI - 90);
	}
	if (attackAnim > 0)
	{
		yBodyRotT = yRot;
	}
	if (!onGround)
	{
		tRun = 0;
	}
	run = run + (tRun - run) * 0.3f;

	walkSpeed = tickHeadTurn(yBodyRotT, walkSpeed);

	while (yRot - yRotO < -180)
		yRotO -= 360;
	while (yRot - yRotO >= 180)
		yRotO += 360;

	while (yBodyRot - yBodyRotO < -180)
		yBodyRotO -= 360;
	while (yBodyRot - yBodyRotO >= 180)
		yBodyRotO += 360;

	while (xRot - xRotO < -180)
		xRotO -= 360;
	while (xRot - xRotO >= 180)
		xRotO += 360;

	while (yHeadRot - yHeadRotO < -180)
		yHeadRotO -= 360;
	while (yHeadRot - yHeadRotO >= 180)
		yHeadRotO += 360;

	animStep += walkSpeed;
}

float LivingEntity::tickHeadTurn(float yBodyRotT, float walkSpeed)
{
	float yBodyRotD = Mth::wrapDegrees(yBodyRotT - yBodyRot);
	yBodyRot += yBodyRotD * 0.3f;

	float headDiff = Mth::wrapDegrees(yRot - yBodyRot);
	bool behind = headDiff < -90 || headDiff >= 90;
	if (headDiff < -75) headDiff = -75;
	if (headDiff >= 75) headDiff = +75;
	yBodyRot = yRot - headDiff;
	if (headDiff * headDiff > 50 * 50)
	{
		yBodyRot += headDiff * 0.2f;
	}

	if (behind)
	{
		walkSpeed *= -1;
	}

	return walkSpeed;
}

void LivingEntity::aiStep()
{
	if (noJumpDelay > 0) noJumpDelay--;
	if (lSteps > 0)
	{
		double xt = x + (lx - x) / lSteps;
		double yt = y + (ly - y) / lSteps;
		double zt = z + (lz - z) / lSteps;

		double yrd = Mth::wrapDegrees(lyr - yRot);
		double xrd = Mth::wrapDegrees(lxr - xRot);

		yRot += static_cast<float>((yrd) / lSteps);
		xRot += static_cast<float>((xrd) / lSteps);

		lSteps--;
		setPos(xt, yt, zt);
		setRot(yRot, xRot);

		// 4J - this collision is carried out to try and stop the lerping push the mob through the floor,
		// in which case gravity can then carry on moving the mob because the collision just won't work anymore.
		// BB for collision used to be calculated as: bb->shrink(1 / 32.0, 0, 1 / 32.0)
		// now using a reduced BB to try and get rid of some issues where mobs pop up the sides of walls, undersides of
		// trees etc.
		AABB *shrinkbb = bb->shrink(0.1, 0, 0.1);
		shrinkbb->y1 = shrinkbb->y0 + 0.1;
		AABBList *collisions = level->getCubes(shared_from_this(), shrinkbb);
		if (collisions->size() > 0)
		{
			double yTop = 0;
			for (const auto& ab : *collisions)
			{
				if (ab->y1 > yTop) yTop = ab->y1;
			}

			yt += yTop - bb->y0;
			setPos(xt, yt, zt);
		}
	}
	else if (!isEffectiveAi())
	{
		// slow down predicted speed, to prevent mobs from sliding through
		// walls etc
		xd *= .98;
		yd *= .98;
		zd *= .98;
	}

	if (abs(xd) < MIN_MOVEMENT_DISTANCE) xd = 0;
	if (abs(yd) < MIN_MOVEMENT_DISTANCE) yd = 0;
	if (abs(zd) < MIN_MOVEMENT_DISTANCE) zd = 0;

	if (isImmobile())
	{
		jumping = false;
		xxa = 0;
		yya = 0;
		yRotA = 0;
	}
	else
	{
		MemSect(25);
		if (isEffectiveAi())
		{
			if (useNewAi())
			{
				newServerAiStep();
			}
			else
			{
				serverAiStep();
				yHeadRot = yRot;
			}
		}
		MemSect(0);
	}

	if (jumping)
	{
		if (isInWater() || isInLava() )
		{
			yd += 0.04f;
		}
		else if (onGround)
		{
			if (noJumpDelay == 0)
			{
				jumpFromGround();
				noJumpDelay = 10;
			}
		}
	}
	else
	{
		noJumpDelay = 0;
	}


	xxa *= 0.98f;
	yya *= 0.98f;
	yRotA *= 0.9f;

	travel(xxa, yya);

	if(!level->isClientSide)
	{
		pushEntities();
	}
}

void LivingEntity::newServerAiStep()
{
}

void LivingEntity::pushEntities()
{

	vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), this->bb->grow(0.2f, 0, 0.2f));
	if (entities != nullptr && !entities->empty())
	{
		for (auto& e : *entities)
		{
			if ( e && e->isPushable())
				e->push(shared_from_this());
		}
	}
}

void LivingEntity::doPush(shared_ptr<Entity> e)
{
	e->push(shared_from_this());
}

void LivingEntity::rideTick()
{
	Entity::rideTick();
	oRun = run;
	run = 0;
	fallDistance = 0;
}

void LivingEntity::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
	heightOffset = 0;
	lx = x;
	ly = y;
	lz = z;
	lyr = yRot;
	lxr = xRot;

	lSteps = steps;
}

void LivingEntity::serverAiMobStep()
{
}

void LivingEntity::serverAiStep()
{
	noActionTime++;
}

void LivingEntity::setJumping(bool jump)
{
	jumping = jump;
}

void LivingEntity::take(shared_ptr<Entity> e, int orgCount)
{
	if (!e->removed && !level->isClientSide)
	{
		EntityTracker *entityTracker = static_cast<ServerLevel *>(level)->getTracker();
		if ( e->instanceof(eTYPE_ITEMENTITY) )
		{
			entityTracker->broadcast(e, std::make_shared<TakeItemEntityPacket>(e->entityId, entityId));
		}
		else if ( e->instanceof(eTYPE_ARROW) )
		{
			entityTracker->broadcast(e, std::make_shared<TakeItemEntityPacket>(e->entityId, entityId));
		}
		else if ( e->instanceof(eTYPE_EXPERIENCEORB) )
		{
			entityTracker->broadcast(e, std::make_shared<TakeItemEntityPacket>(e->entityId, entityId));
		}
	}
}

bool LivingEntity::canSee(shared_ptr<Entity> target)
{
	HitResult *hres = level->clip(Vec3::newTemp(x, y + getHeadHeight(), z), Vec3::newTemp(target->x, target->y + target->getHeadHeight(), target->z));
	bool retVal = (hres == nullptr);
	delete hres;
	return retVal;
}

Vec3 *LivingEntity::getLookAngle()
{
	return getViewVector(1);
}

Vec3 *LivingEntity::getViewVector(float a)
{
	if (a == 1)
	{
		float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
		float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
		float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
		float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

		return Vec3::newTemp(ySin * xCos, xSin, yCos * xCos);
	}
	float xRot = xRotO + (this->xRot - xRotO) * a;
	float yRot = yRotO + (this->yRot - yRotO) * a;

	float yCos = Mth::cos(-yRot * Mth::RAD_TO_GRAD - PI);
	float ySin = Mth::sin(-yRot * Mth::RAD_TO_GRAD - PI);
	float xCos = -Mth::cos(-xRot * Mth::RAD_TO_GRAD);
	float xSin = Mth::sin(-xRot * Mth::RAD_TO_GRAD);

	return Vec3::newTemp(ySin * xCos, xSin, yCos * xCos);
}

float LivingEntity::getAttackAnim(float a)
{
	float diff = attackAnim - oAttackAnim;
	if (diff < 0) diff += 1;
	return oAttackAnim + diff * a;
}

Vec3 *LivingEntity::getPos(float a)
{
	if (a == 1)
	{
		return Vec3::newTemp(x, y, z);
	}
	double x = xo + (this->x - xo) * a;
	double y = yo + (this->y - yo) * a;
	double z = zo + (this->z - zo) * a;

	return Vec3::newTemp(x, y, z);
}

HitResult *LivingEntity::pick(double range, float a)
{
	Vec3 *from = getPos(a);
	Vec3 *b = getViewVector(a);
	Vec3 *to = from->add(b->x * range, b->y * range, b->z * range);
	return level->clip(from, to);
}

bool LivingEntity::isEffectiveAi()
{
	return !level->isClientSide;
}

bool LivingEntity::isPickable()
{
	return !removed;
}

bool LivingEntity::isPushable()
{
	return !removed;
}

float LivingEntity::getHeadHeight()
{
	return bbHeight * 0.85f;
}

void LivingEntity::markHurt()
{
	hurtMarked = random->nextDouble() >= getAttribute(SharedMonsterAttributes::KNOCKBACK_RESISTANCE)->getValue();
}

float LivingEntity::getYHeadRot()
{
	return yHeadRot;
}

void LivingEntity::setYHeadRot(float yHeadRot)
{
	this->yHeadRot = yHeadRot;
}

float LivingEntity::getAbsorptionAmount()
{
	return absorptionAmount;
}

void LivingEntity::setAbsorptionAmount(float absorptionAmount)
{
	if (absorptionAmount < 0) absorptionAmount = 0;
	this->absorptionAmount = absorptionAmount;
}

Team *LivingEntity::getTeam()
{
	return nullptr;
}

bool LivingEntity::isAlliedTo(shared_ptr<LivingEntity> other)
{
	return isAlliedTo(other->getTeam());
}

bool LivingEntity::isAlliedTo(Team *other)
{
	if (getTeam() != nullptr)
	{
		return getTeam()->isAlliedTo(other);
	}
	return false;
}