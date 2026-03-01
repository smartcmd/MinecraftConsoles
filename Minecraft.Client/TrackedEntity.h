#pragma once
class Entity;
//class ServerPlayer;
#include "ServerPlayer.h"
class Packet;
using namespace std;

class EntityTracker;

#define TRACKED_ENTITY_MINIMUM_VIEW_DISTANCE 4

class TrackedEntity
{
private:
	static const int TOLERANCE_LEVEL = 4;

public:
	shared_ptr<Entity> e;

    int range, updateInterval;
    int xp, yp, zp, yRotp, xRotp, yHeadRotp;
    double xap, yap, zap;
    int tickCount;

private:
	double xpu, ypu, zpu;
    bool updatedPlayerVisibility;
    bool trackDelta;
    int teleportDelay;
	shared_ptr<Entity> lastRidingEntity;
	bool wasRiding;

public:
	bool moved;

    unordered_set<shared_ptr<ServerPlayer> , PlayerKeyHash, PlayerKeyEq > seenBy;

    TrackedEntity(shared_ptr<Entity> e, int range, int updateInterval, bool trackDelta);

    void tick(EntityTracker *tracker, vector<shared_ptr<Player> > *players);

private:
	void sendDirtyEntityData();

public:
    void broadcast(shared_ptr<Packet> packet);
    void broadcastAndSend(shared_ptr<Packet> packet);
    void broadcastRemoved();
    void removePlayer(shared_ptr<ServerPlayer> sp);

private:
	bool canBySeenBy(shared_ptr<ServerPlayer> player);

	enum eVisibility
	{
		eVisibility_NotVisible = 0,
		eVisibility_IsVisible = 1,
		eVisibility_SeenAndVisible = 2,
	};

	eVisibility isVisible(EntityTracker *tracker, shared_ptr<ServerPlayer> sp, bool forRider = false); // 4J Added forRider
	
public:
    void updatePlayer(EntityTracker *tracker, shared_ptr<ServerPlayer> sp);
    void updatePlayers(EntityTracker *tracker, vector<shared_ptr<Player> > *players);
private:
	void sendEntityData(shared_ptr<PlayerConnection> conn);
	shared_ptr<Packet> getAddEntityPacket();
public:
    void clear(shared_ptr<ServerPlayer> sp);
};
