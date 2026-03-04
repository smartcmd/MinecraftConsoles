#include "stdafx.h"
#include "BossMobGuiInfo.h"
#include "../Minecraft.World/BossMob.h"

float BossMobGuiInfo::healthProgress = 0.0f;
int BossMobGuiInfo::displayTicks = 0;
wstring BossMobGuiInfo::name = L"";
bool BossMobGuiInfo::darkenWorld = false;

void BossMobGuiInfo::setBossHealth(shared_ptr<BossMob> boss, bool darkenWorld)
{
    healthProgress = (float) boss->getHealth() / (float) boss->getMaxHealth();
    displayTicks = SharedConstants::TICKS_PER_SECOND * 5;
    name = boss->getAName();
    BossMobGuiInfo::darkenWorld = darkenWorld;
}