#pragma once

class Command;
class CommandSender;

class CommandDispatcher
{
private:
#ifdef __ORBIS__
	unordered_map<EGameCommand, Command *,std::hash<int>> commandsById;
#else
	unordered_map<EGameCommand, Command *> commandsById;
#endif
	unordered_set<Command *> commands;

public:
	void performCommand(shared_ptr<CommandSender> sender, EGameCommand command, byteArray commandData);
	Command *addCommand(Command *command);
};