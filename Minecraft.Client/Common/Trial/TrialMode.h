#pragma once
#include "..\Tutorial\FullTutorialMode.h"

class TrialMode : public FullTutorialMode
{
public:	
	TrialMode(int iPad, Minecraft *minecraft, ClientConnection *connection);

	virtual bool isImplemented() { return true; }
};