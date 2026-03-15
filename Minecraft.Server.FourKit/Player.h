#pragma once

#include "Entity.h"
#include "InetSocketAddress.h"
#include "ItemStack.h"

// todo: improve a little

using namespace System;

ref class World;

public enum class GameMode
{
	SURVIVAL = 0,
	CREATIVE = 1,
	ADVENTURE = 2
};

public ref class Player : public Entity
{
public:
	Player(String^ name);

	String^ getName() { return m_name; }
	float getHealth() { return m_health; }
	int getFood() { return m_food; }
	virtual float getFallDistance() override { return m_fallDistance; }
	float getYRot() { return m_yRot; }
	float getXRot() { return m_xRot; }
	bool isSneaking() { return m_sneaking; }
	bool isSprinting() { return m_sprinting; }
	virtual bool isInsideVehicle() { return m_insideVehicle; }
	virtual double getX() override { return m_x; }
	virtual double getY() override { return m_y; }
	virtual double getZ() override { return m_z; }
	virtual int getDimension() override { return m_dimension; }
	virtual EntityType getType() override { return EntityType::PLAYER; }
	InetSocketAddress^ getAddress();

	virtual void setFallDistance(float distance) override;
	void setHealth(float health);
	void setFood(int food);
	//void setSneaking(bool sneaking); doesnt work rn
	void setSprinting(bool sprinting);
	virtual void sendMessage(String^ message) override;
	void kickPlayer(); // String^ reason
	bool banPlayer(String^ reason);
	bool banPlayerIp(String^ reason);
	void teleport(double x, double y, double z);
	virtual bool teleport(Location^ location) override;

	bool getAllowFlight();
	void setAllowFlight(bool flight);
	float getExhaustion();
	void setExhaustion(float value);
	float getSaturation();
	void setSaturation(float value);
	void giveExp(int amount);
	void giveExpLevels(int amount);
	int getTotalExperience();
	bool isFlying();
	void setFlying(bool value);
	void setExp(float exp);
	void setLevel(int level);
	void setWalkSpeed(float value);
	float getWalkSpeed();
	ItemStack^ getItemInHand();
	void setItemInHand(ItemStack^ item);
	GameMode getGameMode();
	void setGameMode(GameMode mode);
	bool isSleeping();

internal:
	property String ^
		name {
			String ^ get() { return m_name; }
		}

	property float health
	{
		float get()
		{
			return m_health;
		}
	}
	property int food
	{
		int get()
		{
			return m_food;
		}
	}
	property float fallDistance
	{
		float get()
		{
			return m_fallDistance;
		}
	}
	property float yRot
	{
		float get()
		{
			return m_yRot;
		}
	}
	property float xRot
	{
		float get()
		{
			return m_xRot;
		}
	}
	property bool sneaking
	{
		bool get()
		{
			return m_sneaking;
		}
	}
	property bool sprinting
	{
		bool get()
		{
			return m_sprinting;
		}
	}

	property double x
	{
		double get()
		{
			return m_x;
		}
	}
	property double y
	{
		double get()
		{
			return m_y;
		}
	}
	property double z
	{
		double get()
		{
			return m_z;
		}
	}
	property int dimension
	{
		int get()
		{
			return m_dimension;
		}
	}
	void SetPlayerData(float health, int food, float fallDistance, float yRot, float xRot,
					   bool sneaking, bool sprinting, bool insideVehicle,
					   double x, double y, double z, int dimension);

private:
	String^ m_name;

	float m_health;
	int m_food;
	float m_yRot;
	float m_xRot;
	bool m_sneaking;
	bool m_sprinting;
	bool m_insideVehicle;
};
