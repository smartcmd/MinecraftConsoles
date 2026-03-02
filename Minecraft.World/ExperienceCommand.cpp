#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\PlayerList.h"
#include "ExperienceCommand.h"

EGameCommand ExperienceCommand::getId()
{
	return eGameCommand_Experience;
}

void ExperienceCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	//if (args.length > 0)
	//{
	//	Player player;
	//	int amount = convertArgToInt(source, args[0], 0, 5000);

	//	if (args.length > 1) {
	//		player = getPlayer(args[1]);
	//	} else {
	//		player = convertSourceToPlayer(source);
	//	}

	//	player.increaseXp(amount);
	//	logAdminAction(source, "commands.xp.success", amount, player.getAName());
	//}
}

shared_ptr<Player> ExperienceCommand::getPlayer(PlayerUID playerId)
{
	return nullptr;
	//shared_ptr<Player> player = MinecraftServer::getInstance()->getPlayers()->getPlayer(playerId);

	//if (player == null)
	//{
	//	throw new PlayerNotFoundException();
	//} else {
	//	return player;
	//}
}