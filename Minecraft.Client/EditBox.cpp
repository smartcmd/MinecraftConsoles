#include "stdafx.h"
#include "EditBox.h"
#include "..\Minecraft.World\SharedConstants.h"

EditBox::EditBox(Screen *screen, Font *font, int x, int y, int width, int height, const wstring& value)
{
	// 4J - added initialisers
	maxLength = 0;
	frame = 0;

	this->screen = screen;
	this->font = font;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->setValue(value);
}

void EditBox::setValue(const wstring& value)
{
	this->value = value;
}

wstring EditBox::getValue()
{
	return value;
}

void EditBox::tick()
{
	frame++;
}

void EditBox::keyPressed(wchar_t ch, int eventKey)
{
    if (!active || !inFocus) {
        return;
    }


    if (ch == 9)
	{
        screen->tabPressed();
    }
/* 4J removed
    if (ch == 22)
	{
        String msg = Screen.getClipboard();
        if (msg == null) msg = "";
        int toAdd = 32 - value.length();
        if (toAdd > msg.length()) toAdd = msg.length();
        if (toAdd > 0) {
            value += msg.substring(0, toAdd);
        }
    }
	*/

    if (eventKey == Keyboard::KEY_BACK && value.length() > 0)
	{
        value = value.substr(0, value.length() - 1);
    }
    if (SharedConstants::acceptableLetters.find(ch) != wstring::npos && (value.length() < maxLength || maxLength == 0))
	{
        value += ch;
    }

}

void EditBox::mouseClicked(int mouseX, int mouseY, int buttonNum)
{
    bool newFocus = active && (mouseX >= x && mouseX < (x + width) && mouseY >= y && mouseY < (y + height));
    focus(newFocus);
}

void EditBox::focus(bool newFocus)
{
    if (newFocus && !inFocus)
	{
        // reset the underscore counter to give quicker selection feedback
        frame = 0;
    }
    inFocus = newFocus;
}

void EditBox::render()
{
    fill(x - 1, y - 1, x + width + 1, y + height + 1, 0xffa0a0a0);
    fill(x, y, x + width, y + height, 0xff000000);

    if (active)
	{
        bool renderUnderscore = inFocus && (frame / 6 % 2 == 0);
        drawString(font, value + (renderUnderscore ? L"_" : L""), x + 4, y + (height - 8) / 2, 0xe0e0e0);
    }
	else
	{
        drawString(font, value, x + 4, y + (height - 8) / 2, 0x707070);
    }
}

void EditBox::setMaxLength(int maxLength)
{
	this->maxLength = maxLength;
}

int EditBox::getMaxLength()
{
	return maxLength;
}