#pragma once
using namespace std;

#include "..\Minecraft.World\Entity.h"
#include "..\Minecraft.World\ParticleTypes.h"
#include "ParticleEngine.h"
class Tesselator;
class CompoundTag;
class Icon;

class Particle : public Entity
{
protected:
	int texX, texY;
    float uo, vo;
    int age;
    int lifetime;
    float size;
    float gravity;
    float rCol, gCol, bCol;
	float alpha;
    Icon *tex;
public:
	static double xOff, yOff, zOff;
private:
	void _init(Level *level, double x, double y, double z);
protected:
	Particle(Level *level, double x, double y, double z);
public:
    Particle(Level *level, double x, double y, double z, double xa, double ya, double za);
    virtual shared_ptr<Particle> setPower(float power);
    virtual shared_ptr<Particle> scale(float scale);
	void setColor(float r, float g, float b);
	void setAlpha(float alpha);
	float getRedCol();
	float getGreenCol();
	float getBlueCol();
	float getAlpha();
protected:
	virtual bool makeStepSound();
    virtual void defineSynchedData();
public:
	virtual void tick();
	virtual void render(Tesselator *t, float a, float xa, float ya, float za, float xa2, float za2);
	virtual int getParticleTexture();
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	virtual void setTex(Textures *textures, Icon *icon);
	virtual void setMiscTex(int slotIndex);
	virtual void setNextMiscAnimTex();
	virtual bool isAttackable();
	virtual wstring toString();
};