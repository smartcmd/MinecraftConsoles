#include "stdafx.h"

#include "Scoreboard.h"

Objective *Scoreboard::getObjective(const wstring &name)
{
	return NULL;
	//return objectivesByName.find(name)->second;
}

Objective *Scoreboard::addObjective(const wstring &name, ObjectiveCriteria *criteria)
{
	return NULL;
//	Objective *objective = getObjective(name);
//	if (objective != NULL)
//	{
//#indef _CONTENT_PACKAGE
//		__debugbreak();
//#endif
//		//throw new IllegalArgumentException("An objective with the name '" + name + "' already exists!");
//	}
//
//	objective = new Objective(this, name, criteria);
//
//	vector<Objective *> *criteriaList = objectivesByCriteria.find(criteria)->second;
//
//	if (criteriaList == NULL)
//	{
//		criteriaList = new vector<Objective *>();
//		objectivesByCriteria[criteria] = criteriaList;
//	}
//
//	criteriaList->push_back(objective);
//	objectivesByName[name] = objective;
//	onObjectiveAdded(objective);
//
//	return objective;
}

vector<Objective *> *Scoreboard::findObjectiveFor(ObjectiveCriteria *criteria)
{
	return NULL;
	//vector<Objective *> *objectives = objectivesByCriteria.find(criteria)->second;

	//return objectives == NULL ? new vector<Objective *>() : new vector<Objective *>(objectives);
}

Score *Scoreboard::getPlayerScore(const wstring &name, Objective *objective)
{
	return NULL;
	//unordered_map<Objective *, Score *> *scores = playerScores.find(name)->it;

	//if (scores == NULL)
	//{
	//	scores = new unordered_map<Objective *, Score *>();
	//	playerScores.put(name, scores);
	//}

	//Score *score = scores->get(objective);

	//if (score == NULL)
	//{
	//	score = new Score(this, objective, name);
	//	scores->put(objective, score);
	//}

	//return score;
}

vector<Score *> *Scoreboard::getPlayerScores(Objective *objective)
{
	return NULL;
	//vector<Score *> *result = new vector<Score *>();

	//for (Map<Objective, Score> scores : playerScores.values())
	//{
	//	Score score = scores.get(objective);
	//	if (score != null) result.add(score);
	//}

	//Collections.sort(result, Score.SCORE_COMPARATOR);

	//return result;
}

vector<Objective *> *Scoreboard::getObjectives()
{
	return NULL;
	//return objectivesByName.values();
}

vector<wstring> *Scoreboard::getTrackedPlayers()
{
	return NULL;
	//return playerScores.keySet();
}

void Scoreboard::resetPlayerScore(const wstring &player)
{
	//unordered_map<Objective *, Score *> *removed = playerScores.remove(player);

	//if (removed != NULL)
	//{
	//	onPlayerRemoved(player);
	//}
}

vector<Score *> *Scoreboard::getScores()
{
	return NULL;
	//Collection<Map<Objective, Score>> values = playerScores.values();
	//List<Score> result = new ArrayList<Score>();

	//for (Map<Objective, Score> map : values)
	//{
	//	result.addAll(map.values());
	//}

	//return result;
}

vector<Score *> *Scoreboard::getScores(Objective *objective)
{
	return NULL;
	//Collection<Map<Objective, Score>> values = playerScores.values();
	//List<Score> result = new ArrayList<Score>();

	//for (Map<Objective, Score> map : values) {
	//	Score score = map.get(objective);
	//	if (score != null) result.add(score);
	//}

	//return result;
}

unordered_map<Objective *, Score *> *Scoreboard::getPlayerScores(const wstring &player)
{
	return NULL;
	//Map<Objective, Score> result = playerScores.get(player);
	//if (result == null) result = new HashMap<Objective, Score>();
	//return result;
}

void Scoreboard::removeObjective(Objective *objective)
{
	//objectivesByName.remove(objective.getName());

	//for (int i = 0; i < DISPLAY_SLOTS; i++) {
	//	if (getDisplayObjective(i) == objective) setDisplayObjective(i, null);
	//}

	//List<Objective> objectives = objectivesByCriteria.get(objective.getCriteria());
	//if (objectives != null) objectives.remove(objective);

	//for (Map<Objective, Score> objectiveScoreMap : playerScores.values()) {
	//	objectiveScoreMap.remove(objective);
	//}

	//onObjectiveRemoved(objective);
}

void Scoreboard::setDisplayObjective(int slot, Objective *objective)
{
	//displayObjectives[slot] = objective;
}

Objective *Scoreboard::getDisplayObjective(int slot)
{
	return NULL;
	//return displayObjectives[slot];
}

PlayerTeam *Scoreboard::getPlayerTeam(const wstring &name)
{
	return NULL;
	//return teamsByName.get(name);
}

PlayerTeam *Scoreboard::addPlayerTeam(const wstring &name)
{
	return NULL;
	//PlayerTeam team = getPlayerTeam(name);
	//if (team != null) throw new IllegalArgumentException("An objective with the name '" + name + "' already exists!");

	//team = new PlayerTeam(this, name);
	//teamsByName.put(name, team);
	//onTeamAdded(team);

	//return team;
}

void Scoreboard::removePlayerTeam(PlayerTeam *team)
{
	//teamsByName.remove(team.getName());

	//// [TODO]: Loop through scores, remove.

	//for (String player : team.getPlayers()) {
	//	teamsByPlayer.remove(player);
	//}

	//onTeamRemoved(team);
}

void Scoreboard::addPlayerToTeam(const wstring &player, PlayerTeam *team)
{
	//if (getPlayersTeam(player) != null) {
	//	removePlayerFromTeam(player);
	//}

	//teamsByPlayer.put(player, team);
	//team.getPlayers().add(player);
}

bool Scoreboard::removePlayerFromTeam(const wstring &player)
{
	return false;
	//PlayerTeam team = getPlayersTeam(player);

	//if (team != null) {
	//	removePlayerFromTeam(player, team);
	//	return true;
	//} else {
	//	return false;
	//}
}

void Scoreboard::removePlayerFromTeam(const wstring &player, PlayerTeam *team)
{
	//if (getPlayersTeam(player) != team) {
	//	throw new IllegalStateException("Player is either on another team or not on any team. Cannot remove from team '" + team.getName() + "'.");
	//}

	//teamsByPlayer.remove(player);
	//team.getPlayers().remove(player);
}

vector<wstring> *Scoreboard::getTeamNames()
{
	return NULL;
	//return teamsByName.keySet();
}

vector<PlayerTeam *> *Scoreboard::getPlayerTeams()
{
	return NULL;
	//return teamsByName.values();
}

shared_ptr<Player> Scoreboard::getPlayer(const wstring &name)
{
	return nullptr;
	//return MinecraftServer.getInstance().getPlayers().getPlayer(name);
}

PlayerTeam *Scoreboard::getPlayersTeam(const wstring &name)
{
	return NULL;
	//return teamsByPlayer.get(name);
}

void Scoreboard::onObjectiveAdded(Objective *objective)
{
}

void Scoreboard::onObjectiveChanged(Objective *objective)
{
}

void Scoreboard::onObjectiveRemoved(Objective *objective)
{
}

void Scoreboard::onScoreChanged(Score *score)
{
}

void Scoreboard::onPlayerRemoved(const wstring &player)
{
}

void Scoreboard::onTeamAdded(PlayerTeam *team)
{
}

void Scoreboard::onTeamChanged(PlayerTeam *team)
{
}

void Scoreboard::onTeamRemoved(PlayerTeam *team)
{
}

wstring Scoreboard::getDisplaySlotName(int slot)
{
	switch (slot)
	{
	case DISPLAY_SLOT_LIST:
		return L"list";
	case DISPLAY_SLOT_SIDEBAR:
		return L"sidebar";
	case DISPLAY_SLOT_BELOW_NAME:
		return L"belowName";
	default:
		return L"";
	}
}

int Scoreboard::getDisplaySlotByName(const wstring &name)
{
	return -1;
	//if (name.equalsIgnoreCase("list"))
	//{
	//	return DISPLAY_SLOT_LIST;
	//}
	//else if (name.equalsIgnoreCase("sidebar"))
	//{
	//	return DISPLAY_SLOT_SIDEBAR;
	//}
	//else if (name.equalsIgnoreCase("belowName"))
	//{
	//	return DISPLAY_SLOT_BELOW_NAME;
	//}
	//else
	//{
	//	return -1;
	//}
}