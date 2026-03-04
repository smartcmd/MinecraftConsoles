#include "stdafx.h"
#include "ChatScreen.h"
#include "MultiplayerLocalPlayer.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "..\Minecraft.World\StringHelpers.h"

const wstring ChatScreen::allowedChars = SharedConstants::acceptableLetters;

ChatScreen::ChatScreen()
{
	frame = 0;
}

void ChatScreen::init()
{
	Keyboard::enableRepeatEvents(true);
}

void ChatScreen::removed()
{
	Keyboard::enableRepeatEvents(false);
}

void ChatScreen::tick()
{
	frame++;
}

void ChatScreen::keyPressed(wchar_t ch, int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE)
	{
        minecraft->setScreen(NULL);
        return;
    }
    if (eventKey == Keyboard::KEY_RETURN)
	{
        wstring msg = trimString(message);
        if (msg.length() > 0)
		{
            wstring trim = trimString(message);
            if (!minecraft->handleClientSideCommand(trim))
			{
                minecraft->player->chat(trim);
            }
        }
        minecraft->setScreen(NULL);
        return;
    }
    if (eventKey == Keyboard::KEY_BACK && message.length() > 0) message = message.substr(0, message.length() - 1);
    if (allowedChars.find(ch) >= 0 && message.length() < SharedConstants::maxChatLength)
	{
        message += ch;
    }

}

void ChatScreen::render(int xm, int ym, float a)
{
    fill(2, height - 14, width - 2, height - 2, 0x80000000);
    drawString(font, L"> " + message + (frame / 6 % 2 == 0 ? L"_" : L""), 4, height - 12, 0xe0e0e0);

    Screen::render(xm, ym, a);
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == 0)
	{
        if (minecraft->gui->selectedName != L"")	// 4J - was NULL comparison
		{
			if (message.length() > 0 && message[message.length()-1]!=L' ')
			{
                message += L" ";
            }
            message += minecraft->gui->selectedName;
            unsigned int maxLength = SharedConstants::maxChatLength;
            if (message.length() > maxLength)
			{
                message = message.substr(0, maxLength);
            }
        }
		else
		{
            Screen::mouseClicked(x, y, buttonNum);
        }
    }

}