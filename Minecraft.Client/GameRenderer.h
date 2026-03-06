#pragma once

#ifdef _WIN64
#include <d3d11.h>
#endif

class Minecraft;
class Entity;
class Random;
class FloatBuffer;
class ItemInHandRenderer;
class DataLayer;
class SparseLightStorage;
class CompressedTileStorage;
class SparseDataStorage;

#include "..\Minecraft.World\SmoothFloat.h"
#include "..\Minecraft.World\C4JThread.h"
#include "ResourceLocation.h"

class GameRenderer
{
private:
	static ResourceLocation RAIN_LOCATION;
    static ResourceLocation SNOW_LOCATION;

public:
	static bool anaglyph3d;
    static int anaglyphPass;

private:
	Minecraft *mc;
    float renderDistance;
public:
	ItemInHandRenderer *itemInHandRenderer;
private:
	int _tick;
    shared_ptr<Entity> hovered;

    // smooth camera movement
    SmoothFloat smoothTurnX;
    SmoothFloat smoothTurnY;

    // third-person distance etc
	SmoothFloat smoothDistance;
	SmoothFloat smoothRotation;
	SmoothFloat smoothTilt;
	SmoothFloat smoothRoll;
	float thirdDistance;
	float thirdDistanceO;
	float thirdRotation;
	float thirdRotationO;
	float thirdTilt;
	float thirdTiltO;
    float accumulatedSmoothXO, accumulatedSmoothYO;
    float tickSmoothXO, tickSmoothYO, lastTickA;
	Vec3 *cameraPos;		// 4J added

    // fov modification
	float fovOffset;
	float fovOffsetO;

    // roll modification
    float cameraRoll;
    float cameraRollO;

	// 4J - changes brought forward from 1.8.2
	static const int NUM_LIGHT_TEXTURES = 4;// * 3;
	int		 lightTexture[NUM_LIGHT_TEXTURES];		// 4J - changed so that we have one lightTexture per level, to support split screen
	int		 getLightTexture(int iPad, Level *level);			// 4J added
	intArray lightPixels[NUM_LIGHT_TEXTURES];

	float fov[4];
	float oFov[4];
	float tFov[4];

	float darkenWorldAmount;
    float darkenWorldAmountO;

    // Gamma caching
    float m_cachedGammaPerPlayer[NUM_LIGHT_TEXTURES];
    static float ComputeGammaFromSlider(float slider0to100);
    void CachePlayerGammas();
    void ApplyGammaPostProcess() const;
    bool ComputeViewportForPlayer(int j, D3D11_VIEWPORT& outViewport) const;
    uint32_t BuildPlayerViewports(D3D11_VIEWPORT* outViewports, float* outGammas, UINT maxCount) const;

    bool isInClouds;

	float m_fov;
public:
	GameRenderer(Minecraft *mc);
	~GameRenderer();
	void SetFovVal(float fov);
	float GetFovVal();

public:
	void tick(bool bFirst);
    void pick(float a);
private:
	void tickFov();
	float getFov(float a, bool applyEffects);
    void bobHurt(float a);
    void bobView(float a);
    void moveCameraToPlayer(float a);
    double zoom;
    double zoom_x;
    double zoom_y;
public:
	void zoomRegion(double zoom, double xa, double ya);
    void unZoomRegion();
private:
	void getFovAndAspect(float& fov, float& aspect, float a, bool applyEffects); // 4J added
public:
	void setupCamera(float a, int eye);
private:
    void renderItemInHand(float a, int eye);
    __int64 lastActiveTime;
    __int64 lastNsTime;
	// 4J - changes brought forward from 1.8.2
	bool _updateLightTexture;
public:
	float blr;
	float blrt;
	float blg;
	float blgt;
	void turnOffLightLayer(double alpha);
	void turnOnLightLayer(double alpha);
private:
	void tickLightTexture();
	void updateLightTexture(float a);
	float getNightVisionScale(shared_ptr<Player> player, float a);
public:
	void render(float a, bool bFirst);		// 4J added bFirst
    void renderLevel(float a);
    void renderLevel(float a, __int64 until);
private:
	Random *random;
    int rainSoundTime;
	void prepareAndRenderClouds(LevelRenderer *levelRenderer, float a);
    void tickRain();
private:
	// 4J - brought forward from 1.8.2
	float *rainXa;
	float *rainZa;
protected:
	void renderSnowAndRain(float a);
    volatile int xMod;
    volatile int yMod;
public:
	void setupGuiScreen(int forceScale=-1);	// 4J - added forceScale parameter

    FloatBuffer *lb;
    float fr;
    float fg;
    float fb;
private:
	void setupClearColor(float a);
    float fogBrO, fogBr;
	int cameraFlip;

    void setupFog(int i, float alpha);
    FloatBuffer *getBuffer(float a, float b, float c, float d);
	static int getFpsCap(int option);
public:
	void updateAllChunks();

#ifdef MULTITHREAD_ENABLE
	static C4JThread*	m_updateThread;
	static int runUpdate(LPVOID lpParam);
	static C4JThread::EventArray* m_updateEvents;
	enum EUpdateEvents
	{
		eUpdateCanRun,
		eUpdateEventIsFinished,
		eUpdateEventCount,
	};
	static bool			nearThingsToDo;
	static bool			updateRunning;
#endif
	static vector<byte *> m_deleteStackByte;
	static vector<SparseLightStorage *> m_deleteStackSparseLightStorage;
	static vector<CompressedTileStorage *> m_deleteStackCompressedTileStorage;
	static vector<SparseDataStorage *> m_deleteStackSparseDataStorage;
	static CRITICAL_SECTION m_csDeleteStack;
	static void         AddForDelete(byte *deleteThis);
	static void         AddForDelete(SparseLightStorage *deleteThis);
	static void         AddForDelete(CompressedTileStorage *deleteThis);
	static void         AddForDelete(SparseDataStorage *deleteThis);
	static void			FinishedReassigning();
	void				EnableUpdateThread();
	void				DisableUpdateThread();
};
