#pragma once

#include <memory>

namespace ServerRuntime
{
	class ServerCliEngine;
	class ServerCliInput;

	/**
	 * **Server CLI facade**
	 *
	 * Owns the command engine and input component, and exposes a small lifecycle API.
	 * CLI 全体の開始・停止・更新をまとめる窓口クラス
	 */
	class ServerCli
	{
	public:
		ServerCli();
		~ServerCli();

		/**
		 * **Start console input processing**
		 *
		 * Connects input to the engine and starts background reading.
		 * 入力処理を開始してエンジンに接続
		 */
		void Start();

		/**
		 * **Stop console input processing**
		 *
		 * Stops background input safely and detaches from the engine.
		 * 入力処理を安全に停止
		 */
		void Stop();

		/**
		 * **Process queued command lines**
		 *
		 * Drains commands collected by input and executes them in the main loop.
		 * 入力キューのコマンドを実行
		 */
		void Poll();

	private:
		std::unique_ptr<ServerCliEngine> m_engine;
		std::unique_ptr<ServerCliInput> m_input;
	};
}
