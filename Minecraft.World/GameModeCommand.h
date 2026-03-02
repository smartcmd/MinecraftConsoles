#pragma once

#include "Command.h"

class GameType;

class GameModeCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	GameType *getModeForString(shared_ptr<CommandSender> source, const wstring &name);
	shared_ptr<Player> getPlayer(PlayerUID playerId);
};