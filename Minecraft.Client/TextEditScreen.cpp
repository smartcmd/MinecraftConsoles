#include "stdafx.h"
#include "TextEditScreen.h"	
#include "Button.h"
#include "TileEntityRenderDispatcher.h"
#include "ClientConnection.h"
#include "MultiPlayerLevel.h"
#include "../Minecraft.World/SignTileEntity.h"
#include "../Minecraft.World/SharedConstants.h"
#include "../Minecraft.World/net.minecraft.network.packet.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"


const wstring TextEditScreen::allowedChars = SharedConstants::acceptableLetters;;

TextEditScreen::TextEditScreen(shared_ptr<SignTileEntity> sign)
{
	// 4J - added initialisers
	line = 0;
	frame = 0;
	title = L"Edit sign message:";

	this->sign = sign;
}

void TextEditScreen::init()
{
    buttons.clear();
    Keyboard::enableRepeatEvents(true);
    buttons.push_back(new Button(0, width / 2 - 100, height / 4 + 24 * 5, L"Done"));
}

void TextEditScreen::removed()
{
    Keyboard::enableRepeatEvents(false);
    if (minecraft->level->isClientSide)
	{
        minecraft->getConnection(0)->send( shared_ptr<SignUpdatePacket>( new SignUpdatePacket(sign->x, sign->y, sign->z, sign->IsVerified(), sign->IsCensored(), sign->GetMessages()) ) );
    }

}

void TextEditScreen::tick()
{
	frame++;
}

void TextEditScreen::buttonClicked(Button *button)
{
    if (!button->active) return;

    if (button->id == 0)
	{
        sign->setChanged();
        minecraft->setScreen(NULL);
    }
}

void TextEditScreen::keyPressed(wchar_t ch, int eventKey)
{
    if (eventKey == Keyboard::KEY_UP) line = (line - 1) & 3;
    if (eventKey == Keyboard::KEY_DOWN || eventKey == Keyboard::KEY_RETURN) line = (line + 1) & 3;

	wstring temp=sign->GetMessage(line);
    if (eventKey == Keyboard::KEY_BACK && temp.length() > 0)
	{
        temp = temp.substr(0, temp.length() - 1);
    }
    if (allowedChars.find(ch) != wstring::npos && temp.length() < 15)
	{
        temp += ch;
    }

	sign->SetMessage(line,temp);

}

void TextEditScreen::render(int xm, int ym, float a)
{
    renderBackground();

    drawCenteredString(font, title, width / 2, 40, 0xffffff);

    glPushMatrix();
    glTranslatef((float)width / 2, (float)height / 2, 50);
    float ss = 60 / (16 / 25.0f);
    glScalef(-ss, -ss, -ss);
    glRotatef(180, 0, 1, 0);

    Tile *tile = sign->getTile();

    if (tile == Tile::sign)
	{
        float rot = sign->getData() * 360 / 16.0f;
        glRotatef(rot, 0, 1, 0);
        glTranslatef(0, 5 / 16.0f, 0);
    }
	else
	{
        int face = sign->getData();
        float rot = 0;

        if (face == 2) rot = 180;
        if (face == 4) rot = 90;
        if (face == 5) rot = -90;
        glRotatef(rot, 0, 1, 0);
        glTranslatef(0, 5 / 16.0f, 0);
    }

    if (frame / 6 % 2 == 0) sign->SetSelectedLine(line);

    TileEntityRenderDispatcher::instance->render(sign, 0 - 0.5f, -0.75f, 0 - 0.5f, 0);
    sign->SetSelectedLine(-1);

    glPopMatrix();

    Screen::render(xm, ym, a);

}