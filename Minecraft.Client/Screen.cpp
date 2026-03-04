#include "stdafx.h"
#include "Screen.h"
#include "Button.h"
#include "GuiParticles.h"
#include "Tesselator.h"
#include "Textures.h"
#include "..\Minecraft.World\SoundTypes.h"
#ifdef _WINDOWS64
#include "Windows64\KeyboardMouseInput.h"
#endif



Screen::Screen()	// 4J added
{
	minecraft = NULL;
	width = 0;
    height = 0;
	passEvents = false;
	font = NULL;
	particles = NULL;
	clickedButton = NULL;
}

void Screen::render(int xm, int ym, float a)
{
	AUTO_VAR(itEnd, buttons.end());
	for (AUTO_VAR(it, buttons.begin()); it != itEnd; it++)
	{
        Button *button = *it; //buttons[i];
        button->render(minecraft, xm, ym);
    }
}

void Screen::keyPressed(wchar_t eventCharacter, int eventKey)
{
	if (eventKey == Keyboard::KEY_ESCAPE)
	{
		minecraft->setScreen(NULL);
//    minecraft->grabMouse();	// 4J - removed
	}
}

wstring Screen::getClipboard()
{
	// 4J - removed
	return NULL;
}

void Screen::setClipboard(const wstring& str)
{
	// 4J - removed
}

void Screen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == 0)
	{
		AUTO_VAR(itEnd, buttons.end());
		for (AUTO_VAR(it, buttons.begin()); it != itEnd; it++)
		{
            Button *button = *it; //buttons[i];
            if (button->clicked(minecraft, x, y))
			{
                clickedButton = button;
                minecraft->soundEngine->playUI(eSoundType_RANDOM_CLICK, 1, 1);
                buttonClicked(button);
            }
        }
    }
}

void Screen::mouseReleased(int x, int y, int buttonNum)
{
    if (clickedButton!=NULL && buttonNum==0)
	{
        clickedButton->released(x, y);
        clickedButton = NULL;
    }
}

void Screen::buttonClicked(Button *button)
{
}

void Screen::init(Minecraft *minecraft, int width, int height)
{
    particles = new GuiParticles(minecraft);
    this->minecraft = minecraft;
    this->font = minecraft->font;
    this->width = width;
    this->height = height;
    buttons.clear();
    init();
}

void Screen::setSize(int width, int height)
{
    this->width = width;
    this->height = height;
}

void Screen::init()
{
}

void Screen::updateEvents()
{
#ifdef _WINDOWS64
	// Poll mouse button state and dispatch click/release events
	for (int btn = 0; btn < 3; btn++)
	{
		if (g_KBMInput.IsMouseButtonPressed(btn))
		{
			int xm = Mouse::getX() * width / minecraft->width;
			int ym = height - Mouse::getY() * height / minecraft->height - 1;
			mouseClicked(xm, ym, btn);
		}
		if (g_KBMInput.IsMouseButtonReleased(btn))
		{
			int xm = Mouse::getX() * width / minecraft->width;
			int ym = height - Mouse::getY() * height / minecraft->height - 1;
			mouseReleased(xm, ym, btn);
		}
	}

	// Poll keyboard events
	for (int vk = 0; vk < 256; vk++)
	{
		if (g_KBMInput.IsKeyPressed(vk))
		{
			// Map Windows virtual key to the Keyboard constants used by Screen::keyPressed
			int mappedKey = -1;
			wchar_t ch = 0;
			if (vk == VK_ESCAPE)  mappedKey = Keyboard::KEY_ESCAPE;
			else if (vk == VK_RETURN)  mappedKey = Keyboard::KEY_RETURN;
			else if (vk == VK_BACK)    mappedKey = Keyboard::KEY_BACK;
			else if (vk == VK_UP)      mappedKey = Keyboard::KEY_UP;
			else if (vk == VK_DOWN)    mappedKey = Keyboard::KEY_DOWN;
			else if (vk == VK_LEFT)    mappedKey = Keyboard::KEY_LEFT;
			else if (vk == VK_RIGHT)   mappedKey = Keyboard::KEY_RIGHT;
			else if (vk == VK_LSHIFT || vk == VK_RSHIFT) mappedKey = Keyboard::KEY_LSHIFT;
			else if (vk == VK_TAB)     mappedKey = Keyboard::KEY_TAB;
			else if (vk >= 'A' && vk <= 'Z')
			{
				ch = (wchar_t)(vk - 'A' + L'a');
				if (g_KBMInput.IsKeyDown(VK_LSHIFT) || g_KBMInput.IsKeyDown(VK_RSHIFT)) ch = (wchar_t)vk;
			}
			else if (vk >= '0' && vk <= '9') ch = (wchar_t)vk;
			else if (vk == VK_SPACE) ch = L' ';

			if (mappedKey != -1) keyPressed(ch, mappedKey);
			else if (ch != 0) keyPressed(ch, -1);
		}
	}
#else
	/* 4J - TODO
	while (Mouse.next()) {
		mouseEvent();
	}

	while (Keyboard.next()) {
		keyboardEvent();
	}
	*/
#endif
}

void Screen::mouseEvent()
{
#ifdef _WINDOWS64
	// Mouse event dispatching is handled directly in updateEvents() for Windows
#else
	/* 4J - TODO
    if (Mouse.getEventButtonState()) {
        int xm = Mouse.getEventX() * width / minecraft.width;
        int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
        mouseClicked(xm, ym, Mouse.getEventButton());
    } else {
        int xm = Mouse.getEventX() * width / minecraft.width;
        int ym = height - Mouse.getEventY() * height / minecraft.height - 1;
        mouseReleased(xm, ym, Mouse.getEventButton());
    }
	*/
#endif
}

void Screen::keyboardEvent()
{
	/* 4J - TODO
    if (Keyboard.getEventKeyState()) {
        if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
            minecraft.toggleFullScreen();
            return;
        }
        keyPressed(Keyboard.getEventCharacter(), Keyboard.getEventKey());
    }
	*/
}

void Screen::tick()
{
}

void Screen::removed()
{
}

void Screen::renderBackground()
{
	renderBackground(0);
}

void Screen::renderBackground(int vo)
{
	if (minecraft->level != NULL)
	{
		fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
	}
	else
	{
		renderDirtBackground(vo);
	}
}

void Screen::renderDirtBackground(int vo)
{
	// 4J Unused
#if 0
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    Tesselator *t = Tesselator::getInstance();
    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(L"/gui/background.png"));
    glColor4f(1, 1, 1, 1);
    float s = 32;
    t->begin();
    t->color(0x404040);
    t->vertexUV((float)(0), (float)( height), (float)( 0), (float)( 0), (float)( height / s + vo));
    t->vertexUV((float)(width), (float)( height), (float)( 0), (float)( width / s), (float)( height / s + vo));
    t->vertexUV((float)(width), (float)( 0), (float)( 0), (float)( width / s), (float)( 0 + vo));
    t->vertexUV((float)(0), (float)( 0), (float)( 0), (float)( 0), (float)( 0 + vo));
    t->end();
#endif
}

bool Screen::isPauseScreen()
{
	return true;
}

void Screen::confirmResult(bool result, int id)
{
}

void Screen::tabPressed()
{
}
