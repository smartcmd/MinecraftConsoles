#pragma once

#include "SimpleContainer.h"

class AnimalChest : public SimpleContainer
{
public:
	AnimalChest(const wstring &name, int size);
	AnimalChest(int iTitle, const wstring &name, bool hasCustomName, int size); // 4J Added iTitle param
};