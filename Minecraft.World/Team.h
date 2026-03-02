#pragma once

class Team
{
public:
	virtual bool isAlliedTo(Team *other);

	virtual wstring getName() = 0;
	virtual wstring getFormattedName(const wstring &teamMemberName) = 0;
	virtual bool canSeeFriendlyInvisibles() = 0;
	virtual bool isAllowFriendlyFire() = 0;

};