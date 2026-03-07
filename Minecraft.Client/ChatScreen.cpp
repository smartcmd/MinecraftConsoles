#include "stdafx.h"
#include "ChatScreen.h"
#include "ClientConnection.h"
#include "Font.h"
#include "MultiplayerLocalPlayer.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\ChatPacket.h"

const wstring ChatScreen::allowedChars = SharedConstants::acceptableLetters;
vector<wstring> ChatScreen::s_chatHistory;
int ChatScreen::s_historyIndex = -1;
wstring ChatScreen::s_historyDraft;

bool ChatScreen::isAllowedChatChar(wchar_t c)
{
	return c >= 0x20 && (c == L'\u00A7' || allowedChars.empty() || allowedChars.find(c) != wstring::npos);
}

ChatScreen::ChatScreen()
{
	frame = 0;
	cursorIndex = 0;
	s_historyIndex = -1;
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
	if (cursorIndex > static_cast<int>(message.length()))
		cursorIndex = static_cast<int>(message.length());
}

void ChatScreen::handlePasteRequest()
{
	wstring pasted = Screen::getClipboard();
	for (size_t i = 0; i < pasted.length() && static_cast<int>(message.length()) < SharedConstants::maxChatLength; i++)
	{
		if (isAllowedChatChar(pasted[i]))
		{
			message.insert(cursorIndex, 1, pasted[i]);
			cursorIndex++;
		}
	}
}

void ChatScreen::applyHistoryMessage()
{
	message = s_historyIndex >= 0 ? s_chatHistory[s_historyIndex] : s_historyDraft;
	cursorIndex = static_cast<int>(message.length());
}

void ChatScreen::handleHistoryUp()
{
	if (s_chatHistory.empty()) return;
	if (s_historyIndex == -1)
	{
		s_historyDraft = message;
		s_historyIndex = static_cast<int>(s_chatHistory.size()) - 1;
	}
	else if (s_historyIndex > 0)
		s_historyIndex--;
	applyHistoryMessage();
}

void ChatScreen::handleHistoryDown()
{
	if (s_chatHistory.empty()) return;
	if (s_historyIndex < static_cast<int>(s_chatHistory.size()) - 1)
		s_historyIndex++;
	else
		s_historyIndex = -1;
	applyHistoryMessage();
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
        wstring trim = trimString(message);
        if (trim.length() > 0)
		{
            if (!minecraft->handleClientSideCommand(trim))
			{
                MultiplayerLocalPlayer* mplp = dynamic_cast<MultiplayerLocalPlayer*>(minecraft->player.get());
                if (mplp && mplp->connection)
                    mplp->connection->send(shared_ptr<ChatPacket>(new ChatPacket(trim)));
            }
            if (s_chatHistory.empty() || s_chatHistory.back() != trim)
			{
                s_chatHistory.push_back(trim);
                if (s_chatHistory.size() > CHAT_HISTORY_MAX)
                    s_chatHistory.erase(s_chatHistory.begin());
            }
        }
        minecraft->setScreen(NULL);
        return;
    }
    if (eventKey == Keyboard::KEY_UP)   { handleHistoryUp();   return; }
    if (eventKey == Keyboard::KEY_DOWN) { handleHistoryDown(); return; }
    if (eventKey == Keyboard::KEY_LEFT)
	{
        if (cursorIndex > 0)
            cursorIndex--;
        return;
    }
    if (eventKey == Keyboard::KEY_RIGHT)
	{
        if (cursorIndex < static_cast<int>(message.length()))
            cursorIndex++;
        return;
    }
    if (eventKey == Keyboard::KEY_BACK && cursorIndex > 0)
	{
        message.erase(cursorIndex - 1, 1);
        cursorIndex--;
        return;
    }
    if (isAllowedChatChar(ch) && static_cast<int>(message.length()) < SharedConstants::maxChatLength)
	{
        message.insert(cursorIndex, 1, ch);
        cursorIndex++;
    }
}

void ChatScreen::render(int xm, int ym, float a)
{
    int iPad = minecraft->player->GetXboxPad();
    int uiSetting = app.GetGameSettings(iPad, eGameSetting_UISize);

    float textScale = 0.6f;
    int barTopOffset = 14;
    int barBottomOffset = 2;
    int textYOffset = 12;
    int yAdjust = 0;
    int widthsubtract = 2;

    if (uiSetting == 0) 
    {
        textScale = textScale * 1.25f;
        barTopOffset = 10;
        barBottomOffset = 0;
        textYOffset = 8;
        yAdjust = 115;
        widthsubtract = -210;
    }
    else if (uiSetting == 1)
    {
        textScale = textScale * 1.0f;
        barTopOffset = 14;
        barBottomOffset = 2;
        textYOffset = 12;
        yAdjust = 0;
        widthsubtract = 2;
    }
    else if (uiSetting == 2)
    {
        textScale = textScale * 0.75f;
        barTopOffset = 18;
        barBottomOffset = 6;
        textYOffset = 16;
        yAdjust = -58;
        widthsubtract = 110;
    }

    int barY = height - barTopOffset + yAdjust;
    int barBottom = height - barBottomOffset + yAdjust;
    int textY = height - textYOffset + yAdjust;

    fill(2, barY, width - widthsubtract, barBottom, 0x80000000);

    const wstring prefix = L"> ";
    int x = 4;

    drawString(font, prefix, x, textY, 0xe0e0e0);
    
    x += font->width(prefix);
    wstring beforeCursor = message.substr(0, cursorIndex);
    wstring afterCursor = message.substr(cursorIndex);

    glPushMatrix();
    glTranslatef((float)x, (float)textY, 0);
    glScalef(textScale, textScale, 1.0f);
    font->drawShadowLiteralCustom(beforeCursor, 0, 0, 1, 1, 0xe0e0e0, 0x000000);
    glPopMatrix();

    x += (int)(font->widthLiteral(beforeCursor) * textScale);

    if (frame / 6 % 2 == 0)
    {
        glPushMatrix();
        glTranslatef((float)x, (float)textY, 0);
        glScalef(textScale, textScale, 1.0f);
        font->drawShadowLiteralCustom(L"_", 0, 0, 1, 1, 0xe0e0e0, 0x000000);
        glPopMatrix();
    }

    x += (int)(font->width(L"_") * textScale);

    glPushMatrix();
    glTranslatef((float)x, (float)textY, 0);
    glScalef(textScale, textScale, 1.0f);
    font->drawShadowLiteralCustom(afterCursor, 0, 0, 1, 1, 0xe0e0e0, 0x000000);
    glPopMatrix();

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
                message = message.substr(0, cursorIndex) + L" " + message.substr(cursorIndex);
                cursorIndex++;
            }
            size_t nameLen = minecraft->gui->selectedName.length();
            size_t insertLen = (message.length() + nameLen <= SharedConstants::maxChatLength) ? nameLen : (SharedConstants::maxChatLength - message.length());
            if (insertLen > 0)
			{
                message = message.substr(0, cursorIndex) + minecraft->gui->selectedName.substr(0, insertLen) + message.substr(cursorIndex);
                cursorIndex += static_cast<int>(insertLen);
            }
        }
		else
		{
            Screen::mouseClicked(x, y, buttonNum);
        }
    }
}