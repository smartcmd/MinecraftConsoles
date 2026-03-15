#pragma once

ref class Command;
ref class CommandSender;

using namespace System;

public interface class CommandExecutor
{
	bool onCommand(CommandSender^ sender, Command^ command, String^ label, cli::array<String^>^ args);
};
