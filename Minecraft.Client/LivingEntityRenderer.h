#pragma once
#include "ResourceLocation.h"
#include "EntityRenderer.h"
#include "../Minecraft.World/LivingEntity.h"

class LivingEntity;

class LivingEntityRenderer : public EntityRenderer
{
	static const int PLAYER_NAME_READABLE_FULLSCREEN = 16;
	static const int PLAYER_NAME_READABLE_DISTANCE_SPLITSCREEN = 8;
	static const int PLAYER_NAME_READABLE_DISTANCE_SD = 8;

	static ResourceLocation ENCHANT_GLINT_LOCATION;
	static int MAX_ARMOR_LAYERS;

protected:
	//Model *model; // 4J Stu - This shadows the one in EntityRenderer
	Model *armor;

public:
	LivingEntityRenderer(Model *model, float shadow);
	virtual void render(shared_ptr<Entity> mob, double x, double y, double z, float rot, float a);
	virtual void setArmor(Model *armor);

private:
	float rotlerp(float from, float to, float a);

protected:
	virtual void renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale);
	virtual void setupPosition(shared_ptr<LivingEntity> mob, double x, double y, double z);
	virtual void setupRotations(shared_ptr<LivingEntity> mob, float bob, float bodyRot, float a);
	virtual float getAttackAnim(shared_ptr<LivingEntity> mob, float a);
	virtual float getBob(shared_ptr<LivingEntity> mob, float a);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void renderArrows(shared_ptr<LivingEntity> mob, float a);
	virtual int prepareArmorOverlay(shared_ptr<LivingEntity> mob, int layer, float a);
	virtual int prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a);
	virtual void prepareSecondPassArmor(shared_ptr<LivingEntity> mob, int layer, float a);
	virtual float getFlipDegrees(shared_ptr<LivingEntity> mob);
	virtual int getOverlayColor(shared_ptr<LivingEntity> mob, float br, float a);
	virtual void scale(shared_ptr<LivingEntity> mob, float a);
	virtual void renderName(shared_ptr<LivingEntity> mob, double x, double y, double z);
	virtual bool shouldShowName(shared_ptr<LivingEntity> mob);
	virtual void renderNameTags(shared_ptr<LivingEntity> mob, double x, double y, double z, const wstring &msg, float scale, double dist);
	virtual void renderNameTag(shared_ptr<LivingEntity> mob, const wstring &name, double x, double y, double z, int maxDist, int color = 0xff000000);
};