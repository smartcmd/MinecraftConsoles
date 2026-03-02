#include "stdafx.h"

#include "Team.h"

bool Team::isAlliedTo(Team *other)
{
	if (other == NULL)
	{
		return false;
	}
	if (this == other)
	{
		return true;
	}
	return false;
}
