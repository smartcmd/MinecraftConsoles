#pragma once

#include "Command.h"

class GameCommandPacket;

class GiveItemCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual int getPermissionLevel();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

public:
	static shared_ptr<GameCommandPacket> preparePacket(shared_ptr<Player> player, int item, int amount = 1, int aux = 0, const wstring &tag = L"");
};