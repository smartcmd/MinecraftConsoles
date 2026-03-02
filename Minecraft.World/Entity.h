#pragma once
using namespace std;
#include "ListTag.h"
#include "DoubleTag.h"
#include "FloatTag.h"
#include "Vec3.h"
#include "Definitions.h"

class Mob;
class LightningBolt;
class ItemEntity;
class EntityPos;
class Material;
class SynchedEntityData;
class Player;
class Random;
class Level;
class CompoundTag;
class DamageSource;

// 4J Stu Added this mainly to allow is to record telemetry for player deaths
enum EEntityDamageType
{
	eEntityDamageType_Entity,
	eEntityDamageType_Fall,
	eEntityDamageType_Fire,
	eEntityDamageType_Lava,
	eEntityDamageType_Water,
	eEntityDamageType_Suffocate,
	eEntityDamageType_OutOfWorld,
	eEntityDamageType_Cactus,
};

class Entity : public enable_shared_from_this<Entity>
{
friend class Gui; // 4J Stu - Added to be able to access the shared flag functions and constants, without making them publicly available to everything
public:
	// 4J-PB - added to replace (e instanceof Type), avoiding dynamic casts
	virtual eINSTANCEOF GetType() = 0;

public:

	static const short TOTAL_AIR_SUPPLY = 20 * 15;

private:
	static int entityCounter;

public:
	int entityId;

	double viewScale;

	bool blocksBuilding;
	weak_ptr<Entity> rider;		// Changed to weak to avoid circular dependency between rider/riding entity
	shared_ptr<Entity> riding;

	Level *level;
	double xo, yo, zo;
	double x, y, z;
	double xd, yd, zd;
	float yRot, xRot;
	float yRotO, xRotO;
	/*const*/ AABB *bb; // 4J Was final
	bool onGround;
	bool horizontalCollision, verticalCollision;
	bool collision;
	bool hurtMarked;

protected:
	bool isStuckInWeb;

public:
	bool slide;
	bool removed;
	float heightOffset;

	float bbWidth;
	float bbHeight;

	float walkDistO;
	float walkDist;
	float fallDistance;

private:
	int nextStep;

public:
	double xOld, yOld, zOld;
	float ySlideOffset;
	float footSize;
	bool noPhysics;
	float pushthrough;

protected:
	Random *random;

public:
	int tickCount;
	int flameTime;

private:
	int onFire;

protected:
	bool wasInWater;

public:
	int invulnerableTime;

private:
	bool firstTick;

public:
	wstring customTextureUrl;
	wstring customTextureUrl2;

protected:
	bool fireImmune;

	// values that need to be sent to clients in SMP
	shared_ptr<SynchedEntityData> entityData;

private:
	// shared flags that are sent to clients (max 8)
	static const int DATA_SHARED_FLAGS_ID = 0;
	static const int FLAG_ONFIRE = 0;
	static const int FLAG_SNEAKING = 1;
	static const int FLAG_RIDING = 2;
	static const int FLAG_SPRINTING = 3;
	static const int FLAG_USING_ITEM = 4;
	static const int FLAG_INVISIBLE = 5;
	static const int FLAG_IDLEANIM = 6;
	static const int FLAG_EFFECT_WEAKENED = 7; //4J ADDED, needed for cure villager tooltip.
	static const int DATA_AIR_SUPPLY_ID = 1;

private:
	double xRideRotA, yRideRotA;

public:
	bool inChunk;
    int xChunk, yChunk, zChunk;
    int xp, yp, zp, xRotp, yRotp;
    bool noCulling;
    bool hasImpulse;

protected:
	// 4J Added so that client side simulations on the host are not affected by zero-lag
	bool m_ignoreVerticalCollisions;

public:
	Entity(Level *level, bool useSmallId = true);	// 4J - added useSmallId parameter
	virtual ~Entity();

protected:
	// 4J - added for common ctor code
	void _init(bool useSmallId);

protected:
	virtual void defineSynchedData() = 0;

public:
	shared_ptr<SynchedEntityData> getEntityData();

	/*
	public bool equals(Object obj) {
	if (obj instanceof Entity) {
	return ((Entity) obj).entityId == entityId;
	}
	return false;
	}

	public int hashCode() {
	return entityId;
	}
	*/

protected:
	virtual void resetPos();

public:
	virtual void remove();

protected:
	virtual void setSize(float w, float h);
	void setPos(EntityPos *pos);
	void setRot(float yRot, float xRot);

public:
	void setPos(double x, double y, double z);
	void turn(float xo, float yo);
	void interpolateTurn(float xo, float yo);
	virtual void tick();
	virtual void baseTick();

protected:
	void lavaHurt();

public:
	virtual void setOnFire(int numberOfSeconds);
	virtual void clearFire();

protected:
	virtual void outOfWorld();

public:
	bool isFree(float xa, float ya, float za, float grow);
	bool isFree(double xa, double ya, double za);
	virtual void move(double xa, double ya, double za, bool noEntityCubes=false);	// 4J - added noEntityCubes parameter

protected:
	virtual void checkInsideTiles();
	virtual void playStepSound(int xt, int yt, int zt, int t);

public:
	virtual void playSound(int iSound, float volume, float pitch);

protected:
	virtual bool makeStepSound();
	virtual void checkFallDamage(double ya, bool onGround);

public:
	virtual AABB *getCollideBox();

protected:
	virtual void burn(int dmg);

public:
	bool isFireImmune();

protected:
	virtual void causeFallDamage(float distance);

public:
	bool isInWaterOrRain();
	virtual bool isInWater();
	virtual bool updateInWaterState();
	bool isUnderLiquid(Material *material);
	virtual float getHeadHeight();
	bool isInLava();
	void moveRelative(float xa, float za, float speed);
	virtual int getLightColor(float a);		// 4J - change brought forward from 1.8.2
	virtual float getBrightness(float a);
	virtual void setLevel(Level *level);
	void absMoveTo(double x, double y, double z, float yRot, float xRot);
	void moveTo(double x, double y, double z, float yRot, float xRot);
	float distanceTo(shared_ptr<Entity> e);
	double distanceToSqr(double x2, double y2, double z2);
	double distanceTo(double x2, double y2, double z2);
	double distanceToSqr(shared_ptr<Entity> e);
	virtual void playerTouch(shared_ptr<Player> player);
	virtual void push(shared_ptr<Entity> e);
	virtual void push(double xa, double ya, double za);

protected:
	void markHurt();

public:
	// 4J Added damageSource param to enable telemetry on player deaths
	virtual bool hurt(DamageSource *source, int damage);
	bool intersects(double x0, double y0, double z0, double x1, double y1, double z1);
	virtual bool isPickable();
	virtual bool isPushable();
	virtual bool isShootable();
	virtual void awardKillScore(shared_ptr<Entity> victim, int score);
	virtual bool shouldRender(Vec3 *c);
	virtual bool shouldRenderAtSqrDistance(double distance);
	virtual int getTexture();			// 4J - changed from wstring to int
	virtual bool isCreativeModeAllowed();
	bool save(CompoundTag *entityTag);
	void saveWithoutId(CompoundTag *entityTag);
	virtual void load(CompoundTag *tag);

protected:
	const wstring getEncodeId();

public:
	virtual void readAdditionalSaveData(CompoundTag *tag) = 0;
	virtual void addAdditonalSaveData(CompoundTag *tag) = 0;

protected:
	ListTag<DoubleTag> *newDoubleList(unsigned int number, double firstValue, ...);
	ListTag<FloatTag> *newFloatList(unsigned int number, float firstValue, float secondValue);

public:
	virtual float getShadowHeightOffs();
	shared_ptr<ItemEntity> spawnAtLocation(int resource, int count);
	shared_ptr<ItemEntity> spawnAtLocation(int resource, int count, float yOffs);
	shared_ptr<ItemEntity> spawnAtLocation(shared_ptr<ItemInstance> itemInstance, float yOffs);
	virtual bool isAlive();
	virtual bool isInWall();
	virtual bool interact(shared_ptr<Player> player);
	virtual AABB *getCollideAgainstBox(shared_ptr<Entity> entity);

	virtual void rideTick();
	virtual void positionRider();
	virtual double getRidingHeight();
	virtual double getRideHeight();
	virtual void ride(shared_ptr<Entity> e);
	virtual void findStandUpPosition(shared_ptr<Entity> vehicle); // 4J Stu - Brought forward from 12w36 to fix #46282 - TU5: Gameplay: Exiting the minecart in a tight corridor damages the player
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual float getPickRadius();
	virtual Vec3 *getLookAngle();
	virtual void handleInsidePortal();
	virtual void lerpMotion(double xd, double yd, double zd);
	virtual void handleEntityEvent(byte eventId);
	virtual void animateHurt();
	virtual void prepareCustomTextures();
	virtual ItemInstanceArray getEquipmentSlots(); // ItemInstance[]
	virtual void setEquippedSlot(int slot, shared_ptr<ItemInstance> item); // 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
	virtual bool isOnFire();
	virtual bool isRiding();
	virtual bool isSneaking();
	virtual void setSneaking(bool value);
	virtual bool isIdle();
	virtual void setIsIdle(bool value);
	virtual bool isSprinting();
	virtual void setSprinting(bool value);
	virtual bool isInvisible();
	virtual bool isInvisibleTo(shared_ptr<Player> plr);
	virtual void setInvisible(bool value);
	virtual bool isUsingItemFlag();
	virtual void setUsingItemFlag(bool value);

	// 4J-ADDED, we need to see if this is weakened
	// on the client for the cure villager tooltip.
	bool isWeakened();
	void setWeakened(bool value);

protected:
	bool getSharedFlag(int flag);
	void setSharedFlag(int flag, bool value);

public:
	// 4J Stu - Brought forward from 1.2.3 to fix 38654 - Gameplay: Player will take damage when air bubbles are present if resuming game from load/autosave underwater.
	int getAirSupply();
	void setAirSupply(int supply);

	virtual void thunderHit(const LightningBolt *lightningBolt);
	virtual void killed(shared_ptr<Mob> mob);

protected:
	bool checkInTile(double x, double y, double z);

public:
	virtual void makeStuckInWeb();

	virtual wstring getAName();

	// TU9
	bool skipAttackInteraction(shared_ptr<Entity> source) {return false;}

	// 4J - added to manage allocation of small ids
private:
	// Things also added here to be able to manage the concept of a number of extra "wandering" entities - normally path finding entities aren't allowed to
	// randomly wander about once they are a certain distance away from any player, but we want to be able to (in a controlled fashion) allow some to be able
	// to move so that we can determine whether they have been enclosed in some kind of farm, and so be able to better determine what shouldn't or shouldn't be despawned.
	static const int EXTRA_WANDER_MAX = 3;				// Number of entities that can simultaneously wander (in addition to the ones that would be wandering in java)
	static const int EXTRA_WANDER_TICKS = 20 * 30;		// Number of ticks each extra entity will be allowed to wander for. This should be enough for it to realistically be able to walk further than the biggest enclosure we want to consider

	int getSmallId();
	void freeSmallId(int index);
	static unsigned int entityIdUsedFlags[2048/32];
	static unsigned int entityIdWanderFlags[2048/32];
	static unsigned int entityIdRemovingFlags[2048/32];
	static int extraWanderIds[EXTRA_WANDER_MAX];
	static int extraWanderCount;
	static int extraWanderTicks;
	static DWORD tlsIdx;
public:
	static void tickExtraWandering();
	static void countFlagsForPIX();
	void resetSmallId();
	static void useSmallIds();
	void considerForExtraWandering(bool enable);
	bool isExtraWanderingEnabled();
	int getWanderingQuadrant();
	
	virtual vector<shared_ptr<Entity> > *getSubEntities();
	virtual bool is(shared_ptr<Entity> other);
	virtual float getYHeadRot();
	virtual void setYHeadRot(float yHeadRot);
	virtual bool isAttackable();
	virtual bool isInvulnerable();
	virtual void copyPosition(shared_ptr<Entity> target);

private:
	unsigned int m_uiAnimOverrideBitmask;
public:
	void setAnimOverrideBitmask(unsigned int uiBitmask);
	unsigned int getAnimOverrideBitmask();

	// 4J added
	virtual bool isDespawnProtected() { return false; }
	virtual void setDespawnProtected() {}
	virtual bool couldWander() { return false; }
	virtual bool canCreateParticles() { return true; }
};
