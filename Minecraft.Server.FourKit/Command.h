#pragma once

#include "CommandSender.h"

using namespace System;
using namespace System::Collections::Generic;

public ref class Command abstract
{
public:
	Command(String^ name)
	{
		m_name = String::IsNullOrEmpty(name) ? String::Empty : name;
		m_aliases = gcnew List<String^>();
		m_description = String::Empty;
		m_usage = String::Empty;
	}

	virtual bool execute(CommandSender^ sender, String^ commandLabel, cli::array<String^>^ args) = 0;

	String^ getName()
	{
		return m_name;
	}

	List<String^>^ getAliases()
	{
		return m_aliases;
	}

	void setAliases(List<String^>^ aliases)
	{
		m_aliases = (aliases != nullptr) ? aliases : gcnew List<String^>();
	}

	String^ getDescription()
	{
		return m_description;
	}

	void setDescription(String^ description)
	{
		m_description = (description != nullptr) ? description : String::Empty;
	}

	String^ getUsage()
	{
		return m_usage;
	}

	void setUsage(String^ usage)
	{
		m_usage = (usage != nullptr) ? usage : String::Empty;
	}

private:
	String^ m_name;
	List<String^>^ m_aliases;
	String^ m_description;
	String^ m_usage;
};
