#pragma once
#include "Entity.h"

class DamageSource;
class Tickable;

class Minecart : public Entity
{
	friend class MinecartRenderer;
public:
	eINSTANCEOF GetType() { return eTYPE_MINECART; };

public:
	static const int TYPE_RIDEABLE = 0;
	static const int TYPE_CHEST = 1;
	static const int TYPE_FURNACE = 2;
	static const int TYPE_TNT = 3;
	static const int TYPE_SPAWNER = 4;
	static const int TYPE_HOPPER = 5;

public:
	static const int serialVersionUID = 0;

private:
	static const int DATA_ID_HURT = 17;
	static const int DATA_ID_HURTDIR = 18;
	static const int DATA_ID_DAMAGE = 19;
	static const int DATA_ID_DISPLAY_TILE = 20;
	static const int DATA_ID_DISPLAY_OFFSET = 21;
	static const int DATA_ID_CUSTOM_DISPLAY = 22;

	bool flipped;
	Tickable *soundUpdater;
	wstring name;

protected:
	// 4J Added
	bool m_bHasPushedCartThisTick;

public:
	void _init();

	Minecart(Level *level);
	virtual ~Minecart();

	static shared_ptr<Minecart> createMinecart(Level *level, double x, double y, double z, int type);

protected:
	virtual bool makeStepSound();
	virtual void defineSynchedData();

public:
	virtual AABB *getCollideAgainstBox(shared_ptr<Entity> entity);
	virtual AABB *getCollideBox();
	virtual bool isPushable();

	Minecart(Level *level, double x, double y, double z);

	virtual double getRideHeight();
	virtual bool hurt(DamageSource *source, float damage);
	virtual void destroy(DamageSource *source);
	virtual void animateHurt();
	virtual bool isPickable();
	virtual void remove();

private:
	static const int EXITS[][2][3];

public:
	virtual void tick();
	virtual void activateMinecart(int xt, int yt, int zt, bool state);

protected:
	virtual void comeOffTrack(double maxSpeed);
	virtual void moveAlongTrack(int xt, int yt, int zt, double maxSpeed, double slideSpeed, int tile, int data);
	virtual void applyNaturalSlowdown();
	virtual Vec3 *getPosOffs(double x, double y, double z, double offs);
	virtual Vec3 *getPos(double x, double y, double z);

protected:
	virtual void addAdditonalSaveData(CompoundTag *base);
	virtual void readAdditionalSaveData(CompoundTag *base);

public:
	virtual float getShadowHeightOffs();
	using Entity::push;
	virtual void push(shared_ptr<Entity> e);

private:
	int lSteps;
	double lx, ly, lz, lyr, lxr;
	double lxd, lyd, lzd;

public:
	virtual void lerpTo(double x, double y, double z, float yRot, float xRot, int steps);
	virtual void lerpMotion(double xd, double yd, double zd);

	virtual void setDamage(float damage);
	virtual float getDamage();
	virtual void setHurtTime(int hurtTime);
	virtual int getHurtTime();
	virtual void setHurtDir(int hurtDir);
	virtual int getHurtDir();

	virtual int getType() = 0;

	virtual Tile *getDisplayTile();
	virtual Tile *getDefaultDisplayTile();
	virtual int getDisplayData();
	virtual int getDefaultDisplayData();
	virtual int getDisplayOffset();
	virtual int getDefaultDisplayOffset();
	virtual void setDisplayTile(int id);
	virtual void setDisplayData(int data);
	virtual void setDisplayOffset(int offset);
	virtual bool hasCustomDisplay();
	virtual void setCustomDisplay(bool value);
	virtual void setCustomName(const wstring &name);
	virtual wstring getAName();
	virtual bool hasCustomName();
	virtual wstring getCustomName();
};
