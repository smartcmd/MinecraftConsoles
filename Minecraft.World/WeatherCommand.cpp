#include "stdafx.h"
#include "..\Minecraft.Client\MinecraftServer.h"
#include "..\Minecraft.Client\ServerLevel.h"
#include "..\Minecraft.World\net.minecraft.commands.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "GameCommandPacket.h"
#include "WeatherCommand.h"

EGameCommand WeatherCommand::getId()
{
	return eGameCommand_Weather;
}

int WeatherCommand::getPermissionLevel()
{
	return LEVEL_GAMEMASTERS;
}

void WeatherCommand::execute(shared_ptr<CommandSender> source, byteArray commandData)
{
	ByteArrayInputStream bais(commandData);
	DataInputStream dis(&bais);

	wstring weatherType = dis.readUTF();
	int duration = dis.readInt();
	
	bais.reset();

	MinecraftServer* server = MinecraftServer::getInstance();
	ServerLevel* level = server->getLevel(0);
	
	if (level == NULL) return;

	LevelData* levelData = level->getLevelData();
	
	if (duration <= 0)
	{
		Random random;
		duration = (300 + random.nextInt(600)) * SharedConstants::TICKS_PER_SECOND;
	}
	else
	{
		duration = duration * SharedConstants::TICKS_PER_SECOND;
	}

	levelData->setRainTime(duration);
	levelData->setThunderTime(duration);

	if (weatherType == L"clear")
	{
		levelData->setRaining(false);
		levelData->setThundering(false);
		logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.weather.clear");
	}
	else if (weatherType == L"rain")
	{
		levelData->setRaining(true);
		levelData->setThundering(false);
		logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.weather.rain");
	}
	else if (weatherType == L"thunder")
	{
		levelData->setRaining(true);
		levelData->setThundering(true);
		logAdminAction(source, ChatPacket::e_ChatCustom, L"commands.weather.thunder");
	}
}

shared_ptr<GameCommandPacket> WeatherCommand::preparePacket(const wstring& weatherType, int duration)
{
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	dos.writeUTF(weatherType);
	dos.writeInt(duration);

	return shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Weather, baos.toByteArray()));
}