#pragma once

#include "Command.h"

class CommandSender;

class ExperienceCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	shared_ptr<Player> getPlayer(PlayerUID playerId);
};