#pragma once
using namespace std;
#include "..\Minecraft.World\HashExtension.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\JavaIntHash.h"

class ClientConnection;
class MultiPlayerChunkCache;

using namespace std;

class MultiPlayerLevel : public Level
{
private:
	static const int TICKS_BEFORE_RESET = 20 * 4;

	class ResetInfo
	{
	public:
		int x, y, z, ticks, tile, data;
		ResetInfo(int x, int y, int z, int tile, int data);
	};

	vector<ResetInfo> updatesToReset;	// 4J - was linked list but vector seems more appropriate
	bool m_bEnableResetChanges; // 4J Added
public:
	void unshareChunkAt(int x, int z);	// 4J - added
	void shareChunkAt(int x, int z);	// 4J - added

	void enableResetChanges(bool enable) { m_bEnableResetChanges = enable; } // 4J Added
private:
	int unshareCheckX;		// 4J - added
	int unshareCheckZ;		// 4J - added
	int compressCheckX;		// 4J - added
	int compressCheckZ;		// 4J - added
	vector<ClientConnection *> connections; // 4J Stu - Made this a vector as we can have more than one local connection
	MultiPlayerChunkCache *chunkCache;
	Minecraft *minecraft;
	Scoreboard *scoreboard;

public:
	MultiPlayerLevel(ClientConnection *connection, LevelSettings *levelSettings, int dimension, int difficulty);
	virtual ~MultiPlayerLevel();
	virtual void tick() ;

	void clearResetRegion(int x0, int y0, int z0, int x1, int y1, int z1);
protected:
	ChunkSource *createChunkSource();	// 4J - was virtual, but was called from parent ctor
public:
	virtual void validateSpawn();
protected:
	virtual void tickTiles();
public:
	void setChunkVisible(int x, int z, bool visible);

private:
	unordered_map<int, shared_ptr<Entity>, IntKeyHash2, IntKeyEq> entitiesById;	// 4J - was IntHashMap
	unordered_set<shared_ptr<Entity> > forced;
	unordered_set<shared_ptr<Entity> > reEntries;

public:
	virtual bool addEntity(shared_ptr<Entity> e);
	virtual void removeEntity(shared_ptr<Entity> e);
protected:
	virtual void entityAdded(shared_ptr<Entity> e);
	virtual void entityRemoved(shared_ptr<Entity> e);
public:
	void putEntity(int id, shared_ptr<Entity> e);
	shared_ptr<Entity> getEntity(int id);
	shared_ptr<Entity> removeEntity(int id);
	virtual void removeEntities(vector<shared_ptr<Entity> > *list); // 4J Added override
	virtual bool setData(int x, int y, int z, int data, int updateFlags, bool forceUpdate =false );
	virtual bool setTileAndData(int x, int y, int z, int tile, int data, int updateFlags);
	bool doSetTileAndData(int x, int y, int z, int tile, int data);
	virtual void disconnect(bool sendDisconnect = true);
	void animateTick(int xt, int yt, int zt);
protected:
	virtual Tickable *makeSoundUpdater(shared_ptr<Minecart> minecart);
	virtual void tickWeather();

	static const int ANIMATE_TICK_MAX_PARTICLES = 500;

public:
	void animateTickDoWork();			// 4J added
	unordered_set<int> chunksToAnimate;	// 4J added

public:
	void removeAllPendingEntityRemovals();

	virtual void playSound(shared_ptr<Entity> entity, int iSound, float volume, float pitch);

	virtual void playLocalSound(double x, double y, double z, int iSound, float volume, float pitch, bool distanceDelay = false, float fClipSoundDist = 16.0f);

	virtual void createFireworks(double x, double y, double z, double xd, double yd, double zd, CompoundTag *infoTag);
	virtual void setScoreboard(Scoreboard *scoreboard);
	virtual void setDayTime(int64_t newTime);

	// 4J Stu - Added so we can have multiple local connections
	void addClientConnection(ClientConnection *c) { connections.push_back( c ); }
	void removeClientConnection(ClientConnection *c, bool sendDisconnect);

	void tickAllConnections();

	void dataReceivedForChunk(int x, int z);	// 4J added
	void removeUnusedTileEntitiesInRegion(int x0, int y0, int z0, int x1, int y1, int z1);	// 4J added
};
