#pragma once

#include <atomic>
#include <thread>

struct linenoiseCompletions;

namespace ServerRuntime
{
	class ServerCliEngine;

	/**
	 * **CLI input worker**
	 *
	 * Owns the interactive input thread and bridges linenoise callbacks to the engine.
	 * 入力スレッドと補完コールバックを管理するクラス
	 */
	class ServerCliInput
	{
	public:
		ServerCliInput();
		~ServerCliInput();

		/**
		 * **Start input loop**
		 *
		 * Binds to an engine and starts reading user input from the console.
		 * エンジンに接続して入力ループを開始
		 */
		void Start(ServerCliEngine *engine);

		/**
		 * **Stop input loop**
		 *
		 * Requests stop and joins the input thread.
		 * 停止要求を出して入力スレッドを終了
		 */
		void Stop();

		/**
		 * **Check running state**
		 *
		 * Returns true while the input thread is active.
		 * 入力処理が動作中かどうか
		 */
		bool IsRunning() const;

	private:
		void RunInputLoop();
		void RunLinenoiseLoop();
		void RunStreamInputLoop();
		void EnqueueLine(const char *line);
		static void CompletionThunk(const char *line, linenoiseCompletions *completions);
		void BuildCompletions(const char *line, linenoiseCompletions *completions);

	private:
		std::atomic<bool> m_running;
		std::thread m_inputThread;
		ServerCliEngine *m_engine;

		static ServerCliInput *s_instance;
	};
}
