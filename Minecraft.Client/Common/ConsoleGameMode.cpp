#include "stdafx.h"
#include "ConsoleGameMode.h"
#include "./Tutorial/Tutorial.h"

ConsoleGameMode::ConsoleGameMode(int iPad, Minecraft *minecraft, ClientConnection *connection)
	: TutorialMode(iPad, minecraft, connection)
{
	tutorial = new Tutorial(iPad);
}
