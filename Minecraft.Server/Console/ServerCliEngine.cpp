#include "stdafx.h"

#include "ServerCliEngine.h"

#include "ServerCliParser.h"
#include "ServerCliRegistry.h"
#include "commands\IServerCliCommand.h"
#include "commands\CliCommandBan.h"
#include "commands\CliCommandBanIp.h"
#include "commands\CliCommandBanList.h"
#include "commands\CliCommandGamemode.h"
#include "commands\CliCommandHelp.h"
#include "commands\CliCommandList.h"
#include "commands\CliCommandPardon.h"
#include "commands\CliCommandPardonIp.h"
#include "commands\CliCommandStop.h"
#include "commands\CliCommandTp.h"
#include "commands\CliCommandWhitelist.h"
#include "..\Common\StringUtils.h"
#include "..\ServerShutdown.h"
#include "..\ServerLogger.h"
#include "..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\Minecraft.Client\PlayerList.h"
#include "..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\Minecraft.World\LevelSettings.h"
#include "..\..\Minecraft.World\StringHelpers.h"

#include <stdlib.h>
#include <unordered_set>

namespace ServerRuntime
{
	ServerCliEngine::ServerCliEngine()
		: m_registry(new ServerCliRegistry())
	{
		RegisterDefaultCommands();
	}

	ServerCliEngine::~ServerCliEngine()
	{
	}

	void ServerCliEngine::RegisterDefaultCommands()
	{
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandHelp()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandStop()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandList()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandBan()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandBanIp()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandPardon()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandPardonIp()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandBanList()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandWhitelist()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandTp()));
		m_registry->Register(std::unique_ptr<IServerCliCommand>(new CliCommandGamemode()));
	}

	void ServerCliEngine::EnqueueCommandLine(const std::string &line)
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_pendingLines.push(line);
	}

	void ServerCliEngine::Poll()
	{
		for (;;)
		{
			std::string line;
			{
				// Keep the lock scope minimal: dequeue only, execute outside.
				std::lock_guard<std::mutex> lock(m_queueMutex);
				if (m_pendingLines.empty())
				{
					break;
				}
				line = m_pendingLines.front();
				m_pendingLines.pop();
			}

			ExecuteCommandLine(line);
		}
	}

	bool ServerCliEngine::ExecuteCommandLine(const std::string &line)
	{
		// Normalize user input before parsing (trim + optional leading slash).
		std::wstring wide = trimString(StringUtils::Utf8ToWide(line));
		if (wide.empty())
		{
			return true;
		}

		std::string normalizedLine = StringUtils::WideToUtf8(wide);
		if (!normalizedLine.empty() && normalizedLine[0] == '/')
		{
			normalizedLine = normalizedLine.substr(1);
		}

		ServerCliParsedLine parsed = ServerCliParser::Parse(normalizedLine);
		if (parsed.tokens.empty())
		{
			return true;
		}

		IServerCliCommand *command = m_registry->FindMutable(parsed.tokens[0]);
		if (command == NULL)
		{
			LogWarn("Unknown command: " + parsed.tokens[0]);
			return false;
		}

		return command->Execute(parsed, this);
	}

	void ServerCliEngine::BuildCompletions(const std::string &line, std::vector<std::string> *out) const
	{
		if (out == NULL)
		{
			return;
		}

		out->clear();
		ServerCliCompletionContext context = ServerCliParser::BuildCompletionContext(line);
		bool slashPrefixedCommand = false;
		std::string commandToken;
		if (!context.parsed.tokens.empty())
		{
			// Completion accepts both "tp" and "/tp" style command heads.
			commandToken = context.parsed.tokens[0];
			if (!commandToken.empty() && commandToken[0] == '/')
			{
				commandToken = commandToken.substr(1);
				slashPrefixedCommand = true;
			}
		}

		if (context.currentTokenIndex == 0)
		{
			std::string prefix = context.prefix;
			if (!prefix.empty() && prefix[0] == '/')
			{
				prefix = prefix.substr(1);
				slashPrefixedCommand = true;
			}

			std::string linePrefix = context.linePrefix;
			if (slashPrefixedCommand && linePrefix.empty())
			{
				// Preserve leading slash when user started with "/".
				linePrefix = "/";
			}

			m_registry->SuggestCommandNames(prefix, linePrefix, out);
		}
		else
		{
			const IServerCliCommand *command = m_registry->Find(commandToken);
			if (command != NULL)
			{
				command->Complete(context, this, out);
			}
		}

		std::unordered_set<std::string> seen;
		std::vector<std::string> unique;
		for (size_t i = 0; i < out->size(); ++i)
		{
			// Remove duplicates while keeping first-seen ordering.
			if (seen.insert((*out)[i]).second)
			{
				unique.push_back((*out)[i]);
			}
		}
		out->swap(unique);
	}

	void ServerCliEngine::LogInfo(const std::string &message) const
	{
		LogInfof("console", "%s", message.c_str());
	}

	void ServerCliEngine::LogWarn(const std::string &message) const
	{
		LogWarnf("console", "%s", message.c_str());
	}

	void ServerCliEngine::LogError(const std::string &message) const
	{
		LogErrorf("console", "%s", message.c_str());
	}

	void ServerCliEngine::RequestShutdown() const
	{
		RequestDedicatedServerShutdown();
	}

	std::vector<std::string> ServerCliEngine::GetOnlinePlayerNamesUtf8() const
	{
		std::vector<std::string> result;
		MinecraftServer *server = MinecraftServer::getInstance();
		if (server == NULL)
		{
			return result;
		}

		PlayerList *players = server->getPlayers();
		if (players == NULL)
		{
			return result;
		}

		for (size_t i = 0; i < players->players.size(); ++i)
		{
			std::shared_ptr<ServerPlayer> player = players->players[i];
			if (player != NULL)
			{
				result.push_back(StringUtils::WideToUtf8(player->getName()));
			}
		}

		return result;
	}

	std::shared_ptr<ServerPlayer> ServerCliEngine::FindPlayerByNameUtf8(const std::string &name) const
	{
		MinecraftServer *server = MinecraftServer::getInstance();
		if (server == NULL)
		{
			return nullptr;
		}

		PlayerList *players = server->getPlayers();
		if (players == NULL)
		{
			return nullptr;
		}

		std::wstring target = StringUtils::Utf8ToWide(name);
		for (size_t i = 0; i < players->players.size(); ++i)
		{
			std::shared_ptr<ServerPlayer> player = players->players[i];
			if (player != NULL && equalsIgnoreCase(player->getName(), target))
			{
				return player;
			}
		}

		return nullptr;
	}

	void ServerCliEngine::SuggestPlayers(const std::string &prefix, const std::string &linePrefix, std::vector<std::string> *out) const
	{
		std::vector<std::string> players = GetOnlinePlayerNamesUtf8();
		std::string loweredPrefix = StringUtils::ToLowerAscii(prefix);
		for (size_t i = 0; i < players.size(); ++i)
		{
			std::string loweredName = StringUtils::ToLowerAscii(players[i]);
			if (loweredName.compare(0, loweredPrefix.size(), loweredPrefix) == 0)
			{
				out->push_back(linePrefix + players[i]);
			}
		}
	}

	void ServerCliEngine::SuggestGamemodes(const std::string &prefix, const std::string &linePrefix, std::vector<std::string> *out) const
	{
		static const char *kModes[] = { "survival", "creative", "s", "c", "0", "1" };
		std::string loweredPrefix = StringUtils::ToLowerAscii(prefix);
		for (size_t i = 0; i < sizeof(kModes) / sizeof(kModes[0]); ++i)
		{
			std::string candidate = kModes[i];
			std::string loweredCandidate = StringUtils::ToLowerAscii(candidate);
			if (loweredCandidate.compare(0, loweredPrefix.size(), loweredPrefix) == 0)
			{
				out->push_back(linePrefix + candidate);
			}
		}
	}

	GameType *ServerCliEngine::ParseGamemode(const std::string &token) const
	{
		std::string lowered = StringUtils::ToLowerAscii(token);
		if (lowered == "survival" || lowered == "s" || lowered == "0")
		{
			return GameType::SURVIVAL;
		}
		if (lowered == "creative" || lowered == "c" || lowered == "1")
		{
			return GameType::CREATIVE;
		}

		char *end = NULL;
		long id = strtol(lowered.c_str(), &end, 10);
		if (end != NULL && *end == 0)
		{
			// Numeric fallback supports extended ids handled by level settings.
			return LevelSettings::validateGameType((int)id);
		}

		return NULL;
	}

	const ServerCliRegistry &ServerCliEngine::Registry() const
	{
		return *m_registry;
	}
}

