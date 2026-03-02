#pragma once
#include "Command.h"

class GameCommandPacket;

class ToggleDownfallCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual int getPermissionLevel();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	void doToggleDownfall();

public:
	static shared_ptr<GameCommandPacket> preparePacket();
};