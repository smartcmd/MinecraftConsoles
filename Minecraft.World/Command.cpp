#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "..\Minecraft.Client\ServerPlayer.h"
#include "Command.h"

AdminLogCommand *Command::logger;

int Command::getPermissionLevel()
{
	return LEVEL_OWNERS;
}

bool Command::canExecute(shared_ptr<CommandSender> source)
{
	return source->hasPermission(getId());
}

void Command::logAdminAction(shared_ptr<CommandSender> source, ChatPacket::EChatPacketMessage messageType, const wstring& message, int customData, const wstring& additionalMessage)
{
	logAdminAction(source, 0, messageType, message, customData, additionalMessage);
}

void Command::logAdminAction(shared_ptr<CommandSender> source, int type, ChatPacket::EChatPacketMessage messageType, const wstring& message, int customData, const wstring& additionalMessage)
{
	if (logger != NULL)
	{
		logger->logAdminCommand(source, type, messageType, message, customData, additionalMessage);
	}
}

void Command::setLogger(AdminLogCommand *logger)
{
	Command::logger = logger;
}

shared_ptr<ServerPlayer> Command::getPlayer(PlayerUID playerId)
{
	shared_ptr<ServerPlayer> player = MinecraftServer::getInstance()->getPlayers()->getPlayer(playerId);

	if (player == NULL)
	{
		return nullptr;
	}
	else
	{
		return player;
	}
}