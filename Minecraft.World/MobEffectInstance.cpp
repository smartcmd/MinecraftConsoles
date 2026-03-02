#include "stdafx.h"
#include "net.minecraft.world.effect.h"

void MobEffectInstance::_init(int id, int duration, int amplifier)
{
	this->id = id;
	this->duration = duration;
	this->amplifier = amplifier;
}

MobEffectInstance::MobEffectInstance(int id)
{
	_init(id, 0, 0);
}

MobEffectInstance::MobEffectInstance(int id, int duration)
{
	_init(id, duration, 0);
}

MobEffectInstance::MobEffectInstance(int id, int duration, int amplifier)
{
	_init(id,duration,amplifier);
}

MobEffectInstance::MobEffectInstance(MobEffectInstance *copy)
{
	this->id = copy->id;
	this->duration = copy->duration;
	this->amplifier = copy->amplifier;
}

void MobEffectInstance::update(MobEffectInstance *takeOver)
{
	if (this->id != takeOver->id)
	{
		app.DebugPrintf("This method should only be called for matching effects!");
	}
	if (takeOver->amplifier > this->amplifier)
	{
		this->amplifier = takeOver->amplifier;
		this->duration = takeOver->duration;
	}
	else if (takeOver->amplifier == this->amplifier && this->duration < takeOver->duration)
	{
		this->duration = takeOver->duration;
	}
}

int MobEffectInstance::getId()
{
	return id;
}

int MobEffectInstance::getDuration()
{
	return duration;
}

int MobEffectInstance::getAmplifier()
{
	return amplifier;
}

/**
* Runs the effect on a Mob target.
* 
* @param target
* @return True if the effect is still active.
*/
bool MobEffectInstance::tick(shared_ptr<Mob> target)
{
	if (duration > 0)
	{
		if (MobEffect::effects[id]->isDurationEffectTick(duration, amplifier))
		{
			applyEffect(target);
		}
		tickDownDuration();
	}
	return duration > 0;
}

int MobEffectInstance::tickDownDuration()
{
	return --duration;
}

void MobEffectInstance::applyEffect(shared_ptr<Mob> mob)
{
	if (duration > 0)
	{
		MobEffect::effects[id]->applyEffectTick(mob, amplifier);
	}
}

int MobEffectInstance::getDescriptionId()
{
	return MobEffect::effects[id]->getDescriptionId();
}

// 4J Added
int MobEffectInstance::getPostfixDescriptionId()
{
	return MobEffect::effects[id]->getPostfixDescriptionId();
}

int MobEffectInstance::hashCode()
{
	//return id;

	// 4J Stu - Changed this to return a value that represents id, amp and duration
	return (id & 0xff) | ( (amplifier & 0xff) << 8) | ( (duration & 0xffff) << 16);
}

wstring MobEffectInstance::toString()
{
	wstring result = L"MobEffectInstance::toString - NON IMPLEMENTED OR LOCALISED FUNCTION";
	//wstring result = "";
	//if (getAmplifier() > 0)
	//{
	//	result = getDescriptionId() + " x " + (getAmplifier() + 1) + ", Duration: " + getDuration();
	//}
	//else
	//{
	//	result = getDescriptionId() + ", Duration: " + getDuration();
	//}
	//if (MobEffect.effects[id].isDisabled())
	//{
	//	return "(" + result + ")";
	//}
	return result;
}

// Was bool equals(Object obj)
bool MobEffectInstance::equals(MobEffectInstance *obj)
{
	return this->id == obj->id && this->amplifier == obj->amplifier && this->duration == obj->duration;
}