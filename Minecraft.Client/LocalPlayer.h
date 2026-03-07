#pragma once
#include "../Minecraft.World/SmoothFloat.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/Pos.h"
class Level;
class User;
class CompoundTag;
class FurnaceTileEntity;
class DispenserTileEntity;
class SignTileEntity;
class Container;
class Input;
class Stat;
class Minecraft;

using namespace std;

// Time in seconds before the players presence is update to Idle
#define PLAYER_IDLE_TIME 300

class LocalPlayer : public Player
{
public:
	static const int SPRINT_DURATION = 20 * 30;

	eINSTANCEOF GetType() { return eTYPE_LOCALPLAYER; }

	Input *input;
protected:
	Minecraft *minecraft;
	int sprintTriggerTime;
	bool sprintTriggerRegisteredReturn;		// 4J added
	bool twoJumpsRegistered; // 4J added

	unsigned int m_uiInactiveTicks; // To measure time for idle anims

	unsigned long long m_awardedThisSession;

	// 4J - Last time we checked for achievement uunlocks.
	//long long m_lastAchievementUpdate;

public:
	int sprintTime;

	float yBob, xBob;
	float yBobO, xBobO;

	float portalTime;
	float oPortalTime;

	LocalPlayer(Minecraft *minecraft, Level *level, User *user, int dimension);
	virtual ~LocalPlayer();

	int m_iScreenSection; // assuming 4player splitscreen for now, or -1 for single player
	uint64_t ullButtonsPressed; // Stores the button presses, since the inputmanager can be ticked faster than the minecraft
	// player tick, and a button press and release combo can be missed in the minecraft::tick

	uint64_t ullDpad_last;
	uint64_t ullDpad_this;
	uint64_t ullDpad_filtered;

	// 4J-PB - moved these in from the minecraft structure, since they are per player things for splitscreen
	//int ticks;
	int missTime;
	int lastClickTick[2];
	bool isRaining ;
	int m_iThirdPersonView;

	bool m_bHasAwardedStayinFrosty;

private:
	float flyX, flyY, flyZ;

	int jumpRidingTicks;
	float jumpRidingScale;

protected:
	// 4J-PB - player's xbox pad
	int m_iPad;

	bool m_bIsIdle;

private:
	// local player fly
	// --------------------------------------------------------------------------
	// smooth camera settings

	SmoothFloat smoothFlyX;
	SmoothFloat smoothFlyY;
	SmoothFloat smoothFlyZ;

	void calculateFlight(float xa, float ya, float za);

public:
	virtual void serverAiStep();

protected:
	bool isEffectiveAi();

public:
	virtual void aiStep();
	virtual void changeDimension(int i);
	virtual float getFieldOfViewModifier();
	virtual void addAdditonalSaveData(CompoundTag *entityTag);
	virtual void readAdditionalSaveData(CompoundTag *entityTag);
	virtual void closeContainer();
	virtual void openTextEdit(shared_ptr<TileEntity> sign);
	virtual bool openContainer(shared_ptr<Container> container); // 4J added bool return
	virtual bool openHopper(shared_ptr<HopperTileEntity> container); // 4J added bool return
	virtual bool openHopper(shared_ptr<MinecartHopper> container); // 4J added bool return
	virtual bool openHorseInventory(shared_ptr<EntityHorse> horse, shared_ptr<Container> container); // 4J added bool return
	virtual bool startCrafting(int x, int y, int z);					// 4J added bool return
	virtual bool openFireworks(int x, int y, int z);					// 4J added
	virtual bool startEnchanting(int x, int y, int z, const wstring &name);					// 4J added bool return
	virtual bool startRepairing(int x, int y, int z);
	virtual bool openFurnace(shared_ptr<FurnaceTileEntity> furnace); // 4J added bool return
	virtual bool openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand); // 4J added bool return
	virtual bool openBeacon(shared_ptr<BeaconTileEntity> beacon); // 4J added bool return
	virtual bool openTrap(shared_ptr<DispenserTileEntity> trap);		// 4J added bool return
	virtual bool openTrading(shared_ptr<Merchant> traderTarget, const wstring &name);
	virtual void crit(shared_ptr<Entity> e);
	virtual void magicCrit(shared_ptr<Entity> e);
	virtual void take(shared_ptr<Entity> e, int orgCount);
	virtual void chat(const wstring& message);
	virtual bool isSneaking();
	//virtual bool isIdle();
	virtual void hurtTo(float newHealth, ETelemetryChallenges damageSource);
	virtual void respawn();
	virtual void animateRespawn();
	virtual void displayClientMessage(int messageId);
	virtual void awardStat(Stat *stat, byteArray param);
	virtual int	ThirdPersonView()	{ return m_iThirdPersonView;}
	// 4J - have changed 3rd person view to be 0 if not enabled, 1 for mode like original, 2 reversed mode
	virtual void SetThirdPersonView(int val)	{m_iThirdPersonView=val;}

	void ResetInactiveTicks()	{ m_uiInactiveTicks=0;}
	unsigned int GetInactiveTicks()	{ return m_uiInactiveTicks;}
	void IncrementInactiveTicks()	{ if(m_uiInactiveTicks<255) m_uiInactiveTicks++;}

	void mapPlayerChunk(unsigned int);
	// 4J-PB - xbox pad for this player
	void SetXboxPad(int iPad)	{m_iPad=iPad;}
	int GetXboxPad()			{return m_iPad;}
	void SetPlayerRespawned(bool bVal)			{m_bPlayerRespawned=bVal;}
	bool GetPlayerRespawned()					{return m_bPlayerRespawned;}

	// 4J-PB - Moved these in here from the minecraft structure since they are local player related
	void handleMouseDown(int button, bool down);
	bool handleMouseClick(int button);

	// 4J - added for improved autorepeat
	bool creativeModeHandleMouseClick(int button, bool buttonPressed);
	float lastClickX;
	float lastClickY;
	float lastClickZ;
	float lastClickdX;
	float lastClickdY;
	float lastClickdZ;
	enum eLastClickState
	{
		lastClick_invalid,
		lastClick_init,
		lastClick_moving,
		lastClick_stopped,
		lastClick_oldRepeat,
		lastClick_disabled
	};
	float  lastClickTolerance;
	int   lastClickState;

	// 4J Stu - Added to allow callback to tutorial to stay within Minecraft.Client
	virtual void onCrafted(shared_ptr<ItemInstance> item);

	virtual void setAndBroadcastCustomSkin(DWORD skinId);
	virtual void setAndBroadcastCustomCape(DWORD capeId);

private:
	bool isSolidBlock(int x, int y, int z);
	bool m_bPlayerRespawned;

protected:
	bool checkInTile(double x, double y, double z);

public:
	void setSprinting(bool value);
	void setExperienceValues(float experienceProgress, int totalExp, int experienceLevel);

	// virtual void sendMessage(ChatMessageComponent *message); // 4J: removed
	virtual Pos getCommandSenderWorldPosition();
	virtual shared_ptr<ItemInstance> getCarriedItem();
	virtual void playSound(int soundId, float volume, float pitch);
	bool isRidingJumpable();
	float getJumpRidingScale();

protected:
	virtual void sendRidingJump();

public:
	bool hasPermission(EGameCommand command);

	void updateRichPresence();

	// 4J Stu - Added for telemetry
	float m_sessionTimeStart;
	float m_dimensionTimeStart;

	void SetSessionTimerStart(void);
	float getSessionTimer(void);

	float getAndResetChangeDimensionTimer();

	virtual void handleCollectItem(shared_ptr<ItemInstance> item);
	void SetPlayerAdditionalModelParts(vector<ModelPart *>pAdditionalModelParts);

private:
	vector<ModelPart *> m_pAdditionalModelParts;
};


