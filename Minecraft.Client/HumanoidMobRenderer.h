#pragma once
#include "MobRenderer.h"

class HumanoidModel;
class Giant;
class ArmorItem;

class HumanoidMobRenderer : public MobRenderer
{
private:
	static const wstring MATERIAL_NAMES[5];
	static std::map<wstring, ResourceLocation> ARMOR_LOCATION_CACHE;

protected:
	HumanoidModel *humanoidModel;
	float _scale;	
	HumanoidModel *armorParts1;
	HumanoidModel *armorParts2;

	void _init(HumanoidModel *humanoidModel, float scale);
public:
	static ResourceLocation *getArmorLocation(ArmorItem *armorItem, int layer);
	static ResourceLocation *getArmorLocation(ArmorItem *armorItem, int layer, bool overlay);

	HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow);
	HumanoidMobRenderer(HumanoidModel *humanoidModel, float shadow, float scale);

	virtual void prepareSecondPassArmor(shared_ptr<LivingEntity> mob, int layer, float a);

protected:
	virtual void createArmorParts();
	virtual int prepareArmor(shared_ptr<LivingEntity> _mob, int layer, float a);
	virtual void render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
	virtual ResourceLocation *getTextureLocation(shared_ptr<Entity> mob);
	virtual void prepareCarriedItem(shared_ptr<Entity> mob, shared_ptr<ItemInstance> item);
	virtual void additionalRendering(shared_ptr<LivingEntity> mob, float a);
	virtual void scale(shared_ptr<LivingEntity> mob, float a);	
};