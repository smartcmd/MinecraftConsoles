#pragma once

#include "Team.h"

class Scoreboard;

class PlayerTeam : public Team
{
public:
	static const int MAX_NAME_LENGTH = 16;
	static const int MAX_DISPLAY_NAME_LENGTH = 32;
	static const int MAX_PREFIX_LENGTH = 16;
	static const int MAX_SUFFIX_LENGTH = 16;

private:
	static const int BIT_FRIENDLY_FIRE = 0;
	static const int BIT_SEE_INVISIBLES = 1;

	Scoreboard *scoreboard;
	wstring name;
	unordered_set<wstring> players;
	wstring displayName;
	wstring prefix;
	wstring suffix;
	bool allowFriendlyFire;
	bool seeFriendlyInvisibles;

public:
	PlayerTeam(Scoreboard *scoreboard, const wstring &name);

	Scoreboard *getScoreboard();
	wstring getName();
	wstring getDisplayName();
	void setDisplayName(const wstring &displayName);
	unordered_set<wstring> *getPlayers();
	wstring getPrefix();
	void setPrefix(const wstring &prefix);
	wstring getSuffix();
	void setSuffix(const wstring &suffix);
	wstring getFormattedName(const wstring &teamMemberName);
	static wstring formatNameForTeam(PlayerTeam *team);
	static wstring formatNameForTeam(Team *team, const wstring &name);
	bool isAllowFriendlyFire();
	void setAllowFriendlyFire(bool allowFriendlyFire);
	bool canSeeFriendlyInvisibles();
	void setSeeFriendlyInvisibles(bool seeFriendlyInvisibles);
	int packOptions();
	void unpackOptions(int options);
};