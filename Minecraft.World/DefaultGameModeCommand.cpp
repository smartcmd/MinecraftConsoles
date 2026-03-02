#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "DefaultGameModeCommand.h"

EGameCommand DefaultGameModeCommand::getId()
{
	return eGameCommand_DefaultGameMode;
}

void DefaultGameModeCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	//if (args.length > 0)
	//{
	//	GameType newMode = getModeForString(source, args[0]);
	//	doSetGameType(newMode);

	//	String modeName = I18n.get("gameMode." + newMode.getName());
	//	logAdminAction(source, "commands.defaultgamemode.success", modeName);

	//}
}

void DefaultGameModeCommand::doSetGameType(GameType *newGameType)
{
	//MinecraftServer::getInstance()->setDefaultGameMode(newGameType);
}