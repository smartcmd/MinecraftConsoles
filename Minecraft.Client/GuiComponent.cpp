#include "stdafx.h"
#include "GuiComponent.h"
#include "Tesselator.h"

void GuiComponent::hLine(int x0, int x1, int y, int col)
{
    if (x1 < x0)
	{
        int tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    fill(x0, y, x1 + 1, y + 1, col);
}

void GuiComponent::vLine(int x, int y0, int y1, int col)
{
    if (y1 < y0)
	{
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    fill(x, y0 + 1, x + 1, y1, col);
}

void GuiComponent::fill(int x0, int y0, int x1, int y1, int col)
{
    if (x0 < x1)
	{
        int tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y0 < y1)
	{
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    float a = ((col >> 24) & 0xff) / 255.0f;
    float r = ((col >> 16) & 0xff) / 255.0f;
    float g = ((col >> 8) & 0xff) / 255.0f;
    float b = ((col) & 0xff) / 255.0f;
    Tesselator *t = Tesselator::getInstance();
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    t->begin();
    t->vertex(static_cast<float>(x0), static_cast<float>(y1), static_cast<float>(0));
    t->vertex(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(0));
    t->vertex(static_cast<float>(x1), static_cast<float>(y0), static_cast<float>(0));
    t->vertex(static_cast<float>(x0), static_cast<float>(y0), static_cast<float>(0));
    t->end();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void GuiComponent::fillGradient(int x0, int y0, int x1, int y1, int col1, int col2)
{
    float a1 = ((col1 >> 24) & 0xff) / 255.0f;
    float r1 = ((col1 >> 16) & 0xff) / 255.0f;
    float g1 = ((col1 >> 8) & 0xff) / 255.0f;
    float b1 = ((col1) & 0xff) / 255.0f;

    float a2 = ((col2 >> 24) & 0xff) / 255.0f;
    float r2 = ((col2 >> 16) & 0xff) / 255.0f;
    float g2 = ((col2 >> 8) & 0xff) / 255.0f;
    float b2 = ((col2) & 0xff) / 255.0f;
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->color(r1, g1, b1, a1);
    t->vertex(static_cast<float>(x1), static_cast<float>(y0), blitOffset);
    t->vertex(static_cast<float>(x0), static_cast<float>(y0), blitOffset);
    t->color(r2, g2, b2, a2);
    t->vertex(static_cast<float>(x0), static_cast<float>(y1), blitOffset);
    t->vertex(static_cast<float>(x1), static_cast<float>(y1), blitOffset);
    t->end();

    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
}

GuiComponent::GuiComponent()
{
	blitOffset = 0;
}

void GuiComponent::drawCenteredString(Font *font, const wstring& str, int x, int y, int color)
{
	font->drawShadow(str, x - (font->width(str)) / 2, y, color);
}

void GuiComponent::drawString(Font *font, const wstring& str, int x, int y, int color)
{
	font->drawShadow(str, x, y, color);
}

void GuiComponent::drawStringLiteral(Font *font, const wstring& str, int x, int y, int color)
{
	font->drawShadowLiteral(str, x, y, color);
}

void GuiComponent::blit(int x, int y, int sx, int sy, int w, int h)
{
    float us = 1 / 256.0f;
    float vs = 1 / 256.0f;
    Tesselator *t = Tesselator::getInstance();
    t->begin();

	// This is a bit of a mystery. In general this ought to be 0.5 to match the centre of texels & pixels in the DX9 version of things. However, when scaling the GUI by a factor of 1.5, I'm
	// really not sure how exactly point sampled rasterisation works, but when shifting by 0.5 we get a discontinuity down the diagonal of quads. Setting this shift to 0.75 in all cases seems to work fine.
	const float extraShift = 0.75f;

	// 4J - subtracting extraShift (actual screen pixels, so need to compensate for physical & game width) from each x & y coordinate to compensate for centre of pixels in directx vs openGL
	float dx = ( extraShift * static_cast<float>(Minecraft::GetInstance()->width) ) / static_cast<float>(Minecraft::GetInstance()->width_phys);
	// 4J - Also factor in the scaling from gui coordinate space to the screen. This varies based on user-selected gui scale, and whether we are in a viewport mode or not
	dx /= Gui::currentGuiScaleFactor;
	float dy = extraShift / Gui::currentGuiScaleFactor;
	// Ensure that the x/y, width and height are actually pixel aligned at our current scale factor - in particular, for split screen mode with the default (3X)
	// scale, we have an overall scale factor of 3 * 0.5 = 1.5, and so any odd pixels won't align
	float fx = (floorf(static_cast<float>(x) * Gui::currentGuiScaleFactor)) / Gui::currentGuiScaleFactor;
	float fy = (floorf(static_cast<float>(y) * Gui::currentGuiScaleFactor)) / Gui::currentGuiScaleFactor;
	float fw = (floorf(static_cast<float>(w) * Gui::currentGuiScaleFactor)) / Gui::currentGuiScaleFactor;
	float fh = (floorf(static_cast<float>(h) * Gui::currentGuiScaleFactor)) / Gui::currentGuiScaleFactor;

    t->vertexUV(fx + 0 - dx,  fy + fh - dy, static_cast<float>(blitOffset), static_cast<float>((sx + 0) * us), static_cast<float>((sy + h) * vs));
    t->vertexUV(fx + fw - dx, fy + fh - dy, static_cast<float>(blitOffset), static_cast<float>((sx + w) * us), static_cast<float>((sy + h) * vs));
    t->vertexUV(fx + fw - dx, fy + 0 - dy, static_cast<float>(blitOffset), static_cast<float>((sx + w) * us), static_cast<float>((sy + 0) * vs));
    t->vertexUV(fx + 0 - dx,  fy + 0 - dy, static_cast<float>(blitOffset), static_cast<float>((sx + 0) * us), static_cast<float>((sy + 0) * vs));
    t->end();
}