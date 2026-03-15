#include "World.h"

World::World(int dimension)
    : m_dimension(dimension)
{
}

String ^ World::getName()
{
    switch (m_dimension)
    {
    case -1:
        return "world_nether";
    case 1:
        return "world_the_end";
    default:
        return "world";
    }
}

int World::getDimension()
{
    return m_dimension;
}

bool World::createExplosion(double x, double y, double z, float power)
{
    return createExplosion(x, y, z, power, false, true);
}

bool World::createExplosion(double x, double y, double z, float power, bool setFire)
{
    return createExplosion(x, y, z, power, setFire, true);
}

bool World::createExplosion(double x, double y, double z, float power, bool setFire, bool breakBlocks)
{
    return NativeWorldCallbacks::CreateExplosion(m_dimension, x, y, z, power, setFire, breakBlocks);
}

bool World::createExplosion(Location ^ loc, float power)
{
    if (loc == nullptr)
    {
        return false;
    }

    return createExplosion(loc->getX(), loc->getY(), loc->getZ(), power);
}

bool World::createExplosion(Location ^ loc, float power, bool setFire)
{
    if (loc == nullptr)
    {
        return false;
    }

    return createExplosion(loc->getX(), loc->getY(), loc->getZ(), power, setFire);
}

bool World::createExplosion(Location ^ loc, float power, bool setFire, bool breakBlocks)
{
    if (loc == nullptr)
    {
        return false;
    }

    return createExplosion(loc->getX(), loc->getY(), loc->getZ(), power, setFire, breakBlocks);
}

Item ^ World::dropItem(Location ^ location, ItemStack ^ item)
{
    if (location == nullptr || item == nullptr)
    {
        return nullptr;
    }

    DroppedItemData data;
    if (!NativeWorldCallbacks::DropItem(
            m_dimension,
            location->getX(),
            location->getY(),
            location->getZ(),
            item->getTypeId(),
            item->getAmount(),
            item->getData(),
            false,
            &data))
    {
        return nullptr;
    }

    return gcnew Item(
        gcnew Location(data.x, data.y, data.z, m_dimension),
        gcnew ItemStack(data.itemId, data.count, data.data));
}

Item ^ World::dropItemNaturally(Location ^ location, ItemStack ^ item)
{
    if (location == nullptr || item == nullptr)
    {
        return nullptr;
    }

    DroppedItemData data;
    if (!NativeWorldCallbacks::DropItem(
            m_dimension,
            location->getX(),
            location->getY(),
            location->getZ(),
            item->getTypeId(),
            item->getAmount(),
            item->getData(),
            true,
            &data))
    {
        return nullptr;
    }

    return gcnew Item(
        gcnew Location(data.x, data.y, data.z, m_dimension),
        gcnew ItemStack(data.itemId, data.count, data.data));
}

Block ^ World::getBlockAt(int x, int y, int z)
{
    int blockType = NativeBlockCallbacks::GetBlockType(x, y, z, m_dimension);
    int blockData = NativeBlockCallbacks::GetBlockData(x, y, z, m_dimension);

    return gcnew Block(x, y, z, m_dimension, blockType, blockData);
}

Block ^ World::getBlockAt(Location ^ location)
{
    if (location == nullptr)
    {
        return nullptr;
    }

    return getBlockAt(
        (int)Math::Floor(location->getX()),
        (int)Math::Floor(location->getY()),
        (int)Math::Floor(location->getZ()));
}

long long World::getFullTime()
{
    WorldInfoData info;
    return TryGetWorldInfo(&info) ? info.fullTime : 0;
}

Block ^ World::getHighestBlockAt(int x, int z)
{
    int y = getHighestBlockYAt(x, z);
    return getBlockAt(x, y < 0 ? 0 : y, z);
}

Block ^ World::getHighestBlockAt(Location ^ location)
{
    if (location == nullptr)
    {
        return nullptr;
    }

    return getHighestBlockAt(
        (int)Math::Floor(location->getX()),
        (int)Math::Floor(location->getZ()));
}

int World::getHighestBlockYAt(int x, int z)
{
    return NativeWorldCallbacks::GetHighestBlockYAt(m_dimension, x, z);
}

int World::getHighestBlockYAt(Location ^ location)
{
    if (location == nullptr)
    {
        return -1;
    }

    return getHighestBlockYAt(
        (int)Math::Floor(location->getX()),
        (int)Math::Floor(location->getZ()));
}

List<Player ^> ^ World::getPlayers()
{
    return FourKit::GetPlayersInDimension(m_dimension);
}

long long World::getSeed()
{
    WorldInfoData info;
    return TryGetWorldInfo(&info) ? info.seed : 0;
}

Location ^ World::getSpawnLocation()
{
    WorldInfoData info;

    if (!TryGetWorldInfo(&info))
    {
        return gcnew Location(0.0, 0.0, 0.0, m_dimension);
    }

    return gcnew Location(
        (double)info.spawnX,
        (double)info.spawnY,
        (double)info.spawnZ,
        m_dimension);
}

int World::getThunderDuration()
{
    WorldInfoData info;
    return TryGetWorldInfo(&info) ? info.thunderDuration : 0;
}

long long World::getTime()
{
    WorldInfoData info;

    if (!TryGetWorldInfo(&info))
    {
        return 0;
    }

    const long long ticksPerDay = 24000;
    long long time = info.dayTime % ticksPerDay;

    return time < 0 ? time + ticksPerDay : time;
}

bool World::hasStorm()
{
    WorldInfoData info;
    return TryGetWorldInfo(&info) ? info.hasStorm : false;
}

bool World::isThundering()
{
    WorldInfoData info;
    return TryGetWorldInfo(&info) ? info.thundering : false;
}

void World::setFullTime(long long time)
{
    NativeWorldCallbacks::SetFullTime(m_dimension, time);
}

bool World::setSpawnLocation(int x, int y, int z)
{
    return NativeWorldCallbacks::SetSpawnLocation(m_dimension, x, y, z);
}

void World::setStorm(bool hasStorm)
{
    NativeWorldCallbacks::SetStorm(m_dimension, hasStorm);
}

void World::setThunderDuration(int duration)
{
    NativeWorldCallbacks::SetThunderDuration(m_dimension, duration);
}

void World::setThundering(bool thundering)
{
    NativeWorldCallbacks::SetThundering(m_dimension, thundering);
}

void World::setTime(long long time)
{
    NativeWorldCallbacks::SetTime(m_dimension, time);
}

void World::setWeatherDuration(int duration)
{
    NativeWorldCallbacks::SetWeatherDuration(m_dimension, duration);
}

bool World::strikeLightning(double x, double y, double z)
{
    return NativeWorldCallbacks::StrikeLightning(m_dimension, x, y, z, false);
}

bool World::strikeLightningEffect(double x, double y, double z)
{
    return NativeWorldCallbacks::StrikeLightning(m_dimension, x, y, z, true);
}

bool World::strikeLightning(Location^ loc)
{
    if (loc == nullptr)
    {
        return false;
    }

    return NativeWorldCallbacks::StrikeLightning(m_dimension, loc->getX(), loc->getY(), loc->getZ(), false);
}

bool World::strikeLightningEffect(Location ^ loc)
{
    if (loc == nullptr)
    {
        return false;
    }

    return NativeWorldCallbacks::StrikeLightning(m_dimension, loc->getX(), loc->getY(), loc->getZ(), true);
}

bool World::TryGetWorldInfo(WorldInfoData *outData)
{
    if (outData == nullptr)
    {
        return false;
    }

    return NativeWorldCallbacks::GetWorldInfo(m_dimension, outData);
}
