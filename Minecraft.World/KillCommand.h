#pragma once

#include "Command.h"

class KillCommand : public Command
{
public:
	virtual EGameCommand getId();
	virtual int getPermissionLevel();
	virtual void execute(shared_ptr<CommandSender> source, byteArray commandData);
};