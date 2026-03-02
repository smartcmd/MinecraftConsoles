#pragma once

// 4J Stu - Based loosely on the Java versions

#include "CommandsEnum.h"
#include "ChatPacket.h"

class AdminLogCommand;
class CommandSender;
class ServerPlayer;

class Command
{
public:
	// commands such as "help" and "emote"
	static const int LEVEL_ALL = 0;
	// commands such as "mute"
	static const int LEVEL_MODERATORS = 1;
	// commands such as "seed", "tp", "spawnpoint" and "give"
	static const int LEVEL_GAMEMASTERS = 2;
	// commands such as "whitelist", "ban", etc
	static const int LEVEL_ADMINS = 3;
	// commands such as "stop", "save-all", etc
	static const int LEVEL_OWNERS = 4;

private:
	static AdminLogCommand *logger;

public:
	virtual EGameCommand getId() = 0;
	virtual int getPermissionLevel();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData) = 0;
	virtual bool canExecute(shared_ptr<CommandSender> source);

	static void logAdminAction(shared_ptr<CommandSender> source, ChatPacket::EChatPacketMessage messageType, const wstring& message = L"", int customData = -1, const wstring& additionalMessage = L"");
	static void logAdminAction(shared_ptr<CommandSender> source, int type, ChatPacket::EChatPacketMessage messageType, const wstring& message = L"", int customData = -1, const wstring& additionalMessage = L"");
	static void setLogger(AdminLogCommand *logger);

protected:
	shared_ptr<ServerPlayer> getPlayer(PlayerUID playerId);
};