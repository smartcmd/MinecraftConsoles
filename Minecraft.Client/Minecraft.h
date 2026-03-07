#pragma once
class Timer;
class MultiPlayerLevel;
class LevelRenderer;
class MultiplayerLocalPlayer;
class Player;
class Mob;
class ParticleEngine;
class User;
class Canvas;
class Textures;
class Font;
class Screen;
class ProgressRenderer;
class GameRenderer;
class BackgroundDownloader;
class HumanoidModel;
class HitResult;
class Options;
class SoundEngine;
class MinecraftApplet;
class MouseHandler;
class TexturePackRepository;
class File;
class LevelStorageSource;
class StatsCounter;
class Component;
class Entity;
class AchievementPopup;
class WaterTexture;
class LavaTexture;
class Gui;
class ClientConnection;
class ConsoleSaveFile;
class ItemInHandRenderer;
class LevelSettings;
class ColourTable;
class MultiPlayerGameMode;
class PsPlusUpsellWrapper;

#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\DisconnectPacket.h"
#include "..\Minecraft.World\C4JThread.h"
#include "ResourceLocation.h"

using namespace std;

class Minecraft
{
private:
	enum  OS{
		linux, solaris, windows, macos, unknown, xbox
	};

	static ResourceLocation DEFAULT_FONT_LOCATION;
	static ResourceLocation ALT_FONT_LOCATION;

public:
	static const wstring VERSION_STRING;
	Minecraft(Component *mouseComponent, Canvas *parent, MinecraftApplet *minecraftApplet, int width, int height, bool fullscreen);
	void init();

	// 4J - removed
	//    void crash(CrashReport crash);
	//    public abstract void onCrash(CrashReport crash);

private:
	static Minecraft *m_instance;

public:
	MultiPlayerGameMode *gameMode;

private:
	bool fullscreen;
	bool hasCrashed;

	C4JThread::EventQueue* levelTickEventQueue;

	static void levelTickUpdateFunc(void* pParam);
	static void levelTickThreadInitFunc();

public:
	int width, height;
	int width_phys, height_phys; // 4J - added
	//    private OpenGLCapabilities openGLCapabilities;

private:
	Timer *timer;
	bool reloadTextures;
public:
	Level *oldLevel; // 4J Stu added to keep a handle on an old level so we can delete it
	//HANDLE m_hPlayerRespawned; // 4J Added so we can wait in menus until it is done (for async in multiplayer)
public:

	MultiPlayerLevel *level;
	LevelRenderer *levelRenderer;
	shared_ptr<MultiplayerLocalPlayer> player;

	MultiPlayerLevelArray levels;

	shared_ptr<MultiplayerLocalPlayer> localplayers[XUSER_MAX_COUNT];
	MultiPlayerGameMode *localgameModes[XUSER_MAX_COUNT];
	int localPlayerIdx;
	ItemInHandRenderer *localitemInHandRenderers[XUSER_MAX_COUNT];
	// 4J-PB - so we can have debugoptions in the server
	unsigned int	uiDebugOptionsA[XUSER_MAX_COUNT];

	// 4J Stu - Added these so that we can show a Xui scene while connecting
	bool m_connectionFailed[XUSER_MAX_COUNT];
	DisconnectPacket::eDisconnectReason m_connectionFailedReason[XUSER_MAX_COUNT];
	ClientConnection *m_pendingLocalConnections[XUSER_MAX_COUNT];

	bool addLocalPlayer(int idx); // Re-arrange the screen and start the connection
	void addPendingLocalConnection(int idx, ClientConnection *connection);
	void connectionDisconnected(int idx, DisconnectPacket::eDisconnectReason reason) { m_connectionFailed[idx] = true; m_connectionFailedReason[idx] = reason; }

	shared_ptr<MultiplayerLocalPlayer> createExtraLocalPlayer(int idx, const wstring& name, int pad, int iDimension, ClientConnection *clientConnection = NULL,MultiPlayerLevel *levelpassedin=NULL);
	void createPrimaryLocalPlayer(int iPad);
	bool setLocalPlayerIdx(int idx);
	int getLocalPlayerIdx();
	void removeLocalPlayerIdx(int idx);
	void storeExtraLocalPlayer(int idx);
	void updatePlayerViewportAssignments();
	int unoccupiedQuadrant;	// 4J - added

	shared_ptr<LivingEntity> cameraTargetPlayer;
	shared_ptr<LivingEntity> crosshairPickMob;
	ParticleEngine *particleEngine;
	User *user;
	wstring serverDomain;
	Canvas *parent;
	bool appletMode;

	// 4J - per player ?
	volatile bool pause;

	Textures *textures;
	Font *font, *altFont;
	Screen *screen;
	ProgressRenderer *progressRenderer;
	GameRenderer *gameRenderer;
private:
	BackgroundDownloader *bgLoader;

	int ticks;
	// 4J-PB - moved to per player

	//int missTime;

	int orgWidth, orgHeight;
public:
	AchievementPopup *achievementPopup;
public:
	Gui *gui;
	// 4J - move to the per player structure?
	bool noRender;

	HumanoidModel *humanoidModel;
	HitResult *hitResult;
	Options *options;
protected:
	MinecraftApplet *minecraftApplet;
public:
	SoundEngine *soundEngine;
	MouseHandler *mouseHandler;
public:
	TexturePackRepository *skins;
	File workingDirectory;
private:
	LevelStorageSource *levelSource;
public:
	static const int frameTimes_length = 512;
	static int64_t frameTimes[frameTimes_length];
	static const int tickTimes_length = 512;
	static int64_t tickTimes[tickTimes_length];
	static int frameTimePos;
	static int64_t warezTime;
private:
	int rightClickDelay;
public:
	// 4J- this should really be in localplayer
	StatsCounter* stats[4];

private:
	wstring connectToIp;
	int connectToPort;

public:
	void clearConnectionFailed();
	void connectTo(const wstring& server, int port);

private:
	void renderLoadingScreen();

public:
	void blit(int x, int y, int sx, int sy, int w, int h);

private:
	static File workDir;

public:
	LevelStorageSource *getLevelSource();
	void setScreen(Screen *screen);
private:
	void checkGlError(const wstring& string);

#ifdef __ORBIS__
	PsPlusUpsellWrapper *m_pPsPlusUpsell;
#endif

public:
	void destroy();
	volatile bool running;
	wstring fpsString;
	void run();
	// 4J-PB - split the run into 3 parts so we can run it from our xbox game loop
	static Minecraft *GetInstance();
	void run_middle();
	void run_end();
#ifdef _WINDOWS64
	void applyFrameMouseLook();  // Per-frame mouse look to reduce input latency
#endif

	void emergencySave();

	// 4J - removed
	//bool wasDown ;
private:
	//	void checkScreenshot();		// 4J - removed
	//    String grabHugeScreenshot(File workDir2, int width, int height, int ssWidth, int ssHeight);	// 4J - removed

	// 4J - per player thing?
	int64_t lastTimer;

	void renderFpsMeter(int64_t tickTime);
public:
	void stop();
	// 4J removed
	//    bool mouseGrabbed;
	//    void grabMouse();
	//    void releaseMouse();
	// 4J-PB - moved these into localplayer
	//void handleMouseDown(int button, bool down);
	//void handleMouseClick(int button);

	void pauseGame();
	//    void toggleFullScreen();	// 4J - removed
private:
	void resize(int width, int height);

public:
	// 4J - Moved to per player
	//bool isRaining ;

	// 4J - Moved to per player
	//int64_t lastTickTime;

private:
	// 4J- per player?
	int recheckPlayerIn;
	void verify();

public:
	// 4J - added bFirst parameter, which is true for the first active viewport in splitscreen
	// 4J - added bUpdateTextures, which is true if the actual renderer textures are to be updated - this will be true for the last time this tick runs with bFirst true
	void tick(bool bFirst, bool bUpdateTextures);
private:
	void reloadSound();
public:
	bool isClientSide();
	void selectLevel(ConsoleSaveFile *saveFile, const wstring& levelId, const wstring& levelName, LevelSettings *levelSettings);
	//void toggleDimension(int targetDimension);
	bool saveSlot(int slot, const wstring& name);
	bool loadSlot(const wstring& userName, int slot);
	void releaseLevel(int message);
	// 4J Stu - Added the doForceStatsSave param
	//void setLevel(Level *level, bool doForceStatsSave = true);
	//void setLevel(Level *level, const wstring& message, bool doForceStatsSave = true);
	void setLevel(MultiPlayerLevel *level, int message = -1, shared_ptr<Player> forceInsertPlayer = nullptr, bool doForceStatsSave = true,bool bPrimaryPlayerSignedOut=false);
	// 4J-PB - added to force in the 'other' level when the main player creates the level at game load time
	void forceaddLevel(MultiPlayerLevel *level);
	void prepareLevel(int title);	// 4J - changed to public
	//  OpenGLCapabilities getOpenGLCapabilities();	// 4J - removed

	wstring gatherStats1();
	wstring gatherStats2();
	wstring gatherStats3();
	wstring gatherStats4();

	void respawnPlayer(int iPad,int dimension,int newEntityId);
	static void start(const wstring& name, const wstring& sid);
	static void startAndConnectTo(const wstring& name, const wstring& sid, const wstring& url);
	ClientConnection *getConnection(int iPad); // 4J Stu added iPad param
	static void main();
	static bool renderNames();
	static bool useFancyGraphics();
	static bool useAmbientOcclusion();
	static bool renderDebug();
	bool handleClientSideCommand(const wstring& chatMessage);

	static int maxSupportedTextureSize();
	void delayTextureReload();
	static int64_t currentTimeMillis();

#ifdef _DURANGO
	static void inGameSignInCheckAllPrivilegesCallback(LPVOID lpParam, bool hasPrivileges, int iPad);
	static int InGame_SignInReturned(void *pParam,bool bContinue, int iPad, int iController);
#else
	static int InGame_SignInReturned(void *pParam,bool bContinue, int iPad);
#endif
	// 4J-PB
	Screen * getScreen();

	// 4J Stu
	void forceStatsSave(int idx);

	CRITICAL_SECTION	m_setLevelCS;
private:
	// A bit field that store whether a particular quadrant is in the full tutorial or not
	BYTE m_inFullTutorialBits;
public:
	bool isTutorial();
	void playerStartedTutorial(int iPad);
	void playerLeftTutorial(int iPad);

	// 4J Added
	MultiPlayerLevel *getLevel(int dimension);

	void tickAllConnections();

	Level *animateTickLevel;	// 4J added

	// 4J - When a client requests a texture, it should add it to here while we are waiting for it
	vector<wstring> m_pendingTextureRequests;
	vector<wstring> m_pendingGeometryRequests; // additional skin box geometry

	// 4J Added
	bool addPendingClientTextureRequest(const wstring &textureName);
	void handleClientTextureReceived(const wstring &textureName);
	void clearPendingClientTextureRequests() { m_pendingTextureRequests.clear(); }
	bool addPendingClientGeometryRequest(const wstring &textureName);
	void handleClientGeometryReceived(const wstring &textureName);
	void clearPendingClientGeometryRequests() { m_pendingGeometryRequests.clear(); }

	unsigned int getCurrentTexturePackId();
	ColourTable *getColourTable();

#if defined __ORBIS__
	static int MustSignInReturnedPSN(void *pParam, int iPad, C4JStorage::EMessageResult result);
#endif
};
