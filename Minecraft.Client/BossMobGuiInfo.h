#pragma once

class BossMob;

class BossMobGuiInfo
{
public:
    static float healthProgress;
    static int displayTicks;
    static wstring name;
    static bool darkenWorld;

    static void setBossHealth(shared_ptr<BossMob> boss, bool darkenWorld);
};