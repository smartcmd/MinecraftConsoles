#include "stdafx.h"
#include "LevelRenderer.h"
#include "Textures.h"
#include "TextureAtlas.h"
#include "Tesselator.h"
#include "Chunk.h"
#include "EntityRenderDispatcher.h"
#include "TileEntityRenderDispatcher.h"
#include "DistanceChunkSorter.h"
#include "DirtyChunkSorter.h"
#include "MobSkinTextureProcessor.h"
#include "MobSkinMemTextureProcessor.h"
#include "GameRenderer.h"
#include "BubbleParticle.h"
#include "SmokeParticle.h"
#include "NoteParticle.h"
#include "NetherPortalParticle.h"
#include "EnderParticle.h"
#include "ExplodeParticle.h"
#include "FlameParticle.h"
#include "LavaParticle.h"
#include "FootstepParticle.h"
#include "SplashParticle.h"
#include "SmokeParticle.h"
#include "RedDustParticle.h"
#include "BreakingItemParticle.h"
#include "SnowShovelParticle.h"
#include "BreakingItemParticle.h"
#include "HeartParticle.h"
#include "HugeExplosionParticle.h"
#include "HugeExplosionSeedParticle.h"
#include "SuspendedParticle.h"
#include "SuspendedTownParticle.h"
#include "CritParticle2.h"
#include "TerrainParticle.h"
#include "SpellParticle.h"
#include "DripParticle.h"
#include "EchantmentTableParticle.h"
#include "DragonBreathParticle.h"
#include "FireworksParticles.h"
#include "Lighting.h"
#include "Options.h"
#include "MultiPlayerChunkCache.h"
#include "..\Minecraft.World\ParticleTypes.h"
#include "..\Minecraft.World\IntCache.h"
#include "..\Minecraft.World\IntBuffer.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\System.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "MultiplayerLocalPlayer.h"
#include "MultiPlayerLevel.h"
#include "..\Minecraft.World\SoundTypes.h"
#include "FrustumCuller.h"
#include "..\Minecraft.World\BasicTypeContainers.h"

//#define DISABLE_SPU_CODE

#ifdef __PS3__
#include "PS3\SPU_Tasks\LevelRenderer_cull\LevelRenderer_cull.h"
#include "PS3\SPU_Tasks\LevelRenderer_FindNearestChunk\LevelRenderer_FindNearestChunk.h"
#include "C4JSpursJob.h"

static LevelRenderer_cull_DataIn g_cullDataIn[4] __attribute__((__aligned__(16)));
static LevelRenderer_FindNearestChunk_DataIn g_findNearestChunkDataIn __attribute__((__aligned__(16)));
#endif

ResourceLocation LevelRenderer::MOON_LOCATION = ResourceLocation(TN_TERRAIN_MOON);
ResourceLocation LevelRenderer::MOON_PHASES_LOCATION = ResourceLocation(TN_TERRAIN_MOON_PHASES);
ResourceLocation LevelRenderer::SUN_LOCATION = ResourceLocation(TN_TERRAIN_SUN);
ResourceLocation LevelRenderer::CLOUDS_LOCATION = ResourceLocation(TN_ENVIRONMENT_CLOUDS);
ResourceLocation LevelRenderer::END_SKY_LOCATION = ResourceLocation(TN_MISC_TUNNEL);

const unsigned int HALO_RING_RADIUS = 100;

#ifdef _LARGE_WORLDS
Chunk LevelRenderer::permaChunk[MAX_CONCURRENT_CHUNK_REBUILDS];
C4JThread *LevelRenderer::rebuildThreads[MAX_CHUNK_REBUILD_THREADS];
C4JThread::EventArray *LevelRenderer::s_rebuildCompleteEvents;
C4JThread::Event *LevelRenderer::s_activationEventA[MAX_CHUNK_REBUILD_THREADS];

// This defines the maximum size of renderable level, must be big enough to cope with actual size of level + view distance at each side
// so that we can render the "infinite" sea at the edges. Currently defined as:
const int overworldSize = LEVEL_MAX_WIDTH + LevelRenderer::PLAYER_VIEW_DISTANCE + LevelRenderer::PLAYER_VIEW_DISTANCE;
const int netherSize = HELL_LEVEL_MAX_WIDTH + 2; // 4J Stu - The plus 2 is really just to make our total chunk count a multiple of 8 for the flags, we will never see these in the nether
const int endSize = END_LEVEL_MAX_WIDTH;
const int LevelRenderer::MAX_LEVEL_RENDER_SIZE[3] = { overworldSize, netherSize, endSize };
const int LevelRenderer::DIMENSION_OFFSETS[3] = { 0, (overworldSize * overworldSize * CHUNK_Y_COUNT) ,  (overworldSize * overworldSize * CHUNK_Y_COUNT) + ( netherSize * netherSize * CHUNK_Y_COUNT ) };
#else
// This defines the maximum size of renderable level, must be big enough to cope with actual size of level + view distance at each side
// so that we can render the "infinite" sea at the edges. Currently defined as:
// Dimension idx 0 (overworld) : 80 ( = 54 + 13 + 13 )
// Dimension idx 1 (nether)    : 44 ( = 18 + 13 + 13 )
// Dimension idx 2 (the end)   : 44 ( = 18 + 13 + 13 )

const int LevelRenderer::MAX_LEVEL_RENDER_SIZE[3] = { 80, 44, 44 };

// Linked directly to the sizes in the previous array, these next values dictate the start offset for each dimension index into the global array for these things.
// Each dimension uses MAX_LEVEL_RENDER_SIZE[i]^2 * 8 indices, as a MAX_LEVEL_RENDER_SIZE * MAX_LEVEL_RENDER_SIZE * 8 sized cube of references.

const int LevelRenderer::DIMENSION_OFFSETS[3] = { 0, (80 * 80 * CHUNK_Y_COUNT) ,  (80 * 80 * CHUNK_Y_COUNT) + ( 44 * 44 * CHUNK_Y_COUNT ) };
#endif

LevelRenderer::LevelRenderer(Minecraft *mc, Textures *textures)
{
	breakingTextures = NULL;

	for( int i = 0; i < 4; i++ )
	{
		level[i] = NULL;
		tileRenderer[i] = NULL;
		xOld[i] = -9999;
		yOld[i] = -9999;
		zOld[i] = -9999;
	}
	xChunks= yChunks= zChunks = 0;
	chunkLists = 0;

	ticks = 0;
	starList= skyList= darkList = 0;
	xMinChunk= yMinChunk= zMinChunk = 0;
	xMaxChunk= yMaxChunk= zMaxChunk = 0;
	lastViewDistance = -1;
	noEntityRenderFrames = 2;
	totalEntities = 0;
	renderedEntities = 0;
	culledEntities = 0;
	chunkFixOffs = 0;
	frame = 0;
	repeatList = MemoryTracker::genLists(1);

	destroyProgress = 0.0f;

	totalChunks= offscreenChunks= occludedChunks= renderedChunks= emptyChunks = 0;
	for( int i = 0; i < 4; i++ )
	{
		//		sortedChunks[i] = NULL;	// 4J - removed - not sorting our chunks anymore
		chunks[i] = ClipChunkArray();
		lastPlayerCount[i] = 0;
	}

	InitializeCriticalSection(&m_csDirtyChunks);
	InitializeCriticalSection(&m_csRenderableTileEntities);
#ifdef _LARGE_WORLDS
	InitializeCriticalSection(&m_csChunkFlags);
#endif

	dirtyChunkPresent = false;
	lastDirtyChunkFound = 0;

	this->mc = mc;
	this->textures = textures;

	chunkLists = MemoryTracker::genLists(getGlobalChunkCount()*2);		// *2 here is because there is one renderlist per chunk here for each of the opaque & transparent layers
	globalChunkFlags = new unsigned char[getGlobalChunkCount()];
	memset(globalChunkFlags, 0, getGlobalChunkCount());

	starList = MemoryTracker::genLists(4);

	glPushMatrix();
	glNewList(starList, GL_COMPILE);
	renderStars();
	glEndList();

	// 4J added - create geometry for rendering clouds
	createCloudMesh();

	glPopMatrix();



	Tesselator *t = Tesselator::getInstance();
	skyList = starList + 1;
	glNewList(skyList, GL_COMPILE);
	glDepthMask(false);				// 4J - added to get depth mask disabled within the command buffer
	float yy;
	int s = 64;
	int d = 256 / s + 2;
	yy = (float) 16;
	for (int xx = -s * d; xx <= s * d; xx += s)
	{
		for (int zz = -s * d; zz <= s * d; zz += s)
		{
			t->begin();
			t->vertex((float)(xx + 0), (float)( yy), (float)( zz + 0));
			t->vertex((float)(xx + s), (float)( yy), (float)( zz + 0));
			t->vertex((float)(xx + s), (float)( yy), (float)( zz + s));
			t->vertex((float)(xx + 0), (float)( yy), (float)( zz + s));
			t->end();
		}
	}
	glEndList();

	darkList = starList + 2;
	glNewList(darkList, GL_COMPILE);
	yy = -(float) 16;
	t->begin();
	for (int xx = -s * d; xx <= s * d; xx += s)
	{
		for (int zz = -s * d; zz <= s * d; zz += s)
		{
			t->vertex((float)(xx + s), (float)( yy), (float)( zz + 0));
			t->vertex((float)(xx + 0), (float)( yy), (float)( zz + 0));
			t->vertex((float)(xx + 0), (float)( yy), (float)( zz + s));
			t->vertex((float)(xx + s), (float)( yy), (float)( zz + s));
		}
	}
	t->end();
	glEndList();

	// HALO ring for the texture pack
	{
		const unsigned int ARC_SEGMENTS = 50;
		const float VERTICAL_OFFSET = HALO_RING_RADIUS * 999/1000; // How much we raise the circle origin to make the circle curve back towards us
		const int WIDTH = 10;
		const float ARC_RADIANS = 2.0f*PI/ARC_SEGMENTS;
		const float HALF_ARC_SEG = ARC_SEGMENTS/2;
		const float WIDE_ARC_SEGS = ARC_SEGMENTS/8;
		const float WIDE_ARC_SEGS_SQR = WIDE_ARC_SEGS * WIDE_ARC_SEGS;

		float u = 0.0f;
		float width = WIDTH;

		haloRingList = starList + 3;
		glNewList(haloRingList, GL_COMPILE);
		t->begin(GL_TRIANGLE_STRIP);
		t->color(0xffffff);

		for(unsigned int i = 0; i <= ARC_SEGMENTS; ++i)
		{
			float DIFF = abs(i - HALF_ARC_SEG);
			if(DIFF<(HALF_ARC_SEG-WIDE_ARC_SEGS)) DIFF = 0;
			else DIFF-=(HALF_ARC_SEG-WIDE_ARC_SEGS);
			width = 1 + ( (DIFF * DIFF) / (WIDE_ARC_SEGS_SQR) ) * WIDTH;
			t->vertexUV((HALO_RING_RADIUS * cos(i*ARC_RADIANS)) - VERTICAL_OFFSET, (HALO_RING_RADIUS * sin(i*ARC_RADIANS)), 0-width, u, 0);
			t->vertexUV((HALO_RING_RADIUS * cos(i*ARC_RADIANS)) - VERTICAL_OFFSET, (HALO_RING_RADIUS * sin(i*ARC_RADIANS)), 0+width, u, 1);
			//--u;
			u -= 0.25;
		}
		t->end();
		glEndList();
	}

	Chunk::levelRenderer = this;

	destroyedTileManager = new DestroyedTileManager();

	dirtyChunksLockFreeStack.Initialize();
#ifdef __PS3__
	m_jobPort_CullSPU = new C4JSpursJobQueue::Port("C4JSpursJob_LevelRenderer_cull");
	m_jobPort_FindNearestChunk = new C4JSpursJobQueue::Port("C4JSpursJob_LevelRenderer_FindNearestChunk");
#endif // __PS3__
}

void LevelRenderer::renderStars()
{
	Random random = Random(10842);
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	for (int i = 0; i < 1500; i++)
	{
		double x = random.nextFloat() * 2 - 1;
		double y = random.nextFloat() * 2 - 1;
		double z = random.nextFloat() * 2 - 1;
		double ss = 0.15f + random.nextFloat() * 0.10f;
		double d = x * x + y * y + z * z;
		if (d < 1 && d > 0.01)
		{
			d = 1 / sqrt(d);
			x *= d;
			y *= d;
			z *= d;
			double xp = x * 160;	// 4J - moved further away (were 100) as they were cutting through far chunks
			double yp = y * 160;
			double zp = z * 160;

			double yRot = atan2(x, z);
			double ySin = sin(yRot);
			double yCos = cos(yRot);

			double xRot = atan2(sqrt(x * x + z * z), y);
			double xSin = sin(xRot);
			double xCos = cos(xRot);

			double zRot = random.nextDouble() * PI * 2;
			double zSin = sin(zRot);
			double zCos = cos(zRot);

			for (int c = 0; c < 4; c++)
			{
				double ___xo = 0;
				double ___yo = ((c & 2) - 1) * ss;
				double ___zo = ((c + 1 & 2) - 1) * ss;

				double __xo = ___xo;
				double __yo = ___yo * zCos - ___zo * zSin;
				double __zo = ___zo * zCos + ___yo * zSin;

				double _zo = __zo;
				double _yo = __yo * xSin + __xo * xCos;
				double _xo = __xo * xSin - __yo * xCos;

				double xo = _xo * ySin - _zo * yCos;
				double yo = _yo;
				double zo = _zo * ySin + _xo * yCos;

				t->vertex((float)(xp + xo), (float)( yp + yo), (float)( zp + zo));
			}
		}
	}
	t->end();

}


void LevelRenderer::setLevel(int playerIndex, MultiPlayerLevel *level)
{
	if (this->level[playerIndex] != NULL)
	{
		// Remove listener for this level if this is the last player referencing it
		Level *prevLevel = this->level[playerIndex];
		int refCount = 0;
		for( int i = 0; i < 4; i++ )
		{
			if( this->level[i] == prevLevel ) refCount++;
		}
		if( refCount == 1 )
		{
			this->level[playerIndex]->removeListener(this);
		}
	}

	xOld[playerIndex] = -9999;
	yOld[playerIndex] = -9999;
	zOld[playerIndex] = -9999;

	this->level[playerIndex] = level;
	if( tileRenderer[playerIndex] != NULL )
	{
		delete tileRenderer[playerIndex];
	}
	tileRenderer[playerIndex] = new TileRenderer(level);
	if (level != NULL)
	{
		// If we're the only player referencing this level, add a new listener for it
		int refCount = 0;
		for( int i = 0; i < 4; i++ )
		{
			if( this->level[i] == level ) refCount++;
		}
		if( refCount == 1 )
		{
			level->addListener(this);
		}

		allChanged(playerIndex);
	}
	else
	{
		//		printf("NULLing player %d, chunks @ 0x%x\n",playerIndex,chunks[playerIndex]);
		if( chunks[playerIndex].data != NULL )
		{
			for (unsigned int i = 0; i < chunks[playerIndex].length; i++)
			{
				chunks[playerIndex][i].chunk->_delete();
				delete chunks[playerIndex][i].chunk;
			}
			delete chunks[playerIndex].data;
			chunks[playerIndex].data = NULL;
			chunks[playerIndex].length = 0;
			//			delete sortedChunks[playerIndex];	// 4J - removed - not sorting our chunks anymore
			//			sortedChunks[playerIndex] = NULL;	// 4J - removed - not sorting our chunks anymore
		}

		// 4J Stu - If we do this for splitscreen players leaving, then all the tile entities in the world dissappear
		// We should only do this when actually exiting the game, so only when the primary player sets there level to NULL
		if(playerIndex == ProfileManager.GetPrimaryPad()) renderableTileEntities.clear();
	}
}

void LevelRenderer::AddDLCSkinsToMemTextures()
{
	for(int i=0;i<app.vSkinNames.size();i++)
	{
		textures->addMemTexture(app.vSkinNames[i], new MobSkinMemTextureProcessor());
	}
}

void LevelRenderer::allChanged()
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added
	allChanged(playerIndex);
}

int LevelRenderer::activePlayers()
{
	int playerCount = 0;
	for( int i = 0; i < 4; i++ )
	{
		if( level[i] ) playerCount++;
	}
	return playerCount;
}

void LevelRenderer::allChanged(int playerIndex)
{
	// 4J Stu - This was required by the threaded Minecraft::tick(). If we need to add it back then:
	// If this CS is entered before DisableUpdateThread is called then (on 360 at least) we can get a
	// deadlock when starting a game in splitscreen.
	//EnterCriticalSection(&m_csDirtyChunks);
	if( level == NULL )
	{
		return;
	}

	Minecraft::GetInstance()->gameRenderer->DisableUpdateThread();

	Tile::leaves->setFancy(mc->options->fancyGraphics);
	lastViewDistance = mc->options->viewDistance;

	// Calculate size of area we can render based on number of players we need to render for
	int dist = (int)sqrtf( (float)PLAYER_RENDER_AREA / (float)activePlayers() );

	// AP - poor little Vita just can't cope with such a big area
#ifdef __PSVITA__
	dist = 10;
#endif

	lastPlayerCount[playerIndex] = activePlayers();

	xChunks = dist;
	yChunks = Level::maxBuildHeight / CHUNK_SIZE;
	zChunks = dist;

	if( chunks[playerIndex].data != NULL )
	{
		for (unsigned int i = 0; i < chunks[playerIndex].length; i++)
		{
			chunks[playerIndex][i].chunk->_delete();
			delete chunks[playerIndex][i].chunk;
		}
		delete chunks[playerIndex].data;
		//		delete sortedChunks[playerIndex];	// 4J - removed - not sorting our chunks anymore
	}

	chunks[playerIndex] = ClipChunkArray(xChunks * yChunks * zChunks);
	//	sortedChunks[playerIndex] = new vector<Chunk *>(xChunks * yChunks * zChunks);		// 4J - removed - not sorting our chunks anymore
	int id = 0;
	int count = 0;

	xMinChunk = 0;
	yMinChunk = 0;
	zMinChunk = 0;
	xMaxChunk = xChunks;
	yMaxChunk = yChunks;
	zMaxChunk = zChunks;

	// 4J removed - we now only fully clear this on exiting the game (setting level to NULL). Apart from that, the chunk rebuilding is responsible for maintaining this
	//	renderableTileEntities.clear();

	for (int x = 0; x < xChunks; x++)
	{
		for (int y = 0; y < yChunks; y++)
		{
			for (int z = 0; z < zChunks; z++)
			{
				chunks[playerIndex][(z * yChunks + y) * xChunks + x].chunk = new Chunk(level[playerIndex], renderableTileEntities, m_csRenderableTileEntities, x * CHUNK_XZSIZE, y * CHUNK_SIZE, z * CHUNK_XZSIZE, &chunks[playerIndex][(z * yChunks + y) * xChunks + x]);
				chunks[playerIndex][(z * yChunks + y) * xChunks + x].visible = true;
				chunks[playerIndex][(z * yChunks + y) * xChunks + x].chunk->id = count++;
				//				sortedChunks[playerIndex]->at((z * yChunks + y) * xChunks + x) = chunks[playerIndex]->at((z * yChunks + y) * xChunks + x);	// 4J - removed - not sorting our chunks anymore

				id += 3;
			}
		}
	}
	nonStackDirtyChunksAdded();

	if (level != NULL)
	{
		shared_ptr<Entity> player = mc->cameraTargetPlayer;
		if (player != NULL)
		{
			this->resortChunks(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
			//			sort(sortedChunks[playerIndex]->begin(),sortedChunks[playerIndex]->end(), DistanceChunkSorter(player));	// 4J - removed - not sorting our chunks anymore
		}
	}

	noEntityRenderFrames = 2;

	Minecraft::GetInstance()->gameRenderer->EnableUpdateThread();

	// 4J Stu - Remove. See comment above.
	//LeaveCriticalSection(&m_csDirtyChunks);
}

void LevelRenderer::renderEntities(Vec3 *cam, Culler *culler, float a)
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added

	// 4J Stu - Set these up every time, even when not rendering as other things (like particle render) may depend on it for those frames.
	TileEntityRenderDispatcher::instance->prepare(level[playerIndex], textures, mc->font, mc->cameraTargetPlayer, a);
	EntityRenderDispatcher::instance->prepare(level[playerIndex], textures, mc->font, mc->cameraTargetPlayer, mc->crosshairPickMob, mc->options, a);

	if (noEntityRenderFrames > 0)
	{
		noEntityRenderFrames--;
		return;
	}

	totalEntities = 0;
	renderedEntities = 0;
	culledEntities = 0;

	shared_ptr<Entity> player = mc->cameraTargetPlayer;

	EntityRenderDispatcher::xOff = (player->xOld + (player->x - player->xOld) * a);
	EntityRenderDispatcher::yOff = (player->yOld + (player->y - player->yOld) * a);
	EntityRenderDispatcher::zOff = (player->zOld + (player->z - player->zOld) * a);
	TileEntityRenderDispatcher::xOff = (player->xOld + (player->x - player->xOld) * a);
	TileEntityRenderDispatcher::yOff = (player->yOld + (player->y - player->yOld) * a);
	TileEntityRenderDispatcher::zOff = (player->zOld + (player->z - player->zOld) * a);

	mc->gameRenderer->turnOnLightLayer(a);		// 4J - brought forward from 1.8.2

	vector<shared_ptr<Entity> > entities = level[playerIndex]->getAllEntities();
	totalEntities = (int)entities.size();

	for (auto& entity : level[playerIndex]->globalEntities)
	{
		renderedEntities++;
		if (entity->shouldRender(cam)) EntityRenderDispatcher::instance->render(entity, a);
	}

	for (auto& entity : entities)
	{
		bool shouldRender = (entity->shouldRender(cam) && (entity->noCulling || culler->isVisible(entity->bb)));

		// Render the mob if the mob's leash holder is within the culler
		if ( !shouldRender && entity->instanceof(eTYPE_MOB) )
		{
			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(entity);
			if ( mob->isLeashed() && (mob->getLeashHolder() != NULL) )
			{
				shared_ptr<Entity> leashHolder = mob->getLeashHolder();
				shouldRender = culler->isVisible(leashHolder->bb);
			}
		}

		if (shouldRender)
		{
			// 4J-PB - changing this to be per player
			//if (entity == mc->cameraTargetPlayer && !mc->options->thirdPersonView && !mc->cameraTargetPlayer->isSleeping()) continue;
			shared_ptr<LocalPlayer> localplayer = mc->cameraTargetPlayer->instanceof(eTYPE_LOCALPLAYER) ? dynamic_pointer_cast<LocalPlayer>(mc->cameraTargetPlayer) : nullptr;

			if (localplayer && entity == mc->cameraTargetPlayer && !localplayer->ThirdPersonView() && !mc->cameraTargetPlayer->isSleeping()) continue;

			if (!level[playerIndex]->hasChunkAt(Mth::floor(entity->x), 0, Mth::floor(entity->z)))
			{
				continue;
			}
			renderedEntities++;
			EntityRenderDispatcher::instance->render(entity, a);
		}
	}

	Lighting::turnOn();
	// 4J - have restructed this so that the tile entities are stored within a hashmap by chunk/dimension index. The index
	// is calculated in the same way as the global flags.
	EnterCriticalSection(&m_csRenderableTileEntities);
    for (auto & it : renderableTileEntities)
    {
		int idx = it.first;
		// Don't render if it isn't in the same dimension as this player
		if( !isGlobalIndexInSameDimension(idx, level[playerIndex]) ) continue;

		for( auto& it2 : it.second)
		{
			TileEntityRenderDispatcher::instance->render(it2, a);
		}
	}

	// Now consider if any of these renderable tile entities have been flagged for removal, and if so, remove
    for (auto it = renderableTileEntities.begin(); it != renderableTileEntities.end();)
    {
		int idx = it->first;

        for (auto it2 = it->second.begin(); it2 != it->second.end();)
        {
			// If it has been flagged for removal, remove
			if((*it2)->shouldRemoveForRender())
			{
				it2 = it->second.erase(it2);
			}
			else
			{
				it2++;
			}
		}

		// If there aren't any entities left for this key, then delete the key
		if( it->second.size() == 0 )
		{
			it = renderableTileEntities.erase(it);
		}
		else
		{
			it++;
		}
	}

	LeaveCriticalSection(&m_csRenderableTileEntities);

	mc->gameRenderer->turnOffLightLayer(a);		// 4J - brought forward from 1.8.2
}

wstring LevelRenderer::gatherStats1()
{
	return L"C: " + std::to_wstring(renderedChunks) + L"/" + std::to_wstring(totalChunks) + L". F: " + std::to_wstring(offscreenChunks) + L", O: " + std::to_wstring(occludedChunks) + L", E: " + std::to_wstring(emptyChunks);
}

wstring LevelRenderer::gatherStats2()
{
	return L"E: " + std::to_wstring(renderedEntities) + L"/" + std::to_wstring(totalEntities) + L". B: " + std::to_wstring(culledEntities) + L", I: " + std::to_wstring((totalEntities - culledEntities) - renderedEntities);
}

void LevelRenderer::resortChunks(int xc, int yc, int zc)
{
	EnterCriticalSection(&m_csDirtyChunks);
	xc -= CHUNK_XZSIZE / 2;
	yc -= CHUNK_SIZE / 2;
	zc -= CHUNK_XZSIZE / 2;
	xMinChunk = INT_MAX;
	yMinChunk = INT_MAX;
	zMinChunk = INT_MAX;
	xMaxChunk = INT_MIN;
	yMaxChunk = INT_MIN;
	zMaxChunk = INT_MIN;

	int playerIndex = mc->player->GetXboxPad();	// 4J added

	int s2 = xChunks * CHUNK_XZSIZE;
	int s1 = s2 / 2;

	for (int x = 0; x < xChunks; x++)
	{
		int xx = x * CHUNK_XZSIZE;

		int xOff = (xx + s1 - xc);
		if (xOff < 0) xOff -= (s2 - 1);
		xOff /= s2;
		xx -= xOff * s2;

		if (xx < xMinChunk) xMinChunk = xx;
		if (xx > xMaxChunk) xMaxChunk = xx;

		for (int z = 0; z < zChunks; z++)
		{
			int zz = z * CHUNK_XZSIZE;
			int zOff = (zz + s1 - zc);
			if (zOff < 0) zOff -= (s2 - 1);
			zOff /= s2;
			zz -= zOff * s2;

			if (zz < zMinChunk) zMinChunk = zz;
			if (zz > zMaxChunk) zMaxChunk = zz;

			for (int y = 0; y < yChunks; y++)
			{
				int yy = y * CHUNK_SIZE;
				if (yy < yMinChunk) yMinChunk = yy;
				if (yy > yMaxChunk) yMaxChunk = yy;

				Chunk *chunk = chunks[playerIndex][(z * yChunks + y) * xChunks + x].chunk;
				chunk->setPos(xx, yy, zz);
			}
		}
	}
	nonStackDirtyChunksAdded();
	LeaveCriticalSection(&m_csDirtyChunks);
}

int LevelRenderer::render(shared_ptr<LivingEntity> player, int layer, double alpha, bool updateChunks)
{
	int playerIndex = mc->player->GetXboxPad();

	// 4J - added - if the number of players has changed, we need to rebuild things for the new draw distance this will require
	if( lastPlayerCount[playerIndex] != activePlayers() )
	{
		allChanged();
	}
	else if (mc->options->viewDistance != lastViewDistance)
	{
		allChanged();
	}

	if (layer == 0)
	{
		totalChunks = 0;
		offscreenChunks = 0;
		occludedChunks = 0;
		renderedChunks = 0;
		emptyChunks = 0;
	}

	double xOff = player->xOld + (player->x - player->xOld) * alpha;
	double yOff = player->yOld + (player->y - player->yOld) * alpha;
	double zOff = player->zOld + (player->z - player->zOld) * alpha;

	double xd = player->x - xOld[playerIndex];
	double yd = player->y - yOld[playerIndex];
	double zd = player->z - zOld[playerIndex];

	if (xd * xd + yd * yd + zd * zd > 4 * 4)
	{
		xOld[playerIndex] = player->x;
		yOld[playerIndex] = player->y;
		zOld[playerIndex] = player->z;

		resortChunks(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
		//		sort(sortedChunks[playerIndex]->begin(),sortedChunks[playerIndex]->end(), DistanceChunkSorter(player));	// 4J - removed - not sorting our chunks anymore
	}
	Lighting::turnOff();

	int count = renderChunks(0, (int)chunks[playerIndex].length, layer, alpha);

	return count;

}

#ifdef __PSVITA__
#include <stdlib.h>

// this is need to sort the chunks by depth
typedef struct
{
	int Index;
	float Depth;
} SChunckSort;

int compare (const void * a, const void * b)
{
	return ( ((SChunckSort*)a)->Depth - ((SChunckSort*)b)->Depth );
}

#endif

int LevelRenderer::renderChunks(int from, int to, int layer, double alpha)
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added

#if 1
	// 4J - cut down version, we're not using offsetted render lists, or a sorted chunk list, anymore
	mc->gameRenderer->turnOnLightLayer(alpha);		// 4J - brought forward from 1.8.2
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;
	double xOff = player->xOld + (player->x - player->xOld) * alpha;
	double yOff = player->yOld + (player->y - player->yOld) * alpha;
	double zOff = player->zOld + (player->z - player->zOld) * alpha;

	glPushMatrix();
	glTranslatef((float)-xOff, (float)-yOff, (float)-zOff);

	if (layer == 1)
		glDepthMask(false);

#ifdef __PSVITA__
	// AP - also set the camera position so we can work out if a chunk is fogged or not
	RenderManager.SetCameraPosition((float)-xOff, (float)-yOff, (float)-zOff);
#endif

#if defined __PS3__ && !defined DISABLE_SPU_CODE
	// pre- calc'd on the SPU
	int count = 0;
	waitForCull_SPU();
	if(layer == 0)
	{
		count = g_cullDataIn[playerIndex].numToRender_layer0;
		RenderManager.CBuffCallMultiple(g_cullDataIn[playerIndex].listArray_layer0, count);
	}
	else // layer == 1
	{
		count = g_cullDataIn[playerIndex].numToRender_layer1;
		RenderManager.CBuffCallMultiple(g_cullDataIn[playerIndex].listArray_layer1, count);
	}

#else // __PS3__

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. First render all the non-alpha cut outs
	glDisable(GL_ALPHA_TEST);
#endif

	bool first = true;
	int count = 0;
	ClipChunk *pClipChunk = chunks[playerIndex].data;
	unsigned char emptyFlag = LevelRenderer::CHUNK_FLAG_EMPTY0 << layer;
	for( int i = 0; i < chunks[playerIndex].length; i++, pClipChunk++ )
	{
		if( !pClipChunk->visible ) continue;													// This will be set if the chunk isn't visible, or isn't compiled, or has both empty flags set
		if( pClipChunk->globalIdx == -1 ) continue;												// Not sure if we should ever encounter this... TODO check
		if( ( globalChunkFlags[pClipChunk->globalIdx] & emptyFlag ) == emptyFlag ) continue;	// Check that this particular layer isn't empty

		// List can be calculated directly from the chunk's global idex
		int list = pClipChunk->globalIdx * 2 + layer;
		list += chunkLists;

		if(RenderManager.CBuffCall(list, first))
		{
			first = false;
		}
		count++;
	}

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Now we render all the alpha cut outs
	glEnable(GL_ALPHA_TEST);
	RenderManager.StateSetForceLOD(0);	// AP - force mipmapping off for cut outs
	first = true;
	pClipChunk = chunks[playerIndex].data;
	emptyFlag = LevelRenderer::CHUNK_FLAG_EMPTY0 << layer;
	for( int i = 0; i < chunks[playerIndex].length; i++, pClipChunk++ )
	{
		if( !pClipChunk->visible ) continue;													// This will be set if the chunk isn't visible, or isn't compiled, or has both empty flags set
		if( pClipChunk->globalIdx == -1 ) continue;												// Not sure if we should ever encounter this... TODO check
		if( ( globalChunkFlags[pClipChunk->globalIdx] & emptyFlag ) == emptyFlag ) continue;	// Check that this particular layer isn't empty
		if( !(globalChunkFlags[pClipChunk->globalIdx] & LevelRenderer::CHUNK_FLAG_CUT_OUT) ) continue;	// Does this chunk contain any cut out geometry

		// List can be calculated directly from the chunk's global idex
		int list = pClipChunk->globalIdx * 2 + layer;
		list += chunkLists;

		if(RenderManager.CBuffCallCutOut(list, first))
		{
			first = false;
		}
	}
	RenderManager.StateSetForceLOD(-1);	// AP - back to normal mipmapping
#endif

#endif // __PS3__

	glPopMatrix();

	if (layer == 1)
		glDepthMask(true);

	mc->gameRenderer->turnOffLightLayer(alpha);		// 4J - brought forward from 1.8.2

#else
	_renderChunks.clear();
	// int p = 0;
	int count = 0;
	for (int i = from; i < to; i++)
	{
		if (layer == 0)
		{
			totalChunks++;
			if (sortedChunks[playerIndex]->at(i)->emptyFlagSet(layer)) emptyChunks++;
			else if (!sortedChunks[playerIndex]->at(i)->visible) offscreenChunks++;
			else renderedChunks++;
		}

		//            if (!sortedChunks[i].empty[layer] && sortedChunks[i].visible && (sortedChunks[i].occlusion_visible)) {
		if (!(sortedChunks[playerIndex]->at(i)->emptyFlagSet(layer) && sortedChunks[playerIndex]->at(i)->visible ))
		{
			int list = sortedChunks[playerIndex]->at(i)->getList(layer);
			if (list >= 0)
			{
				_renderChunks.push_back(sortedChunks[playerIndex]->at(i));
				count++;
			}
		}
	}

	shared_ptr<Mob> player = mc->cameraTargetPlayer;
	double xOff = player->xOld + (player->x - player->xOld) * alpha;
	double yOff = player->yOld + (player->y - player->yOld) * alpha;
	double zOff = player->zOld + (player->z - player->zOld) * alpha;

	int lists = 0;
	for (int l = 0; l < RENDERLISTS_LENGTH; l++)
	{
		renderLists[l].clear();
	}

	for ( Chunk *chunk : _renderChunks )
	{
		int list = -1;
		for (int l = 0; l < lists; l++)
		{
			if (renderLists[l].isAt(chunk->xRender, chunk->yRender, chunk->zRender))
			{
				list = l;
			}
		}
		if (list < 0)
		{
			list = lists++;
			renderLists[list].init(chunk->xRender, chunk->yRender, chunk->zRender, xOff, yOff, zOff);
		}

		renderLists[list].add(chunk->getList(layer));
	}

	renderSameAsLast(layer, alpha);
#endif

	return count;

}


void LevelRenderer::renderSameAsLast(int layer, double alpha)
{
	for (int i = 0; i < RENDERLISTS_LENGTH; i++)
	{
		renderLists[i].render();
	}
}

void LevelRenderer::tick()
{
	ticks++;

	if ((ticks % SharedConstants::TICKS_PER_SECOND) == 0)
	{
        auto it = destroyingBlocks.begin();
        while (it != destroyingBlocks.end())
		{
			BlockDestructionProgress *block = it->second;

			int updatedRenderTick = block->getUpdatedRenderTick();

			if (ticks - updatedRenderTick > (SharedConstants::TICKS_PER_SECOND * 20))
			{
				delete it->second;
				it = destroyingBlocks.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

void LevelRenderer::renderSky(float alpha)
{
	if (mc->level->dimension->id == 1)
	{
		glDisable(GL_FOG);
		glDisable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Lighting::turnOff();


		glDepthMask(false);
		textures->bindTexture(&END_SKY_LOCATION); // 4J was L"/1_2_2/misc/tunnel.png"
		Tesselator *t = Tesselator::getInstance();
		t->setMipmapEnable(false);
		for (int i = 0; i < 6; i++)
		{
			glPushMatrix();
			if (i == 1) glRotatef(90, 1, 0, 0);
			if (i == 2) glRotatef(-90, 1, 0, 0);
			if (i == 3) glRotatef(180, 1, 0, 0);
			if (i == 4) glRotatef(90, 0, 0, 1);
			if (i == 5) glRotatef(-90, 0, 0, 1);
			t->begin();
			t->color(0x282828);
			t->vertexUV(-100, -100, -100, 0, 0);
			t->vertexUV(-100, -100, +100, 0, 16);
			t->vertexUV(+100, -100, +100, 16, 16);
			t->vertexUV(+100, -100, -100, 16, 0);
			t->end();
			glPopMatrix();
		}
		t->setMipmapEnable(true);
		glDepthMask(true);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA_TEST);

		return;
	}

	if (!mc->level->dimension->isNaturalDimension()) return;

	glDisable(GL_TEXTURE_2D);

	int playerIndex = mc->player->GetXboxPad();
	Vec3 *sc = level[playerIndex]->getSkyColor(mc->cameraTargetPlayer, alpha);
	float sr = (float) sc->x;
	float sg = (float) sc->y;
	float sb = (float) sc->z;

	if (mc->options->anaglyph3d)
	{
		float srr = (sr * 30 + sg * 59 + sb * 11) / 100;
		float sgg = (sr * 30 + sg * 70) / (100);
		float sbb = (sr * 30 + sb * 70) / (100);

		sr = srr;
		sg = sgg;
		sb = sbb;
	}

	glColor3f(sr, sg, sb);

	Tesselator *t = Tesselator::getInstance();

	glDepthMask(false);

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita.
	glDisable(GL_ALPHA_TEST);
#endif

	glEnable(GL_FOG);
	glColor3f(sr, sg, sb);
	glCallList(skyList);

	glDisable(GL_FOG);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Lighting::turnOff();

	float *c = level[playerIndex]->dimension->getSunriseColor(level[playerIndex]->getTimeOfDay(alpha), alpha);
	if (c != NULL)
	{
		glDisable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);

		glPushMatrix();
		{
			glRotatef(90, 1, 0, 0);
			glRotatef(Mth::sin(level[playerIndex]->getSunAngle(alpha)) < 0 ? 180 : 0, 0, 0, 1);
			glRotatef(90, 0, 0, 1);

			float r = c[0];
			float g = c[1];
			float b = c[2];
			if (mc->options->anaglyph3d)
			{
				float srr = (r * 30 + g * 59 + b * 11) / 100;
				float sgg = (r * 30 + g * 70) / (100);
				float sbb = (r * 30 + b * 70) / (100);

				r = srr;
				g = sgg;
				b = sbb;
			}

			t->begin(GL_TRIANGLE_FAN);
			t->color(r, g, b, c[3]);

			t->vertex((float)(0), (float)( 100), (float)( 0));
			int steps = 16;
			t->color(c[0], c[1], c[2], 0.0f);
			for (int i = 0; i <= steps; i++)
			{
				float a = i * PI * 2 / steps;
				float _sin = Mth::sin(a);
				float _cos = Mth::cos(a);
				t->vertex((float)(_sin * 120), (float)( _cos * 120), (float)( -_cos * 40 * c[3]));
			}
			t->end();
		}
		glPopMatrix();
		glShadeModel(GL_FLAT);
	}

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPushMatrix();
	{
		float rainBrightness = 1 - level[playerIndex]->getRainLevel(alpha);
		float xp = 0;
		float yp = 0;
		float zp = 0;
		glColor4f(1, 1, 1, rainBrightness);
		glTranslatef(xp, yp, zp);
		glRotatef(-90, 0, 1, 0);
		glRotatef(level[playerIndex]->getTimeOfDay(alpha) * 360, 1, 0, 0);
		float ss = 30;

		MemSect(31);
		textures->bindTexture(&SUN_LOCATION);
		MemSect(0);
		t->begin();
		t->vertexUV((float)(-ss), (float)( 100), (float)( -ss), (float)( 0), (float)( 0));
		t->vertexUV((float)(+ss), (float)( 100), (float)( -ss), (float)( 1), (float)( 0));
		t->vertexUV((float)(+ss), (float)( 100), (float)( +ss), (float)( 1), (float)( 1));
		t->vertexUV((float)(-ss), (float)( 100), (float)( +ss), (float)( 0), (float)( 1));
		t->end();

		ss = 20;
		textures->bindTexture(&MOON_PHASES_LOCATION); // 4J was L"/1_2_2/terrain/moon_phases.png"
		int phase = level[playerIndex]->getMoonPhase();
		int u = phase % 4;
		int v = phase / 4 % 2;
		float u0 = (u + 0) / 4.0f;
		float v0 = (v + 0) / 2.0f;
		float u1 = (u + 1) / 4.0f;
		float v1 = (v + 1) / 2.0f;
		t->begin();
		t->vertexUV(-ss, -100, +ss, u1, v1);
		t->vertexUV(+ss, -100, +ss, u0, v1);
		t->vertexUV(+ss, -100, -ss, u0, v0);
		t->vertexUV(-ss, -100, -ss, u1, v0);
		t->end();

		glDisable(GL_TEXTURE_2D);
		float br = level[playerIndex]->getStarBrightness(alpha) * rainBrightness;
		if (br > 0)
		{
			glColor4f(br, br, br, br);
			glCallList(starList);
		}
		glColor4f(1, 1, 1, 1);
	}
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_FOG);

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita.
	glDisable(GL_ALPHA_TEST);
#endif

	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	glColor3f(0, 0, 0);

	double yy = mc->player->getPos(alpha)->y - level[playerIndex]->getHorizonHeight();	// 4J - getHorizonHeight moved forward from 1.2.3
	if (yy < 0)
	{
		glPushMatrix();
		glTranslatef(0, -(float) (-12), 0);
		glCallList(darkList);
		glPopMatrix();

		// 4J - can't work out what this big black box is for. Taking it out until someone misses it... it causes a big black box to visible appear in 3rd person mode whilst under the ground.
#if 0
		float ss = 1;
		float yo = -(float) (yy + 65);
		float y0 = -ss;
		float y1 = yo;


		t->begin();
		t->color(0x000000, 255);
		t->vertex(-ss, y1, ss);
		t->vertex(+ss, y1, ss);
		t->vertex(+ss, y0, ss);
		t->vertex(-ss, y0, ss);

		t->vertex(-ss, y0, -ss);
		t->vertex(+ss, y0, -ss);
		t->vertex(+ss, y1, -ss);
		t->vertex(-ss, y1, -ss);

		t->vertex(+ss, y0, -ss);
		t->vertex(+ss, y0, +ss);
		t->vertex(+ss, y1, +ss);
		t->vertex(+ss, y1, -ss);

		t->vertex(-ss, y1, -ss);
		t->vertex(-ss, y1, +ss);
		t->vertex(-ss, y0, +ss);
		t->vertex(-ss, y0, -ss);

		t->vertex(-ss, y0, -ss);
		t->vertex(-ss, y0, +ss);
		t->vertex(+ss, y0, +ss);
		t->vertex(+ss, y0, -ss);
		t->end();
#endif
	}

	if (level[playerIndex]->dimension->hasGround())
	{
		glColor3f(sr * 0.2f + 0.04f, sg * 0.2f + 0.04f, sb * 0.6f + 0.1f);
	}
	else
	{
		glColor3f(sr, sg, sb);
	}
	glPushMatrix();
	glTranslatef(0, -(float) (yy - 16), 0);
	glCallList(darkList);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);

	glDepthMask(true);
}

void LevelRenderer::renderHaloRing(float alpha)
{
#if !defined(__PS3__) && !defined(__ORBIS__) && !defined(__PSVITA__)
	if (!mc->level->dimension->isNaturalDimension()) return;

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glDepthMask(false);
	glEnable(GL_FOG);

	int playerIndex = mc->player->GetXboxPad();

	Vec3 *sc = level[playerIndex]->getSkyColor(mc->cameraTargetPlayer, alpha);
	float sr = (float) sc->x;
	float sg = (float) sc->y;
	float sb = (float) sc->z;

	// Rough lumninance calculation
	float Y = (sr+sr+sb+sg+sg+sg)/6;
	float br = 0.6f + (Y*0.4f);
	//app.DebugPrintf("Luminance = %f, brightness = %f\n", Y, br);
	glColor3f(br,br,br);

	// Fog at the base near the world
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, HALO_RING_RADIUS);
	glFogf(GL_FOG_END, HALO_RING_RADIUS * 0.20f);

	Lighting::turnOn();

	glDepthMask(false);
	textures->bindTexture(L"misc/haloRing.png"); // 4J was L"/1_2_2/misc/tunnel.png"
	Tesselator *t = Tesselator::getInstance();
	bool prev = t->setMipmapEnable(true);

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);
	glRotatef(90, 0, 1, 0);
	glCallList(haloRingList);
	glPopMatrix();
	t->setMipmapEnable(prev);

	glDepthMask(true);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA_TEST);

	glDisable(GL_FOG);
#endif
}

void LevelRenderer::renderClouds(float alpha)
{
	int iTicks=ticks;
	int playerIndex = mc->player->GetXboxPad();

	// if the primary player has clouds off, so do all players on this machine
	if(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Clouds)==0)
	{
		return;
	}

	// debug setting added to keep it at day time
	if (!mc->level->dimension->isNaturalDimension()) return;

	if (mc->options->fancyGraphics)
	{
		renderAdvancedClouds(alpha);
		return;
	}

	if(app.DebugSettingsOn())
	{
		if(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime))
		{
			iTicks=m_freezeticks;
		}
	}
	glDisable(GL_CULL_FACE);
	float yOffs = (float) (mc->cameraTargetPlayer->yOld + (mc->cameraTargetPlayer->y - mc->cameraTargetPlayer->yOld) * alpha);
	int s = 32;
	int d = 256 / s;
	Tesselator *t = Tesselator::getInstance();

	textures->bindTexture(&CLOUDS_LOCATION);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3 *cc = level[playerIndex]->getCloudColor(alpha);
	float cr = (float) cc->x;
	float cg = (float) cc->y;
	float cb = (float) cc->z;

	if (mc->options->anaglyph3d)
	{
		float crr = (cr * 30 + cg * 59 + cb * 11) / 100;
		float cgg = (cr * 30 + cg * 70) / (100);
		float cbb = (cr * 30 + cb * 70) / (100);

		cr = crr;
		cg = cgg;
		cb = cbb;
	}



	float scale = 1 / 2048.0f;

	double time = (ticks + alpha);
	double xo = mc->cameraTargetPlayer->xo + (mc->cameraTargetPlayer->x - mc->cameraTargetPlayer->xo) * alpha + time * 0.03f;
	double zo = mc->cameraTargetPlayer->zo + (mc->cameraTargetPlayer->z - mc->cameraTargetPlayer->zo) * alpha;
	int xOffs = Mth::floor(xo / 2048);
	int zOffs = Mth::floor(zo / 2048);
	xo -= xOffs * 2048;
	zo -= zOffs * 2048;

	float yy = (float) (level[playerIndex]->dimension->getCloudHeight() - yOffs + 0.33f);
	float uo = (float) (xo * scale);
	float vo = (float) (zo * scale);
	t->begin();

	t->color(cr, cg, cb, 0.8f);
	for (int xx = -s * d; xx < +s * d; xx += s)
	{
		for (int zz = -s * d; zz < +s * d; zz += s)
		{
			t->vertexUV((float)(xx + 0), (float)( yy), (float)( zz + s), (float)( (xx + 0) * scale + uo), (float)( (zz + s) * scale + vo));
			t->vertexUV((float)(xx + s), (float)( yy), (float)( zz + s), (float)( (xx + s) * scale + uo), (float)( (zz + s) * scale + vo));
			t->vertexUV((float)(xx + s), (float)( yy), (float)( zz + 0), (float)( (xx + s) * scale + uo), (float)( (zz + 0) * scale + vo));
			t->vertexUV((float)(xx + 0), (float)( yy), (float)( zz + 0), (float)( (xx + 0) * scale + uo), (float)( (zz + 0) * scale + vo));
		}
	}
	t->end();

	glColor4f(1, 1, 1, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	if(app.DebugSettingsOn())
	{

		if(!(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime)))
		{
			m_freezeticks=iTicks;
		}
	}
}

bool LevelRenderer::isInCloud(double x, double y, double z, float alpha)
{
	return false;
}

// 4J - new geometry for clouds. This is a full array of cubes, one per texel - the original is an array of intersecting fins which aren't ever going to render perfectly.
// The geometry is split into 6 command buffers, one per facing direction. This is to keep rendering similar to the original, where the geometry isn't backface culled,
// but a decision on which sides to render is made per 8x8 chunk of sky - this keeps the cloud more solid looking when you are actually inside it. Also make a 7th
// list that includes all 6 directions, to make rendering of all 6 at once more optimal (we do this when the player isn't potentially inside the clouds)
void LevelRenderer::createCloudMesh()
{
	cloudList = MemoryTracker::genLists(7);

	Tesselator *t = Tesselator::getInstance();
	const float h = 4.0f;
	const int D = 8;

	for( int i = 0; i < 7; i++ )
	{
		glNewList(cloudList + i, GL_COMPILE);

		if( ( i == 0 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(0.7f, 0.7f, 0.7f, 0.8f);
					t->normal(0, -1, 0);
					t->vertexUV(x0, y0, z0, u, v );
					t->vertexUV(x1, y0, z0, u, v );
					t->vertexUV(x1, y0, z1, u, v );
					t->vertexUV(x0, y0, z1, u, v );
				}
			}
			t->end();
		}
		if( ( i == 1 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(1.0f, 1.0f, 1.0f, 0.8f);
					t->normal(0, 1, 0);
					t->vertexUV(x0, y1, z1, u, v );
					t->vertexUV(x1, y1, z1, u, v );
					t->vertexUV(x1, y1, z0, u, v );
					t->vertexUV(x0, y1, z0, u, v );
				}
			}
			t->end();
		}
		if( ( i == 2 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(0.9f, 0.9f, 0.9f, 0.8f);
					t->normal(-1, 0, 0);
					t->vertexUV(x0, y0, z1, u, v );
					t->vertexUV(x0, y1, z1, u, v );
					t->vertexUV(x0, y1, z0, u, v );
					t->vertexUV(x0, y0, z0, u, v );
				}
			}
			t->end();
		}
		if( ( i == 3 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(0.9f, 0.9f, 0.9f, 0.8f);
					t->normal(1, 0, 0);
					t->vertexUV(x1, y0, z0, u, v );
					t->vertexUV(x1, y1, z0, u, v );
					t->vertexUV(x1, y1, z1, u, v );
					t->vertexUV(x1, y0, z1, u, v );
				}
			}
			t->end();
		}
		if( ( i == 4 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(0.8f, 0.8f, 0.8f, 0.8f);
					t->normal(-1, 0, 0);
					t->vertexUV(x0, y1, z0, u, v );
					t->vertexUV(x1, y1, z0, u, v );
					t->vertexUV(x1, y0, z0, u, v );
					t->vertexUV(x0, y0, z0, u, v );
				}
			}
			t->end();
		}
		if( ( i == 5 ) || ( i == 6 ) )
		{
			t->begin();
			for( int zt = 0; zt < D; zt++ )
			{
				for( int xt = 0; xt < D; xt++ )
				{
					float u = (((float) xt ) + 0.5f ) / 256.0f;
					float v = (((float) zt ) + 0.5f ) / 256.0f;
					float x0 = (float)xt;
					float x1 = x0 + 1.0f;
					float y0 = 0;
					float y1 = h;
					float z0 = (float)zt;
					float z1 = z0 + 1.0f;
					t->color(0.8f, 0.8f, 0.8f, 0.8f);
					t->normal(1, 0, 0);
					t->vertexUV(x0, y0, z1, u, v );
					t->vertexUV(x1, y0, z1, u, v );
					t->vertexUV(x1, y1, z1, u, v );
					t->vertexUV(x0, y1, z1, u, v );
				}
			}
			t->end();
		}
		glEndList();
	}
}

void LevelRenderer::renderAdvancedClouds(float alpha)
{
	// MGH - added, we were getting dark clouds sometimes on PS3, with this being setup incorrectly
	glMultiTexCoord2f(GL_TEXTURE1, 0, 0);


	// 4J - most of our viewports are now rendered with no clip planes but using stencilling to limit the area drawn to. Clouds have a relatively large fill area compared to
	// the number of vertices that they have, and so enabling clipping here to try and reduce fill rate cost.
	RenderManager.StateSetEnableViewportClipPlanes(true);
	float yOffs = (float) (mc->cameraTargetPlayer->yOld + (mc->cameraTargetPlayer->y - mc->cameraTargetPlayer->yOld) * alpha);
	Tesselator *t = Tesselator::getInstance();
	int playerIndex = mc->player->GetXboxPad();

	int iTicks=ticks;

	if(app.DebugSettingsOn())
	{
		if(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime))
		{
			iTicks=m_freezeticks;
		}
	}

	float ss = 12.0f;
	float h = 4.0f;

	double time = (ticks + alpha);
	double xo = (mc->cameraTargetPlayer->xo + (mc->cameraTargetPlayer->x - mc->cameraTargetPlayer->xo) * alpha + time * 0.03f) / ss;
	double zo = (mc->cameraTargetPlayer->zo + (mc->cameraTargetPlayer->z - mc->cameraTargetPlayer->zo) * alpha) / ss + 0.33f;
	float yy = (float) (level[playerIndex]->dimension->getCloudHeight() - yOffs + 0.33f);
	int xOffs = Mth::floor(xo / 2048);
	int zOffs = Mth::floor(zo / 2048);
	xo -= xOffs * 2048;
	zo -= zOffs * 2048;

	// 4J - we are now conditionally rendering the clouds in two ways
	// (1) if we are (by our y height) in the clouds, then we render in a mode quite like the original, with no backface culling, and
	// decisions on which sides of the clouds to render based on the positions of the 8x8 blocks of cloud texels
	// (2) if we aren't in the clouds, then we do a simpler form of rendering with backface culling on
	// This is because the complex sort of rendering is really there so that the clouds seem more solid when you might be in them, but it has more risk of artifacts so
	// we don't want to do it when not necessary

	bool noBFCMode = ( (yy > -h - 1) && (yy <= h + 1) );
	if( noBFCMode )
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}

	MemSect(31);
	textures->bindTexture(&CLOUDS_LOCATION);	// 4J was L"/environment/clouds.png"
	MemSect(0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3 *cc = level[playerIndex]->getCloudColor(alpha);
	float cr = (float) cc->x;
	float cg = (float) cc->y;
	float cb = (float) cc->z;

	if (mc->options->anaglyph3d)
	{
		float crr = (cr * 30 + cg * 59 + cb * 11) / 100;
		float cgg = (cr * 30 + cg * 70) / (100);
		float cbb = (cr * 30 + cb * 70) / (100);

		cr = crr;
		cg = cgg;
		cb = cbb;
	}

	float uo = (float) (xo * 0);
	float vo = (float) (zo * 0);

	float scale = 1 / 256.0f;

	uo = (float) (Mth::floor(xo)) * scale;
	vo = (float) (Mth::floor(zo)) * scale;
	// 4J - keep our UVs +ve - there's a small bug in the xbox GPU that incorrectly rounds small -ve UVs (between -1/(64*size) and 0) up to 0, which leaves gaps in our clouds...
	while( uo < 1.0f ) uo += 1.0f;
	while( vo < 1.0f ) vo += 1.0f;

	float xoffs = (float) (xo - Mth::floor(xo));
	float zoffs = (float) (zo - Mth::floor(zo));

	int D = 8;

	int radius = 3;
	if( activePlayers() > 2 ) radius = 2;	// 4J - reduce the cloud render distance a bit for 3 & 4 player split screen
	float e = 1 / 1024.0f;
	glScalef(ss, 1, ss);
	FrustumData* pFrustumData = Frustum::getFrustum();
	for (int pass = 0; pass < 2; pass++)
	{
		if (pass == 0)
		{
			// 4J - changed to use blend rather than color mask to avoid writing to frame buffer, to work with our command buffers
			glBlendFunc(GL_ZERO, GL_ONE);
			//				glColorMask(false, false, false, false);
		}
		else
		{
			// 4J - changed to use blend rather than color mask to avoid writing to frame buffer, to work with our command buffers
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//				glColorMask(true, true, true, true);
		}
		for (int xPos = -radius + 1; xPos <= radius; xPos++)
		{
			for (int zPos = -radius + 1; zPos <= radius; zPos++)
			{
				// 4J - reimplemented the clouds with full cube-per-texel geometry to get rid of seams. This is a huge amount more quads to render, so
				// now using command buffers to render each section to cut CPU hit.
#if 1
				float xx = (float)(xPos * D);
				float zz = (float)(zPos * D);
				float xp = xx - xoffs;
				float zp = zz - zoffs;

				if( !pFrustumData->cubeInFrustum(0+xp,0+yy,0+zp, 8+xp,4+yy,8+zp) )
					continue;



				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glTranslatef(xx / 256.0f + uo, zz / 256.0f + vo, 0);
				glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glTranslatef(xp,yy,zp);

				glColor4f(cr, cg, cb, 1.0f );
				if( noBFCMode )
				{
					// This is the more complex form of render the clouds, based on the way that the original code picked which sides to render, with backface culling disabled.
					// This is to give a more solid version of the clouds for when the player might be inside them.
					bool draw[6] = {false,false,false,false,false,false};

					// These rules to decide which sides to draw are the same as the original code below
					if (yy > -h - 1)	draw[0] = true;
					if (yy <= h + 1)	draw[1] = true;
					if (xPos > -1)		draw[2] = true;
					if (xPos <= 1)		draw[3] = true;
					if (zPos > -1)		draw[4] = true;
					if (zPos <= 1)		draw[5] = true;

					// Top and bottom just render when required
					if( draw[0] ) glCallList(cloudList);
					if( draw[1] ) glCallList(cloudList + 1);
					// For x facing sides, if we are actually in the clouds and about to draw both sides of the x sides too, then
					// do a little offsetting here to avoid z fighting
					if( draw[0] && draw[1] && draw[2] && draw[3] )
					{
						glTranslatef(e, 0.0f, 0.0f );
						glCallList(cloudList + 2);
						glTranslatef(-e, 0.0f, 0.0f );
						glCallList(cloudList + 3);
					}
					else
					{
						if( draw[2] ) glCallList(cloudList + 2);
						if( draw[3] ) glCallList(cloudList + 3);
					}
					// For z facing sides, if we are actually in the clouds and about to draw both sides of the z sides too, then
					// do a little offsetting here to avoid z fighting
					if( draw[0] && draw[1] && draw[4] && draw[5] )
					{
						glTranslatef(0.0f, 0.0f, e );
						glCallList(cloudList + 4);
						glTranslatef(0.0f, 0.0f, -e );
						glCallList(cloudList + 5);
					}
					else
					{
						if( draw[4] ) glCallList(cloudList + 4);
						if( draw[5] ) glCallList(cloudList + 5);
					}
				}
				else
				{
					// Simpler form of rendering that we can do most of the time, when we aren't potentially inside a cloud
					glCallList(cloudList + 6);
				}
				glPopMatrix();
				glMatrixMode(GL_TEXTURE);
				glLoadIdentity();
				glMatrixMode(GL_MODELVIEW);
#else

				t->begin();
				float xx = (float)(xPos * D);
				float zz = (float)(zPos * D);
				float xp = xx - xoffs;
				float zp = zz - zoffs;


				if (yy > -h - 1)
				{
					t->color(cr * 0.7f, cg * 0.7f, cb * 0.7f, 0.8f);
					t->normal(0, -1, 0);
					t->vertexUV((float)(xp + 0), (float)( yy + 0), (float)( zp + D), (float)( (xx + 0) * scale + uo), (float)( (zz + D) * scale + vo));
					t->vertexUV((float)(xp + D), (float)( yy + 0), (float)( zp + D), (float)( (xx + D) * scale + uo), (float)( (zz + D) * scale + vo));
					t->vertexUV((float)(xp + D), (float)( yy + 0), (float)( zp + 0), (float)( (xx + D) * scale + uo), (float)( (zz + 0) * scale + vo));
					t->vertexUV((float)(xp + 0), (float)( yy + 0), (float)( zp + 0), (float)( (xx + 0) * scale + uo), (float)( (zz + 0) * scale + vo));
				}

				if (yy <= h + 1)
				{
					t->color(cr, cg, cb, 0.8f);
					t->normal(0, 1, 0);
					t->vertexUV((float)(xp + 0), (float)( yy + h - e), (float)( zp + D), (float)( (xx + 0) * scale + uo), (float)( (zz + D) * scale + vo));
					t->vertexUV((float)(xp + D), (float)( yy + h - e), (float)( zp + D), (float)( (xx + D) * scale + uo), (float)( (zz + D) * scale + vo));
					t->vertexUV((float)(xp + D), (float)( yy + h - e), (float)( zp + 0), (float)( (xx + D) * scale + uo), (float)( (zz + 0) * scale + vo));
					t->vertexUV((float)(xp + 0), (float)( yy + h - e), (float)( zp + 0), (float)( (xx + 0) * scale + uo), (float)( (zz + 0) * scale + vo));
				}

				t->color(cr * 0.9f, cg * 0.9f, cb * 0.9f, 0.8f);
				if (xPos > -1)
				{
					t->normal(-1, 0, 0);
					for (int i = 0; i < D; i++)
					{
						t->vertexUV((float)(xp + i + 0), (float)( yy + 0), (float)( zp + D), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + D) * scale + vo));
						t->vertexUV((float)(xp + i + 0), (float)( yy + h), (float)( zp + D), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + D) * scale + vo));
						t->vertexUV((float)(xp + i + 0), (float)( yy + h), (float)( zp + 0), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + 0) * scale + vo));
						t->vertexUV((float)(xp + i + 0), (float)( yy + 0), (float)( zp + 0), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + 0) * scale + vo));
					}
				}

				if (xPos <= 1)
				{
					t->normal(+1, 0, 0);
					for (int i = 0; i < D; i++)
					{
						t->vertexUV((float)(xp + i + 1 - e), (float)( yy + 0), (float)( zp + D), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + D) * scale + vo));
						t->vertexUV((float)(xp + i + 1 - e), (float)( yy + h), (float)( zp + D), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + D) * scale + vo));
						t->vertexUV((float)(xp + i + 1 - e), (float)( yy + h), (float)( zp + 0), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + 0) * scale + vo));
						t->vertexUV((float)(xp + i + 1 - e), (float)( yy + 0), (float)( zp + 0), (float)( (xx + i + 0.5f) * scale + uo), (float)( (zz + 0) * scale + vo));
					}
				}

				t->color(cr * 0.8f, cg * 0.8f, cb * 0.8f, 0.8f);
				if (zPos > -1)
				{
					t->normal(0, 0, -1);
					for (int i = 0; i < D; i++)
					{
						t->vertexUV((float)(xp + 0), (float)( yy + h), (float)( zp + i + 0), (float)( (xx + 0) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + D), (float)( yy + h), (float)( zp + i + 0), (float)( (xx + D) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + D), (float)( yy + 0), (float)( zp + i + 0), (float)( (xx + D) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + 0), (float)( yy + 0), (float)( zp + i + 0), (float)( (xx + 0) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
					}
				}

				if (zPos <= 1)
				{
					t->normal(0, 0, 1);
					for (int i = 0; i < D; i++)
					{
						t->vertexUV((float)(xp + 0), (float)( yy + h), (float)( zp + i + 1 - e), (float)( (xx + 0) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + D), (float)( yy + h), (float)( zp + i + 1 - e), (float)( (xx + D) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + D), (float)( yy + 0), (float)( zp + i + 1 - e), (float)( (xx + D) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
						t->vertexUV((float)(xp + 0), (float)( yy + 0), (float)( zp + i + 1 - e), (float)( (xx + 0) * scale + uo), (float)( (zz + i + 0.5f) * scale + vo));
					}
				}
				t->end();
#endif
			}
		}
	}

	glColor4f(1, 1, 1, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);


	if(app.DebugSettingsOn())
	{
		if(!(app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_FreezeTime)))
		{
			m_freezeticks=iTicks;
		}
	}
	RenderManager.StateSetEnableViewportClipPlanes(false);
}


bool LevelRenderer::updateDirtyChunks()
{
#ifdef _LARGE_WORLDS
	std::list< std::pair<ClipChunk *, int> > nearestClipChunks;
#endif

	ClipChunk *nearChunk = NULL;			// Nearest chunk that is dirty
	int veryNearCount = 0;
	int minDistSq = 0x7fffffff;		// Distances to this chunk


	// Set a flag if we should only rebuild existing chunks, not create anything new
	unsigned int memAlloc = RenderManager.CBuffSize(-1);
	/*
	static int throttle = 0;
	if( ( throttle % 100 ) == 0 )
	{
	app.DebugPrintf("CBuffSize: %d\n",memAlloc/(1024*1024));
	}
	throttle++;
	*/
	PIXAddNamedCounter(((float)memAlloc)/(1024.0f*1024.0f),"Command buffer allocations");
	bool onlyRebuild = ( memAlloc >= MAX_COMMANDBUFFER_ALLOCATIONS );
	EnterCriticalSection(&m_csDirtyChunks);

	// Move any dirty chunks stored in the lock free stack into global flags
	int index = 0;

	do
	{
		// See comment on dirtyChunksLockFreeStack.Push() regarding details of this casting/subtracting -2.
		index = (size_t)dirtyChunksLockFreeStack.Pop();
#ifdef _CRITICAL_CHUNKS
		int oldIndex = index;
		index &= 0x0fffffff;							// remove the top bit that marked the chunk as non-critical
#endif
		if( index == 1 ) dirtyChunkPresent = true;		// 1 is a special value passed to let this thread know that a chunk which isn't on this stack has been set to dirty
		else if( index > 1 )
		{
			int i2 = index - 2;
			if( i2 >= DIMENSION_OFFSETS[2] )
			{
				i2 -= DIMENSION_OFFSETS[2];
				int y2 = i2 & (CHUNK_Y_COUNT-1);
				i2 /= CHUNK_Y_COUNT;
				int z2 = i2 / MAX_LEVEL_RENDER_SIZE[2];
				int x2 = i2 - z2 * MAX_LEVEL_RENDER_SIZE[2];
				x2 -= MAX_LEVEL_RENDER_SIZE[2] / 2;
				z2 -= MAX_LEVEL_RENDER_SIZE[2] / 2;
			}
			setGlobalChunkFlag(index - 2, CHUNK_FLAG_DIRTY);

#ifdef _CRITICAL_CHUNKS
			if( !(oldIndex & 0x10000000) )					// was this chunk not marked as non-critical. Ugh double negatives
			{
				setGlobalChunkFlag(index - 2, CHUNK_FLAG_CRITICAL);
			}
#endif

			dirtyChunkPresent = true;
		}
	} while( index );

	// Only bother searching round all the chunks if we have some dirty chunk(s)
	if( dirtyChunkPresent )
	{
		lastDirtyChunkFound = System::currentTimeMillis();
		PIXBeginNamedEvent(0,"Finding nearest chunk\n");
#if defined __PS3__ && !defined DISABLE_SPU_CODE
		// find the nearest chunk with a spu task, copy all the data over here for uploading to SPU
		g_findNearestChunkDataIn.numGlobalChunks = getGlobalChunkCount();
		g_findNearestChunkDataIn.pGlobalChunkFlags = globalChunkFlags;
		g_findNearestChunkDataIn.onlyRebuild = onlyRebuild;
		g_findNearestChunkDataIn.lowerOffset = (int)&((LevelChunk*)0)->lowerBlocks;		// dodgy bit of class structure poking, as we don't want to try and get the whole of LevelChunk copmpiling on SPU
		g_findNearestChunkDataIn.upperOffset = (int)&((LevelChunk*)0)->upperBlocks;
		g_findNearestChunkDataIn.xChunks = xChunks;
		g_findNearestChunkDataIn.yChunks = yChunks;
		g_findNearestChunkDataIn.zChunks = zChunks;

		for(int i=0;i<4;i++)
		{
			g_findNearestChunkDataIn.chunks[i] = (LevelRenderer_FindNearestChunk_DataIn::ClipChunk*)chunks[i].data;
			g_findNearestChunkDataIn.chunkLengths[i] = chunks[i].length;
			g_findNearestChunkDataIn.level[i] = level[i];
			g_findNearestChunkDataIn.playerData[i].bValid = mc->localplayers[i] != NULL;
			if(mc->localplayers[i] != NULL)
			{
				g_findNearestChunkDataIn.playerData[i].x = mc->localplayers[i]->x;
				g_findNearestChunkDataIn.playerData[i].y = mc->localplayers[i]->y;
				g_findNearestChunkDataIn.playerData[i].z = mc->localplayers[i]->z;

			}
			if(level[i] != NULL)
			{
				g_findNearestChunkDataIn.multiplayerChunkCache[i].XZOFFSET = ((MultiPlayerChunkCache*)(level[i]->chunkSource))->XZOFFSET;
				g_findNearestChunkDataIn.multiplayerChunkCache[i].XZSIZE = ((MultiPlayerChunkCache*)(level[i]->chunkSource))->XZSIZE;
				g_findNearestChunkDataIn.multiplayerChunkCache[i].cache = (void**)((MultiPlayerChunkCache*)(level[i]->chunkSource))->cache;
			}

		}

		// 		assert(sizeof(LevelRenderer_FindNearestChunk_DataIn::Chunk) == sizeof(Chunk));
		C4JSpursJob_LevelRenderer_FindNearestChunk findJob(&g_findNearestChunkDataIn);
		m_jobPort_FindNearestChunk->submitJob(&findJob);
		m_jobPort_FindNearestChunk->waitForCompletion();
		nearChunk = (ClipChunk*)g_findNearestChunkDataIn.nearChunk;
		veryNearCount = g_findNearestChunkDataIn.veryNearCount;
#else // __PS3__

#ifdef _LARGE_WORLDS
		int maxNearestChunks = MAX_CONCURRENT_CHUNK_REBUILDS;
		// 4J Stu - On XboxOne we should cut this down if in a constrained state so the saving threads get more time
#endif
		// Find nearest chunk that is dirty
		for( int p = 0; p < XUSER_MAX_COUNT; p++ )
		{
			// It's possible that the localplayers member can be set to NULL on the main thread when a player chooses to exit the game
			// So take a reference to the player object now. As it is a shared_ptr it should live as long as we need it
			shared_ptr<LocalPlayer> player = mc->localplayers[p];
			if( player == NULL ) continue;
			if( chunks[p].data == NULL ) continue;
			if( level[p] == NULL ) continue;
			if( chunks[p].length != xChunks * zChunks * CHUNK_Y_COUNT ) continue;
			int px = (int)player->x;
			int py = (int)player->y;
			int pz = (int)player->z;

			//			app.DebugPrintf("!! %d %d %d, %d %d %d {%d,%d} ",px,py,pz,stackChunkDirty,nonStackChunkDirty,onlyRebuild, xChunks, zChunks);

			int considered = 0;
			int wouldBeNearButEmpty = 0;
			for( int x = 0; x < xChunks; x++ )
			{
				for( int z = 0; z < zChunks; z++ )
				{
					for( int y = 0; y < CHUNK_Y_COUNT; y++ )
					{
						ClipChunk *pClipChunk = &chunks[p][(z * yChunks + y) * xChunks + x];
						// Get distance to this chunk - deliberately not calling the chunk's method of doing this to avoid overheads (passing entitie, type conversion etc.) that this involves
						int xd = pClipChunk->xm - px;
						int yd = pClipChunk->ym - py;
						int zd = pClipChunk->zm - pz;
						int distSq = xd * xd + yd * yd + zd * zd;
						int distSqWeighted = xd * xd + yd * yd * 4 + zd * zd;  // Weighting against y to prioritise things in same x/z plane as player first

						if( globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_DIRTY )
						{
							if( (!onlyRebuild) ||
								globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_COMPILED ||
								( distSq < 96 * 96 ) )	// Always rebuild really near things or else building (say) at tower up into empty blocks when we are low on memory will not create render data
							{	// distSq adjusted from 20 * 20 to 96 * 96 - updated by detectiveren
								considered++;
								// Is this chunk nearer than our nearest?
#ifdef _LARGE_WORLDS
								bool isNearer = nearestClipChunks.empty();
                                auto itNearest = nearestClipChunks.begin();
                                for(; itNearest != nearestClipChunks.end(); ++itNearest)
								{
									isNearer = distSqWeighted < itNearest->second;
									if(isNearer) break;
								}
								isNearer = isNearer || (nearestClipChunks.size() < maxNearestChunks);
#else
								bool isNearer = distSqWeighted < minDistSq;
#endif

#ifdef _CRITICAL_CHUNKS
								// AP - this will make sure that if a deferred grouping has started, only critical chunks go into that
								// grouping, even if a non-critical chunk is closer.
								if( (!veryNearCount && isNearer) ||
									(distSq < 20 * 20 && (globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_CRITICAL)) )
#else
								if( isNearer )
#endif
								{
									// At this point we've got a chunk that we would like to consider for rendering, at least based on its proximity to the player(s).
									// Its *quite* quick to generate empty render data for render chunks, but if we let the rebuilding do that then the after rebuilding we will have
									// to start searching for the next nearest chunk from scratch again. Instead, its better to detect empty chunks at this stage, flag them up as not dirty
									// (and empty), and carry on. The levelchunk's isRenderChunkEmpty method can be quite optimal as it can make use of the chunk's data compression to detect
									// emptiness without actually testing as many data items as uncompressed data would.
									Chunk *chunk = pClipChunk->chunk;
									LevelChunk *lc = level[p]->getChunkAt(chunk->x,chunk->z);
									if( !lc->isRenderChunkEmpty(y * 16) )
									{
										nearChunk = pClipChunk;
										minDistSq = distSqWeighted;
#ifdef _LARGE_WORLDS
										nearestClipChunks.insert(itNearest, std::make_pair(nearChunk, minDistSq) );
										if(nearestClipChunks.size() > maxNearestChunks)
										{
											nearestClipChunks.pop_back();
										}
#endif
									}
									else
									{
										chunk->clearDirty();
										globalChunkFlags[ pClipChunk->globalIdx ] |= CHUNK_FLAG_EMPTYBOTH;
										wouldBeNearButEmpty++;
									}
								}

#ifdef _CRITICAL_CHUNKS
								// AP - is the chunk near and also critical
								if( distSq < 20 * 20 && ((globalChunkFlags[ pClipChunk->globalIdx ] & CHUNK_FLAG_CRITICAL)) )
#else
								if( distSq < 20 * 20 )
#endif
								{
									veryNearCount++;
								}
							}
						}
					}
				}
			}
			//			app.DebugPrintf("[%d,%d,%d]\n",nearestClipChunks.empty(),considered,wouldBeNearButEmpty);
		}
#endif // __PS3__
		PIXEndNamedEvent();
	}



	Chunk *chunk = NULL;
#ifdef _LARGE_WORLDS
	if(!nearestClipChunks.empty())
	{
		int index = 0;
		for(auto & it : nearestClipChunks)
		{
			chunk = it.first->chunk;
			// If this chunk is very near, then move the renderer into a deferred mode. This won't commit any command buffers
			// for rendering until we call CBuffDeferredModeEnd(), allowing us to group any near changes into an atomic unit. This
			// is essential so we don't temporarily create any holes in the environment whilst updating one chunk and not the neighbours.
			// The "ver near" aspect of this is just a cosmetic nicety - exactly the same thing would happen further away, but we just don't
			// care about it so much from terms of visual impact.
			if( veryNearCount > 0 )
			{
				RenderManager.CBuffDeferredModeStart();
			}
			// Build this chunk & return false to continue processing
			chunk->clearDirty();
			// Take a copy of the details that are required for chunk rebuilding, and rebuild That instead of the original chunk data. This is done within
			// the m_csDirtyChunks critical section, which means that any chunks can't be repositioned whilst we are doing this copy. The copy will then
			// be guaranteed to be consistent whilst rebuilding takes place outside of that critical section.
			permaChunk[index].makeCopyForRebuild(chunk);
			++index;
		}
		LeaveCriticalSection(&m_csDirtyChunks);

		--index; // Bring it back into 0 counted range

		for(int i = MAX_CHUNK_REBUILD_THREADS - 1; i >= 0; --i)
		{
			// Set the events that won't run
			if( (i+1) > index) s_rebuildCompleteEvents->Set(i);
			else break;
		}

		for(; index >=0; --index)
		{
			bool bAtomic = false;
			if((veryNearCount > 0))
				bAtomic = true;		//MGH -  if veryNearCount, then we're trying to rebuild atomically, so do it all on the main thread

			if( bAtomic || (index == 0) )
			{
				//PIXBeginNamedEvent(0,"Rebuilding near chunk %d %d %d",chunk->x, chunk->y, chunk->z);
				//		static int64_t totalTime = 0;
				//		static int64_t countTime = 0;
				//		int64_t startTime = System::currentTimeMillis();

				//app.DebugPrintf("Rebuilding permaChunk %d\n", index);

				permaChunk[index].rebuild();

				if(index !=0)
					s_rebuildCompleteEvents->Set(index-1);		// MGH - this rebuild happening on the main thread instead, mark the thread it should have been running on as complete

				//		int64_t endTime = System::currentTimeMillis();
				//		totalTime += (endTime - startTime);
				//		countTime++;
				//		printf("%d : %f\n", countTime, (float)totalTime / (float)countTime);
				//PIXEndNamedEvent();
			}
			// 4J Stu - Ignore this path when in constrained mode on Xbox One
			else
			{
				// Activate thread to rebuild this chunk
				s_activationEventA[index - 1]->Set();
			}
		}

		// Wait for the other threads to be done as well
		s_rebuildCompleteEvents->WaitForAll(INFINITE);
	}
#else
	if( nearChunk )
	{
		chunk = nearChunk->chunk;
		PIXBeginNamedEvent(0,"Rebuilding near chunk %d %d %d",chunk->x, chunk->y, chunk->z);
		// If this chunk is very near, then move the renderer into a deferred mode. This won't commit any command buffers
		// for rendering until we call CBuffDeferredModeEnd(), allowing us to group any near changes into an atomic unit. This
		// is essential so we don't temporarily create any holes in the environment whilst updating one chunk and not the neighbours.
		// The "ver near" aspect of this is just a cosmetic nicety - exactly the same thing would happen further away, but we just don't
		// care about it so much from terms of visual impact.
		if( veryNearCount > 0 )
		{
			RenderManager.CBuffDeferredModeStart();
		}
		// Build this chunk & return false to continue processing
		chunk->clearDirty();
		// Take a copy of the details that are required for chunk rebuilding, and rebuild That instead of the original chunk data. This is done within
		// the m_csDirtyChunks critical section, which means that any chunks can't be repositioned whilst we are doing this copy. The copy will then
		// be guaranteed to be consistent whilst rebuilding takes place outside of that critical section.
		static Chunk permaChunk;
		permaChunk.makeCopyForRebuild(chunk);
		LeaveCriticalSection(&m_csDirtyChunks);
		//		static int64_t totalTime = 0;
		//		static int64_t countTime = 0;
		//		int64_t startTime = System::currentTimeMillis();
		permaChunk.rebuild();
		//		int64_t endTime = System::currentTimeMillis();
		//		totalTime += (endTime - startTime);
		//		countTime++;
		//		printf("%d : %f\n", countTime, (float)totalTime / (float)countTime);
		PIXEndNamedEvent();
	}
#endif
	else
	{
		// Nothing to do - clear flags that there are things to process, unless it's been a while since we found any dirty chunks in which case force a check next time through
		if( ( System::currentTimeMillis() - lastDirtyChunkFound ) > FORCE_DIRTY_CHUNK_CHECK_PERIOD_MS )
		{
			dirtyChunkPresent = true;
		}
		else
		{
			dirtyChunkPresent = false;
		}
		LeaveCriticalSection(&m_csDirtyChunks);
#ifdef __PS3__
		Sleep(5);
#endif // __PS3__
		return false;
	}

	// If there was more than one very near thing found in our initial assessment, then return true so that we will keep doing the other one(s)
	// in an atomic unit
	if( veryNearCount > 1 )
	{
		destroyedTileManager->updatedChunkAt(chunk->level, chunk->x, chunk->y, chunk->z, veryNearCount );
		return true;
	}
	// If the chunk we've just built was near, and it has been marked dirty at some point while we are rebuilding, also return true so
	// we can rebuild the same thing atomically - if its data was changed during creating render data, it may well be invalid
	if( ( veryNearCount == 1 ) && getGlobalChunkFlag(chunk->x, chunk->y, chunk->z, chunk->level, CHUNK_FLAG_DIRTY ) )
	{
		destroyedTileManager->updatedChunkAt(chunk->level, chunk->x, chunk->y, chunk->z, veryNearCount + 1);
		return true;
	}

	if( nearChunk )	destroyedTileManager->updatedChunkAt(chunk->level, chunk->x, chunk->y, chunk->z, veryNearCount );

	return false;
}

void LevelRenderer::renderHit(shared_ptr<Player> player, HitResult *h, int mode, shared_ptr<ItemInstance> inventoryItem, float a)
{
	Tesselator *t = Tesselator::getInstance();
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor4f(1, 1, 1, ((float) (Mth::sin(Minecraft::currentTimeMillis() / 100.0f)) * 0.2f + 0.4f) * 0.5f);
	if (mode != 0 && inventoryItem != NULL)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float br = (Mth::sin(Minecraft::currentTimeMillis() / 100.0f) * 0.2f + 0.8f);
		glColor4f(br, br, br, (Mth::sin(Minecraft::currentTimeMillis() / 200.0f) * 0.2f + 0.5f));

		textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	}
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}

void LevelRenderer::renderDestroyAnimation(Tesselator *t, shared_ptr<Player> player, float a)
{
	double xo = player->xOld + (player->x - player->xOld) * a;
	double yo = player->yOld + (player->y - player->yOld) * a;
	double zo = player->zOld + (player->z - player->zOld) * a;

	int playerIndex = mc->player->GetXboxPad();
	if (!destroyingBlocks.empty())
	{
		glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

		textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
		glColor4f(1, 1, 1, 0.5f);
		glPushMatrix();

		glDisable(GL_ALPHA_TEST);

		glPolygonOffset(-3.0f, -3.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);

		glEnable(GL_ALPHA_TEST);
		t->begin();
#ifdef __PSVITA__
		// AP : fix for bug 4952. No amount of polygon offset will push this close enough to be seen above the second tile layer when looking straight down
		// so just add on a little bit of y to fix this. hacky hacky
		t->offset((float)-xo, (float)-yo + 0.01f,(float) -zo);
#else
		t->offset((float)-xo, (float)-yo,(float) -zo);
#endif
		t->noColor();

        auto it = destroyingBlocks.begin();
        while (it != destroyingBlocks.end())
		{
			BlockDestructionProgress *block = it->second;
			double xd = block->getX() - xo;
			double yd = block->getY() - yo;
			double zd = block->getZ() - zo;

			if (xd * xd + yd * yd + zd * zd < 32 * 32) // 4J MGH - now only culling instead of removing, as the list is shared in split screen
			{
				int iPad = mc->player->GetXboxPad();	// 4J added
				int tileId = level[iPad]->getTile(block->getX(), block->getY(), block->getZ());
				Tile *tile = tileId > 0 ? Tile::tiles[tileId] : NULL;
				if (tile == NULL) tile = Tile::stone;
				tileRenderer[iPad]->tesselateInWorldFixedTexture(tile, block->getX(), block->getY(), block->getZ(), breakingTextures[block->getProgress()]);	// 4J renamed to differentiate from tesselateInWorld
			}
			++it;
		}

		t->end();
		t->offset(0, 0, 0);
		glDisable(GL_ALPHA_TEST);
		/*
		* for (int i = 0; i < 6; i++) { tile.renderFace(t, h.x, h.y,
		* h.z, i, 15 * 16 + (int) (destroyProgress * 10)); }
		*/
		glPolygonOffset(0.0f, 0.0f);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_ALPHA_TEST);

		glDepthMask(true);
		glPopMatrix();
	}
}

void LevelRenderer::renderHitOutline(shared_ptr<Player> player, HitResult *h, int mode, float a)
{

	if (mode == 0 && h->type == HitResult::TILE)
	{
		int iPad = mc->player->GetXboxPad();	// 4J added

		// 4J-PB - If Display HUD is false, don't render the hit outline
		if ( app.GetGameSettings(iPad,eGameSetting_DisplayHUD)==0 ) return;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0, 0, 0, 0.4f);
		glLineWidth(2.0f);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(false);
		float ss = 0.002f;
		int tileId = level[iPad]->getTile(h->x, h->y, h->z);

		if (tileId > 0)
		{
			Tile::tiles[tileId]->updateShape(level[iPad], h->x, h->y, h->z);
			double xo = player->xOld + (player->x - player->xOld) * a;
			double yo = player->yOld + (player->y - player->yOld) * a;
			double zo = player->zOld + (player->z - player->zOld) * a;
			render(Tile::tiles[tileId]->getTileAABB(level[iPad], h->x, h->y, h->z)->grow(ss, ss, ss)->cloneMove(-xo, -yo, -zo));
		}
		glDepthMask(true);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

void LevelRenderer::render(AABB *b)
{
	Tesselator *t = Tesselator::getInstance();

	t->begin(GL_LINE_STRIP);
	t->vertex((float)(b->x0), (float)( b->y0), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y0), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y0), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y0), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y0), (float)( b->z0));
	t->end();

	t->begin(GL_LINE_STRIP);
	t->vertex((float)(b->x0), (float)( b->y1), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y1), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y1), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y1), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y1), (float)( b->z0));
	t->end();

	t->begin(GL_LINES);
	t->vertex((float)(b->x0), (float)( b->y0), (float)( b->z0));
	t->vertex((float)(b->x0), (float)( b->y1), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y0), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y1), (float)( b->z0));
	t->vertex((float)(b->x1), (float)( b->y0), (float)( b->z1));
	t->vertex((float)(b->x1), (float)( b->y1), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y0), (float)( b->z1));
	t->vertex((float)(b->x0), (float)( b->y1), (float)( b->z1));
	t->end();
}

void LevelRenderer::setDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level)	// 4J - added level param
{
	// 4J - level is passed if this is coming from setTilesDirty, which could come from when connection is being ticked outside of normal level tick, and player won't
	// be set up
	if( level == NULL ) level = this->level[mc->player->GetXboxPad()];
	//	EnterCriticalSection(&m_csDirtyChunks);
	int _x0 = Mth::intFloorDiv(x0, CHUNK_XZSIZE);
	int _y0 = Mth::intFloorDiv(y0, CHUNK_SIZE);
	int _z0 = Mth::intFloorDiv(z0, CHUNK_XZSIZE);
	int _x1 = Mth::intFloorDiv(x1, CHUNK_XZSIZE);
	int _y1 = Mth::intFloorDiv(y1, CHUNK_SIZE);
	int _z1 = Mth::intFloorDiv(z1, CHUNK_XZSIZE);

	for (int x = _x0; x <= _x1; x++)
	{
		for (int y = _y0; y <= _y1; y++)
		{
			for (int z = _z0; z <= _z1; z++)
			{
				//				printf("Setting %d %d %d dirty\n",x,y,z);
				int index = getGlobalIndexForChunk(x * 16, y * 16, z * 16, level);
				// Rather than setting the flags directly, add any dirty chunks into a lock free stack - this avoids having to lock m_csDirtyChunks .
				// These chunks are then added to the global flags in the render update thread.
				// An XLockFreeQueue actually implements a queue of pointers to its templated type, and I don't want to have to go allocating ints here just to store the
				// pointer to them in a queue. Hence actually pretending that the int Is a pointer here. Our Index has a a valid range from 0 to something quite big,
				// but including zero. The lock free queue, since it thinks it is dealing with pointers, uses a NULL pointer to signify that a Pop hasn't succeeded.
				// We also want to reserve one special value (of 1 ) for use when multiple chunks not individually listed are made dirty. Therefore adding 2 to our
				// index value here to move our valid range from 1 to something quite big + 2
				if( index > -1 )
				{
#ifdef _CRITICAL_CHUNKS
					index += 2;

					// AP - by the time we reach this function the area passed in has a 1 block border added to it to make sure geometry and lighting is updated correctly.
					// Some of those blocks will only need lighting updated so it is acceptable to not have those blocks grouped in the deferral system as the mismatch
					// will hardly be noticable. The blocks that need geometry updated will be adjacent to the original, non-bordered area.
					// This bit of code will mark a chunk as 'non-critical' if all of the blocks inside it are NOT adjacent to the original area. This has the greatest effect
					// when digging a single block. Only 6 of the blocks out of the possible 26 are actually adjacent to the original block. The other 20 only need lighting updated.
					// Note I have noticed a new side effect of this system where it's possible to see into the sides of water but this is acceptable compared to seeing through
					// the entire landscape.
					// is the left or right most block just inside this chunk
					if( ((x0 & 15) == 15 && x == _x0) || ((x1 & 15) == 0 && x == _x1) )
					{
						// is the front, back, top or bottom most block just inside this chunk
						if( ((z0 & 15) == 15 && z == _z0) || ((z1 & 15) == 0 && z == _z1) ||
							((y0 & 15) == 15 && y == _y0) || ((y1 & 15) == 0 && y == _y1))
						{
							index |= 0x10000000;
						}
					}
					else
					{
						// is the front or back most block just inside this chunk
						if( ((z0 & 15) == 15 && z == _z0) || ((z1 & 15) == 0 && z == _z1) )
						{
							// is the top or bottom most block just inside this chunk
							if( ((y0 & 15) == 15 && y == _y0) || ((y1 & 15) == 0 && y == _y1))
							{
								index |= 0x10000000;
							}
						}
					}

					dirtyChunksLockFreeStack.Push((int *)(index));
#else
					dirtyChunksLockFreeStack.Push((int *)(index + 2));
#endif

#ifdef _XBOX
					PIXSetMarker(0,"Setting chunk %d %d %d dirty",x * 16,y * 16,z * 16);
#else
					PIXSetMarkerDeprecated(0,"Setting chunk %d %d %d dirty",x * 16,y * 16,z * 16);
#endif
				}
				//				setGlobalChunkFlag(x * 16, y * 16, z * 16, level, CHUNK_FLAG_DIRTY);
			}
		}
	}
	//	LeaveCriticalSection(&m_csDirtyChunks);
}

void LevelRenderer::tileChanged(int x, int y, int z)
{
	setDirty(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1, NULL);
}

void LevelRenderer::tileLightChanged(int x, int y, int z)
{
	setDirty(x - 1, y - 1, z - 1, x + 1, y + 1, z + 1, NULL);
}

void LevelRenderer::setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level)	// 4J - added level param
{
	setDirty(x0 - 1, y0 - 1, z0 - 1, x1 + 1, y1 + 1, z1 + 1, level);
}

bool inline clip(float *bb, float *frustum)
{
	for (int i = 0; i < 6; ++i, frustum += 4)
	{
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[1]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[1]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[4]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[4]) + frustum[2] * (bb[2]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[1]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[1]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[0]) + frustum[1] * (bb[4]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;
		if (frustum[0] * (bb[3]) + frustum[1] * (bb[4]) + frustum[2] * (bb[5]) + frustum[3] > 0) continue;

		return false;
	}

	return true;
}

#ifdef __PS3__
int g_listArray_layer0[4][LevelRenderer_cull_DataIn::sc_listSize]__attribute__((__aligned__(16)));	// 8000
int g_listArray_layer1[4][LevelRenderer_cull_DataIn::sc_listSize]__attribute__((__aligned__(16)));
float g_zDepth_layer0[4][LevelRenderer_cull_DataIn::sc_listSize]__attribute__((__aligned__(16)));	// 8000
float g_zDepth_layer1[4][LevelRenderer_cull_DataIn::sc_listSize]__attribute__((__aligned__(16)));

volatile bool g_useIdent = false;
volatile float g_maxDepthRender = 1000;
volatile float g_maxHeightRender = -1000;
volatile float g_offMulVal = 1;

void LevelRenderer::cull_SPU(int playerIndex, Culler *culler, float a)
{
	if(m_bSPUCullStarted[playerIndex])
	{
		return; // running already
	}

	FrustumCuller *fc = (FrustumCuller *)culler;
	FrustumData *fd = fc->frustum;
	float fdraw[6 * 4];
	for( int i = 0; i < 6; i++ )
	{
		double fx = fd->m_Frustum[i][0];
		double fy = fd->m_Frustum[i][1];
		double fz = fd->m_Frustum[i][2];
		fdraw[i * 4 + 0] = (float)fx;
		fdraw[i * 4 + 1] = (float)fy;
		fdraw[i * 4 + 2] = (float)fz;
		fdraw[i * 4 + 3] = (float)(fd->m_Frustum[i][3] + ( fx * -fc->xOff ) + ( fy * - fc->yOff ) + ( fz * -fc->zOff ));
	}

	memcpy(&g_cullDataIn[playerIndex].fdraw, fdraw, sizeof(fdraw));
	g_cullDataIn[playerIndex].numClipChunks = chunks[playerIndex].length;
	g_cullDataIn[playerIndex].pClipChunks = (ClipChunk_SPU*)chunks[playerIndex].data;
	g_cullDataIn[playerIndex].numGlobalChunks = getGlobalChunkCount();
	g_cullDataIn[playerIndex].pGlobalChunkFlags = globalChunkFlags;
	g_cullDataIn[playerIndex].chunkLists = chunkLists;
	g_cullDataIn[playerIndex].listArray_layer0 = g_listArray_layer0[playerIndex];
	g_cullDataIn[playerIndex].listArray_layer1 = g_listArray_layer1[playerIndex];
	g_cullDataIn[playerIndex].zDepth_layer0 = g_zDepth_layer0[playerIndex];
	g_cullDataIn[playerIndex].zDepth_layer1 = g_zDepth_layer1[playerIndex];
	g_cullDataIn[playerIndex].maxDepthRender = g_maxDepthRender;
	g_cullDataIn[playerIndex].maxHeightRender = g_maxHeightRender;

	if(g_useIdent)
		g_cullDataIn[playerIndex].clipMat = Vectormath::Aos::Matrix4::identity();
	else
	{
		memcpy(&g_cullDataIn[playerIndex].clipMat, &fc->frustum->modl[0], sizeof(float) * 16);
		g_cullDataIn[playerIndex].clipMat[3][0] = -fc->xOff;
		g_cullDataIn[playerIndex].clipMat[3][1] = -fc->yOff;
		g_cullDataIn[playerIndex].clipMat[3][2] = -fc->zOff;
	}


	C4JSpursJob_LevelRenderer_cull cullJob(&g_cullDataIn[playerIndex]);
	C4JSpursJob_LevelRenderer_zSort sortJob(&g_cullDataIn[playerIndex]);

	m_jobPort_CullSPU->submitJob(&cullJob);
	m_jobPort_CullSPU->submitSync();
	// 	static int doSort = false;
	// 	if(doSort)
	{
		m_jobPort_CullSPU->submitJob(&sortJob);
	}
	// 	doSort ^= 1;
	m_bSPUCullStarted[playerIndex] = true;
}
void LevelRenderer::waitForCull_SPU()
{
	m_jobPort_CullSPU->waitForCompletion();
	int playerIndex = mc->player->GetXboxPad();	// 4J added
	m_bSPUCullStarted[playerIndex] = false;
}
#endif // __PS3__

void LevelRenderer::cull(Culler *culler, float a)
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added

#if defined __PS3__ && !defined DISABLE_SPU_CODE
	cull_SPU(playerIndex, culler, a);
	return;
#endif // __PS3__


	FrustumCuller *fc = (FrustumCuller *)culler;
	FrustumData *fd = fc->frustum;
	float fdraw[6 * 4];
	for( int i = 0; i < 6; i++ )
	{
		double fx = fd->m_Frustum[i][0];
		double fy = fd->m_Frustum[i][1];
		double fz = fd->m_Frustum[i][2];
		fdraw[i * 4 + 0] = (float)fx;
		fdraw[i * 4 + 1] = (float)fy;
		fdraw[i * 4 + 2] = (float)fz;
		fdraw[i * 4 + 3] = (float)(fd->m_Frustum[i][3] + ( fx * -fc->xOff ) + ( fy * - fc->yOff ) + ( fz * -fc->zOff ));
	}

	ClipChunk *pClipChunk = chunks[playerIndex].data;
	int vis = 0;
	int total = 0;
	int numWrong = 0;
	for (unsigned int i = 0; i < chunks[playerIndex].length; i++)
	{
		unsigned char flags = pClipChunk->globalIdx == -1 ? 0 : globalChunkFlags[ pClipChunk->globalIdx ];

		// Always perform frustum cull test
		bool clipres = clip(pClipChunk->aabb, fdraw);

		if ( (flags & CHUNK_FLAG_COMPILED ) && ( ( flags & CHUNK_FLAG_EMPTYBOTH ) != CHUNK_FLAG_EMPTYBOTH ) )
		{
			pClipChunk->visible = clipres;
			if( pClipChunk->visible ) vis++;
			total++;
		}
		else if (clipres)
		{
			pClipChunk->visible = true;
		}
		else
		{
			pClipChunk->visible = false;
		}
		pClipChunk++;
	}
}


void LevelRenderer::playStreamingMusic(const wstring& name, int x, int y, int z)
{
	if (name != L"")
	{
		mc->gui->setNowPlaying(L"C418 - " + name);
	}
	mc->soundEngine->playStreaming(name, (float) x, (float) y, (float) z, 1, 1);
}

void LevelRenderer::playSound(int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist)
{
	// 4J-PB - removed in 1.4

	//float dd = 16;
	/*if (volume > 1) fSoundClipDist *= volume;

	// 4J - find min distance to any players rather than just the current one
	float minDistSq = FLT_MAX;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
	if( mc->localplayers[i] )
	{
	float distSq = mc->localplayers[i]->distanceToSqr(x, y, z );
	if( distSq < minDistSq )
	{
	minDistSq = distSq;
	}
	}
	}

	if (minDistSq < fSoundClipDist * fSoundClipDist)
	{
	mc->soundEngine->play(iSound, (float) x, (float) y, (float) z, volume, pitch);
	}	*/
}

void LevelRenderer::playSound(shared_ptr<Entity> entity,int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist)
{
}

void LevelRenderer::playSoundExceptPlayer(shared_ptr<Player> player, int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist)
{
}

// 4J-PB - original function. I've changed to an enum instead of string compares
// 4J removed -
/*
void LevelRenderer::addParticle(const wstring& name, double x, double y, double z, double xa, double ya, double za)
{
if (mc == NULL || mc->cameraTargetPlayer == NULL || mc->particleEngine == NULL) return;

double xd = mc->cameraTargetPlayer->x - x;
double yd = mc->cameraTargetPlayer->y - y;
double zd = mc->cameraTargetPlayer->z - z;

double particleDistance = 16;
if (xd * xd + yd * yd + zd * zd > particleDistance * particleDistance) return;

int playerIndex = mc->player->GetXboxPad();	// 4J added

if (name== L"bubble") mc->particleEngine->add(shared_ptr<BubbleParticle>( new BubbleParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"smoke") mc->particleEngine->add(shared_ptr<SmokeParticle>( new SmokeParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"note") mc->particleEngine->add(shared_ptr<NoteParticle>( new NoteParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"portal") mc->particleEngine->add(shared_ptr<PortalParticle>( new PortalParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"explode") mc->particleEngine->add(shared_ptr<ExplodeParticle>( new ExplodeParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"flame") mc->particleEngine->add(shared_ptr<FlameParticle>( new FlameParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"lava") mc->particleEngine->add(shared_ptr<LavaParticle>( new LavaParticle(level[playerIndex], x, y, z) ) );
else if (name== L"footstep") mc->particleEngine->add(shared_ptr<FootstepParticle>( new FootstepParticle(textures, level[playerIndex], x, y, z) ) );
else if (name== L"splash") mc->particleEngine->add(shared_ptr<SplashParticle>( new SplashParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"largesmoke") mc->particleEngine->add(shared_ptr<SmokeParticle>( new SmokeParticle(level[playerIndex], x, y, z, xa, ya, za, 2.5f) ) );
else if (name== L"reddust") mc->particleEngine->add(shared_ptr<RedDustParticle>( new RedDustParticle(level[playerIndex], x, y, z, (float) xa, (float) ya, (float) za) ) );
else if (name== L"snowballpoof") mc->particleEngine->add(shared_ptr<BreakingItemParticle>( new BreakingItemParticle(level[playerIndex], x, y, z, Item::snowBall) ) );
else if (name== L"snowshovel") mc->particleEngine->add(shared_ptr<SnowShovelParticle>( new SnowShovelParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
else if (name== L"slime") mc->particleEngine->add(shared_ptr<BreakingItemParticle>( new BreakingItemParticle(level[playerIndex], x, y, z, Item::slimeBall)) ) ;
else if (name== L"heart") mc->particleEngine->add(shared_ptr<HeartParticle>( new HeartParticle(level[playerIndex], x, y, z, xa, ya, za) ) );
}
*/

void LevelRenderer::addParticle(ePARTICLE_TYPE eParticleType, double x, double y, double z, double xa, double ya, double za)
{
	addParticleInternal( eParticleType, x, y, z, xa, ya, za );
}

shared_ptr<Particle> LevelRenderer::addParticleInternal(ePARTICLE_TYPE eParticleType, double x, double y, double z, double xa, double ya, double za)
{
	if (mc == NULL || mc->cameraTargetPlayer == NULL || mc->particleEngine == NULL)
	{
		return nullptr;
	}

	// 4J added - do some explicit checking for NaN. The normal depth clipping seems to generally work for NaN (ie they get rejected), except on optimised PS3 code which
	// reverses the logic on the comparison with particleDistanceSquared and gets the opposite result to what you might expect.
	if( Double::isNaN(x) ) return nullptr;
	if( Double::isNaN(y) ) return nullptr;
	if( Double::isNaN(z) ) return nullptr;

	int particleLevel = mc->options->particles;

	Level *lev;
	int playerIndex = mc->player->GetXboxPad();	// 4J added
	lev = level[playerIndex];

	if (particleLevel == 1)
	{
		// when playing at "decreased" particle level, randomly filter
		// particles by setting the level to "minimal"
		if (level[playerIndex]->random->nextInt(3) == 0)
		{
			particleLevel = 2;
		}
	}

	// 4J - the java code doesn't distance cull these two particle types, we need to implement this behaviour differently as our distance check is
	// mixed up with other things
	bool distCull = true;
	if ( (eParticleType == eParticleType_hugeexplosion) || (eParticleType == eParticleType_largeexplode) || (eParticleType == eParticleType_dragonbreath) )
	{
		distCull = false;
	}

	// 4J - this is a bit of hack to get communication through from the level itself, but if Minecraft::animateTickLevel is NULL then
	// we are to behave as normal, and if it is set, then we should use that as a pointer to the level the particle is to be created with
	// rather than try to work it out from the current player. This is because in this state we are calling from a loop that is trying
	// to amalgamate particle creation between all players for a particular level. Also don't do distance clipping as it isn't for a particular
	// player, and distance is already taken into account before we get here anyway by the code in Level::animateTickDoWork
	if( mc->animateTickLevel == NULL )
	{
		double particleDistanceSquared = 16 * 16;
		double xd = 0.0f;
		double yd = 0.0f;
		double zd = 0.0f;

		// 4J Stu - Changed this as we need to check all local players in case one of them is in range of this particle
		// Fix for #13454 - art : note blocks do not show notes
		bool inRange = false;
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			shared_ptr<Player> thisPlayer = mc->localplayers[i];
			if(thisPlayer != NULL && level[i] == lev)
			{
				xd = thisPlayer->x - x;
				yd = thisPlayer->y - y;
				zd = thisPlayer->z - z;
				if (xd * xd + yd * yd + zd * zd <= particleDistanceSquared) inRange = true;
			}
		}
		if( (!inRange) && distCull ) return nullptr;
	}
	else
	{
		lev =  mc->animateTickLevel;
	}

	if (particleLevel > 1)
	{
		// TODO: If any of the particles below are necessary even if
		// particles are turned off, then modify this if statement
		return nullptr;
	}

	shared_ptr<Particle> particle;

	switch(eParticleType)
	{
	case eParticleType_hugeexplosion:
		particle = shared_ptr<Particle>(new HugeExplosionSeedParticle(lev, x, y, z, xa, ya, za));
		break;
	case eParticleType_largeexplode:
		particle = shared_ptr<Particle>(new HugeExplosionParticle(textures, lev, x, y, z, xa, ya, za));
		break;
	case eParticleType_fireworksspark:
		particle = shared_ptr<Particle>(new FireworksParticles::FireworksSparkParticle(lev, x, y, z, xa, ya, za, mc->particleEngine));
		particle->setAlpha(0.99f);
		break;

	case eParticleType_bubble:
		particle = shared_ptr<Particle>( new BubbleParticle(lev, x, y, z, xa, ya, za) );
		break;

	case eParticleType_suspended:
		particle = shared_ptr<Particle>( new SuspendedParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_depthsuspend:
		particle = shared_ptr<Particle>( new SuspendedTownParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_townaura:
		particle = shared_ptr<Particle>( new SuspendedTownParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_crit:
		{
			shared_ptr<CritParticle2> critParticle2 = shared_ptr<CritParticle2>(new CritParticle2(lev, x, y, z, xa, ya, za));
			critParticle2->CritParticle2PostConstructor();
			particle = shared_ptr<Particle>( critParticle2 );
			// request from 343 to set pink for the needler in the Halo Texture Pack
			// Set particle colour from colour-table.
			unsigned int cStart = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_CritStart );
			unsigned int cEnd = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_CritEnd );

			// If the start and end colours are the same, just set that colour, otherwise random between them
			if(cStart==cEnd)
			{
				critParticle2->SetAgeUniformly();
				particle->setColor( ( (cStart>>16)&0xFF )/255.0f, ( (cStart>>8)&0xFF )/255.0, ( cStart&0xFF )/255.0 );
			}
			else
			{
				float fStart=((float)(cStart&0xFF));
				float fDiff=(float)((cEnd-cStart)&0xFF);

				float fCol =  (fStart + (Math::random() * fDiff))/255.0f;
				particle->setColor( fCol, fCol, fCol );
			}
		}
		break;
	case eParticleType_magicCrit:
		{
			shared_ptr<CritParticle2> critParticle2 = shared_ptr<CritParticle2>(new CritParticle2(lev, x, y, z, xa, ya, za));
			critParticle2->CritParticle2PostConstructor();
			particle = shared_ptr<Particle>(critParticle2);
			particle->setColor(particle->getRedCol() * 0.3f, particle->getGreenCol() * 0.8f, particle->getBlueCol());
			particle->setNextMiscAnimTex();
		}
		break;
	case eParticleType_smoke:
		particle = shared_ptr<Particle>( new SmokeParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_endportal: // 4J - Added.
		{
			SmokeParticle *tmp = new SmokeParticle(lev, x, y, z, xa, ya, za);

			// 4J-JEV: Set particle colour from colour-table.
			unsigned int col = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Particle_EnderPortal );
			tmp->setColor( ( (col>>16)&0xFF )/255.0f, ( (col>>8)&0xFF )/255.0, ( col&0xFF )/255.0 );

			particle = shared_ptr<Particle>(tmp);
		}
		break;
	case eParticleType_mobSpell:
		particle = shared_ptr<Particle>(new SpellParticle(lev, x, y, z, 0, 0, 0));
		particle->setColor((float) xa, (float) ya, (float) za);
		break;
	case eParticleType_mobSpellAmbient:
		particle = shared_ptr<SpellParticle>(new SpellParticle(lev, x, y, z, 0, 0, 0));
		particle->setAlpha(0.15f);
		particle->setColor((float) xa, (float) ya, (float) za);
		break;
	case eParticleType_spell:
		particle = shared_ptr<Particle>( new SpellParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_witchMagic:
		{
			particle = shared_ptr<SpellParticle>(new SpellParticle(lev, x, y, z, xa, ya, za));
			dynamic_pointer_cast<SpellParticle>(particle)->setBaseTex(9 * 16);
			float randBrightness = lev->random->nextFloat() * 0.5f + 0.35f;
			particle->setColor(1 * randBrightness, 0 * randBrightness, 1 * randBrightness);
		}
		break;
	case eParticleType_instantSpell:
		particle = shared_ptr<Particle>(new SpellParticle(lev, x, y, z, xa, ya, za));
		dynamic_pointer_cast<SpellParticle>(particle)->setBaseTex(9 * 16);
		break;
	case eParticleType_note:
		particle = shared_ptr<Particle>( new NoteParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_netherportal:
		particle = shared_ptr<Particle>( new NetherPortalParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_ender:
		particle = shared_ptr<Particle>( new EnderParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_enchantmenttable:
		particle = shared_ptr<Particle>(new EchantmentTableParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_explode:
		particle = shared_ptr<Particle>( new ExplodeParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_flame:
		particle = shared_ptr<Particle>( new FlameParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_lava:
		particle = shared_ptr<Particle>( new LavaParticle(lev, x, y, z) );
		break;
	case eParticleType_footstep:
		particle = shared_ptr<Particle>( new FootstepParticle(textures, lev, x, y, z) );
		break;
	case eParticleType_splash:
		particle = shared_ptr<Particle>( new SplashParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_largesmoke:
		particle = shared_ptr<Particle>( new SmokeParticle(lev, x, y, z, xa, ya, za, 2.5f) );
		break;
	case eParticleType_reddust:
		particle = shared_ptr<Particle>( new RedDustParticle(lev, x, y, z, (float) xa, (float) ya, (float) za) );
		break;
	case eParticleType_snowballpoof:
		particle = shared_ptr<Particle>( new BreakingItemParticle(lev, x, y, z, Item::snowBall, textures) );
		break;
	case eParticleType_dripWater:
		particle = shared_ptr<Particle>( new DripParticle(lev, x, y, z, Material::water) );
		break;
	case eParticleType_dripLava:
		particle = shared_ptr<Particle>( new DripParticle(lev, x, y, z, Material::lava) );
		break;
	case eParticleType_snowshovel:
		particle = shared_ptr<Particle>( new SnowShovelParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_slime:
		particle = shared_ptr<Particle>( new BreakingItemParticle(lev, x, y, z, Item::slimeBall, textures));
		break;
	case eParticleType_heart:
		particle = shared_ptr<Particle>( new HeartParticle(lev, x, y, z, xa, ya, za) );
		break;
	case eParticleType_angryVillager:
		particle = shared_ptr<Particle>( new HeartParticle(lev, x, y + 0.5f, z, xa, ya, za) );
		particle->setMiscTex(1 + 16 * 5);
		particle->setColor(1, 1, 1);
		break;
	case eParticleType_happyVillager:
		particle = shared_ptr<Particle>( new SuspendedTownParticle(lev, x, y, z, xa, ya, za) );
		particle->setMiscTex(2 + 16 * 5);
		particle->setColor(1, 1, 1);
		break;
	case eParticleType_dragonbreath:
		particle = shared_ptr<Particle>( new DragonBreathParticle(lev, x, y, z, xa, ya, za) );
		break;
	default:
		if( ( eParticleType >= eParticleType_iconcrack_base ) &&  ( eParticleType <= eParticleType_iconcrack_last )  )
		{
			int id = PARTICLE_CRACK_ID(eParticleType), data = PARTICLE_CRACK_DATA(eParticleType);
			particle = shared_ptr<Particle>(new BreakingItemParticle(lev, x, y, z, xa, ya, za, Item::items[id], textures, data));
		}
		else if( ( eParticleType >= eParticleType_tilecrack_base ) &&  ( eParticleType <= eParticleType_tilecrack_last )  )
		{
			int id = PARTICLE_CRACK_ID(eParticleType), data = PARTICLE_CRACK_DATA(eParticleType);
			particle = dynamic_pointer_cast<Particle>( shared_ptr<TerrainParticle>(new TerrainParticle(lev, x, y, z, xa, ya, za, Tile::tiles[id], 0, data, textures))->init(data) );
		}
	}

	if (particle != NULL)
	{
		mc->particleEngine->add(particle);
	}

	return particle;
}

void LevelRenderer::entityAdded(shared_ptr<Entity> entity)
{
	if(entity->instanceof(eTYPE_PLAYER))
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(entity);
		player->prepareCustomTextures();

		// 4J-PB - adding these from global title storage
		if (player->customTextureUrl != L"")
		{
			textures->addMemTexture(player->customTextureUrl, new MobSkinMemTextureProcessor());
		}
		if (player->customTextureUrl2 != L"")
		{
			textures->addMemTexture(player->customTextureUrl2, new MobSkinMemTextureProcessor());
		}
	}
}

void LevelRenderer::entityRemoved(shared_ptr<Entity> entity)
{
	if(entity->instanceof(eTYPE_PLAYER))
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(entity);
		if (player->customTextureUrl != L"")
		{
			textures->removeMemTexture(player->customTextureUrl);
		}
		if (player->customTextureUrl2 != L"")
		{
			textures->removeMemTexture(player->customTextureUrl2);
		}
	}
}

void LevelRenderer::skyColorChanged()
{
	// 4J - no longer used
#if 0
	EnterCriticalSection(&m_csDirtyChunks);
	for( int i = 0; i < getGlobalChunkCountForOverworld(); i++ )
	{
		if( ( globalChunkFlags[i] & CHUNK_FLAG_NOTSKYLIT ) == 0 )
		{
			globalChunkFlags[i] |= CHUNK_FLAG_DIRTY;
		}
	}
	LeaveCriticalSection(&m_csDirtyChunks);
#endif
}

void LevelRenderer::clear()
{
	MemoryTracker::releaseLists(chunkLists);
}

void LevelRenderer::globalLevelEvent(int type, int sourceX, int sourceY, int sourceZ, int data)
{
	Level *lev;
	int playerIndex = mc->player->GetXboxPad();	// 4J added
	lev = level[playerIndex];

	Random *random = lev->random;

	switch (type)
	{
	case LevelEvent::SOUND_WITHER_BOSS_SPAWN:
	case LevelEvent::SOUND_DRAGON_DEATH:
		if (mc->cameraTargetPlayer != NULL)
		{
			// play the sound at an offset from the player
			double dx = sourceX - mc->cameraTargetPlayer->x;
			double dy = sourceY - mc->cameraTargetPlayer->y;
			double dz = sourceZ - mc->cameraTargetPlayer->z;

			double len = sqrt(dx * dx + dy * dy + dz * dz);
			double sx = mc->cameraTargetPlayer->x;
			double sy = mc->cameraTargetPlayer->y;
			double sz = mc->cameraTargetPlayer->z;

			if (len > 0)
			{
				sx += dx / len * 2;
				sy += dy / len * 2;
				sz += dz / len * 2;
			}
			if (type == LevelEvent::SOUND_WITHER_BOSS_SPAWN)
			{
				lev->playLocalSound(sx, sy, sz, eSoundType_MOB_WITHER_SPAWN, 1.0f, 1.0f, false);
			}
			else if (type == LevelEvent::SOUND_DRAGON_DEATH)
			{
				lev->playLocalSound(sx, sy, sz, eSoundType_MOB_ENDERDRAGON_END, 5.0f, 1.0f, false);
			}
		}
		break;
	}
}

void LevelRenderer::levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data)
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added
	Random *random = level[playerIndex]->random;
	switch (type)
	{
		//case LevelEvent::SOUND_WITHER_BOSS_SPAWN:
	case LevelEvent::SOUND_DRAGON_DEATH:
		if (mc->cameraTargetPlayer != NULL)
		{
			// play the sound at an offset from the player
			double dx = x - mc->cameraTargetPlayer->x;
			double dy = y - mc->cameraTargetPlayer->y;
			double dz = z - mc->cameraTargetPlayer->z;

			double len = sqrt(dx * dx + dy * dy + dz * dz);
			double sx = mc->cameraTargetPlayer->x;
			double sy = mc->cameraTargetPlayer->y;
			double sz = mc->cameraTargetPlayer->z;

			if (len > 0)
			{
				sx += (dx / len) * 2;
				sy += (dy / len) * 2;
				sz += (dz / len) * 2;
			}

			level[playerIndex]->playLocalSound(sx, sy, sz, eSoundType_MOB_ENDERDRAGON_END, 5.0f, 1.0f);
		}
		break;
	case LevelEvent::SOUND_CLICK_FAIL:
		//level[playerIndex]->playSound(x, y, z, L"random.click", 1.0f, 1.2f);
		level[playerIndex]->playLocalSound(x, y, z, eSoundType_RANDOM_CLICK, 1.0f, 1.2f, false);
		break;
	case LevelEvent::SOUND_CLICK:
		level[playerIndex]->playLocalSound(x, y, z, eSoundType_RANDOM_CLICK, 1.0f, 1.0f, false);
		break;
	case LevelEvent::SOUND_LAUNCH:
		level[playerIndex]->playLocalSound(x, y, z, eSoundType_RANDOM_BOW, 1.0f, 1.2f, false);
		break;
	case LevelEvent::PARTICLES_SHOOT:
		{
			int xd = (data % 3) - 1;
			int zd = (data / 3 % 3) - 1;
			double xp = x + xd * 0.6 + 0.5;
			double yp = y + 0.5;
			double zp = z + zd * 0.6 + 0.5;
			for (int i = 0; i < 10; i++)
			{
				double pow = random->nextDouble() * 0.2 + 0.01;
				double xs = xp + xd * 0.01 + (random->nextDouble() - 0.5) * zd * 0.5;
				double ys = yp + (random->nextDouble() - 0.5) * 0.5;
				double zs = zp + zd * 0.01 + (random->nextDouble() - 0.5) * xd * 0.5;
				double xsa = xd * pow + random->nextGaussian() * 0.01;
				double ysa = -0.03 + random->nextGaussian() * 0.01;
				double zsa = zd * pow + random->nextGaussian() * 0.01;
				addParticle(eParticleType_smoke, xs, ys, zs, xsa, ysa, zsa);
			}
			break;
		}
	case LevelEvent::PARTICLES_EYE_OF_ENDER_DEATH:
		{
			double xp = x + 0.5;
			double yp = y;
			double zp = z + 0.5;

			ePARTICLE_TYPE particle = PARTICLE_ICONCRACK(Item::eyeOfEnder->id,0);
			for (int i = 0; i < 8; i++)
			{
				addParticle(particle, xp, yp, zp, random->nextGaussian() * 0.15, random->nextDouble() * 0.2, random->nextGaussian() * .15);
			}
			for (double a = 0; a < PI * 2.0; a += PI * 0.05)
			{
				addParticle(eParticleType_ender, xp + cos(a) * 5, yp - .4, zp + sin(a) * 5, cos(a) * -5, 0, sin(a) * -5);
				addParticle(eParticleType_ender, xp + cos(a) * 5, yp - .4, zp + sin(a) * 5, cos(a) * -7, 0, sin(a) * -7);
			}

		}
		break;
	case LevelEvent::PARTICLES_POTION_SPLASH:
		{
			double xp = x;
			double yp = y;
			double zp = z;

			ePARTICLE_TYPE particle = PARTICLE_ICONCRACK(Item::potion->id, data);
			for (int i = 0; i < 8; i++)
			{
				addParticle(particle, xp, yp, zp, random->nextGaussian() * 0.15, random->nextDouble() * 0.2, random->nextGaussian() * 0.15);
			}


			int colorValue = Item::potion->getColor(data);

			float red = (float) ((colorValue >> 16) & 0xff) / 255.0f;
			float green = (float) ((colorValue >> 8) & 0xff) / 255.0f;
			float blue = (float) ((colorValue >> 0) & 0xff) / 255.0f;

			ePARTICLE_TYPE particleName = eParticleType_spell;
			if (Item::potion->hasInstantenousEffects(data))
			{
				particleName = eParticleType_instantSpell;
			}

			for (int i = 0; i < 100; i++)
			{
				double dist = random->nextDouble() * ThrownPotion::SPLASH_RANGE;
				double angle = random->nextDouble() * PI * 2;
				double xs = cos(angle) * dist;
				double ys = 0.01 + random->nextDouble() * 0.5;
				double zs = sin(angle) * dist;

				shared_ptr<Particle> spellParticle = addParticleInternal(particleName, xp + xs * 0.1, yp + 0.3, zp + zs * 0.1, xs, ys, zs);
				if (spellParticle != NULL)
				{
					float randBrightness = 0.75f + random->nextFloat() * 0.25f;
					spellParticle->setColor(red * randBrightness, green * randBrightness, blue * randBrightness);
					spellParticle->setPower((float) dist);
				}
			}
			level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_GLASS, 1, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		}
		break;
	case LevelEvent::ENDERDRAGON_FIREBALL_SPLASH:
		{
			double xp = x;
			double yp = y;
			double zp = z;

			ePARTICLE_TYPE particleName = eParticleType_dragonbreath;

			for (int i = 0; i < 200; i++)
			{
				double dist = random->nextDouble() * DragonFireball::SPLASH_RANGE;
				double angle = random->nextDouble() * PI * 2;
				double xs = cos(angle) * dist;
				double ys = 0.01 + random->nextDouble() * 0.5;
				double zs = sin(angle) * dist;

				shared_ptr<Particle> acidParticle = addParticleInternal(particleName, xp + xs * 0.1, yp + 0.3, zp + zs * 0.1, xs, ys, zs);
				if (acidParticle != NULL)
				{
					float randBrightness = 0.75f + random->nextFloat() * 0.25f;
					acidParticle->setPower((float) dist);
				}
			}
			level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_EXPLODE, 1, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f);
		}
		break;
	case LevelEvent::PARTICLES_DESTROY_BLOCK:
		{
			int t = data & Tile::TILE_NUM_MASK;
			if (t > 0)
			{
				Tile *oldTile = Tile::tiles[t];
				mc->soundEngine->play(oldTile->soundType->getBreakSound(), x + 0.5f, y + 0.5f, z + 0.5f, (oldTile->soundType->getVolume() + 1) / 2, oldTile->soundType->getPitch() * 0.8f);
			}

			mc->particleEngine->destroy(x, y, z, data & Tile::TILE_NUM_MASK, (data >> Tile::TILE_NUM_SHIFT) & 0xff);
			break;
		}
	case LevelEvent::PARTICLES_MOBTILE_SPAWN:
		{
			for (int i = 0; i < 20; i++)
			{

				double xP = x + 0.5 + (level[playerIndex]->random->nextFloat() - 0.5) * 2;
				double yP = y + 0.5 + (level[playerIndex]->random->nextFloat() - 0.5) * 2;
				double zP = z + 0.5 + (level[playerIndex]->random->nextFloat() - 0.5) * 2;

				level[playerIndex]->addParticle(eParticleType_smoke, xP, yP, zP, 0, 0, 0);
				level[playerIndex]->addParticle(eParticleType_flame, xP, yP, zP, 0, 0, 0);
			}
			break;
		}
	case LevelEvent::PARTICLES_PLANT_GROWTH:
        DyePowderItem::addGrowthParticles(level[playerIndex], x, y, z, data);
        break;
	case LevelEvent::SOUND_OPEN_DOOR:
		if (Math::random() < 0.5)
		{
			level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_DOOR_OPEN, 1.0f, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		} else {
			level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_RANDOM_DOOR_CLOSE, 1.0f, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		}
		break;
	case LevelEvent::SOUND_FIZZ:
		level[playerIndex]->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_FIZZ, 0.5f, 2.6f + (random->nextFloat() - random->nextFloat()) * 0.8f, false);
		break;
	case LevelEvent::SOUND_ANVIL_BROKEN:
		level[playerIndex]->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_ANVIL_BREAK, 1.0f, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		break;
	case LevelEvent::SOUND_ANVIL_USED:
		level[playerIndex]->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_ANVIL_USE, 1.0f, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		break;
	case LevelEvent::SOUND_ANVIL_LAND:
		level[playerIndex]->playLocalSound(x + 0.5f, y + 0.5f, z + 0.5f, eSoundType_RANDOM_ANVIL_LAND, 0.3f, level[playerIndex]->random->nextFloat() * 0.1f + 0.9f, false);
		break;
	case LevelEvent::SOUND_PLAY_RECORDING:
		{
			RecordingItem *rci = dynamic_cast<RecordingItem *>(Item::items[data]);
			if (rci != NULL)
			{
				level[playerIndex]->playStreamingMusic(rci->recording, x, y, z);
			}
			else
			{
				// 4J-PB - only play streaming music if there isn't already some playing - the CD playing may have finished, and game music started playing already
				if(!mc->soundEngine->GetIsPlayingStreamingGameMusic())
				{
					level[playerIndex]->playStreamingMusic(L"", x, y, z);	// 4J - used to pass NULL, but using empty string here now instead
				}
			}
			mc->localplayers[playerIndex]->updateRichPresence();
		}
		break;
		// 4J - new level event sounds brought forward from 1.2.3
	case LevelEvent::SOUND_GHAST_WARNING:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_GHAST_CHARGE, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f, false, 80.0f);
		break;
	case LevelEvent::SOUND_GHAST_FIREBALL:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_GHAST_FIREBALL, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f, false, 80.0f);
		break;
	case LevelEvent::SOUND_ZOMBIE_WOODEN_DOOR:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_ZOMBIE_WOOD, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		break;
	case LevelEvent::SOUND_ZOMBIE_DOOR_CRASH:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_ZOMBIE_WOOD_BREAK, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		break;
	case LevelEvent::SOUND_ZOMBIE_IRON_DOOR:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_ZOMBIE_METAL, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		break;
	case LevelEvent::SOUND_BLAZE_FIREBALL:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_GHAST_FIREBALL, 2, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);//, false);
		break;
	case LevelEvent::SOUND_WITHER_BOSS_SHOOT:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_WITHER_SHOOT, 2, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);//, false);
		break;
	case LevelEvent::SOUND_ZOMBIE_INFECTED:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_ZOMBIE_INFECT, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);//, false);
		break;
	case LevelEvent::SOUND_ZOMBIE_CONVERTED:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_ZOMBIE_UNFECT, 2.0f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);//, false);
		break;
		// 4J Added TU9 to fix #77475 - TU9: Content: Art: Dragon egg teleport particle effect isn't present.
	case LevelEvent::END_EGG_TELEPORT:
		// 4J Added to show the paricles when the End egg teleports after being attacked
		EggTile::generateTeleportParticles(level[playerIndex],x,y,z,data);
		break;
	case LevelEvent::SOUND_BAT_LIFTOFF:
		level[playerIndex]->playLocalSound(x + 0.5, y + 0.5, z + 0.5, eSoundType_MOB_BAT_TAKEOFF, .05f, (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
		break;
	}

}

void LevelRenderer::destroyTileProgress(int id, int x, int y, int z, int progress)
{
	if (progress < 0 || progress >= 10)
	{
        auto it = destroyingBlocks.find(id);
        if(it != destroyingBlocks.end())
		{
			delete it->second;
			destroyingBlocks.erase(it);
		}
		//destroyingBlocks.remove(id);
	}
	else
	{
		BlockDestructionProgress *entry = NULL;

        auto it = destroyingBlocks.find(id);
        if(it != destroyingBlocks.end()) entry = it->second;

		if (entry == NULL || entry->getX() != x || entry->getY() != y || entry->getZ() != z)
		{
			entry = new BlockDestructionProgress(id, x, y, z);
			destroyingBlocks.insert( unordered_map<int, BlockDestructionProgress *>::value_type(id, entry) );
		}

		entry->setProgress(progress);
		entry->updateTick(ticks);
	}
}

void LevelRenderer::registerTextures(IconRegister *iconRegister)
{
	breakingTextures = new Icon*[10];

	for (int i = 0; i < 10; i++)
	{
		breakingTextures[i] = iconRegister->registerIcon(L"destroy_" + std::to_wstring(i) );
	}
}

// Gets a dimension index (0, 1, or 2) from an id ( 0, -1, 1)
int LevelRenderer::getDimensionIndexFromId(int id)
{
	return ( 3 - id ) % 3;
}

// 4J - added for new render list handling. Render lists used to be allocated per chunk, but these are now allocated per fixed chunk position
// in our (now finite) maps.
int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, Level *level)
{
	return getGlobalIndexForChunk(x,y,z,level->dimension->id);
}

int LevelRenderer::getGlobalIndexForChunk(int x, int y, int z, int dimensionId)
{
	int dimIdx = getDimensionIndexFromId(dimensionId);
	int xx = ( x / CHUNK_XZSIZE ) + ( MAX_LEVEL_RENDER_SIZE[dimIdx] / 2 );
	int yy = y / CHUNK_SIZE;
	int zz = ( z / CHUNK_XZSIZE )  + ( MAX_LEVEL_RENDER_SIZE[dimIdx] / 2 );

	if( ( xx < 0 ) || ( xx >= MAX_LEVEL_RENDER_SIZE[dimIdx] ) ) return -1;
	if( ( zz < 0 ) || ( zz >= MAX_LEVEL_RENDER_SIZE[dimIdx] ) ) return -1;
	if( ( yy < 0 ) || ( yy >= CHUNK_Y_COUNT ) ) return -1;

	int dimOffset = DIMENSION_OFFSETS[dimIdx];

	int offset = dimOffset;												// Offset caused by current dimension
	offset += ( zz * MAX_LEVEL_RENDER_SIZE[dimIdx] + xx ) * CHUNK_Y_COUNT;			// Offset by x/z pos
	offset += yy;														// Offset by y pos

	return offset;
}

bool LevelRenderer::isGlobalIndexInSameDimension( int idx, Level *level)
{
	int dim = getDimensionIndexFromId(level->dimension->id);
	int idxDim = 0;
	if( idx >= DIMENSION_OFFSETS[2] ) idxDim = 2;
	else if ( idx >= DIMENSION_OFFSETS[1] ) idxDim = 1;
	return (dim == idxDim);
}

int LevelRenderer::getGlobalChunkCount()
{
	return  ( MAX_LEVEL_RENDER_SIZE[0] * MAX_LEVEL_RENDER_SIZE[0] * CHUNK_Y_COUNT ) +
		( MAX_LEVEL_RENDER_SIZE[1] * MAX_LEVEL_RENDER_SIZE[1] * CHUNK_Y_COUNT ) +
		( MAX_LEVEL_RENDER_SIZE[2] * MAX_LEVEL_RENDER_SIZE[2] * CHUNK_Y_COUNT );
}

int LevelRenderer::getGlobalChunkCountForOverworld()
{
	return ( MAX_LEVEL_RENDER_SIZE[0] * MAX_LEVEL_RENDER_SIZE[0] * CHUNK_Y_COUNT );
}

unsigned char LevelRenderer::getGlobalChunkFlags(int x, int y, int z, Level *level)
{
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index == -1 )
	{
		return 0;
	}
	else
	{
		return globalChunkFlags[ index ];
	}
}

void LevelRenderer::setGlobalChunkFlags(int x, int y, int z, Level *level, unsigned char flags)
{
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index != -1 )
	{
#ifdef _LARGE_WORLDS
		EnterCriticalSection(&m_csChunkFlags);
#endif
		globalChunkFlags[ index ] = flags;
#ifdef _LARGE_WORLDS
		LeaveCriticalSection(&m_csChunkFlags);
#endif
	}
}

void LevelRenderer::setGlobalChunkFlag(int index, unsigned char flag, unsigned char shift)
{
	unsigned char sflag = flag << shift;

	if( index != -1 )
	{
#ifdef _LARGE_WORLDS
		EnterCriticalSection(&m_csChunkFlags);
#endif
		globalChunkFlags[ index ] |= sflag;
#ifdef _LARGE_WORLDS
		LeaveCriticalSection(&m_csChunkFlags);
#endif
	}
}

void LevelRenderer::setGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift)
{
	unsigned char sflag = flag << shift;
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index != -1 )
	{
#ifdef _LARGE_WORLDS
		EnterCriticalSection(&m_csChunkFlags);
#endif
		globalChunkFlags[ index ] |= sflag;
#ifdef _LARGE_WORLDS
		LeaveCriticalSection(&m_csChunkFlags);
#endif
	}
}

void LevelRenderer::clearGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift)
{
	unsigned char sflag = flag << shift;
	int index =  getGlobalIndexForChunk(x, y, z, level);
	if( index != -1 )
	{
#ifdef _LARGE_WORLDS
		EnterCriticalSection(&m_csChunkFlags);
#endif
		globalChunkFlags[ index ] &= ~sflag;
#ifdef _LARGE_WORLDS
		LeaveCriticalSection(&m_csChunkFlags);
#endif
	}
}

bool LevelRenderer::getGlobalChunkFlag(int x, int y, int z, Level *level, unsigned char flag, unsigned char shift)
{
	unsigned char sflag = flag << shift;
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index == -1 )
	{
		return false;
	}
	else
	{
		return ( globalChunkFlags[ index ] & sflag ) == sflag;
	}
}

unsigned char LevelRenderer::incGlobalChunkRefCount(int x, int y, int z, Level *level)
{
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index != -1 )
	{
		unsigned char flags = globalChunkFlags[ index ];
		unsigned char refCount = (flags >> CHUNK_FLAG_REF_SHIFT ) & CHUNK_FLAG_REF_MASK;
		refCount++;
		flags &= ~(CHUNK_FLAG_REF_MASK<<CHUNK_FLAG_REF_SHIFT);
		flags |= refCount << CHUNK_FLAG_REF_SHIFT;
		globalChunkFlags[ index ] = flags;

		return refCount;
	}
	else
	{
		return 0;
	}

}

unsigned char LevelRenderer::decGlobalChunkRefCount(int x, int y, int z, Level *level)
{
	int index = getGlobalIndexForChunk(x, y, z, level);
	if( index != -1 )
	{
		unsigned char flags = globalChunkFlags[ index ];
		unsigned char refCount = (flags >> CHUNK_FLAG_REF_SHIFT ) & CHUNK_FLAG_REF_MASK;
		refCount--;
		flags &= ~(CHUNK_FLAG_REF_MASK<<CHUNK_FLAG_REF_SHIFT);
		flags |= refCount << CHUNK_FLAG_REF_SHIFT;
		globalChunkFlags[ index ] = flags;

		return refCount;
	}
	else
	{
		return 0;
	}
}

// 4J added
void LevelRenderer::fullyFlagRenderableTileEntitiesToBeRemoved()
{
	EnterCriticalSection(&m_csRenderableTileEntities);
    for (auto& it : renderableTileEntities)
    {
		for(auto& it2 : it.second)
		{
			it2->upgradeRenderRemoveStage();
		}
	}
	LeaveCriticalSection(&m_csRenderableTileEntities);
}

LevelRenderer::DestroyedTileManager::RecentTile::RecentTile(int x, int y, int z, Level *level) : x(x), y(y), z(z), level(level)
{
	timeout_ticks = 20;
	rebuilt = false;
}

LevelRenderer::DestroyedTileManager::RecentTile::~RecentTile()
{
	for(auto& it : boxes)
	{
		delete it;
	}
}

LevelRenderer::DestroyedTileManager::DestroyedTileManager()
{
	InitializeCriticalSection(&m_csDestroyedTiles);
}

LevelRenderer::DestroyedTileManager::~DestroyedTileManager()
{
	DeleteCriticalSection(&m_csDestroyedTiles);
	for( unsigned int i = 0; i < m_destroyedTiles.size(); i++ )
	{
		delete m_destroyedTiles[i];
	}
}


// For game to let this manager know that a tile is about to be destroyed (must be called before it actually is)
void LevelRenderer::DestroyedTileManager::destroyingTileAt( Level *level, int x, int y, int z )
{
	EnterCriticalSection(&m_csDestroyedTiles);

	// Store a list of AABBs that the tile to be destroyed would have made, before we go and destroy it. This
	// is made slightly more complicated as the addAABBs method for tiles adds temporary AABBs and we need permanent
	// ones, so make a temporary list and then copy over

	RecentTile *recentTile = new RecentTile(x, y, z, level);
	AABB *box = AABB::newTemp((float)x, (float)y, (float)z, (float)(x+1), (float)(y+1), (float)(z+1));
	Tile *tile = Tile::tiles[level->getTile(x, y, z)];

	if (tile != NULL)
	{
		tile->addAABBs(level, x, y, z, box, &recentTile->boxes, nullptr);
	}

	// Make these temporary AABBs into permanently allocated AABBs
	for( unsigned int i = 0; i < recentTile->boxes.size(); i++ )
	{
		recentTile->boxes[i] = AABB::newPermanent(recentTile->boxes[i]->x0,
			recentTile->boxes[i]->y0,
			recentTile->boxes[i]->z0,
			recentTile->boxes[i]->x1,
			recentTile->boxes[i]->y1,
			recentTile->boxes[i]->z1);
	}

	m_destroyedTiles.push_back( recentTile );

	LeaveCriticalSection(&m_csDestroyedTiles);
}

// For chunk rebuilding to inform the manager that a chunk (a 16x16x16 tile render chunk) has been updated
void LevelRenderer::DestroyedTileManager::updatedChunkAt(Level *level, int x, int y, int z, int veryNearCount)
{
	EnterCriticalSection(&m_csDestroyedTiles);

	// There's 2 stages to this. This function is called when a renderer chunk has been rebuilt, but that chunk's render data might be grouped atomically with
	// changes to other very near chunks. Therefore, we don't want to consider the render data to be fully updated until the chunk that it is in has been
	// rebuilt, AND there aren't any very near things waiting to be rebuilt.

	// First pass through - see if any tiles are within the chunk which is being rebuilt, and mark up by setting their rebuilt flag
	bool printed = false;
	for( unsigned int i = 0; i < m_destroyedTiles.size(); i++)
	{
		if( ( m_destroyedTiles[i]->level == level ) &&
			( m_destroyedTiles[i]->x >= x ) && ( m_destroyedTiles[i]->x < ( x + 16 ) ) &&
			( m_destroyedTiles[i]->y >= y ) && ( m_destroyedTiles[i]->y < ( y + 16 ) ) &&
			( m_destroyedTiles[i]->z >= z ) && ( m_destroyedTiles[i]->z < ( z + 16 ) ) )
		{
			printed = true;
			m_destroyedTiles[i]->rebuilt = true;
		}
	}

	// Now go through every tile that has been marked up as already being rebuilt, and fully remove it once there aren't going to be any more
	// very near chunks. This might not happen on the same call to this function that rebuilt the chunk with the tile in.
	if( veryNearCount <= 1 )
	{
		for( unsigned int i = 0; i < m_destroyedTiles.size(); )
		{
			if( m_destroyedTiles[i]->rebuilt )
			{
				printed = true;
				delete m_destroyedTiles[i];
				m_destroyedTiles[i] = m_destroyedTiles[m_destroyedTiles.size() - 1];
				m_destroyedTiles.pop_back();
			}
			else
			{
				i++;
			}
		}
	}

	LeaveCriticalSection(&m_csDestroyedTiles);
}

// For game to get any AABBs that the user should be colliding with as render data has not yet been updated
void LevelRenderer::DestroyedTileManager::addAABBs( Level *level, AABB *box, AABBList *boxes )
{
	EnterCriticalSection(&m_csDestroyedTiles);

	for( unsigned int i = 0; i < m_destroyedTiles.size(); i++ )
	{
		if( m_destroyedTiles[i]->level == level )
		{
			for( unsigned int j = 0; j < m_destroyedTiles[i]->boxes.size(); j++ )
			{
				// If we find any AABBs intersecting the region we are interested in, add them to the output list, making a temp AABB copy so that we can destroy our own copy
				// without worrying about the lifespan of the copy we've passed out
				if( m_destroyedTiles[i]->boxes[j]->intersects( box ) )
				{
					boxes->push_back(AABB::newTemp( m_destroyedTiles[i]->boxes[j]->x0,
						m_destroyedTiles[i]->boxes[j]->y0,
						m_destroyedTiles[i]->boxes[j]->z0,
						m_destroyedTiles[i]->boxes[j]->x1,
						m_destroyedTiles[i]->boxes[j]->y1,
						m_destroyedTiles[i]->boxes[j]->z1 ) );
				}
			}
		}
	}

	LeaveCriticalSection(&m_csDestroyedTiles);
}

void LevelRenderer::DestroyedTileManager::tick()
{
	EnterCriticalSection(&m_csDestroyedTiles);

	// Remove any tiles that have timed out
	for( unsigned int i = 0; i < m_destroyedTiles.size(); )
	{
		if( --m_destroyedTiles[i]->timeout_ticks == 0 )
		{
			delete m_destroyedTiles[i];
			m_destroyedTiles[i] = m_destroyedTiles[m_destroyedTiles.size() - 1];
			m_destroyedTiles.pop_back();
		}
		else
		{
			i++;
		}
	}

	LeaveCriticalSection(&m_csDestroyedTiles);
}

#ifdef _LARGE_WORLDS
void LevelRenderer::staticCtor()
{
	s_rebuildCompleteEvents = new C4JThread::EventArray(MAX_CHUNK_REBUILD_THREADS);
	char threadName[256];
	for(unsigned int i = 0; i < MAX_CHUNK_REBUILD_THREADS; ++i)
	{
		sprintf(threadName,"Rebuild Chunk Thread %d\n",i);
		rebuildThreads[i] = new C4JThread(rebuildChunkThreadProc,(void *)i,threadName);

		s_activationEventA[i] = new C4JThread::Event();

		// Threads 1,3 and 5 are generally idle so use them
		if((i%3) == 0) rebuildThreads[i]->SetProcessor(CPU_CORE_CHUNK_REBUILD_A);
		else if((i%3) == 1)
		{
			rebuildThreads[i]->SetProcessor(CPU_CORE_CHUNK_REBUILD_B);
#ifdef __ORBIS__
			rebuildThreads[i]->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);	// On Orbis, this core is also used for Matching 2, and that priority of that seems to be always at default no matter what we set it to. Prioritise this below Matching 2.
#endif
		}
		else if((i%3) == 2) rebuildThreads[i]->SetProcessor(CPU_CORE_CHUNK_REBUILD_C);

		//ResumeThread( saveThreads[j] );
		rebuildThreads[i]->Run();
	}
}

int LevelRenderer::rebuildChunkThreadProc(LPVOID lpParam)
{
	Vec3::CreateNewThreadStorage();
	AABB::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Tesselator::CreateNewThreadStorage(1024*1024);
	RenderManager.InitialiseContext();
	Chunk::CreateNewThreadStorage();
	Tile::CreateNewThreadStorage();

	int index = (size_t)lpParam;

	while(true)
	{
		s_activationEventA[index]->WaitForSignal(INFINITE);

		//app.DebugPrintf("Rebuilding permaChunk %d\n", index + 1);
		permaChunk[index + 1].rebuild();

		// Inform the producer thread that we are done with this chunk
		s_rebuildCompleteEvents->Set(index);
	}

	return 0;
}
#endif

// This is called when chunks require rebuilding, but they haven't been added individually to the dirtyChunksLockFreeStack. Once in this
// state, the rebuilding thread will keep assuming there are dirty chunks until it has had a full pass through the chunks and found no dirty ones
void LevelRenderer::nonStackDirtyChunksAdded()
{
	dirtyChunksLockFreeStack.Push((int *)1);
}

// 4J - for test purposes, check all chunks that are currently present for the player. Currently this is implemented to do tests to identify missing client chunks in flat worlds, but
// this could be extended to do other kinds of automated testing. Returns the number of chunks that are present, so that from the calling function we can determine when chunks have
// finished loading/generating round the current location.
int LevelRenderer::checkAllPresentChunks(bool *faultFound)
{
	int playerIndex = mc->player->GetXboxPad();	// 4J added

	int presentCount = 0;
	ClipChunk *pClipChunk = chunks[playerIndex].data;
	for( int i = 0; i < chunks[playerIndex].length; i++, pClipChunk++ )
	{
		if(pClipChunk->chunk->y == 0 )
		{
			bool chunkPresent = level[0]->reallyHasChunk(pClipChunk->chunk->x>>4,pClipChunk->chunk->z>>4);
			if( chunkPresent )
			{
				presentCount++;
				LevelChunk *levelChunk = level[0]->getChunk(pClipChunk->chunk->x>>4,pClipChunk->chunk->z>>4);

				for( int cx = 4; cx <= 12; cx++ )
				{
					for( int cz = 4; cz <= 12; cz++ )
					{
						int t0 = levelChunk->getTile(cx, 0, cz);
						if( ( t0 != Tile::unbreakable_Id ) && (t0 != Tile::dirt_Id) )
						{
							*faultFound = true;
						}
					}
				}
			}
		}
	}
	return presentCount;
}

