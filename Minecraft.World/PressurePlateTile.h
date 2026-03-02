#pragma once
#include "BasePressurePlateTile.h"

class PressurePlateTile : public BasePressurePlateTile
{
public:
	enum Sensitivity
	{
		everything, mobs, players
	};

private:
	Sensitivity sensitivity;

public:
	PressurePlateTile(int id, const wstring &tex, Material *material, Sensitivity sensitivity);

protected:
	virtual int getDataForSignal(int signal);
	virtual int getSignalForData(int data);
	virtual int getSignalStrength(Level *level, int x, int y, int z);
};