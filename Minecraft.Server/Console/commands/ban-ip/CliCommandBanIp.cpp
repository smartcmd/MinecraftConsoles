#include "stdafx.h"

#include "CliCommandBanIp.h"

#include "..\..\ServerCliEngine.h"
#include "..\..\ServerCliParser.h"
#include "..\..\..\Access\Access.h"
#include "..\..\..\Common\NetworkUtils.h"
#include "..\..\..\Common\StringUtils.h"
#include "..\..\..\ServerLogManager.h"
#include "..\..\..\..\Minecraft.Client\MinecraftServer.h"
#include "..\..\..\..\Minecraft.Client\PlayerConnection.h"
#include "..\..\..\..\Minecraft.Client\PlayerList.h"
#include "..\..\..\..\Minecraft.Client\ServerPlayer.h"
#include "..\..\..\..\Minecraft.World\Connection.h"
#include "..\..\..\..\Minecraft.World\DisconnectPacket.h"

namespace ServerRuntime
{
	namespace
	{
		// The dedicated server keeps the accepted remote IP in ServerLogManager, keyed by connection smallId.
		// It's a bit strange from a responsibility standpoint, so we'll need to implement it separately.
		static bool TryGetPlayerRemoteIp(const std::shared_ptr<ServerPlayer> &player, std::string *outIp)
		{
			if (outIp == nullptr || player == nullptr || player->connection == nullptr || player->connection->connection == nullptr || player->connection->connection->getSocket() == nullptr)
			{
				return false;
			}

			const unsigned char smallId = player->connection->connection->getSocket()->getSmallId();
			if (smallId == 0)
			{
				return false;
			}

			return ServerRuntime::ServerLogManager::TryGetConnectionRemoteIp(smallId, outIp);
		}

		// After persisting the ban, walk a snapshot of current players so every matching session is removed.
		static int DisconnectPlayersByRemoteIp(const std::string &ip)
		{
			auto *server = MinecraftServer::getInstance();
			if (server == nullptr || server->getPlayers() == nullptr)
			{
				return 0;
			}

			const std::string normalizedIp = NetworkUtils::NormalizeIpToken(ip);
			const std::vector<std::shared_ptr<ServerPlayer>> playerSnapshot = server->getPlayers()->players;
			int disconnectedCount = 0;
			for (const auto &player : playerSnapshot)
			{
				std::string playerIp;
				if (!TryGetPlayerRemoteIp(player, &playerIp))
				{
					continue;
				}

				if (NetworkUtils::NormalizeIpToken(playerIp) == normalizedIp)
				{
					if (player != nullptr && player->connection != nullptr)
					{
						player->connection->disconnect(DisconnectPacket::eDisconnect_Banned);
						++disconnectedCount;
					}
				}
			}

			return disconnectedCount;
		}
	}

	const char *CliCommandBanIp::Name() const
	{
		return "ban-ip";
	}

	const char *CliCommandBanIp::Usage() const
	{
		return "ban-ip <address|player> [reason ...]";
	}

	const char *CliCommandBanIp::Description() const
	{
		return "Ban an IP address or a player's current IP.";
	}

	/**
	 * Resolves either a literal IP or an online player's current IP, persists the ban, and disconnects every matching connection
	 * IPまたは接続中プレイヤーの現在IPをBANし一致する接続を切断する
	 */
	bool CliCommandBanIp::Execute(const ServerCliParsedLine &line, ServerCliEngine *engine)
	{
		if (line.tokens.size() < 2)
		{
			engine->LogWarn("Usage: ban-ip <address|player> [reason ...]");
			return false;
		}
		if (!ServerRuntime::Access::IsInitialized())
		{
			engine->LogWarn("Access manager is not initialized.");
			return false;
		}

		const std::string targetToken = line.tokens[1];
		std::string remoteIp;
		// Match Java Edition behavior by accepting either a literal IP or an online player name.
		const auto targetPlayer = engine->FindPlayerByNameUtf8(targetToken);
		if (targetPlayer != nullptr)
		{
			if (!TryGetPlayerRemoteIp(targetPlayer, &remoteIp))
			{
				engine->LogWarn("Cannot ban that player's IP because no current remote IP is available.");
				return false;
			}
		}
		else if (NetworkUtils::IsIpLiteral(targetToken))
		{
			remoteIp = StringUtils::TrimAscii(targetToken);
		}
		else
		{
			engine->LogWarn("Unknown player or invalid IP address: " + targetToken);
			return false;
		}

		// Refuse duplicate bans so operators get immediate feedback instead of rewriting the same entry.
		if (ServerRuntime::Access::IsIpBanned(remoteIp))
		{
			engine->LogWarn("That IP address is already banned.");
			return false;
		}

		ServerRuntime::Access::BanMetadata metadata = ServerRuntime::Access::BanManager::BuildDefaultMetadata("Console");
		metadata.reason = StringUtils::JoinTokens(line.tokens, 2);
		if (metadata.reason.empty())
		{
			metadata.reason = "Banned by an operator.";
		}

		// Publish the ban before disconnecting players so reconnect attempts are rejected immediately.
		if (!ServerRuntime::Access::AddIpBan(remoteIp, metadata))
		{
			engine->LogError("Failed to write IP ban.");
			return false;
		}

		const int disconnectedCount = DisconnectPlayersByRemoteIp(remoteIp);
		// Report the resolved IP rather than the original token so player-name targets are explicit in the console.
		engine->LogInfo("Banned IP address " + remoteIp + ".");
		if (disconnectedCount > 0)
		{
			engine->LogInfo("Disconnected " + std::to_string(disconnectedCount) + " player(s) with that IP.");
		}
		return true;
	}

	/**
	 * Suggests online player names for the player-target form of the Java Edition command
	 * プレイヤー名指定時の補完候補を返す
	 */
	void CliCommandBanIp::Complete(const ServerCliCompletionContext &context, const ServerCliEngine *engine, std::vector<std::string> *out) const
	{
		if (context.currentTokenIndex == 1)
		{
			engine->SuggestPlayers(context.prefix, context.linePrefix, out);
		}
	}
}

