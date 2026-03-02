#pragma once

#include "Command.h"

class EffectCommand : public Command
{
public:
	EGameCommand getId();
	int getPermissionLevel();
	wstring getUsage(CommandSender *source);
	void execute(shared_ptr<CommandSender> source, byteArray commandData);

protected:
	wstring getPlayerNames();

public:
	bool isValidWildcardPlayerArgument(wstring args, int argumentIndex);
};