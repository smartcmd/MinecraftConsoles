#pragma once

class Mob;

class MobEffectInstance
{
private:
	// sent as byte
	int id;
	// sent as short
	int duration;
	// sent as byte
	int amplifier;
	bool splash;
	bool ambient;
	bool noCounter;

	void _init(int id, int duration, int amplifier);

public:
	MobEffectInstance(int id);
	MobEffectInstance(int id, int duration);
	MobEffectInstance(int id, int duration, int amplifier);
	MobEffectInstance(int id, int duration, int amplifier, bool ambient);
	MobEffectInstance(MobEffectInstance *copy);

	void update(MobEffectInstance *takeOver);
	int getId();
	int getDuration();
	int getAmplifier();

	bool isSplash();
	void setSplash(bool splash);
	bool isAmbient();

	bool tick(shared_ptr<LivingEntity> target);

private:
	int tickDownDuration();

public:
	void applyEffect(shared_ptr<LivingEntity> mob);
	int getDescriptionId();
	int getPostfixDescriptionId(); // 4J Added
	int hashCode();

	wstring toString();

	// Was bool equals(Object obj)
	bool equals(MobEffectInstance *obj);

	CompoundTag *save(CompoundTag *tag);
	static MobEffectInstance *load(CompoundTag *tag);
	void setNoCounter(bool noCounter);
	bool isNoCounter();
};