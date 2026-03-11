#pragma once

#include "Animal.h"
#include "net.minecraft.world.ContainerListener.h"
#include "MobGroupData.h"
#include "EntitySelector.h"

class Attribute;
class AnimalChest;

class HorseEntitySelector : public EntitySelector
{
public:
	bool matches(shared_ptr<Entity> entity) const;
};

class EntityHorse : public Animal, public net_minecraft_world::ContainerListener
{
public:
	eINSTANCEOF GetType() { return eTYPE_HORSE; }
	static Entity *create(Level *level) { return new EntityHorse(level); }

private:
	static const wstring TEX_FOLDER;

	static const EntitySelector *PARENT_HORSE_SELECTOR;

	static Attribute *JUMP_STRENGTH;

	static const int DATA_ID_HORSE_FLAGS = 16;
	static const int DATA_ID_TYPE = 19;
	static const int DATA_ID_TYPE_VARIANT = 20;
	static const int DATA_ID_OWNER_NAME = 21;
	static const int DATA_ID_ARMOR = 22;

	static const int FLAG_TAME = 1 << 1;
	static const int FLAG_SADDLE = 1 << 2;
	static const int FLAG_CHESTED = 1 << 3;
	static const int FLAG_BRED = 1 << 4;
	static const int FLAG_EATING = 1 << 5;
	static const int FLAG_STANDING = 1 << 6;
	static const int FLAG_OPEN_MOUTH = 1 << 7;

public:
	static const int INV_SLOT_SADDLE = 0;
	static const int INV_SLOT_ARMOR = 1;
	static const int INV_BASE_COUNT = 2;
	static const int INV_DONKEY_CHEST_COUNT = 15;

	// TODO: USE ENUMS! // Original comment
	static const int ARMOR_NONE = 0;
	static const int ARMOR_IRON = 1;
	static const int ARMOR_GOLD = 2;
	static const int ARMOR_DIAMOND = 3;

private:
	static const int ARMORS = 4;
	static wstring ARMOR_TEXTURES[ARMORS];
	static int ARMOR_TEXTURES_ID[ARMORS];
	static wstring ARMOR_HASHES[ARMORS];
	static int ARMOR_PROTECTION[ARMORS];

public:
	static const int TYPE_HORSE = 0;
	static const int TYPE_DONKEY = 1;
	static const int TYPE_MULE = 2;
	static const int TYPE_UNDEAD = 3;
	static const int TYPE_SKELETON = 4;

	static const int VARIANT_WHITE = 0;
	static const int VARIANT_CREAMY = 1;
	static const int VARIANT_CHESTNUT = 2;
	static const int VARIANT_BROWN = 3;
	static const int VARIANT_BLACK = 4;
	static const int VARIANT_GRAY = 5;
	static const int VARIANT_DARKBROWN = 6;

private:
	static const int VARIANTS = 7;
	static wstring VARIANT_TEXTURES[VARIANTS];
	static int VARIANT_TEXTURES_ID[VARIANTS];
	static wstring VARIANT_HASHES[VARIANTS];

public:
	static const int MARKING_NONE = 0;
	static const int MARKING_WHITE_DETAILS = 1;
	static const int MARKING_WHITE_FIELDS = 2;
	static const int MARKING_WHITE_DOTS = 3;
	static const int MARKING_BLACK_DOTS = 4;

private:
	static const int MARKINGS = 5;
	static wstring MARKING_TEXTURES[MARKINGS];
	static int MARKING_TEXTURES_ID[MARKINGS];
	static wstring MARKING_HASHES[MARKINGS];

private:
	int countEating; // eating timer
	int mouthCounter;
	int standCounter;

public:
	int tailCounter;
	int sprintCounter;

protected:
	bool isEntityJumping;

private:
	shared_ptr<AnimalChest> inventory;
	bool hasReproduced;

protected:
	int temper;
	float playerJumpPendingScale;

private:
	bool allowStandSliding;

	// animation data
	float eatAnim, eatAnimO;
	float standAnim, standAnimO;
	float mouthAnim, mouthAnimO;

public:
	EntityHorse(Level *world);
	~EntityHorse();

protected:
	virtual void defineSynchedData();

public:
	virtual void setType(int i);
	virtual int getType();
	virtual void setVariant(int i);
	virtual int getVariant();
	virtual wstring getAName();

private:
	virtual bool getHorseFlag(int flag);
	virtual void setHorseFlag(int flag, bool value);

public:
	virtual bool isAdult();
	virtual bool isTamed();
	virtual bool isRidable();
	virtual wstring getOwnerName();
	virtual void setOwner(const wstring &par1Str);
	virtual float getFoalScale();
	virtual void updateSize(bool isBaby);
	virtual bool getIsJumping();
	virtual void setTamed(bool flag);
	virtual void setIsJumping(bool flag);
	virtual bool canBeLeashed();

protected:
	virtual void onLeashDistance(float distanceToLeashHolder);

public:
	virtual bool isChestedHorse();
	virtual int getArmorType();
	virtual int getArmorTypeForItem(shared_ptr<ItemInstance> armorItem);
	virtual bool isEating();
	virtual bool isStanding();
	virtual bool isBred();
	virtual bool getHasReproduced();
	virtual void setArmorType(int i);
	virtual void setBred(bool flag);
	virtual void setChestedHorse(bool flag);
	virtual void setReproduced(bool flag);
	virtual void setSaddled(bool flag);
	virtual int getTemper();
	virtual void setTemper(int temper);
	virtual int modifyTemper(int amount);
	virtual bool hurt(DamageSource *damagesource, float dmg);
	virtual int getArmorValue();
	virtual bool isPushable();
	virtual bool checkSpawningBiome();
	virtual void dropBags();

private:
	virtual void eatingHorse();

protected:
	virtual void causeFallDamage(float fallDistance);

private:
	virtual int getInventorySize();
	virtual void createInventory();
	virtual void updateEquipment();

public:
	virtual void containerChanged();
	virtual bool canSpawn();
	virtual bool removeWhenFarAway() override;

protected:
	virtual shared_ptr<EntityHorse> getClosestMommy(shared_ptr<Entity> baby, double searchRadius);

public:
	virtual double getCustomJump();

protected:
	virtual int getDeathSound();
	virtual int getDeathLoot();
	virtual int getHurtSound();

public:
	virtual bool isSaddled();

protected:
	virtual int getAmbientSound();
	virtual int getMadSound();

private:
	int gallopSoundCounter;


protected:
	virtual void playStepSound(int xt, int yt, int zt, int t);
	virtual void registerAttributes();

public:
	virtual int getMaxSpawnClusterSize();
	virtual int getMaxTemper();

protected:
	virtual float getSoundVolume();

public:
	virtual int getAmbientSoundInterval();
	virtual bool hasLayeredTextures();

private:
	wstring layerTextureHashName;
	intArray layerTextureLayers;

private:
	virtual void clearLayeredTextureInfo();
	virtual void rebuildLayeredTextureInfo();

public:
	virtual wstring getLayeredTextureHashName();
	virtual intArray getLayeredTextureLayers();
	virtual void openInventory(shared_ptr<Player> player);
	virtual bool mobInteract(shared_ptr<Player> player);

private:
	virtual void doPlayerRide(shared_ptr<Player> player);

public:
	virtual bool isAmuletHorse();
	virtual bool canWearArmor();
	virtual bool canWearBags();

protected:
	virtual bool isImmobile();

public:
	virtual bool isPureBreed();
	virtual bool isUndead();
	virtual bool isSterile();
	virtual bool isFood(shared_ptr<ItemInstance> itemInstance);

private:
	virtual void moveTail();

public:
	virtual int nameYOffset();
	virtual void die(DamageSource *damagesource);
	virtual void aiStep();
	virtual void tick();

private:
	virtual void openMouth();

public:
	// 4J-JEV: Made public for tooltip code, doesn't change state anyway.
	virtual bool isReadyForParenting();

public:
	virtual bool renderName();
	virtual bool rideableEntity();
	virtual void setUsingItemFlag(bool flag);
	virtual void setEating(bool state);
	virtual void setStanding(bool state);

private:
	virtual void stand();

public:
	virtual void makeMad();
	virtual void dropMyStuff();

private:
	virtual void dropInventory(shared_ptr<Entity> entity, shared_ptr<AnimalChest> animalchest);

public:
	virtual bool tameWithName(shared_ptr<Player> player);
	virtual void travel(float xa, float ya);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual bool canMate(shared_ptr<Animal> partner);
	virtual shared_ptr<AgableMob> getBreedOffspring(shared_ptr<AgableMob> partner);
	virtual MobGroupData *finalizeMobSpawn(MobGroupData *groupData, int extraData = 0); // 4J Added extraData param
	virtual float getEatAnim(float a);
	virtual float getStandAnim(float a);
	virtual float getMouthAnim(float a);

protected:
	virtual bool useNewAi();

public:
	virtual void onPlayerJump(int jumpAmount);

protected:
	virtual void spawnTamingParticles(bool success);

public:
	virtual void handleEntityEvent(byte id);
	virtual void positionRider();

private:
	virtual float generateRandomMaxHealth();
	virtual double generateRandomJumpStrength();
	virtual double generateRandomSpeed();
	
	shared_ptr<Player> getOwner();

public:
	class HorseGroupData : public MobGroupData
	{

	public:
		int horseType;
		int horseVariant;

		HorseGroupData(int type, int variant);
	};

	static bool isHorseArmor(int itemId);
	virtual bool onLadder();
};