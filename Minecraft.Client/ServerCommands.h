#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

class MinecraftServer;
class ServerPlayer;
class ConsoleInputSource;
class PlayerList;

// Command sender abstraction - can be server console or a player
class CommandSource
{
public:
	enum Type { CONSOLE, PLAYER };

	CommandSource(MinecraftServer* server, ConsoleInputSource* source, Type type, const wstring& name);
	CommandSource(MinecraftServer* server, shared_ptr<ServerPlayer> player);

	void sendMessage(const wstring& message);
	void sendSuccess(const wstring& message);
	void sendError(const wstring& message);

	bool isConsole() const { return m_type == CONSOLE; }
	bool isPlayer() const { return m_type == PLAYER; }
	bool isOp() const;

	MinecraftServer* getServer() const { return m_server; }
	shared_ptr<ServerPlayer> getPlayer() const { return m_player; }
	wstring getName() const { return m_name; }

private:
	MinecraftServer* m_server;
	ConsoleInputSource* m_consoleSource;
	shared_ptr<ServerPlayer> m_player;
	Type m_type;
	wstring m_name;
};

// Server-side op list management
class OpList
{
public:
	void addOp(const wstring& name);
	void removeOp(const wstring& name);
	bool isOp(const wstring& name) const;
	const unordered_set<wstring>& getOps() const { return m_ops; }

	void save() const;
	void load();

private:
	unordered_set<wstring> m_ops;
};

// Server-side ban list management (persistent)
class BanList
{
public:
	void ban(const wstring& name, const wstring& reason = L"");
	void unban(const wstring& name);
	bool isBanned(const wstring& name) const;
	wstring getBanReason(const wstring& name) const;
	const unordered_map<wstring, wstring>& getBans() const { return m_bans; }

	void save() const;
	void load();

private:
	unordered_map<wstring, wstring> m_bans; // name -> reason
};

// Vanish manager
class VanishManager
{
public:
	void setVanished(const wstring& name, bool vanished);
	bool isVanished(const wstring& name) const;
	const unordered_set<wstring>& getVanished() const { return m_vanished; }

private:
	unordered_set<wstring> m_vanished;
};

// Main command registry and execution engine
class ServerCommands
{
public:
	static void initialize();

	// Execute a command string (with or without leading /)
	static bool execute(MinecraftServer* server, const wstring& rawCommand, CommandSource& source);

	// Access op and vanish managers
	static OpList& getOpList() { return s_opList; }
	static VanishManager& getVanishManager() { return s_vanishManager; }
	static BanList& getBanList() { return s_banList; }

private:
	// Utility functions
	static vector<wstring> splitCommand(const wstring& command);
	static wstring joinTokens(const vector<wstring>& tokens, size_t startIndex);
	static bool tryParseInt(const wstring& text, int& value);
	static bool tryParseDouble(const wstring& text, double& value);
	static shared_ptr<ServerPlayer> findPlayer(PlayerList* playerList, const wstring& name);
	static vector<shared_ptr<ServerPlayer>> resolvePlayerSelector(MinecraftServer* server, const wstring& selector, CommandSource& source);

	// Command handlers
	static bool cmdHelp(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdStop(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdList(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdSay(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdSaveAll(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdKill(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdGamemode(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdTeleport(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdGive(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdEnchant(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdTime(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdWeather(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdSummon(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdSetblock(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdFill(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdEffect(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdClear(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdVanish(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdOp(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdDeop(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdKick(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdBan(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdUnban(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);
	static bool cmdBanList(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source);

	// Command handler map
	typedef bool (*CommandHandler)(MinecraftServer*, const vector<wstring>&, CommandSource&);
	static unordered_map<wstring, CommandHandler> s_commands;
	static unordered_set<wstring> s_opOnlyCommands;
	static OpList s_opList;
	static BanList s_banList;
	static VanishManager s_vanishManager;
	static bool s_initialized;
};
