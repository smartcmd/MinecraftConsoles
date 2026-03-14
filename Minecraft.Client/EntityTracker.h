#pragma once
#include "../Minecraft.World/HashExtension.h"
#include "../Minecraft.World/JavaIntHash.h"
class Entity;
class ServerPlayer;
class TrackedEntity;
class MinecraftServer;
class Packet;

using namespace std;

class EntityTracker
{
private:
	ServerLevel *level;
	unordered_set<shared_ptr<TrackedEntity> > entities;
    unordered_map<int, shared_ptr<TrackedEntity> , IntKeyHash2, IntKeyEq> entityMap;	// was IntHashMap
    int maxRange;

public:
	EntityTracker(ServerLevel *level);
    void addEntity(shared_ptr<Entity> e);
    void addEntity(shared_ptr<Entity> e, int range, int updateInterval);
    void addEntity(shared_ptr<Entity> e, int range, int updateInterval, bool trackDeltas);
    void removeEntity(shared_ptr<Entity> e);
	void removePlayer(shared_ptr<Entity> e);	// 4J added
    void tick();
    void broadcast(shared_ptr<Entity> e, shared_ptr<Packet> packet);
    void broadcastAndSend(shared_ptr<Entity> e, shared_ptr<Packet> packet);
    void clear(shared_ptr<ServerPlayer> serverPlayer);
	void playerLoadedChunk(shared_ptr<ServerPlayer> player, LevelChunk *chunk);
	void updateMaxRange();	// AP added for Vita


	// 4J-JEV: Added, needed access to tracked entity of a riders mount.
	shared_ptr<TrackedEntity> getTracker(shared_ptr<Entity> entity);
};
