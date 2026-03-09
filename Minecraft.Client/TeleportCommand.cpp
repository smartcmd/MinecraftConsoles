#include "stdafx.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "PlayerConnection.h"
#include "..\Minecraft.World\net.minecraft.commands.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "TeleportCommand.h"

EGameCommand TeleportCommand::getId()
{
	return eGameCommand_Teleport;
}

void TeleportCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	ByteArrayInputStream bais(commandData);
	DataInputStream dis(&bais);

	PlayerUID subjectID = dis.readPlayerUID();
	PlayerUID destinationID = dis.readPlayerUID();
	
	bais.reset();

	PlayerList *players = MinecraftServer::getInstance()->getPlayerList();

	shared_ptr<ServerPlayer> subject = players->getPlayer(subjectID);
	shared_ptr<ServerPlayer> destination = players->getPlayer(destinationID);

	if(subject != nullptr && destination != nullptr && subject->level->dimension->id == destination->level->dimension->id && subject->isAlive() )
	{
		subject->ride(nullptr);
		subject->connection->teleport(destination->x, destination->y, destination->z, destination->yRot, destination->xRot);
		//logAdminAction(source, "commands.tp.success", subject->getAName(), destination->getAName());
		logAdminAction(source, ChatPacket::e_ChatCommandTeleportSuccess, subject->getName(), eTYPE_SERVERPLAYER, destination->getName());

		if(subject == source)
		{
			destination->sendMessage(subject->getName(), ChatPacket::e_ChatCommandTeleportToMe);
		}
		else
		{
			subject->sendMessage(destination->getName(), ChatPacket::e_ChatCommandTeleportMe);
		}
	}

	//if (args.length >= 1) {
	//	MinecraftServer server = MinecraftServer.getInstance();
	//	ServerPlayer victim;

	//	if (args.length == 2 || args.length == 4) {
	//		victim = server.getPlayers().getPlayer(args[0]);
	//		if (victim == null) throw new PlayerNotFoundException();
	//	} else {
	//		victim = (ServerPlayer) convertSourceToPlayer(source);
	//	}

	//	if (args.length == 3 || args.length == 4) {
	//		if (victim.level != null) {
	//			int pos = args.length - 3;
	//			int maxPos = Level.MAX_LEVEL_SIZE;
	//			int x = convertArgToInt(source, args[pos++], -maxPos, maxPos);
	//			int y = convertArgToInt(source, args[pos++], Level.minBuildHeight, Level.maxBuildHeight);
	//			int z = convertArgToInt(source, args[pos++], -maxPos, maxPos);

	//			victim.teleportTo(x + 0.5f, y, z + 0.5f);
	//			logAdminAction(source, "commands.tp.coordinates", victim.getAName(), x, y, z);
	//		}
	//	} else if (args.length == 1 || args.length == 2) {
	//		ServerPlayer destination = server.getPlayers().getPlayer(args[args.length - 1]);
	//		if (destination == null) throw new PlayerNotFoundException();

	//		victim.connection.teleport(destination.x, destination.y, destination.z, destination.yRot, destination.xRot);
	//		logAdminAction(source, "commands.tp.success", victim.getAName(), destination.getAName());
	//	}
	//}
}

shared_ptr<GameCommandPacket> TeleportCommand::preparePacket(PlayerUID subject, PlayerUID destination)
{
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	dos.writePlayerUID(subject);
	dos.writePlayerUID(destination);

	return std::make_shared<GameCommandPacket>(eGameCommand_Teleport, baos.toByteArray());
}