#pragma once
#include "Command.h"

class GameCommandPacket;

class WeatherCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual int getPermissionLevel();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

	static shared_ptr<GameCommandPacket> preparePacket(const wstring& weatherType, int duration = -1);
};