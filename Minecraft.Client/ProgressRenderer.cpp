#include "stdafx.h"
#include "Tesselator.h"
#include "Textures.h"
#include "ProgressRenderer.h"
#include "..\Minecraft.World\System.h"

CRITICAL_SECTION ProgressRenderer::s_progress;

ProgressRenderer::ProgressRenderer(Minecraft *minecraft)
{
	status = -1;
	title = -1;
	lastTime = System::currentTimeMillis();
	noAbort = false;
	this->minecraft = minecraft;
	this->m_eType=eProgressStringType_ID;
}

void ProgressRenderer::progressStart(int title)
{
	noAbort = false;
	_progressStart(title);
}

void ProgressRenderer::progressStartNoAbort(int string)
{
    noAbort = true;
    _progressStart(string);
}

void ProgressRenderer::_progressStart(int title)
{
	// 4J Stu - Removing all progressRenderer rendering. This will be replaced on the xbox
    if (!minecraft->running)
	{
        if (noAbort) return;
//        throw new StopGameException();		// 4J - removed
    }

	EnterCriticalSection( &ProgressRenderer::s_progress );
	lastPercent = 0;
    this->title = title;
	LeaveCriticalSection( &ProgressRenderer::s_progress );

#if 0
    ScreenSizeCalculator ssc(minecraft->options, minecraft->width, minecraft->height);

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (float)ssc.rawWidth, (float)ssc.rawHeight, 0, 100, 300);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -200);
#endif
}

void ProgressRenderer::progressStage(int status)
{
    if (!minecraft->running)
	{
        if (noAbort) return;
//        throw new StopGameException();		// 4J - removed
    }


    lastTime = 0;
	EnterCriticalSection( &ProgressRenderer::s_progress );
	setType(eProgressStringType_ID);
    this->status = status;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
    progressStagePercentage(-1);
    lastTime = 0;
}

void ProgressRenderer::progressStagePercentage(int i)
{
	// 4J Stu - Removing all progressRenderer rendering. This will be replaced on the xbox
	EnterCriticalSection( &ProgressRenderer::s_progress );
	lastPercent = i;
	LeaveCriticalSection( &ProgressRenderer::s_progress );

#if 0
    if (!minecraft->running)
	{
        if (noAbort) return;
//        throw new StopGameException();	// 4J - removed
    }


    int64_t now = System::currentTimeMillis();
    if (now - lastTime < 20) return;
    lastTime = now;

    ScreenSizeCalculator ssc(minecraft->options, minecraft->width, minecraft->height);
    int screenWidth = ssc.getWidth();
    int screenHeight = ssc.getHeight();

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (float)ssc.rawWidth, (float)ssc.rawHeight, 0, 100, 300);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -200);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Tesselator *t = Tesselator::getInstance();
    int id = minecraft->textures->loadTexture(L"/gui/background.png");
    glBindTexture(GL_TEXTURE_2D, id);
    float s = 32;
    t->begin();
    t->color(0x404040);
    t->vertexUV((float)(0), (float)( screenHeight), (float)( 0), (float)( 0), (float)( screenHeight / s));
    t->vertexUV((float)(screenWidth), (float)( screenHeight), (float)( 0), (float)( screenWidth / s), (float)( screenHeight / s));
    t->vertexUV((float)(screenWidth), (float)( 0), (float)( 0), (float)( screenWidth / s), (float)( 0));
    t->vertexUV((float)(0), (float)( 0), (float)( 0), (float)( 0), (float)( 0));
    t->end();

    if (i >= 0)
	{
        int w = 100;
        int h = 2;
        int x = screenWidth / 2 - w / 2;
        int y = screenHeight / 2 + 16;

        glDisable(GL_TEXTURE_2D);
        t->begin();
        t->color(0x808080);
        t->vertex((float)(x), (float)( y), (float)( 0));
        t->vertex((float)(x), (float)( y + h), (float)( 0));
        t->vertex((float)(x + w), (float)( y + h), (float)( 0));
        t->vertex((float)(x + w), (float)( y), (float)( 0));

        t->color(0x80ff80);
        t->vertex((float)(x), (float)( y), (float)( 0));
        t->vertex((float)(x), (float)( y + h), (float)( 0));
        t->vertex((float)(x + i), (float)( y + h), (float)( 0));
        t->vertex((float)(x + i), (float)( y), (float)( 0));
        t->end();
        glEnable(GL_TEXTURE_2D);
    }

    minecraft->font->drawShadow(title, (screenWidth - minecraft->font->width(title)) / 2, screenHeight / 2 - 4 - 16, 0xffffff);
    minecraft->font->drawShadow(status, (screenWidth - minecraft->font->width(status)) / 2, screenHeight / 2 - 4 + 8, 0xffffff);
    Display::update();

	/*	// 4J - removed
    try {
        Thread.yield();
    } catch (Exception e) {
    }
	*/
#endif
}

int ProgressRenderer::getCurrentPercent()
{
	int returnValue = 0;
	EnterCriticalSection( &ProgressRenderer::s_progress );
	returnValue = lastPercent;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
	return returnValue;
}

int ProgressRenderer::getCurrentTitle()
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	int returnValue = title;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
	return returnValue;
}

int ProgressRenderer::getCurrentStatus()
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	int returnValue = status;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
	return returnValue;
}

ProgressRenderer::eProgressStringType ProgressRenderer::getType()
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	eProgressStringType returnValue = m_eType;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
	return returnValue;
}

void ProgressRenderer::setType(eProgressStringType eType)
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	m_eType=eType;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
}

void ProgressRenderer::progressStage(wstring &wstrText)
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	m_wstrText=wstrText;
	setType(eProgressStringType_String);
	LeaveCriticalSection( &ProgressRenderer::s_progress );
}

wstring& ProgressRenderer::getProgressString(void)
{
	EnterCriticalSection( &ProgressRenderer::s_progress );
	wstring &temp=m_wstrText;
	LeaveCriticalSection( &ProgressRenderer::s_progress );
	return temp;
}


