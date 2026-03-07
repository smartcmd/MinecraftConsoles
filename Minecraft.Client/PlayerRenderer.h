#pragma once
#include "MobRenderer.h"
#include "../Minecraft.World/Player.h"

class HumanoidModel;

using namespace std;

class PlayerRenderer : public LivingEntityRenderer
{
public:
	// 4J: Made public for use in skull renderer
	static ResourceLocation DEFAULT_LOCATION;

private:
	HumanoidModel *humanoidModel;
    HumanoidModel *armorParts1;
    HumanoidModel *armorParts2;

public:
	PlayerRenderer();

	static unsigned int getNametagColour(int index);

private:
	static const wstring MATERIAL_NAMES[5];

protected:
	virtual int prepareArmor(shared_ptr<LivingEntity> _player, int layer, float a);
	virtual void prepareSecondPassArmor(shared_ptr<LivingEntity> mob, int layer, float a);

public:
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);

protected:
    virtual void additionalRendering(shared_ptr<LivingEntity> _mob, float a);
	void renderNameTags(shared_ptr<LivingEntity> player, double x, double y, double z, wstring msg, float scale, double dist);

    virtual void scale(shared_ptr<LivingEntity> _player, float a);
public:
	void renderHand();

protected:
	virtual void setupPosition(shared_ptr<LivingEntity> _mob, double x, double y, double z);
    virtual void setupRotations(shared_ptr<LivingEntity> _mob, float bob, float bodyRot, float a);

private:	
    virtual void renderShadow(shared_ptr<Entity> e, double x, double y, double z, float pow, float a); // 4J Added override

public:
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> entity);
	
	using LivingEntityRenderer::bindTexture;
	virtual void bindTexture(shared_ptr<Entity> entity); // 4J Added override
};