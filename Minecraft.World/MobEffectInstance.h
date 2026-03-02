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

	void _init(int id, int duration, int amplifier);

public:
	MobEffectInstance(int id);
	MobEffectInstance(int id, int duration);
	MobEffectInstance(int id, int duration, int amplifier);
	MobEffectInstance(MobEffectInstance *copy);

	void update(MobEffectInstance *takeOver);
	int getId();
	int getDuration();
	int getAmplifier();
	bool tick(shared_ptr<Mob> target);

private:
	int tickDownDuration();

public:
	void applyEffect(shared_ptr<Mob> mob);
	int getDescriptionId();
	int getPostfixDescriptionId(); // 4J Added
	int hashCode();

	wstring toString();

	// Was bool equals(Object obj)
	bool equals(MobEffectInstance *obj);
};