#include "stdafx.h"
#include "..\..\Minecraft.h"
#include "FullTutorial.h"
#include "FullTutorialMode.h"

FullTutorialMode::FullTutorialMode(int iPad, Minecraft *minecraft, ClientConnection *connection)
	: TutorialMode(iPad, minecraft, connection)
{
	tutorial = new FullTutorial( iPad );
	minecraft->playerStartedTutorial( iPad );
}

bool FullTutorialMode::isTutorial()
{
	return !tutorial->m_fullTutorialComplete;
}