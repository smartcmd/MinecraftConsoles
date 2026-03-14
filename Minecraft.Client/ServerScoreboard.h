#pragma once

#include "../Minecraft.World/Scoreboard.h"

class MinecraftServer;
class ScoreboardSaveData;
class Score;
class Objective;
class PlayerTeam;

class ServerScoreboard : public Scoreboard
{
private:
	MinecraftServer *server;
	unordered_set<Objective *> trackedObjectives;
	ScoreboardSaveData *saveData;

public:
	ServerScoreboard(MinecraftServer *server);

	MinecraftServer *getServer();
	void onScoreChanged(Score *score);
	void onPlayerRemoved(const wstring &player);
	void setDisplayObjective(int slot, Objective *objective);
	void addPlayerToTeam(const wstring &player, PlayerTeam *team);
	void removePlayerFromTeam(const wstring &player, PlayerTeam *team);
	void onObjectiveAdded(Objective *objective);
	void onObjectiveChanged(Objective *objective);
	void onObjectiveRemoved(Objective *objective);
	void onTeamAdded(PlayerTeam *team);
	void onTeamChanged(PlayerTeam *team);
	void onTeamRemoved(PlayerTeam *team);
	void setSaveData(ScoreboardSaveData *data);

protected:
	void setDirty();

public:
	vector<shared_ptr<Packet> > *getStartTrackingPackets(Objective *objective);
	void startTrackingObjective(Objective *objective);
	vector<shared_ptr<Packet> > *getStopTrackingPackets(Objective *objective);
	void stopTrackingObjective(Objective *objective);
	int getObjectiveDisplaySlotCount(Objective *objective);
};