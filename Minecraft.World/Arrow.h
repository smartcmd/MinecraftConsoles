#pragma once
using namespace std;

#include "Entity.h"

class Level;
class CompoundTag;

class Arrow : public Entity
{
public:
	eINSTANCEOF GetType() {	return eTYPE_ARROW; }
	static Entity *create(Level *level) { return new Arrow(level); }

private:
	// base damage, multiplied with velocity
	static const double ARROW_BASE_DAMAGE;

public:
	static const int PICKUP_DISALLOWED = 0;
	static const int PICKUP_ALLOWED = 1;
	static const int PICKUP_CREATIVE_ONLY = 2;

private:
	static const int ID_FLAGS = 16;
	static const int FLAG_CRIT = 1;

private:
    int xTile;
    int yTile;
    int zTile;
    int lastTile;
    int lastData;
    bool inGround;

public:
	int pickup;
    int shakeTime;
    shared_ptr<Entity> owner;

private:
	double baseDamage;

    int knockback;

private:
	int life;
    int flightTime;

	// 4J - added common ctor code.
	void _init();

public:
	Arrow(Level *level);
	Arrow(Level *level, shared_ptr<Mob> mob, shared_ptr<Mob> target, float power, float uncertainty);
    Arrow(Level *level, double x, double y, double z);
    Arrow(Level *level, shared_ptr<Mob> mob, float power);

protected:
	virtual void defineSynchedData();

public:
	void shoot(double xd, double yd, double zd, float pow, float uncertainty);
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);
    virtual void tick();
    virtual void addAdditonalSaveData(CompoundTag *tag);
    virtual void readAdditionalSaveData(CompoundTag *tag);
    virtual void playerTouch(shared_ptr<Player> player);
    virtual float getShadowHeightOffs();

	void setBaseDamage(double baseDamage);
	double getBaseDamage();
	void setKnockback(int knockback);
	virtual bool isAttackable();
	void setCritArrow(bool critArrow);
	bool isCritArrow();
};