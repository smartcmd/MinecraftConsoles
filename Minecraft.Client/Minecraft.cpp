#include "stdafx.h"
#include "Minecraft.h"
#include "Common/UI/UIScene.h"
#include "GameMode.h"
#include "Timer.h"
#include "ProgressRenderer.h"
#include "LevelRenderer.h"
#include "ParticleEngine.h"
#include "MultiPlayerLocalPlayer.h"
#include "User.h"
#include "Textures.h"
#include "GameRenderer.h"
#include "ItemInHandRenderer.h"
#include "HumanoidModel.h"
#include "Options.h"
#include "TexturePackRepository.h"
#include "StatsCounter.h"
#include "EntityRenderDispatcher.h"
#include "TileEntityRenderDispatcher.h"
#include "SurvivalMode.h"
#include "Chunk.h"
#include "CreativeMode.h"
#include "DemoLevel.h"
#include "MultiPlayerLevel.h"
#include "DemoUser.h"
#include "GuiParticles.h"
#include "Screen.h"
#include "DeathScreen.h"
#include "ErrorScreen.h"
#include "TitleScreen.h"
#include "InventoryScreen.h"
#include "InBedChatScreen.h"
#include "AchievementPopup.h"
#include "Input.h"
#include "FrustumCuller.h"
#include "Camera.h"

#include "..\Minecraft.World\MobEffect.h"
#include "..\Minecraft.World\Difficulty.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\net.minecraft.h"
#include "..\Minecraft.World\net.minecraft.stats.h"
#include "..\Minecraft.World\System.h"
#include "..\Minecraft.World\ByteBuffer.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\Minecraft.World.h"
#include "Windows64\Windows64_Xuid.h"
#include "ClientConnection.h"
#include "..\Minecraft.World\HellRandomLevelSource.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\StrongholdFeature.h"
#include "..\Minecraft.World\IntCache.h"
#include "..\Minecraft.World\Villager.h"
#include "..\Minecraft.World\SparseLightStorage.h"
#include "..\Minecraft.World\SparseDataStorage.h"
#include "..\Minecraft.World\ChestTileEntity.h"
#include "TextureManager.h"
#ifdef _XBOX
#include "Xbox\Network\NetworkPlayerXbox.h"
#endif
#include "Common\UI\IUIScene_CreativeMenu.h"
#include "Common\UI\UIFontData.h"
#include "DLCTexturePack.h"

#ifdef __ORBIS__
#include "Orbis\Network\PsPlusUpsellWrapper_Orbis.h"
#endif

// If not disabled, this creates an event queue on a separate thread so that the Level::tick calls can be offloaded
// from the main thread, and have longer to run, since it's called at 20Hz instead of 60
#define DISABLE_LEVELTICK_THREAD

Minecraft *Minecraft::m_instance = nullptr;
int64_t Minecraft::frameTimes[512];
int64_t Minecraft::tickTimes[512];
int Minecraft::frameTimePos = 0;
int64_t Minecraft::warezTime = 0;
File Minecraft::workDir = File(L"");

extern ConsoleUIController ui;

#ifdef __PSVITA__

TOUCHSCREENRECT QuickSelectRect[3]=
{
	{ 560, 890, 1360, 980 },
	{ 450, 840, 1449, 960 },
	{ 320, 840, 1600, 970 },
};

int QuickSelectBoxWidth[3]=
{
	89,
	111,
	142
};

int iToolTipOffset = 85;

#endif

ResourceLocation Minecraft::DEFAULT_FONT_LOCATION = ResourceLocation(TN_DEFAULT_FONT);
ResourceLocation Minecraft::ALT_FONT_LOCATION = ResourceLocation(TN_ALT_FONT);


Minecraft::Minecraft(Component *mouseComponent, Canvas *parent, MinecraftApplet *minecraftApplet, int width, int height, bool fullscreen)
{
	gameMode = nullptr;
	hasCrashed = false;
	timer = new Timer(SharedConstants::TICKS_PER_SECOND);
	oldLevel = nullptr;
	level = nullptr;
	levels = MultiPlayerLevelArray(3);
	levelRenderer = nullptr;
	player = nullptr;
	cameraTargetPlayer = nullptr;
	particleEngine = nullptr;
	user = nullptr;
	parent = nullptr;
	pause = false;
	textures = nullptr;
	font = nullptr;
	screen = nullptr;
	localPlayerIdx = 0;
	rightClickDelay = 0;

	InitializeCriticalSection( &ProgressRenderer::s_progress );
	InitializeCriticalSection(&m_setLevelCS);

	progressRenderer = nullptr;
	gameRenderer = nullptr;
	bgLoader = nullptr;

	ticks = 0;

	orgWidth = orgHeight = 0;
	achievementPopup = new AchievementPopup(this);
	gui = nullptr;
	noRender = false;
	humanoidModel = new HumanoidModel(0);
	hitResult = nullptr;
	options = nullptr;
	soundEngine = new SoundEngine();
	mouseHandler = nullptr;
	skins = nullptr;
	workingDirectory = File(L"");
	levelSource = nullptr;
	stats[0] = nullptr;
	stats[1] = nullptr;
	stats[2] = nullptr;
	stats[3] = nullptr;
	connectToPort = 0;
	workDir = File(L"");
	lastTimer = -1;

	recheckPlayerIn = 0;
	running = true;
	unoccupiedQuadrant = -1;

	Stats::init();

	orgHeight = height;
	this->fullscreen = fullscreen;
	this->minecraftApplet = nullptr;

	this->parent = parent;
	// Our actual physical frame buffer is always 1280x720 (16:9). For 4:3 mode, we tell the original
	// Minecraft code the width is 3/4 of actual, to correctly present a 4:3 image.
	// width_phys and height_phys hold the real physical frame buffer dimensions.
	if( RenderManager.IsWidescreen() )
	{
		this->width = width;
	}
	else
	{
		this->width = (width * 3 ) / 4;
	}
	this->height = height;
	this->width_phys = width;
	this->height_phys = height;

	this->fullscreen = fullscreen;

	appletMode = false;

	Minecraft::m_instance = this;
	TextureManager::createInstance();

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_pendingLocalConnections[i] = nullptr;
		m_connectionFailed[i] = false;
		localgameModes[i]= nullptr;
		localitemInHandRenderers[i] = nullptr;
	}

	animateTickLevel = nullptr;
	m_inFullTutorialBits = 0;
	reloadTextures = false;

	// Initialise audio before any textures are loaded to avoid a Win64 issue where
	// Miles audio causes the texture codec to be unloaded.
#ifndef __ORBIS__
	this->soundEngine->init(nullptr);
#endif

#ifndef DISABLE_LEVELTICK_THREAD
	levelTickEventQueue = new C4JThread::EventQueue(levelTickUpdateFunc, levelTickThreadInitFunc, "LevelTick_EventQueuePoll");
	levelTickEventQueue->setProcessor(3);
	levelTickEventQueue->setPriority(THREAD_PRIORITY_NORMAL);
#endif
}

void Minecraft::clearConnectionFailed()
{
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_connectionFailed[i] = false;
		m_connectionFailedReason[i] = DisconnectPacket::eDisconnect_None;
	}
	app.SetDisconnectReason(DisconnectPacket::eDisconnect_None);
}

void Minecraft::connectTo(const wstring& server, int port)
{
	connectToIp = server;
	connectToPort = port;
}

void Minecraft::init()
{
	workingDirectory = File(L"");
	levelSource = new McRegionLevelStorageSource(File(workingDirectory, L"saves"));
	options = new Options(this, workingDirectory);
	skins = new TexturePackRepository(workingDirectory, this);
	skins->addDebugPacks();
	textures = new Textures(skins, options);

	font = new Font(options, L"font/Default.png", textures, false, &DEFAULT_FONT_LOCATION, 23, 20, 8, 8, SFontData::Codepoints);
	altFont = new Font(options, L"font/alternate.png", textures, false, &ALT_FONT_LOCATION, 16, 16, 8, 8);

	gameRenderer = new GameRenderer(this);
	EntityRenderDispatcher::instance->itemInHandRenderer = new ItemInHandRenderer(this,false);

	for( int i=0 ; i<4 ; ++i )
		stats[i] = new StatsCounter();

	Mouse::create();

	MemSect(31);
	checkGlError(L"Pre startup");
	MemSect(0);

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glCullFace(GL_BACK);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	MemSect(31);
	checkGlError(L"Startup");
	MemSect(0);

	levelRenderer = new LevelRenderer(this, textures);
	textures->stitch();

	glViewport(0, 0, width, height);

	particleEngine = new ParticleEngine(level, textures);

	MemSect(31);
	checkGlError(L"Post startup");
	MemSect(0);
	gui = new Gui(this);

	if (connectToIp != L"")
	{
		// TODO: setScreen(new ConnectScreen(this, connectToIp, connectToPort));
	}
	else
	{
		setScreen(new TitleScreen());
	}
	progressRenderer = new ProgressRenderer(this);

	RenderManager.CBuffLockStaticCreations();
}

void Minecraft::renderLoadingScreen()
{
#ifdef __PSVITA__
	ScreenSizeCalculator ssc(options, width, height);

	RenderManager.StartFrame();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (float)ssc.rawWidth, (float)ssc.rawHeight, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);
	glViewport(0, 0, width, height);
	glClearColor(0, 0, 0, 0);

	Tesselator *t = Tesselator::getInstance();

	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
	glBindTexture(GL_TEXTURE_2D, textures->loadTexture(TN_MOB_PIG));
	t->begin();
	t->color(0xffffff);
	t->vertexUV((float)(0), (float)( height), (float)( 0), (float)( 0), (float)( 0));
	t->vertexUV((float)(width), (float)( height), (float)( 0), (float)( 0), (float)( 0));
	t->vertexUV((float)(width), (float)( 0), (float)( 0), (float)( 0), (float)( 0));
	t->vertexUV((float)(0), (float)( 0), (float)( 0), (float)( 0), (float)( 0));
	t->end();

	int lw = 256;
	int lh = 256;
	glColor4f(1, 1, 1, 1);
	t->color(0xffffff);
	blit((ssc.getWidth() - lw) / 2, (ssc.getHeight() - lh) / 2, 0, 0, lw, lh);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);

	Display::swapBuffers();
	RenderManager.Present();
#endif
}

void Minecraft::blit(int x, int y, int sx, int sy, int w, int h)
{
	float us = 1 / 256.0f;
	float vs = 1 / 256.0f;
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->vertexUV(static_cast<float>(x + 0), static_cast<float>(y + h), static_cast<float>(0), (float)( (sx + 0) * us), (float)( (sy + h) * vs));
	t->vertexUV(static_cast<float>(x + w), static_cast<float>(y + h), static_cast<float>(0), (float)( (sx + w) * us), (float)( (sy + h) * vs));
	t->vertexUV(static_cast<float>(x + w), static_cast<float>(y + 0), static_cast<float>(0), (float)( (sx + w) * us), (float)( (sy + 0) * vs));
	t->vertexUV(static_cast<float>(x + 0), static_cast<float>(y + 0), static_cast<float>(0), (float)( (sx + 0) * us), (float)( (sy + 0) * vs));
	t->end();
}

LevelStorageSource *Minecraft::getLevelSource()
{
	return levelSource;
}

void Minecraft::setScreen(Screen *screen)
{
	if (this->screen != nullptr)
	{
		this->screen->removed();
	}

#ifdef _WINDOWS64
	if (screen != nullptr && g_KBMInput.IsMouseGrabbed())
	{
		g_KBMInput.SetMouseGrabbed(false);
	}
#endif

	if (screen == nullptr && level == nullptr)
	{
		screen = new TitleScreen();
	}
	else if (player != nullptr && !ui.GetMenuDisplayed(player->GetXboxPad()) && player->getHealth() <= 0)
	{
		if(ticks==0)
		{
			player->respawn();
		}
		else
		{
			ui.NavigateToScene(player->GetXboxPad(),eUIScene_DeathMenu,nullptr);
		}
	}

	if (dynamic_cast<TitleScreen *>(screen)!=nullptr)
	{
		options->renderDebug = false;
		gui->clearMessages();
	}

	this->screen = screen;
	if (screen != nullptr)
	{
		ScreenSizeCalculator ssc(options, width, height);
		int screenWidth = ssc.getWidth();
		int screenHeight = ssc.getHeight();
		screen->init(this, screenWidth, screenHeight);
		noRender = false;
	}
}

void Minecraft::checkGlError(const wstring& string)
{
	// TODO
}

void Minecraft::destroy()
{
	setLevel(nullptr);
	MemoryTracker::release();
	soundEngine->destroy();
	Display::destroy();
}

void Minecraft::run()
{
	running = true;
	init();
}

bool Minecraft::setLocalPlayerIdx(int idx)
{
	localPlayerIdx = idx;
	// If the player is not null but the game mode is, this is a temp player
	// whose only real purpose is to hold the viewport position.
	if( localplayers[idx] == nullptr || localgameModes[idx] == nullptr ) return false;

	gameMode = localgameModes[idx];
	player = localplayers[idx];
	cameraTargetPlayer = localplayers[idx];
	gameRenderer->itemInHandRenderer = localitemInHandRenderers[idx];
	level = getLevel( localplayers[idx]->dimension );
	particleEngine->setLevel( level );

	return true;
}

int Minecraft::getLocalPlayerIdx()
{
	return localPlayerIdx;
}

void Minecraft::updatePlayerViewportAssignments()
{
	unoccupiedQuadrant = -1;
	int viewportsRequired = 0;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		if( localplayers[i] != nullptr ) viewportsRequired++;
	}
	if( viewportsRequired == 3 ) viewportsRequired = 4;

	if( viewportsRequired == 1 )
	{
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != nullptr ) localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
		}
	}
	else if( viewportsRequired == 2 )
	{
		int found = 0;
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != nullptr )
			{
				if(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_SplitScreenVertical))
				{
					localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_SPLIT_LEFT + found;
				}
				else
				{
					localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_SPLIT_TOP + found;
				}
				found++;
			}
		}
	}
	else if( viewportsRequired >= 3 )
	{
		// Quadrant allocation: persist existing assignments to avoid rearranging viewports
		// when going from 3 to 4 players (or vice versa).
		bool quadrantsAllocated[4] = {false,false,false,false};

		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != nullptr )
			{
				// If the game hasn't started yet, ignore current allocations so the primary
				// player doesn't end up in the wrong quadrant.
				if(app.GetGameStarted())
				{
					if( ( localplayers[i]->m_iScreenSection >= C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT ) &&
						( localplayers[i]->m_iScreenSection <= C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT ) )
					{
						quadrantsAllocated[localplayers[i]->m_iScreenSection - C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT] = true;
					}
				}
				else
				{
					localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
				}
			}
		}

		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != nullptr )
			{
				if( ( localplayers[i]->m_iScreenSection < C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT ) ||
					( localplayers[i]->m_iScreenSection > C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT ) )
				{
					for( int j = 0; j < 4; j++ )
					{
						if( !quadrantsAllocated[j] )
						{
							localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + j;
							quadrantsAllocated[j] = true;
							break;
						}
					}
				}
			}
		}
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( quadrantsAllocated[i] == false )
			{
				unoccupiedQuadrant = i;
			}
		}
	}

	if(app.GetGameStarted()) ui.UpdatePlayerBasePositions();
}

bool Minecraft::addLocalPlayer(int idx)
{
	if( m_pendingLocalConnections[idx] != nullptr )
	{
		assert(false);
		m_pendingLocalConnections[idx]->close();
	}
	m_connectionFailed[idx] = false;
	m_pendingLocalConnections[idx] = nullptr;

	bool success=g_NetworkManager.AddLocalPlayerByUserIndex(idx);

	if(success)
	{
		app.DebugPrintf("Adding temp local player on pad %d\n", idx);
		localplayers[idx] = shared_ptr<MultiplayerLocalPlayer>(new MultiplayerLocalPlayer(this, level, user, nullptr));
		localgameModes[idx] = nullptr;

		updatePlayerViewportAssignments();

#ifdef _XBOX
		XUIMessage xuiMsg;
		CustomMessage_Splitscreenplayer_Struct myMsgData;
		CustomMessage_Splitscreenplayer( &xuiMsg, &myMsgData, true);

		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(i), &xuiMsg );
		}
#endif

		ConnectionProgressParams *param = new ConnectionProgressParams();
		param->iPad = idx;
		param->stringId = IDS_PROGRESS_CONNECTING;
		param->showTooltips = true;
		param->setFailTimer = true;
		param->timerTime = CONNECTING_PROGRESS_CHECK_TIME;

		ui.NavigateToScene(idx, eUIScene_ConnectingProgress, param);
	}
	else
	{
		app.DebugPrintf("g_NetworkManager.AddLocalPlayerByUserIndex failed\n");
#ifdef _DURANGO
		ProfileManager.RemoveGamepadFromGame(idx);
#endif
	}

	return success;
}

void Minecraft::addPendingLocalConnection(int idx, ClientConnection *connection)
{
	m_pendingLocalConnections[idx] = connection;
}

shared_ptr<MultiplayerLocalPlayer> Minecraft::createExtraLocalPlayer(int idx, const wstring& name, int iPad, int iDimension, ClientConnection *clientConnection /*= nullptr*/,MultiPlayerLevel *levelpassedin)
{
	if( clientConnection == nullptr) return nullptr;

	if( clientConnection == m_pendingLocalConnections[idx] )
	{
		int tempScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
		if( localplayers[idx] != nullptr && localgameModes[idx] == nullptr )
		{
			tempScreenSection = localplayers[idx]->m_iScreenSection;
		}
		wstring prevname = user->name;
		user->name = name;

		m_pendingLocalConnections[idx] = nullptr;

		MultiPlayerLevel *mpLevel;

		if(levelpassedin)
		{
			level=levelpassedin;
			mpLevel=levelpassedin;
		}
		else
		{
			level=getLevel( iDimension );
			mpLevel = getLevel( iDimension );
			mpLevel->addClientConnection( clientConnection );
		}

		if( app.GetTutorialMode() )
		{
			localgameModes[idx] = new FullTutorialMode(idx, this, clientConnection);
		}
		else if(ProfileManager.IsFullVersion()==false)
		{
			localgameModes[idx] = new TrialMode(idx, this, clientConnection);
		}
		else
		{
			localgameModes[idx] = new ConsoleGameMode(idx, this, clientConnection);
		}

		localplayers[idx] = localgameModes[idx]->createPlayer(level);

		PlayerUID playerXUIDOffline = INVALID_XUID;
		PlayerUID playerXUIDOnline = INVALID_XUID;
		ProfileManager.GetXUID(idx,&playerXUIDOffline,false);
		ProfileManager.GetXUID(idx,&playerXUIDOnline,true);
#ifdef _WINDOWS64
		// Compatibility rule for Win64 id migration:
		// host keeps legacy host XUID, non-host uses persistent uid.dat XUID.
		INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(idx);
		if(localNetworkPlayer != nullptr && localNetworkPlayer->IsHost())
		{
			playerXUIDOffline = Win64Xuid::GetLegacyEmbeddedHostXuid();
		}
		else
		{
			playerXUIDOffline = Win64Xuid::ResolvePersistentXuid();
		}
#endif
		localplayers[idx]->setXuid(playerXUIDOffline);
		localplayers[idx]->setOnlineXuid(playerXUIDOnline);
		localplayers[idx]->setIsGuest(ProfileManager.IsGuest(idx));

		localplayers[idx]->m_displayName = ProfileManager.GetDisplayName(idx);

		localplayers[idx]->m_iScreenSection = tempScreenSection;

		if( levelpassedin == nullptr) level->addEntity(localplayers[idx]);

		localplayers[idx]->SetXboxPad(iPad);

		if( localplayers[idx]->input != nullptr ) delete localplayers[idx]->input;
		localplayers[idx]->input = new Input();

		localplayers[idx]->resetPos();

		levelRenderer->setLevel(idx, level);
		localplayers[idx]->level = level;

		user->name = prevname;

		updatePlayerViewportAssignments();
	}

	return localplayers[idx];
}

void Minecraft::storeExtraLocalPlayer(int idx)
{
	localplayers[idx] = player;

	if( localplayers[idx]->input != nullptr ) delete localplayers[idx]->input;
	localplayers[idx]->input = new Input();

	if(ProfileManager.IsSignedIn(idx))
	{
		localplayers[idx]->name = convStringToWstring( ProfileManager.GetGamertag(idx) );
	}
}

void Minecraft::removeLocalPlayerIdx(int idx)
{
	bool updateXui = true;
	if(localgameModes[idx] != nullptr)
	{
		if( getLevel( localplayers[idx]->dimension )->isClientSide )
		{
			shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
			( (MultiPlayerLevel *)getLevel( localplayers[idx]->dimension ) )->removeClientConnection(mplp->connection, true);
			delete mplp->connection;
			mplp->connection = nullptr;
			g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
		}
		getLevel( localplayers[idx]->dimension )->removeEntity(localplayers[idx]);

#ifdef _XBOX
		app.TutorialSceneNavigateBack(idx);
#endif

		playerLeftTutorial( idx );

		delete localgameModes[idx];
		localgameModes[idx] = nullptr;
	}
	else if( m_pendingLocalConnections[idx] != nullptr )
	{
		m_pendingLocalConnections[idx]->sendAndDisconnect(std::make_shared<DisconnectPacket>(DisconnectPacket::eDisconnect_Quitting));;
		delete m_pendingLocalConnections[idx];
		m_pendingLocalConnections[idx] = nullptr;
		g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
	}
	else
	{
#ifdef _XBOX
		updateXui = false;
#endif
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
#endif
	}
	localplayers[idx] = nullptr;

	if( idx == ProfileManager.GetPrimaryPad() )
	{
		// We should never try to remove the primary player this way.
		assert(false);
	}
	else if( updateXui )
	{
		gameRenderer->DisableUpdateThread();
		levelRenderer->setLevel(idx, nullptr);
		gameRenderer->EnableUpdateThread();
		ui.CloseUIScenes(idx,true);
		updatePlayerViewportAssignments();
	}
}

void Minecraft::createPrimaryLocalPlayer(int iPad)
{
	localgameModes[iPad] = gameMode;
	localplayers[iPad] = player;
	if(ProfileManager.IsSignedIn(ProfileManager.GetPrimaryPad()))
	{
		user->name = convStringToWstring( ProfileManager.GetGamertag(ProfileManager.GetPrimaryPad()) );
	}
}

#ifdef _WINDOWS64
void Minecraft::applyFrameMouseLook()
{
	// Per-frame mouse look: consume mouse deltas every frame instead of waiting
	// for the 20Hz game tick. Apply the same delta to both xRot/yRot AND xRotO/yRotO
	// so the render interpolation instantly reflects the change without waiting for a tick.
	if (level == nullptr) return;

	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		if (localplayers[i] == nullptr) continue;
		int iPad = localplayers[i]->GetXboxPad();
		if (iPad != 0) continue;

		if (!g_KBMInput.IsMouseGrabbed()) continue;
		if (localgameModes[iPad] == nullptr) continue;

		float rawDx, rawDy;
		g_KBMInput.ConsumeMouseDelta(rawDx, rawDy);
		if (rawDx == 0.0f && rawDy == 0.0f) continue;

		float mouseSensitivity = static_cast<float>(app.GetGameSettings(iPad, eGameSetting_Sensitivity_InGame)) / 100.0f;
		float mdx = rawDx * mouseSensitivity;
		float mdy = -rawDy * mouseSensitivity;
		if (app.GetGameSettings(iPad, eGameSetting_ControlInvertLook))
			mdy = -mdy;

		// Apply 0.15f scaling (same as Entity::interpolateTurn / Entity::turn)
		float dyaw = mdx * 0.15f;
		float dpitch = -mdy * 0.15f;

		// Apply to both current and old rotation so render interpolation
		// reflects the change immediately (no 50ms tick delay).
		localplayers[i]->yRot += dyaw;
		localplayers[i]->yRotO += dyaw;
		localplayers[i]->xRot += dpitch;
		localplayers[i]->xRotO += dpitch;

		if (localplayers[i]->xRot < -90.0f) localplayers[i]->xRot = -90.0f;
		if (localplayers[i]->xRot > 90.0f) localplayers[i]->xRot = 90.0f;
		if (localplayers[i]->xRotO < -90.0f) localplayers[i]->xRotO = -90.0f;
		if (localplayers[i]->xRotO > 90.0f) localplayers[i]->xRotO = 90.0f;
	}
}
#endif

void Minecraft::run_middle()
{
	static int64_t lastTime = 0;
	static bool bFirstTimeIntoGame = true;
	static bool bAutosaveTimerSet=false;
	static unsigned int uiAutosaveTimer=0;
	static int iFirstTimeCountdown=60;
	if( lastTime == 0 ) lastTime = System::nanoTime();
	static int frames = 0;

	EnterCriticalSection(&m_setLevelCS);

	if(running)
	{
		if (reloadTextures)
		{
			reloadTextures = false;
			textures->reloadAll();
		}

		{
			AABB::resetPool();
			Vec3::resetPool();

			// Autosave timer — only in full game when we are the host
			if(level!=nullptr && ProfileManager.IsFullVersion() && g_NetworkManager.IsHost())
			{
				{
					if(!StorageManager.GetSaveDisabled() && (app.GetXuiAction(ProfileManager.GetPrimaryPad())==eAppAction_Idle) )
					{
						if(!ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) && !ui.IsIgnoreAutosaveMenuDisplayed(ProfileManager.GetPrimaryPad()))
						{
							unsigned char ucAutosaveVal=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Autosave);
							bool bTrialTexturepack=false;
							if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
							{
								TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
								DLCTexturePack *pDLCTexPack=static_cast<DLCTexturePack *>(tPack);

								DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();

								if( pDLCPack )
								{
									if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
									{
										bTrialTexturepack=true;
									}
								}
							}

							if((ucAutosaveVal!=0) && !bTrialTexturepack)
							{
								if(app.AutosaveDue())
								{
									ui.ShowAutosaveCountdownTimer(false);

									app.DebugPrintf("+++++++++++\n");
									app.DebugPrintf("+++Autosave\n");
									app.DebugPrintf("+++++++++++\n");
									app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_AutosaveSaveGame);
#ifndef _CONTENT_PACKAGE
									{
										SYSTEMTIME UTCSysTime;
										GetSystemTime( &UTCSysTime );
										app.DebugPrintf("%02d:%02d:%02d\n",UTCSysTime.wHour,UTCSysTime.wMinute,UTCSysTime.wSecond);
									}
#endif
								}
								else
								{
									unsigned int uiTimeToAutosave=app.SecondsToAutosave();

									if(uiTimeToAutosave<6)
									{
										ui.ShowAutosaveCountdownTimer(true);
										ui.UpdateAutosaveCountdownTimer(uiTimeToAutosave);
									}
								}
							}
						}
						else
						{
							ui.ShowAutosaveCountdownTimer(false);
						}
					}
				}
			}

			// Once in the level, check if players have the level in their banned list
			for( int i = 0; i < XUSER_MAX_COUNT; i++ )
			{
				if( localplayers[i] && (app.GetBanListCheck(i)==false) && !Minecraft::GetInstance()->isTutorial() && ProfileManager.IsSignedInLive(i) && !ProfileManager.IsGuest(i) )
				{
					if(!ProfileManager.IsSystemUIDisplayed())
					{
						app.SetBanListCheck(i,true);
#if defined _XBOX || defined _XBOX_ONE
						INetworkPlayer *pHostPlayer = g_NetworkManager.GetHostPlayer();

#ifdef _XBOX
						PlayerUID xuid=((NetworkPlayerXbox *)pHostPlayer)->GetUID();
#else
						PlayerUID xuid=pHostPlayer->GetUID();
#endif

						if(app.IsInBannedLevelList(i,xuid,app.GetUniqueMapName()))
						{
							app.DebugPrintf("This level is banned\n");
							app.SetAction(i,eAppAction_LevelInBanLevelList,(void *)TRUE);
						}
#endif
					}
				}
			}

			if(!ProfileManager.IsSystemUIDisplayed() && app.DLCInstallProcessCompleted() && !app.DLCInstallPending() && app.m_dlcManager.NeedsCorruptCheck() )
			{
				app.m_dlcManager.checkForCorruptDLCAndAlert();
			}

			// On first level load, check for connected pads not yet in the game and prompt them to press start
			if(level!=nullptr && bFirstTimeIntoGame && g_NetworkManager.SessionHasSpace())
			{
				if(iFirstTimeCountdown==0)
				{
					bFirstTimeIntoGame=false;

					if(app.IsLocalMultiplayerAvailable())
					{
						for( int i = 0; i < XUSER_MAX_COUNT; i++ )
						{
							if((localplayers[i] == nullptr) && InputManager.IsPadConnected(i))
							{
								if(!ui.PressStartPlaying(i))
								{
									ui.ShowPressStart(i);
								}
							}
						}
					}
				}
				else iFirstTimeCountdown--;
			}

			// Store button toggles for all players. Minecraft::tick may not be called every
			// frame (runs at 20Hz), so a press-and-release within one frame would be missed.
			for( int i = 0; i < XUSER_MAX_COUNT; i++ )
			{
#ifdef __ORBIS__
				if ( m_pPsPlusUpsell != nullptr && m_pPsPlusUpsell->hasResponse() && m_pPsPlusUpsell->m_userIndex == i )
				{
					delete m_pPsPlusUpsell;
					m_pPsPlusUpsell = nullptr;

					if ( ProfileManager.HasPlayStationPlus(i) )
					{
						app.DebugPrintf("<Minecraft.cpp> Player_%i is now authorised for PsPlus.\n", i);
						if (!ui.PressStartPlaying(i)) ui.ShowPressStart(i);
					}
					else
					{
						UINT uiIDA[1] = { IDS_OK };
						ui.RequestErrorMessage( IDS_CANTJOIN_TITLE, IDS_NO_PLAYSTATIONPLUS, uiIDA, 1, i);
					}
				}
				else
#endif
				if(localplayers[i])
				{
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_JUMP))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_JUMP;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_USE))					localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_USE;

					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_INVENTORY))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_INVENTORY;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_ACTION))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_ACTION;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_CRAFTING))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_CRAFTING;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_PAUSEMENU))
					{
						localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_PAUSEMENU;
						app.DebugPrintf("PAUSE PRESSED - ipad = %d, Storing press\n",i);
					}
#ifdef _DURANGO
					if(InputManager.ButtonPressed(i, ACTION_MENU_GTC_PAUSE))				localplayers[i]->ullButtonsPressed|=1LL<<ACTION_MENU_GTC_PAUSE;
#endif
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DROP))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_DROP;

					if(localplayers[i]->abilities.flying)
					{
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_SNEAK_TOGGLE))		localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
					}
					else
					{
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_SNEAK_TOGGLE))	localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
					}
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_RENDER_THIRD_PERSON))	localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_THIRD_PERSON;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_GAME_INFO))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_GAME_INFO;

#ifdef _WINDOWS64
					if (i == 0)
					{
						if (g_KBMInput.IsKBMActive())
						{
							if(g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_ACTION;

							if(g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_RIGHT))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_USE;

							bool isClosableByEitherKey = ui.IsSceneInStack(i, eUIScene_FurnaceMenu) ||
								ui.IsSceneInStack(i, eUIScene_ContainerMenu) ||
								ui.IsSceneInStack(i, eUIScene_DispenserMenu) ||
								ui.IsSceneInStack(i, eUIScene_EnchantingMenu) ||
								ui.IsSceneInStack(i, eUIScene_BrewingStandMenu) ||
								ui.IsSceneInStack(i, eUIScene_TradingMenu) ||
								ui.IsSceneInStack(i, eUIScene_AnvilMenu) ||
								ui.IsSceneInStack(i, eUIScene_HopperMenu) ||
								ui.IsSceneInStack(i, eUIScene_BeaconMenu) ||
								ui.IsSceneInStack(i, eUIScene_InventoryMenu) ||
								ui.IsSceneInStack(i, eUIScene_HorseMenu);
							bool isEditing = ui.GetTopScene(i) && ui.GetTopScene(i)->isDirectEditBlocking();

							if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_INVENTORY))
							{
								if(isClosableByEitherKey && !isEditing)
								{
									ui.CloseUIScenes(i);
								}
								else
								{
									localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_INVENTORY;
								}
							}

							if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_DROP))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_DROP;

							if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_CRAFTING) || g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_CRAFTING_ALT))
							{
								if((ui.IsSceneInStack(i, eUIScene_Crafting2x2Menu) || ui.IsSceneInStack(i, eUIScene_Crafting3x3Menu) || ui.IsSceneInStack(i, eUIScene_CreativeMenu) || isClosableByEitherKey) && !isEditing)
								{
									ui.CloseUIScenes(i);
								}
								else
								{
									localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_CRAFTING;
								}
							}

							for (int slot = 0; slot < 9; slot++)
							{
								if (g_KBMInput.IsKeyPressed('1' + slot))
								{
									if (localplayers[i]->inventory)
										localplayers[i]->inventory->selected = slot;
								}
							}
						}

						if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_PAUSE) && !ui.GetMenuDisplayed(i))
						{
							localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_PAUSEMENU;
							app.DebugPrintf("PAUSE PRESSED (keyboard) - ipad = %d\n",i);
						}

						if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_THIRD_PERSON))
							localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_THIRD_PERSON;

						if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_DEBUG_MENU))
						{
							localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_DEBUG;
						}

						if(g_KBMInput.IsKBMActive() && g_KBMInput.IsKeyDown(KeyboardMouseInput::KEY_SNEAK))
						{
							if (localplayers[i]->abilities.flying && !ui.GetMenuDisplayed(i))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
						}
					}
#endif

#if _DEBUG
					if( app.DebugSettingsOn() && app.GetUseDPadForDebug() )
					{
						localplayers[i]->ullDpad_last = 0;
						localplayers[i]->ullDpad_this = 0;
						localplayers[i]->ullDpad_filtered = 0;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_RIGHT))	localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_CHANGE_SKIN;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_UP))		localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_FLY_TOGGLE;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_DOWN))	localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_DEBUG;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_LEFT))	localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SPAWN_CREEPER;
					}
					else
#endif
					{
						// D-Pad movement: ignore diagonals by only reporting the last single direction pressed,
						// to avoid accidental diagonal inputs.
						localplayers[i]->ullDpad_this = 0;
						int dirCount = 0;

#ifndef __PSVITA__
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_LEFT))		{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_LEFT;	dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_RIGHT))	{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_RIGHT;	dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_UP))		{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_UP;		dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_DOWN))		{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_DOWN;	dirCount++; }
#endif

						if( dirCount <= 1 )
						{
							localplayers[i]->ullDpad_last = localplayers[i]->ullDpad_this;
							localplayers[i]->ullDpad_filtered = localplayers[i]->ullDpad_this;
						}
						else
						{
							localplayers[i]->ullDpad_filtered = localplayers[i]->ullDpad_last;
						}
					}

					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_LEFT_SCROLL) || InputManager.ButtonPressed(i, MINECRAFT_ACTION_RIGHT_SCROLL))
					{
						app.SetOpacityTimer(i);
					}
				}
				else
				{
#ifndef _DURANGO
#ifdef _WINDOWS64
					// The 4J toggle system is unreliable here; bypass it with raw XInput and own edge detection.
					// A latch counter keeps startJustPressed active for ~120 frames after the rising edge.
					static WORD s_prevXButtons[XUSER_MAX_COUNT] = {};
					static int  s_startPressLatch[XUSER_MAX_COUNT] = {};
					XINPUT_STATE xstate_join;
					memset(&xstate_join, 0, sizeof(xstate_join));
					WORD xCurButtons = 0;
					if (XInputGetState(i, &xstate_join) == ERROR_SUCCESS)
					{
						xCurButtons = xstate_join.Gamepad.wButtons;
						if ((xCurButtons & XINPUT_GAMEPAD_START) != 0 && (s_prevXButtons[i] & XINPUT_GAMEPAD_START) == 0)
							s_startPressLatch[i] = 120;
						else if (s_startPressLatch[i] > 0)
							s_startPressLatch[i]--;
						s_prevXButtons[i] = xCurButtons;
					}
					bool startJustPressed = s_startPressLatch[i] > 0;
					bool tryJoin = !pause && !ui.IsIgnorePlayerJoinMenuDisplayed(ProfileManager.GetPrimaryPad()) && g_NetworkManager.SessionHasSpace() && xCurButtons != 0;
#else
					bool tryJoin = !pause && !ui.IsIgnorePlayerJoinMenuDisplayed(ProfileManager.GetPrimaryPad()) && g_NetworkManager.SessionHasSpace() && RenderManager.IsHiDef() && InputManager.ButtonPressed(i);
#endif
#ifdef __ORBIS__
					tryJoin = tryJoin && InputManager.IsLocalMultiplayerAvailable();

					if( !g_NetworkManager.IsLocalGame() )
					{
						tryJoin = tryJoin && ProfileManager.GetChatAndContentRestrictions(i,true,nullptr,nullptr,nullptr);
					}
#endif
					if(tryJoin)
					{
						if(!ui.PressStartPlaying(i))
						{
#ifdef __ORBIS__
							if (g_NetworkManager.IsLocalGame() || !ProfileManager.RequestingPlaystationPlus(i))
#endif
							{
								ui.ShowPressStart(i);
							}
						}
						else
						{
#ifdef __ORBIS__
							if(InputManager.ButtonPressed(i, ACTION_MENU_A))
#elif defined _WINDOWS64
							if(startJustPressed)
#else
							if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_PAUSEMENU))
#endif
							{
#ifdef _WINDOWS64
								if(ProfileManager.IsSignedIn(i) || (g_NetworkManager.IsLocalGame() && InputManager.IsPadConnected(i)))
#else
								if(ProfileManager.IsSignedIn(i))
#endif
								{
									if( g_NetworkManager.IsLocalGame() || (ProfileManager.IsSignedInLive(i) && ProfileManager.AllowedToPlayMultiplayer(i) ) )
									{
#ifdef __ORBIS__
										bool contentRestricted = false;
										ProfileManager.GetChatAndContentRestrictions(i,false,nullptr,&contentRestricted,nullptr);

										if (!g_NetworkManager.IsLocalGame() && contentRestricted)
										{
											ui.RequestContentRestrictedMessageBox(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_CONTENT_RESTRICTION, i);
										}
										else if(!g_NetworkManager.IsLocalGame() && !ProfileManager.HasPlayStationPlus(i))
										{
											m_pPsPlusUpsell = new PsPlusUpsellWrapper(i);
											m_pPsPlusUpsell->displayUpsell();
										}
										else
#endif
										if( level->isClientSide )
										{
											bool success=addLocalPlayer(i);

											if(!success)
											{
												app.DebugPrintf("Bringing up the sign in ui\n");
												ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,i);
											}
											else
											{
#ifdef __ORBIS__
												if(g_NetworkManager.IsLocalGame() == false)
												{
													bool chatRestricted = false;
													ProfileManager.GetChatAndContentRestrictions(i,false,&chatRestricted,nullptr,nullptr);
													if(chatRestricted)
													{
														ProfileManager.DisplaySystemMessage( SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_PSN_CHAT_RESTRICTION, i );
													}
												}
#endif
											}
										}
										else
										{
											shared_ptr<Player> player = localplayers[i];
											if( player == nullptr)
											{
												player = createExtraLocalPlayer(i, (convStringToWstring( ProfileManager.GetGamertag(i) )).c_str(), i, level->dimension->id);
											}
										}
									}
									else
									{
										if( ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && !ProfileManager.AllowedToPlayMultiplayer(i) )
										{
											ProfileManager.RequestConvertOfflineToGuestUI( &Minecraft::InGame_SignInReturned, this,i);

#ifndef _XBOX
											ui.HidePressStart();
#endif

#ifdef __ORBIS__
											int npAvailability = ProfileManager.getNPAvailability(i);

											if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
											{
												UINT uiIDA[1];
												uiIDA[0] = IDS_OK;
												ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, i);
											}
											else if (ProfileManager.IsSignedIn(i) && !ProfileManager.IsSignedInLive(i))
											{
												UINT uiIDA[2];
												uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
												uiIDA[1] = IDS_CANCEL;
												ui.RequestAlertMessage(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 2, i,&Minecraft::MustSignInReturnedPSN, this);
											}
											else
#endif
											{
												UINT uiIDA[1];
												uiIDA[0]=IDS_CONFIRM_OK;
												ui.RequestErrorMessage(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA, 1, i);
											}
										}
										app.DebugPrintf("Bringing up the sign in ui\n");
										ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,i);
									}
								}
								else
								{
									app.DebugPrintf("Bringing up the sign in ui\n");
									ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,i);
								}
							}
						}
					}
#endif // _DURANGO
				}
			}

#ifdef _DURANGO
			if(!pause && !ui.IsIgnorePlayerJoinMenuDisplayed(ProfileManager.GetPrimaryPad()) && g_NetworkManager.SessionHasSpace() && RenderManager.IsHiDef() )
			{
				int firstEmptyUser = 0;
				for( int i = 0; i < XUSER_MAX_COUNT; i++ )
				{
					if(localplayers[i] == nullptr)
					{
						firstEmptyUser = i;
						break;
					}
				}

				for(unsigned int iPad = XUSER_MAX_COUNT; iPad < (XUSER_MAX_COUNT + InputManager.MAX_GAMEPADS); ++iPad)
				{
					bool isPadLocked = InputManager.IsPadLocked(iPad), isPadConnected = InputManager.IsPadConnected(iPad), buttonPressed = InputManager.ButtonPressed(iPad);
					if (isPadLocked || !isPadConnected || !buttonPressed) continue;

					if(!ui.PressStartPlaying(firstEmptyUser))
					{
						ui.ShowPressStart(firstEmptyUser);
					}
					else
					{
						if(InputManager.ButtonPressed(iPad, MINECRAFT_ACTION_PAUSEMENU))
						{
							app.DebugPrintf("Bringing up the sign in ui\n");
							ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,iPad);
							break;
						}
					}
				}
			}
#endif

			if (pause && level != nullptr)
			{
				float lastA = timer->a;
				timer->advanceTime();
				timer->a = lastA;
			}
			else
			{
				timer->advanceTime();
			}

			for (int i = 0; i < timer->ticks; i++)
			{
				bool bLastTimerTick = ( i == ( timer->ticks - 1 ) );
				// When the tick runs more than once, re-tick the input manager so a key
				// press+release within the same frame isn't seen twice.
				if(i!=0)
				{
					InputManager.Tick();
					app.HandleButtonPresses();
				}

				ticks++;
				bool bFirst = true;
				for( int idx = 0; idx < XUSER_MAX_COUNT; idx++ )
				{
					if( m_pendingLocalConnections[idx] != nullptr )
					{
						m_pendingLocalConnections[idx]->tick();
					}

					if(localplayers[idx]!=nullptr)
					{
						if((localplayers[idx]->ullButtonsPressed!=0) || InputManager.GetJoypadStick_LX(idx,false)!=0.0f ||
							InputManager.GetJoypadStick_LY(idx,false)!=0.0f || InputManager.GetJoypadStick_RX(idx,false)!=0.0f ||
							InputManager.GetJoypadStick_RY(idx,false)!=0.0f )
						{
							localplayers[idx]->ResetInactiveTicks();
						}
						else
						{
							localplayers[idx]->IncrementInactiveTicks();
						}

						if(localplayers[idx]->GetInactiveTicks()>200)
						{
							if(!localplayers[idx]->isIdle() && localplayers[idx]->onGround)
							{
								localplayers[idx]->setIsIdle(true);
							}
						}
						else
						{
							if(localplayers[idx]->isIdle())
							{
								localplayers[idx]->setIsIdle(false);
							}
						}
					}

					if( setLocalPlayerIdx(idx) )
					{
						tick(bFirst, bLastTimerTick);
						bFirst = false;
						player->ullButtonsPressed=0LL;
					}
				}

				ui.HandleGameTick();

				setLocalPlayerIdx(ProfileManager.GetPrimaryPad());

				for( int l = 0; l < levels.length; l++ )
				{
					if( levels[l] )
					{
						levels[l]->animateTickDoWork();
					}
				}
			}

			MemSect(31);
			checkGlError(L"Pre render");
			MemSect(0);

			TileRenderer::fancy = options->fancyGraphics;

			PIXBeginNamedEvent(0,"Sound engine update");
			soundEngine->tick((shared_ptr<Mob> *)localplayers, timer->a);
			PIXEndNamedEvent();

			PIXBeginNamedEvent(0,"Light update");
			glEnable(GL_TEXTURE_2D);
			PIXEndNamedEvent();

			if (player != nullptr && player->isInWall()) player->SetThirdPersonView(0);

			if (!noRender)
			{
				bool bFirst = true;
				int iPrimaryPad=ProfileManager.GetPrimaryPad();
				for( int i = 0; i < XUSER_MAX_COUNT; i++ )
				{
					if( setLocalPlayerIdx(i) )
					{
						PIXBeginNamedEvent(0,"Game render player idx %d",i);
						RenderManager.StateSetViewport(static_cast<C4JRender::eViewportType>(player->m_iScreenSection));
						gameRenderer->render(timer->a, bFirst);
						bFirst = false;
						PIXEndNamedEvent();

						if(i==iPrimaryPad)
						{
#ifdef __ORBIS__
							RenderManager.InternalScreenCapture();
#endif
							switch(app.GetXuiAction(i))
							{
							case eAppAction_ExitWorldCapturedThumbnail:
							case eAppAction_SaveGameCapturedThumbnail:
							case eAppAction_AutosaveSaveGameCapturedThumbnail:
								app.CaptureSaveThumbnail();
								break;
							}
						}
					}
				}
				if( unoccupiedQuadrant > -1 )
				{
					RenderManager.StateSetViewport(static_cast<C4JRender::eViewportType>(C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + unoccupiedQuadrant));
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					ui.SetEmptyQuadrantLogo(C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + unoccupiedQuadrant);
				}
				setLocalPlayerIdx(iPrimaryPad);
				RenderManager.StateSetViewport(C4JRender::VIEWPORT_TYPE_FULLSCREEN);

#ifdef _XBOX
				for( int i = 0; i < XUSER_MAX_COUNT; i++ )
				{
					if(app.GetXuiAction(i)==eAppAction_SocialPostScreenshot)
					{
						app.CaptureScreenshot(i);
					}
				}
#endif
			}
			glFlush();

#if PACKET_ENABLE_STAT_TRACKING
			Packet::updatePacketStatsPIX();
#endif

			if (options->renderDebug)
			{
#if DEBUG_RENDER_SHOWS_PACKETS
				Packet::renderAllPacketStats();
#else
				g_NetworkManager.renderQueueMeter();
#endif
			}
			else
			{
				lastTimer = System::nanoTime();
			}

			achievementPopup->render();

			PIXBeginNamedEvent(0,"Sleeping");
			Sleep(0);
			PIXEndNamedEvent();

			PIXBeginNamedEvent(0,"Display update");
			Display::update();
			PIXEndNamedEvent();

			MemSect(31);
			checkGlError(L"Post render");
			MemSect(0);
			frames++;
			pause = app.IsAppPaused();

#ifndef _CONTENT_PACKAGE
			while (System::nanoTime() >= lastTime + 1000000000)
			{
				MemSect(31);
				fpsString = std::to_wstring(frames) + L" fps (" + std::to_wstring(Chunk::updates) + L" chunk updates)";
				MemSect(0);
				Chunk::updates = 0;
				lastTime += 1000000000;
				frames = 0;
			}
#endif
		}
	}
	LeaveCriticalSection(&m_setLevelCS);
}

void Minecraft::run_end()
{
	destroy();
}

void Minecraft::emergencySave()
{
	levelRenderer->clear();
	AABB::clearPool();
	Vec3::clearPool();
	setLevel(nullptr);
}

void Minecraft::renderFpsMeter(int64_t tickTime)
{
	int nsPer60Fps = 1000000000l / 60;
	if (lastTimer == -1)
	{
		lastTimer = System::nanoTime();
	}
	int64_t now = System::nanoTime();
	Minecraft::tickTimes[(Minecraft::frameTimePos) & (Minecraft::frameTimes_length - 1)] = tickTime;
	Minecraft::frameTimes[(Minecraft::frameTimePos++) & (Minecraft::frameTimes_length - 1)] = now - lastTimer;
	lastTimer = now;

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glEnable(GL_COLOR_MATERIAL);
	glLoadIdentity();
	glOrtho(0, static_cast<float>(width), static_cast<float>(height), 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();
	t->begin(GL_QUADS);
	int hh1 = (int) (nsPer60Fps / 200000);
	t->color(0x20000000);
	t->vertex(static_cast<float>(0), static_cast<float>(height - hh1), static_cast<float>(0));
	t->vertex(static_cast<float>(0), static_cast<float>(height), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height - hh1), static_cast<float>(0));

	t->color(0x20200000);
	t->vertex(static_cast<float>(0), static_cast<float>(height - hh1 * 2), static_cast<float>(0));
	t->vertex(static_cast<float>(0), static_cast<float>(height - hh1), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height - hh1), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height - hh1 * 2), static_cast<float>(0));

	t->end();
	int64_t totalTime = 0;
	for (int i = 0; i < Minecraft::frameTimes_length; i++)
	{
		totalTime += Minecraft::frameTimes[i];
	}
	int hh = static_cast<int>(totalTime / 200000 / Minecraft::frameTimes_length);
	t->begin(GL_QUADS);
	t->color(0x20400000);
	t->vertex(static_cast<float>(0), static_cast<float>(height - hh), static_cast<float>(0));
	t->vertex(static_cast<float>(0), static_cast<float>(height), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height), static_cast<float>(0));
	t->vertex(static_cast<float>(Minecraft::frameTimes_length), static_cast<float>(height - hh), static_cast<float>(0));
	t->end();
	t->begin(GL_LINES);
	for (int i = 0; i < Minecraft::frameTimes_length; i++)
	{
		int col = ((i - Minecraft::frameTimePos) & (Minecraft::frameTimes_length - 1)) * 255 / Minecraft::frameTimes_length;
		int cc = col * col / 255;
		cc = cc * cc / 255;
		int cc2 = cc * cc / 255;
		cc2 = cc2 * cc2 / 255;
		if (Minecraft::frameTimes[i] > nsPer60Fps)
		{
			t->color(0xff000000 + cc * 65536);
		}
		else
		{
			t->color(0xff000000 + cc * 256);
		}

		int64_t time = Minecraft::frameTimes[i] / 200000;
		int64_t time2 = Minecraft::tickTimes[i] / 200000;

		t->vertex((float)(i + 0.5f), (float)( height - time + 0.5f), static_cast<float>(0));
		t->vertex((float)(i + 0.5f), (float)( height + 0.5f), static_cast<float>(0));

		t->color(0xff000000 + cc * 65536 + cc * 256 + cc * 1);
		t->vertex((float)(i + 0.5f), (float)( height - time + 0.5f), static_cast<float>(0));
		t->vertex((float)(i + 0.5f), (float)( height - (time - time2) + 0.5f), static_cast<float>(0));
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}

void Minecraft::stop()
{
	running = false;
}

void Minecraft::pauseGame()
{
	if (screen != nullptr) return;
}

void Minecraft::resize(int width, int height)
{
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;
	this->width = width;
	this->height = height;

	if (screen != nullptr)
	{
		ScreenSizeCalculator ssc(options, width, height);
		int screenWidth = ssc.getWidth();
		int screenHeight = ssc.getHeight();
	}
}

void Minecraft::verify()
{
	// TODO: verify session with minecraft.net
}

void Minecraft::levelTickUpdateFunc(void* pParam)
{
	Level* pLevel = static_cast<Level *>(pParam);
	pLevel->tick();
}

void Minecraft::levelTickThreadInitFunc()
{
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::UseDefaultThreadStorage();
}

void Minecraft::tick(bool bFirst, bool bUpdateTextures)
{
	int iPad=player->GetXboxPad();

	stats[iPad]->tick(iPad);

	app.TickOpacityTimer(iPad);

	if( bFirst ) levelRenderer->destroyedTileManager->tick();

	gui->tick();
	gameRenderer->pick(1);

	if (!pause && level != nullptr) gameMode->tick();
	MemSect(31);
	glBindTexture(GL_TEXTURE_2D, textures->loadTexture(TN_TERRAIN));
	MemSect(0);
	if( bFirst )
	{
		PIXBeginNamedEvent(0,"Texture tick");
		if (!pause) textures->tick(bUpdateTextures);
		PIXEndNamedEvent();
	}

	if (screen == nullptr && player != nullptr )
	{
		if (player->getHealth() <= 0 && !ui.GetMenuDisplayed(iPad) )
		{
			setScreen(nullptr);
		}
		else if (player->isSleeping() && level != nullptr && level->isClientSide)
		{
			// TODO: setScreen(new InBedChatScreen());
		}
	}
	else if (screen != nullptr && (dynamic_cast<InBedChatScreen *>(screen)!=nullptr) && !player->isSleeping())
	{
		setScreen(nullptr);
	}

	if (screen != nullptr)
	{
		player->missTime = 10000;
		player->lastClickTick[0] = ticks + 10000;
		player->lastClickTick[1] = ticks + 10000;
	}

	if (screen != nullptr)
	{
		screen->updateEvents();
		if (screen != nullptr)
		{
			screen->particles->tick();
			screen->tick();
		}
	}

#ifdef _WINDOWS64
	// Mouse grab/release only for the primary (KBM) player; splitscreen players
	// use controllers and must never fight over the cursor state.
	if (iPad == ProfileManager.GetPrimaryPad())
	{
		if ((screen != nullptr || ui.GetMenuDisplayed(iPad)) && g_KBMInput.IsMouseGrabbed())
		{
			g_KBMInput.SetMouseGrabbed(false);
		}
	}
#endif

	if (screen == nullptr && !ui.GetMenuDisplayed(iPad) )
	{
#ifdef _WINDOWS64
		if (iPad == ProfileManager.GetPrimaryPad() && !g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsWindowFocused())
		{
			g_KBMInput.SetMouseGrabbed(true);
		}
#endif
		int iA=-1, iB=-1, iX, iY=IDS_CONTROLS_INVENTORY, iLT=-1, iRT=-1, iLB=-1, iRB=-1, iLS=-1, iRS=-1;

		if(player->abilities.instabuild)
		{
			iX=IDS_TOOLTIPS_CREATIVE;
		}
		else
		{
			iX=IDS_CONTROLS_CRAFTING;
		}

		int *piAction;
		int *piJump;
		int *piUse;
		int *piAlt;

		unsigned int uiAction = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_ACTION );
		unsigned int uiJump = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_JUMP );
		unsigned int uiUse = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_USE );
		unsigned int uiAlt = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_SNEAK_TOGGLE );

		switch(uiAction)
		{
		case _360_JOY_BUTTON_RT:	piAction=&iRT;	break;
		case _360_JOY_BUTTON_LT:	piAction=&iLT;	break;
		case _360_JOY_BUTTON_LB:	piAction=&iLB;	break;
		case _360_JOY_BUTTON_RB:	piAction=&iRB;	break;
		case _360_JOY_BUTTON_A:
		default:					piAction=&iA;	break;
		}

		switch(uiJump)
		{
		case _360_JOY_BUTTON_LT:	piJump=&iLT;	break;
		case _360_JOY_BUTTON_RT:	piJump=&iRT;	break;
		case _360_JOY_BUTTON_LB:	piJump=&iLB;	break;
		case _360_JOY_BUTTON_RB:	piJump=&iRB;	break;
		case _360_JOY_BUTTON_A:
		default:					piJump=&iA;		break;
		}

		switch(uiUse)
		{
		case _360_JOY_BUTTON_LB:	piUse=&iLB;		break;
		case _360_JOY_BUTTON_RB:	piUse=&iRB;		break;
		case _360_JOY_BUTTON_LT:	piUse=&iLT;		break;
		case _360_JOY_BUTTON_RT:
		default:					piUse=&iRT;		break;
		}

		switch(uiAlt)
		{
		default:
		case _360_JOY_BUTTON_LSTICK_RIGHT:
			piAlt=&iRS;
			break;
		}

		if (player->isUnderLiquid(Material::water))
		{
			*piJump=IDS_TOOLTIPS_SWIMUP;
		}
		else
		{
			*piJump=-1;
		}

		*piUse=-1;
		*piAction=-1;
		*piAlt=-1;

		if (player->isSleeping() && (level != nullptr) && level->isClientSide)
		{
			*piUse=IDS_TOOLTIPS_WAKEUP;
		}
		else
		{
			if (player->isRiding())
			{
				shared_ptr<Entity> mount = player->riding;

				if ( mount->instanceof(eTYPE_MINECART) || mount->instanceof(eTYPE_BOAT) )
				{
					*piAlt = IDS_TOOLTIPS_EXIT;
				}
				else
				{
					*piAlt = IDS_TOOLTIPS_DISMOUNT;
				}
			}

			shared_ptr<ItemInstance> itemInstance = player->inventory->getSelected();

			if (itemInstance)
			{
				bool bUseItem = gameMode->useItem(player, level, itemInstance, true);

				switch (itemInstance->getItem()->id)
				{
				case Item::potatoBaked_Id:
				case Item::potato_Id:
				case Item::pumpkinPie_Id:
				case Item::potatoPoisonous_Id:
				case Item::carrotGolden_Id:
				case Item::carrots_Id:
				case Item::mushroomStew_Id:
				case Item::apple_Id:
				case Item::bread_Id:
				case Item::porkChop_raw_Id:
				case Item::porkChop_cooked_Id:
				case Item::apple_gold_Id:
				case Item::fish_raw_Id:
				case Item::fish_cooked_Id:
				case Item::cookie_Id:
				case Item::beef_cooked_Id:
				case Item::beef_raw_Id:
				case Item::chicken_cooked_Id:
				case Item::chicken_raw_Id:
				case Item::melon_Id:
				case Item::rotten_flesh_Id:
				case Item::spiderEye_Id:
					{
						FoodItem *food = static_cast<FoodItem *>(itemInstance->getItem());
						if (food != nullptr && food->canEat(player))
						{
							*piUse=IDS_TOOLTIPS_EAT;
						}
					}
					break;

				case Item::bucket_milk_Id:
					*piUse=IDS_TOOLTIPS_DRINK;
					break;

				case Item::fishingRod_Id:
				case Item::emptyMap_Id:
					*piUse=IDS_TOOLTIPS_USE;
					break;

				case Item::egg_Id:
				case Item::snowBall_Id:
					*piUse=IDS_TOOLTIPS_THROW;
					break;

				case Item::bow_Id:
					if ( player->abilities.instabuild || player->inventory->hasResource(Item::arrow_Id) )
					{
						if (player->isUsingItem())	*piUse=IDS_TOOLTIPS_RELEASE_BOW;
						else						*piUse=IDS_TOOLTIPS_DRAW_BOW;
					}
					break;

				case Item::sword_wood_Id:
				case Item::sword_stone_Id:
				case Item::sword_iron_Id:
				case Item::sword_diamond_Id:
				case Item::sword_gold_Id:
					*piUse=IDS_TOOLTIPS_BLOCK;
					break;

				case Item::bucket_empty_Id:
				case Item::glassBottle_Id:
					if (bUseItem) *piUse=IDS_TOOLTIPS_COLLECT;
					break;

				case Item::bucket_lava_Id:
				case Item::bucket_water_Id:
					*piUse=IDS_TOOLTIPS_EMPTY;
					break;

				case Item::boat_Id:
				case Tile::waterLily_Id:
					if (bUseItem) *piUse=IDS_TOOLTIPS_PLACE;
					break;

				case Item::potion_Id:
					if (bUseItem)
					{
						if (MACRO_POTION_IS_SPLASH(itemInstance->getAuxValue()))	*piUse=IDS_TOOLTIPS_THROW;
						else														*piUse=IDS_TOOLTIPS_DRINK;
					}
					break;

				case Item::enderPearl_Id:
					if (bUseItem) *piUse=IDS_TOOLTIPS_THROW;
					break;

				case Item::eyeOfEnder_Id:
					if ( bUseItem && (level->dimension->id==0) && level->getLevelData()->getHasStronghold() )
					{
						*piUse=IDS_TOOLTIPS_THROW;
					}
					break;

				case Item::expBottle_Id:
					if (bUseItem) *piUse=IDS_TOOLTIPS_THROW;
					break;
				}
			}

			if (hitResult!=nullptr)
			{
				switch(hitResult->type)
				{
				case HitResult::TILE:
					{
						int x,y,z;
						x=hitResult->x;
						y=hitResult->y;
						z=hitResult->z;
						int face = hitResult->f;

						int iTileID=level->getTile(x,y ,z );
						int iData = level->getData(x, y, z);

						if( gameMode != nullptr && gameMode->getTutorial() != nullptr )
						{
							gameMode->getTutorial()->onLookAt(iTileID,iData);
						}

						bool bUseItemOn=gameMode->useItemOn(player, level, itemInstance, x, y, z, face, hitResult->pos, true);

						if (bUseItemOn && itemInstance!=nullptr)
						{
							switch (itemInstance->getItem()->id)
							{
							case Tile::mushroom_brown_Id:
							case Tile::mushroom_red_Id:
							case Tile::tallgrass_Id:
							case Tile::cactus_Id:
							case Tile::sapling_Id:
							case Tile::reeds_Id:
							case Tile::flower_Id:
							case Tile::rose_Id:
								*piUse=IDS_TOOLTIPS_PLANT;
								break;

							case Item::hoe_wood_Id:
							case Item::hoe_stone_Id:
							case Item::hoe_iron_Id:
							case Item::hoe_diamond_Id:
							case Item::hoe_gold_Id:
								*piUse=IDS_TOOLTIPS_TILL;
								break;

							case Item::seeds_wheat_Id:
							case Item::netherwart_seeds_Id:
								*piUse=IDS_TOOLTIPS_PLANT;
								break;

							case Item::dye_powder_Id:
								if (itemInstance->getAuxValue() == DyePowderItem::WHITE)
								{
									switch(iTileID)
									{
									case Tile::sapling_Id:
									case Tile::wheat_Id:
									case Tile::grass_Id:
									case Tile::mushroom_brown_Id:
									case Tile::mushroom_red_Id:
									case Tile::melonStem_Id:
									case Tile::pumpkinStem_Id:
									case Tile::carrots_Id:
									case Tile::potatoes_Id:
										*piUse=IDS_TOOLTIPS_GROW;
										break;
									}
								}
								break;

							case Item::painting_Id:
								*piUse=IDS_TOOLTIPS_HANG;
								break;

							case Item::flintAndSteel_Id:
							case Item::fireball_Id:
								*piUse=IDS_TOOLTIPS_IGNITE;
								break;

							case Item::fireworks_Id:
								*piUse=IDS_TOOLTIPS_FIREWORK_LAUNCH;
								break;

							case Item::lead_Id:
								*piUse=IDS_TOOLTIPS_ATTACH;
								break;

							default:
								*piUse=IDS_TOOLTIPS_PLACE;
								break;
							}
						}

						switch(iTileID)
						{
						case Tile::anvil_Id:
						case Tile::enchantTable_Id:
						case Tile::brewingStand_Id:
						case Tile::workBench_Id:
						case Tile::furnace_Id:
						case Tile::furnace_lit_Id:
						case Tile::door_wood_Id:
						case Tile::dispenser_Id:
						case Tile::lever_Id:
						case Tile::button_stone_Id:
						case Tile::button_wood_Id:
						case Tile::trapdoor_Id:
						case Tile::fenceGate_Id:
						case Tile::beacon_Id:
							*piAction=IDS_TOOLTIPS_MINE;
							*piUse=IDS_TOOLTIPS_USE;
							break;

						case Tile::chest_Id:
							*piAction = IDS_TOOLTIPS_MINE;
							*piUse = (Tile::chest->getContainer(level,x,y,z) != nullptr) ? IDS_TOOLTIPS_OPEN : -1;
							break;

						case Tile::enderChest_Id:
						case Tile::chest_trap_Id:
						case Tile::dropper_Id:
						case Tile::hopper_Id:
							*piUse=IDS_TOOLTIPS_OPEN;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::activatorRail_Id:
						case Tile::goldenRail_Id:
						case Tile::detectorRail_Id:
						case Tile::rail_Id:
							if (bUseItemOn) *piUse=IDS_TOOLTIPS_PLACE;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::bed_Id:
							if (bUseItemOn)	*piUse=IDS_TOOLTIPS_SLEEP;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::noteblock_Id:
							if (player->abilities.instabuild)	*piAction=IDS_TOOLTIPS_MINE;
							else								*piAction=IDS_TOOLTIPS_PLAY;
							*piUse=IDS_TOOLTIPS_CHANGEPITCH;
							break;

						case Tile::sign_Id:
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::cauldron_Id:
							if (itemInstance)
							{
								int iID=itemInstance->getItem()->id;
								int currentData = level->getData(x, y, z);
								if ((iID==Item::glassBottle_Id) && (currentData > 0))
								{
									*piUse=IDS_TOOLTIPS_COLLECT;
								}
							}
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::cake_Id:
							if (player->abilities.instabuild)
							{
								*piAction=IDS_TOOLTIPS_MINE;
							}
							else
							{
								if (player->getFoodData()->needsFood() )
								{
									*piAction=IDS_TOOLTIPS_EAT;
									*piUse=IDS_TOOLTIPS_EAT;
								}
								else
								{
									*piAction=IDS_TOOLTIPS_MINE;
								}
							}
							break;

						case Tile::jukebox_Id:
							if (!bUseItemOn && itemInstance!=nullptr)
							{
								int iID=itemInstance->getItem()->id;
								if ( (iID>=Item::record_01_Id) && (iID<=Item::record_12_Id) )
								{
									*piUse=IDS_TOOLTIPS_PLAY;
								}
								*piAction=IDS_TOOLTIPS_MINE;
							}
							else
							{
								if (Tile::jukebox->TestUse(level, x, y, z, player))
								{
									*piUse=IDS_TOOLTIPS_EJECT;
								}
								*piAction=IDS_TOOLTIPS_MINE;
							}
							break;

						case Tile::flowerPot_Id:
							if ( !bUseItemOn && (itemInstance != nullptr) && (iData == 0) )
							{
								int iID = itemInstance->getItem()->id;
								if (iID<256)
								{
									switch(iID)
									{
									case Tile::flower_Id:
									case Tile::rose_Id:
									case Tile::sapling_Id:
									case Tile::mushroom_brown_Id:
									case Tile::mushroom_red_Id:
									case Tile::cactus_Id:
									case Tile::deadBush_Id:
										*piUse=IDS_TOOLTIPS_PLANT;
										break;

									case Tile::tallgrass_Id:
										if (itemInstance->getAuxValue() != TallGrass::TALL_GRASS) *piUse=IDS_TOOLTIPS_PLANT;
										break;
									}
								}
							}
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::comparator_off_Id:
						case Tile::comparator_on_Id:
							*piUse=IDS_TOOLTIPS_USE;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::diode_off_Id:
						case Tile::diode_on_Id:
							*piUse=IDS_TOOLTIPS_USE;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::redStoneOre_Id:
							if (bUseItemOn)	*piUse=IDS_TOOLTIPS_USE;
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::door_iron_Id:
							if(*piUse==IDS_TOOLTIPS_PLACE)
							{
								*piUse = -1;
							}
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						default:
							*piAction=IDS_TOOLTIPS_MINE;
							break;
						}
					}
					break;

				case HitResult::ENTITY:
					eINSTANCEOF entityType = hitResult->entity->GetType();

					if ( (gameMode != nullptr) && (gameMode->getTutorial() != nullptr) )
					{
						gameMode->getTutorial()->onLookAtEntity(hitResult->entity);
					}

					shared_ptr<ItemInstance> heldItem = nullptr;
					if (player->inventory->IsHeldItem())
					{
						heldItem = player->inventory->getSelected();
					}
					int heldItemId = heldItem != nullptr ? heldItem->getItem()->id : -1;

					switch(entityType)
					{
					case eTYPE_CHICKEN:
						{
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Animal> animal = dynamic_pointer_cast<Animal>(hitResult->entity);

							if (animal->isLeashed() && animal->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

							switch(heldItemId)
							{
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;

							case Item::lead_Id:
								if (!animal->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;

							default:
								{
									if(!animal->isBaby() && !animal->isInLove() && (animal->getAge() == 0) && animal->isFood(heldItem))
									{
										*piUse=IDS_TOOLTIPS_LOVEMODE;
									}
								}
								break;

							case -1: break;
							}
						}
						break;

					case eTYPE_COW:
						{
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Animal> animal = dynamic_pointer_cast<Animal>(hitResult->entity);

							if (animal->isLeashed() && animal->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

							switch (heldItemId)
							{
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;
							case Item::lead_Id:
								if (!animal->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;
							case Item::bucket_empty_Id:
								*piUse=IDS_TOOLTIPS_MILK;
								break;
							default:
								{
									if(!animal->isBaby() && !animal->isInLove() && (animal->getAge() == 0) && animal->isFood(heldItem))
									{
										*piUse=IDS_TOOLTIPS_LOVEMODE;
									}
								}
								break;

							case -1: break;
							}
						}
						break;

					case eTYPE_MUSHROOMCOW:
						{
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Animal> animal = dynamic_pointer_cast<Animal>(hitResult->entity);

							if (animal->isLeashed() && animal->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

							switch(heldItemId)
							{
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;

							case Item::lead_Id:
								if (!animal->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;

							case Item::bowl_Id:
							case Item::bucket_empty_Id:
								*piUse=IDS_TOOLTIPS_MILK;
								break;
							case Item::shears_Id:
								{
									if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;
									if(!animal->isBaby()) *piUse=IDS_TOOLTIPS_SHEAR;
								}
								break;
							default:
								{
									if(!animal->isBaby() && !animal->isInLove() && (animal->getAge() == 0) && animal->isFood(heldItem))
									{
										*piUse=IDS_TOOLTIPS_LOVEMODE;
									}
								}
								break;

							case -1: break;
							}
						}
						break;

					case eTYPE_BOAT:
						*piAction=IDS_TOOLTIPS_MINE;
						*piUse=IDS_TOOLTIPS_SAIL;
						break;

					case eTYPE_MINECART_RIDEABLE:
						*piAction = IDS_TOOLTIPS_MINE;
						*piUse = IDS_TOOLTIPS_RIDE;
						break;

					case eTYPE_MINECART_FURNACE:
						*piAction = IDS_TOOLTIPS_MINE;
						if (heldItemId == Item::coal_Id) *piUse=IDS_TOOLTIPS_USE;
						break;

					case eTYPE_MINECART_CHEST:
					case eTYPE_MINECART_HOPPER:
						*piAction = IDS_TOOLTIPS_MINE;
						*piUse = IDS_TOOLTIPS_OPEN;
						break;

					case eTYPE_MINECART_SPAWNER:
					case eTYPE_MINECART_TNT:
						*piUse = IDS_TOOLTIPS_MINE;
						break;

					case eTYPE_SHEEP:
						{
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Sheep> sheep = dynamic_pointer_cast<Sheep>(hitResult->entity);

							if (sheep->isLeashed() && sheep->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

							switch(heldItemId)
							{
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;

							case Item::lead_Id:
								if (!sheep->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;

							case Item::dye_powder_Id:
								{
									int newColor = ColoredTile::getTileDataForItemAuxValue(heldItem->getAuxValue());
									if(!(sheep->isSheared() && sheep->getColor() != newColor))
									{
										*piUse=IDS_TOOLTIPS_DYE;
									}
								}
								break;
							case Item::shears_Id:
								{
									if ( !sheep->isBaby() && !sheep->isSheared() )
									{
										*piUse=IDS_TOOLTIPS_SHEAR;
									}
								}
								break;
							default:
								{
									if(!sheep->isBaby() && !sheep->isInLove() && (sheep->getAge() == 0) && sheep->isFood(heldItem))
									{
										*piUse=IDS_TOOLTIPS_LOVEMODE;
									}
								}
								break;

							case -1: break;
							}
						}
						break;

					case eTYPE_PIG:
						{
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Pig> pig = dynamic_pointer_cast<Pig>(hitResult->entity);

							if (pig->isLeashed() && pig->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
							}
							else if (heldItemId == Item::lead_Id)
							{
								if (!pig->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
							}
							else if (heldItemId == Item::nameTag_Id)
							{
								*piUse = IDS_TOOLTIPS_NAME;
							}
							else if (pig->hasSaddle())
							{
								*piUse=IDS_TOOLTIPS_MOUNT;
							}
							else if (!pig->isBaby())
							{
								if(player->inventory->IsHeldItem())
								{
									switch(heldItemId)
									{
									case Item::saddle_Id:
										*piUse=IDS_TOOLTIPS_SADDLE;
										break;

									default:
										{
											if (!pig->isInLove() && (pig->getAge() == 0) && pig->isFood(heldItem))
											{
												*piUse=IDS_TOOLTIPS_LOVEMODE;
											}
										}
										break;
									}
								}
							}
						}
						break;

					case eTYPE_WOLF:
						{
							shared_ptr<Wolf> wolf = dynamic_pointer_cast<Wolf>(hitResult->entity);

							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							if (wolf->isLeashed() && wolf->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

							switch(heldItemId)
							{
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;

							case Item::lead_Id:
								if (!wolf->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;

							case Item::bone_Id:
								if (!wolf->isAngry() && !wolf->isTame())
								{
									*piUse=IDS_TOOLTIPS_TAME;
								}
								else if (equalsIgnoreCase(player->getUUID(), wolf->getOwnerUUID()))
								{
									if(wolf->isSitting())	*piUse=IDS_TOOLTIPS_FOLLOWME;
									else					*piUse=IDS_TOOLTIPS_SIT;
								}
								break;
							case Item::enderPearl_Id:
								break;
							case Item::dye_powder_Id:
								if (wolf->isTame())
								{
									if (ColoredTile::getTileDataForItemAuxValue(heldItem->getAuxValue()) != wolf->getCollarColor())
									{
										*piUse=IDS_TOOLTIPS_DYECOLLAR;
									}
									else if (wolf->isSitting())	*piUse=IDS_TOOLTIPS_FOLLOWME;
									else						*piUse=IDS_TOOLTIPS_SIT;
								}
								break;
							default:
								if(wolf->isTame())
								{
									if(wolf->isFood(heldItem))
									{
										if(wolf->GetSynchedHealth() < wolf->getMaxHealth())
										{
											*piUse=IDS_TOOLTIPS_HEAL;
										}
										else
										{
											if(!wolf->isBaby() && !wolf->isInLove() && (wolf->getAge() == 0))
											{
												*piUse=IDS_TOOLTIPS_LOVEMODE;
											}
										}
										break;
									}

									if (equalsIgnoreCase(player->getUUID(), wolf->getOwnerUUID()))
									{
										if(wolf->isSitting())	*piUse=IDS_TOOLTIPS_FOLLOWME;
										else					*piUse=IDS_TOOLTIPS_SIT;
									}
								}
								break;
							}
						}
						break;

					case eTYPE_OCELOT:
						{
							shared_ptr<Ocelot> ocelot = dynamic_pointer_cast<Ocelot>(hitResult->entity);

							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							if (ocelot->isLeashed() && ocelot->getLeashHolder() == player)
							{
								*piUse = IDS_TOOLTIPS_UNLEASH;
							}
							else if (heldItemId == Item::lead_Id)
							{
								if (!ocelot->isLeashed()) *piUse = IDS_TOOLTIPS_LEASH;
							}
							else if (heldItemId == Item::nameTag_Id)
							{
								*piUse = IDS_TOOLTIPS_NAME;
							}
							else if(ocelot->isTame())
							{
								if(ocelot->isFood(heldItem))
								{
									if(!ocelot->isBaby())
									{
										if(!ocelot->isInLove())
										{
											if(ocelot->getAge() == 0)
											{
												*piUse=IDS_TOOLTIPS_LOVEMODE;
											}
										}
										else
										{
											*piUse=IDS_TOOLTIPS_FEED;
										}
									}
								}
								else if (equalsIgnoreCase(player->getUUID(), ocelot->getOwnerUUID()) && !ocelot->isSittingOnTile() )
								{
									if(ocelot->isSitting())	*piUse=IDS_TOOLTIPS_FOLLOWME;
									else					*piUse=IDS_TOOLTIPS_SIT;
								}
							}
							else if(heldItemId >= 0)
							{
								if (ocelot->isFood(heldItem)) *piUse=IDS_TOOLTIPS_TAME;
							}
						}
						break;

					case eTYPE_PLAYER:
						{
							shared_ptr<Player> TargetPlayer = dynamic_pointer_cast<Player>(hitResult->entity);

							if(!TargetPlayer->hasInvisiblePrivilege())
							{
								if( app.GetGameHostOption(eGameHostOption_PvP) && player->isAllowedToAttackPlayers())
								{
									*piAction=IDS_TOOLTIPS_HIT;
								}
							}
						}
						break;

					case eTYPE_ITEM_FRAME:
						{
							shared_ptr<ItemFrame> itemFrame = dynamic_pointer_cast<ItemFrame>(hitResult->entity);

							if(itemFrame->getItem()!=nullptr)
							{
								*piUse=IDS_TOOLTIPS_ROTATE;
							}
							else
							{
								if(heldItemId >= 0) *piUse=IDS_TOOLTIPS_PLACE;
							}

							*piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_VILLAGER:
						{
							shared_ptr<Villager> villager = dynamic_pointer_cast<Villager>(hitResult->entity);
							if (!villager->isBaby())
							{
								*piUse=IDS_TOOLTIPS_TRADE;
							}
							*piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_ZOMBIE:
						{
							shared_ptr<Zombie> zomb = dynamic_pointer_cast<Zombie>(hitResult->entity);
							static GoldenAppleItem *goldapple = static_cast<GoldenAppleItem *>(Item::apple_gold);

							if ( zomb->isVillager() && zomb->isWeakened() && (heldItemId == Item::apple_gold_Id) && !goldapple->isFoil(heldItem) )
							{
								*piUse=IDS_TOOLTIPS_CURE;
							}
							*piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_HORSE:
						{
							shared_ptr<EntityHorse> horse = dynamic_pointer_cast<EntityHorse>(hitResult->entity);

							bool heldItemIsFood = false, heldItemIsLove = false, heldItemIsArmour = false;

							switch( heldItemId )
							{
								case Item::wheat_Id:
								case Item::sugar_Id:
								case Item::bread_Id:
								case Tile::hayBlock_Id:
								case Item::apple_Id:
									heldItemIsFood = true;
									break;
								case Item::carrotGolden_Id:
								case Item::apple_gold_Id:
									heldItemIsLove = true;
									heldItemIsFood = true;
									break;
								case Item::horseArmorDiamond_Id:
								case Item::horseArmorGold_Id:
								case Item::horseArmorMetal_Id:
									heldItemIsArmour = true;
									break;
							}

							if (horse->isLeashed() && horse->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
							}
							else if ( heldItemId == Item::lead_Id)
							{
								if (!horse->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
							}
							else if (heldItemId == Item::nameTag_Id)
							{
								*piUse = IDS_TOOLTIPS_NAME;
							}
							else if (horse->isBaby())
							{
								if (heldItemIsFood)
								{
									*piUse = IDS_TOOLTIPS_FEED;
								}
							}
							else if ( !horse->isTamed() )
							{
								if (heldItemId == -1)
								{
									*piUse = IDS_TOOLTIPS_TAME;
								}
								else if (heldItemIsFood)
								{
									*piUse = IDS_TOOLTIPS_FEED;
								}
							}
							else if (	player->isSneaking()
									||	(heldItemId == Item::saddle_Id)
									||	(horse->canWearArmor() && heldItemIsArmour)
									)
							{
								if (*piUse == -1) *piUse = IDS_TOOLTIPS_OPEN;
							}
							else if (	horse->canWearBags()
									&&	!horse->isChestedHorse()
									&&	(heldItemId == Tile::chest_Id) )
							{
								*piUse = IDS_TOOLTIPS_ATTACH;
							}
							else if (	horse->isReadyForParenting()
									&&	heldItemIsLove )
							{
								*piUse = IDS_TOOLTIPS_LOVEMODE;
							}
							else if ( heldItemIsFood && (horse->getHealth() < horse->getMaxHealth()) )
							{
								*piUse = IDS_TOOLTIPS_HEAL;
							}
							else
							{
								*piUse = IDS_TOOLTIPS_MOUNT;
							}

							if (player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_ENDERDRAGON:
						*piAction = IDS_TOOLTIPS_HIT;
						break;

					case eTYPE_LEASHFENCEKNOT:
						*piAction = IDS_TOOLTIPS_UNLEASH;
						if (heldItemId == Item::lead_Id && LeashItem::bindPlayerMobsTest(player, level, player->x, player->y, player->z))
						{
							*piUse = IDS_TOOLTIPS_ATTACH;
						}
						else
						{
							*piUse = IDS_TOOLTIPS_UNLEASH;
						}
						break;

					default:
						if (  hitResult->entity->instanceof(eTYPE_MOB) )
						{
							shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(hitResult->entity);
							if (mob->isLeashed() && mob->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
							}
							else if (heldItemId == Item::lead_Id)
							{
								if (!mob->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
							}
							else if (heldItemId == Item::nameTag_Id)
							{
								*piUse=IDS_TOOLTIPS_NAME;
							}
						}
						*piAction=IDS_TOOLTIPS_HIT;
						break;
					}
					break;
				}
			}
		}

		if (!ui.IsReloadingSkin()) ui.SetTooltips( iPad, iA, iB, iX, iY, iLT, iRT, iLB, iRB, iLS, iRS);

		int wheel = 0;
		if (InputManager.GetValue(iPad, MINECRAFT_ACTION_LEFT_SCROLL, true) > 0 && gameMode->isInputAllowed(MINECRAFT_ACTION_LEFT_SCROLL) )
		{
			wheel = 1;
		}
		else if (InputManager.GetValue(iPad, MINECRAFT_ACTION_RIGHT_SCROLL,true) > 0 && gameMode->isInputAllowed(MINECRAFT_ACTION_RIGHT_SCROLL) )
		{
			wheel = -1;
		}

#ifdef _WINDOWS64
		if (iPad == 0 && wheel == 0 && g_KBMInput.IsKBMActive())
		{
			wheel = g_KBMInput.GetMouseWheel();
		}
#endif
		if (wheel != 0)
		{
			player->inventory->swapPaint(wheel);

			if( gameMode != nullptr && gameMode->getTutorial() != nullptr )
			{
				gameMode->getTutorial()->onSelectedItemChanged(player->inventory->getSelected());
			}

			player->updateRichPresence();

			if (options->isFlying)
			{
				if (wheel > 0) wheel = 1;
				if (wheel < 0) wheel = -1;

				options->flySpeed += wheel * .25f;
			}
		}

		if( gameMode->isInputAllowed(MINECRAFT_ACTION_ACTION) )
		{
			if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_ACTION)))
			{
				player->handleMouseClick(0);
				player->lastClickTick[0] = ticks;
			}

#ifdef _WINDOWS64
			bool actionPressed = InputManager.ButtonPressed(iPad, MINECRAFT_ACTION_ACTION) || (iPad == 0 && g_KBMInput.IsKBMActive() && g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT));
			bool actionHeld = InputManager.ButtonDown(iPad, MINECRAFT_ACTION_ACTION) || (iPad == 0 && g_KBMInput.IsKBMActive() && g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_LEFT));
#else
			bool actionPressed = InputManager.ButtonPressed(iPad, MINECRAFT_ACTION_ACTION);
			bool actionHeld = InputManager.ButtonDown(iPad, MINECRAFT_ACTION_ACTION);
#endif

			if (actionPressed)
			{
				player->handleMouseClick(0);
				player->lastClickTick[0] = ticks;
			}

			if(actionHeld)	player->handleMouseDown(0, true );
			else			player->handleMouseDown(0, false );
		}

#ifdef _WINDOWS64
		bool useHeld = InputManager.ButtonDown(iPad, MINECRAFT_ACTION_USE) || (iPad == 0 && g_KBMInput.IsKBMActive() && g_KBMInput.IsMouseButtonDown(KeyboardMouseInput::MOUSE_RIGHT));
#else
		bool useHeld = InputManager.ButtonDown(iPad, MINECRAFT_ACTION_USE);
#endif
		if( player->isUsingItem() )
		{
			if(!useHeld) gameMode->releaseUsingItem(player);
		}
		else if( gameMode->isInputAllowed(MINECRAFT_ACTION_USE) )
		{
			if( player->abilities.instabuild )
			{
				// Creative mode: attempt to handle click in special creative fashion (regular interval block placing)
				bool didClick = player->creativeModeHandleMouseClick(1, useHeld );
				if( player->lastClickState == LocalPlayer::lastClick_oldRepeat )
				{
					if( didClick )
					{
						player->lastClickTick[1] = ticks;
					}
					else
					{
						if (useHeld && ticks - player->lastClickTick[1] >= timer->ticksPerSecond / 4)
						{
							player->handleMouseClick(1);
							player->lastClickTick[1] = ticks;
						}
					}
				}
			}
			else
			{
				// Auto-repeat is disabled while riding or sprinting to avoid photo-sensitivity issues
				// when placing fire, and when sleeping to prevent immediately waking up.
				bool firstClick = ( player->lastClickTick[1] == 0 );
				bool autoRepeat = ticks - player->lastClickTick[1] >= timer->ticksPerSecond / 4;
				if ( player->isRiding() || player->isSprinting() || player->isSleeping() ) autoRepeat = false;
				if (useHeld )
				{
					if(player->isSleeping() ) player->lastClickTick[1] = ticks + (timer->ticksPerSecond * 2);
					if( firstClick || autoRepeat )
					{
						bool wasSleeping = player->isSleeping();

						player->handleMouseClick(1);

						if(wasSleeping) player->lastClickTick[1] = ticks + (timer->ticksPerSecond * 2);
						else player->lastClickTick[1] = ticks;
					}
				}
				else
				{
					player->lastClickTick[1] = 0;
				}
			}
		}

		if(app.DebugSettingsOn())
		{
			if (player->ullButtonsPressed & ( 1LL << MINECRAFT_ACTION_CHANGE_SKIN) )
			{
				player->ChangePlayerSkin();
			}
		}

		if (player->missTime > 0) player->missTime--;

#ifdef _DEBUG
		if(app.DebugSettingsOn())
		{
#ifndef __PSVITA__
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_RENDER_DEBUG)) )
				{
#ifndef _CONTENT_PACKAGE
#ifdef _XBOX
					app.EnableDebugOverlay(options->renderDebug,iPad);
#else
					ui.NavigateToScene(0, eUIScene_DebugOverlay, nullptr, eUILayer_Debug);
#endif
#endif
				}

				if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_SPAWN_CREEPER)))
				{
#ifndef _CONTENT_PACKAGE
					options->renderDebug = !options->renderDebug;
#endif
				}
			}

			if( (player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_FLY_TOGGLE)) )
			{
				player->abilities.debugflying = !player->abilities.debugflying;
				player->abilities.flying = !player->abilities.flying;
			}
#endif
		}
#endif

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_RENDER_THIRD_PERSON)) && gameMode->isInputAllowed(MINECRAFT_ACTION_RENDER_THIRD_PERSON))
		{
			player->SetThirdPersonView((player->ThirdPersonView()+1)%3);
		}

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_GAME_INFO)) && gameMode->isInputAllowed(MINECRAFT_ACTION_GAME_INFO))
		{
			ui.NavigateToScene(iPad,eUIScene_InGameInfoMenu);
			ui.PlayUISFX(eSFX_Press);
		}

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_INVENTORY)) && gameMode->isInputAllowed(MINECRAFT_ACTION_INVENTORY))
		{
			shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->player;
			ui.PlayUISFX(eSFX_Press);

			if(gameMode->isServerControlledInventory())
			{
				player->sendOpenInventory();
			}
			else
			{
				app.LoadInventoryMenu(iPad,player);
			}
		}

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_CRAFTING)) && gameMode->isInputAllowed(MINECRAFT_ACTION_CRAFTING))
		{
			shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->player;

			if(gameMode->hasInfiniteItems())
			{
				ui.PlayUISFX(eSFX_Press);
				app.LoadCreativeMenu(iPad,player);
			}
			else if ((hitResult!=nullptr) && (hitResult->type == HitResult::TILE) && (level->getTile(hitResult->x, hitResult->y, hitResult->z) == Tile::workBench_Id))
			{
				bool usedItem = false;
				gameMode->useItemOn(player, level, nullptr, hitResult->x, hitResult->y, hitResult->z, 0, hitResult->pos, false, &usedItem);
			}
			else
			{
				ui.PlayUISFX(eSFX_Press);
				app.LoadCrafting2x2Menu(iPad,player);
			}
		}

		if ( (player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_PAUSEMENU))
#ifdef _DURANGO
			|| (player->ullButtonsPressed&(1LL<<ACTION_MENU_GTC_PAUSE))
#endif
			)
		{
			app.DebugPrintf("PAUSE PRESS PROCESSING - ipad = %d, NavigateToScene\n",player->GetXboxPad());
			ui.PlayUISFX(eSFX_Press);
			ui.NavigateToScene(iPad, eUIScene_PauseMenu, nullptr, eUILayer_Scene);
		}

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) && gameMode->isInputAllowed(MINECRAFT_ACTION_DROP))
		{
			player->drop();
		}

		uint64_t ullButtonsPressed=player->ullButtonsPressed;

		bool selected = false;
#ifdef __PSVITA__
		SceTouchData* pTouchData = InputManager.GetTouchPadData(iPad,false);

		if(pTouchData->reportNum==1)
		{
			int iHudSize=app.GetGameSettings(iPad,eGameSetting_UISize);
			int iYOffset = (app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Tooltips) == 0) ? iToolTipOffset : 0;
			if((pTouchData->report[0].x>QuickSelectRect[iHudSize].left)&&(pTouchData->report[0].x<QuickSelectRect[iHudSize].right) &&
				(pTouchData->report[0].y>QuickSelectRect[iHudSize].top+iYOffset)&&(pTouchData->report[0].y<QuickSelectRect[iHudSize].bottom+iYOffset))
			{
				player->inventory->selected=(pTouchData->report[0].x-QuickSelectRect[iHudSize].left)/QuickSelectBoxWidth[iHudSize];
				selected = true;
				app.DebugPrintf("Touch %d\n",player->inventory->selected);
			}
		}
#endif
		if( selected || wheel != 0 || (player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) )
		{
			wstring itemName = L"";
			shared_ptr<ItemInstance> selectedItem = player->getSelectedItem();
			int iCount=0;

			if(selectedItem != nullptr) iCount=selectedItem->GetCount();
			if(selectedItem != nullptr && !( (player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) && selectedItem->GetCount() == 1))
			{
				itemName = selectedItem->getHoverName();
			}
			if( !(player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) || (selectedItem != nullptr && selectedItem->GetCount() <= 1) ) ui.SetSelectedItem( iPad, itemName );
		}
	}
	else
	{
		if (InputManager.GetValue(iPad, ACTION_MENU_CANCEL) > 0 && gameMode->isInputAllowed(ACTION_MENU_CANCEL))
		{
			setScreen(nullptr);
		}
	}

	if (level != nullptr)
	{
		if (player != nullptr)
		{
			recheckPlayerIn++;
			if (recheckPlayerIn == 30)
			{
				recheckPlayerIn = 0;
				level->ensureAdded(player);
			}
		}
		if( !level->isClientSide )
		{
			level->difficulty = options->difficulty;
		}

		PIXBeginNamedEvent(0,"Game renderer tick");
		if (!pause) gameRenderer->tick( bFirst);
		PIXEndNamedEvent();

		// Each level should be ticked exactly once per frame, when a player actually in that
		// level happens to be the active player. Use flags to avoid double-ticking.
		static unsigned int levelsTickedFlags;
		if( bFirst )
		{
			levelsTickedFlags = 0;

#ifndef DISABLE_LEVELTICK_THREAD
			PIXBeginNamedEvent(0,"levelTickEventQueue waitForFinish");
			levelTickEventQueue->waitForFinish();
			PIXEndNamedEvent();
#endif
			SparseLightStorage::tick();
			CompressedTileStorage::tick();
			SparseDataStorage::tick();
		}

		for(unsigned int i = 0; i < levels.length; ++i)
		{
			if( player->level != levels[i] ) continue;

			if (!pause && levels[i] != nullptr) levels[i]->animateTick(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));

			if( levelsTickedFlags & ( 1 << i ) ) continue;
			levelsTickedFlags |= (1 << i);

			PIXBeginNamedEvent(0,"Level renderer tick");
			if (!pause) levelRenderer->tick();
			PIXEndNamedEvent();

			if( levels[i] != nullptr )
			{
				if (!pause)
				{
					if (levels[i]->skyFlashTime > 0) levels[i]->skyFlashTime--;
					PIXBeginNamedEvent(0,"Level entity tick");
					levels[i]->tickEntities();
					PIXEndNamedEvent();
				}

#if defined __PS3__ && !defined DISABLE_SPU_CODE
				int currPlayerIdx = getLocalPlayerIdx();
				for( int idx = 0; idx < XUSER_MAX_COUNT; idx++ )
				{
					if(localplayers[idx]!=nullptr)
					{
						if( localplayers[idx]->level == levels[i] )
						{
							setLocalPlayerIdx(idx);
							gameRenderer->setupCamera(timer->a, i);
							Camera::prepare(localplayers[idx], localplayers[idx]->ThirdPersonView() == 2);
							shared_ptr<LivingEntity> cameraEntity = cameraTargetPlayer;
							double xOff = cameraEntity->xOld + (cameraEntity->x - cameraEntity->xOld) * timer->a;
							double yOff = cameraEntity->yOld + (cameraEntity->y - cameraEntity->yOld) * timer->a;
							double zOff = cameraEntity->zOld + (cameraEntity->z - cameraEntity->zOld) * timer->a;
							FrustumCuller frustObj;
							Culler *frustum = &frustObj;
							MemSect(0);
							frustum->prepare(xOff, yOff, zOff);
							levelRenderer->cull_SPU(idx, frustum, 0);
						}
					}
				}
				setLocalPlayerIdx(currPlayerIdx);
#endif

				if (!pause)
				{
					levels[i]->setSpawnSettings(level->difficulty > 0, true);
					PIXBeginNamedEvent(0,"Level tick");
#ifdef DISABLE_LEVELTICK_THREAD
					levels[i]->tick();
#else
					levelTickEventQueue->sendEvent(levels[i]);
#endif
					PIXEndNamedEvent();
				}
			}
		}

		if( bFirst )
		{
			PIXBeginNamedEvent(0,"Particle tick");
			if (!pause) particleEngine->tick();
			PIXEndNamedEvent();
		}

		if( pause ) tickAllConnections();
	}
}

void Minecraft::reloadSound()
{
	soundEngine = new SoundEngine();
	soundEngine->init(options);
	bgLoader->forceReload();
}

bool Minecraft::isClientSide()
{
	return level != nullptr && level->isClientSide;
}

void Minecraft::selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, const wstring& levelName, LevelSettings *levelSettings)
{
}

bool Minecraft::saveSlot(int slot, const wstring& name)
{
	return false;
}

bool Minecraft::loadSlot(const wstring& userName, int slot)
{
	return false;
}

void Minecraft::releaseLevel(int message)
{
	setLevel(nullptr, message);
}

void Minecraft::forceStatsSave(int idx)
{
	stats[idx]->save(idx, true);

	if( ProfileManager.IsSignedInLive(idx) )
	{
		int tempLockedProfile = ProfileManager.GetLockedProfile();
		ProfileManager.SetLockedProfile(idx);
		stats[idx]->saveLeaderboards();
		ProfileManager.SetLockedProfile(tempLockedProfile);
	}
}

MultiPlayerLevel *Minecraft::getLevel(int dimension)
{
	if (dimension == -1) return levels[1];
	else if(dimension == 1) return levels[2];
	else return levels[0];
}

void Minecraft::forceaddLevel(MultiPlayerLevel *level)
{
	int dimId = level->dimension->id;
	if (dimId == -1) levels[1] = level;
	else if(dimId == 1) levels[2] = level;
	else levels[0] = level;
}

void Minecraft::setLevel(MultiPlayerLevel *level, int message /*=-1*/, shared_ptr<Player> forceInsertPlayer /*=nullptr*/, bool doForceStatsSave /*=true*/, bool bPrimaryPlayerSignedOut /*=false*/)
{
	EnterCriticalSection(&m_setLevelCS);
	bool playerAdded = false;
	this->cameraTargetPlayer = nullptr;

	if(progressRenderer != nullptr)
	{
		this->progressRenderer->progressStart(message);
		this->progressRenderer->progressStage(-1);
	}

	gameRenderer->DisableUpdateThread();

	if (level == nullptr || player == nullptr)
	{
		for (int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if (localitemInHandRenderers[i] != nullptr)
			{
				localitemInHandRenderers[i]->reset();
			}
		}
	}

	for(unsigned int i = 0; i < levels.length; ++i)
	{
		if (levels[i] != nullptr && level == nullptr)
		{
			if((doForceStatsSave==true) && player!=nullptr)
				forceStatsSave(player->GetXboxPad() );

			if (levelRenderer != nullptr)
			{
				for(DWORD p = 0; p < XUSER_MAX_COUNT; ++p)
				{
					levelRenderer->setLevel(p, nullptr);
				}
			}
			if (particleEngine != nullptr) particleEngine->setLevel(nullptr);
		}
	}

	if( level == nullptr )
	{
		if(levels[0]!=nullptr)
		{
			delete levels[0];
			levels[0] = nullptr;

			if(levels[1]!=nullptr) levels[1]->savedDataStorage = nullptr;
		}
		if(levels[1]!=nullptr)
		{
			delete levels[1];
			levels[1] = nullptr;
		}
		if(levels[2]!=nullptr)
		{
			delete levels[2];
			levels[2] = nullptr;
		}

		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
			if(mplp != nullptr && mplp->connection != nullptr )
			{
				delete mplp->connection;
				mplp->connection = nullptr;
			}

			if( localgameModes[idx] != nullptr )
			{
				delete localgameModes[idx];
				localgameModes[idx] = nullptr;
			}

			if( m_pendingLocalConnections[idx] != nullptr )
			{
				delete m_pendingLocalConnections[idx];
				m_pendingLocalConnections[idx] = nullptr;
			}

			localplayers[idx] = nullptr;
		}
		gameMode = nullptr;
		player = nullptr;
		cameraTargetPlayer = nullptr;
		EntityRenderDispatcher::instance->cameraEntity = nullptr;
		TileEntityRenderDispatcher::instance->cameraEntity = nullptr;
	}
	this->level = level;

	if (level != nullptr)
	{
		int dimId = level->dimension->id;
		if (dimId == -1) levels[1] = level;
		else if(dimId == 1) levels[2] = level;
		else levels[0] = level;

		if (player == nullptr)
		{
			int iPrimaryPlayer = ProfileManager.GetPrimaryPad();

			player = gameMode->createPlayer(level);

			PlayerUID playerXUIDOffline = INVALID_XUID;
			PlayerUID playerXUIDOnline = INVALID_XUID;
			ProfileManager.GetXUID(iPrimaryPlayer,&playerXUIDOffline,false);
			ProfileManager.GetXUID(iPrimaryPlayer,&playerXUIDOnline,true);
#ifdef __PSVITA__
			if(CGameNetworkManager::usingAdhocMode() && playerXUIDOnline.getOnlineID()[0] == 0)
			{
				playerXUIDOnline.setForAdhoc();
			}
#endif
#ifdef _WINDOWS64
			INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(iPrimaryPlayer);
			if(localNetworkPlayer != nullptr && localNetworkPlayer->IsHost())
			{
				playerXUIDOffline = Win64Xuid::GetLegacyEmbeddedHostXuid();
			}
			else
			{
				playerXUIDOffline = Win64Xuid::ResolvePersistentXuid();
			}
#endif
			player->setXuid(playerXUIDOffline);
			player->setOnlineXuid(playerXUIDOnline);

			player->m_displayName = ProfileManager.GetDisplayName(iPrimaryPlayer);

			player->resetPos();
			gameMode->initPlayer(player);

			player->SetXboxPad(iPrimaryPlayer);

			for(int i=0;i<XUSER_MAX_COUNT;i++)
			{
				m_pendingLocalConnections[i] = nullptr;
				if( i != iPrimaryPlayer ) localgameModes[i] = nullptr;
			}
		}

		if (player != nullptr)
		{
			player->resetPos();
			if (level != nullptr)
			{
				level->addEntity(player);
				playerAdded = true;
			}
		}

		if(player->input != nullptr) delete player->input;
		player->input = new Input();

		if (levelRenderer != nullptr) levelRenderer->setLevel(player->GetXboxPad(), level);
		if (particleEngine != nullptr) particleEngine->setLevel(level);

		gameMode->adjustPlayer(player);

		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			m_pendingLocalConnections[i] = nullptr;
		}
		updatePlayerViewportAssignments();

		this->cameraTargetPlayer = player;

		gameRenderer->EnableUpdateThread();
	}
	else
	{
		levelSource->clearAll();
		player = nullptr;

		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if( m_pendingLocalConnections[i] != nullptr ) m_pendingLocalConnections[i]->close();
			m_pendingLocalConnections[i] = nullptr;
			localplayers[i] = nullptr;
			localgameModes[i] = nullptr;
		}
	}

	LeaveCriticalSection(&m_setLevelCS);
}

void Minecraft::prepareLevel(int title)
{
	if(progressRenderer != nullptr)
	{
		this->progressRenderer->progressStart(title);
		this->progressRenderer->progressStage(IDS_PROGRESS_BUILDING_TERRAIN);
	}
	int r = 128;
	if (gameMode->isCutScene()) r = 64;
	int pp = 0;
	int max = r * 2 / 16 + 1;
	max = max * max;
	ChunkSource *cs = level->getChunkSource();

	Pos *spawnPos = level->getSharedSpawnPos();
	if (player != nullptr)
	{
		spawnPos->x = static_cast<int>(player->x);
		spawnPos->z = static_cast<int>(player->z);
	}

	for (int x = -r; x <= r; x += 16)
	{
		for (int z = -r; z <= r; z += 16)
		{
			if(progressRenderer != nullptr) this->progressRenderer->progressStagePercentage((pp++) * 100 / max);
			level->getTile(spawnPos->x + x, 64, spawnPos->z + z);
			if (!gameMode->isCutScene()) {
			}
		}
	}
	delete spawnPos;
	if (!gameMode->isCutScene())
	{
		if(progressRenderer != nullptr) this->progressRenderer->progressStage(IDS_PROGRESS_SIMULATING_WORLD);
		max = 2000;
	}
}

wstring Minecraft::gatherStats1()
{
	return L"Time to autosave: " + std::to_wstring( app.SecondsToAutosave() ) + L"s";
}

wstring Minecraft::gatherStats2()
{
	return g_NetworkManager.GatherStats();
}

wstring Minecraft::gatherStats3()
{
	return g_NetworkManager.GatherRTTStats();
}

wstring Minecraft::gatherStats4()
{
	return level->gatherChunkSourceStats();
}

void Minecraft::respawnPlayer(int iPad, int dimension, int newEntityId)
{
	gameRenderer->DisableUpdateThread();
	shared_ptr<MultiplayerLocalPlayer> localPlayer = localplayers[iPad];

	level->validateSpawn();
	level->removeAllPendingEntityRemovals();

	if (localPlayer != nullptr)
	{
		level->removeEntity(localPlayer);
	}

	shared_ptr<Player> oldPlayer = localPlayer;
	cameraTargetPlayer = nullptr;

	int iTempPad=localPlayer->GetXboxPad();
	int iTempScreenSection = localPlayer->m_iScreenSection;
	EDefaultSkins skin = localPlayer->getPlayerDefaultSkin();
	player = localgameModes[iPad]->createPlayer(level);

	PlayerUID playerXUIDOffline = INVALID_XUID;
	PlayerUID playerXUIDOnline = INVALID_XUID;
	ProfileManager.GetXUID(iTempPad,&playerXUIDOffline,false);
	ProfileManager.GetXUID(iTempPad,&playerXUIDOnline,true);
#ifdef _WINDOWS64
	INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(iTempPad);
	if(localNetworkPlayer != nullptr && localNetworkPlayer->IsHost())
	{
		playerXUIDOffline = Win64Xuid::GetLegacyEmbeddedHostXuid();
	}
	else
	{
		playerXUIDOffline = Win64Xuid::ResolvePersistentXuid();
	}
#endif
	player->setXuid(playerXUIDOffline);
	player->setOnlineXuid(playerXUIDOnline);
	player->setIsGuest( ProfileManager.IsGuest(iTempPad) );

	player->m_displayName = ProfileManager.GetDisplayName(iPad);

	player->SetXboxPad(iTempPad);

	player->m_iScreenSection = iTempScreenSection;
	player->setPlayerIndex( localPlayer->getPlayerIndex() );
	player->setCustomSkin(localPlayer->getCustomSkin());
	player->setPlayerDefaultSkin( skin );
	player->setCustomCape(localPlayer->getCustomCape());
	player->m_sessionTimeStart = localPlayer->m_sessionTimeStart;
	player->m_dimensionTimeStart = localPlayer->m_dimensionTimeStart;
	player->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All, localPlayer->getAllPlayerGamePrivileges());

	player->SetThirdPersonView(oldPlayer->ThirdPersonView());

	// Preserve hotbar selection on dimension change/respawn, but not on death
	if( localPlayer->getHealth() > 0 && localPlayer->y > -64)
	{
		player->inventory->selected = localPlayer->inventory->selected;
	}

	DWORD dwSkinID=app.getSkinIdFromPath(player->customTextureUrl);
	if(GET_IS_DLC_SKIN_FROM_BITMASK(dwSkinID))
	{
		player->setAnimOverrideBitmask(player->getSkinAnimOverrideBitmask(dwSkinID));
	}

	player->dimension = dimension;
	cameraTargetPlayer = player;

	if(iPad==ProfileManager.GetPrimaryPad())
	{
		createPrimaryLocalPlayer(iPad);
		app.SetGameSettingsDebugMask(ProfileManager.GetPrimaryPad(),app.GetGameSettingsDebugMask(-1,true));
	}
	else
	{
		storeExtraLocalPlayer(iPad);
	}

	player->setShowOnMaps(app.GetGameHostOption(eGameHostOption_Gamertags)!=0?true:false);

	player->resetPos();
	level->addEntity(player);
	gameMode->initPlayer(player);

	if(player->input != nullptr) delete player->input;
	player->input = new Input();
	player->entityId = newEntityId;
	player->animateRespawn();
	gameMode->adjustPlayer(player);

	if (!level->isClientSide)
	{
		prepareLevel(IDS_PROGRESS_RESPAWNING);
	}

	player->SetPlayerRespawned(true);

	if (dynamic_cast<DeathScreen *>(screen) != nullptr) setScreen(nullptr);

	gameRenderer->EnableUpdateThread();
}

void Minecraft::start(const wstring& name, const wstring& sid)
{
	startAndConnectTo(name, sid, L"");
}

void Minecraft::startAndConnectTo(const wstring& name, const wstring& sid, const wstring& url)
{
	const bool fullScreen = false;
	const wstring userName = name;

	Minecraft *minecraft;
	extern int g_iScreenWidth;
	extern int g_iScreenHeight;
	constexpr int logicalH = 720;
	const int logicalW = logicalH * g_iScreenWidth / g_iScreenHeight;

	minecraft = new Minecraft(nullptr, nullptr, nullptr, logicalW, logicalH, fullScreen);

	minecraft->serverDomain = L"www.minecraft.net";

	if (userName != L"" && sid != L"")
	{
		minecraft->user = new User(userName, sid);
	}
	else
	{
		minecraft->user = new User(L"Player" + std::to_wstring(System::currentTimeMillis() % 1000), L"");
	}

	minecraft->run();
}

ClientConnection *Minecraft::getConnection(int iPad)
{
	return localplayers[iPad]->connection;
}

Minecraft *Minecraft::GetInstance()
{
	return m_instance;
}

bool useLomp = false;

int g_iMainThreadId;

void Minecraft::main()
{
	wstring name;
	wstring sessionId;

	useLomp = true;

	MinecraftWorld_RunStaticCtors();
	EntityRenderDispatcher::staticCtor();
	TileEntityRenderDispatcher::staticCtor();
	User::staticCtor();
	Tutorial::staticCtor();
	ColourTable::staticCtor();
	app.loadDefaultGameRules();

#ifdef _LARGE_WORLDS
	LevelRenderer::staticCtor();
#endif

	name = L"Player" + std::to_wstring(System::currentTimeMillis() % 1000);
	sessionId = L"-";

	IUIScene_CreativeMenu::staticCtor();

#ifndef __ORBIS__
	Minecraft::start(name, sessionId);
#endif
}

bool Minecraft::renderNames()
{
	if (m_instance == nullptr || !m_instance->options->hideGui)
	{
		return true;
	}
	return false;
}

bool Minecraft::useFancyGraphics()
{
	return (m_instance != nullptr && m_instance->options->fancyGraphics);
}

bool Minecraft::useAmbientOcclusion()
{
	return (m_instance != nullptr && m_instance->options->ambientOcclusion != Options::AO_OFF);
}

bool Minecraft::renderDebug()
{
	return (m_instance != nullptr && m_instance->options->renderDebug);
}

bool Minecraft::handleClientSideCommand(const wstring& chatMessage)
{
	return false;
}

int Minecraft::maxSupportedTextureSize()
{
	return 1024;
}

void Minecraft::delayTextureReload()
{
	reloadTextures = true;
}

int64_t Minecraft::currentTimeMillis()
{
	return System::currentTimeMillis();
}

Screen * Minecraft::getScreen()
{
	return screen;
}

bool Minecraft::isTutorial()
{
	return m_inFullTutorialBits > 0;
}

void Minecraft::playerStartedTutorial(int iPad)
{
	if( app.GetTutorialMode() ) m_inFullTutorialBits = m_inFullTutorialBits | ( 1 << iPad );
}

void Minecraft::playerLeftTutorial(int iPad)
{
	if(m_inFullTutorialBits == 0)
	{
		app.SetTutorialMode( false );
		return;
	}

	m_inFullTutorialBits = m_inFullTutorialBits & ~( 1 << iPad );
	if(m_inFullTutorialBits == 0)
	{
		app.SetTutorialMode( false );

#ifndef _XBOX_ONE
		for(DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if(localplayers[idx] != nullptr)
			{
				TelemetryManager->RecordLevelStart(idx, eSen_FriendOrMatch_Playing_With_Invited_Friends, eSen_CompeteOrCoop_Coop_and_Competitive, level->difficulty, app.GetLocalPlayerCount(), g_NetworkManager.GetOnlinePlayerCount());
			}
		}
#endif
	}
}

#ifdef _DURANGO
void Minecraft::inGameSignInCheckAllPrivilegesCallback(LPVOID lpParam, bool hasPrivileges, int iPad)
{
	Minecraft* pClass = (Minecraft*)lpParam;

	if(!hasPrivileges)
	{
		ProfileManager.RemoveGamepadFromGame(iPad);
	}
	else
	{
		if( !g_NetworkManager.SessionHasSpace() )
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			ui.RequestErrorMessage(IDS_MULTIPLAYER_FULL_TITLE, IDS_MULTIPLAYER_FULL_TEXT, uiIDA, 1);
			ProfileManager.RemoveGamepadFromGame(iPad);
		}
		else if( ProfileManager.IsSignedInLive(iPad) && ProfileManager.AllowedToPlayMultiplayer(iPad) )
		{
			shared_ptr<Player> player = pClass->localplayers[iPad];
			if( player == nullptr)
			{
				if( pClass->level->isClientSide )
				{
					pClass->addLocalPlayer(iPad);
				}
				else
				{
					shared_ptr<Player> player = pClass->localplayers[iPad];
					if( player == nullptr)
					{
						player = pClass->createExtraLocalPlayer(iPad, (convStringToWstring( ProfileManager.GetGamertag(iPad) )).c_str(), iPad, pClass->level->dimension->id);
					}
				}
			}
		}
	}
}
#endif

#ifdef _XBOX_ONE
int Minecraft::InGame_SignInReturned(void *pParam,bool bContinue, int iPad, int iController)
#else
int Minecraft::InGame_SignInReturned(void *pParam,bool bContinue, int iPad)
#endif
{
	Minecraft* pMinecraftClass = static_cast<Minecraft *>(pParam);

	if(g_NetworkManager.IsInSession())
	{
		app.DebugPrintf("Disabling Guest Signin\n");
		XEnableGuestSignin(FALSE);
	}

	if(bContinue==true && g_NetworkManager.IsInSession() && pMinecraftClass->localplayers[iPad] == nullptr)
	{
		if(ProfileManager.IsSignedIn(iPad))
		{
#ifdef _DURANGO
			if(!g_NetworkManager.IsLocalGame() && ProfileManager.IsSignedInLive(iPad) && ProfileManager.AllowedToPlayMultiplayer(iPad))
			{
				ProfileManager.CheckMultiplayerPrivileges(iPad, true, &inGameSignInCheckAllPrivilegesCallback, pMinecraftClass);
			}
			else
#endif
			if( !g_NetworkManager.SessionHasSpace() )
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestErrorMessage(IDS_MULTIPLAYER_FULL_TITLE, IDS_MULTIPLAYER_FULL_TEXT, uiIDA, 1);
#ifdef _DURANGO
				ProfileManager.RemoveGamepadFromGame(iPad);
#endif
			}
			else if( g_NetworkManager.IsLocalGame() || (ProfileManager.IsSignedInLive(iPad) && ProfileManager.AllowedToPlayMultiplayer(iPad)) )
			{
#ifdef __ORBIS__
				bool contentRestricted = false;
				ProfileManager.GetChatAndContentRestrictions(iPad,false,nullptr,&contentRestricted,nullptr);

				if (!g_NetworkManager.IsLocalGame() && contentRestricted)
				{
					ui.RequestContentRestrictedMessageBox(IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_CONTENT_RESTRICTION, iPad);
				}
				else if(!g_NetworkManager.IsLocalGame() && !ProfileManager.HasPlayStationPlus(iPad))
				{
					pMinecraftClass->m_pPsPlusUpsell = new PsPlusUpsellWrapper(iPad);
					pMinecraftClass->m_pPsPlusUpsell->displayUpsell();
				}
				else
#endif
				if( pMinecraftClass->level->isClientSide )
				{
					pMinecraftClass->addLocalPlayer(iPad);
				}
				else
				{
					shared_ptr<Player> player = pMinecraftClass->localplayers[iPad];
					if( player == nullptr)
					{
						player = pMinecraftClass->createExtraLocalPlayer(iPad, (convStringToWstring( ProfileManager.GetGamertag(iPad) )).c_str(), iPad, pMinecraftClass->level->dimension->id);
					}
				}
			}
			else if( ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && !ProfileManager.AllowedToPlayMultiplayer(iPad) )
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestErrorMessage( IDS_NO_MULTIPLAYER_PRIVILEGE_TITLE, IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT, uiIDA,1,iPad);
#ifdef _DURANGO
				ProfileManager.RemoveGamepadFromGame(iPad);
#endif
			}
		}
	}
	return 0;
}

void Minecraft::tickAllConnections()
{
	int oldIdx = getLocalPlayerIdx();
	for(unsigned int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[i];
		if( mplp && mplp->connection)
		{
			setLocalPlayerIdx(i);
			mplp->connection->tick();
		}
	}
	setLocalPlayerIdx(oldIdx);
}

bool Minecraft::addPendingClientTextureRequest(const wstring &textureName)
{
	auto it = find(m_pendingTextureRequests.begin(), m_pendingTextureRequests.end(), textureName);
	if( it == m_pendingTextureRequests.end() )
	{
		m_pendingTextureRequests.push_back(textureName);
		return true;
	}
	return false;
}

void Minecraft::handleClientTextureReceived(const wstring &textureName)
{
	auto it = find(m_pendingTextureRequests.begin(), m_pendingTextureRequests.end(), textureName);
	if( it != m_pendingTextureRequests.end() )
	{
		m_pendingTextureRequests.erase(it);
	}
}

unsigned int Minecraft::getCurrentTexturePackId()
{
	return skins->getSelected()->getId();
}

ColourTable *Minecraft::getColourTable()
{
	TexturePack *selected = skins->getSelected();

	ColourTable *colours = selected->getColourTable();

	if(colours == nullptr)
	{
		colours = skins->getDefault()->getColourTable();
	}

	return colours;
}

#if defined __ORBIS__
int Minecraft::MustSignInReturnedPSN(void *pParam, int iPad, C4JStorage::EMessageResult result)
{
	Minecraft* pMinecraft = (Minecraft *)pParam;

	if(result == C4JStorage::EMessage_ResultAccept)
	{
		SQRNetworkManager_Orbis::AttemptPSNSignIn(&Minecraft::InGame_SignInReturned, pMinecraft, false, iPad);
	}

	return 0;
}
#endif
