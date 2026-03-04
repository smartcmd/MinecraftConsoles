#pragma once
#include "Minecraft.Client/Common/Tutorial/TutorialMode.h"

class XboxGameMode : public TutorialMode
{
public:	
	XboxGameMode(int iPad, Minecraft *minecraft, ClientConnection *connection);

	virtual bool isImplemented() { return true; }
};