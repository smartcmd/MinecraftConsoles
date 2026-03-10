#pragma once
#ifdef _WINDOWS64

#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <windows.h>

class MinecraftServer;

class ServerConsole
{
public:
	static const int MAX_HISTORY = 500;
	static const int MAX_INPUT_LENGTH = 512;

	struct Color
	{
		static const char* Reset;
		static const char* Red;
		static const char* Green;
		static const char* Yellow;
		static const char* Blue;
		static const char* Magenta;
		static const char* Cyan;
		static const char* White;
		static const char* Gray;
		static const char* BrightRed;
		static const char* BrightGreen;
		static const char* BrightYellow;
		static const char* BrightBlue;
		static const char* BrightMagenta;
		static const char* BrightCyan;
		static const char* BrightWhite;
	};

	ServerConsole(MinecraftServer* server);
	~ServerConsole();

	void enableVirtualTerminal();

	void run();
	static void logInfo(const char* fmt, ...);
	static void logWarn(const char* fmt, ...);
	static void logError(const char* fmt, ...);
	static void logSuccess(const char* fmt, ...);
	static void logCommand(const char* label, const char* message);
	static void logInfo(const wchar_t* message);
	static void logWarn(const wchar_t* message);
	static void logError(const wchar_t* message);

	std::vector<std::string> getCompletions(const std::string& partial);

	static ServerConsole* getInstance() { return s_instance; }

private:
	void readInputLine(std::string& outLine);
	void redrawInputLine();
	void clearInputLine();
	void handleTabCompletion();
	void insertChar(char c);
	void deleteCharBack();
	void deleteCharForward();
	void moveCursorLeft();
	void moveCursorRight();
	void moveCursorHome();
	void moveCursorEnd();
	void historyUp();
	void historyDown();
	std::string highlightCommand(const std::string& input);
	void printPrompt();

	MinecraftServer* m_server;
	std::string m_inputBuffer;
	int m_cursorPos;
	std::deque<std::string> m_history;
	int m_historyIndex; 
	std::string m_savedInput; 

	std::vector<std::string> m_tabCompletions;
	int m_tabIndex;
	std::string m_tabPartial;

	HANDLE m_hConsoleIn;
	HANDLE m_hConsoleOut;
	bool m_running;

	static ServerConsole* s_instance;
};

#endif // _WINDOWS64
