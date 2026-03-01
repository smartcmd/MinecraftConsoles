#pragma once

class Objective;
class ObjectiveCriteria;
class PlayerTeam;
class Score;

class Scoreboard
{
public:
	static const int DISPLAY_SLOT_LIST = 0;
	static const int DISPLAY_SLOT_SIDEBAR = 1;
	static const int DISPLAY_SLOT_BELOW_NAME = 2;
	static const int DISPLAY_SLOTS = 3;

private:
	unordered_map<wstring, Objective *> objectivesByName;
	unordered_map<ObjectiveCriteria *, vector<Objective *> *> objectivesByCriteria;
	unordered_map<wstring, unordered_map<Objective *, Score *> > playerScores;
	Objective *displayObjectives[DISPLAY_SLOTS];
	unordered_map<wstring, PlayerTeam *> teamsByName;
	unordered_map<wstring, PlayerTeam *> teamsByPlayer;

public:
	Objective *getObjective(const wstring &name);
	Objective *addObjective(const wstring &name, ObjectiveCriteria *criteria);
	vector<Objective *> *findObjectiveFor(ObjectiveCriteria *criteria);
	Score *getPlayerScore(const wstring &name, Objective *objective);
	vector<Score *> *getPlayerScores(Objective *objective);
	vector<Objective *> *getObjectives();
	vector<wstring> *getTrackedPlayers();
	void resetPlayerScore(const wstring &player);
	vector<Score *> *getScores();
	vector<Score *> *getScores(Objective *objective);
	unordered_map<Objective *, Score *> *getPlayerScores(const wstring &player);
	void removeObjective(Objective *objective);
	void setDisplayObjective(int slot, Objective *objective);
	Objective *getDisplayObjective(int slot);
	PlayerTeam *getPlayerTeam(const wstring &name);
	PlayerTeam *addPlayerTeam(const wstring &name);
	void removePlayerTeam(PlayerTeam *team);
	void addPlayerToTeam(const wstring &player, PlayerTeam *team);
	bool removePlayerFromTeam(const wstring &player);
	void removePlayerFromTeam(const wstring &player, PlayerTeam *team);
	vector<wstring> *getTeamNames();
	vector<PlayerTeam *> *getPlayerTeams();
	shared_ptr<Player> getPlayer(const wstring &name);
	PlayerTeam *getPlayersTeam(const wstring &name);
	void onObjectiveAdded(Objective *objective);
	void onObjectiveChanged(Objective *objective);
	void onObjectiveRemoved(Objective *objective);
	void onScoreChanged(Score *score);
	void onPlayerRemoved(const wstring &player);
	void onTeamAdded(PlayerTeam *team);
	void onTeamChanged(PlayerTeam *team);
	void onTeamRemoved(PlayerTeam *team);
	static wstring getDisplaySlotName(int slot);
	static int getDisplaySlotByName(const wstring &name);
};