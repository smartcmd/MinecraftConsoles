#include "stdafx.h"

#include "ServerLogManager.h"

#include "Common\StringUtils.h"
#include "ServerLogger.h"

#include <array>
#include <mutex>

extern bool g_Win64DedicatedServer;

namespace ServerRuntime
{
	namespace ServerLogManager
	{
		namespace
		{
			/**
			 * **!! This information is managed solely for logging purposes, but it is questionable from a liability perspective, so it will eventually need to be separated !!**
			 * 
			 * Tracks the remote IP and accepted player name associated with one `smallId`
			 * 1つのsmallIdに紐づく接続IPとプレイヤー名を保持する
			 */
			struct ConnectionLogEntry
			{
				std::string remoteIp;
				std::string playerName;
			};

			/**
			 * Owns the shared connection cache used by hook points running on different threads
			 * 複数スレッドのhookから共有される接続キャッシュを保持する
			 */
			struct ServerLogState
			{
				std::mutex stateLock;
				std::array<ConnectionLogEntry, 256> entries;
			};

			ServerLogState g_serverLogState;

			static bool IsDedicatedServerLoggingEnabled()
			{
				return g_Win64DedicatedServer;
			}

			static void ResetConnectionLogEntry(ConnectionLogEntry *entry)
			{
				if (entry == NULL)
				{
					return;
				}

				entry->remoteIp.clear();
				entry->playerName.clear();
			}

			static std::string NormalizeRemoteIp(const char *ip)
			{
				if (ip == NULL || ip[0] == 0)
				{
					return std::string("unknown");
				}

				return std::string(ip);
			}

			static std::string NormalizePlayerName(const std::wstring &playerName)
			{
				std::string playerNameUtf8 = StringUtils::WideToUtf8(playerName);
				if (playerNameUtf8.empty())
				{
					playerNameUtf8 = "<unknown>";
				}

				return playerNameUtf8;
			}

            // Default to the main app channel when the caller does not provide a source tag.
            static const char *NormalizeClientLogSource(const char *source)
            {
                if (source == NULL || source[0] == 0)
                {
                    return "app";
                }

                return source;
            }

            static void EmitClientDebugLogLine(const char *source, const std::string &line)
            {
                if (line.empty())
                {
                    return;
                }

                LogDebugf("client", "[%s] %s", NormalizeClientLogSource(source), line.c_str());
            }

            // Split one debug payload into individual lines so each line becomes a prompt-safe server log entry.
            static void ForwardClientDebugMessage(const char *source, const char *message)
            {
                if (message == NULL || message[0] == 0)
                {
                    return;
                }

                const char *cursor = message;
                while (*cursor != 0)
                {
                    const char *lineStart = cursor;
                    while (*cursor != 0 && *cursor != '\r' && *cursor != '\n')
                    {
                        ++cursor;
                    }

                    // Split multi-line client debug output into prompt-safe server log entries.
                    if (cursor > lineStart)
                    {
                        EmitClientDebugLogLine(source, std::string(lineStart, (size_t)(cursor - lineStart)));
                    }

                    while (*cursor == '\r' || *cursor == '\n')
                    {
                        ++cursor;
                    }
                }
            }

            // Share the same formatting path for app, user, and legacy debug-spew forwards.
            static void ForwardFormattedClientDebugLogV(const char *source, const char *format, va_list args)
            {
                if (!IsDedicatedServerLoggingEnabled() || format == NULL || format[0] == 0)
                {
                    return;
                }

                char messageBuffer[2048] = {};
                vsnprintf_s(messageBuffer, sizeof(messageBuffer), _TRUNCATE, format, args);
                ForwardClientDebugMessage(source, messageBuffer);
            }

            static const char *TcpRejectReasonToString(ETcpRejectReason reason)
			{
				switch (reason)
				{
				case eTcpRejectReason_BannedIp: return "banned-ip";
				case eTcpRejectReason_GameNotReady: return "game-not-ready";
				case eTcpRejectReason_ServerFull: return "server-full";
				default: return "unknown";
				}
			}

			static const char *LoginRejectReasonToString(ELoginRejectReason reason)
			{
				switch (reason)
				{
				case eLoginRejectReason_BannedXuid: return "banned-xuid";
				case eLoginRejectReason_NotWhitelisted: return "not-whitelisted";
				case eLoginRejectReason_DuplicateXuid: return "duplicate-xuid";
				case eLoginRejectReason_DuplicateName: return "duplicate-name";
				default: return "unknown";
				}
			}

			static const char *DisconnectReasonToString(DisconnectPacket::eDisconnectReason reason)
			{
				switch (reason)
				{
				case DisconnectPacket::eDisconnect_None: return "none";
				case DisconnectPacket::eDisconnect_Quitting: return "quitting";
				case DisconnectPacket::eDisconnect_Closed: return "closed";
				case DisconnectPacket::eDisconnect_LoginTooLong: return "login-too-long";
				case DisconnectPacket::eDisconnect_IllegalStance: return "illegal-stance";
				case DisconnectPacket::eDisconnect_IllegalPosition: return "illegal-position";
				case DisconnectPacket::eDisconnect_MovedTooQuickly: return "moved-too-quickly";
				case DisconnectPacket::eDisconnect_NoFlying: return "no-flying";
				case DisconnectPacket::eDisconnect_Kicked: return "kicked";
				case DisconnectPacket::eDisconnect_TimeOut: return "timeout";
				case DisconnectPacket::eDisconnect_Overflow: return "overflow";
				case DisconnectPacket::eDisconnect_EndOfStream: return "end-of-stream";
				case DisconnectPacket::eDisconnect_ServerFull: return "server-full";
				case DisconnectPacket::eDisconnect_OutdatedServer: return "outdated-server";
				case DisconnectPacket::eDisconnect_OutdatedClient: return "outdated-client";
				case DisconnectPacket::eDisconnect_UnexpectedPacket: return "unexpected-packet";
				case DisconnectPacket::eDisconnect_ConnectionCreationFailed: return "connection-creation-failed";
				case DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesHost: return "no-multiplayer-privileges-host";
				case DisconnectPacket::eDisconnect_NoMultiplayerPrivilegesJoin: return "no-multiplayer-privileges-join";
				case DisconnectPacket::eDisconnect_NoUGC_AllLocal: return "no-ugc-all-local";
				case DisconnectPacket::eDisconnect_NoUGC_Single_Local: return "no-ugc-single-local";
				case DisconnectPacket::eDisconnect_ContentRestricted_AllLocal: return "content-restricted-all-local";
				case DisconnectPacket::eDisconnect_ContentRestricted_Single_Local: return "content-restricted-single-local";
				case DisconnectPacket::eDisconnect_NoUGC_Remote: return "no-ugc-remote";
				case DisconnectPacket::eDisconnect_NoFriendsInGame: return "no-friends-in-game";
				case DisconnectPacket::eDisconnect_Banned: return "banned";
				case DisconnectPacket::eDisconnect_NotFriendsWithHost: return "not-friends-with-host";
				case DisconnectPacket::eDisconnect_NATMismatch: return "nat-mismatch";
				default: return "unknown";
				}
			}
		}

        // Only forward client-side debug output while the process is running as the dedicated server.
        bool ShouldForwardClientDebugLogs()
        {
            return IsDedicatedServerLoggingEnabled();
        }

        void ForwardClientAppDebugLogV(const char *format, va_list args)
        {
            ForwardFormattedClientDebugLogV("app", format, args);
        }

        void ForwardClientUserDebugLogV(int user, const char *format, va_list args)
        {
            char source[32] = {};
            _snprintf_s(source, sizeof(source), _TRUNCATE, "app:user=%d", user);
            ForwardFormattedClientDebugLogV(source, format, args);
        }

        void ForwardClientDebugSpewLogV(const char *format, va_list args)
        {
            ForwardFormattedClientDebugLogV("debug-spew", format, args);
        }

        // Clear every cached connection slot during startup so stale metadata never leaks into future logs.
        void Initialize()
		{
			std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
			for (size_t index = 0; index < g_serverLogState.entries.size(); ++index)
			{
				ResetConnectionLogEntry(&g_serverLogState.entries[index]);
			}
		}

			// Reuse Initialize as the shutdown cleanup path because both operations wipe the cache.
		void Shutdown()
		{
			Initialize();
		}

		// Log the raw socket arrival before a smallId is assigned so early rejects still have an IP in the logs.
		void OnIncomingTcpConnection(const char *ip)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			const std::string remoteIp = NormalizeRemoteIp(ip);
			LogInfof("network", "incoming tcp connection from %s", remoteIp.c_str());
		}

		// TCP rejects happen before connection state is cached, so log directly from the supplied remote IP.
		void OnRejectedTcpConnection(const char *ip, ETcpRejectReason reason)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			const std::string remoteIp = NormalizeRemoteIp(ip);
			LogWarnf("network", "rejected tcp connection from %s: reason=%s", remoteIp.c_str(), TcpRejectReasonToString(reason));
		}

			// Cache the accepted remote IP immediately so later login and disconnect logs can reuse it.
		void OnAcceptedTcpConnection(unsigned char smallId, const char *ip)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			const std::string remoteIp = NormalizeRemoteIp(ip);
			{
				std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
				ConnectionLogEntry &entry = g_serverLogState.entries[smallId];
				ResetConnectionLogEntry(&entry);
				entry.remoteIp = remoteIp;
			}

			LogInfof("network", "accepted tcp connection from %s as smallId=%u", remoteIp.c_str(), (unsigned)smallId);
		}

		// Once login succeeds, bind the resolved player name onto the cached transport entry.
		void OnAcceptedPlayerLogin(unsigned char smallId, const std::wstring &playerName)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			const std::string playerNameUtf8 = NormalizePlayerName(playerName);
			std::string remoteIp("unknown");
			{
				std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
				ConnectionLogEntry &entry = g_serverLogState.entries[smallId];
				entry.playerName = playerNameUtf8;
				if (!entry.remoteIp.empty())
				{
					remoteIp = entry.remoteIp;
				}
			}

			LogInfof("network", "accepted player login: name=\"%s\" ip=%s smallId=%u", playerNameUtf8.c_str(), remoteIp.c_str(), (unsigned)smallId);
		}

		// Read the cached IP for the rejection log, then clear the slot because the player never fully joined.
		void OnRejectedPlayerLogin(unsigned char smallId, const std::wstring &playerName, ELoginRejectReason reason)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			const std::string playerNameUtf8 = NormalizePlayerName(playerName);
			std::string remoteIp("unknown");
			{
				std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
				ConnectionLogEntry &entry = g_serverLogState.entries[smallId];
				if (!entry.remoteIp.empty())
				{
					remoteIp = entry.remoteIp;
				}
				ResetConnectionLogEntry(&entry);
			}

			LogWarnf("network", "rejected login from %s: name=\"%s\" reason=%s", remoteIp.c_str(), playerNameUtf8.c_str(), LoginRejectReasonToString(reason));
		}

		// Disconnect logging is the final consumer of cached metadata, so it also clears the slot afterward.
		void OnPlayerDisconnected(
			unsigned char smallId,
			const std::wstring &playerName,
			DisconnectPacket::eDisconnectReason reason,
			bool initiatedByServer)
		{
			if (!IsDedicatedServerLoggingEnabled())
			{
				return;
			}

			std::string playerNameUtf8 = NormalizePlayerName(playerName);
			std::string remoteIp("unknown");
			{
				// Copy state under lock and emit the log after unlocking so CLI output never blocks connection bookkeeping.
				std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
				ConnectionLogEntry &entry = g_serverLogState.entries[smallId];
				if (!entry.remoteIp.empty())
				{
					remoteIp = entry.remoteIp;
				}
				if (playerNameUtf8 == "<unknown>" && !entry.playerName.empty())
				{
					playerNameUtf8 = entry.playerName;
				}
				ResetConnectionLogEntry(&entry);
			}

			LogInfof(
				"network",
				"%s: name=\"%s\" ip=%s smallId=%u reason=%s",
				initiatedByServer ? "disconnecting player" : "player disconnected",
				playerNameUtf8.c_str(),
				remoteIp.c_str(),
				(unsigned)smallId,
				DisconnectReasonToString(reason));
		}

		/** 
		 * For logging purposes, the responsibility is technically misplaced, but the IP is cached in `LogManager`.
		 * Those cached values are then used to retrieve the player's IP.
		 * 
		 * Eventually, this should be implemented in a separate class or on the `Minecraft.Client` side instead.
		 */
		bool TryGetConnectionRemoteIp(unsigned char smallId, std::string *outIp)
		{
			if (!IsDedicatedServerLoggingEnabled() || outIp == NULL)
			{
				return false;
			}

			std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
			const ConnectionLogEntry &entry = g_serverLogState.entries[smallId];
			if (entry.remoteIp.empty() || entry.remoteIp == "unknown")
			{
				return false;
			}

			*outIp = entry.remoteIp;
			return true;
		}

		// Provide explicit cache cleanup for paths that terminate without going through disconnect logging.
		void ClearConnection(unsigned char smallId)
		{
			std::lock_guard<std::mutex> stateLock(g_serverLogState.stateLock);
			ResetConnectionLogEntry(&g_serverLogState.entries[smallId]);
		}
	}
}
