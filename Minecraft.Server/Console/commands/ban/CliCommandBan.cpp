#include "stdafx.h"

#include "CliCommandBan.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Access\Access.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\..\Minecraft.Client\PlayerConnection.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\..\Minecraft.World\DisconnectPacket.h"

#include <algorithm>

namespace ServerRuntime
{
	namespace
	{
		static void AppendUniqueXuid(PlayerUID xuid, std::vector<PlayerUID> *out)
		{
			if (out == nullptr || xuid == INVALID_XUID)
			{
				return;
			}

			if (std::find(out->begin(), out->end(), xuid) == out->end())
			{
				out->push_back(xuid);
			}
		}

		static void CollectPlayerBanXuids(const std::shared_ptr<ServerPlayer> &player, std::vector<PlayerUID> *out)
		{
			if (player == nullptr || out == nullptr)
			{
				return;
			}

			// Keep both identity variants because the dedicated server checks login and online XUIDs separately.
			AppendUniqueXuid(player->getXuid(), out);
			AppendUniqueXuid(player->getOnlineXuid(), out);
		}
	}

	const char *CliCommandBan::Name() const
	{
		return "ban";
	}

	const char *CliCommandBan::Usage() const
	{
		return "ban <player> [reason ...]";
	}

	const char *CliCommandBan::Description() const
	{
		return "Ban an online player.";
	}

	/**
	 * Resolves the live player, writes one or more Access ban entries, and disconnects the target with the banned reason
	 * 対象プレイヤーを解決してBANを保存し切断する
	 */
	bool CliCommandBan::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2)
		{
			engine->LogWarn("Usage: ban <player> [reason ...]");
			return false;
		}
		if (!ServerRuntime::Access::IsInitialized())
		{
			engine->LogWarn("Access manager is not initialized.");
			return false;
		}

		const auto target = engine->FindPlayerByNameUtf8(line.tokens[1]);
		if (target == nullptr)
		{
			engine->LogWarn("Unknown player: " + line.tokens[1] + " (this server build can only ban players that are currently online).");
			return false;
		}

		std::vector<PlayerUID> xuids;
		CollectPlayerBanXuids(target, &xuids);
		if (xuids.empty())
		{
			engine->LogWarn("Cannot ban that player because no valid XUID is available.");
			return false;
		}

		const bool hasUnbannedIdentity = std::any_of(
			xuids.begin(),
			xuids.end(),
			[](PlayerUID xuid) { return !ServerRuntime::Access::IsPlayerBanned(xuid); });
		if (!hasUnbannedIdentity)
		{
			engine->LogWarn("That player is already banned.");
			return false;
		}

		ServerRuntime::Access::BanMetadata metadata = ServerRuntime::Access::BanManager::BuildDefaultMetadata("Console");
		metadata.reason = StringUtils::JoinTokens(line.tokens, 2);
		if (metadata.reason.empty())
		{
			metadata.reason = "Banned by an operator.";
		}

		const std::string playerName = StringUtils::WideToUtf8(target->getName());
		for (const auto xuid : xuids)
		{
			if (ServerRuntime::Access::IsPlayerBanned(xuid))
			{
				continue;
			}

			if (!ServerRuntime::Access::AddPlayerBan(xuid, playerName, metadata))
			{
				engine->LogError("Failed to write player ban.");
				return false;
			}
		}

		if (target->connection != nullptr)
		{
			target->connection->disconnect(DisconnectPacket::eDisconnect_Banned);
		}

		engine->LogInfo("Banned player " + playerName + ".");
		return true;
	}

	/**
	 * Suggests currently connected player names for the Java-style player argument
	 * プレイヤー引数の補完候補を返す
	 */
	void CliCommandBan::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}

