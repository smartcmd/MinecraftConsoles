#pragma once
using namespace std;

#include "Entity.h"
#include "MobType.h"
#include "GoalSelector.h"
#include "SharedConstants.h"

class CombatTracker;
class AttributeInstance;
class AttributeModifier;
class MobEffectInstance;
class BaseAttributeMap;
class Team;
class Attribute;
class MobEffect;
class HitResult;
class Vec3;

class LivingEntity : public Entity
{
	friend class MobSpawner;
protected:
	// 4J - added for common ctor code
	void _init();
public:
	// 4J-PB - added to replace (e instanceof Type), avoiding dynamic casts
	eINSTANCEOF GetType()						{ return eTYPE_LIVINGENTITY;}
	static Entity *create(Level *level) { return nullptr; }

private:
	static AttributeModifier *SPEED_MODIFIER_SPRINTING;

public:
	static const int SLOT_WEAPON = 0;
	static const int SLOT_BOOTS = 1;
	static const int SLOT_LEGGINGS = 2;
	static const int SLOT_CHEST = 3;
	static const int SLOT_HELM = 4;

	static const int SWING_DURATION = 6;
	static const int PLAYER_HURT_EXPERIENCE_TIME = SharedConstants::TICKS_PER_SECOND * 5;

private:
	static const double MIN_MOVEMENT_DISTANCE;

public:
	static const int DATA_HEALTH_ID = 6;
	static const int DATA_EFFECT_COLOR_ID = 7;
	static const int DATA_EFFECT_AMBIENCE_ID = 8;
	static const int DATA_ARROW_COUNT_ID = 9;

private:
	BaseAttributeMap *attributes;
	CombatTracker *combatTracker;
	unordered_map<int, MobEffectInstance *> activeEffects;
	ItemInstanceArray lastEquipment;

public:
	bool swinging;
	int swingTime;
	int removeArrowTime;
	float lastHealth;

	int hurtTime;
	int hurtDuration;
	float hurtDir;
	int deathTime;
	int attackTime;
	float oAttackAnim, attackAnim;

	float walkAnimSpeedO;
	float walkAnimSpeed;
	float walkAnimPos;
	int invulnerableDuration;
	float oTilt, tilt;
	float timeOffs;
	float rotA;
	float yBodyRot, yBodyRotO;
	float yHeadRot, yHeadRotO;
	float flyingSpeed;

	int nametagColor;

protected:
	shared_ptr<Player> lastHurtByPlayer;
	int lastHurtByPlayerTime;
	bool dead;
	int noActionTime;
	float oRun, run;
	float animStep, animStepO;
	float rotOffs;
	int deathScore;
	float lastHurt;
	bool jumping;

public:
	float xxa;
	float yya;

protected:
	float yRotA;
	int lSteps;
	double lx, ly, lz, lyr, lxr;

private:
	bool effectsDirty;

	shared_ptr<LivingEntity> lastHurtByMob;
	int lastHurtByMobTimestamp;
	shared_ptr<LivingEntity> lastHurtMob;
	int lastHurtMobTimestamp;

	float speed;

protected:
	int noJumpDelay;

private:
	float absorptionAmount;

public:
	LivingEntity(Level* level);
	virtual ~LivingEntity();

protected:
	virtual void defineSynchedData();
	virtual void registerAttributes();
	virtual void checkFallDamage(double ya, bool onGround);

public:
	virtual bool isWaterMob();
	virtual void baseTick();
	virtual bool isBaby();

protected:
	virtual void tickDeath();
	virtual int decreaseAirSupply(int currentSupply);
	virtual int getExperienceReward(shared_ptr<Player> killedBy);
	virtual bool isAlwaysExperienceDropper();

public:
	virtual Random *getRandom();
	virtual shared_ptr<LivingEntity> getLastHurtByMob();
	virtual int getLastHurtByMobTimestamp();
	virtual void setLastHurtByMob(shared_ptr<LivingEntity> hurtBy);
	virtual shared_ptr<LivingEntity> getLastHurtMob();
	virtual int getLastHurtMobTimestamp();
	virtual void setLastHurtMob(shared_ptr<Entity> target);
	virtual int getNoActionTime();
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);

protected:
	virtual void tickEffects();

public:
	virtual void removeAllEffects();
	virtual vector<MobEffectInstance *> *getActiveEffects();
	virtual bool hasEffect(int id);
	virtual bool hasEffect(MobEffect *effect);
	virtual MobEffectInstance *getEffect(MobEffect *effect);
	virtual void addEffect(MobEffectInstance *newEffect);
	virtual void addEffectNoUpdate(MobEffectInstance *newEffect); // 4J added
	virtual bool canBeAffected(MobEffectInstance *newEffect);
	virtual bool isInvertedHealAndHarm();
	virtual  void removeEffectNoUpdate(int effectId);
	virtual void removeEffect(int effectId);

protected:
	virtual void onEffectAdded(MobEffectInstance *effect);
	virtual void onEffectUpdated(MobEffectInstance *effect, bool doRefreshAttributes);
	virtual void onEffectRemoved(MobEffectInstance *effect);

public:
	virtual void heal(float heal);
	virtual float getHealth();
	virtual void setHealth(float health);
	virtual bool hurt(DamageSource *source, float dmg);
	virtual void breakItem(shared_ptr<ItemInstance> itemInstance);
	virtual void die(DamageSource *source);

protected:
	virtual void dropEquipment(bool byPlayer, int playerBonusLevel);

public:
	virtual void knockback(shared_ptr<Entity> source, float dmg, double xd, double zd);

protected:
	virtual int getHurtSound();
	virtual int getDeathSound();

protected:
	virtual void dropRareDeathLoot(int rareLootLevel);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);

public:
	virtual bool onLadder();
	virtual bool isShootable();
	virtual bool isAlive();
	virtual void causeFallDamage(float distance);
	virtual void animateHurt();
	virtual int getArmorValue();

protected:
	virtual void hurtArmor(float damage);
	virtual float getDamageAfterArmorAbsorb(DamageSource *damageSource, float damage);
	virtual float getDamageAfterMagicAbsorb(DamageSource *damageSource, float damage);
	virtual void actuallyHurt(DamageSource *source, float dmg);

public:
	virtual CombatTracker *getCombatTracker();
	virtual shared_ptr<LivingEntity> getKillCredit();
	virtual float getMaxHealth();
	virtual int getArrowCount();
	virtual void setArrowCount(int count);

private:
	int getCurrentSwingDuration();

public:
	virtual void swing();
	virtual void handleEntityEvent(byte id);

protected:
	virtual void outOfWorld();
	virtual void updateSwingTime();

public:
	virtual AttributeInstance *getAttribute(Attribute *attribute);
	virtual BaseAttributeMap *getAttributes();
	virtual MobType getMobType();

	virtual shared_ptr<ItemInstance> getCarriedItem() = 0;
	virtual shared_ptr<ItemInstance> getCarried(int slot) = 0;
	virtual shared_ptr<ItemInstance> getArmor(int pos) = 0;
	virtual void setEquippedSlot(int slot, shared_ptr<ItemInstance> item) = 0;
	virtual void setSprinting(bool value);

	virtual ItemInstanceArray getEquipmentSlots() = 0;

	virtual Icon *getItemInHandIcon(shared_ptr<ItemInstance> item, int layer);

protected:
	virtual float getSoundVolume();
	virtual float getVoicePitch();
	virtual bool isImmobile();

public:
	virtual void teleportTo(double x, double y, double z);

protected:
	virtual void findStandUpPosition(shared_ptr<Entity> vehicle);
	
public:
	virtual bool shouldShowName();

protected:
	virtual void jumpFromGround();

public:
	virtual void travel(float xa, float ya);

	virtual int getLightColor(float a);		// 4J - added

protected:
	virtual bool useNewAi();

public:
	virtual float getSpeed();
	virtual void setSpeed(float speed);
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	virtual bool isSleeping();
	virtual void tick();

protected:
	virtual float tickHeadTurn(float yBodyRotT, float walkSpeed);

public:
	virtual void aiStep();

protected:
	virtual void newServerAiStep();
	virtual void pushEntities();
	virtual void doPush(shared_ptr<Entity> e);

public:
	virtual void rideTick();
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);

protected:
	virtual void serverAiMobStep();
	virtual void serverAiStep();

public:
	virtual void setJumping(bool jump);
	virtual void take(shared_ptr<Entity> e, int orgCount);
	virtual bool canSee(shared_ptr<Entity> target);


public:
	virtual Vec3 *getLookAngle();
	virtual Vec3 *getViewVector(float a);
	virtual float getAttackAnim(float a);
	virtual Vec3 *getPos(float a);
	virtual HitResult *pick(double range, float a);
	virtual bool isEffectiveAi();

	virtual bool isPickable();
	virtual bool isPushable();
	virtual float getHeadHeight();

protected:
	virtual void markHurt();

public:
	virtual float getYHeadRot();
	virtual void setYHeadRot(float yHeadRot);

	virtual float getAbsorptionAmount();
	virtual void setAbsorptionAmount(float absorptionAmount);
	virtual Team *getTeam();
	virtual bool isAlliedTo(shared_ptr<LivingEntity> other);
	virtual bool isAlliedTo(Team *other);
};
