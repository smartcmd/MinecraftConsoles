#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "..\..\Minecraft.World\ArrayWithLength.h"
#include "..\..\Minecraft.World\CommandsEnum.h"

class GameType;
class ServerPlayer;
class CommandSender;

namespace ServerRuntime
{
	class ServerCliRegistry;

	/**
	 * **CLI execution engine**
	 *
	 * Handles parsing, command dispatch, completion suggestions, and server-side helpers.
	 * 解析・実行・補完エンジン
	 */
	class ServerCliEngine
	{
	public:
		ServerCliEngine();
		~ServerCliEngine();

		/**
		 * **Queue one raw command line**
		 *
		 * Called by input thread; execution is deferred to `Poll()`.
		 * 入力行を実行キューに追加
		 */
		void EnqueueCommandLine(const std::string &line);

		/**
		 * **Execute queued commands**
		 *
		 * Drains pending lines and dispatches them in order.
		 * キュー済みコマンドを順番に実行
		 */
		void Poll();

		/**
		 * **Execute one command line immediately**
		 *
		 * Parses and dispatches a normalized line to a registered command.
		 * 1行を直接パースしてコマンド実行
		 */
		bool ExecuteCommandLine(const std::string &line);

		/**
		 * **Build completion candidates for current line**
		 *
		 * Produces command or argument suggestions based on parser context.
		 * 現在入力に対する補完候補を作成
		 */
		void BuildCompletions(const std::string &line, std::vector<std::string> *out) const;

		void LogInfo(const std::string &message) const;
		void LogWarn(const std::string &message) const;
		void LogError(const std::string &message) const;
		void RequestShutdown() const;

		/**
		 * **List connected players as UTF-8 names**
		 * 
		 * ここら辺は分けてもいいかも
		 */
		std::vector<std::string> GetOnlinePlayerNamesUtf8() const;

		/**
		 * **Find a player by UTF-8 name**
		 */
		std::shared_ptr<ServerPlayer> FindPlayerByNameUtf8(const std::string &name) const;

		/**
		 * **Suggest player-name arguments**
		 *
		 * Appends matching player candidates using the given completion prefix.
		 * プレイヤー名の補完候補
		 */
		void SuggestPlayers(const std::string &prefix, const std::string &linePrefix, std::vector<std::string> *out) const;

		/**
		 * **Suggest gamemode arguments**
		 *
		 * Appends standard gamemode aliases (survival/creative/0/1).
		 * ゲームモードの補完候補
		 */
		void SuggestGamemodes(const std::string &prefix, const std::string &linePrefix, std::vector<std::string> *out) const;

		/**
		 * **Parse gamemode token**
		 *
		 * Supports names, short aliases, and numeric ids.
		 * 文字列からゲームモードを解決
		 */
		GameType *ParseGamemode(const std::string &token) const;

		/**
		 * **Dispatch one Minecraft.World game command**
		 *
		 * Uses `Minecraft.World::CommandDispatcher` for actual execution.
		 * When `sender` is null, an internal console command sender is used.
		 * 
		 * Minecraft.Worldのコマンドを実行するためのディスパッチャーのラッパー
		 * 内部でsenderがnullの場合はコンソールコマンド送信者を使用
		 */
		bool DispatchWorldCommand(EGameCommand command, byteArray commandData, const std::shared_ptr<CommandSender> &sender = nullptr) const;

		const ServerCliRegistry &Registry() const;

	private:
		void RegisterDefaultCommands();

	private:
		mutable std::mutex m_queueMutex;
		std::queue<std::string> m_pendingLines;
		std::unique_ptr<ServerCliRegistry> m_registry;
		std::shared_ptr<CommandSender> m_consoleSender;
	};
}
