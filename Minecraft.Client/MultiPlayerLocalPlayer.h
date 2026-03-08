#pragma once
#include "LocalPlayer.h"
#include "../Minecraft.World/SharedConstants.h"

class ClientConnection;
class Minecraft;
class Level;

//#define STRESS_TEST_MOVE

class MultiplayerLocalPlayer : public LocalPlayer
{
private:
	static const int POSITION_REMINDER_INTERVAL = SharedConstants::TICKS_PER_SECOND;
public:
	ClientConnection *connection;
private:
    bool flashOnSetHealth;
public:
	MultiplayerLocalPlayer(Minecraft *minecraft, Level *level, User *user, ClientConnection *connection);
private:
	double xLast, yLast1, yLast2, zLast;
    float yRotLast, xRotLast;
public:
	virtual bool hurt(DamageSource *source, float dmg);
    virtual void heal(float heal);
    virtual void tick();
private:
	bool lastOnGround;
    bool lastSneaked;
	bool lastIdle;
	bool lastSprinting;
    int positionReminder;
public:
	void sendPosition();

	using Player::drop;
    virtual shared_ptr<ItemEntity> drop();
protected:
	virtual void reallyDrop(shared_ptr<ItemEntity> itemEntity);
public:
	virtual void chat(const wstring& message);
    virtual void swing();
    virtual void respawn();
protected:
	virtual void actuallyHurt(DamageSource *source, float dmg);

	// 4J Added override to capture event for tutorial messages
	virtual void completeUsingItem();

	// 4J Added overrides to capture events for tutorial
	virtual void onEffectAdded(MobEffectInstance *effect);
	virtual void onEffectUpdated(MobEffectInstance *effect, bool doRefreshAttributes);
	virtual void onEffectRemoved(MobEffectInstance *effect);
public:
	virtual void closeContainer();
	void clientSideCloseContainer();
    virtual void hurtTo(float newHealth, ETelemetryChallenges damageSource);
    virtual void awardStat(Stat *stat, byteArray param);
    void awardStatFromServer(Stat *stat, byteArray param);
	void onUpdateAbilities();
	bool isLocalPlayer();

protected:
	virtual void sendRidingJump();

public:
    virtual void sendOpenInventory();

	// 4J - send the custom skin texture data if there is one
	//void CustomSkin(PBYTE pbData, DWORD dwBytes);

	// 4J Overriding this so we can flag an event for the tutorial
	virtual void ride(shared_ptr<Entity> e);

	// 4J - added for the Stop Sleeping
	virtual void StopSleeping();

	// 4J Added
	virtual void setAndBroadcastCustomSkin(DWORD skinId);
	virtual void setAndBroadcastCustomCape(DWORD capeId);

	// 4J added for testing
#ifdef STRESS_TEST_MOVE
	void StressTestMove(double *tempX, double *tempY, double *tempZ);
#endif
};
