#pragma once

#include "Player.h"
#include "Block.h"

using namespace System;

public enum class TeleportCause
{
	END_PORTAL = 0,
	ENDER_PEARL = 1,
	NETHER_PORTAL = 2,
	PLUGIN = 3,
	UNKNOWN = 4
};

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
	Player^ getPlayer() { return PlayerObject; }
internal:
	property Player^ PlayerObject;
};

public ref class PlayerLeaveEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
internal:
	property Player^ PlayerObject;
};

public ref class PlayerChatEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
	String^ getMessage() { return Message; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
internal:
	property Player^ PlayerObject;
	property String^ Message;
	property bool Cancelled;
};

public ref class BlockBreakEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
	Block^ getBlock() { return BlockObject; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
internal:
	property Player^ PlayerObject;
	property Block^ BlockObject;
	property bool Cancelled;
};

public ref class BlockPlaceEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
	Block^ getBlock() { return BlockObject; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
internal:
	property Player ^ PlayerObject;
    property Block ^ BlockObject;
    property bool Cancelled;
};

public ref class PlayerMoveEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
	Location^ getFrom() { return From; }
	Location^ getTo() { return To; }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
internal: 
	property Player ^ PlayerObject;
    property Location ^ From;
    property Location ^ To;
    property bool Cancelled;
};

public ref class PlayerPortalEvent
{
public:
	Player^ getPlayer() { return PlayerObject; }
	TeleportCause getCause() { return Cause; }
	Location^ getFrom() { return From; }
	Location^ getTo() { return To; }
	void setTo(double x, double y, double z) { To = gcnew Location(x, y, z); }
	void setFrom(double x, double y, double z) { From = gcnew Location(x, y, z); }
	bool isCancelled() { return Cancelled; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
internal: 
	property Player ^ PlayerObject;
    property TeleportCause Cause;
    property Location ^ From;
    property Location ^ To;
    property bool Cancelled;
};

public ref class SignChangeEvent
{
public:
	String^ getLine(int index)
	{
		if (Lines == nullptr || index < 0 || index >= Lines->Length) return String::Empty;
		return Lines[index];
	}
	Block^ getBlock() { return BlockObject; }
	cli::array<String^>^ getLines() { return Lines; }
	Player^ getPlayer() { return PlayerObject; }
	void setCancelled(bool cancelled) { Cancelled = cancelled; }
	void setLine(int index, String^ content)
	{
		if (Lines == nullptr || index < 0 || index >= Lines->Length) return;
		Lines[index] = (content == nullptr) ? String::Empty : content;
	}
	bool isCancelled() { return Cancelled; }
internal:
	property Player ^ PlayerObject;
    property Block ^ BlockObject;
    property cli::array<String ^> ^ Lines;
    property bool Cancelled;
};
