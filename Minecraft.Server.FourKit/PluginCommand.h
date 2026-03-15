#pragma once

#include "Command.h"
#include "CommandExecutor.h"

using namespace System;

public ref class PluginCommand : public Command
{
public:
	PluginCommand(String^ name)
		: Command(name)
		, m_executor(nullptr)
	{
	}

	virtual bool execute(CommandSender^ sender, String^ commandLabel, cli::array<String^>^ args) override
	{
		if (m_executor == nullptr)
		{
			return false;
		}

		return m_executor->onCommand(sender, this, commandLabel, args);
	}

	void setExecutor(CommandExecutor^ executor)
	{
		m_executor = executor;
	}

	CommandExecutor^ getExecutor()
	{
		return m_executor;
	}

private:
	CommandExecutor^ m_executor;
};
