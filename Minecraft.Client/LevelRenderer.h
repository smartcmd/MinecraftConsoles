#pragma once
#include "..\Minecraft.World\LevelListener.h"
#include "..\Minecraft.World\Definitions.h"
#include "OffsettedRenderList.h"
#include "..\Minecraft.World\JavaIntHash.h"
#include "..\Minecraft.World\Level.h"
#include <xmcore.h>
#ifdef __PS3__
#include "C4JSpursJob.h"
#endif
class MultiPlayerLevel;
class Textures;
class Chunk;
class Minecraft;
class TileRenderer;
class Culler;
class Entity;
class TileEntity;
class Mob;
class Vec3;
class Particle;
class BlockDestructionProgress;
class IconRegister;
class Tesselator;
using namespace std;

// AP - this is a system that works out which chunks actually need to be grouped together via the deferral system when doing chunk::rebuild. Doing this will reduce the number
// of chunks built in a single group and reduce the chance of seeing through the landscape when digging near the edges/corners of a chunk.
// I've added another chunk flag to mark a chunk critical so it swipes a bit from the reference count value (goes to 3 bits to 2). This works on Vita because it doesn't have 
// split screen reference counting.
#ifdef __PSVITA__
#define _CRITICAL_CHUNKS
#endif

class LevelRenderer : public LevelListener
{
	friend class Chunk;
public:
	static const int CHUNK_XZSIZE = 16;
#ifdef _LARGE_WORLDS
	static const int CHUNK_SIZE = 16;
#else
	static const int CHUNK_SIZE = 16;
#endif
	static const int CHUNK_Y_COUNT = Level::maxBuildHeight / CHUNK_SIZE;
#if defined _XBOX_ONE
	static const int MAX_COMMANDBUFFER_ALLOCATIONS = 2047 * 1024 * 1024;		// Changed to 2047. 4J had set to 512.
#elif defined __ORBIS__
	static const int MAX_COMMANDBUFFER_ALLOCATIONS = 448 * 1024 * 1024;		// 4J - added - hard limit is 512 so giving a lot of headroom here for fragmentation (have seen 16MB lost to fragmentation in multiplayer crash dump before)
#elif defined __PS3__
	static const int MAX_COMMANDBUFFER_ALLOCATIONS = 110 * 1024 * 1024;		// 4J - added
#else
	static const int MAX_COMMANDBUFFER_ALLOCATIONS = 55 * 1024 * 1024;		// 4J - added
#endif
public:
	LevelRenderer(Minecraft *mc, Textures *textures);
private:
	void renderStars();
	void createCloudMesh();	// 4J added
public:
	void setLevel(int playerIndex, MultiPlayerLevel *level);
	void allChanged();
	void allChanged(int playerIndex);

	// 4J-PB added
	void AddDLCSkinsToMemTextures();
public:
	void renderEntities(Vec3 *cam, Culler *culler, float a);
	wstring gatherStats1();
	wstring gatherStats2();
private:
	void resortChunks(int xc, int yc, int zc);
public:
	int render(shared_ptr<Mob> player, int layer, double alpha, bool updateChunks);
private:
	int renderChunks(int from, int to, int layer, double alpha);
public:
	int activePlayers();	// 4J - added
public:
	void renderSameAsLast(int layer, double alpha);
	void tick();
	void renderSky(float alpha);
	void renderHaloRing(float alpha);
	void renderClouds(float alpha);
	bool isInCloud(double x, double y, double z, float alpha);
	void renderAdvancedClouds(float alpha);
	bool updateDirtyChunks();

public:
	void renderHit(shared_ptr<Player> player, HitResult *h, int mode, shared_ptr<ItemInstance> inventoryItem, float a);
	void renderDestroyAnimation(Tesselator *t, shared_ptr<Player> player, float a);
	void renderHitOutline(shared_ptr<Player> player, HitResult *h, int mode, shared_ptr<ItemInstance> inventoryItem, float a);
	void render(AABB *b);
	void setDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level);		// 4J - added level param
	void tileChanged(int x, int y, int z);
	void tileLightChanged(int x, int y, int z);
	void setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level);	// 4J - added level param

#ifdef __PS3__
	void cull_SPU(int playerIndex, Culler *culler, float a);
	void waitForCull_SPU();
	C4JSpursJobQueue::Port* m_jobPort_CullSPU;
	C4JSpursJobQueue::Port* m_jobPort_FindNearestChunk;
	bool	m_bSPUCullStarted[4];
#endif // __PS3__
	void cull(Culler *culler, float a);
	void playStreamingMusic(const wstring& name, int x, int y, int z);
	void playSound(int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist=16.0f);
	void playSound(shared_ptr<Entity> entity,int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist=16.0f);
	void addParticle(ePARTICLE_TYPE eParticleType, double x, double y, double z, double xa, double ya, double za); // 4J added
	shared_ptr<Particle> addParticleInternal(ePARTICLE_TYPE eParticleType, double x, double y, double z, double xa, double ya, double za); // 4J added
	void entityAdded(shared_ptr<Entity> entity);
	void entityRemoved(shared_ptr<Entity> entity);
	void playerRemoved(shared_ptr<Entity> entity) {}		// 4J added - for when a player is removed from the level's player array, not just the entity storage
	void skyColorChanged();
	void clear();
	void levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data);
	void destroyTileProgress(int id, int x, int y, int z, int progress);
	void registerTextures(IconRegister *iconRegister);

	typedef unordered_map<int, vector<shared_ptr<TileEntity> >, IntKeyHash, IntKeyEq> rteMap;
private:

	// debug
	int m_freezeticks; // used to freeze the clouds

	// 4J - this block of declarations was scattered round the code but have gathered everything into one place
	rteMap renderableTileEntities;			// 4J - changed - was vector<shared_ptr<TileEntity>, now hashed by chunk so we can find them
	CRITICAL_SECTION					m_csRenderableTileEntities;
	MultiPlayerLevel *level[4];					// 4J - now one per player
	Textures *textures;
	//    vector<Chunk *> *sortedChunks[4];	// 4J - removed - not sorting our chunks anymore
	ClipChunkArray chunks[4];			// 4J - now one per player
	int lastPlayerCount[4];				// 4J - added
	int xChunks, yChunks, zChunks;
	int chunkLists;
	Minecraft *mc;
	TileRenderer *tileRenderer[4];		// 4J - now one per player
	int ticks;
	int starList, skyList, darkList, haloRingList;
	int cloudList;	// 4J added
	int xMinChunk, yMinChunk, zMinChunk;
	int xMaxChunk, yMaxChunk, zMaxChunk;
	int lastViewDistance;
	int noEntityRenderFrames;
	int totalEntities;
	int renderedEntities;
	int culledEntities;
	int chunkFixOffs;
	vector<Chunk *> _renderChunks;
	int frame;
	int repeatList;
	double xOld[4];						// 4J - now one per player
	double yOld[4];						// 4J - now one per player
	double zOld[4];						// 4J - now one per player

	int totalChunks, offscreenChunks, occludedChunks, renderedChunks, emptyChunks;
	static const int RENDERLISTS_LENGTH = 4;		// 4J - added
	OffsettedRenderList renderLists[RENDERLISTS_LENGTH];

	unordered_map<int, BlockDestructionProgress *> destroyingBlocks;
	Icon **breakingTextures;

public:
	void				fullyFlagRenderableTileEntitiesToBeRemoved();		// 4J added

	CRITICAL_SECTION	m_csDirtyChunks;
	bool				m_nearDirtyChunk;


	// 4J - Destroyed Tile Management - these things added so we can track tiles which have been recently destroyed, and
	// provide temporary collision for them until the render data has been updated to reflect this change
	class DestroyedTileManager
	{
	private:
		class RecentTile
		{
		public:
			int			x;
			int			y;
			int			z;
			Level		*level;
			AABBList	boxes;
			int			timeout_ticks;
			bool		rebuilt;
			RecentTile(int x, int y, int z, Level *level);
			~RecentTile();
		};
		CRITICAL_SECTION			m_csDestroyedTiles;
		vector<RecentTile *>		m_destroyedTiles;
	public:
		void destroyingTileAt( Level *level, int x, int y, int z );									// For game to let this manager know that a tile is about to be destroyed (must be called before it actually is)
		void updatedChunkAt( Level * level, int x, int y, int z, int veryNearCount );				// For chunk rebuilding to inform the manager that a chunk (a 16x16x16 tile render chunk) has been updated
		void addAABBs( Level *level, AABB *box, AABBList *boxes );									// For game to get any AABBs that the user should be colliding with as render data has not yet been updated
		void tick();
		DestroyedTileManager();
		~DestroyedTileManager();
	};
	DestroyedTileManager *destroyedTileManager;

	float destroyProgress;

	// 4J - added for new render list handling
	// This defines the maximum size of renderable level, must be big enough to cope with actual size of level + view distance at each side
	// so that we can render the "infinite" sea at the edges
	static const int	MAX_LEVEL_RENDER_SIZE[3];	
	static const int    DIMENSION_OFFSETS[3];
	// This is the TOTAL area of columns of chunks to be allocated for render round the players. So for one player, it would be a region of
	// sqrt(PLAYER_RENDER_AREA) x sqrt(PLAYER_RENDER_AREA)
#ifdef _LARGE_WORLDS
	static const int	PLAYER_VIEW_DISTANCE = 18; // Straight line distance from centre to extent of visible world
	static const int	PLAYER_RENDER_AREA = (PLAYER_VIEW_DISTANCE * PLAYER_VIEW_DISTANCE * 4);
#else
	static const int	PLAYER_RENDER_AREA = 400;
#endif

	static int			getDimensionIndexFromId(int id);
	static int			getGlobalIndexForChunk(int x, int y, int z, Level *level);
	static int			getGlobalIndexForChunk(int x, int y, int z, int dimensionId);
	static bool			isGlobalIndexInSameDimension( int idx, Level *level);
	static int			getGlobalChunkCount();
	static int			getGlobalChunkCountForOverworld();

	// Get/set/clear individual flags
	bool				getGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift = 0);
	void				setGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift = 0);
	void				setGlobalChunkFlag(int index, unsigned char flag, unsigned char shift = 0);
	void				clearGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift = 0);

	// Get/set whole byte of flags
	unsigned char		getGlobalChunkFlags(int x, int y, int z, Level *level);
	void				setGlobalChunkFlags(int x, int y, int z, Level *level, unsigned char flags);

	// Reference counting
	unsigned char		incGlobalChunkRefCount(int x, int y, int z, Level *level);
	unsigned char		decGlobalChunkRefCount(int x, int y, int z, Level *level);

	// Actual storage for flags
	unsigned char		*globalChunkFlags;

	// The flag definitions
	static const int    CHUNK_FLAG_COMPILED		= 0x01;
	static const int    CHUNK_FLAG_DIRTY		= 0x02;
	static const int    CHUNK_FLAG_EMPTY0		= 0x04;
	static const int    CHUNK_FLAG_EMPTY1		= 0x08;
	static const int	CHUNK_FLAG_EMPTYBOTH	= 0x0c;
	static const int    CHUNK_FLAG_NOTSKYLIT	= 0x10;
#ifdef _CRITICAL_CHUNKS
	static const int    CHUNK_FLAG_CRITICAL		= 0x20;
	static const int    CHUNK_FLAG_CUT_OUT		= 0x40;
	static const int	CHUNK_FLAG_REF_MASK		= 0x01;
	static const int	CHUNK_FLAG_REF_SHIFT	= 7;
#else
	static const int	CHUNK_FLAG_REF_MASK		= 0x07;
	static const int	CHUNK_FLAG_REF_SHIFT	= 5;
#endif

	XLockFreeStack<int> dirtyChunksLockFreeStack;

	bool				dirtyChunkPresent;
	__int64				lastDirtyChunkFound;
	static const int	FORCE_DIRTY_CHUNK_CHECK_PERIOD_MS = 250;

#ifdef _LARGE_WORLDS
	static const int MAX_CONCURRENT_CHUNK_REBUILDS = 4;
	static const int MAX_CHUNK_REBUILD_THREADS = MAX_CONCURRENT_CHUNK_REBUILDS - 1;
	static Chunk permaChunk[MAX_CONCURRENT_CHUNK_REBUILDS];
	static C4JThread *rebuildThreads[MAX_CHUNK_REBUILD_THREADS];
	static C4JThread::EventArray *s_rebuildCompleteEvents;
	static C4JThread::Event *s_activationEventA[MAX_CHUNK_REBUILD_THREADS];
	static void staticCtor();
	static int rebuildChunkThreadProc(LPVOID lpParam);

	CRITICAL_SECTION m_csChunkFlags;
#endif
	void nonStackDirtyChunksAdded();
};
