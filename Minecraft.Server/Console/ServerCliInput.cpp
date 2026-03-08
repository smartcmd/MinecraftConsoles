#include "stdafx.h"

#include "ServerCliInput.h"

#include "ServerCliEngine.h"
#include "..\ServerLogger.h"
#include "..\vendor\linenoise\linenoise.h"

namespace ServerRuntime
{
	// C-style completion callback bridge requires a static instance pointer.
	ServerCliInput *ServerCliInput::s_instance = NULL;

	ServerCliInput::ServerCliInput()
		: m_running(false)
		, m_engine(NULL)
	{
	}

	ServerCliInput::~ServerCliInput()
	{
		Stop();
	}

	void ServerCliInput::Start(ServerCliEngine *engine)
	{
		if (engine == NULL || m_running.exchange(true))
		{
			return;
		}

		m_engine = engine;
		s_instance = this;
		linenoiseResetStop();
		linenoiseHistorySetMaxLen(128);
		linenoiseSetCompletionCallback(&ServerCliInput::CompletionThunk);
		m_inputThread = std::thread(&ServerCliInput::RunInputLoop, this);
		LogInfo("console", "CLI input thread started.");
	}

	void ServerCliInput::Stop()
	{
		if (!m_running.exchange(false))
		{
			return;
		}

		// Ask linenoise to break out first, then join thread safely.
		linenoiseRequestStop();
		if (m_inputThread.joinable())
		{
			m_inputThread.join();
		}
		linenoiseSetCompletionCallback(NULL);

		if (s_instance == this)
		{
			s_instance = NULL;
		}

		m_engine = NULL;
		LogInfo("console", "CLI input thread stopped.");
	}

	bool ServerCliInput::IsRunning() const
	{
		return m_running.load();
	}

	void ServerCliInput::RunInputLoop()
	{
		while (m_running)
		{
			char *line = linenoise("server> ");
			if (line == NULL)
			{
				// NULL is expected on stop request (or Ctrl+C inside linenoise).
				if (!m_running)
				{
					break;
				}
				Sleep(10);
				continue;
			}

			if (line[0] != 0 && m_engine != NULL)
			{
				// Keep local history and forward command for main-thread execution.
				linenoiseHistoryAdd(line);
				m_engine->EnqueueCommandLine(line);
			}

			linenoiseFree(line);
		}
	}

	void ServerCliInput::CompletionThunk(const char *line, linenoiseCompletions *completions)
	{
		// Static thunk forwards callback into instance state.
		if (s_instance != NULL)
		{
			s_instance->BuildCompletions(line, completions);
		}
	}

	void ServerCliInput::BuildCompletions(const char *line, linenoiseCompletions *completions)
	{
		if (line == NULL || completions == NULL || m_engine == NULL)
		{
			return;
		}

		std::vector<std::string> suggestions;
		m_engine->BuildCompletions(line, &suggestions);
		for (size_t i = 0; i < suggestions.size(); ++i)
		{
			linenoiseAddCompletion(completions, suggestions[i].c_str());
		}
	}
}
