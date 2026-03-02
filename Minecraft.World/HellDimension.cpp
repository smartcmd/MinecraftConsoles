#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.storage.h"
#include "HellDimension.h"
#include "net.minecraft.world.level.levelgen.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.tile.h"
#include "..\Minecraft.Client\Minecraft.h"
#include "..\Minecraft.Client\Common\Colours\ColourTable.h"

void HellDimension::init()
{
    biomeSource = new FixedBiomeSource(Biome::hell, 1, 0);
    ultraWarm = true;
    hasCeiling = true;
    id = -1;
}

Vec3 *HellDimension::getFogColor(float td, float a) const
{
	int colour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Nether_Fog_Colour );
	byte redComponent = ((colour>>16)&0xFF);
	byte greenComponent = ((colour>>8)&0xFF);
	byte blueComponent = ((colour)&0xFF);

	float rr = (float)redComponent/256;//0.2f;
	float gg = (float)greenComponent/256;//0.03f;
	float bb = (float)blueComponent/256;//0.03f;
	return Vec3::newTemp(rr, gg, bb);
}

void HellDimension::updateLightRamp()
{
    float ambientLight = 0.10f;
    for (int i = 0; i <= Level::MAX_BRIGHTNESS; i++)
	{
        float v = (1 - i / (float) (Level::MAX_BRIGHTNESS));
        brightnessRamp[i] = ((1 - v) / (v * 3 + 1)) * (1 - ambientLight) + ambientLight;
    }
}

ChunkSource *HellDimension::createRandomLevelSource() const
{
#ifdef _DEBUG_MENUS_ENABLED
	if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_SuperflatNether))
	{
		return new HellFlatLevelSource(level, level->getSeed());
	}
	else
#endif
	if (levelType == LevelType::lvl_flat) 
	{
		return new HellFlatLevelSource(level, level->getSeed());
	}
	else
	{
		return new HellRandomLevelSource(level, level->getSeed());
	}
}

bool HellDimension::isNaturalDimension()
{
	return false;
}

bool HellDimension::isValidSpawn(int x, int z) const
{
    return false;
}

float HellDimension::getTimeOfDay(__int64 time, float a) const
{
	return 0.5f;
}

bool HellDimension::mayRespawn() const
{
	return false;
}

bool HellDimension::isFoggyAt(int x, int z)
{
	return true;
}

int HellDimension::getXZSize()
{
	return ceil((float)level->getLevelData()->getXZSize() / level->getLevelData()->getHellScale());
}
