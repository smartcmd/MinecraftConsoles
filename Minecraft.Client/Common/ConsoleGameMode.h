#pragma once
#include "./Tutorial/TutorialMode.h"

class ConsoleGameMode : public TutorialMode
{
public:	
	ConsoleGameMode(int iPad, Minecraft *minecraft, ClientConnection *connection);

	virtual bool isImplemented() { return true; }
};
