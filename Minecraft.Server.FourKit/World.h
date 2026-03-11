#pragma once

#include "FourKitStructs.h"
#include "Block.h"
#include "Player.h"
#include "Events.h"
#include "FourKit.h"
#include "Item.h"
#include "ItemStack.h"
#include "NativeBlockCallbacks.h"
#include "NativeWorldCallbacks.h"

public ref class World
{
public:
	World(int dimension);

	String^ getName();
	int getDimension();

	bool createExplosion(double x, double y, double z, float power);
	bool createExplosion(double x, double y, double z, float power, bool setFire);
	bool createExplosion(double x, double y, double z, float power, bool setFire, bool breakBlocks);

	bool createExplosion(Location ^ loc, float power);
    bool createExplosion(Location ^ loc, float power, bool setFire);
    bool createExplosion(Location ^ loc, float power, bool setFire, bool breakBlocks);

	Item^ dropItem(Location^ location, ItemStack^ item);
	Item^ dropItemNaturally(Location^ location, ItemStack^ item);

	Block^ getBlockAt(int x, int y, int z);
	Block^ getBlockAt(Location^ location);

	long long getFullTime();

	Block^ getHighestBlockAt(int x, int z);
	Block^ getHighestBlockAt(Location^ location);

	int getHighestBlockYAt(int x, int z);
	int getHighestBlockYAt(Location^ location);

	List<Player^>^ getPlayers();

	long long getSeed();
	Location^ getSpawnLocation();

	int getThunderDuration();

	long long getTime();

	bool hasStorm();
	bool isThundering();

	void setFullTime(long long time);
	bool setSpawnLocation(int x, int y, int z);

	void setStorm(bool hasStorm);
	void setThunderDuration(int duration);
	void setThundering(bool thundering);

	void setTime(long long time);
	void setWeatherDuration(int duration);

    bool strikeLightning(double x, double y, double z);
    bool strikeLightningEffect(double x, double y, double z);
    bool strikeLightning(Location^ loc);
    bool strikeLightningEffect(Location ^ loc);

private:
	bool TryGetWorldInfo(WorldInfoData* outData);

	int m_dimension;
};