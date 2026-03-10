#include "stdafx.h"
#include "ServerCommands.h"
#include "MinecraftServer.h"
#include "PlayerList.h"
#include "ServerPlayer.h"
#include "ServerLevel.h"
#include "PlayerConnection.h"
#include "EntityTracker.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\EntityIO.h"
#include "..\Minecraft.World\Entity.h"
#include "..\Minecraft.World\Tile.h"
#include "..\Minecraft.World\Dimension.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\MobEffect.h"
#include "..\Minecraft.World\MobEffectInstance.h"
#include "..\Minecraft.World\LevelSettings.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.enchantment.h"
#include "..\Minecraft.World\net.minecraft.world.damagesource.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\SharedConstants.h"
#ifdef _WINDOWS64
#include "ServerConsole.h"
#endif
#include <sstream>
#include <algorithm>
#include <fstream>

unordered_map<wstring, ServerCommands::CommandHandler> ServerCommands::s_commands;
unordered_set<wstring> ServerCommands::s_opOnlyCommands;
OpList ServerCommands::s_opList;
BanList ServerCommands::s_banList;
VanishManager ServerCommands::s_vanishManager;
bool ServerCommands::s_initialized = false;

CommandSource::CommandSource(MinecraftServer* server, ConsoleInputSource* source, Type type, const wstring& name)
	: m_server(server), m_consoleSource(source), m_player(nullptr), m_type(type), m_name(name)
{
}

CommandSource::CommandSource(MinecraftServer* server, shared_ptr<ServerPlayer> player)
	: m_server(server), m_consoleSource(nullptr), m_player(player), m_type(PLAYER), m_name(player->getName())
{
}

void CommandSource::sendMessage(const wstring& message)
{
	if (m_type == CONSOLE)
	{
#ifdef _WINDOWS64
		if (ServerConsole::getInstance())
			ServerConsole::logInfo(message.c_str());
		else
#endif
		if (m_consoleSource)
			m_consoleSource->info(message);
	}
	else if (m_player != nullptr)
	{
		if (m_player->connection != nullptr)
		{
			m_player->connection->send(std::make_shared<ChatPacket>(message));
		}
	}
}

void CommandSource::sendSuccess(const wstring& message)
{
	if (m_type == CONSOLE)
	{
#ifdef _WINDOWS64
		if (ServerConsole::getInstance())
			ServerConsole::logSuccess("%ls", message.c_str());
		else
#endif
		if (m_consoleSource)
			m_consoleSource->info(message);
	}
	else if (m_player != nullptr)
	{
		if (m_player->connection != nullptr)
		{
			m_player->connection->send(std::make_shared<ChatPacket>(message));
		}
	}
}

void CommandSource::sendError(const wstring& message)
{
	if (m_type == CONSOLE)
	{
#ifdef _WINDOWS64
		if (ServerConsole::getInstance())
			ServerConsole::logError("%ls", message.c_str());
		else
#endif
		if (m_consoleSource)
			m_consoleSource->warn(message);
	}
	else if (m_player != nullptr)
	{
		if (m_player->connection != nullptr)
		{
			m_player->connection->send(std::make_shared<ChatPacket>(message));
		}
	}
}

bool CommandSource::isOp() const
{
	if (m_type == CONSOLE) return true;
	return ServerCommands::getOpList().isOp(m_name);
}

static std::string getExeDirFilePath(const char* filename)
{
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	char* p = strrchr(path, '\\');
	if (p) *(p + 1) = '\0';
	strncat_s(path, sizeof(path), filename, _TRUNCATE);
	return std::string(path);
}

void OpList::addOp(const wstring& name)
{
	m_ops.insert(toLower(name));
}

void OpList::removeOp(const wstring& name)
{
	m_ops.erase(toLower(name));
}

bool OpList::isOp(const wstring& name) const
{
	return m_ops.find(toLower(name)) != m_ops.end();
}

void OpList::save() const
{
	std::string path = getExeDirFilePath("ops.txt");
	std::wofstream file(path, std::ios::out | std::ios::trunc);
	if (!file.is_open()) return;
	for (const wstring& op : m_ops)
		file << op << L"\n";
}

void OpList::load()
{
	std::string path = getExeDirFilePath("ops.txt");
	std::wifstream file(path);
	if (!file.is_open()) return;
	wstring line;
	while (std::getline(file, line))
	{
		if (!line.empty())
			m_ops.insert(toLower(line));
	}
}


void BanList::ban(const wstring& name, const wstring& reason)
{
	m_bans[toLower(name)] = reason;
}

void BanList::unban(const wstring& name)
{
	m_bans.erase(toLower(name));
}

bool BanList::isBanned(const wstring& name) const
{
	return m_bans.find(toLower(name)) != m_bans.end();
}

wstring BanList::getBanReason(const wstring& name) const
{
	auto it = m_bans.find(toLower(name));
	if (it != m_bans.end())
		return it->second;
	return L"";
}

void BanList::save() const
{
	std::string path = getExeDirFilePath("bans.txt");
	std::wofstream file(path, std::ios::out | std::ios::trunc);
	if (!file.is_open()) return;
	for (const auto& entry : m_bans)
	{
		file << entry.first << L"|" << entry.second << L"\n";
	}
}

void BanList::load()
{
	std::string path = getExeDirFilePath("bans.txt");
	std::wifstream file(path);
	if (!file.is_open()) return;
	wstring line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;
		size_t sep = line.find(L'|');
		if (sep != wstring::npos)
			m_bans[toLower(line.substr(0, sep))] = line.substr(sep + 1);
		else
			m_bans[toLower(line)] = L"";
	}
}


void VanishManager::setVanished(const wstring& name, bool vanished)
{
	wstring lower = toLower(name);
	if (vanished)
		m_vanished.insert(lower);
	else
		m_vanished.erase(lower);
}

bool VanishManager::isVanished(const wstring& name) const
{
	return m_vanished.find(toLower(name)) != m_vanished.end();
}


vector<wstring> ServerCommands::splitCommand(const wstring& command)
{
	vector<wstring> tokens;
	std::wistringstream stream(command);
	wstring token;
	while (stream >> token)
		tokens.push_back(token);
	return tokens;
}

wstring ServerCommands::joinTokens(const vector<wstring>& tokens, size_t startIndex)
{
	wstring joined;
	for (size_t i = startIndex; i < tokens.size(); ++i)
	{
		if (!joined.empty()) joined += L" ";
		joined += tokens[i];
	}
	return joined;
}

bool ServerCommands::tryParseInt(const wstring& text, int& value)
{
	std::wistringstream stream(text);
	stream >> value;
	return !stream.fail() && stream.eof();
}

bool ServerCommands::tryParseDouble(const wstring& text, double& value)
{
	std::wistringstream stream(text);
	stream >> value;
	return !stream.fail() && stream.eof();
}

shared_ptr<ServerPlayer> ServerCommands::findPlayer(PlayerList* playerList, const wstring& name)
{
	if (playerList == nullptr) return nullptr;

	for (size_t i = 0; i < playerList->players.size(); ++i)
	{
		shared_ptr<ServerPlayer> player = playerList->players[i];
		if (player != nullptr && equalsIgnoreCase(player->getName(), name))
			return player;
	}
	return nullptr;
}

vector<shared_ptr<ServerPlayer>> ServerCommands::resolvePlayerSelector(MinecraftServer* server, const wstring& selector, CommandSource& source)
{
	vector<shared_ptr<ServerPlayer>> result;
	PlayerList* playerList = server->getPlayers();
	if (playerList == nullptr) return result;

	if (selector == L"@a")
	{
		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] != nullptr)
				result.push_back(playerList->players[i]);
		}
	}
	else if (selector == L"@p")
	{
		if (source.isPlayer() && source.getPlayer() != nullptr)
		{
			result.push_back(source.getPlayer());
		}
		else if (!playerList->players.empty() && playerList->players[0] != nullptr)
		{
			result.push_back(playerList->players[0]);
		}
	}
	else if (selector == L"@r")
	{
		if (!playerList->players.empty())
		{
			int idx = rand() % (int)playerList->players.size();
			if (playerList->players[idx] != nullptr)
				result.push_back(playerList->players[idx]);
		}
	}
	else if (selector == L"@s")
	{
		if (source.isPlayer() && source.getPlayer() != nullptr)
			result.push_back(source.getPlayer());
	}
	else
	{
		shared_ptr<ServerPlayer> player = findPlayer(playerList, selector);
		if (player != nullptr)
			result.push_back(player);
	}

	return result;
}

void ServerCommands::initialize()
{
	if (s_initialized) return;
	s_initialized = true;

	s_commands[L"help"]        = cmdHelp;
	s_commands[L"?"]           = cmdHelp;
	s_commands[L"stop"]        = cmdStop;
	s_commands[L"list"]        = cmdList;
	s_commands[L"say"]         = cmdSay;
	s_commands[L"save-all"]    = cmdSaveAll;
	s_commands[L"kill"]        = cmdKill;
	s_commands[L"gamemode"]    = cmdGamemode;
	s_commands[L"teleport"]    = cmdTeleport;
	s_commands[L"tp"]          = cmdTeleport;
	s_commands[L"give"]        = cmdGive;
	s_commands[L"giveitem"]    = cmdGive;
	s_commands[L"enchant"]     = cmdEnchant;
	s_commands[L"enchantitem"] = cmdEnchant;
	s_commands[L"time"]        = cmdTime;
	s_commands[L"weather"]     = cmdWeather;
	s_commands[L"summon"]      = cmdSummon;
	s_commands[L"setblock"]    = cmdSetblock;
	s_commands[L"fill"]        = cmdFill;
	s_commands[L"effect"]      = cmdEffect;
	s_commands[L"clear"]       = cmdClear;
	s_commands[L"vanish"]      = cmdVanish;
	s_commands[L"op"]          = cmdOp;
	s_commands[L"deop"]        = cmdDeop;
	s_commands[L"kick"]        = cmdKick;
	s_commands[L"ban"]         = cmdBan;
	s_commands[L"unban"]       = cmdUnban;
	s_commands[L"pardon"]      = cmdUnban;
	s_commands[L"banlist"]     = cmdBanList;

	s_opOnlyCommands = {
		L"stop", L"save-all", L"say", L"kill", L"gamemode",
		L"teleport", L"tp", L"give", L"giveitem",
		L"enchant", L"enchantitem", L"time", L"weather",
		L"summon", L"setblock", L"fill", L"effect",
		L"clear", L"vanish", L"op", L"deop",
		L"kick", L"ban", L"unban", L"pardon", L"banlist"
	};

	s_opList.load();
	s_banList.load();
}

bool ServerCommands::execute(MinecraftServer* server, const wstring& rawCommand, CommandSource& source)
{
	if (server == nullptr) return false;

	wstring command = trimString(rawCommand);
	if (command.empty()) return true;

	if (command[0] == L'/') command = trimString(command.substr(1));

	vector<wstring> tokens = splitCommand(command);
	if (tokens.empty()) return true;

	wstring action = toLower(tokens[0]);

	auto it = s_commands.find(action);
	if (it == s_commands.end())
	{
		source.sendError(L"Unknown command: " + command + L". Type 'help' for a list of commands.");
		return false;
	}

	if (source.isPlayer() && !source.isOp() && s_opOnlyCommands.count(action))
	{
		source.sendError(L"You do not have permission to use this command.");
		return false;
	}

	return it->second(server, tokens, source);
}


bool ServerCommands::cmdHelp(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (!source.isOp())
	{
		source.sendMessage(L" - Server Commands - ");
		source.sendMessage(L"help - Show this help message");
		source.sendMessage(L"list - List online players");
		return true;
	}

	int page = 1;
	if (tokens.size() >= 2)
	{
		if (tokens[1] == L"2")
			page = 2;
	}

	if (page == 1)
	{
		source.sendMessage(L" - Server Commands (Page 1/2) - ");
		source.sendMessage(L"help [1|2] - Show this help message");
		source.sendMessage(L"list - List online players");
		source.sendMessage(L"stop - Stop the server");
		source.sendMessage(L"save-all - Save the world");
		source.sendMessage(L"say <message> - Broadcast a message");
		source.sendMessage(L"kill <player|@a|@p|@r|@s> - Kill a player");
		source.sendMessage(L"gamemode <survival|creative|adventure|0|1|2> [player] - Set game mode");
		source.sendMessage(L"tp <player> <target> | tp <player> <x> <y> <z> - Teleport");
		source.sendMessage(L"give <player> <itemId> [amount] [data] - Give items");
		source.sendMessage(L"enchant <player> <enchantId> [level] - Enchant held item");
		source.sendMessage(L"time <set|add|query> <value> - Manage world time");
		source.sendMessage(L"weather <clear|rain|thunder> [duration] - Set weather");
		source.sendMessage(L"Type /help 2 for more commands.");
	}
	else
	{
		source.sendMessage(L" - Server Commands (Page 2/2) - ");
		source.sendMessage(L"summon <entityName> [x] [y] [z] - Summon an entity");
		source.sendMessage(L"setblock <x> <y> <z> <tileId> [data] - Set a block");
		source.sendMessage(L"fill <x1> <y1> <z1> <x2> <y2> <z2> <tileId> [data] [mode] - Fill blocks");
		source.sendMessage(L"effect <give|clear> <player> [effectId] [seconds] [amplifier] - Manage effects");
		source.sendMessage(L"clear <player> [itemId] [data] - Clear inventory");
		source.sendMessage(L"vanish <player> - Toggle vanish (fake disconnect)");
		source.sendMessage(L"op <player> - Grant operator status");
		source.sendMessage(L"deop <player> - Revoke operator status");
		source.sendMessage(L"kick <player> [reason] - Kick a player");
		source.sendMessage(L"ban <player> [reason] - Ban a player");
		source.sendMessage(L"unban <player> - Unban a player (alias: pardon)");
		source.sendMessage(L"banlist - List all banned players");
	}
	return true;
}

bool ServerCommands::cmdStop(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	source.sendMessage(L"Stopping server...");
	server->setSaveOnExit(true);
	app.m_bShutdown = true;
	MinecraftServer::HaltServer();
	return true;
}

bool ServerCommands::cmdList(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	PlayerList* playerList = server->getPlayers();
	int count = (playerList != nullptr) ? playerList->getPlayerCount() : 0;

	wstring names;
	if (playerList != nullptr)
	{
		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] != nullptr)
			{
				if (!names.empty()) names += L", ";
				wstring pname = playerList->players[i]->getName();
				if (s_vanishManager.isVanished(pname))
					names += pname + L" (vanished)";
				else
					names += pname;
			}
		}
	}
	if (names.empty()) names = L"(none)";

	source.sendMessage(L"Players (" + std::to_wstring(count) + L"): " + names);
	return true;
}


bool ServerCommands::cmdSay(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: say <message>");
		return false;
	}

	wstring message = L"[Server] " + joinTokens(tokens, 1);
	PlayerList* playerList = server->getPlayers();
	if (playerList != nullptr)
		playerList->broadcastAll(std::make_shared<ChatPacket>(message));
	source.sendMessage(message);
	return true;
}


bool ServerCommands::cmdSaveAll(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	// stub. non-functional for now.
	return true;
}


bool ServerCommands::cmdKill(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		if (source.isPlayer() && source.getPlayer() != nullptr)
		{
			source.getPlayer()->hurt(DamageSource::outOfWorld, 3.4e38f);
			source.sendSuccess(L"Killed " + source.getName());
			return true;
		}
		source.sendError(L"Usage: kill <player|@a|@p|@r|@s>");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"No player found: " + tokens[1]);
		return false;
	}

	for (auto& player : targets)
	{
		player->hurt(DamageSource::outOfWorld, 3.4e38f);
		source.sendSuccess(L"Killed " + player->getName());
	}
	return true;
}


bool ServerCommands::cmdGamemode(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: gamemode <survival|creative|adventure|0|1|2|s|c|a> [player]");
		return false;
	}

	wstring modeName = toLower(tokens[1]);
	GameType* gameType = nullptr;

	if (modeName == L"survival" || modeName == L"s" || modeName == L"0")
		gameType = GameType::SURVIVAL;
	else if (modeName == L"creative" || modeName == L"c" || modeName == L"1")
		gameType = GameType::CREATIVE;
	else if (modeName == L"adventure" || modeName == L"a" || modeName == L"2")
		gameType = GameType::ADVENTURE;
	else
	{
		source.sendError(L"Unknown game mode: " + tokens[1]);
		return false;
	}

	vector<shared_ptr<ServerPlayer>> targets;
	if (tokens.size() >= 3)
	{
		targets = resolvePlayerSelector(server, tokens[2], source);
	}
	else if (source.isPlayer())
	{
		targets.push_back(source.getPlayer());
	}
	else
	{
		source.sendError(L"Usage: gamemode <mode> <player>");
		return false;
	}

	if (targets.empty())
	{
		source.sendError(L"No player found.");
		return false;
	}

	for (auto& player : targets)
	{
		player->setGameMode(gameType);
		source.sendSuccess(L"Set " + player->getName() + L"'s game mode to " + gameType->getName());
	}
	return true;
}


bool ServerCommands::cmdTeleport(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	PlayerList* playerList = server->getPlayers();

	if (tokens.size() == 3)
	{
		auto subjects = resolvePlayerSelector(server, tokens[1], source);
		shared_ptr<ServerPlayer> destination = findPlayer(playerList, tokens[2]);

		if (subjects.empty())
		{
			source.sendError(L"Unknown player: " + tokens[1]);
			return false;
		}
		if (destination == nullptr)
		{
			source.sendError(L"Unknown player: " + tokens[2]);
			return false;
		}

		for (auto& subject : subjects)
		{
			if (subject->level->dimension->id != destination->level->dimension->id || !subject->isAlive())
			{
				source.sendError(L"Cannot teleport " + subject->getName() + L" (different dimension or dead).");
				continue;
			}
			subject->ride(nullptr);
			subject->connection->teleport(destination->x, destination->y, destination->z, destination->yRot, destination->xRot);
			source.sendSuccess(L"Teleported " + subject->getName() + L" to " + destination->getName());
		}
		return true;
	}

	if (tokens.size() == 5)
	{
		auto subjects = resolvePlayerSelector(server, tokens[1], source);
		if (subjects.empty())
		{
			source.sendError(L"Unknown player: " + tokens[1]);
			return false;
		}

		double x, y, z;
		if (!tryParseDouble(tokens[2], x) || !tryParseDouble(tokens[3], y) || !tryParseDouble(tokens[4], z))
		{
			source.sendError(L"Invalid coordinates.");
			return false;
		}

		for (auto& subject : subjects)
		{
			subject->ride(nullptr);
			subject->connection->teleport(x, y, z, subject->yRot, subject->xRot);
			source.sendSuccess(L"Teleported " + subject->getName() + L" to " +
				std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z));
		}
		return true;
	}

	if (tokens.size() == 4 && source.isPlayer())
	{
		double x, y, z;
		if (!tryParseDouble(tokens[1], x) || !tryParseDouble(tokens[2], y) || !tryParseDouble(tokens[3], z))
		{
			source.sendError(L"Invalid coordinates.");
			return false;
		}
		auto player = source.getPlayer();
		player->ride(nullptr);
		player->connection->teleport(x, y, z, player->yRot, player->xRot);
		source.sendSuccess(L"Teleported to " +
			std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z));
		return true;
	}

	source.sendError(L"Usage: tp <player> <target> | tp <player> <x> <y> <z>");
	return false;
}

bool ServerCommands::cmdGive(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 3)
	{
		source.sendError(L"Usage: give <player> <itemId> [amount] [data]");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"Unknown player: " + tokens[1]);
		return false;
	}

	int itemId = 0, amount = 1, data = 0;
	if (!tryParseInt(tokens[2], itemId))
	{
		source.sendError(L"Invalid item id: " + tokens[2]);
		return false;
	}
	if (tokens.size() >= 4 && !tryParseInt(tokens[3], amount))
	{
		source.sendError(L"Invalid amount: " + tokens[3]);
		return false;
	}
	if (tokens.size() >= 5 && !tryParseInt(tokens[4], data))
	{
		source.sendError(L"Invalid data value: " + tokens[4]);
		return false;
	}

	if (itemId <= 0 || Item::items[itemId] == nullptr)
	{
		source.sendError(L"Unknown item id: " + std::to_wstring(itemId));
		return false;
	}
	if (amount <= 0) amount = 1;
	if (amount > 64) amount = 64;

	for (auto& player : targets)
	{
		shared_ptr<ItemInstance> itemInstance(new ItemInstance(itemId, amount, data));
		shared_ptr<ItemEntity> drop = player->drop(itemInstance);
		if (drop != nullptr)
			drop->throwTime = 0;
		source.sendSuccess(L"Gave " + std::to_wstring(amount) + L" x [" + std::to_wstring(itemId) + L":" + std::to_wstring(data) + L"] to " + player->getName());
	}
	return true;
}

bool ServerCommands::cmdEnchant(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 3)
	{
		source.sendError(L"Usage: enchant <player> <enchantId> [level]");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"Unknown player: " + tokens[1]);
		return false;
	}

	int enchantmentId = 0, enchantmentLevel = 1;
	if (!tryParseInt(tokens[2], enchantmentId))
	{
		source.sendError(L"Invalid enchantment id: " + tokens[2]);
		return false;
	}
	if (tokens.size() >= 4 && !tryParseInt(tokens[3], enchantmentLevel))
	{
		source.sendError(L"Invalid enchantment level: " + tokens[3]);
		return false;
	}

	for (auto& player : targets)
	{
		shared_ptr<ItemInstance> selectedItem = player->getSelectedItem();
		if (selectedItem == nullptr)
		{
			source.sendError(player->getName() + L" is not holding an item.");
			continue;
		}

		Enchantment* enchantment = Enchantment::enchantments[enchantmentId];
		if (enchantment == nullptr)
		{
			source.sendError(L"Unknown enchantment id: " + std::to_wstring(enchantmentId));
			return false;
		}
		if (!enchantment->canEnchant(selectedItem))
		{
			source.sendError(L"That enchantment cannot be applied to the selected item.");
			continue;
		}

		if (enchantmentLevel < enchantment->getMinLevel()) enchantmentLevel = enchantment->getMinLevel();
		if (enchantmentLevel > enchantment->getMaxLevel()) enchantmentLevel = enchantment->getMaxLevel();

		if (selectedItem->hasTag())
		{
			ListTag<CompoundTag>* enchantmentTags = selectedItem->getEnchantmentTags();
			if (enchantmentTags != nullptr)
			{
				bool conflict = false;
				for (int i = 0; i < enchantmentTags->size(); i++)
				{
					int type = enchantmentTags->get(i)->getShort((wchar_t*)ItemInstance::TAG_ENCH_ID);
					if (Enchantment::enchantments[type] != nullptr && !Enchantment::enchantments[type]->isCompatibleWith(enchantment))
					{
						source.sendError(L"Enchantment conflicts with existing enchantment on " + player->getName() + L"'s item.");
						conflict = true;
						break;
					}
				}
				if (conflict) continue;
			}
		}

		selectedItem->enchant(enchantment, enchantmentLevel);
		source.sendSuccess(L"Enchanted " + player->getName() + L"'s held item with enchantment " +
			std::to_wstring(enchantmentId) + L" level " + std::to_wstring(enchantmentLevel));
	}
	return true;
}


bool ServerCommands::cmdTime(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: time <set|add|query> <value>");
		return false;
	}

	wstring subCmd = toLower(tokens[1]);

	if (subCmd == L"query")
	{
		if (server->levels[0] != nullptr)
		{
			int64_t dayTime = server->levels[0]->getDayTime();
			int64_t gameTime = server->levels[0]->getGameTime();
			source.sendMessage(L"Day time: " + std::to_wstring(dayTime) + L", Game time: " + std::to_wstring(gameTime));
		}
		return true;
	}

	if (subCmd == L"add")
	{
		if (tokens.size() < 3)
		{
			source.sendError(L"Usage: time add <ticks>");
			return false;
		}

		int delta = 0;
		if (!tryParseInt(tokens[2], delta))
		{
			source.sendError(L"Invalid tick value: " + tokens[2]);
			return false;
		}

		for (unsigned int i = 0; i < server->levels.length; ++i)
		{
			if (server->levels[i] != nullptr)
				server->levels[i]->setDayTime(server->levels[i]->getDayTime() + delta);
		}
		source.sendSuccess(L"Added " + std::to_wstring(delta) + L" ticks to the time.");
		return true;
	}
	wstring timeValue;
	if (subCmd == L"set")
	{
		if (tokens.size() < 3)
		{
			source.sendError(L"Usage: time set <day|night|noon|midnight|ticks>");
			return false;
		}
		timeValue = toLower(tokens[2]);
	}
	else
	{
		timeValue = subCmd; 
	}

	int targetTime = 0;
	if (timeValue == L"day")
		targetTime = 1000;
	else if (timeValue == L"noon")
		targetTime = 6000;
	else if (timeValue == L"night")
		targetTime = 13000;
	else if (timeValue == L"midnight")
		targetTime = 18000;
	else if (!tryParseInt(timeValue, targetTime))
	{
		source.sendError(L"Invalid time value: " + timeValue);
		return false;
	}

	for (unsigned int i = 0; i < server->levels.length; ++i)
	{
		if (server->levels[i] != nullptr)
			server->levels[i]->setDayTime(targetTime);
	}
	source.sendSuccess(L"Set the time to " + std::to_wstring(targetTime));
	return true;
}


bool ServerCommands::cmdWeather(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: weather <clear|rain|thunder> [duration_seconds]");
		return false;
	}

	int durationSeconds = 600;
	if (tokens.size() >= 3 && !tryParseInt(tokens[2], durationSeconds))
	{
		source.sendError(L"Invalid duration: " + tokens[2]);
		return false;
	}

	if (server->levels[0] == nullptr)
	{
		source.sendError(L"The overworld is not loaded.");
		return false;
	}

	LevelData* levelData = server->levels[0]->getLevelData();
	int duration = durationSeconds * SharedConstants::TICKS_PER_SECOND;
	levelData->setRainTime(duration);
	levelData->setThunderTime(duration);

	wstring weather = toLower(tokens[1]);
	if (weather == L"clear")
	{
		levelData->setRaining(false);
		levelData->setThundering(false);
	}
	else if (weather == L"rain")
	{
		levelData->setRaining(true);
		levelData->setThundering(false);
	}
	else if (weather == L"thunder")
	{
		levelData->setRaining(true);
		levelData->setThundering(true);
	}
	else
	{
		source.sendError(L"Unknown weather type: " + tokens[1] + L". Use clear, rain, or thunder.");
		return false;
	}

	source.sendSuccess(L"Set weather to " + weather + L" for " + std::to_wstring(durationSeconds) + L" seconds.");
	return true;
}

bool ServerCommands::cmdSummon(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: summon <entityName> [x] [y] [z]");
		return false;
	}

	wstring entityName = tokens[1];

	double x = 0, y = 64, z = 0;
	bool hasPos = false;

	if (tokens.size() >= 5)
	{
		if (!tryParseDouble(tokens[2], x) || !tryParseDouble(tokens[3], y) || !tryParseDouble(tokens[4], z))
		{
			source.sendError(L"Invalid coordinates.");
			return false;
		}
		hasPos = true;
	}
	else if (source.isPlayer() && source.getPlayer() != nullptr)
	{
		x = source.getPlayer()->x;
		y = source.getPlayer()->y;
		z = source.getPlayer()->z;
		hasPos = true;
	}

	if (!hasPos && server->levels[0] != nullptr)
	{
		LevelData* ld = server->levels[0]->getLevelData();
		if (ld != nullptr)
		{
			x = ld->getXSpawn();
			y = ld->getYSpawn();
			z = ld->getZSpawn();
		}
	}
	ServerLevel* level = server->levels[0]; 
	if (source.isPlayer() && source.getPlayer() != nullptr)
	{
		level = (ServerLevel*)source.getPlayer()->level;
	}

	if (level == nullptr)
	{
		source.sendError(L"No level available.");
		return false;
	}

	shared_ptr<Entity> entity = EntityIO::newEntity(entityName, level);
	if (entity == nullptr)
	{
		wstring lowerName = toLower(entityName);
		for (auto& pair : EntityIO::idsSpawnableInCreative)
		{
			wstring knownName = EntityIO::getEncodeId(pair.first);
			if (toLower(knownName) == lowerName)
			{
				entity = EntityIO::newEntity(knownName, level);
				entityName = knownName;
				break;
			}
		}
	}

	if (entity == nullptr)
	{
		source.sendError(L"Unknown entity type: " + tokens[1]);
		return false;
	}

	entity->moveTo(x, y, z, 0.0f, 0.0f);
	level->addEntity(entity);

	source.sendSuccess(L"Summoned " + entityName + L" at " +
		std::to_wstring((int)x) + L", " + std::to_wstring((int)y) + L", " + std::to_wstring((int)z));
	return true;
}


bool ServerCommands::cmdSetblock(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 5)
	{
		source.sendError(L"Usage: setblock <x> <y> <z> <tileId> [data]");
		return false;
	}

	int x, y, z, tileId, data = 0;
	if (!tryParseInt(tokens[1], x) || !tryParseInt(tokens[2], y) || !tryParseInt(tokens[3], z))
	{
		source.sendError(L"Invalid coordinates.");
		return false;
	}
	if (!tryParseInt(tokens[4], tileId))
	{
		source.sendError(L"Invalid tile id: " + tokens[4]);
		return false;
	}
	if (tokens.size() >= 6 && !tryParseInt(tokens[5], data))
	{
		source.sendError(L"Invalid data value: " + tokens[5]);
		return false;
	}

	if (tileId < 0 || tileId >= Tile::TILE_NUM_COUNT)
	{
		source.sendError(L"Tile id out of range (0-" + std::to_wstring(Tile::TILE_NUM_COUNT - 1) + L").");
		return false;
	}

	if (tileId != 0 && Tile::tiles[tileId] == nullptr)
	{
		source.sendError(L"Unknown tile id: " + std::to_wstring(tileId));
		return false;
	}

	ServerLevel* level = server->levels[0];
	if (source.isPlayer() && source.getPlayer() != nullptr)
		level = (ServerLevel*)source.getPlayer()->level;

	if (level == nullptr)
	{
		source.sendError(L"No level available.");
		return false;
	}

	level->setTileAndData(x, y, z, tileId, data, Tile::UPDATE_ALL);

	source.sendSuccess(L"Set block at " + std::to_wstring(x) + L", " + std::to_wstring(y) + L", " + std::to_wstring(z) +
		L" to " + std::to_wstring(tileId) + L":" + std::to_wstring(data));
	return true;
}



bool ServerCommands::cmdFill(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 8)
	{
		source.sendError(L"Usage: fill <x1> <y1> <z1> <x2> <y2> <z2> <tileId> [data] [destroy|hollow|keep|outline|replace]");
		return false;
	}

	int x1, y1, z1, x2, y2, z2, tileId, data = 0;
	if (!tryParseInt(tokens[1], x1) || !tryParseInt(tokens[2], y1) || !tryParseInt(tokens[3], z1) ||
		!tryParseInt(tokens[4], x2) || !tryParseInt(tokens[5], y2) || !tryParseInt(tokens[6], z2))
	{
		source.sendError(L"Invalid coordinates.");
		return false;
	}
	if (!tryParseInt(tokens[7], tileId))
	{
		source.sendError(L"Invalid tile id: " + tokens[7]);
		return false;
	}
	if (tokens.size() >= 9 && !tryParseInt(tokens[8], data))
	{
		source.sendError(L"Invalid data value: " + tokens[8]);
		return false;
	}

	if (tileId < 0 || tileId >= Tile::TILE_NUM_COUNT)
	{
		source.sendError(L"Tile id out of range.");
		return false;
	}
	if (tileId != 0 && Tile::tiles[tileId] == nullptr)
	{
		source.sendError(L"Unknown tile id: " + std::to_wstring(tileId));
		return false;
	}

	wstring mode = L"replace";
	if (tokens.size() >= 10) mode = toLower(tokens[9]);

	int minX = (x1 < x2) ? x1 : x2;
	int minY = (y1 < y2) ? y1 : y2;
	int minZ = (z1 < z2) ? z1 : z2;
	int maxX = (x1 > x2) ? x1 : x2;
	int maxY = (y1 > y2) ? y1 : y2;
	int maxZ = (z1 > z2) ? z1 : z2;

	int64_t volume = (int64_t)(maxX - minX + 1) * (maxY - minY + 1) * (maxZ - minZ + 1);
	if (volume > 32768)
	{
		source.sendError(L"Too many blocks in the fill region (" + std::to_wstring(volume) + L"). Maximum is 32768.");
		return false;
	}

	ServerLevel* level = server->levels[0];
	if (source.isPlayer() && source.getPlayer() != nullptr)
		level = (ServerLevel*)source.getPlayer()->level;

	if (level == nullptr)
	{
		source.sendError(L"No level available.");
		return false;
	}

	int blocksChanged = 0;

	for (int bx = minX; bx <= maxX; ++bx)
	{
		for (int by = minY; by <= maxY; ++by)
		{
			for (int bz = minZ; bz <= maxZ; ++bz)
			{
				bool shouldPlace = true;

				if (mode == L"keep")
				{
					if (level->getTile(bx, by, bz) != 0) shouldPlace = false;
				}
				else if (mode == L"hollow")
				{
					bool isEdge = (bx == minX || bx == maxX || by == minY || by == maxY || bz == minZ || bz == maxZ);
					if (isEdge)
						shouldPlace = true;
					else
					{
						level->setTileAndData(bx, by, bz, 0, 0, Tile::UPDATE_ALL);
						blocksChanged++;
						shouldPlace = false;
					}
				}
				else if (mode == L"outline")
				{
					bool isEdge = (bx == minX || bx == maxX || by == minY || by == maxY || bz == minZ || bz == maxZ);
					if (!isEdge) shouldPlace = false;
				}

				if (shouldPlace)
				{
					level->setTileAndData(bx, by, bz, tileId, data, Tile::UPDATE_ALL);
					blocksChanged++;
				}
			}
		}
	}

	source.sendSuccess(L"Filled " + std::to_wstring(blocksChanged) + L" blocks.");
	return true;
}


bool ServerCommands::cmdEffect(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 3)
	{
		source.sendError(L"Usage: effect <give|clear> <player> [effectId] [seconds] [amplifier] | effect clear <player>");
		return false;
	}

	wstring subCmd = toLower(tokens[1]);

	if (subCmd == L"clear")
	{
		auto targets = resolvePlayerSelector(server, tokens[2], source);
		if (targets.empty())
		{
			source.sendError(L"Unknown player: " + tokens[2]);
			return false;
		}

		for (auto& player : targets)
		{
			player->removeAllEffects();
			source.sendSuccess(L"Cleared all effects from " + player->getName());
		}
		return true;
	}

	if (subCmd == L"give")
	{
		if (tokens.size() < 4)
		{
			source.sendError(L"Usage: effect give <player> <effectId> [seconds] [amplifier]");
			return false;
		}

		auto targets = resolvePlayerSelector(server, tokens[2], source);
		if (targets.empty())
		{
			source.sendError(L"Unknown player: " + tokens[2]);
			return false;
		}

		int effectId = 0, seconds = 30, amplifier = 0;
		if (!tryParseInt(tokens[3], effectId))
		{
			source.sendError(L"Invalid effect id: " + tokens[3]);
			return false;
		}
		if (tokens.size() >= 5 && !tryParseInt(tokens[4], seconds))
		{
			source.sendError(L"Invalid duration: " + tokens[4]);
			return false;
		}
		if (tokens.size() >= 6 && !tryParseInt(tokens[5], amplifier))
		{
			source.sendError(L"Invalid amplifier: " + tokens[5]);
			return false;
		}

		if (effectId < 0 || effectId >= MobEffect::NUM_EFFECTS || MobEffect::effects[effectId] == nullptr)
		{
			source.sendError(L"Unknown effect id: " + std::to_wstring(effectId) + L" (valid range: 1-" + std::to_wstring(MobEffect::NUM_EFFECTS - 1) + L")");
			return false;
		}

		if (seconds <= 0) seconds = 1;
		if (seconds > 1000000) seconds = 1000000;
		if (amplifier < 0) amplifier = 0;
		if (amplifier > 255) amplifier = 255;

		int durationTicks = seconds * SharedConstants::TICKS_PER_SECOND;

		for (auto& player : targets)
		{
			MobEffectInstance* effectInstance = new MobEffectInstance(effectId, durationTicks, amplifier);
			player->addEffect(effectInstance);
			source.sendSuccess(L"Applied effect " + std::to_wstring(effectId) +
				L" (amplifier " + std::to_wstring(amplifier) + L", " + std::to_wstring(seconds) + L"s) to " + player->getName());
		}
		return true;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (!targets.empty() && tokens.size() >= 3)
	{
		int effectId = 0, seconds = 30, amplifier = 0;
		if (tryParseInt(tokens[2], effectId))
		{
			if (tokens.size() >= 4) tryParseInt(tokens[3], seconds);
			if (tokens.size() >= 5) tryParseInt(tokens[4], amplifier);

			if (effectId < 0 || effectId >= MobEffect::NUM_EFFECTS || MobEffect::effects[effectId] == nullptr)
			{
				source.sendError(L"Unknown effect id: " + std::to_wstring(effectId));
				return false;
			}

			if (seconds <= 0) seconds = 1;
			if (amplifier < 0) amplifier = 0;
			if (amplifier > 255) amplifier = 255;

			int durationTicks = seconds * SharedConstants::TICKS_PER_SECOND;

			for (auto& player : targets)
			{
				MobEffectInstance* effectInstance = new MobEffectInstance(effectId, durationTicks, amplifier);
				player->addEffect(effectInstance);
				source.sendSuccess(L"Applied effect " + std::to_wstring(effectId) + L" to " + player->getName());
			}
			return true;
		}
	}

	source.sendError(L"Usage: effect <give|clear> <player> [effectId] [seconds] [amplifier]");
	return false;
}



bool ServerCommands::cmdClear(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		if (source.isPlayer())
		{
			auto player = source.getPlayer();
			player->inventory->clearInventory(-1, -1);
			source.sendSuccess(L"Cleared inventory of " + player->getName());
			return true;
		}
		source.sendError(L"Usage: clear <player> [itemId] [data]");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"Unknown player: " + tokens[1]);
		return false;
	}

	int filterItemId = -1, filterData = -1;
	if (tokens.size() >= 3 && !tryParseInt(tokens[2], filterItemId))
	{
		source.sendError(L"Invalid item id: " + tokens[2]);
		return false;
	}
	if (tokens.size() >= 4 && !tryParseInt(tokens[3], filterData))
	{
		source.sendError(L"Invalid data value: " + tokens[3]);
		return false;
	}

	for (auto& player : targets)
	{
		if (filterItemId < 0)
		{
			player->inventory->clearInventory(-1, -1);
			source.sendSuccess(L"Cleared inventory of " + player->getName());
		}
		else
		{
			int removed = 0;
			for (int slot = 0; slot < player->inventory->getContainerSize(); ++slot)
			{
				shared_ptr<ItemInstance> item = player->inventory->getItem(slot);
				if (item != nullptr && item->id == filterItemId)
				{
					if (filterData >= 0 && item->getAuxValue() != filterData) continue;
					removed += item->count;
					player->inventory->setItem(slot, nullptr);
				}
			}
			source.sendSuccess(L"Cleared " + std::to_wstring(removed) + L" items from " + player->getName());
		}
	}
	return true;
}


static void vanishPlayer(shared_ptr<ServerPlayer> player, PlayerList* playerList)
{
	if (playerList != nullptr)
	{
		auto fakeLeave = std::make_shared<ChatPacket>(player->getName(), ChatPacket::e_ChatPlayerLeftGame);
		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] != nullptr && playerList->players[i] != player)
				playerList->players[i]->connection->send(fakeLeave);
		}
	}

	ServerLevel* level = (ServerLevel*)player->level;
	if (level != nullptr)
	{
		EntityTracker* tracker = level->getTracker();
		if (tracker != nullptr)
			tracker->removeEntity(player);
	}
}

static void unvanishPlayer(shared_ptr<ServerPlayer> player, PlayerList* playerList)
{
	ServerLevel* level = (ServerLevel*)player->level;
	if (level != nullptr)
	{
		EntityTracker* tracker = level->getTracker();
		if (tracker != nullptr)
			tracker->addEntity(player);
	}

	if (playerList != nullptr)
	{
		auto fakeJoin = std::make_shared<ChatPacket>(player->getName(), ChatPacket::e_ChatPlayerJoinedGame);
		for (size_t i = 0; i < playerList->players.size(); ++i)
		{
			if (playerList->players[i] != nullptr && playerList->players[i] != player)
				playerList->players[i]->connection->send(fakeJoin);
		}
	}
}

bool ServerCommands::cmdVanish(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		if (source.isPlayer())
		{
			wstring name = source.getName();
			bool isVanished = s_vanishManager.isVanished(name);
			s_vanishManager.setVanished(name, !isVanished);

			PlayerList* playerList = server->getPlayers();
			if (!isVanished)
			{
				vanishPlayer(source.getPlayer(), playerList);
				source.sendSuccess(L"You are now vanished. Other players think you disconnected.");
			}
			else
			{
				unvanishPlayer(source.getPlayer(), playerList);
				source.sendSuccess(L"You are no longer vanished.");
			}
			return true;
		}
		source.sendError(L"Usage: vanish <player>");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"Unknown player: " + tokens[1]);
		return false;
	}

	PlayerList* playerList = server->getPlayers();

	for (auto& player : targets)
	{
		wstring name = player->getName();
		bool isVanished = s_vanishManager.isVanished(name);
		s_vanishManager.setVanished(name, !isVanished);

		if (!isVanished)
		{
			vanishPlayer(player, playerList);
			source.sendSuccess(name + L" is now vanished.");
		}
		else
		{
			unvanishPlayer(player, playerList);
			source.sendSuccess(name + L" is no longer vanished.");
		}
	}
	return true;
}


bool ServerCommands::cmdOp(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: op <player>");
		return false;
	}

	wstring playerName = tokens[1];
	shared_ptr<ServerPlayer> player = findPlayer(server->getPlayers(), playerName);
	if (player != nullptr)
		playerName = player->getName();

	if (s_opList.isOp(playerName))
	{
		source.sendMessage(playerName + L" is already an operator.");
		return true;
	}

	s_opList.addOp(playerName);
	s_opList.save();
	source.sendSuccess(L"Made " + playerName + L" a server operator.");

	if (player != nullptr && player->connection != nullptr)
	{
		player->connection->send(std::make_shared<ChatPacket>(L"You are now a server operator."));
	}

	return true;
}

bool ServerCommands::cmdDeop(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: deop <player>");
		return false;
	}

	wstring playerName = tokens[1];
	shared_ptr<ServerPlayer> player = findPlayer(server->getPlayers(), playerName);
	if (player != nullptr)
		playerName = player->getName();

	if (!s_opList.isOp(playerName))
	{
		source.sendMessage(playerName + L" is not an operator.");
		return true;
	}

	s_opList.removeOp(playerName);
	s_opList.save();
	source.sendSuccess(L"Removed " + playerName + L" from server operators.");

	if (player != nullptr && player->connection != nullptr)
	{
		player->connection->send(std::make_shared<ChatPacket>(L"You are no longer a server operator."));
	}

	return true;
}


bool ServerCommands::cmdKick(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: kick <player> [reason]");
		return false;
	}

	auto targets = resolvePlayerSelector(server, tokens[1], source);
	if (targets.empty())
	{
		source.sendError(L"Unknown player: " + tokens[1]);
		return false;
	}

	wstring reason = tokens.size() > 2 ? joinTokens(tokens, 2) : L"Kicked by an operator";

	for (auto& player : targets)
	{
		if (player->connection != nullptr)
		{
			player->connection->send(std::make_shared<ChatPacket>(L"Kicked: " + reason));
			player->connection->setWasKicked();
			player->connection->disconnect(DisconnectPacket::eDisconnect_Kicked);
		}
		source.sendSuccess(L"Kicked " + player->getName() + L": " + reason);
	}
	return true;
}

bool ServerCommands::cmdBan(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: ban <player> [reason]");
		return false;
	}

	wstring playerName = tokens[1];
	wstring reason = tokens.size() > 2 ? joinTokens(tokens, 2) : L"Banned by an operator";

	shared_ptr<ServerPlayer> player = findPlayer(server->getPlayers(), playerName);
	if (player != nullptr)
		playerName = player->getName();

	s_banList.ban(playerName, reason);
	s_banList.save();
	source.sendSuccess(L"Banned " + playerName + L": " + reason);

	if (player != nullptr && player->connection != nullptr)
	{
		player->connection->send(std::make_shared<ChatPacket>(L"Banned: " + reason));
		player->connection->setWasKicked();
		player->connection->disconnect(DisconnectPacket::eDisconnect_Banned);
	}

	return true;
}

bool ServerCommands::cmdUnban(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	if (tokens.size() < 2)
	{
		source.sendError(L"Usage: unban <player>");
		return false;
	}

	wstring playerName = tokens[1];
	if (!s_banList.isBanned(playerName))
	{
		source.sendError(playerName + L" is not banned.");
		return true;
	}

	s_banList.unban(playerName);
	s_banList.save();
	source.sendSuccess(L"Unbanned " + playerName + L".");
	return true;
}

bool ServerCommands::cmdBanList(MinecraftServer* server, const vector<wstring>& tokens, CommandSource& source)
{
	const auto& bans = s_banList.getBans();
	if (bans.empty())
	{
		source.sendMessage(L"No players are banned.");
		return true;
	}

	source.sendMessage(L"Banned players (" + std::to_wstring(bans.size()) + L"):");
	for (const auto& entry : bans)
	{
		if (entry.second.empty())
			source.sendMessage(L"  " + entry.first);
		else
			source.sendMessage(L"  " + entry.first + L" - " + entry.second);
	}
	return true;
}
