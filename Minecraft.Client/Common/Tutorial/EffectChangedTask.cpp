#include "stdafx.h"
#include "../../../Minecraft.World/net.minecraft.world.effect.h"
#include "EffectChangedTask.h"

EffectChangedTask::EffectChangedTask(Tutorial *tutorial, int descriptionId, MobEffect *effect, bool apply,
		bool enablePreCompletion, bool bShowMinimumTime, bool bAllowFade, bool bTaskReminders )
		: TutorialTask(tutorial,descriptionId,enablePreCompletion,NULL,bShowMinimumTime,bAllowFade,bTaskReminders)
{
	m_effect = effect;
	m_apply = apply;
}

bool EffectChangedTask::isCompleted()
{
	return bIsCompleted;
}

void EffectChangedTask::onEffectChanged(MobEffect *effect, bool bRemoved /*=false*/)
{
	if(effect == m_effect)
	{
		if(m_apply == !bRemoved)
		{
			bIsCompleted = true;
		}
		else
		{
			bIsCompleted = false;
		}
	}
}