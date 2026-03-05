#include "stdafx.h"
#include "net.minecraft.commands.h"
#include "CommandDispatcher.h"

int CommandDispatcher::performCommand(shared_ptr<CommandSender> sender, EGameCommand command, byteArray commandData)
{
	auto it = commandsById.find(command);

	if(it != commandsById.end())
	{
		Command *command = it->second;
		if (command->canExecute(sender))
		{
			command->execute(sender, commandData);
		}
		else
		{
#ifndef _CONTENT_PACKAGE
			sender->sendMessage(L"\u00A7cYou do not have permission to use this command.");
#endif
		}
	}
	else
	{
		app.DebugPrintf("Command %d not found!\n", command);
	}

	return 0;
}

Command *CommandDispatcher::addCommand(Command *command)
{
	commandsById[command->getId()] = command;
	commands.insert(command);
	return command;
}