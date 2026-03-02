#pragma once
using namespace std;
#include "BossMob.h"
#include "MultiEntityMob.h"
#include "Enemy.h"

class MultiEntityMobPart;
class EnderCrystal;
class Node;
class BinaryHeap;
class Path;

class EnderDragon : public Mob, public BossMob, public MultiEntityMob, public Enemy
{
public:
	eINSTANCEOF GetType() { return eTYPE_ENDERDRAGON; };
	static Entity *create(Level *level) { return new EnderDragon(level); }

private:	
	// 4J Added for new behaviours
	static const int DATA_ID_SYNCHED_ACTION = 17;

	static const int positionsLength = 64; // 4J Stu - Defined this so that we can keep the positions array as a basic type array
public:
	double xTarget, yTarget, zTarget;

	double positions[positionsLength][3];
	int posPointer;

	//MultiEntityMobPart[] subEntities;
	vector<shared_ptr<Entity> > subEntities;
	shared_ptr<MultiEntityMobPart> head;
	shared_ptr<MultiEntityMobPart> neck; // 4J Added
	shared_ptr<MultiEntityMobPart> body;
	shared_ptr<MultiEntityMobPart> tail1;
	shared_ptr<MultiEntityMobPart> tail2;
	shared_ptr<MultiEntityMobPart> tail3;
	shared_ptr<MultiEntityMobPart> wing1;
	shared_ptr<MultiEntityMobPart> wing2;

	float oFlapTime;
	float flapTime;
	bool newTarget;
	bool inWall;

	// 4J Stu - Added for new dragon behaviour
private:
	int m_fireballCharge;
	float m_holdingPatternAngle;
	bool m_holdingPatternClockwise;
	int m_actionTicks;
	int m_sittingDamageReceived;
	int m_remainingCrystalsCount;
	int m_flameAttacks;

	int m_iGrowlTimer;

	double m_headYRot;
	AABB *m_acidArea;

	NodeArray *m_nodes;
	int m_nodeAdjacency[24];
	BinaryHeap *openSet;
	Path *m_currentPath;

	enum EEnderdragonAction
	{
		e_EnderdragonAction_HoldingPattern,
		e_EnderdragonAction_StrafePlayer,
		e_EnderdragonAction_LandingApproach,
		e_EnderdragonAction_Landing,
		e_EnderdragonAction_Takeoff,
		e_EnderdragonAction_Sitting_Flaming,
		e_EnderdragonAction_Sitting_Scanning,
		e_EnderdragonAction_Sitting_Attacking,
	};

	static const int CRYSTAL_COUNT;

	static const int FLAME_TICKS;
	static const float FLAME_ANGLE;
	static const int FLAME_PASSES;
	static const int FLAME_FREQUENCY;
	static const int FLAME_RANGE;

	static const int ATTACK_TICKS;

	static const int SITTING_ATTACK_Y_VIEW_RANGE;
	static const int SITTING_ATTACK_VIEW_RANGE;
	static const int SITTING_ATTACK_RANGE;
	static const int SITTING_POST_ATTACK_IDLE_TICKS;
	static const int SITTING_SCANNING_IDLE_TICKS;
	static const int SITTING_FLAME_ATTACKS_COUNT;

	// The percentage of max health that the dragon will take while in the "Sitting" states before flying away
	static const float SITTING_ALLOWED_DAMAGE_PERCENTAGE;

	static const int PODIUM_X_POS = 0;
	static const int PODIUM_Z_POS = 0;

private:
	shared_ptr<Entity> attackTarget;

public:
	int dragonDeathTime;

public:
	shared_ptr<EnderCrystal> nearestCrystal;

private:
	void _init();

public:
	EnderDragon(Level *level);
	void AddParts();
	virtual ~EnderDragon();

protected:
	virtual void registerAttributes();
	virtual void defineSynchedData();

public:
	void getLatencyPos(doubleArray result, int step, float a);
	virtual void aiStep();

private:
	using MultiEntityMob::hurt;

	void checkCrystals();
	void checkAttack();
	void knockBack(vector<shared_ptr<Entity> > *entities);
	void hurt(vector<shared_ptr<Entity> > *entities);
	void findNewTarget();
	float rotWrap(double d);
	bool checkWalls(AABB *bb);

public:
	virtual bool hurt(shared_ptr<MultiEntityMobPart> MultiEntityMobPart, DamageSource *source, float damage);
	virtual bool hurt(DamageSource *source, float damage);

protected:
	virtual bool reallyHurt(DamageSource *source, float damage);
	virtual void tickDeath();

private:
	void spawnExitPortal(int x, int z);

protected:
	virtual void checkDespawn();
public:
	virtual vector<shared_ptr<Entity> > *getSubEntities();
	virtual bool isPickable();
	Level *getLevel();

protected:
	int getAmbientSound();
	int getHurtSound();
	float getSoundVolume();

private:
	// 4J added for new dragon behaviour
	bool setSynchedAction(EEnderdragonAction action, bool force = false);
	EEnderdragonAction getSynchedAction();
	int findClosestNode(double tX, double tY, double tZ);
	int findClosestNode();
	Path *findPath(int startIndex, int endIndex, Node *finalNode  = NULL);
	Path *reconstruct_path(Node *from, Node *to);

	void strafeAttackTarget();
	void navigateToNextPathNode();

public:	
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	
public:
	void handleCrystalDestroyed(DamageSource *source);

	float getTilt(float a);
	double getHeadYOffset(float a);
	double getHeadYRotDiff(float a);
	double getHeadPartYOffset(int partIndex, doubleArray bodyPos, doubleArray partPos);
	double getHeadPartYRotDiff(int partIndex, doubleArray bodyPos, doubleArray partPos);
	Vec3 *getHeadLookVector(float a);

	virtual wstring getAName() { return app.GetString(IDS_ENDERDRAGON); };
	virtual float getHealth() { return LivingEntity::getHealth(); };
	virtual float getMaxHealth() { return LivingEntity::getMaxHealth(); };
};
