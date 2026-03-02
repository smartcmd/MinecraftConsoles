#pragma once
using namespace std;

#include "Entity.h"
#include "MobType.h"
#include "GoalSelector.h"

class HitResult;
class Level;
class CompoundTag;
class MobEffectInstance;
class DamageSource;
class MobEffect;
class LookControl;
class MoveControl;
class JumpControl;
class BodyControl;
class PathNavigation;
class Sensing;
class Icon;
class Pos;

class Mob : public Entity
{
	friend class MobSpawner;
protected:
	// 4J - added for common ctor code
	void _init();
public:
	Mob(Level* level);
	virtual ~Mob();

	// 4J-PB - added to replace (e instanceof Type), avoiding dynamic casts
	eINSTANCEOF GetType()						{ return eTYPE_MOB;}
	static Entity *create(Level *level) { return NULL; }

public:
	static const int ATTACK_DURATION = 5;
	static const int PLAYER_HURT_EXPERIENCE_TIME = 20 * 3;

public: // 4J Stu - Made public
	static const int DATA_EFFECT_COLOR_ID = 8;

private:
	static const double MIN_MOVEMENT_DISTANCE;

public:
	int invulnerableDuration;
	float timeOffs;
	float rotA;
	float yBodyRot, yBodyRotO;
	float yHeadRot, yHeadRotO;

protected:
	float oRun, run;
	float animStep, animStepO;
	bool hasHair;
	//	wstring textureName;
	int textureIdx;		// 4J changed from wstring textureName
	bool allowAlpha;
	float rotOffs;
	wstring modelName;
	float bobStrength;
	int deathScore;
	float renderOffset;

public:
	float walkingSpeed;
	float flyingSpeed;
	float oAttackAnim, attackAnim;

protected:
	int health;

public:
	int lastHealth;

protected:
	int dmgSpill;

public:
	int ambientSoundTime;
	int hurtTime;
	int hurtDuration;
	float hurtDir;
	int deathTime;
	int attackTime;
	float oTilt, tilt;

protected:
	bool dead;
	int xpReward;

public:
	int modelNum;
	float animSpeed;
	float walkAnimSpeedO;
	float walkAnimSpeed;
	float walkAnimPos;

protected:
	shared_ptr<Player> lastHurtByPlayer;
	int lastHurtByPlayerTime;

private:
	shared_ptr<Mob> lastHurtByMob;
	int lastHurtByMobTime;
	shared_ptr<Mob> lastHurtMob;

public:
	int arrowCount;
	int removeArrowTime;

protected:
	map<int, MobEffectInstance *> activeEffects;

private:
	bool effectsDirty;
	int effectColor;

	LookControl *lookControl;
	MoveControl *moveControl;
	JumpControl *jumpControl;
	BodyControl *bodyControl;
	PathNavigation *navigation;

protected:
	GoalSelector goalSelector;
	GoalSelector targetSelector;

private:
	shared_ptr<Mob> target;
	Sensing *sensing;
	float speed;

	Pos *restrictCenter;
	float restrictRadius;

public:
	virtual LookControl *getLookControl();
	virtual MoveControl *getMoveControl();
	virtual JumpControl *getJumpControl();
	virtual PathNavigation *getNavigation();
	virtual Sensing *getSensing();
	virtual Random *getRandom();
	virtual shared_ptr<Mob> getLastHurtByMob();
	virtual shared_ptr<Mob> getLastHurtMob();
	void setLastHurtMob(shared_ptr<Entity> target);
	virtual int getNoActionTime();
	float getYHeadRot();
	void setYHeadRot(float yHeadRot);
	float getSpeed();
	void setSpeed(float speed);
	virtual bool doHurtTarget(shared_ptr<Entity> target);
	shared_ptr<Mob> getTarget();
	virtual void setTarget(shared_ptr<Mob> target);
	virtual bool canAttackType(eINSTANCEOF targetType);
	virtual void ate();

	bool isWithinRestriction();
	bool isWithinRestriction(int x, int y, int z);
	void restrictTo(int x, int y, int z, int radius);
	Pos *getRestrictCenter();
	float getRestrictRadius();
	void clearRestriction();
	bool hasRestriction();

	virtual void setLastHurtByMob(shared_ptr<Mob> hurtBy);

protected:
	virtual void defineSynchedData();

public:
	bool canSee(shared_ptr<Entity> target);
	virtual int getTexture();		// 4J - changed from wstring to int
	virtual bool isPickable() ;
	virtual bool isPushable();
	virtual float getHeadHeight();
	virtual int getAmbientSoundInterval();
	void playAmbientSound();
	virtual void baseTick();

protected:
	virtual void tickDeath();
	virtual int decreaseAirSupply(int currentSupply);
	virtual int getExperienceReward(shared_ptr<Player> killedBy);
	virtual bool isAlwaysExperienceDropper();

public:
	void spawnAnim();
	virtual void rideTick();

protected:
	int lSteps;
	double lx, ly, lz, lyr, lxr;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);

private:
	float fallTime;

public:
	void superTick();
	virtual void tick();
	virtual void heal(int heal);
	virtual int getMaxHealth() = 0;
	virtual int getHealth();
	virtual void setHealth(int health);

protected:
	int lastHurt;

public:
	virtual bool hurt(DamageSource *source, int dmg);

protected:
	float getVoicePitch();

public:
	virtual void animateHurt();

	/**
	* Fetches the mob's armor value, from 0 (no armor) to 20 (full armor)
	* 
	* @return
	*/
	virtual int getArmorValue();

protected:
	virtual void hurtArmor(int damage);
	virtual int getDamageAfterArmorAbsorb(DamageSource *damageSource, int damage);
	virtual int getDamageAfterMagicAbsorb(DamageSource *damageSource, int damage);

	virtual void actuallyHurt(DamageSource *source, int dmg);
	virtual float getSoundVolume();
	virtual int getAmbientSound();
	virtual int getHurtSound();
	virtual int getDeathSound();

public:
	void knockback(shared_ptr<Entity> source, int dmg, double xd, double zd);
	virtual void die(DamageSource *source);

protected:
	virtual void dropRareDeathLoot(int rareLootLevel);
	virtual void dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel);
	virtual int getDeathLoot();
	virtual void causeFallDamage(float distance);

public:
	virtual void travel(float xa, float ya);
	virtual bool onLadder();
	virtual bool isShootable();
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool isAlive();
	virtual bool isWaterMob();
	virtual int getLightColor(float a);		// 4J - added

protected:
	int noActionTime;
	float xxa, yya, yRotA;
	bool jumping;
	float defaultLookAngle;
	float runSpeed;
protected:
	int noJumpDelay;

public:
	virtual void setYya(float yya);
	virtual void setJumping(bool jump);

	virtual void aiStep();

protected:
	virtual bool useNewAi();
	virtual bool isEffectiveAI();
	virtual bool isImmobile();

public:
	virtual bool isBlocking();

protected:
	virtual void jumpFromGround();
	virtual bool removeWhenFarAway();

private:
	shared_ptr<Entity> lookingAt;

protected:
	int lookTime;

	virtual void checkDespawn();
	virtual void newServerAiStep();
	virtual void serverAiMobStep();
	virtual void serverAiStep();

public:
	virtual int getMaxHeadXRot();

protected:
	void lookAt(shared_ptr<Entity> e, float yMax, float xMax);
	bool isLookingAtAnEntity();
	shared_ptr<Entity> getLookingAt();

private:
	float rotlerp(float a, float b, float max);

public:
	virtual bool canSpawn();

protected:
	virtual void outOfWorld();

public:
	float getAttackAnim(float a);
	virtual Vec3 *getPos(float a);
	virtual Vec3 *getLookAngle();
	Vec3 *getViewVector(float a);
	virtual float getSizeScale();
	virtual float getHeadSizeScale();
	HitResult *pick(double range, float a);
	virtual int getMaxSpawnClusterSize();
	virtual shared_ptr<ItemInstance> getCarriedItem();
	virtual shared_ptr<ItemInstance> getArmor(int pos);
	virtual void handleEntityEvent(byte id);
	virtual bool isSleeping();
	virtual Icon *getItemInHandIcon(shared_ptr<ItemInstance> item, int layer);
	virtual bool shouldRender(Vec3 *c);

protected:
	void tickEffects();

public:
	void removeAllEffects();
	vector<MobEffectInstance *> *getActiveEffects();
	bool hasEffect(int id);
	bool hasEffect(MobEffect *effect);
	MobEffectInstance *getEffect(MobEffect *effect);
	void addEffect(MobEffectInstance *newEffect);
	void addEffectNoUpdate(MobEffectInstance *newEffect); // 4J Added
	virtual bool canBeAffected(MobEffectInstance *newEffect);
	virtual bool isInvertedHealAndHarm();
	void removeEffectNoUpdate(int effectId);
	void removeEffect(int effectId);

protected:
	virtual void onEffectAdded(MobEffectInstance *effect);
	virtual void onEffectUpdated(MobEffectInstance *effect);
	virtual void onEffectRemoved(MobEffectInstance *effect);

public:
	virtual float getWalkingSpeedModifier();

	// 4J-Pb added (from 1.2.3)
	virtual void teleportTo(double x, double y, double z);
	virtual bool isBaby();
	virtual MobType getMobType();
	virtual void breakItem(shared_ptr<ItemInstance> itemInstance);

	virtual bool isInvulnerable();

	virtual void finalizeMobSpawn();
	virtual bool canBeControlledByRider();

	// 4J Added override to update ai elements when loading entity from schematics
	virtual void setLevel(Level *level);
};
