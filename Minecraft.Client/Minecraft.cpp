#include "stdafx.h"
#include "Minecraft.h"
#include "GameMode.h"
#include "Timer.h"
#include "ProgressRenderer.h"
#include "LevelRenderer.h"
#include "ParticleEngine.h"
#include "MultiPlayerLocalPlayer.h"
#include "User.h"
#include "Textures.h"
#include "GameRenderer.h"
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
#include "MultiPlayerLocalPlayer.h"
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

// #define DISABLE_SPU_CODE
// 4J Turning this on will change the graph at the bottom of the debug overlay to show the number of packets of each type added per fram
//#define DEBUG_RENDER_SHOWS_PACKETS 1
//#define SPLITSCREEN_TEST

// If not disabled, this creates an event queue on a seperate thread so that the Level::tick calls can be offloaded
// from the main thread, and have longer to run, since it's called at 20Hz instead of 60
#define DISABLE_LEVELTICK_THREAD

Minecraft *Minecraft::m_instance = NULL;
int64_t Minecraft::frameTimes[512];
int64_t Minecraft::tickTimes[512];
int Minecraft::frameTimePos = 0;
int64_t Minecraft::warezTime = 0;
File Minecraft::workDir = File(L"");

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

// 4J - TomK ToDo: these really shouldn't be magic numbers, it should read the hud position from flash.
int iToolTipOffset = 85;

#endif

ResourceLocation Minecraft::DEFAULT_FONT_LOCATION = ResourceLocation(TN_DEFAULT_FONT);
ResourceLocation Minecraft::ALT_FONT_LOCATION = ResourceLocation(TN_ALT_FONT);


Minecraft::Minecraft(Component *mouseComponent, Canvas *parent, MinecraftApplet *minecraftApplet, int width, int height, bool fullscreen)
{
	// 4J - added this block of initialisers
	gameMode = NULL;
	hasCrashed = false;
	timer = new Timer(SharedConstants::TICKS_PER_SECOND);
	oldLevel = NULL; //4J Stu added
	level = NULL;
	levels = MultiPlayerLevelArray(3); // 4J Added
	levelRenderer = NULL;
	player = nullptr;
	cameraTargetPlayer = nullptr;
	particleEngine = NULL;
	user = NULL;
	parent = NULL;
	pause = false;
	textures = NULL;
	font = NULL;
	screen = NULL;
	localPlayerIdx = 0;
	rightClickDelay = 0;

	// 4J Stu Added
	InitializeCriticalSection( &ProgressRenderer::s_progress );
	InitializeCriticalSection(&m_setLevelCS);
	//m_hPlayerRespawned = CreateEvent(NULL, FALSE, FALSE, NULL);

	progressRenderer = NULL;
	gameRenderer = NULL;
	bgLoader = NULL;

	ticks = 0;
	// 4J-PB - moved into the local player
	//missTime = 0;
	//lastClickTick = 0;
	//isRaining = false;
	// 4J-PB - end

	orgWidth = orgHeight = 0;
	achievementPopup = new AchievementPopup(this);
	gui = NULL;
	noRender = false;
	humanoidModel = new HumanoidModel(0);
	hitResult = 0;
	options = NULL;
	soundEngine = new SoundEngine();
	mouseHandler = NULL;
	skins = NULL;
	workingDirectory = File(L"");
	levelSource = NULL;
	stats[0] = NULL;
	stats[1] = NULL;
	stats[2] = NULL;
	stats[3] = NULL;
	connectToPort = 0;
	workDir = File(L"");
	// 4J removed
	//wasDown = false;
	lastTimer = -1;

	// 4J removed
	//lastTickTime = System::currentTimeMillis();
	recheckPlayerIn = 0;
	running = true;
	unoccupiedQuadrant = -1;

	Stats::init();

	orgHeight = height;
	this->fullscreen = fullscreen;
	this->minecraftApplet = NULL;

	this->parent = parent;
	// 4J - Our actual physical frame buffer is always 1280x720 ie in a 16:9 ratio. If we want to do a 4:3 mode, we are telling the original minecraft code
	// that the width is 3/4 what it actually is, to correctly present a 4:3 image. Have added width_phys and height_phys for any code we add that requires
	// to know the real physical dimensions of the frame buffer.
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
		m_pendingLocalConnections[i] = NULL;
		m_connectionFailed[i] = false;
		localgameModes[i]=NULL;
	}

	animateTickLevel = NULL;	// 4J added
	m_inFullTutorialBits = 0; // 4J Added
	reloadTextures = false;

	// initialise the audio before any textures are loaded - to avoid the problem in win64 of the Miles audio causing the codec for textures to be unloaded

	// 4J-PB - Removed it from here on Orbis due to it causing a crash with the network init.
	// We should work out why...
#ifndef __ORBIS__
	this->soundEngine->init(NULL);
#endif

#ifndef DISABLE_LEVELTICK_THREAD
	levelTickEventQueue = new C4JThread::EventQueue(levelTickUpdateFunc, levelTickThreadInitFunc, "LevelTick_EventQueuePoll");
	levelTickEventQueue->setProcessor(3);
	levelTickEventQueue->setPriority(THREAD_PRIORITY_NORMAL);
#endif // DISABLE_LEVELTICK_THREAD
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
#if 0 // 4J - removed
	if (parent != null)
	{
		Graphics g = parent.getGraphics();
		if (g != null) {
			g.setColor(Color.BLACK);
			g.fillRect(0, 0, width, height);
			g.dispose();
		}
		Display.setParent(parent);
	} else {
		if (fullscreen) {
			Display.setFullscreen(true);
			width = Display.getDisplayMode().getWidth();
			height = Display.getDisplayMode().getHeight();
			if (width <= 0) width = 1;
			if (height <= 0) height = 1;
		} else {
			Display.setDisplayMode(new DisplayMode(width, height));
		}
	}

	Display.setTitle("Minecraft " + VERSION_STRING);
	try {
		Display.create();
		/*
		* System.out.println("LWJGL version: " + Sys.getVersion());
		* System.out.println("GL RENDERER: " +
		* GL11.glGetString(GL11.GL_RENDERER));
		* System.out.println("GL VENDOR: " +
		* GL11.glGetString(GL11.GL_VENDOR));
		* System.out.println("GL VERSION: " +
		* GL11.glGetString(GL11.GL_VERSION)); ContextCapabilities caps =
		* GLContext.getCapabilities(); System.out.println("OpenGL 3.0: " +
		* caps.OpenGL30); System.out.println("OpenGL 3.1: " +
		* caps.OpenGL31); System.out.println("OpenGL 3.2: " +
		* caps.OpenGL32); System.out.println("ARB_compatibility: " +
		* caps.GL_ARB_compatibility); if (caps.OpenGL32) { IntBuffer buffer
		* = ByteBuffer.allocateDirect(16 *
		* 4).order(ByteOrder.nativeOrder()).asIntBuffer();
		* GL11.glGetInteger(GL32.GL_CONTEXT_PROFILE_MASK, buffer); int
		* profileMask = buffer.get(0); System.out.println("PROFILE MASK: "
		* + Integer.toBinaryString(profileMask));
		* System.out.println("CORE PROFILE: " + ((profileMask &
		* GL32.GL_CONTEXT_CORE_PROFILE_BIT) != 0));
		* System.out.println("COMPATIBILITY PROFILE: " + ((profileMask &
		* GL32.GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) != 0)); }
		*/
	} catch (LWJGLException e) {
		// This COULD be because of a bug! A delay followed by a new attempt
		// is supposed getWorkingDirectoryto fix it.
		e.printStackTrace();
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e1) {
		}
		Display.create();
	}

	if (Minecraft.FLYBY_MODE) {
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	}
#endif

	// glClearColor(0.2f, 0.2f, 0.2f, 1);

	workingDirectory = File(L"");//getWorkingDirectory();
	levelSource = new McRegionLevelStorageSource(File(workingDirectory, L"saves"));
	//        levelSource = new MemoryLevelStorageSource();
	options = new Options(this, workingDirectory);
	skins = new TexturePackRepository(workingDirectory, this);
	skins->addDebugPacks();
	textures = new Textures(skins, options);
	//renderLoadingScreen();

	font = new Font(options, L"font/Default.png", textures, false, &DEFAULT_FONT_LOCATION, 23, 20, 8, 8, SFontData::Codepoints);
	altFont = new Font(options, L"font/alternate.png", textures, false, &ALT_FONT_LOCATION, 16, 16, 8, 8);

	//if (options.languageCode != null) {
	//	Language.getInstance().loadLanguage(options.languageCode);
	//	//            font.setEnforceUnicodeSheet("true".equalsIgnoreCase(I18n.get("language.enforceUnicode")));
	//	font.setEnforceUnicodeSheet(Language.getInstance().isSelectedLanguageIsUnicode());
	//	font.setBidirectional(Language.isBidirectional(options.languageCode));
	//}

	// 4J Stu - Not using these any more
	//WaterColor::init(textures->loadTexturePixels(L"misc/watercolor.png"));
	//GrassColor::init(textures->loadTexturePixels(L"misc/grasscolor.png"));
	//FoliageColor::init(textures->loadTexturePixels(L"misc/foliagecolor.png"));

	gameRenderer = new GameRenderer(this);
	EntityRenderDispatcher::instance->itemInHandRenderer = new ItemInHandRenderer(this,false);

	for( int i=0 ; i<4 ; ++i )
		stats[i] = new StatsCounter();

	/*		4J - TODO, 4J-JEV: Unnecessary.
	Achievements::openInventory->setDescFormatter(NULL);
	Achievements.openInventory.setDescFormatter(new DescFormatter(){
	public String format(String i18nValue) {
	return String.format(i18nValue, Keyboard.getKeyName(options.keyBuild.key));
	}
	});
	*/

	// 4J-PB - We'll do this in a xui intro
	//renderLoadingScreen();

	//Keyboard::create();
	Mouse::create();
#if 0	// 4J - removed
	mouseHandler = new MouseHandler(parent);
	try {
		Controllers.create();
	} catch (Exception e) {
		e.printStackTrace();
	}
#endif

	MemSect(31);
	checkGlError(L"Pre startup");
	MemSect(0);

	// width = Display.getDisplayMode().getWidth();
	// height = Display.getDisplayMode().getHeight();

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

	//    openGLCapabilities = new OpenGLCapabilities();	// 4J - removed

	levelRenderer = new LevelRenderer(this, textures);
	//textures->register(&TextureAtlas::LOCATION_BLOCKS, new TextureAtlas(Icon::TYPE_TERRAIN, TN_TERRAIN));
	//textures->register(&TextureAtlas::LOCATION_ITEMS, new TextureAtlas(Icon::TYPE_ITEM, TN_GUI_ITEMS));
	textures->stitch();

	glViewport(0, 0, width, height);

	particleEngine = new ParticleEngine(level, textures);

	MemSect(31);
	checkGlError(L"Post startup");
	MemSect(0);
	gui = new Gui(this);

	if (connectToIp != L"")	// 4J - was NULL comparison
	{
		//        setScreen(new ConnectScreen(this, connectToIp, connectToPort));		// 4J TODO - put back in
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
	// 4J Unused
	// testing stuff on vita just now
#ifdef __PSVITA__
	ScreenSizeCalculator ssc(options, width, height);

	// xxx
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
	// xxx
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
	// xxx
	RenderManager.Present();
#endif
}

void Minecraft::blit(int x, int y, int sx, int sy, int w, int h)
{
	float us = 1 / 256.0f;
	float vs = 1 / 256.0f;
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->vertexUV((float)(x + 0), (float)( y + h), (float)( 0), (float)( (sx + 0) * us), (float)( (sy + h) * vs));
	t->vertexUV((float)(x + w), (float)( y + h), (float)( 0), (float)( (sx + w) * us), (float)( (sy + h) * vs));
	t->vertexUV((float)(x + w), (float)( y + 0), (float)( 0), (float)( (sx + w) * us), (float)( (sy + 0) * vs));
	t->vertexUV((float)(x + 0), (float)( y + 0), (float)( 0), (float)( (sx + 0) * us), (float)( (sy + 0) * vs));
	t->end();
}

LevelStorageSource *Minecraft::getLevelSource()
{
	return levelSource;
}

void Minecraft::setScreen(Screen *screen)
{
	if (this->screen != NULL)
	{
		this->screen->removed();
	}

#ifdef _WINDOWS64
	if (screen != NULL && g_KBMInput.IsMouseGrabbed())
	{
		g_KBMInput.SetMouseGrabbed(false);
	}
#endif

	//4J Gordon: Do not force a stats save here
	/*if (dynamic_cast<TitleScreen *>(screen)!=NULL)
	{
	stats->forceSend();
	}
	stats->forceSave();*/

	if (screen == NULL && level == NULL)
	{
		screen = new TitleScreen();
	}
	else if (player != NULL && !ui.GetMenuDisplayed(player->GetXboxPad()) && player->getHealth() <= 0)
	{
		//screen = new DeathScreen();

		// 4J Stu - If we exit from the death screen then we are saved as being dead. In the Java
		// game when you load the game you are still dead, but this is silly so only show the dead
		// screen if we have died during gameplay
		if(ticks==0)
		{
			player->respawn();
		}
		else
		{
			ui.NavigateToScene(player->GetXboxPad(),eUIScene_DeathMenu,NULL);
		}
	}

	if (dynamic_cast<TitleScreen *>(screen)!=NULL)
	{
		options->renderDebug = false;
		gui->clearMessages();
	}

	this->screen = screen;
	if (screen != NULL)
	{
		//        releaseMouse();	// 4J - removed
		ScreenSizeCalculator ssc(options, width, height);
		int screenWidth = ssc.getWidth();
		int screenHeight = ssc.getHeight();
		screen->init(this, screenWidth, screenHeight);
		noRender = false;
	}
	else
	{
		//        grabMouse();	// 4J - removed
	}

	// 4J-PB - if a screen has been set, go into menu mode
	// it's possible that player doesn't exist here yet
	/*if(screen!=NULL)
	{
	if(player && player->GetXboxPad()!=-1)
	{
	InputManager.SetMenuDisplayed(player->GetXboxPad(),true);
	}
	else
	{
	// set all
	//InputManager.SetMenuDisplayed(XUSER_INDEX_ANY,true);
	}
	}
	else
	{
	if(player && player->GetXboxPad()!=-1)
	{
	InputManager.SetMenuDisplayed(player->GetXboxPad(),false);
	}
	else
	{
	//InputManager.SetMenuDisplayed(XUSER_INDEX_ANY,false);
	}
	}*/
}

void Minecraft::checkGlError(const wstring& string)
{
	// 4J - TODO
}

void Minecraft::destroy()
{
	//4J Gordon: Do not force a stats save here
	/*stats->forceSend();
	stats->forceSave();*/

	//    try {
	setLevel(NULL);
	//    } catch (Throwable e) {
	//    }

	//    try {
	MemoryTracker::release();
	//    } catch (Throwable e) {
	//    }

	soundEngine->destroy();
	//} finally {
	Display::destroy();
	//    if (!hasCrashed) System.exit(0);	//4J - removed
	//}
	//System.gc();	// 4J - removed
}

// 4J-PB - splitting this function into 3 parts, so we can call the middle part from our xbox game loop

#if 0
void Minecraft::run()
{
	running = true;
	//    try {	// 4J - removed try/catch
	init();
	//    } catch (Exception e) {
	//        e.printStackTrace();
	//       crash(new CrashReport("Failed to start game", e));
	//        return;
	//    }
	//    try {	// 4J - removed try/catch
	if (Minecraft::FLYBY_MODE)
	{
		generateFlyby();
		return;
	}

	int64_t lastTime = System::currentTimeMillis();
	int frames = 0;

	while (running)
	{
		//        try {	// 4J - removed try/catch
		//            if (minecraftApplet != null && !minecraftApplet.isActive()) break;	// 4J - removed
		AABB::resetPool();
		Vec3::resetPool();

		//            if (parent == NULL && Display.isCloseRequested()) {		// 4J - removed
		//                stop();
		//            }

		if (pause && level != NULL)
		{
			float lastA = timer->a;
			timer->advanceTime();
			timer->a = lastA;
		}
		else
		{
			timer->advanceTime();
		}

		int64_t beforeTickTime = System::nanoTime();
		for (int i = 0; i < timer->ticks; i++)
		{
			ticks++;
			//            try {		// 4J - try/catch removed
			tick();
			//            } catch (LevelConflictException e) {
			//                this.level = null;
			//                setLevel(null);
			//                setScreen(new LevelConflictScreen());
			//            }
		}
		int64_t tickDuraction = System::nanoTime() - beforeTickTime;
		checkGlError(L"Pre render");

		TileRenderer::fancy = options->fancyGraphics;

		// if (pause) timer.a = 1;

		soundEngine->update(player, timer->a);

		glEnable(GL_TEXTURE_2D);
		if (level != NULL) level->updateLights();

		//        if (!Keyboard::isKeyDown(Keyboard.KEY_F7)) Display.update();		// 4J - removed

		if (player != NULL && player->isInWall()) options->thirdPersonView = false;
		if (!noRender)
		{
			if (gameMode != NULL) gameMode->render(timer->a);
			gameRenderer->render(timer->a);
		}

		/*	4J - removed
		if (!Display::isActive())
		{
		if (fullscreen)
		{
		this->toggleFullScreen();
		}
		Sleep(10);
		}
		*/

		if (options->renderDebug)
		{
			renderFpsMeter(tickDuraction);
		}
		else
		{
			lastTimer = System::nanoTime();
		}

		achievementPopup->render();

		Sleep(0);	// 4J - was Thread.yield()

		//        if (Keyboard::isKeyDown(Keyboard::KEY_F7)) Display.update();	// 4J - removed condition
		Display::update();

		//        checkScreenshot();	// 4J - removed

		/* 4J - removed
		if (parent != NULL && !fullscreen)
		{
		if (parent.getWidth() != width || parent.getHeight() != height)
		{
		width = parent.getWidth();
		height = parent.getHeight();
		if (width <= 0) width = 1;
		if (height <= 0) height = 1;

		resize(width, height);
		}
		}
		*/
		checkGlError(L"Post render");
		frames++;
		pause = !isClientSide() && screen != NULL && screen->isPauseScreen();

		while (System::currentTimeMillis() >= lastTime + 1000)
		{
			fpsString = std::to_wstring(frames) + L" fps, " + std::to_wstring(Chunk::updates) + L" chunk updates";
			Chunk::updates = 0;
			lastTime += 1000;
			frames = 0;
		}
		/*
		} catch (LevelConflictException e) {
		this.level = null;
		setLevel(null);
		setScreen(new LevelConflictScreen());
		} catch (OutOfMemoryError e) {
		emergencySave();
		setScreen(new OutOfMemoryScreen());
		System.gc();
		}
		*/
	}
	/*
	} catch (StopGameException e) {
	} catch (Throwable e) {
	emergencySave();
	e.printStackTrace();
	crash(new CrashReport("Unexpected error", e));
	} finally {
	destroy();
	}
	*/
	destroy();
}
#endif

void Minecraft::run()
{
	running = true;
	//    try {	// 4J - removed try/catch
	init();
	//    } catch (Exception e) {
	//        e.printStackTrace();
	//       crash(new CrashReport("Failed to start game", e));
	//        return;
	//    }
	//    try {	// 4J - removed try/catch
	}

// 4J added - Selects which local player is currently active for processing by the existing minecraft code
bool Minecraft::setLocalPlayerIdx(int idx)
{
	localPlayerIdx = idx;
	// If the player is not null, but the game mode is then this is just a temp player
	// whose only real purpose is to hold the viewport position
	if( localplayers[idx] == NULL || localgameModes[idx] == NULL ) return false;

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
	// Find out how many viewports we'll be needing
	int viewportsRequired = 0;
	for( int i = 0; i < XUSER_MAX_COUNT; i++ )
	{
		if( localplayers[i] != NULL ) viewportsRequired++;
	}
	if( viewportsRequired == 3 ) viewportsRequired = 4;

	// Allocate away...
	if( viewportsRequired == 1 )
	{
		// Single viewport
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != NULL ) localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
		}
	}
	else if( viewportsRequired == 2 )
	{
		// Split screen - TODO - option for vertical/horizontal split
		int found = 0;
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != NULL )
			{
				// Primary player settings decide what the mode is
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
		// Quadrants - this is slightly more complicated. We don't want to move viewports around if we are going from 3 to 4, or 4 to 3 players,
		// so persist any allocations for quadrants that already exist.
		bool quadrantsAllocated[4] = {false,false,false,false};

		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != NULL )
			{

				// 4J Stu - If the game hasn't started, ignore current allocations (as the players won't have seen them)
				// This fixes an issue with the primary player being the 4th controller quadrant, but ending up in the 3rd viewport.
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
					// Reset the viewport so that it can be assigned in the next loop
					localplayers[i]->m_iScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
				}
			}
		}

		// Found which quadrants are currently in use, now allocate out any spares that are required
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( localplayers[i] != NULL )
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
		// If there's an unoccupied quadrant, record which one so we can clear it to black when rendering
		for( int i = 0; i < XUSER_MAX_COUNT; i++ )
		{
			if( quadrantsAllocated[i] == false )
			{
				unoccupiedQuadrant = i;
			}
		}
	}

	// 4J Stu - If the game is not running we do not want to do this yet, and should wait until the task
	// that caused the app to not be running is finished
	if(app.GetGameStarted())ui.UpdatePlayerBasePositions();
}

// Add a temporary player so that the viewports get re-arranged, and add the player to the game session
bool Minecraft::addLocalPlayer(int idx)
{
	//int iLocalPlayerC=app.GetLocalPlayerCount();
	if( m_pendingLocalConnections[idx] != NULL )
	{
		// 4J Stu - Should we ever be in a state where this happens?
		assert(false);
		m_pendingLocalConnections[idx]->close();
	}
	m_connectionFailed[idx] = false;
	m_pendingLocalConnections[idx] = NULL;

	bool success=g_NetworkManager.AddLocalPlayerByUserIndex(idx);

	if(success)
	{
		app.DebugPrintf("Adding temp local player on pad %d\n", idx);
		localplayers[idx] = shared_ptr<MultiplayerLocalPlayer>( new MultiplayerLocalPlayer(this, level, user, NULL ) );
		localgameModes[idx] = NULL;

		updatePlayerViewportAssignments();

#ifdef _XBOX
		// tell the xui scenes a splitscreen player joined
		XUIMessage xuiMsg;
		CustomMessage_Splitscreenplayer_Struct myMsgData;
		CustomMessage_Splitscreenplayer( &xuiMsg, &myMsgData, true);

		// send the message
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			//if((i!=idx) && (localplayers[i]!=NULL))
			{
				XuiBroadcastMessage( CXuiSceneBase::GetPlayerBaseScene(i), &xuiMsg );
			}
		}
#endif

		ConnectionProgressParams *param = new ConnectionProgressParams();
		param->iPad = idx;
		param->stringId = IDS_PROGRESS_CONNECTING;
		param->showTooltips = true;
		param->setFailTimer = true;
		param->timerTime = CONNECTING_PROGRESS_CHECK_TIME;

		// Joining as second player so always the small progress
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

shared_ptr<MultiplayerLocalPlayer> Minecraft::createExtraLocalPlayer(int idx, const wstring& name, int iPad, int iDimension, ClientConnection *clientConnection /*= NULL*/,MultiPlayerLevel *levelpassedin)
{
	if( clientConnection == NULL) return nullptr;

	if( clientConnection == m_pendingLocalConnections[idx] )
	{
		int tempScreenSection = C4JRender::VIEWPORT_TYPE_FULLSCREEN;
		if( localplayers[idx] != NULL && localgameModes[idx] == NULL )
		{
			// A temp player displaying a connecting screen
			tempScreenSection = localplayers[idx]->m_iScreenSection;
		}
		wstring prevname = user->name;
		user->name = name;

		// Don't need this any more
		m_pendingLocalConnections[idx] = NULL;

		// Add the connection to the level which will now take responsibility for ticking it
		// 4J-PB - can't use the dimension from localplayers[idx], since there may be no localplayers at this point
		//MultiPlayerLevel *mpLevel = (MultiPlayerLevel *)getLevel( localplayers[idx]->dimension );

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
		// check if we're in the trial version
		else if(ProfileManager.IsFullVersion()==false)
		{
			localgameModes[idx] = new TrialMode(idx, this, clientConnection);
		}
		else
		{
			localgameModes[idx] = new ConsoleGameMode(idx, this, clientConnection);
		}

		// 4J-PB - can't do this here because they use a render context, but this is running from a thread.
		// Moved the creation of these into the main thread, before level launch
		//localitemInHandRenderers[idx] = new ItemInHandRenderer(this);
		localplayers[idx] = localgameModes[idx]->createPlayer(level);

		PlayerUID playerXUIDOffline = INVALID_XUID;
		PlayerUID playerXUIDOnline = INVALID_XUID;
		ProfileManager.GetXUID(idx,&playerXUIDOffline,false);
		ProfileManager.GetXUID(idx,&playerXUIDOnline,true);
#ifdef _WINDOWS64
		// Compatibility rule for Win64 id migration
		// host keeps legacy host XUID, non-host uses persistent uid.dat XUID.
		INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(idx);
		if(localNetworkPlayer != NULL && localNetworkPlayer->IsHost())
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

		if( levelpassedin == NULL)	level->addEntity(localplayers[idx]);	// Don't add if we're passing the level in, we only do this from the client connection & we'll be handling adding it ourselves

		localplayers[idx]->SetXboxPad(iPad);

		if( localplayers[idx]->input != NULL ) delete localplayers[idx]->input;
		localplayers[idx]->input = new Input();

		localplayers[idx]->resetPos();

		levelRenderer->setLevel(idx, level);
		localplayers[idx]->level = level;

		user->name = prevname;

		updatePlayerViewportAssignments();

		// Fix for #105852 - TU12: Content: Gameplay: Local splitscreen Players are spawned at incorrect places after re-joining previously saved and loaded "Mass Effect World".
		// Move this check to ClientConnection::handleMovePlayer
//		// 4J-PB - can't call this when this function is called from the qnet thread (GetGameStarted will be false)
//		if(app.GetGameStarted())
//		{
//			ui.CloseUIScenes(idx);
//		}
	}

	return localplayers[idx];
}

// on a respawn of the local player, just store them
void Minecraft::storeExtraLocalPlayer(int idx)
{
	localplayers[idx] = player;

	if( localplayers[idx]->input != NULL ) delete localplayers[idx]->input;
	localplayers[idx]->input = new Input();

	if(ProfileManager.IsSignedIn(idx))
	{
		localplayers[idx]->name = convStringToWstring( ProfileManager.GetGamertag(idx) );
	}
}

void Minecraft::removeLocalPlayerIdx(int idx)
{
	bool updateXui = true;
	if(localgameModes[idx] != NULL)
	{
		if( getLevel( localplayers[idx]->dimension )->isClientSide )
		{
			shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
			( (MultiPlayerLevel *)getLevel( localplayers[idx]->dimension ) )->removeClientConnection(mplp->connection, true);
			delete mplp->connection;
			mplp->connection = NULL;
			g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
		}
		getLevel( localplayers[idx]->dimension )->removeEntity(localplayers[idx]);

#ifdef _XBOX
		// 4J Stu - Fix for #12368 - Crash: Game crashes when saving then exiting and selecting to save
		app.TutorialSceneNavigateBack(idx);
#endif

		// 4J Stu - Fix for #13257 - CRASH: Gameplay: Title crashed after exiting the tutorial
		// It doesn't matter if they were in the tutorial already
		playerLeftTutorial( idx );

		delete localgameModes[idx];
		localgameModes[idx] = NULL;
	}
	else if( m_pendingLocalConnections[idx] != NULL )
	{
		m_pendingLocalConnections[idx]->sendAndDisconnect( shared_ptr<DisconnectPacket>( new DisconnectPacket(DisconnectPacket::eDisconnect_Quitting) ) );;
		delete m_pendingLocalConnections[idx];
		m_pendingLocalConnections[idx] = NULL;
		g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
	}
	else
	{
		// Not sure how this works on qnet, but for other platforms, calling RemoveLocalPlayerByUserIndex won't do anything if there isn't a local user to remove
		// Now just updating the UI directly in this case
#ifdef _XBOX
		// 4J Stu - A signout early in the game creation before this player has connected to the game server
		updateXui = false;
#endif
		// 4J Stu - Adding this back in for exactly the reason my comment above suggests it was added in the first place
#if defined(_XBOX_ONE) || defined(__ORBIS__)
		g_NetworkManager.RemoveLocalPlayerByUserIndex(idx);
#endif
	}
	localplayers[idx] = nullptr;

	if( idx == ProfileManager.GetPrimaryPad() )
	{
		// We should never try to remove the Primary player in this way
		assert(false);
		/*
		// If we are removing the primary player then there can't be a valid gamemode left anymore, this
		// pointer will be referring to the one we've just deleted
		gameMode = NULL;
		// Remove references to player
		player = NULL;
		cameraTargetPlayer = NULL;
		EntityRenderDispatcher::instance->cameraEntity = NULL;
		TileEntityRenderDispatcher::instance->cameraEntity = NULL;
		*/
	}
	else if( updateXui )
	{
		gameRenderer->DisableUpdateThread();
		levelRenderer->setLevel(idx, NULL);
		gameRenderer->EnableUpdateThread();
		ui.CloseUIScenes(idx,true);
		updatePlayerViewportAssignments();
	}

	// We only create these once ever so don't delete it here
	//delete localitemInHandRenderers[idx];
}

void Minecraft::createPrimaryLocalPlayer(int iPad)
{
	localgameModes[iPad] = gameMode;
	localplayers[iPad] = player;
	//gameRenderer->itemInHandRenderer = localitemInHandRenderers[iPad];
	// Give them the gamertag if they're signed in
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
	if (level == NULL) return;

	for (int i = 0; i < XUSER_MAX_COUNT; i++)
	{
		if (localplayers[i] == NULL) continue;
		int iPad = localplayers[i]->GetXboxPad();
		if (iPad != 0) continue;  // Mouse only applies to pad 0

		if (!g_KBMInput.IsMouseGrabbed()) continue;
		if (localgameModes[iPad] == NULL) continue;

		float rawDx, rawDy;
		g_KBMInput.ConsumeMouseDelta(rawDx, rawDy);
		if (rawDx == 0.0f && rawDy == 0.0f) continue;

		float mouseSensitivity = ((float)app.GetGameSettings(iPad, eGameSetting_Sensitivity_InGame)) / 100.0f;
		float mdx = rawDx * mouseSensitivity;
		float mdy = -rawDy * mouseSensitivity;
		if (app.GetGameSettings(iPad, eGameSetting_ControlInvertLook))
			mdy = -mdy;

		// Apply 0.15f scaling (same as Entity::interpolateTurn / Entity::turn)
		float dyaw = mdx * 0.15f;
		float dpitch = -mdy * 0.15f;

		// Apply to both current and old rotation so render interpolation
		// reflects the change immediately (no 50ms tick delay)
		localplayers[i]->yRot += dyaw;
		localplayers[i]->yRotO += dyaw;
		localplayers[i]->xRot += dpitch;
		localplayers[i]->xRotO += dpitch;

		// Clamp pitch
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

		//while (running)
		{
			//        try {	// 4J - removed try/catch
			//            if (minecraftApplet != null && !minecraftApplet.isActive()) break;	// 4J - removed
			AABB::resetPool();
			Vec3::resetPool();

			//            if (parent == NULL && Display.isCloseRequested()) {		// 4J - removed
			//                stop();
			//            }

			// 4J-PB - AUTOSAVE TIMER - only in the full game and if the player is the host
			if(level!=NULL && ProfileManager.IsFullVersion() && g_NetworkManager.IsHost())
			{
				/*if(!bAutosaveTimerSet)
				{
				// set the timer
				bAutosaveTimerSet=true;

				app.SetAutosaveTimerTime();
				}
				else*/
				{
					// if the pause menu is up for the primary player, don't autosave
					// If saving isn't disabled, and the main player has a app action running , or has any crafting or containers open, don't autosave
					if(!StorageManager.GetSaveDisabled() && (app.GetXuiAction(ProfileManager.GetPrimaryPad())==eAppAction_Idle) )
					{
						if(!ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()) && !ui.IsIgnoreAutosaveMenuDisplayed(ProfileManager.GetPrimaryPad()))
						{
							// check if the autotimer countdown has reached zero
							unsigned char ucAutosaveVal=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_Autosave);
							bool bTrialTexturepack=false;
							if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
							{
								TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
								DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

								DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();

								if( pDLCPack )
								{
									if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
									{
										bTrialTexturepack=true;
									}
								}
							}

							// If the autosave value is not zero, and the player isn't using a trial texture pack, then check whether we need to save this tick
							if((ucAutosaveVal!=0) && !bTrialTexturepack)
							{
								if(app.AutosaveDue())
								{
									// disable the autosave countdown
									ui.ShowAutosaveCountdownTimer(false);

									// Need to save now
									app.DebugPrintf("+++++++++++\n");
									app.DebugPrintf("+++Autosave\n");
									app.DebugPrintf("+++++++++++\n");
									app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_AutosaveSaveGame);
									//app.SetAutosaveTimerTime();
#ifndef _CONTENT_PACKAGE
									{
										// print the time
										SYSTEMTIME UTCSysTime;
										GetSystemTime( &UTCSysTime );
										//char szTime[15];

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
							// disable the autosave countdown
							ui.ShowAutosaveCountdownTimer(false);
						}
					}
				}
			}

			// 4J-PB - Once we're in the level, check if the players have the level in their banned list and ask if they want to play it
			for( int i = 0; i < XUSER_MAX_COUNT; i++ )
			{
				if( localplayers[i] && (app.GetBanListCheck(i)==false) && !Minecraft::GetInstance()->isTutorial() && ProfileManager.IsSignedInLive(i) && !ProfileManager.IsGuest(i) )
				{
					// If there is a sys ui displayed, we can't display the message box here, so ignore until we can
					if(!ProfileManager.IsSystemUIDisplayed())
					{
						app.SetBanListCheck(i,true);
						// 4J-PB - check if the level is in the banned level list
						// get the unique save name and xuid from whoever is the host
#if defined _XBOX || defined _XBOX_ONE
						INetworkPlayer *pHostPlayer = g_NetworkManager.GetHostPlayer();

#ifdef _XBOX
						PlayerUID xuid=((NetworkPlayerXbox *)pHostPlayer)->GetUID();
#else
						PlayerUID xuid=pHostPlayer->GetUID();
#endif

						if(app.IsInBannedLevelList(i,xuid,app.GetUniqueMapName()))
						{
							// put up a message box asking if the player would like to unban this level
							app.DebugPrintf("This level is banned\n");
							// set the app action to bring up the message box to give them the option to remove from the ban list or exit the level
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

			// When we go into the first loaded level, check if the console has active joypads that are not in the game, and bring up the quadrant display to remind them to press start (if the session has space)
			if(level!=NULL && bFirstTimeIntoGame && g_NetworkManager.SessionHasSpace())
			{
				// have a short delay before the display
				if(iFirstTimeCountdown==0)
				{
					bFirstTimeIntoGame=false;

					if(app.IsLocalMultiplayerAvailable())
					{
						for( int i = 0; i < XUSER_MAX_COUNT; i++ )
						{
							if((localplayers[i] == NULL) && InputManager.IsPadConnected(i))
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
			// 4J-PB - store any button toggles for the players, since the minecraft::tick may not be called if we're running fast, and a button press and release will be missed

			for( int i = 0; i < XUSER_MAX_COUNT; i++ )
			{
#ifdef __ORBIS__
				if ( m_pPsPlusUpsell != NULL && m_pPsPlusUpsell->hasResponse() && m_pPsPlusUpsell->m_userIndex == i )
				{
					delete m_pPsPlusUpsell;
					m_pPsPlusUpsell = NULL;

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
					// 4J-PB - add these to check for coming out of idle
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_JUMP))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_JUMP;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_USE))					localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_USE;

					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_INVENTORY))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_INVENTORY;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_ACTION))					localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_ACTION;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_CRAFTING))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_CRAFTING;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_PAUSEMENU))
					{
						localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_PAUSEMENU;
						app.DebugPrintf("PAUSE PRESSED - ipad = %d, Storing press\n",i);
					}
#ifdef _DURANGO
					if(InputManager.ButtonPressed(i, ACTION_MENU_GTC_PAUSE))					localplayers[i]->ullButtonsPressed|=1LL<<ACTION_MENU_GTC_PAUSE;
#endif
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DROP))					localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_DROP;

					// 4J-PB - If we're flying, the sneak needs to be held on to go down
					if(localplayers[i]->abilities.flying)
					{
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_SNEAK_TOGGLE))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
					}
					else
					{
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_SNEAK_TOGGLE))		localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
					}
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_RENDER_THIRD_PERSON))		localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_THIRD_PERSON;
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_GAME_INFO))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_GAME_INFO;

#ifdef _WINDOWS64
					// Keyboard/mouse button presses for player 0
					if (i == 0)
					{
						if (g_KBMInput.IsKBMActive())
						{
							if(g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_ACTION;

							if(g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_RIGHT))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_USE;

							if(g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_INVENTORY))
							{
								if(ui.IsSceneInStack(i, eUIScene_InventoryMenu))
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
							if(ui.IsSceneInStack(i, eUIScene_Crafting2x2Menu) || ui.IsSceneInStack(i, eUIScene_Crafting3x3Menu) || ui.IsSceneInStack(i, eUIScene_CreativeMenu))
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

						// Utility keys always work regardless of KBM active state
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

						// In flying mode, Shift held = sneak/descend
						if(g_KBMInput.IsKBMActive() && g_KBMInput.IsKeyDown(KeyboardMouseInput::KEY_SNEAK))
						{
							if (localplayers[i]->abilities.flying && !ui.GetMenuDisplayed(i))
								localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SNEAK_TOGGLE;
						}
					}
#endif

#if _DEBUG // ndef _FINAL_BUILD // Disable conflicting debug functionality in release builds
					if( app.DebugSettingsOn() && app.GetUseDPadForDebug() )
					{
						localplayers[i]->ullDpad_last = 0;
						localplayers[i]->ullDpad_this = 0;
						localplayers[i]->ullDpad_filtered = 0;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_RIGHT))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_CHANGE_SKIN;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_UP))				localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_FLY_TOGGLE;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_DOWN))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_RENDER_DEBUG;
						if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_DPAD_LEFT))			localplayers[i]->ullButtonsPressed|=1LL<<MINECRAFT_ACTION_SPAWN_CREEPER;
					}
					else
#endif
					{
						// Movement on DPAD is stored ulimately into ullDpad_filtered - this ignores any diagonals pressed, instead reporting the last single direction - otherwise
						// we get loads of accidental diagonal movements

						localplayers[i]->ullDpad_this = 0;
						int dirCount = 0;

#ifndef __PSVITA__
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_LEFT))					{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_LEFT;		dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_RIGHT))					{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_RIGHT;		dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_UP))					{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_UP;			dirCount++; }
						if(InputManager.ButtonDown(i, MINECRAFT_ACTION_DPAD_DOWN))					{ localplayers[i]->ullDpad_this|=1LL<<MINECRAFT_ACTION_DPAD_DOWN;		dirCount++; }
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

					// for the opacity timer
					if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_LEFT_SCROLL) || InputManager.ButtonPressed(i, MINECRAFT_ACTION_RIGHT_SCROLL))
						//InputManager.ButtonPressed(i, MINECRAFT_ACTION_USE) || InputManager.ButtonPressed(i, MINECRAFT_ACTION_ACTION))
					{
						app.SetOpacityTimer(i);
					}
				}
				else
				{
					// 4J Stu - This doesn't make any sense with the way we handle XboxOne users
#ifndef _DURANGO
					// did we just get input from a player who doesn't exist? They'll be wanting to join the game then
#ifdef _WINDOWS64
					// The 4J toggle system is unreliable here: UIController::handleInput() calls
					// ButtonPressed for every ACTION_MENU_* mapped button (which covers all physical
					// buttons) before run_middle() runs. Bypass it with raw XInput and own edge detection.
					// A latch counter keeps startJustPressed active for ~120 frames after the rising edge
					// so the detection window is large enough to be caught reliably.
					static WORD s_prevXButtons[XUSER_MAX_COUNT] = {};
					static int  s_startPressLatch[XUSER_MAX_COUNT] = {};
					XINPUT_STATE xstate_join;
					memset(&xstate_join, 0, sizeof(xstate_join));
					WORD xCurButtons = 0;
					if (XInputGetState(i, &xstate_join) == ERROR_SUCCESS)
					{
						xCurButtons = xstate_join.Gamepad.wButtons;
						if ((xCurButtons & XINPUT_GAMEPAD_START) != 0 && (s_prevXButtons[i] & XINPUT_GAMEPAD_START) == 0)
							s_startPressLatch[i] = 120; // rising edge: latch for ~120 frames (~2s at 60fps)
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
					// Check for remote play
					tryJoin = tryJoin && InputManager.IsLocalMultiplayerAvailable();

					// 4J Stu - Check that content restriction information has been received
					if( !g_NetworkManager.IsLocalGame() )
					{
						tryJoin = tryJoin && ProfileManager.GetChatAndContentRestrictions(i,true,NULL,NULL,NULL);
					}
#endif
					if(tryJoin)
					{
						if(!ui.PressStartPlaying(i))
						{
#ifdef __ORBIS__
							// Don't let player start joining until their PS Plus check has finished
							if (g_NetworkManager.IsLocalGame() || !ProfileManager.RequestingPlaystationPlus(i))
#endif
							{
								ui.ShowPressStart(i);
							}
						}
						else
						{
							// did we just get input from a player who doesn't exist? They'll be wanting to join the game then
#ifdef __ORBIS__
							if(InputManager.ButtonPressed(i, ACTION_MENU_A))
#elif defined _WINDOWS64
							if(startJustPressed)
#else
							if(InputManager.ButtonPressed(i, MINECRAFT_ACTION_PAUSEMENU))
#endif
							{
								// Let them join

								// are they signed in?
#ifdef _WINDOWS64
								if(ProfileManager.IsSignedIn(i) || (g_NetworkManager.IsLocalGame() && InputManager.IsPadConnected(i)))
#else
								if(ProfileManager.IsSignedIn(i))
#endif
								{
									// if this is a local game, then the player just needs to be signed in
									if( g_NetworkManager.IsLocalGame() || (ProfileManager.IsSignedInLive(i) && ProfileManager.AllowedToPlayMultiplayer(i) ) )
									{
#ifdef __ORBIS__
										bool contentRestricted = false;
										ProfileManager.GetChatAndContentRestrictions(i,false,NULL,&contentRestricted,NULL); // TODO!

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
													ProfileManager.GetChatAndContentRestrictions(i,false,&chatRestricted,NULL,NULL);
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
											// create the localplayer
											shared_ptr<Player> player = localplayers[i];
											if( player == NULL)
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
											// 4J Stu - Don't allow converting to guests as we don't allow any guest sign-in while in the game
											// Fix for #66516 - TCR #124: MPS Guest Support ; #001: BAS Game Stability: TU8: The game crashes when second Guest signs-in on console which takes part in Xbox LIVE multiplayer session.
											//ProfileManager.RequestConvertOfflineToGuestUI( &Minecraft::InGame_SignInReturned, this,i);

#ifndef _XBOX
											ui.HidePressStart();
#endif

#ifdef __ORBIS__
											int npAvailability = ProfileManager.getNPAvailability(i);

											// Check if PSN is unavailable because of age restriction
											if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
											{
												UINT uiIDA[1];
												uiIDA[0] = IDS_OK;
												ui.RequestErrorMessage(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, i);
											}
											else if (ProfileManager.IsSignedIn(i) && !ProfileManager.IsSignedInLive(i))
											{
												// You're not signed in to PSN!
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
										//else
										{
											// player not signed in to live
											// bring up the sign in dialog
											app.DebugPrintf("Bringing up the sign in ui\n");
											ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,i);
										}
									}
								}
								else
								{
									// bring up the sign in dialog
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
			// did we just get input from a player who doesn't exist? They'll be wanting to join the game then
			if(!pause && !ui.IsIgnorePlayerJoinMenuDisplayed(ProfileManager.GetPrimaryPad()) && g_NetworkManager.SessionHasSpace() && RenderManager.IsHiDef() )
			{
				int firstEmptyUser = 0;
				for( int i = 0; i < XUSER_MAX_COUNT; i++ )
				{
					if(localplayers[i] == NULL)
					{
						firstEmptyUser = i;
						break;
					}
				}

				// For durango, check for unmapped controllers
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
						// did we just get input from a player who doesn't exist? They'll be wanting to join the game then
						if(InputManager.ButtonPressed(iPad, MINECRAFT_ACTION_PAUSEMENU))
						{
							// bring up the sign in dialog
							app.DebugPrintf("Bringing up the sign in ui\n");
							ProfileManager.RequestSignInUI(false, g_NetworkManager.IsLocalGame(), true, false,true,&Minecraft::InGame_SignInReturned, this,iPad);

							// 4J Stu - If we are joining a pad here, then we don't want to try and join any others
							break;
						}
					}
				}
			}
#endif

			if (pause && level != NULL)
			{
				float lastA = timer->a;
				timer->advanceTime();
				timer->a = lastA;
			}
			else
			{
				timer->advanceTime();
			}

			//int64_t beforeTickTime = System::nanoTime();
			for (int i = 0; i < timer->ticks; i++)
			{
				bool bLastTimerTick = ( i == ( timer->ticks - 1 ) );
				// 4J-PB - the tick here can run more than once, and this is a problem for our input, which would see the a key press twice with the same time - let's tick the inputmanager again
				if(i!=0)
				{
					InputManager.Tick();
					app.HandleButtonPresses();
				}

				ticks++;
				//            try {		// 4J - try/catch removed
				bool bFirst = true;
				for( int idx = 0; idx < XUSER_MAX_COUNT; idx++ )
				{
					// 4J - If we are waiting for this connection to do something, then tick it here.
					// This replaces many of the original Java scenes which would tick the connection while showing that scene
					if( m_pendingLocalConnections[idx] != NULL )
					{
						m_pendingLocalConnections[idx]->tick();
					}

					// reset the player inactive tick
					if(localplayers[idx]!=NULL)
					{
						// any input received?
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
						// clear the stored button downs since the tick for this player will now have actioned them
						player->ullButtonsPressed=0LL;
					}
				}

				ui.HandleGameTick();

				setLocalPlayerIdx(ProfileManager.GetPrimaryPad());

				// 4J - added - now do the equivalent of level::animateTick, but taking into account the positions of all our players

				for( int l = 0; l < levels.length; l++ )
				{
					if( levels[l] )
					{
						levels[l]->animateTickDoWork();
					}
				}

				//            } catch (LevelConflictException e) {
				//                this.level = null;
				//                setLevel(null);
				//                setScreen(new LevelConflictScreen());
				//            }
// 				SparseLightStorage::tick();	// 4J added
// 				CompressedTileStorage::tick();	// 4J added
// 				SparseDataStorage::tick();		// 4J added
			}
			//int64_t tickDuraction = System::nanoTime() - beforeTickTime;
			MemSect(31);
			checkGlError(L"Pre render");
			MemSect(0);

			TileRenderer::fancy = options->fancyGraphics;

			// if (pause) timer.a = 1;

			PIXBeginNamedEvent(0,"Sound engine update");
			soundEngine->tick((shared_ptr<Mob> *)localplayers, timer->a);
			PIXEndNamedEvent();

			PIXBeginNamedEvent(0,"Light update");

			glEnable(GL_TEXTURE_2D);

			PIXEndNamedEvent();

			//        if (!Keyboard::isKeyDown(Keyboard.KEY_F7)) Display.update();		// 4J - removed

			// 4J-PB - changing this to be per player
			//if (player != NULL && player->isInWall()) options->thirdPersonView = false;
			if (player != NULL && player->isInWall()) player->SetThirdPersonView(0);

			if (!noRender)
			{
				bool bFirst = true;
				int iPrimaryPad=ProfileManager.GetPrimaryPad();
				for( int i = 0; i < XUSER_MAX_COUNT; i++ )
				{
					if( setLocalPlayerIdx(i) )
					{
						PIXBeginNamedEvent(0,"Game render player idx %d",i);
						RenderManager.StateSetViewport((C4JRender::eViewportType)player->m_iScreenSection);
						gameRenderer->render(timer->a, bFirst);
						bFirst = false;
						PIXEndNamedEvent();

						if(i==iPrimaryPad)
						{
#ifdef __ORBIS__
							// PS4 does much of the screen-capturing for every frame, to simplify the synchronisation when we actually want a capture. This call tells it the point in the frame to do it.
							RenderManager.InternalScreenCapture();
#endif
							// check to see if we need to capture a screenshot for the save game thumbnail
							switch(app.GetXuiAction(i))
							{
							case eAppAction_ExitWorldCapturedThumbnail:
							case eAppAction_SaveGameCapturedThumbnail:
							case eAppAction_AutosaveSaveGameCapturedThumbnail:
								// capture the save thumbnail
								app.CaptureSaveThumbnail();
								break;
							}
						}
					}
				}
				// If there's an unoccupied quadrant, then clear that to black
				if( unoccupiedQuadrant > -1 )
				{
					// render a logo
					RenderManager.StateSetViewport((C4JRender::eViewportType)(C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + unoccupiedQuadrant));
					glClearColor(0, 0, 0, 0);
					glClear(GL_COLOR_BUFFER_BIT);

					ui.SetEmptyQuadrantLogo(C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT + unoccupiedQuadrant);
				}
				setLocalPlayerIdx(iPrimaryPad);
				RenderManager.StateSetViewport(C4JRender::VIEWPORT_TYPE_FULLSCREEN);

#ifdef _XBOX
				// Do we need to capture a screenshot for a social post?
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

			/*	4J - removed
			if (!Display::isActive())
			{
			if (fullscreen)
			{
			this->toggleFullScreen();
			}
			Sleep(10);
			}
			*/

#if PACKET_ENABLE_STAT_TRACKING
			Packet::updatePacketStatsPIX();
#endif

			if (options->renderDebug)
			{
				//renderFpsMeter(tickDuraction);

#if DEBUG_RENDER_SHOWS_PACKETS
				// To show data for only one packet type
				//Packet::renderPacketStats(31);

				// To show data for all packet types selected as being renderable in the Packet:static_ctor call to Packet::map
				Packet::renderAllPacketStats();
#else
				// To show the size of the QNet queue in bytes and messages
				g_NetworkManager.renderQueueMeter();
#endif
			}
			else
			{
				lastTimer = System::nanoTime();
			}

			achievementPopup->render();

			PIXBeginNamedEvent(0,"Sleeping");
			Sleep(0);	// 4J - was Thread.yield()
			PIXEndNamedEvent();

			//        if (Keyboard::isKeyDown(Keyboard::KEY_F7)) Display.update();	// 4J - removed condition
			PIXBeginNamedEvent(0,"Display update");
			Display::update();
			PIXEndNamedEvent();

			//        checkScreenshot();	// 4J - removed

			/* 4J - removed
			if (parent != NULL && !fullscreen)
			{
			if (parent.getWidth() != width || parent.getHeight() != height)
			{
			width = parent.getWidth();
			height = parent.getHeight();
			if (width <= 0) width = 1;
			if (height <= 0) height = 1;

			resize(width, height);
			}
			}
			*/
			MemSect(31);
			checkGlError(L"Post render");
			MemSect(0);
			frames++;
			//pause = !isClientSide() && screen != NULL && screen->isPauseScreen();
			//pause = g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 && app.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad());
			pause = app.IsAppPaused();

#ifndef _CONTENT_PACKAGE
			while (System::nanoTime() >= lastTime + 1000000000)
			{
				MemSect(31);
				fpsString = std::to_wstring(frames) + L" fps, " + std::to_wstring(Chunk::updates) + L" chunk updates";
				MemSect(0);
				Chunk::updates = 0;
				lastTime += 1000000000;
				frames = 0;
			}
#endif
			/*
			} catch (LevelConflictException e) {
			this.level = null;
			setLevel(null);
			setScreen(new LevelConflictScreen());
			} catch (OutOfMemoryError e) {
			emergencySave();
			setScreen(new OutOfMemoryScreen());
			System.gc();
			}
			*/
		}
		/*
		} catch (StopGameException e) {
		} catch (Throwable e) {
		emergencySave();
		e.printStackTrace();
		crash(new CrashReport("Unexpected error", e));
		} finally {
		destroy();
		}
		*/
	}
	LeaveCriticalSection(&m_setLevelCS);
}

void Minecraft::run_end()
{
	destroy();
}

void Minecraft::emergencySave()
{
	// 4J - lots of try/catches removed here, and garbage collector things
	levelRenderer->clear();
	AABB::clearPool();
	Vec3::clearPool();
	setLevel(NULL);
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
	glOrtho(0, (float)width, (float)height, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();
	t->begin(GL_QUADS);
	int hh1 = (int) (nsPer60Fps / 200000);
	t->color(0x20000000);
	t->vertex((float)(0), (float)( height - hh1), (float)( 0));
	t->vertex((float)(0), (float)( height), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height - hh1), (float)( 0));

	t->color(0x20200000);
	t->vertex((float)(0), (float)( height - hh1 * 2), (float)( 0));
	t->vertex((float)(0), (float)( height - hh1), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height - hh1), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height - hh1 * 2), (float)( 0));

	t->end();
	int64_t totalTime = 0;
	for (int i = 0; i < Minecraft::frameTimes_length; i++)
	{
		totalTime += Minecraft::frameTimes[i];
	}
	int hh = (int) (totalTime / 200000 / Minecraft::frameTimes_length);
	t->begin(GL_QUADS);
	t->color(0x20400000);
	t->vertex((float)(0), (float)( height - hh), (float)( 0));
	t->vertex((float)(0), (float)( height), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height), (float)( 0));
	t->vertex((float)(Minecraft::frameTimes_length), (float)( height - hh), (float)( 0));
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

		t->vertex((float)(i + 0.5f), (float)( height - time + 0.5f), (float)( 0));
		t->vertex((float)(i + 0.5f), (float)( height + 0.5f), (float)( 0));

		// if (Minecraft.frameTimes[i]>nsPer60Fps) {
		t->color(0xff000000 + cc * 65536 + cc * 256 + cc * 1);
		// } else {
		// t.color(0xff808080 + cc/2 * 256);
		// }
		t->vertex((float)(i + 0.5f), (float)( height - time + 0.5f), (float)( 0));
		t->vertex((float)(i + 0.5f), (float)( height - (time - time2) + 0.5f), (float)( 0));
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}

void Minecraft::stop()
{
	running = false;
	// keepPolling = false;
}

void Minecraft::pauseGame()
{
	if (screen != NULL) return;

	//    setScreen(new PauseScreen());	// 4J - TODO put back in
}

void Minecraft::resize(int width, int height)
{
	if (width <= 0) width = 1;
	if (height <= 0) height = 1;
	this->width = width;
	this->height = height;

	if (screen != NULL)
	{
		ScreenSizeCalculator ssc(options, width, height);
		int screenWidth = ssc.getWidth();
		int screenHeight = ssc.getHeight();
		//        screen->init(this, screenWidth, screenHeight);	// 4J - TODO - put back in
	}
}

void Minecraft::verify()
{
	/* 4J - TODO
	new Thread() {
	public void run() {
	try {
	HttpURLConnection huc = (HttpURLConnection) new URL("https://login.minecraft.net/session?name=" + user.name + "&session=" + user.sessionId).openConnection();
	huc.connect();
	if (huc.getResponseCode() == 400) {
	warezTime = System.currentTimeMillis();
	}
	huc.disconnect();
	} catch (Exception e) {
	e.printStackTrace();
	}
	}
	}.start();
	*/
}




void Minecraft::levelTickUpdateFunc(void* pParam)
{
	Level* pLevel = (Level*)pParam;
	pLevel->tick();
}

void Minecraft::levelTickThreadInitFunc()
{
	AABB::CreateNewThreadStorage();
	Vec3::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Compression::UseDefaultThreadStorage();
}


// 4J - added bFirst parameter, which is true for the first active viewport in splitscreen
// 4J - added bUpdateTextures, which is true if the actual renderer textures are to be updated - this will be true for the last time this tick runs with bFirst true
void Minecraft::tick(bool bFirst, bool bUpdateTextures)
{
	int iPad=player->GetXboxPad();
	//OutputDebugString("Minecraft::tick\n");

	//4J-PB - only tick this player's stats
	stats[iPad]->tick(iPad);

	// Tick the opacity timer (to display the interface at default opacity for a certain time if the user has been navigating it)
	app.TickOpacityTimer(iPad);

	// 4J added
	if( bFirst ) levelRenderer->destroyedTileManager->tick();

	gui->tick();
	gameRenderer->pick(1);
#if 0
	// 4J - removed - we don't use ChunkCache anymore
	if (player != NULL)
	{
		ChunkSource *cs = level->getChunkSource();
		if (dynamic_cast<ChunkCache *>(cs) != NULL)
		{
			ChunkCache *spcc = (ChunkCache *)cs;

			// 4J - there was also Mth::floors on these ints but that seems superfluous
			int xt = ((int) player->x) >> 4;
			int zt = ((int) player->z) >> 4;
			spcc->centerOn(xt, zt);
		}
	}
#endif

	// soundEngine.playMusicTick();

	if (!pause && level != NULL) gameMode->tick();
	MemSect(31);
	glBindTexture(GL_TEXTURE_2D, textures->loadTexture(TN_TERRAIN)); //L"/terrain.png"));
	MemSect(0);
	if( bFirst )
	{
		PIXBeginNamedEvent(0,"Texture tick");
		if (!pause) textures->tick(bUpdateTextures);
		PIXEndNamedEvent();
	}

	/*
	* if (serverConnection != null && !(screen instanceof ErrorScreen)) {
	* if (!serverConnection.isConnected()) {
	* progressRenderer.progressStart("Connecting..");
	* progressRenderer.progressStagePercentage(0); } else {
	* serverConnection.tick(); serverConnection.sendPosition(player); } }
	*/
	if (screen == NULL && player != NULL )
	{
		if (player->getHealth() <= 0 && !ui.GetMenuDisplayed(iPad) )
		{
			setScreen(NULL);
		}
		else if (player->isSleeping() && level != NULL && level->isClientSide)
		{
			//            setScreen(new InBedChatScreen());		// 4J - TODO put back in
		}
	}
	else if (screen != NULL && (dynamic_cast<InBedChatScreen *>(screen)!=NULL) && !player->isSleeping())
	{
		setScreen(NULL);
	}

	if (screen != NULL)
	{
		player->missTime = 10000;
		player->lastClickTick[0] = ticks + 10000;
		player->lastClickTick[1] = ticks + 10000;
	}

	if (screen != NULL)
	{
		screen->updateEvents();
		if (screen != NULL)
		{
			screen->particles->tick();
			screen->tick();
		}
	}

#ifdef _WINDOWS64
	if ((screen != NULL || ui.GetMenuDisplayed(iPad)) && g_KBMInput.IsMouseGrabbed())
	{
		g_KBMInput.SetMouseGrabbed(false);
	}
#endif

	if (screen == NULL && !ui.GetMenuDisplayed(iPad) )
	{
#ifdef _WINDOWS64
		if (!g_KBMInput.IsMouseGrabbed() && g_KBMInput.IsWindowFocused())
		{
			g_KBMInput.SetMouseGrabbed(true);
		}
#endif
		// 4J-PB - add some tooltips if required
		int iA=-1, iB=-1, iX, iY=IDS_CONTROLS_INVENTORY, iLT=-1, iRT=-1, iLB=-1, iRB=-1, iLS=-1, iRS=-1;

		if(player->abilities.instabuild)
		{
			iX=IDS_TOOLTIPS_CREATIVE;
		}
		else
		{
			iX=IDS_CONTROLS_CRAFTING;
		}
		// control scheme remapping can move the Action button, so we need to check this
		int *piAction;
		int *piJump;
		int *piUse;
		int *piAlt;

		unsigned int uiAction = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_ACTION );
		unsigned int uiJump = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_JUMP );
		unsigned int uiUse = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_USE );
		unsigned int uiAlt = InputManager.GetGameJoypadMaps(InputManager.GetJoypadMapVal( iPad ) ,MINECRAFT_ACTION_SNEAK_TOGGLE );

		// Also need to handle PS3 having swapped triggers/bumpers
		switch(uiAction)
		{
		case _360_JOY_BUTTON_RT:
			piAction=&iRT;
			break;
		case _360_JOY_BUTTON_LT:
			piAction=&iLT;
			break;
		case _360_JOY_BUTTON_LB:
			piAction=&iLB;
			break;
		case _360_JOY_BUTTON_RB:
			piAction=&iRB;
			break;
		case _360_JOY_BUTTON_A:
		default:
			piAction=&iA;
			break;
		}

		switch(uiJump)
		{
		case _360_JOY_BUTTON_LT:
			piJump=&iLT;
			break;
		case _360_JOY_BUTTON_RT:
			piJump=&iRT;
			break;
		case _360_JOY_BUTTON_LB:
			piJump=&iLB;
			break;
		case _360_JOY_BUTTON_RB:
			piJump=&iRB;
			break;
		case _360_JOY_BUTTON_A:
		default:
			piJump=&iA;
			break;
		}

		switch(uiUse)
		{
		case _360_JOY_BUTTON_LB:
			piUse=&iLB;
			break;
		case _360_JOY_BUTTON_RB:
			piUse=&iRB;
			break;
		case _360_JOY_BUTTON_LT:
			piUse=&iLT;
			break;
		case _360_JOY_BUTTON_RT:
		default:
			piUse=&iRT;
			break;
		}

		switch(uiAlt)
		{
		default:
		case _360_JOY_BUTTON_LSTICK_RIGHT:
			piAlt=&iRS;
			break;

			//TODO
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

		// 4J-PB another special case for when the player is sleeping in a bed
		if (player->isSleeping() && (level != NULL) && level->isClientSide)
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

			// no hit result, but we may have something in our hand that we can do something with
			shared_ptr<ItemInstance> itemInstance = player->inventory->getSelected();

			// 4J-JEV: Moved all this here to avoid having it in 3 different places.
			if (itemInstance)
			{
				// 4J-PB - very special case for boat and empty bucket and glass bottle and more
				bool bUseItem = gameMode->useItem(player, level, itemInstance, true);

				switch (itemInstance->getItem()->id)
				{
					// food
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
					// Check that we are actually hungry so will eat this item
					{
						FoodItem *food = (FoodItem *)itemInstance->getItem();
						if (food != NULL && food->canEat(player))
						{
							*piUse=IDS_TOOLTIPS_EAT;
						}
					}
					break;

				case Item::bucket_milk_Id:
					*piUse=IDS_TOOLTIPS_DRINK;
					break;

				case Item::fishingRod_Id:	// use
				case Item::emptyMap_Id:
					*piUse=IDS_TOOLTIPS_USE;
					break;

				case Item::egg_Id:			// throw
				case Item::snowBall_Id:
					*piUse=IDS_TOOLTIPS_THROW;
					break;

				case Item::bow_Id:	// draw or release
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
					// This will only work if there is a stronghold in this dimension
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

			if (hitResult!=NULL)
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

						if( gameMode != NULL && gameMode->getTutorial() != NULL )
						{
							// 4J Stu - For the tutorial we want to be able to record what items we look at so that we can give hints
							gameMode->getTutorial()->onLookAt(iTileID,iData);
						}

						// 4J-PB - Call the useItemOn with the TestOnly flag set
						bool bUseItemOn=gameMode->useItemOn(player, level, itemInstance, x, y, z, face, hitResult->pos, true);

						/* 4J-Jev:
						 *	Moved this here so we have item tooltips to fallback on
						 *	for noteblocks, enderportals and flowerpots in case of non-standard items.
						 *	(ie. ignite behaviour)
						 */
						if (bUseItemOn && itemInstance!=NULL)
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

								// Things to USE
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
								// bonemeal grows various plants
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
							*piUse = (Tile::chest->getContainer(level,x,y,z) != NULL) ? IDS_TOOLTIPS_OPEN : -1;
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
							// if in creative mode, we will mine
							if (player->abilities.instabuild)	*piAction=IDS_TOOLTIPS_MINE;
							else								*piAction=IDS_TOOLTIPS_PLAY;
							*piUse=IDS_TOOLTIPS_CHANGEPITCH;
							break;

						case Tile::sign_Id:
							*piAction=IDS_TOOLTIPS_MINE;
							break;

						case Tile::cauldron_Id:
							// special case for a cauldron of water and an empty bottle
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
							if (player->abilities.instabuild) // if in creative mode, we will mine
							{
								*piAction=IDS_TOOLTIPS_MINE;
							}
							else
							{
								if (player->getFoodData()->needsFood() ) // 4J-JEV: Changed from healthto hunger.
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
							if (!bUseItemOn && itemInstance!=NULL)
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
								if (Tile::jukebox->TestUse(level, x, y, z, player)) // means we can eject
								{
									*piUse=IDS_TOOLTIPS_EJECT;
								}
								*piAction=IDS_TOOLTIPS_MINE;
							}
							break;

						case Tile::flowerPot_Id:
							if ( !bUseItemOn && (itemInstance != NULL) && (iData == 0) )
							{
								int iID = itemInstance->getItem()->id;
								if (iID<256) // is it a tile?
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

					if ( (gameMode != NULL) && (gameMode->getTutorial() != NULL) )
					{
						// 4J Stu - For the tutorial we want to be able to record what items we look at so that we can give hints
						gameMode->getTutorial()->onLookAtEntity(hitResult->entity);
					}

					shared_ptr<ItemInstance> heldItem = nullptr;
					if (player->inventory->IsHeldItem())
					{
						heldItem = player->inventory->getSelected();
					}
					int heldItemId = heldItem != NULL ? heldItem->getItem()->id : -1;

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

							case -1: break; // 4J-JEV: Empty hand.
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
									// Things to USE
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

							case -1: break; // 4J-JEV: Empty hand.
								}
						}
						break;
					case eTYPE_MUSHROOMCOW:
						{
							// 4J-PB - Fix for #13081 - No tooltip is displayed for hitting a cow when you have nothing in your hand
							if(player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;

							shared_ptr<Animal> animal = dynamic_pointer_cast<Animal>(hitResult->entity);

							if (animal->isLeashed() && animal->getLeashHolder() == player)
							{
								*piUse=IDS_TOOLTIPS_UNLEASH;
								break;
							}

								// It's an item
							switch(heldItemId)
							{
								// Things to USE
							case Item::nameTag_Id:
								*piUse=IDS_TOOLTIPS_NAME;
								break;

							case Item::lead_Id:
								if (!animal->isLeashed()) *piUse=IDS_TOOLTIPS_LEASH;
								break;

								case Item::bowl_Id:
								case Item::bucket_empty_Id: // You can milk a mooshroom with either a bowl (mushroom soup) or a bucket (milk)!
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

							case -1: break; // 4J-JEV: Empty hand.
							}
						}
						break;

					case eTYPE_BOAT:
						*piAction=IDS_TOOLTIPS_MINE;
						*piUse=IDS_TOOLTIPS_SAIL;
						break;

					case eTYPE_MINECART_RIDEABLE:
						*piAction = IDS_TOOLTIPS_MINE;
						*piUse = IDS_TOOLTIPS_RIDE; // are we in the minecart already? - 4J-JEV: Doesn't matter anymore.
						break;

					case eTYPE_MINECART_FURNACE:
						*piAction = IDS_TOOLTIPS_MINE;

						// if you have coal, it'll go. Is there an object in hand?
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
							// can dye a sheep
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
									// convert to tile-based color value (0 is white instead of black)
									int newColor = ColoredTile::getTileDataForItemAuxValue(heldItem->getAuxValue());

									// can only use a dye on sheep that haven't been sheared
									if(!(sheep->isSheared() && sheep->getColor() != newColor))
									{
										*piUse=IDS_TOOLTIPS_DYE;
									}
								}
								break;
							case Item::shears_Id:
								{
									// can only shear a sheep that hasn't been sheared
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

							case -1: break; // 4J-JEV: Empty hand.
							}
						}
						break;

					case eTYPE_PIG:
						{
							// can ride a pig
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
							else if (pig->hasSaddle()) // does the pig have a saddle?
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
						// can be tamed, fed, and made to sit/stand, or enter love mode
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
									if(wolf->isSitting())
									{
										*piUse=IDS_TOOLTIPS_FOLLOWME;
									}
									else
									{
										*piUse=IDS_TOOLTIPS_SIT;
									}
								}

								break;
							case Item::enderPearl_Id:
								// Use is throw, so don't change the tips for the wolf
								break;
							case Item::dye_powder_Id:
								if (wolf->isTame())
								{
									if (ColoredTile::getTileDataForItemAuxValue(heldItem->getAuxValue()) != wolf->getCollarColor())
									{
										*piUse=IDS_TOOLTIPS_DYECOLLAR;
									}
									else if (wolf->isSitting())
									{
										*piUse=IDS_TOOLTIPS_FOLLOWME;
									}
									else
									{
										*piUse=IDS_TOOLTIPS_SIT;
									}
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
										// break out here
										break;
									}

									if (equalsIgnoreCase(player->getUUID(), wolf->getOwnerUUID()))
									{
										if(wolf->isSitting())
										{
											*piUse=IDS_TOOLTIPS_FOLLOWME;
										}
										else
										{
											*piUse=IDS_TOOLTIPS_SIT;
										}
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
								// 4J-PB - if you have a raw fish in your hand, you will feed the ocelot rather than have it sit/follow
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
									if(ocelot->isSitting())
									{
										*piUse=IDS_TOOLTIPS_FOLLOWME;
									}
									else
									{
										*piUse=IDS_TOOLTIPS_SIT;
									}
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
							// Fix for #58576 - TU6: Content: Gameplay: Hit button prompt is available when attacking a host who has "Invisible" option turned on
							shared_ptr<Player> TargetPlayer = dynamic_pointer_cast<Player>(hitResult->entity);

							if(!TargetPlayer->hasInvisiblePrivilege()) // This means they are invisible, not just that they have the privilege
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

							// is the frame occupied?
							if(itemFrame->getItem()!=NULL)
							{
								// rotate the item
								*piUse=IDS_TOOLTIPS_ROTATE;
							}
							else
							{
								// is there an object in hand?
								if(heldItemId >= 0) *piUse=IDS_TOOLTIPS_PLACE;
							}

							*piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_VILLAGER:
						{
							// 4J-JEV: Cannot leash villagers.

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
							static GoldenAppleItem *goldapple = (GoldenAppleItem *) Item::apple_gold;

							//zomb->hasEffect(MobEffect::weakness) - not present on client.
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
							else if (horse->isBaby()) // 4J-JEV: Can't ride baby horses due to morals.
							{
								if (heldItemIsFood)
								{
									// 4j - Can feed foles to speed growth.
									*piUse = IDS_TOOLTIPS_FEED;
								}
							}
							else if ( !horse->isTamed() )
							{
								if (heldItemId == -1)
								{
									// 4j - Player not holding anything, ride and attempt to break untamed horse.
									*piUse = IDS_TOOLTIPS_TAME;
								}
								else if (heldItemIsFood)
								{
									// 4j - Attempt to make it like you more by feeding it.
									*piUse = IDS_TOOLTIPS_FEED;
								}
							}
							else if (	player->isSneaking()
									||	(heldItemId == Item::saddle_Id)
									||	(horse->canWearArmor() && heldItemIsArmour)
									)
							{
								// 4j - Access horses inventory
								if (*piUse == -1) *piUse = IDS_TOOLTIPS_OPEN;
							}
							else if (	horse->canWearBags()
									&&	!horse->isChestedHorse()
									&&	(heldItemId == Tile::chest_Id) )
							{
								// 4j - Attach saddle-bags (chest) to donkey or mule.
								*piUse = IDS_TOOLTIPS_ATTACH;
							}
							else if (	horse->isReadyForParenting()
									&&	heldItemIsLove )
							{
								// 4j - Different food to mate horses.
								*piUse = IDS_TOOLTIPS_LOVEMODE;
							}
							else if ( heldItemIsFood && (horse->getHealth() < horse->getMaxHealth()) )
							{
								// 4j - Horse is damaged and can eat held item to heal
								*piUse = IDS_TOOLTIPS_HEAL;
							}
							else
							{
								// 4j - Ride tamed horse.
								*piUse = IDS_TOOLTIPS_MOUNT;
							}

							if (player->isAllowedToAttackAnimals()) *piAction=IDS_TOOLTIPS_HIT;
						}
						break;

					case eTYPE_ENDERDRAGON:
						// 4J-JEV: Enderdragon cannot be named.
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

		// 4J-JEV: Don't set tooltips when we're reloading the skin, it'll crash.
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

			if( gameMode != NULL && gameMode->getTutorial() != NULL )
			{
				// 4J Stu - For the tutorial we want to be able to record what items we are using so that we can give hints
				gameMode->getTutorial()->onSelectedItemChanged(player->inventory->getSelected());
			}

			// Update presence
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
				//if(InputManager.ButtonPressed(iPad, MINECRAFT_ACTION_ACTION) )
			{
				//printf("MINECRAFT_ACTION_ACTION ButtonPressed");
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
				//printf("MINECRAFT_ACTION_ACTION ButtonDown");
				player->handleMouseClick(0);
				player->lastClickTick[0] = ticks;
			}

			if(actionHeld)
			{
				player->handleMouseDown(0, true );
			}
			else
			{
				player->handleMouseDown(0, false );
			}
		}

		// 4J Stu - This is how we used to handle the USE action. It has now been replaced with the block below which is more like the way the Java game does it,
		// however we may find that the way we had it previously is more fun to play.
		/*
		if ((InputManager.GetValue(iPad, MINECRAFT_ACTION_USE,true)>0) && gameMode->isInputAllowed(MINECRAFT_ACTION_USE) )
		{
		handleMouseClick(1);
		lastClickTick = ticks;
		}
		*/
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
				// 4J - attempt to handle click in special creative mode fashion if possible (used for placing blocks at regular intervals)
				bool didClick = player->creativeModeHandleMouseClick(1, useHeld );
				// If this handler has put us in lastClick_oldRepeat mode then it is because we aren't placing blocks - behave largely as the code used to
				if( player->lastClickState == LocalPlayer::lastClick_oldRepeat )
				{
					// If we've already handled the click in creativeModeHandleMouseClick then just record the time of this click
					if( didClick )
					{
						player->lastClickTick[1] = ticks;
					}
					else
					{
						// Otherwise just the original game code for handling autorepeat
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
				// Consider as a click if we've had a period of not pressing the button, or we've reached auto-repeat time since the last time
				// Auto-repeat is only considered if we aren't riding or sprinting, to avoid photo sensitivity issues when placing fire whilst doing fast things
				// Also disable repeat when the player is sleeping to stop the waking up right after using the bed
				bool firstClick = ( player->lastClickTick[1] == 0 );
				bool autoRepeat = ticks - player->lastClickTick[1] >= timer->ticksPerSecond / 4;
				if ( player->isRiding() || player->isSprinting() || player->isSleeping() ) autoRepeat = false;
				if (useHeld )
				{
					// If the player has just exited a bed, then delay the time before a repeat key is allowed without releasing
					if(player->isSleeping() ) player->lastClickTick[1] = ticks + (timer->ticksPerSecond * 2);
					if( firstClick || autoRepeat )
					{
						bool wasSleeping = player->isSleeping();

						player->handleMouseClick(1);

						// If the player has just exited a bed, then delay the time before a repeat key is allowed without releasing
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

#ifdef _DEBUG//_MENUS_ENABLED // disable DPad cheats on release builds
		if(app.DebugSettingsOn())
		{
#ifndef __PSVITA__
			// 4J-PB - debugoverlay for primary player only
			if(iPad==ProfileManager.GetPrimaryPad())
			{
				if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_RENDER_DEBUG)) )
				{
#ifndef _CONTENT_PACKAGE
#ifdef _XBOX
					app.EnableDebugOverlay(options->renderDebug,iPad);
#else
					// 4J Stu - The xbox uses a completely different way of navigating to this scene
					ui.NavigateToScene(0, eUIScene_DebugOverlay, NULL, eUILayer_Debug);
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
#endif // PSVITA
		}
#endif

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_RENDER_THIRD_PERSON)) && gameMode->isInputAllowed(MINECRAFT_ACTION_RENDER_THIRD_PERSON))
		{
			// 4J-PB - changing this to be per player
			player->SetThirdPersonView((player->ThirdPersonView()+1)%3);
			//options->thirdPersonView = !options->thirdPersonView;
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

			// 4J-PB - reordered the if statement so creative mode doesn't bring up the crafting table
			// Fix for #39014 - TU5:  Creative Mode:  Pressing X to access the creative menu while looking at a crafting table causes the crafting menu to display
			if(gameMode->hasInfiniteItems())
			{
				// Creative mode

				ui.PlayUISFX(eSFX_Press);
				app.LoadCreativeMenu(iPad,player);
			}
			// 4J-PB - Microsoft request that we use the 3x3 crafting if someone presses X while at the workbench
			else if ((hitResult!=NULL) && (hitResult->type == HitResult::TILE) && (level->getTile(hitResult->x, hitResult->y, hitResult->z) == Tile::workBench_Id))
			{
				//ui.PlayUISFX(eSFX_Press);
				//app.LoadXuiCrafting3x3Menu(iPad,player,hitResult->x, hitResult->y, hitResult->z);
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
			ui.NavigateToScene(iPad, eUIScene_PauseMenu, NULL, eUILayer_Scene);
		}

		if((player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) && gameMode->isInputAllowed(MINECRAFT_ACTION_DROP))
		{
			player->drop();
		}

		uint64_t ullButtonsPressed=player->ullButtonsPressed;

		bool selected = false;
#ifdef __PSVITA__
		// 4J-PB - use the touchscreen for quickselect
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
			// Dropping items happens over network, so if we only have one then assume that we dropped it and should hide the item
			int iCount=0;

			if(selectedItem != NULL) iCount=selectedItem->GetCount();
			if(selectedItem != NULL && !( (player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) && selectedItem->GetCount() == 1))
			{
				itemName = selectedItem->getHoverName();
			}
			if( !(player->ullButtonsPressed&(1LL<<MINECRAFT_ACTION_DROP)) || (selectedItem != NULL && selectedItem->GetCount() <= 1) ) ui.SetSelectedItem( iPad, itemName );
		}
	}
	else
	{
		// 4J-PB
		if (InputManager.GetValue(iPad, ACTION_MENU_CANCEL) > 0 && gameMode->isInputAllowed(ACTION_MENU_CANCEL))
		{
			setScreen(NULL);
		}
	}

	// monitor for keyboard input
	// #ifndef _CONTENT_PACKAGE
	// 	if(!(ui.GetMenuDisplayed(iPad)))
	// 	{
	// 		WCHAR wchInput;
	// 		if(InputManager.InputDetected(iPad,&wchInput))
	// 		{
	// 			printf("Input Detected!\n");
	//
	// 			// see if we can react to this
	// 			if(app.GetXuiAction(iPad)==eAppAction_Idle)
	// 			{
	// 				app.SetAction(iPad,eAppAction_DebugText,(LPVOID)wchInput);
	// 			}
	// 		}
	// 	}
	// #endif

#if 0
	// 4J - TODO - some replacement for input handling...
	if (screen == NULL || screen.passEvents)
	{
		while (Mouse.next())
		{
			long passedTime = System.currentTimeMillis() - lastTickTime;
			if (passedTime > 200) continue;

			int wheel = Mouse.getEventDWheel();
			if (wheel != 0) {
				player->inventory.swapPaint(wheel);

				if (options.isFlying) {
					if (wheel > 0) wheel = 1;
					if (wheel < 0) wheel = -1;

					options.flySpeed += wheel * .25f;
				}
			}

			if (screen == null) {
				if (!mouseGrabbed && Mouse.getEventButtonState()) {
					grabMouse();
				} else {
					if (Mouse.getEventButton() == 0 && Mouse.getEventButtonState()) {
						handleMouseClick(0);
						lastClickTick = ticks;
					}
					if (Mouse.getEventButton() == 1 && Mouse.getEventButtonState()) {
						handleMouseClick(1);
						lastClickTick = ticks;
					}
					if (Mouse.getEventButton() == 2 && Mouse.getEventButtonState()) {
						handleGrabTexture();
					}
				}
			} else if (screen != null) {
				screen.mouseEvent();
			}
		}

		if (missTime > 0) missTime--;

		while (Keyboard.next()) {
			player->setKey(Keyboard.getEventKey(), Keyboard.getEventKeyState());
			if (Keyboard.getEventKeyState()) {
				if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
					toggleFullScreen();
					continue;
				}
				/*
				* if (Keyboard.getEventKey() == Keyboard.KEY_F4) { new
				* PortalForcer().createPortal(level, player); continue; }
				*/

				/*
				* if (Keyboard.getEventKey() == Keyboard.KEY_RETURN) {
				* level.pathFind(); continue; }
				*/

				if (screen != null) {
					screen.keyboardEvent();
				} else {
					if (Keyboard.getEventKey() == Keyboard.KEY_ESCAPE) {
						pauseGame();
					}

					if (Keyboard.getEventKey() == Keyboard.KEY_S && Keyboard.isKeyDown(Keyboard.KEY_F3)) {
						reloadSound();
					}

					//                        if (Keyboard.getEventKey() == Keyboard.KEY_P) {
					//                            gameMode = new DemoMode(this);
					//                            selectLevel(CreateWorldScreen.findAvailableFolderName(getLevelSource(), "Demo"), "Demo World", 0L);
					//                            setScreen(null);
					//
					//                        }

					if (Keyboard.getEventKey() == Keyboard.KEY_F1) {
						options.hideGui = !options.hideGui;
					}
					if (Keyboard.getEventKey() == Keyboard.KEY_F3) {
						options.renderDebug = !options.renderDebug;
					}
					if (Keyboard.getEventKey() == Keyboard.KEY_F5) {
						options.thirdPersonView = !options.thirdPersonView;
					}
					if (Keyboard.getEventKey() == Keyboard.KEY_F8) {
						options.smoothCamera = !options.smoothCamera;
					}
					if (DEADMAU5_CAMERA_CHEATS) {
						if (Keyboard.getEventKey() == Keyboard.KEY_F6) {
							options.isFlying = !options.isFlying;
						}
						if (Keyboard.getEventKey() == Keyboard.KEY_F9) {
							options.fixedCamera = !options.fixedCamera;
						}
						if (Keyboard.getEventKey() == Keyboard.KEY_ADD) {
							options.cameraSpeed += .1f;
						}
						if (Keyboard.getEventKey() == Keyboard.KEY_SUBTRACT) {
							options.cameraSpeed -= .1f;
							if (options.cameraSpeed < 0) {
								options.cameraSpeed = 0;
							}
						}
					}

					if (Keyboard.getEventKey() == options.keyBuild.key) {
						setScreen(new InventoryScreen(player));
					}

					if (Keyboard.getEventKey() == options.keyDrop.key) {
						player->drop();
					}
					if (isClientSide() && Keyboard.getEventKey() == options.keyChat.key) {
						setScreen(new ChatScreen());
					}
				}

				for (int i = 0; i < 9; i++) {
					if (Keyboard.getEventKey() == Keyboard.KEY_1 + i) player->inventory.selected = i;
				}
				if (Keyboard.getEventKey() == options.keyFog.key) {
					options.toggle(Options.Option.RENDER_DISTANCE, Keyboard.isKeyDown(Keyboard.KEY_LSHIFT) || Keyboard.isKeyDown(Keyboard.KEY_RSHIFT) ? -1 : 1);
				}
			}
		}

		if (screen == null) {
			if (Mouse.isButtonDown(0) && ticks - lastClickTick >= timer.ticksPerSecond / 4 && mouseGrabbed) {
				handleMouseClick(0);
				lastClickTick = ticks;
			}
			if (Mouse.isButtonDown(1) && ticks - lastClickTick >= timer.ticksPerSecond / 4 && mouseGrabbed) {
				handleMouseClick(1);
				lastClickTick = ticks;
			}
		}

		handleMouseDown(0, screen == null && Mouse.isButtonDown(0) && mouseGrabbed);
	}
#endif

	if (level != NULL)
	{
		if (player != NULL)
		{
			recheckPlayerIn++;
			if (recheckPlayerIn == 30)
			{
				recheckPlayerIn = 0;
				level->ensureAdded(player);
			}
		}
		// 4J Changed - We are setting the difficulty the same as the server so that leaderboard updates work correctly
		//level->difficulty = options->difficulty;
		//if (level->isClientSide) level->difficulty = Difficulty::HARD;
		if( !level->isClientSide )
		{
			//app.DebugPrintf("Minecraft::tick - Difficulty = %d",options->difficulty);
			level->difficulty = options->difficulty;
		}

		PIXBeginNamedEvent(0,"Game renderer tick");
		if (!pause) gameRenderer->tick( bFirst);
		PIXEndNamedEvent();

		// 4J - we want to tick each level once only per frame, and do it when a player that is actually in that level happens to be active.
		// This is important as things that get called in the level tick (eg the levellistener) eventually end up working out what the current
		// level is by determing it from the current player. Use flags here to make sure each level is only ticked the once.
		static unsigned int levelsTickedFlags;
		if( bFirst )
		{
			levelsTickedFlags = 0;

#ifndef DISABLE_LEVELTICK_THREAD
			PIXBeginNamedEvent(0,"levelTickEventQueue waitForFinish");
			levelTickEventQueue->waitForFinish();
			PIXEndNamedEvent();
#endif // DISABLE_LEVELTICK_THREAD
			SparseLightStorage::tick();	// 4J added
			CompressedTileStorage::tick();	// 4J added
			SparseDataStorage::tick();		// 4J added
		}

		for(unsigned int i = 0; i < levels.length; ++i)
		{
			if( player->level != levels[i] ) continue;	// Don't tick if the current player isn't in this level

			// 4J - this doesn't fully tick the animateTick here, but does register this player's position. The actual
			// work is now done in Level::animateTickDoWork() so we can take into account multiple players in the one level.
			if (!pause && levels[i] != NULL) levels[i]->animateTick(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));

			if( levelsTickedFlags & ( 1 << i ) ) continue; // Don't tick further if we've already ticked this level this frame
			levelsTickedFlags |= (1 << i);

			PIXBeginNamedEvent(0,"Level renderer tick");
			if (!pause) levelRenderer->tick();
			PIXEndNamedEvent();
			// if (!pause && player!=null) {
			// if (player != null && !level.entities.contains(player)) {
			// level.addEntity(player);
			// }
			// }
			if( levels[i] != NULL )
			{
				if (!pause)
				{
					if (levels[i]->skyFlashTime > 0) levels[i]->skyFlashTime--;
					PIXBeginNamedEvent(0,"Level entity tick");
					levels[i]->tickEntities();
					PIXEndNamedEvent();
				}

				// optimisation to set the culling off early, in parallel with other stuff
#if defined __PS3__ && !defined DISABLE_SPU_CODE
				// kick off the culling for all valid players in this level
				int currPlayerIdx = getLocalPlayerIdx();
				for( int idx = 0; idx < XUSER_MAX_COUNT; idx++ )
				{
					if(localplayers[idx]!=NULL)
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
#endif // __PS3__

				// 4J Stu - We are always online, but still could be paused
				if (!pause) // || isClientSide())
				{
					//app.DebugPrintf("Minecraft::tick spawn settings - Difficulty = %d",options->difficulty);
					levels[i]->setSpawnSettings(level->difficulty > 0, true);
					PIXBeginNamedEvent(0,"Level tick");
#ifdef DISABLE_LEVELTICK_THREAD
					levels[i]->tick();
#else
					levelTickEventQueue->sendEvent(levels[i]);
#endif // DISABLE_LEVELTICK_THREAD
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

		// 4J Stu - Keep ticking the connections if paused so that they don't time out
		if( pause ) tickAllConnections();
		// player->tick();
	}
#ifdef __PS3__

// 	while(!g_tickLevelQueue.empty())
// 	{
// 		Level* pLevel = g_tickLevelQueue.front();
// 		g_tickLevelQueue.pop();
// 		pLevel->tick();
// 	};

#endif

	// if (Keyboard.isKeyDown(Keyboard.KEY_NUMPAD7) ||
	// Keyboard.isKeyDown(Keyboard.KEY_Q)) rota++;
	// if (Keyboard.isKeyDown(Keyboard.KEY_NUMPAD9) ||
	// Keyboard.isKeyDown(Keyboard.KEY_E)) rota--;
	// 4J removed
	//lastTickTime = System::currentTimeMillis();
}

void Minecraft::reloadSound()
{
	//    System.out.println("FORCING RELOAD!");		// 4J - removed
	soundEngine = new SoundEngine();
	soundEngine->init(options);
	bgLoader->forceReload();
}

bool Minecraft::isClientSide()
{
	return level != NULL && level->isClientSide;
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
	//this->level = NULL;
	setLevel(NULL, message);
}

// 4J Stu - This code was within setLevel, but I moved it out so that I can call it at a better
// time when exiting from an online game
void Minecraft::forceStatsSave(int idx)
{
	//4J Gordon: Force a stats save
	stats[idx]->save(idx, true);

	//4J Gordon: If the player is signed in, save the leaderboards
	if( ProfileManager.IsSignedInLive(idx) )
	{
		int tempLockedProfile = ProfileManager.GetLockedProfile();
		ProfileManager.SetLockedProfile(idx);
		stats[idx]->saveLeaderboards();
		ProfileManager.SetLockedProfile(tempLockedProfile);
	}
}

// 4J Added
MultiPlayerLevel *Minecraft::getLevel(int dimension)
{
	if (dimension == -1) return levels[1];
	else if(dimension == 1) return levels[2];
	else return levels[0];
}

// 4J Stu - Removed as redundant with default values in params.
//void Minecraft::setLevel(Level *level, bool doForceStatsSave /*= true*/)
//{
//	setLevel(level, -1, NULL, doForceStatsSave);
//}

// Also causing ambiguous call for some reason
// as it is matching shared_ptr<Player> from the func below with bool from this one
//void Minecraft::setLevel(Level *level, const wstring& message, bool doForceStatsSave /*= true*/)
//{
//	setLevel(level, message, NULL, doForceStatsSave);
//}

void Minecraft::forceaddLevel(MultiPlayerLevel *level)
{
	int dimId = level->dimension->id;
	if (dimId == -1) levels[1] = level;
	else if(dimId == 1) levels[2] = level;
	else levels[0] = level;
}

void Minecraft::setLevel(MultiPlayerLevel *level, int message /*=-1*/, shared_ptr<Player> forceInsertPlayer /*=NULL*/, bool doForceStatsSave /*=true*/, bool bPrimaryPlayerSignedOut /*=false*/)
{
	EnterCriticalSection(&m_setLevelCS);
	bool playerAdded = false;
	this->cameraTargetPlayer = nullptr;

	if(progressRenderer != NULL)
	{
		this->progressRenderer->progressStart(message);
		this->progressRenderer->progressStage(-1);
	}

	// 4J-PB - since we now play music in the menu, just let it keep playing
	//soundEngine->playStreaming(L"", 0, 0, 0, 0, 0);

	// 4J - stop update thread from processing this level, which blocks until it is safe to move on - will be re-enabled if we set the level to be non-NULL
	gameRenderer->DisableUpdateThread();

	for(unsigned int i = 0; i < levels.length; ++i)
	{
		// 4J We only need to save out in multiplayer is we are setting the level to NULL
		// If we ever go back to making single player only then this will not work properly!
		if (levels[i] != NULL && level == NULL)
		{
			// 4J Stu - This is really only relevant for single player (ie not what we do at the moment)
			if((doForceStatsSave==true) && player!=NULL)
				forceStatsSave(player->GetXboxPad() );

			// 4J Stu - Added these for the case when we exit a level so we are setting the level to NULL
			// The level renderer needs to have it's stored level set to NULL so that it doesn't break next time we set one
			if (levelRenderer != NULL)
			{
				for(DWORD p = 0; p < XUSER_MAX_COUNT; ++p)
				{
					levelRenderer->setLevel(p, NULL);
				}
			}
			if (particleEngine != NULL) particleEngine->setLevel(NULL);
		}
	}
	// 4J If we are setting the level to NULL then we are exiting, so delete the levels
	if( level == NULL )
	{
		if(levels[0]!=NULL)
		{
			delete levels[0];
			levels[0] = NULL;

			// Both level share the same savedDataStorage
			if(levels[1]!=NULL) levels[1]->savedDataStorage = NULL;
		}
		if(levels[1]!=NULL)
		{
			delete levels[1];
			levels[1] = NULL;
		}
		if(levels[2]!=NULL)
		{
			delete levels[2];
			levels[2] = NULL;
		}

		// Delete all the player objects
		for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			shared_ptr<MultiplayerLocalPlayer> mplp = localplayers[idx];
			if(mplp != NULL && mplp->connection != NULL )
			{
				delete mplp->connection;
				mplp->connection = NULL;
			}

			if( localgameModes[idx] != NULL )
			{
				delete localgameModes[idx];
				localgameModes[idx] = NULL;
			}

			if( m_pendingLocalConnections[idx] != NULL )
			{
				delete m_pendingLocalConnections[idx];
				m_pendingLocalConnections[idx] = NULL;
			}

			localplayers[idx] = nullptr;
		}
		// If we are removing the primary player then there can't be a valid gamemode left anymore, this
		// pointer will be referring to the one we've just deleted
		gameMode = NULL;
		// Remove references to player
		player = nullptr;
		cameraTargetPlayer = nullptr;
		EntityRenderDispatcher::instance->cameraEntity = nullptr;
		TileEntityRenderDispatcher::instance->cameraEntity = nullptr;
	}
	this->level = level;

	if (level != NULL)
	{
		int dimId = level->dimension->id;
		if (dimId == -1) levels[1] = level;
		else if(dimId == 1) levels[2] = level;
		else levels[0] = level;

		// If no player has been set, then this is the first level to be set this game, so set up
		// a primary player & initialise some other things
		if (player == NULL)
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
				// player doesn't have an online UID, set it from the player name
				playerXUIDOnline.setForAdhoc();
			}
#endif
#ifdef _WINDOWS64
			// On Windows, the implementation has been changed to use a per-client pseudo XUID based on `uid.dat`.
			// To maintain player data compatibility with existing worlds, the world host (the first player) will use the previous embedded pseudo XUID.
			INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(iPrimaryPlayer);
			if(localNetworkPlayer != NULL && localNetworkPlayer->IsHost())
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
				m_pendingLocalConnections[i] = NULL;
				if( i != iPrimaryPlayer ) localgameModes[i] = NULL;
			}
		}

		if (player != NULL)
		{
			player->resetPos();
			// gameMode.initPlayer(player);
			if (level != NULL)
			{
				level->addEntity(player);
				playerAdded = true;
			}
		}

		if(player->input != NULL) delete player->input;
		player->input = new Input();

		if (levelRenderer != NULL) levelRenderer->setLevel(player->GetXboxPad(), level);
		if (particleEngine != NULL) particleEngine->setLevel(level);

#if 0
		// 4J - removed - we don't use ChunkCache anymore
		ChunkSource *cs = level->getChunkSource();
		if (dynamic_cast<ChunkCache *>(cs) != NULL)
		{
			ChunkCache *spcc = (ChunkCache *)cs;

			// 4J - these had a Mth::floor which seems unrequired
			int xt = ((int) player->x) >> 4;
			int zt = ((int) player->z) >> 4;

			spcc->centerOn(xt, zt);
		}
#endif
		gameMode->adjustPlayer(player);

		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			m_pendingLocalConnections[i] = NULL;
		}
		updatePlayerViewportAssignments();

		this->cameraTargetPlayer = player;

		// 4J - allow update thread to start processing the level now both it & the player should be ok
		gameRenderer->EnableUpdateThread();
	}
	else
	{
		levelSource->clearAll();
		player = nullptr;

		// Clear all players if the new level is NULL
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if( m_pendingLocalConnections[i] != NULL ) m_pendingLocalConnections[i]->close();
			m_pendingLocalConnections[i] = NULL;
			localplayers[i] = nullptr;
			localgameModes[i] = NULL;
		}
	}

	//    System.gc();	// 4J - removed
	// 4J removed
	//this->lastTickTime = 0;
	LeaveCriticalSection(&m_setLevelCS);
}

void Minecraft::prepareLevel(int title)
{
	if(progressRenderer != NULL)
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
	if (player != NULL)
	{
		spawnPos->x = (int) player->x;
		spawnPos->z = (int) player->z;
	}

#if 0
	// 4J - removed - we don't use ChunkCache anymore
	if (dynamic_cast<ChunkCache *>(cs)!=NULL)
	{
		ChunkCache *spcc = (ChunkCache *) cs;

		spcc->centerOn(spawnPos->x >> 4, spawnPos->z >> 4);
	}
#endif

	for (int x = -r; x <= r; x += 16)
	{
		for (int z = -r; z <= r; z += 16)
		{
			if(progressRenderer != NULL) this->progressRenderer->progressStagePercentage((pp++) * 100 / max);
			level->getTile(spawnPos->x + x, 64, spawnPos->z + z);
			if (!gameMode->isCutScene()) {
			}
		}
	}
	delete spawnPos;
	if (!gameMode->isCutScene())
	{
		if(progressRenderer != NULL) this->progressRenderer->progressStage(IDS_PROGRESS_SIMULATING_WORLD);
		max = 2000;
}
}

wstring Minecraft::gatherStats1()
{
	//return levelRenderer->gatherStats1();
	return L"Time to autosave: " + std::to_wstring( app.SecondsToAutosave() ) + L"s";
}

wstring Minecraft::gatherStats2()
{
	return g_NetworkManager.GatherStats();
	//return levelRenderer->gatherStats2();
}

wstring Minecraft::gatherStats3()
{
	return g_NetworkManager.GatherRTTStats();
	//return L"P: " + particleEngine->countParticles() + L". T: " + level->gatherStats();
}

wstring Minecraft::gatherStats4()
{
	return level->gatherChunkSourceStats();
}

void Minecraft::respawnPlayer(int iPad, int dimension, int newEntityId)
{
	gameRenderer->DisableUpdateThread(); // 4J - don't do updating whilst we are adjusting the player & localplayer array
	shared_ptr<MultiplayerLocalPlayer> localPlayer = localplayers[iPad];

	level->validateSpawn();
	level->removeAllPendingEntityRemovals();

	if (localPlayer != NULL)
	{
		level->removeEntity(localPlayer);
	}

	shared_ptr<Player> oldPlayer = localPlayer;
	cameraTargetPlayer = nullptr;

	// 4J-PB - copy and set the players xbox pad
	int iTempPad=localPlayer->GetXboxPad();
	int iTempScreenSection = localPlayer->m_iScreenSection;
	EDefaultSkins skin = localPlayer->getPlayerDefaultSkin();
	player = localgameModes[iPad]->createPlayer(level);

	PlayerUID playerXUIDOffline = INVALID_XUID;
	PlayerUID playerXUIDOnline = INVALID_XUID;
	ProfileManager.GetXUID(iTempPad,&playerXUIDOffline,false);
	ProfileManager.GetXUID(iTempPad,&playerXUIDOnline,true);
#ifdef _WINDOWS64
	// Same compatibility rule as create/init paths.
	INetworkPlayer *localNetworkPlayer = g_NetworkManager.GetLocalPlayerByUserIndex(iTempPad);
	if(localNetworkPlayer != NULL && localNetworkPlayer->IsHost())
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

	// Fix for #63021 - TU7: Content: UI: Travelling from/to the Nether results in switching currently held item to another.
	// Fix for #81759 - TU9: Content: Gameplay: Entering The End Exit Portal replaces the Player's currently held item with the first one from the Quickbar
	if( localPlayer->getHealth() > 0 && localPlayer->y > -64)
	{
		player->inventory->selected = localPlayer->inventory->selected;
	}

	// Set the animation override if the skin has one
	DWORD dwSkinID=app.getSkinIdFromPath(player->customTextureUrl);
	if(GET_IS_DLC_SKIN_FROM_BITMASK(dwSkinID))
	{
		player->setAnimOverrideBitmask(player->getSkinAnimOverrideBitmask(dwSkinID));
	}

	player->dimension = dimension;
	cameraTargetPlayer = player;

	// 4J-PB - are we the primary player or a local player?
	if(iPad==ProfileManager.GetPrimaryPad())
	{
		createPrimaryLocalPlayer(iPad);

		// update the debugoptions
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

	if(player->input != NULL) delete player->input;
	player->input = new Input();
	player->entityId = newEntityId;
	player->animateRespawn();
	gameMode->adjustPlayer(player);

	// 4J - added isClientSide check here
	if (!level->isClientSide)
	{
		prepareLevel(IDS_PROGRESS_RESPAWNING);
	}

	// 4J Added for multiplayer. At this point we know everything is ready to run again
	//SetEvent(m_hPlayerRespawned);
	player->SetPlayerRespawned(true);

	if (dynamic_cast<DeathScreen *>(screen) != NULL) setScreen(NULL);

	gameRenderer->EnableUpdateThread();
}

void Minecraft::start(const wstring& name, const wstring& sid)
{
	startAndConnectTo(name, sid, L"");
}

void Minecraft::startAndConnectTo(const wstring& name, const wstring& sid, const wstring& url)
{
	bool fullScreen = false;
	wstring userName = name;

	/* 4J - removed window handling things here
	final Frame frame = new Frame("Minecraft");
	Canvas canvas = new Canvas();
	frame.setLayout(new BorderLayout());

	frame.add(canvas, BorderLayout.CENTER);

	// OverlayLayout oll = new OverlayLayout(frame);
	// oll.addLayoutComponent(canvas, BorderLayout.CENTER);
	// oll.addLayoutComponent(new JLabel("TEST"), BorderLayout.EAST);

	canvas.setPreferredSize(new Dimension(854, 480));
	frame.pack();
	frame.setLocationRelativeTo(null);
	*/

	Minecraft *minecraft;
	// 4J - was new Minecraft(frame, canvas, NULL, 854, 480, fullScreen);

	minecraft = new Minecraft(NULL, NULL, NULL, 1280, 720, fullScreen);

	/* - 4J - removed
	{
	@Override
	public void onCrash(CrashReport crashReport) {
	frame.removeAll();
	frame.add(new CrashInfoPanel(crashReport), BorderLayout.CENTER);
	frame.validate();
	}
	}; */

	/* 4J - removed
	final Thread thread = new Thread(minecraft, "Minecraft main thread");
	thread.setPriority(Thread.MAX_PRIORITY);
	*/
	minecraft->serverDomain = L"www.minecraft.net";

	// 4J Stu - We never want the player to be DemoUser, we always want them to have their gamertag displayed
	//if (ProfileManager.IsFullVersion())
	{
		if (userName != L"" && sid != L"")	// 4J - username & side were compared with NULL rather than empty strings
		{
			minecraft->user = new User(userName, sid);
		}
		else
		{
			minecraft->user = new User(L"Player" + std::to_wstring(System::currentTimeMillis() % 1000), L"");
		}
	}
	//else
	//{
	//	minecraft->user = new DemoUser();
	//}

	/* 4J - TODO
	if (url != NULL)
	{
	String[] tokens = url.split(":");
	minecraft.connectTo(tokens[0], Integer.parseInt(tokens[1]));
	}
	*/

	/* 4J - removed
	frame.setVisible(true);
	frame.addWindowListener(new WindowAdapter() {
	public void windowClosing(WindowEvent arg0) {
	minecraft.stop();
	try {
	thread.join();
	} catch (InterruptedException e) {
	e.printStackTrace();
	}
	System.exit(0);
	}
	});
	*/
	// 4J - TODO - consider whether we need to actually create a thread here
	minecraft->run();
}

ClientConnection *Minecraft::getConnection(int iPad)
{
	return localplayers[iPad]->connection;
}

// 4J-PB - so we can access this from within our xbox game loop
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

	//g_iMainThreadId = GetCurrentThreadId();

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

	// 4J Stu - This block generates XML for the game rules schema
#if 0
	for(unsigned int i = 0; i < Item::items.length; ++i)
	{
		if(Item::items[i] != NULL)
		{
			app.DebugPrintf("<xs:enumeration value=\"%d\"><xs:annotation><xs:documentation>%ls</xs:documentation></xs:annotation></xs:enumeration>\n", i, app.GetString( Item::items[i]->getDescriptionId() ));
		}
	}

	app.DebugPrintf("\n\n\n\n\n");

	for(unsigned int i = 0; i < 256; ++i)
	{
		if(Tile::tiles[i] != NULL)
		{
			app.DebugPrintf("<xs:enumeration value=\"%d\"><xs:annotation><xs:documentation>%ls</xs:documentation></xs:annotation></xs:enumeration>\n", i, app.GetString( Tile::tiles[i]->getDescriptionId() ));
		}
	}
	__debugbreak();
#endif

	// 4J-PB - Can't call this for the first 5 seconds of a game - MS rule
	//if (ProfileManager.IsFullVersion())
	{
		name = L"Player" + std::to_wstring(System::currentTimeMillis() % 1000);
		sessionId = L"-";
		/* 4J - TODO - get a session ID from somewhere?
		if (args.length > 0) name = args[0];
		sessionId = "-";
		if (args.length > 1) sessionId = args[1];
		*/
	}

	// Common for all platforms
	IUIScene_CreativeMenu::staticCtor();

	// On PS4, we call Minecraft::Start from another thread, as this has been timed taking ~2.5 seconds and we need to do some basic
	// rendering stuff so that we don't break the TRCs on SubmitDone calls
#ifndef __ORBIS__
	Minecraft::start(name, sessionId);
#endif
}

bool Minecraft::renderNames()
{
	if (m_instance == NULL || !m_instance->options->hideGui)
	{
		return true;
	}
	return false;
}

bool Minecraft::useFancyGraphics()
{
	return (m_instance != NULL && m_instance->options->fancyGraphics);
}

bool Minecraft::useAmbientOcclusion()
{
	return (m_instance != NULL && m_instance->options->ambientOcclusion != Options::AO_OFF);
}

bool Minecraft::renderDebug()
{
	return (m_instance != NULL && m_instance->options->renderDebug);
}

bool Minecraft::handleClientSideCommand(const wstring& chatMessage)
{
	return false;
}

int Minecraft::maxSupportedTextureSize()
{
	// 4J Force value
	return 1024;

	//for (int texSize = 16384; texSize > 0; texSize >>= 1) {
	//	GL11.glTexImage2D(GL11.GL_PROXY_TEXTURE_2D, 0, GL11.GL_RGBA, texSize, texSize, 0, GL11.GL_RGBA, GL11.GL_UNSIGNED_BYTE, (ByteBuffer) null);
	//	final int width = GL11.glGetTexLevelParameteri(GL11.GL_PROXY_TEXTURE_2D, 0, GL11.GL_TEXTURE_WIDTH);
	//	if (width != 0) {
	//		return texSize;
	//	}
	//}
	//return -1;
}

void Minecraft::delayTextureReload()
{
	reloadTextures = true;
}

int64_t Minecraft::currentTimeMillis()
{
	return System::currentTimeMillis();//(Sys.getTime() * 1000) / Sys.getTimerResolution();
}

/*void Minecraft::handleMouseDown(int button, bool down)
{
if (gameMode->instaBuild) return;
if (!down) missTime = 0;
if (button == 0 && missTime > 0) return;

if (down && hitResult != NULL && hitResult->type == HitResult::TILE && button == 0)
{
int x = hitResult->x;
int y = hitResult->y;
int z = hitResult->z;
gameMode->continueDestroyBlock(x, y, z, hitResult->f);
particleEngine->crack(x, y, z, hitResult->f);
}
else
{
gameMode->stopDestroyBlock();
}
}

void Minecraft::handleMouseClick(int button)
{
if (button == 0 && missTime > 0) return;
if (button == 0)
{
app.DebugPrintf("handleMouseClick - Player %d is swinging\n",player->GetXboxPad());
player->swing();
}

bool mayUse = true;

//	* if (button == 1) { ItemInstance item =
//	* player.inventory.getSelected(); if (item != null) { if
//	* (gameMode.useItem(player, item)) {
//	* gameRenderer.itemInHandRenderer.itemUsed(); return; } } }

// 4J-PB - Adding a special case in here for sleeping in a bed in a multiplayer game - we need to wake up, and we don't have the inbedchatscreen with a button

if(button==1 && (player->isSleeping() && level != NULL && level->isClientSide))
{
shared_ptr<MultiplayerLocalPlayer> mplp = dynamic_pointer_cast<MultiplayerLocalPlayer>( player );

if(mplp) mplp->StopSleeping();

// 4J - TODO
//if (minecraft.player instanceof MultiplayerLocalPlayer)
//{
//    ClientConnection connection = ((MultiplayerLocalPlayer) minecraft.player).connection;
//    connection.send(new PlayerCommandPacket(minecraft.player, PlayerCommandPacket.STOP_SLEEPING));
//}
}

if (hitResult == NULL)
{
if (button == 0 && !(dynamic_cast<CreativeMode *>(gameMode) != NULL)) missTime = 10;
}
else if (hitResult->type == HitResult::ENTITY)
{
if (button == 0)
{
gameMode->attack(player, hitResult->entity);
}
if (button == 1)
{
gameMode->interact(player, hitResult->entity);
}
}
else if (hitResult->type == HitResult::TILE)
{
int x = hitResult->x;
int y = hitResult->y;
int z = hitResult->z;
int face = hitResult->f;

//	* if (button != 0) { if (hitResult.f == 0) y--; if (hitResult.f ==
//	* 1) y++; if (hitResult.f == 2) z--; if (hitResult.f == 3) z++; if
//	* (hitResult.f == 4) x--; if (hitResult.f == 5) x++; }

// if (isClientSide())
// {
// return;
// }

if (button == 0)
{
gameMode->startDestroyBlock(x, y, z, hitResult->f);
}
else
{
shared_ptr<ItemInstance> item = player->inventory->getSelected();
int oldCount = item != NULL ? item->count : 0;
if (gameMode->useItemOn(player, level, item, x, y, z, face))
{
mayUse = false;
app.DebugPrintf("Player %d is swinging\n",player->GetXboxPad());
player->swing();
}
if (item == NULL)
{
return;
}

if (item->count == 0)
{
player->inventory->items[player->inventory->selected] = NULL;
}
else if (item->count != oldCount)
{
gameRenderer->itemInHandRenderer->itemPlaced();
}
}
}

if (mayUse && button == 1)
{
shared_ptr<ItemInstance> item = player->inventory->getSelected();
if (item != NULL)
{
if (gameMode->useItem(player, level, item))
{
gameRenderer->itemInHandRenderer->itemUsed();
}
}
}
}
*/

// 4J-PB
Screen * Minecraft::getScreen()
{
	return screen;
}

bool Minecraft::isTutorial()
{
	return m_inFullTutorialBits > 0;

	/*if( gameMode != NULL && gameMode->isTutorial() )
	{
	return true;
	}
	else
	{
	return false;
	}*/
}

void Minecraft::playerStartedTutorial(int iPad)
{
	// If the app doesn't think we are in a tutorial mode then just ignore this add
	if( app.GetTutorialMode() ) m_inFullTutorialBits = m_inFullTutorialBits | ( 1 << iPad );
}

void Minecraft::playerLeftTutorial(int iPad)
{
	// 4J Stu - Fix for bug that was flooding Sentient with LevelStart events
	// If the tutorial bits are already 0 then don't need to update anything
	if(m_inFullTutorialBits == 0)
	{
		app.SetTutorialMode( false );
		return;
	}

	m_inFullTutorialBits = m_inFullTutorialBits & ~( 1 << iPad );
	if(m_inFullTutorialBits == 0)
	{
		app.SetTutorialMode( false );

		// 4J Stu -This telemetry event means something different on XboxOne, so we don't call it for simple state changes like this
#ifndef _XBOX_ONE
		for(DWORD idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if(localplayers[idx] != NULL)
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
			// create the local player for the iPad
			shared_ptr<Player> player = pClass->localplayers[iPad];
			if( player == NULL)
			{
				if( pClass->level->isClientSide )
				{
					pClass->addLocalPlayer(iPad);
				}
				else
				{
					// create the local player for the iPad
					shared_ptr<Player> player = pClass->localplayers[iPad];
					if( player == NULL)
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
	Minecraft* pMinecraftClass = (Minecraft*)pParam;

	if(g_NetworkManager.IsInSession())
	{
		// 4J Stu - There seems to be a bug in the signin ui call that enables guest sign in. We never allow this within game, so make sure that it's disabled
		// Fix for #66516 - TCR #124: MPS Guest Support ; #001: BAS Game Stability: TU8: The game crashes when second Guest signs-in on console which takes part in Xbox LIVE multiplayer session.
		app.DebugPrintf("Disabling Guest Signin\n");
		XEnableGuestSignin(FALSE);
	}

	// If sign in succeded, we're in game and this player isn't already playing, continue
	if(bContinue==true && g_NetworkManager.IsInSession() && pMinecraftClass->localplayers[iPad] == NULL)
	{
		// It's possible that the player has not signed in - they can back out or choose no for the converttoguest
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
			// if this is a local game then profiles just need to be signed in
			else if( g_NetworkManager.IsLocalGame() || (ProfileManager.IsSignedInLive(iPad) && ProfileManager.AllowedToPlayMultiplayer(iPad)) )
			{
#ifdef __ORBIS__
				bool contentRestricted = false;
				ProfileManager.GetChatAndContentRestrictions(iPad,false,NULL,&contentRestricted,NULL); // TODO!

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
					// create the local player for the iPad
					shared_ptr<Player> player = pMinecraftClass->localplayers[iPad];
					if( player == NULL)
					{
						player = pMinecraftClass->createExtraLocalPlayer(iPad, (convStringToWstring( ProfileManager.GetGamertag(iPad) )).c_str(), iPad, pMinecraftClass->level->dimension->id);
					}
				}
			}
			else if( ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()) && !ProfileManager.AllowedToPlayMultiplayer(iPad) )
			{
				// 4J Stu - Don't allow converting to guests as we don't allow any guest sign-in while in the game
				// Fix for #66516 - TCR #124: MPS Guest Support ; #001: BAS Game Stability: TU8: The game crashes when second Guest signs-in on console which takes part in Xbox LIVE multiplayer session.
				//ProfileManager.RequestConvertOfflineToGuestUI( &Minecraft::InGame_SignInReturned, pMinecraftClass,iPad);
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

	if(colours == NULL)
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

