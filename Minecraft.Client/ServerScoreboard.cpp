#include "stdafx.h"

#include "ServerScoreboard.h"

ServerScoreboard::ServerScoreboard(MinecraftServer *server)
{
	this->server = server;
}

MinecraftServer *ServerScoreboard::getServer()
{
	return server;
}

void ServerScoreboard::onScoreChanged(Score *score)
{
	//Scoreboard::onScoreChanged(score);

	//if (trackedObjectives.contains(score.getObjective()))
	//{
	//	server->getPlayers()->broadcastAll( shared_ptr<SetScorePacket>( new SetScorePacket(score, SetScorePacket::METHOD_CHANGE)));
	//}

	//setDirty();
}

void ServerScoreboard::onPlayerRemoved(const wstring &player)
{
	//Scoreboard::onPlayerRemoved(player);
	//server->getPlayers()->broadcastAll( shared_ptr<SetScorePacket>( new SetScorePacket(player)));
	//setDirty();
}

void ServerScoreboard::setDisplayObjective(int slot, Objective *objective)
{
	//Objective *old = getDisplayObjective(slot);

	//Scoreboard::setDisplayObjective(slot, objective);

	//if (old != objective && old != nullptr)
	//{
	//	if (getObjectiveDisplaySlotCount(old) > 0)
	//	{
	//		server->getPlayers()->broadcastAll( shared_ptr<SetDisplayObjectivePacket>( new SetDisplayObjectivePacket(slot, objective)));
	//	}
	//	else
	//	{
	//		stopTrackingObjective(old);
	//	}
	//}

	//if (objective != nullptr)
	//{
	//	if (trackedObjectives.contains(objective))
	//	{
	//		server->getPlayers()->broadcastAll( shared_ptr<SetDisplayObjectivePacket>( new SetDisplayObjectivePacket(slot, objective)));
	//	}
	//	else
	//	{
	//		startTrackingObjective(objective);
	//	}
	//}

	//setDirty();
}

void ServerScoreboard::addPlayerToTeam(const wstring &player, PlayerTeam *team)
{
	//Scoreboard::addPlayerToTeam(player, team);

	//server->getPlayers()->broadcastAll( shared_ptr<SetPlayerTeamPacket>( new SetPlayerTeamPacket(team, Arrays::asList(player), SetPlayerTeamPacket::METHOD_JOIN)));

	//setDirty();
}

void ServerScoreboard::removePlayerFromTeam(const wstring &player, PlayerTeam *team)
{
	//Scoreboard::removePlayerFromTeam(player, team);

	//server->getPlayers()->broadcastAll( shared_ptr<SetPlayerTeamPacket>( new SetPlayerTeamPacket(team, Arrays::asList(player), SetPlayerTeamPacket::METHOD_LEAVE)));

	//setDirty();
}

void ServerScoreboard::onObjectiveAdded(Objective *objective)
{
	//Scoreboard::onObjectiveAdded(objective);
	//setDirty();
}

void ServerScoreboard::onObjectiveChanged(Objective *objective)
{
	//Scoreaboard::onObjectiveChanged(objective);

	//if (trackedObjectives.contains(objective))
	//{
	//	server->getPlayers()->broadcastAll( shared_ptr<SetObjectivePacket>( new SetObjectivePacket(objective, SetObjectivePacket::METHOD_CHANGE)));
	//}

	//setDirty();
}

void ServerScoreboard::onObjectiveRemoved(Objective *objective)
{
	//Scoreboard::onObjectiveRemoved(objective);

	//if (trackedObjectives.contains(objective))
	//{
	//	stopTrackingObjective(objective);
	//}

	//setDirty();
}

void ServerScoreboard::onTeamAdded(PlayerTeam *team)
{
	//Scoreboard::onTeamAdded(team);

	//server->getPlayers()->broadcastAll( shared_ptr<SetPlayerTeamPacket>( new SetPlayerTeamPacket(team, SetPlayerTeamPacket::METHOD_ADD)) );

	//setDirty();
}

void ServerScoreboard::onTeamChanged(PlayerTeam *team)
{
	//Scoreboard::onTeamChanged(team);

	//server->getPlayers()->broadcastAll( shared_ptr<SetPlayerTeamPacket>( new SetPlayerTeamPacket(team, SetPlayerTeamPacket::METHOD_CHANGE)));

	//setDirty();
}

void ServerScoreboard::onTeamRemoved(PlayerTeam *team)
{
	//Scoreboard::onTeamRemoved(team);

	//server->getPlayers()->broadcastAll( shared_ptr<SetPlayerTeamPacket>( new SetPlayerTeamPacket(team, SetPlayerTeamPacket::METHOD_REMOVE)) );

	//setDirty();
}

void ServerScoreboard::setSaveData(ScoreboardSaveData *data)
{
	//saveData = data;
}

void ServerScoreboard::setDirty()
{
	//if (saveData != nullptr)
	//{
	//	saveData->setDirty();
	//}
}

vector<shared_ptr<Packet> > *ServerScoreboard::getStartTrackingPackets(Objective *objective)
{
	return nullptr;

	//vector<shared_ptr<Packet> > *packets = new vector<shared_ptr<Packet> >();
	//packets.push_back( shared_ptr<SetObjectivePacket>( new SetObjectivePacket(objective, SetObjectivePacket::METHOD_ADD)));

	//for (int slot = 0; slot < DISPLAY_SLOTS; slot++)
	//{
	//	if (getDisplayObjective(slot) == objective) packets.push_back( shared_ptr<SetDisplayObjectivePacket>( new SetDisplayObjectivePacket(slot, objective)));
	//}

	//for (Score score : getPlayerScores(objective))
	//{
	//	packets.push_back( shared_ptr<SetScorePacket>( new SetScorePacket(score, SetScorePacket::METHOD_CHANGE)));
	//}

	//return packets;
}

void ServerScoreboard::startTrackingObjective(Objective *objective)
{
	//vector<shared_ptr<Packet> > *packets = getStartTrackingPackets(objective);

	//for (ServerPlayer player : server.getPlayers().players)
	//{
	//	for (Packet packet : packets)
	//	{
	//		player.connection.send(packet);
	//	}
	//}

	//trackedObjectives.push_back(objective);
}

vector<shared_ptr<Packet> > *ServerScoreboard::getStopTrackingPackets(Objective *objective)
{
	return nullptr;

	//vector<shared_ptr<Packet> > *packets = new ArrayList<Packet>();
	//packets->push_back( shared_ptr<SetObjectivePacket( new SetObjectivePacket(objective, SetObjectivePacket.METHOD_REMOVE)));

	//for (int slot = 0; slot < DISPLAY_SLOTS; slot++)
	//{
	//	if (getDisplayObjective(slot) == objective) packets.add(new SetDisplayObjectivePacket(slot, objective));
	//}

	//return packets;
}

void ServerScoreboard::stopTrackingObjective(Objective *objective)
{
	//vector<shared_ptr<Packet> > *packets = getStopTrackingPackets(objective);

	//for (ServerPlayer player : server.getPlayers().players)
	//{
	//	for (Packet packet : packets)
	//	{
	//		player->connection->send(packet);
	//	}
	//}

	//trackedObjectives.remove(objective);
}

int ServerScoreboard::getObjectiveDisplaySlotCount(Objective *objective)
{
	return 0;
	//int count = 0;

	//for (int slot = 0; slot < DISPLAY_SLOTS; slot++)
	//{
	//	if (getDisplayObjective(slot) == objective) count++;
	//}

	//return count;
}