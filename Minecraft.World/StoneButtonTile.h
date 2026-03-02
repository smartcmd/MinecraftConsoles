#pragma once

#include "ButtonTile.h"

class StoneButtonTile : public ButtonTile
{
public:
	StoneButtonTile(int id);

	virtual Icon *getTexture(int face, int data);
};