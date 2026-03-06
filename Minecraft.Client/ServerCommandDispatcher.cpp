#include "stdafx.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "..\Minecraft.World\net.minecraft.commands.h"
#include "..\Minecraft.World\net.minecraft.commands.common.h"
#include "TeleportCommand.h"
#include "ServerCommandDispatcher.h"

ServerCommandDispatcher::ServerCommandDispatcher()
{
	addCommand(new TimeCommand());
	addCommand(new GameModeCommand());
	addCommand(new DefaultGameModeCommand());
	addCommand(new KillCommand());
	addCommand(new ToggleDownfallCommand());
	addCommand(new ExperienceCommand());
	addCommand(new TeleportCommand());
	addCommand(new GiveItemCommand());
	addCommand(new EnchantItemCommand());
	//addCommand(new EmoteCommand());
	//addCommand(new ShowSeedCommand());
	//addCommand(new HelpCommand());
	//addCommand(new DebugCommand());
	//addCommand(new MessageCommand());

	//if (MinecraftServer::getInstance()->isDedicatedServer())
	//{
	//	addCommand(new OpCommand());
	//	addCommand(new DeOpCommand());
	//	addCommand(new StopCommand());
	//	addCommand(new SaveAllCommand());
	//	addCommand(new SaveOffCommand());
	//	addCommand(new SaveOnCommand());
	//	addCommand(new BanIpCommand());
	//	addCommand(new PardonIpCommand());
	//	addCommand(new BanPlayerCommand());
	//	addCommand(new ListBansCommand());
	//	addCommand(new PardonPlayerCommand());
	//	addCommand(new KickCommand());
	//	addCommand(new ListPlayersCommand());
	//	addCommand(new BroadcastCommand());
	//	addCommand(new WhitelistCommand());
	//}
	//else
	//{
	//	addCommand(new PublishLocalServerCommand());
	//}

	//        addCommand(new ServerTempDebugCommand());

	Command::setLogger(this);
}

void ServerCommandDispatcher::logAdminCommand(shared_ptr<CommandSender> source, int type, ChatPacket::EChatPacketMessage messageType, const wstring& message, int customData, const wstring& additionalMessage)
{
	PlayerList *playerList = MinecraftServer::getInstance()->getPlayers();
	//for (Player player : MinecraftServer.getInstance().getPlayers().players)
    for (auto& player : playerList->players )
    {
		if ( player && player != source && playerList->isOp(player))
		{
			// TODO: Change chat packet to be able to send more bits of data
			// 4J Stu - Take this out until we can add the name of the player performing the action. Also if the target is a mod then maybe don't need the message?
			//player->sendMessage(message, messageType, customData, additionalMessage);
			//player->sendMessage("\u00A77\u00A7o[" + source.getName() + ": " + player.localize(message, args) + "]");
		}
	}

	if ((type & LOGTYPE_DONT_SHOW_TO_SELF) != LOGTYPE_DONT_SHOW_TO_SELF)
	{
		source->sendMessage(message, messageType, customData, additionalMessage);
	}
}