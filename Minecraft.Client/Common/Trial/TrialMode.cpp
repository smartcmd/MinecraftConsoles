#include "stdafx.h"
#include "TrialMode.h"
#include "../Tutorial/FullTutorial.h"

TrialMode::TrialMode(int iPad, Minecraft *minecraft, ClientConnection *connection)
	: FullTutorialMode(iPad, minecraft, connection)
{
	tutorial = new FullTutorial(iPad, true);
}