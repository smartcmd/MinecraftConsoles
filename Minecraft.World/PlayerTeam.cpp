#include "stdafx.h"
#include "net.minecraft.world.scores.h"
#include "PlayerTeam.h"

PlayerTeam::PlayerTeam(Scoreboard *scoreboard, const wstring &name)
{
	this->scoreboard = scoreboard;
	this->name = name;
	displayName = name;

	prefix = L"";
	suffix = L"";
	allowFriendlyFire = true;
	seeFriendlyInvisibles = true;
}

Scoreboard *PlayerTeam::getScoreboard()
{
	return scoreboard;
}

wstring PlayerTeam::getName()
{
	return name;
}

wstring PlayerTeam::getDisplayName()
{
	return displayName;
}

void PlayerTeam::setDisplayName(const wstring &displayName)
{
	//if (displayName == null) throw new IllegalArgumentException("Name cannot be null");
	this->displayName = displayName;
	scoreboard->onTeamChanged(this);
}

unordered_set<wstring> *PlayerTeam::getPlayers()
{
	return &players;
}

wstring PlayerTeam::getPrefix()
{
	return prefix;
}

void PlayerTeam::setPrefix(const wstring &prefix)
{
	//if (prefix == null) throw new IllegalArgumentException("Prefix cannot be null");
	this->prefix = prefix;
	scoreboard->onTeamChanged(this);
}

wstring PlayerTeam::getSuffix()
{
	return suffix;
}

void PlayerTeam::setSuffix(const wstring &suffix)
{
	//if (suffix == null) throw new IllegalArgumentException("Suffix cannot be null");
	this->suffix = suffix;
	scoreboard->onTeamChanged(this);
}

wstring PlayerTeam::getFormattedName(const wstring &teamMemberName)
{
	return getPrefix() + teamMemberName + getSuffix();
}

wstring PlayerTeam::formatNameForTeam(PlayerTeam *team)
{
	return formatNameForTeam(team, team->getDisplayName());
}

wstring PlayerTeam::formatNameForTeam(Team *team, const wstring &name)
{
	if (team == nullptr) return name;
	return team->getFormattedName(name);
}

bool PlayerTeam::isAllowFriendlyFire()
{
	return allowFriendlyFire;
}

void PlayerTeam::setAllowFriendlyFire(bool allowFriendlyFire)
{
	this->allowFriendlyFire = allowFriendlyFire;
	scoreboard->onTeamChanged(this);
}

bool PlayerTeam::canSeeFriendlyInvisibles()
{
	return seeFriendlyInvisibles;
}

void PlayerTeam::setSeeFriendlyInvisibles(bool seeFriendlyInvisibles)
{
	this->seeFriendlyInvisibles = seeFriendlyInvisibles;
	scoreboard->onTeamChanged(this);
}

int PlayerTeam::packOptions()
{
	int result = 0;

	if (isAllowFriendlyFire()) result |= 1 << BIT_FRIENDLY_FIRE;
	if (canSeeFriendlyInvisibles()) result |= 1 << BIT_SEE_INVISIBLES;

	return result;
}

void PlayerTeam::unpackOptions(int options)
{
	setAllowFriendlyFire((options & (1 << BIT_FRIENDLY_FIRE)) > 0);
	setSeeFriendlyInvisibles((options & (1 << BIT_SEE_INVISIBLES)) > 0);
}