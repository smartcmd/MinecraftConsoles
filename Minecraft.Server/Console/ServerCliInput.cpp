#include "stdafx.h"

#include "ServerCliInput.h"

#include "ServerCliEngine.h"
#include "..\ServerLogger.h"
#include "..\vendor\linenoise\linenoise.h"

#include <ctype.h>
#include <stdlib.h>

namespace
{
	bool UseStreamInputMode()
	{
		const char *mode = getenv("SERVER_CLI_INPUT_MODE");
		if (mode != NULL)
		{
			return _stricmp(mode, "stream") == 0
				|| _stricmp(mode, "stdin") == 0;
		}

		return false;
	}

	int WaitForStdinReadable(HANDLE stdinHandle, DWORD waitMs)
	{
		if (stdinHandle == NULL || stdinHandle == INVALID_HANDLE_VALUE)
		{
			return -1;
		}

		DWORD fileType = GetFileType(stdinHandle);
		if (fileType == FILE_TYPE_PIPE)
		{
			DWORD available = 0;
			if (!PeekNamedPipe(stdinHandle, NULL, 0, NULL, &available, NULL))
			{
				return -1;
			}
			return available > 0 ? 1 : 0;
		}

		if (fileType == FILE_TYPE_CHAR)
		{
			// console/pty char handles are often not waitable across Wine+Docker.
			return 1;
		}

		DWORD waitResult = WaitForSingleObject(stdinHandle, waitMs);
		if (waitResult == WAIT_OBJECT_0)
		{
			return 1;
		}
		if (waitResult == WAIT_TIMEOUT)
		{
			return 0;
		}

		return -1;
	}
}

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
			CancelSynchronousIo((HANDLE)m_inputThread.native_handle());
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
		if (UseStreamInputMode())
		{
			LogInfo("console", "CLI input mode: stream(file stdin)");
			RunStreamInputLoop();
			return;
		}

		RunLinenoiseLoop();
	}

	/**
	 *  use linenoise for interactive console input, with line editing and history support
	 */
	void ServerCliInput::RunLinenoiseLoop()
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

			EnqueueLine(line);

			linenoiseFree(line);
		}
	}

	/**
	 * use file-based stdin reading instead of linenoise when requested or when stdin is not a console/pty (e.g. piped input or non-interactive docker)
	 */
	void ServerCliInput::RunStreamInputLoop()
	{
		HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
		if (stdinHandle == NULL || stdinHandle == INVALID_HANDLE_VALUE)
		{
			LogWarn("console", "stream input mode requested but STDIN handle is unavailable; falling back to linenoise.");
			RunLinenoiseLoop();
			return;
		}

		std::string line;
		bool skipNextLf = false;

		printf("server> ");
		fflush(stdout);

		while (m_running)
		{
			int readable = WaitForStdinReadable(stdinHandle, 50);
			if (readable <= 0)
			{
				Sleep(10);
				continue;
			}

			char ch = 0;
			DWORD bytesRead = 0;
			if (!ReadFile(stdinHandle, &ch, 1, &bytesRead, NULL) || bytesRead == 0)
			{
				Sleep(10);
				continue;
			}

			if (skipNextLf && ch == '\n')
			{
				skipNextLf = false;
				continue;
			}

			if (ch == '\r' || ch == '\n')
			{
				if (ch == '\r')
				{
					skipNextLf = true;
				}
				else
				{
					skipNextLf = false;
				}

				if (!line.empty())
				{
					EnqueueLine(line.c_str());
					line.clear();
				}

				printf("server> ");
				fflush(stdout);
				continue;
			}

			skipNextLf = false;

			if ((unsigned char)ch == 3)
			{
				continue;
			}

			if ((unsigned char)ch == 8 || (unsigned char)ch == 127)
			{
				if (!line.empty())
				{
					line.resize(line.size() - 1);
				}
				continue;
			}

			if (isprint((unsigned char)ch) && line.size() < 4096)
			{
				line.push_back(ch);
			}
		}
	}

	void ServerCliInput::EnqueueLine(const char *line)
	{
		if (line == NULL || line[0] == 0 || m_engine == NULL)
		{
			return;
		}

		// Keep local history and forward command for main-thread execution.
		linenoiseHistoryAdd(line);
		m_engine->EnqueueCommandLine(line);
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
