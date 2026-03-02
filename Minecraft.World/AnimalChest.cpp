#include "stdafx.h"

#include "AnimalChest.h"

AnimalChest::AnimalChest(const wstring &name, int size) : SimpleContainer(IDS_CONTAINER_ANIMAL, name, false, size)
{
}

AnimalChest::AnimalChest(int iTitle, const wstring &name, bool hasCustomName, int size) : SimpleContainer(iTitle, name, hasCustomName, size)
{
}