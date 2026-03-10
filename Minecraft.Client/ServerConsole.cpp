#include "stdafx.h"

#ifdef _WINDOWS64

#include "ServerConsole.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\EntityIO.h"
#include "..\Minecraft.World\Tile.h"
#include "..\Minecraft.World\MobEffect.h"
#include "..\Minecraft.World\LevelSettings.h"

#include <cstdarg>
#include <algorithm>
#include <sstream>
#include <cstdio>

ServerConsole* ServerConsole::s_instance = nullptr;

const char* ServerConsole::Color::Reset       = "\033[0m";
const char* ServerConsole::Color::Red         = "\033[31m";
const char* ServerConsole::Color::Green       = "\033[32m";
const char* ServerConsole::Color::Yellow      = "\033[33m";
const char* ServerConsole::Color::Blue        = "\033[34m";
const char* ServerConsole::Color::Magenta     = "\033[35m";
const char* ServerConsole::Color::Cyan        = "\033[36m";
const char* ServerConsole::Color::White       = "\033[37m";
const char* ServerConsole::Color::Gray        = "\033[90m";
const char* ServerConsole::Color::BrightRed   = "\033[91m";
const char* ServerConsole::Color::BrightGreen = "\033[92m";
const char* ServerConsole::Color::BrightYellow= "\033[93m";
const char* ServerConsole::Color::BrightBlue  = "\033[94m";
const char* ServerConsole::Color::BrightMagenta="\033[95m";
const char* ServerConsole::Color::BrightCyan  = "\033[96m";
const char* ServerConsole::Color::BrightWhite = "\033[97m";

// Known command names for completion and highlighting
static const char* s_commandNames[] = {
	"help", "stop", "list", "say", "save-all",
	"kill", "gamemode", "teleport", "tp", "give", "giveitem",
	"enchant", "enchantitem", "time", "weather", "summon",
	"setblock", "fill", "effect", "clear", "vanish", "op", "deop",
	"kick", "ban", "unban", "pardon", "banlist",
	nullptr
};

// Subcommand completions for specific commands
static const char* s_timeSubcmds[]    = { "set", "add", "query", nullptr };
static const char* s_timeSetValues[]  = { "day", "night", "noon", "midnight", nullptr };
static const char* s_weatherTypes[]   = { "clear", "rain", "thunder", nullptr };
static const char* s_gamemodeNames[]  = { "survival", "creative", "adventure", "0", "1", "2", "s", "c", "a", nullptr };
static const char* s_effectActions[]  = { "give", "clear", nullptr };

ServerConsole::ServerConsole(MinecraftServer* server)
	: m_server(server)
	, m_cursorPos(0)
	, m_historyIndex(-1)
	, m_tabIndex(-1)
	, m_running(true)
{
	m_hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	m_hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	s_instance = this;
}

ServerConsole::~ServerConsole()
{
	if (s_instance == this)
		s_instance = nullptr;
}

void ServerConsole::enableVirtualTerminal()
{
	DWORD dwMode = 0;
	if (GetConsoleMode(m_hConsoleOut, &dwMode))
	{
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(m_hConsoleOut, dwMode);
	}

	if (GetConsoleMode(m_hConsoleIn, &dwMode))
	{
		// Disable line input and echo so we can handle raw key events
		dwMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
		dwMode |= ENABLE_WINDOW_INPUT;
		SetConsoleMode(m_hConsoleIn, dwMode);
	}
}

void ServerConsole::logInfo(const char* fmt, ...)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf("%s[INFO]%s %s\n", Color::BrightCyan, Color::Reset, buf);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logWarn(const char* fmt, ...)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf("%s[WARN]%s %s%s%s\n", Color::BrightYellow, Color::Reset, Color::Yellow, buf, Color::Reset);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logError(const char* fmt, ...)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf("%s[ERROR]%s %s%s%s\n", Color::BrightRed, Color::Reset, Color::Red, buf, Color::Reset);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logSuccess(const char* fmt, ...)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	char buf[2048];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printf("%s[INFO]%s %s%s%s\n", Color::BrightGreen, Color::Reset, Color::Green, buf, Color::Reset);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logCommand(const char* label, const char* message)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	printf("%s[%s]%s %s\n", Color::BrightMagenta, label, Color::Reset, message);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logInfo(const wchar_t* message)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	printf("%s[INFO]%s %ls\n", Color::BrightCyan, Color::Reset, message);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logWarn(const wchar_t* message)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	printf("%s[WARN]%s %s%ls%s\n", Color::BrightYellow, Color::Reset, Color::Yellow, message, Color::Reset);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::logError(const wchar_t* message)
{
	ServerConsole* sc = getInstance();
	if (sc) sc->clearInputLine();

	printf("%s[ERROR]%s %s%ls%s\n", Color::BrightRed, Color::Reset, Color::Red, message, Color::Reset);
	fflush(stdout);

	if (sc) sc->redrawInputLine();
}

void ServerConsole::printPrompt()
{
	printf("%s> %s", Color::BrightGreen, Color::Reset);
	fflush(stdout);
}

void ServerConsole::clearInputLine()
{
	// Move to start of line, clear it
	printf("\r\033[K");
	fflush(stdout);
}

void ServerConsole::redrawInputLine()
{
	printf("\r\033[K");
	printPrompt();

	if (!m_inputBuffer.empty())
	{
		std::string highlighted = highlightCommand(m_inputBuffer);
		printf("%s", highlighted.c_str());
	}

	// Position cursor correctly
	int promptLen = 2; // "> "
	int targetCol = promptLen + m_cursorPos;
	printf("\r\033[%dC", targetCol);
	fflush(stdout);
}

std::string ServerConsole::highlightCommand(const std::string& input)
{
	if (input.empty()) return input;

	std::string result;
	std::istringstream stream(input);
	std::string token;
	bool firstToken = true;
	size_t pos = 0;

	// Find the first word (command name)
	size_t firstSpace = input.find(' ');
	std::string cmdName = (firstSpace != std::string::npos) ? input.substr(0, firstSpace) : input;
	std::string cmdLower = cmdName;
	std::transform(cmdLower.begin(), cmdLower.end(), cmdLower.begin(), ::tolower);

	// Check if it starts with /
	size_t cmdStart = 0;
	if (!cmdLower.empty() && cmdLower[0] == '/')
	{
		result += Color::Gray;
		result += '/';
		cmdLower = cmdLower.substr(1);
		cmdName = cmdName.substr(1);
		cmdStart = 1;
	}

	// Highlight command name
	bool knownCmd = false;
	for (int i = 0; s_commandNames[i] != nullptr; ++i)
	{
		if (cmdLower == s_commandNames[i])
		{
			knownCmd = true;
			break;
		}
	}

	if (knownCmd)
		result += std::string(Color::BrightGreen) + cmdName + Color::Reset;
	else
		result += std::string(Color::BrightRed) + cmdName + Color::Reset;

	// Highlight rest (arguments)
	if (firstSpace != std::string::npos)
	{
		std::string rest = input.substr(firstSpace);
		// Color numbers differently from strings
		std::string argResult;
		bool inNumber = false;
		for (size_t i = 0; i < rest.size(); ++i)
		{
			char c = rest[i];
			if (c == ' ')
			{
				if (inNumber) { argResult += Color::Reset; inNumber = false; }
				argResult += c;
			}
			else if ((c >= '0' && c <= '9') || c == '-' || c == '.')
			{
				if (!inNumber) { argResult += Color::BrightYellow; inNumber = true; }
				argResult += c;
			}
			else if (c == '~')
			{
				if (inNumber) { argResult += Color::Reset; inNumber = false; }
				argResult += Color::BrightMagenta;
				argResult += c;
				argResult += Color::Reset;
			}
			else
			{
				if (inNumber) { argResult += Color::Reset; inNumber = false; }
				argResult += Color::BrightWhite;
				argResult += c;
				argResult += Color::Reset;
			}
		}
		if (inNumber) argResult += Color::Reset;
		result += argResult;
	}

	return result;
}

static std::string toLowerStr(const std::string& s)
{
	std::string r = s;
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

std::vector<std::string> ServerConsole::getCompletions(const std::string& partial)
{
	std::vector<std::string> results;
	if (partial.empty()) return results;

	// Parse the input to figure out what we're completing
	std::string input = partial;
	bool hasSlash = false;
	if (!input.empty() && input[0] == '/')
	{
		hasSlash = true;
		input = input.substr(1);
	}

	// Split into tokens
	std::vector<std::string> tokens;
	{
		std::istringstream ss(input);
		std::string tok;
		while (ss >> tok) tokens.push_back(tok);
	}

	// If input ends with a space, we're completing the NEXT token
	bool completingNext = (!partial.empty() && partial.back() == ' ');
	if (completingNext) tokens.push_back("");

	if (tokens.empty())
	{
		// Complete command names
		for (int i = 0; s_commandNames[i] != nullptr; ++i)
		{
			std::string name = s_commandNames[i];
			results.push_back(hasSlash ? ("/" + name) : name);
		}
		return results;
	}

	if (tokens.size() == 1)
	{
		// Complete command name
		std::string prefix = toLowerStr(tokens[0]);
		for (int i = 0; s_commandNames[i] != nullptr; ++i)
		{
			std::string name = s_commandNames[i];
			if (name.find(prefix) == 0)
			{
				results.push_back(hasSlash ? ("/" + name) : name);
			}
		}
		return results;
	}

	// Token 1+ : subcommand or argument completions
	std::string cmdName = toLowerStr(tokens[0]);
	std::string argPrefix = toLowerStr(tokens.back());

	// Subcommand completions
	const char** subCmds = nullptr;
	if (tokens.size() == 2)
	{
		if (cmdName == "time") subCmds = s_timeSubcmds;
		else if (cmdName == "weather") subCmds = s_weatherTypes;
		else if (cmdName == "gamemode") subCmds = s_gamemodeNames;
		else if (cmdName == "effect") subCmds = s_effectActions;
	}
	else if (tokens.size() == 3 && cmdName == "time" && toLowerStr(tokens[1]) == "set")
	{
		subCmds = s_timeSetValues;
	}

	if (subCmds != nullptr)
	{
		for (int i = 0; subCmds[i] != nullptr; ++i)
		{
			std::string sub = subCmds[i];
			if (sub.find(argPrefix) == 0)
			{
				results.push_back(sub);
			}
		}
		// Don't return yet - also try player names
	}

	// Player name completions for commands that take player names
	bool wantsPlayerName = false;
	if (cmdName == "kill" || cmdName == "tp" || cmdName == "teleport" ||
		cmdName == "give" || cmdName == "giveitem" || cmdName == "enchant" || cmdName == "enchantitem" ||
		cmdName == "gamemode" || cmdName == "effect" || cmdName == "clear" ||
		cmdName == "vanish" || cmdName == "op" || cmdName == "deop" ||
		cmdName == "kick" || cmdName == "ban")
	{
		// Most commands take a player name as the first or second arg
		wantsPlayerName = true;
	}

	if (wantsPlayerName)
	{
		MinecraftServer* server = MinecraftServer::getInstance();
		if (server != nullptr)
		{
			PlayerList* playerList = server->getPlayers();
			if (playerList != nullptr)
			{
				for (size_t i = 0; i < playerList->players.size(); ++i)
				{
					if (playerList->players[i] != nullptr)
					{
						wstring wname = playerList->players[i]->getName();
						// Convert to narrow string
						std::string name(wname.begin(), wname.end());
						std::string nameLower = toLowerStr(name);
						if (nameLower.find(argPrefix) == 0)
						{
							results.push_back(name);
						}
					}
				}
				// Add @a, @p, @r, @s selectors
				const char* selectors[] = { "@a", "@p", "@r", "@s", nullptr };
				for (int i = 0; selectors[i] != nullptr; ++i)
				{
					std::string sel = selectors[i];
					if (sel.find(argPrefix) == 0)
						results.push_back(sel);
				}
			}
		}
	}

	// Entity name completions for summon (use public idsSpawnableInCreative)
	if (cmdName == "summon" && tokens.size() == 2)
	{
		for (auto& pair : EntityIO::idsSpawnableInCreative)
		{
			wstring wname = EntityIO::getEncodeId(pair.first);
			if (!wname.empty())
			{
				std::string name(wname.begin(), wname.end());
				std::string nameLower = toLowerStr(name);
				if (nameLower.find(argPrefix) == 0)
				{
					results.push_back(name);
				}
			}
		}
	}

	return results;
}

void ServerConsole::handleTabCompletion()
{
	if (m_tabIndex == -1)
	{
		// Start new tab completion
		m_tabPartial = m_inputBuffer;
		m_tabCompletions = getCompletions(m_tabPartial);
		if (m_tabCompletions.empty()) return;
		m_tabIndex = 0;
	}
	else
	{
		m_tabIndex = (m_tabIndex + 1) % (int)m_tabCompletions.size();
	}

	if (m_tabCompletions.empty()) return;

	// Figure out what portion to replace
	// Find the last space in the partial to know what word we're completing
	std::string completion = m_tabCompletions[m_tabIndex];

	size_t lastSpace = m_tabPartial.rfind(' ');
	if (lastSpace == std::string::npos)
	{
		// Replacing the whole input
		m_inputBuffer = completion;
	}
	else
	{
		// Replacing the last word
		m_inputBuffer = m_tabPartial.substr(0, lastSpace + 1) + completion;
	}

	m_cursorPos = (int)m_inputBuffer.size();

	// If there are multiple completions, show them
	if (m_tabCompletions.size() > 1)
	{
		clearInputLine();
		printf("\r\n");
		for (size_t i = 0; i < m_tabCompletions.size(); ++i)
		{
			if ((int)i == m_tabIndex)
				printf("%s%s%s  ", Color::BrightGreen, m_tabCompletions[i].c_str(), Color::Reset);
			else
				printf("%s  ", m_tabCompletions[i].c_str());
		}
		printf("\r\n");
		fflush(stdout);
	}

	redrawInputLine();
}

void ServerConsole::insertChar(char c)
{
	if ((int)m_inputBuffer.size() >= MAX_INPUT_LENGTH) return;

	m_inputBuffer.insert(m_inputBuffer.begin() + m_cursorPos, c);
	m_cursorPos++;
	m_tabIndex = -1; // Reset tab completion
	redrawInputLine();
}

void ServerConsole::deleteCharBack()
{
	if (m_cursorPos > 0)
	{
		m_inputBuffer.erase(m_cursorPos - 1, 1);
		m_cursorPos--;
		m_tabIndex = -1;
		redrawInputLine();
	}
}

void ServerConsole::deleteCharForward()
{
	if (m_cursorPos < (int)m_inputBuffer.size())
	{
		m_inputBuffer.erase(m_cursorPos, 1);
		m_tabIndex = -1;
		redrawInputLine();
	}
}

void ServerConsole::moveCursorLeft()
{
	if (m_cursorPos > 0)
	{
		m_cursorPos--;
		redrawInputLine();
	}
}

void ServerConsole::moveCursorRight()
{
	if (m_cursorPos < (int)m_inputBuffer.size())
	{
		m_cursorPos++;
		redrawInputLine();
	}
}

void ServerConsole::moveCursorHome()
{
	m_cursorPos = 0;
	redrawInputLine();
}

void ServerConsole::moveCursorEnd()
{
	m_cursorPos = (int)m_inputBuffer.size();
	redrawInputLine();
}

void ServerConsole::historyUp()
{
	if (m_history.empty()) return;

	if (m_historyIndex == -1)
	{
		m_savedInput = m_inputBuffer;
		m_historyIndex = 0;
	}
	else if (m_historyIndex < (int)m_history.size() - 1)
	{
		m_historyIndex++;
	}
	else
	{
		return; // at oldest entry
	}

	m_inputBuffer = m_history[m_historyIndex];
	m_cursorPos = (int)m_inputBuffer.size();
	m_tabIndex = -1;
	redrawInputLine();
}

void ServerConsole::historyDown()
{
	if (m_historyIndex == -1) return;

	m_historyIndex--;
	if (m_historyIndex < 0)
	{
		m_historyIndex = -1;
		m_inputBuffer = m_savedInput;
	}
	else
	{
		m_inputBuffer = m_history[m_historyIndex];
	}

	m_cursorPos = (int)m_inputBuffer.size();
	m_tabIndex = -1;
	redrawInputLine();
}

void ServerConsole::readInputLine(std::string& outLine)
{
	m_inputBuffer.clear();
	m_cursorPos = 0;
	m_historyIndex = -1;
	m_tabIndex = -1;
	m_savedInput.clear();

	printPrompt();

	while (m_running)
	{
		INPUT_RECORD ir;
		DWORD count = 0;

		if (!ReadConsoleInputA(m_hConsoleIn, &ir, 1, &count) || count == 0)
		{
			Sleep(10);
			continue;
		}

		if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown)
			continue;

		KEY_EVENT_RECORD& key = ir.Event.KeyEvent;
		WORD vk = key.wVirtualKeyCode;
		char ch = key.uChar.AsciiChar;

		if (vk == VK_RETURN)
		{
			printf("\r\n");
			fflush(stdout);
			outLine = m_inputBuffer;
			return;
		}
		else if (vk == VK_TAB)
		{
			handleTabCompletion();
		}
		else if (vk == VK_BACK)
		{
			deleteCharBack();
		}
		else if (vk == VK_DELETE)
		{
			deleteCharForward();
		}
		else if (vk == VK_LEFT)
		{
			moveCursorLeft();
		}
		else if (vk == VK_RIGHT)
		{
			moveCursorRight();
		}
		else if (vk == VK_UP)
		{
			historyUp();
		}
		else if (vk == VK_DOWN)
		{
			historyDown();
		}
		else if (vk == VK_HOME)
		{
			moveCursorHome();
		}
		else if (vk == VK_END)
		{
			moveCursorEnd();
		}
		else if (vk == VK_ESCAPE)
		{
			m_inputBuffer.clear();
			m_cursorPos = 0;
			m_tabIndex = -1;
			redrawInputLine();
		}
		else if (ch >= 32 && ch < 127)
		{
			insertChar(ch);
		}
	}

	outLine.clear();
}

void ServerConsole::run()
{
	enableVirtualTerminal();

	printf("\n%s========================================%s\n", Color::BrightCyan, Color::Reset);
	printf("%s  Minecraft Legacy Console Server%s\n", Color::BrightWhite, Color::Reset);
	printf("%s========================================%s\n", Color::BrightCyan, Color::Reset);
	printf("Type %shelp%s for available commands.\n", Color::BrightGreen, Color::Reset);
	printf("Use %sTab%s for auto-completion.\n\n", Color::BrightYellow, Color::Reset);

	while (m_running && !MinecraftServer::serverHalted())
	{
		std::string line;
		readInputLine(line);

		if (!m_running || MinecraftServer::serverHalted()) break;

		// Trim
		size_t start = line.find_first_not_of(" \t");
		size_t end = line.find_last_not_of(" \t");
		if (start == std::string::npos) continue;
		line = line.substr(start, end - start + 1);

		if (line.empty()) continue;

		// Add to history (avoid duplicates at front)
		if (m_history.empty() || m_history.front() != line)
		{
			m_history.push_front(line);
			if ((int)m_history.size() > MAX_HISTORY)
				m_history.pop_back();
		}

		// Convert to wstring and send to server
		wstring command = convStringToWstring(line);

		if (m_server != nullptr)
		{
			m_server->handleConsoleInput(command, m_server);
		}
	}
}

#endif // _WINDOWS64
