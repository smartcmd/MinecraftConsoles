#include "stdafx.h"
#include "ScreenSizeCalculator.h"
#include "Options.h"

ScreenSizeCalculator::ScreenSizeCalculator(Options *options, int width, int height, int forceScale/*=-1*/)
{
    w = width;
    h = height;
	if( forceScale == -1 )
	{
		scale = 1;

		int maxScale = options->guiScale;
		if (maxScale == 0) maxScale = 1000;
		while (scale < maxScale && w / (scale + 1) >= 320 && h / (scale + 1) >= 240)
		{
			scale++;
		}
	}
	else
	{
		scale = forceScale;
	}
    rawWidth = w / static_cast<double>(scale);
    rawHeight = h / static_cast<double>(scale);
    w = static_cast<int>(ceil(rawWidth));
    h = static_cast<int>(ceil(rawHeight));
}

int ScreenSizeCalculator::getWidth()
{
	return w;
}

int ScreenSizeCalculator::getHeight()
{
	return h;
}