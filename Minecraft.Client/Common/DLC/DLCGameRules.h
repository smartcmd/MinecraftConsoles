#pragma once

#include "DLCFile.h"
#include "../GameRules/LevelGenerationOptions.h"

class DLCGameRules : public DLCFile
{
public:
	DLCGameRules(DLCManager::EDLCType type, const wstring &path) : DLCFile(type,path) {}
};