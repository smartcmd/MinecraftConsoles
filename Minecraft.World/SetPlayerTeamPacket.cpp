#include "stdafx.h"
#include "net.minecraft.world.scores.h"
#include "net.minecraft.world.entity.player.h"
#include "PacketListener.h"
#include "SetPlayerTeamPacket.h"

SetPlayerTeamPacket::SetPlayerTeamPacket()
{
	name = L"";
	displayName = L"";
	prefix = L"";
	suffix = L"";
	method = 0;
	options = 0;
}

SetPlayerTeamPacket::SetPlayerTeamPacket(PlayerTeam *team, int method)
{
	name = team->getName();
	this->method = method;

	if (method == METHOD_ADD || method == METHOD_CHANGE)
	{
		displayName = team->getDisplayName();
		prefix = team->getPrefix();
		suffix = team->getSuffix();
		options = team->packOptions();
	}
	if (method == METHOD_ADD)
	{
		unordered_set<wstring> *playerNames = team->getPlayers();
		players.insert(players.end(), playerNames->begin(), playerNames->end());
	}
}

SetPlayerTeamPacket::SetPlayerTeamPacket(PlayerTeam *team, vector<wstring> *playerNames, int method)
{
	if (method != METHOD_JOIN && method != METHOD_LEAVE)
	{
		app.DebugPrintf("Method must be join or leave for player constructor");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
	}
	if (playerNames == nullptr || playerNames->empty())
	{
		app.DebugPrintf("Players cannot be null/empty");
#ifndef _CONTENT_PACKAGE
		__debugbreak();
#endif
	}

	this->method = method;
	name = team->getName();
	this->players.insert(players.end(), playerNames->begin(), playerNames->end());
}

void SetPlayerTeamPacket::read(DataInputStream *dis)
{
	name = readUtf(dis, Objective::MAX_NAME_LENGTH);
	method = dis->readByte();

	if (method == METHOD_ADD || method == METHOD_CHANGE)
	{
		displayName = readUtf(dis, PlayerTeam::MAX_DISPLAY_NAME_LENGTH);
		prefix = readUtf(dis, PlayerTeam::MAX_PREFIX_LENGTH);
		suffix = readUtf(dis, PlayerTeam::MAX_SUFFIX_LENGTH);
		options = dis->readByte();
	}

	if (method == METHOD_ADD || method == METHOD_JOIN || method == METHOD_LEAVE)
	{
		int count = dis->readShort();

		for (int i = 0; i < count; i++)
		{
			players.push_back(readUtf(dis, Player::MAX_NAME_LENGTH));
		}
	}
}

void SetPlayerTeamPacket::write(DataOutputStream *dos)
{
	writeUtf(name, dos);
	dos->writeByte(method);

	if (method == METHOD_ADD || method == METHOD_CHANGE)
	{
		writeUtf(displayName, dos);
		writeUtf(prefix, dos);
		writeUtf(suffix, dos);
		dos->writeByte(options);
	}

	if (method == METHOD_ADD || method == METHOD_JOIN || method == METHOD_LEAVE)
	{
		dos->writeShort(players.size());

		for (auto& player : players)
		{
			writeUtf(player, dos);
		}
	}
}

void SetPlayerTeamPacket::handle(PacketListener *listener)
{
	listener->handleSetPlayerTeamPacket(shared_from_this());
}

int SetPlayerTeamPacket::getEstimatedSize()
{
	return 1 + 2 + name.length();
}