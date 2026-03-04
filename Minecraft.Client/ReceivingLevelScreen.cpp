#include "stdafx.h"
#include "ReceivingLevelScreen.h"
#include "ClientConnection.h"
#include "..\Minecraft.World\net.minecraft.locale.h"

ReceivingLevelScreen::ReceivingLevelScreen(ClientConnection *connection)
{
	tickCount = 0;
	this->connection = connection;
}

void ReceivingLevelScreen::keyPressed(char eventCharacter, int eventKey)
{
}

void ReceivingLevelScreen::init()
{
	buttons.clear();
}

void ReceivingLevelScreen::tick()
{
    tickCount++;
    if (tickCount % 20 == 0)
	{
        connection->send( shared_ptr<KeepAlivePacket>( new KeepAlivePacket() ) );
    }
    if (connection != NULL)
	{
        connection->tick();
    }
}

void ReceivingLevelScreen::buttonClicked(Button *button)
{
}

void ReceivingLevelScreen::render(int xm, int ym, float a)
{
    renderDirtBackground(0);

    Language *language = Language::getInstance();

    drawCenteredString(font, language->getElement(L"multiplayer.downloadingTerrain"), width / 2, height / 2 - 50, 0xffffff);

    Screen::render(xm, ym, a);
}