#include "stdafx.h"
#include "GuiParticle.h"
#include "..\Minecraft.World\Random.h"

Random *GuiParticle::random = new Random();

GuiParticle::GuiParticle(double x, double y, double xa, double ya)
{
	// 4J - added initialisation block
    removed = false;
    life = 0;
    a = 1;
    oR = oG = oB = oA = 0;
		
	this->xo = this->x = x;
    this->yo = this->y = y;
    this->xa = xa;
    this->ya = ya;

    int col = Color::HSBtoRGB(random->nextFloat(), 0.5f, 1);
    r = ((col >> 16) & 0xff) / 255.0;
    g = ((col >> 8) & 0xff) / 255.0;
    b = ((col) & 0xff) / 255.0;

    friction = 1.0 / (random->nextDouble() * 0.05 + 1.01);

    lifeTime = static_cast<int>(10.0 / (random->nextDouble() * 2 + 0.1));
}

void GuiParticle::tick(GuiParticles *guiParticles)
{
    x += xa;
    y += ya;

    xa *= friction;
    ya *= friction;

    ya += 0.1;
    if (++life > lifeTime) remove();
    a = 2 - (life / static_cast<double>(lifeTime)) * 2;
    if (a > 1) a = 1;
    a = a * a;
    a *= 0.5;
}

void GuiParticle::preTick()
{
    oR = r;
    oG = g;
    oB = b;
    oA = a;

    xo = x;
    yo = y;
}

void GuiParticle::remove()
{
	removed = true;
}