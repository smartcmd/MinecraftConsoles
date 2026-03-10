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

public enum class InteractAction
{
	RIGHT_CLICK_BLOCK = 0,
	//LEFT_CLICK_AIR = 1,
	LEFT_CLICK_BLOCK = 2,
	//RIGHT_CLICK_AIR = 3,
	PHYSICAL = 4
};

public enum class BlockFace
{
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
	UP = 4,
	DOWN = 5,
	NORTH_EAST = 6,
	NORTH_WEST = 7,
	SOUTH_EAST = 8,
	SOUTH_WEST = 9,
	WEST_NORTH_WEST = 10,
	NORTH_NORTH_WEST = 11,
	NORTH_NORTH_EAST = 12,
	EAST_NORTH_EAST = 13,
	EAST_SOUTH_EAST = 14,
	SOUTH_SOUTH_EAST = 15,
	SOUTH_SOUTH_WEST = 16,
	WEST_SOUTH_WEST = 17,
	SELF = 18
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

public ref class PlayerInteractEvent
{
  public:
    void setCancelled(bool cancelled)
    {
        Cancelled = cancelled;
    }
    bool isCancelled()
    {
        return Cancelled;
    }
    InteractAction getAction()
    {
        return Action;
    }
    Player^ getPlayer() { return PlayerObject; }
    Block^ getClickedBlock() { return ClickedBlock; }
    BlockFace getBlockFace()
    {
        return Face;
    }
    bool hasItem()
    {
        return HasItem;
    }
internal:
    property Player^ PlayerObject;
    property InteractAction Action;
    property Block^ ClickedBlock;
    property BlockFace Face;
    property bool HasBlock;
    property bool HasItem;
    property bool Cancelled;
};

public ref class PlayerDeathEvent
{
public:
    Player^ getEntity() { return PlayerObject; }
    String^ getDeathMessage() { return DeathMessage; }
    void setDeathMessage(String^ msg) { DeathMessage = msg; }
    bool getKeepInventory() { return KeepInventory; }
    bool getKeepLevel() { return KeepLevel; }
    int getNewExp() { return NewExp; }
    int getNewLevel() { return NewLevel; }
    //int getNewTotalExp() { return NewTotalExp; }
    void setKeepInventory(bool val) { KeepInventory = val; }
    void setKeepLevel(bool val) { KeepLevel = val; }
    void setNewExp(int exp) { NewExp = exp; }
    void setNewLevel(int level) { NewLevel = level; }
    //void setNewTotalExp(int totalExp) { NewTotalExp = totalExp; }
internal:
    property Player^ PlayerObject;
    property String^ DeathMessage;
    property bool KeepInventory;
    property bool KeepLevel;
    property int NewExp;
    property int NewLevel;
    property int NewTotalExp;
};
