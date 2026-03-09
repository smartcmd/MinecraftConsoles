#include "stdafx.h"
#include "MobSkinTextureProcessor.h"

BufferedImage *MobSkinTextureProcessor::process(BufferedImage *in)
{
    if (in == nullptr) return nullptr;

    width = 64;
    height = 32;

    BufferedImage *out = new BufferedImage(width, height, BufferedImage::TYPE_INT_ARGB);
    Graphics *g = out->getGraphics();
    g->drawImage(in, 0, 0, nullptr);
    g->dispose();

    pixels = out->getData();

    setNoAlpha(0, 0, 32, 16);
    setForceAlpha(32, 0, 64, 32);
    setNoAlpha(0, 16, 64, 32);
    bool hasAlpha = false;
    for (int x = 32; x < 64; x++)
        for (int y = 0; y < 16; y++)
		{
            int pix = pixels[x + y * 64];
            if (((pix >> 24) & 0xff) < 128) hasAlpha = true;
        }

    if (!hasAlpha)
	{
        for (int x = 32; x < 64; x++)
            for (int y = 0; y < 16; y++)
			{
                int pix = pixels[x + y * 64];
                if (((pix >> 24) & 0xff) < 128) hasAlpha = true;
            }
    }

    return out;
}

void MobSkinTextureProcessor::setForceAlpha(int x0, int y0, int x1, int y1)
{
    if (hasAlpha(x0, y0, x1, y1)) return;

    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
		{
            pixels[x + y * width] &= 0x00ffffff;
        }
}

void MobSkinTextureProcessor::setNoAlpha(int x0, int y0, int x1, int y1)
{
    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
		{
            pixels[x + y * width] |= 0xff000000;
        }
}

bool MobSkinTextureProcessor::hasAlpha(int x0, int y0, int x1, int y1)
{
    for (int x = x0; x < x1; x++)
        for (int y = y0; y < y1; y++)
		{
            int pix = pixels[x + y * width];
            if (((pix >> 24) & 0xff) < 128) return true;
        }
    return false;
}