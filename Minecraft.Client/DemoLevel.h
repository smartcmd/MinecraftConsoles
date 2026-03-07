#pragma once
#include "../Minecraft.World/net.minecraft.world.level.h"

class DemoLevel : public Level
{
private:
	static const int64_t DEMO_LEVEL_SEED = 0;	// 4J - TODO - was "Don't Look Back".hashCode();
    static const int DEMO_SPAWN_X = 796;
    static const int DEMO_SPAWN_Y = 72;
    static const int DEMO_SPAWN_Z = -731;
public:
	DemoLevel(shared_ptr<LevelStorage> levelStorage, const wstring& levelName);
    DemoLevel(Level *level, Dimension *dimension);
protected:
	virtual void setInitialSpawn();
};
