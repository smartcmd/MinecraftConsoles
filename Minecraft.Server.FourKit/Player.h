#pragma once

#include "InetSocketAddress.h"
#include "CommandSender.h"

// todo: improve a little

using namespace System;

public ref class Player : public CommandSender
{
public:
	Player(String^ name);
	
	String^ getName() { return m_name; }
	float getHealth() { return m_health; }
	int getFood() { return m_food; }
	float getFallDistance() { return m_fallDistance; }
	float getYRot() { return m_yRot; }
	float getXRot() { return m_xRot; }
	bool isSneaking() { return m_sneaking; }
	bool isSprinting() { return m_sprinting; }
	double getX() { return m_x; }
	double getY() { return m_y; }
	double getZ() { return m_z; }
	int getDimension() { return m_dimension; }
	InetSocketAddress^ getAddress();
	
	void setFallDistance(float distance);
	void setHealth(float health);
	void setFood(int food);
	//void setSneaking(bool sneaking); doesnt work rn
	void setSprinting(bool sprinting);
	virtual void sendMessage(String^ message) override;
    void kickPlayer(); // String^ reason
	void teleport(double x, double y, double z);

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
	                   bool sneaking, bool sprinting,
	                   double x, double y, double z, int dimension);

private:
	String^ m_name;
	
	float m_health;
	int m_food;
	float m_fallDistance;
	float m_yRot;
	float m_xRot;
	bool m_sneaking;
	bool m_sprinting;
	
	double m_x;
	double m_y;
	double m_z;
	int m_dimension;
};
