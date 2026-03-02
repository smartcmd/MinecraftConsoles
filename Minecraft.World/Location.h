#pragma once

#include "Position.h"

class Level;

class Location : public Position
{
public:
	virtual Level *getWorld() = 0;
};