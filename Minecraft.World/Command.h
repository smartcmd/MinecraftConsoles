#pragma once

// 4J Stu - Based loosely on the Java versions

#include "CommandsEnum.h"
#include "ChatPacket.h"

class AdminLogCommand;
class CommandSender;
class ServerPlayer;

class Command
{
private:
	static AdminLogCommand *logger;

public:
	virtual EGameCommand getId() = 0;
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData) = 0;
	virtual bool canExecute(shared_ptr<CommandSender> source);

	static void logAdminAction(shared_ptr<CommandSender> source, ChatPacket::EChatPacketMessage messageType, const wstring& message = L"", int customData = -1, const wstring& additionalMessage = L"");
	static void logAdminAction(shared_ptr<CommandSender> source, int type, ChatPacket::EChatPacketMessage messageType, const wstring& message = L"", int customData = -1, const wstring& additionalMessage = L"");
	static void setLogger(AdminLogCommand *logger);

protected:
	shared_ptr<ServerPlayer> getPlayer(PlayerUID playerId);
};