#include "stdafx.h"
#include "../../../Minecraft.World/Entity.h"
#include "../../../Minecraft.World/Level.h"
#include "../../../Minecraft.World/ItemInstance.h"
#include "UseItemTask.h"

UseItemTask::UseItemTask(const int itemId, Tutorial *tutorial, int descriptionId,
	bool enablePreCompletion, vector<TutorialConstraint *> *inConstraints, bool bShowMinimumTime, bool bAllowFade, bool bTaskReminders)
	: TutorialTask( tutorial, descriptionId, enablePreCompletion, inConstraints, bShowMinimumTime, bAllowFade, bTaskReminders ),
	itemId( itemId )
{
}

bool UseItemTask::isCompleted()
{
	return bIsCompleted;
}

void UseItemTask::useItem(shared_ptr<ItemInstance> item,bool bTestUseOnly)
{
	if(bTestUseOnly) return;

	if( item->id == itemId )
		bIsCompleted = true;
}