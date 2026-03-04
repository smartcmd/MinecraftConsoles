#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "net.minecraft.network.packet.h"
#include "TimeCommand.h"

EGameCommand TimeCommand::getId()
{
	return eGameCommand_Time;
}

int TimeCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

void TimeCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	ByteArrayInputStream bais(commandData);
	DataInputStream dis(&bais);

	bool night = dis.readBoolean();

	bais.reset();

	int amount = 0;
	if(night) amount = 12500;
	doSetTime(source, amount);
	//logAdminAction(source, "commands.time.set", amount);
	logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.time.set");

	//if (args.length > 1) {
	//	if (args[0].equals("set")) {
	//		int amount;

	//		if (args[1].equals("day")) {
	//			amount = 0;
	//		} else if (args[1].equals("night")) {
	//			amount = 12500;
	//		} else {
	//			amount = convertArgToInt(source, args[1], 0);
	//		}

	//		doSetTime(source, amount);
	//		logAdminAction(source, "commands.time.set", amount);
	//		return;
	//	} else if (args[0].equals("add")) {
	//		int amount = convertArgToInt(source, args[1], 0);
	//		doAddTime(source, amount);

	//		logAdminAction(source, "commands.time.added", amount);
	//		return;
	//	}
	//}

	//throw new UsageException("commands.time.usage");
}

void TimeCommand::doSetTime(shared_ptr<CommandSender> source, int value)
{
	for (int i = 0; i < MinecraftServer::getInstance()->levels.length; i++)
	{
		MinecraftServer::getInstance()->levels[i]->setDayTime(value);
	}
}

void TimeCommand::doAddTime(shared_ptr<CommandSender> source, int value)
{
	for (int i = 0; i < MinecraftServer::getInstance()->levels.length; i++)
	{
		ServerLevel *level = MinecraftServer::getInstance()->levels[i];
		level->setDayTime(level->getDayTime() + value);
	}
}

shared_ptr<GameCommandPacket> TimeCommand::preparePacket(bool night)
{
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	dos.writeBoolean(night);

	return shared_ptr<GameCommandPacket>( new GameCommandPacket(eGameCommand_Time, baos.toByteArray() ));
}