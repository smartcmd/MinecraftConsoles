#pragma once

#include "Player.h"
#include "Block.h"

using namespace System;

public ref class Location
{
public:
	Location(double x, double y, double z) : m_x(x), m_y(y), m_z(z) {}

	double getX() { return m_x; }
	double getY() { return m_y; }
	double getZ() { return m_z; }

private:
	double m_x;
	double m_y;
	double m_z;
};

public ref class ServerLoadEvent
{
public:
	property String^ ServerName;
};

public ref class ServerShutdownEvent
{
public:
	property String^ Reason;
};

public ref class PlayerJoinEvent
{
public:
	property Player^ PlayerObject;
	
	Player^ getPlayer() { return PlayerObject; }
};

public ref class PlayerLeaveEvent
{
public:
	property Player^ PlayerObject;
	
	Player^ getPlayer() { return PlayerObject; }
};

public ref class PlayerChatEvent
{
public:
	property Player^ PlayerObject;
	property String^ Message;
	property bool Cancelled;

	Player^ getPlayer() { return PlayerObject; }
	String^ getMessage() { return Message; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
};

public ref class BlockBreakEvent
{
public:
	property Player^ PlayerObject;
	property Block^ BlockObject;
	property bool Cancelled;

	Player^ getPlayer() { return PlayerObject; }
	Block^ getBlock() { return BlockObject; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
};

public ref class BlockPlaceEvent
{
public:
	property Player^ PlayerObject;
	property Block^ BlockObject;
	property bool Cancelled;

	Player^ getPlayer() { return PlayerObject; }
	Block^ getBlock() { return BlockObject; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
};

public ref class PlayerMoveEvent
{
public:
	property Player^ PlayerObject;
	property Location^ From;
	property Location^ To;
	property bool Cancelled;

	Player^ getPlayer() { return PlayerObject; }
	Location^ getFrom() { return From; }
	Location^ getTo() { return To; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
};
