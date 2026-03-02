#pragma once

#include "MobRenderer.h"

#ifdef _XBOX
class EnderDragon;
#endif
class DragonModel;

class EnderDragonRenderer : public MobRenderer
{
public:
	static shared_ptr<EnderDragon> bossInstance;

private:
	static int currentModel;

protected:
	DragonModel *dragonModel;

public:
	EnderDragonRenderer();

protected:
	virtual void setupRotations(shared_ptr<Mob> _mob, float bob, float bodyRot, float a);

protected:
	void renderModel(shared_ptr<Entity> _mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);

public:
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
	virtual void additionalRendering(shared_ptr<Mob> _mob, float a);
	virtual int prepareArmor(shared_ptr<Mob> _mob, int layer, float a);
};