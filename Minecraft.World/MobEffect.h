#pragma once
using namespace std;

#include "AttributeModifier.h"

class Mob;
class MobEffectInstance;
class Attribute;

class MobEffect
{
public:
	enum EMobEffectIcon
	{
		e_MobEffectIcon_None,
		e_MobEffectIcon_Blindness,
		e_MobEffectIcon_FireResistance,
		e_MobEffectIcon_Haste,
		e_MobEffectIcon_Hunger,
		e_MobEffectIcon_Invisiblity,
		e_MobEffectIcon_JumpBoost,
		e_MobEffectIcon_MiningFatigue,
		e_MobEffectIcon_Nausea,
		e_MobEffectIcon_NightVision,
		e_MobEffectIcon_Poison,
		e_MobEffectIcon_Regeneration,
		e_MobEffectIcon_Resistance,
		e_MobEffectIcon_Slowness,
		e_MobEffectIcon_Speed,
		e_MobEffectIcon_Strength,
		e_MobEffectIcon_WaterBreathing,
		e_MobEffectIcon_Weakness,
		e_MobEffectIcon_Wither,
		e_MobEffectIcon_HealthBoost,
		e_MobEffectIcon_Absorption,

		e_MobEffectIcon_COUNT,
	};

	static const int NUM_EFFECTS = 32;
	static MobEffect *effects[NUM_EFFECTS];

	static MobEffect *voidEffect;
	static MobEffect *movementSpeed;
	static MobEffect *movementSlowdown;
	static MobEffect *digSpeed;
	static MobEffect *digSlowdown;
	static MobEffect *damageBoost;
	static MobEffect *heal;
	static MobEffect *harm;
	static MobEffect *jump;
	static MobEffect *confusion;
	static MobEffect *regeneration;
	static MobEffect *damageResistance;
	static MobEffect *fireResistance;
	static MobEffect *waterBreathing;
	static MobEffect *invisibility;
	static MobEffect *blindness;
	static MobEffect *nightVision;
	static MobEffect *hunger;
	static MobEffect *weakness;
	static MobEffect *poison;
	static MobEffect *wither;
	static MobEffect *healthBoost;
	static MobEffect *absorption;
	static MobEffect *saturation;
	static MobEffect *reserved_24;
	static MobEffect *reserved_25;
	static MobEffect *reserved_26;
	static MobEffect *reserved_27;
	static MobEffect *reserved_28;
	static MobEffect *reserved_29;
	static MobEffect *reserved_30;
	static MobEffect *reserved_31;

	const int id;

	static void staticCtor();

private:
	unordered_map<Attribute *, AttributeModifier *> attributeModifiers;
	int descriptionId;
	int m_postfixDescriptionId; // 4J added
	EMobEffectIcon icon; // 4J changed type
	const bool _isHarmful;
	double durationModifier;
	bool _isDisabled;
	const eMinecraftColour color;

protected:
	MobEffect(int id, bool isHarmful, eMinecraftColour color);

	//MobEffect *setIcon(int xPos, int yPos);
	MobEffect *setIcon(EMobEffectIcon icon);

public:
	virtual int getId();
	virtual void applyEffectTick(shared_ptr<LivingEntity> mob, int amplification);
	virtual void applyInstantenousEffect(shared_ptr<LivingEntity> source, shared_ptr<LivingEntity> mob, int amplification, double scale);
	virtual bool isInstantenous();
	virtual bool isDurationEffectTick(int remainingDuration, int amplification);

	MobEffect *setDescriptionId(unsigned int id);
	unsigned int getDescriptionId(int iData = -1);

	// 4J Added
	MobEffect *setPostfixDescriptionId(unsigned int id);
	unsigned int getPostfixDescriptionId(int iData = -1);

	bool hasIcon();
	EMobEffectIcon getIcon(); // 4J changed return type
	bool isHarmful();
	static wstring formatDuration(MobEffectInstance *instance);

protected:
	MobEffect *setDurationModifier(double durationModifier);

public:
	virtual double getDurationModifier();
	virtual MobEffect *setDisabled();
	virtual bool isDisabled();
	virtual eMinecraftColour getColor();

	virtual MobEffect *addAttributeModifier(Attribute *attribute, eMODIFIER_ID id, double amount, int operation);
	virtual unordered_map<Attribute *, AttributeModifier *> *getAttributeModifiers();
	virtual void removeAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier);
	virtual void addAttributeModifiers(shared_ptr<LivingEntity> entity, BaseAttributeMap *attributes, int amplifier);
	virtual double getAttributeModifierValue(int amplifier, AttributeModifier *original);
};