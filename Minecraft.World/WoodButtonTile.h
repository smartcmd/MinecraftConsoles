#pragma once

#include "ButtonTile.h"

class WoodButtonTile : public ButtonTile
{
public:
	WoodButtonTile(int id);

	Icon *getTexture(int face, int data);
};