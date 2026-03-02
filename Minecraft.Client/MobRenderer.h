#pragma once
#include "EntityRenderer.h"
class Mob;
using namespace std;

#define PLAYER_NAME_READABLE_FULLSCREEN 16
#define PLAYER_NAME_READABLE_DISTANCE_SPLITSCREEN 8
#define PLAYER_NAME_READABLE_DISTANCE_SD 8

// 4J - this used to be a generic : public class MobRenderer<T extends Mob> extends EntityRenderer<T>
class MobRenderer : public EntityRenderer
{
private:
	static const int MAX_ARMOR_LAYERS = 4;

protected:
	Model *model;
    Model *armor;

public:
	MobRenderer(Model *model, float shadow);
    virtual void setArmor(Model *armor);
private:
	float rotlerp(float from, float to, float a);
public:
    virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
protected:
	virtual void renderModel(shared_ptr<Entity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
	virtual void setupPosition(shared_ptr<Mob> mob, double x, double y, double z);
    virtual void setupRotations(shared_ptr<Mob> mob, float bob, float bodyRot, float a);
    virtual float getAttackAnim(shared_ptr<Mob> mob, float a);
    virtual float getBob(shared_ptr<Mob> mob, float a);
    virtual void additionalRendering(shared_ptr<Mob> mob, float a);
    virtual int prepareArmorOverlay(shared_ptr<Mob> mob, int layer, float a);
    virtual int prepareArmor(shared_ptr<Mob> mob, int layer, float a);
	virtual void prepareSecondPassArmor(shared_ptr<Mob> mob, int layer, float a);
    virtual float getFlipDegrees(shared_ptr<Mob> mob);
    virtual int getOverlayColor(shared_ptr<Mob> mob, float br, float a);
    virtual void scale(shared_ptr<Mob> mob, float a);
    virtual void renderName(shared_ptr<Mob> mob, double x, double y, double z);
    virtual void renderNameTag(shared_ptr<Mob> mob, const wstring& name, double x, double y, double z, int maxDist, int color = 0xff000000);

public:
	// 4J Added
	virtual Model *getModel() { return model; }
};
