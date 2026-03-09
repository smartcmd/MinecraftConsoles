#include "stdafx.h"
#include "FastNoise.h"

FastNoise::FastNoise(int levels)
{
	Random random;
	init(&random, levels);
}

FastNoise::FastNoise(Random *random, int levels)
{
	init(random,levels);
}

void FastNoise::init(Random *random, int levels)
{
    this->levels = levels;
    noiseMaps = new byte *[levels];
    for (int i = 0; i < levels; i++)
	{
		noiseMaps[i] = new byte[0x100000];
        random->nextBytes(noiseMaps[i],0x100000);
    }
}

FastNoise::~FastNoise()
{
	for( int i = 0; i < levels; i++ )
	{
		delete [] noiseMaps[i];
	}
	delete [] noiseMaps;
}

doubleArray FastNoise::getRegion(doubleArray buffer, double x, double y, double z, int xSize, int ySize, int zSize, double xScale, double yScale, double zScale)
{
    if (buffer.data == nullptr) buffer = doubleArray(xSize * ySize * zSize);
    else for (unsigned int i = 0; i < buffer.length; i++)
        buffer[i] = 0;


    double pow = 1;
    int AA = 487211441;
    int BB = 21771;
    for (int i = 0; i < levels; i++)
	{
        byte *map = noiseMaps[i];
        int pp = 0;

        for (int zp = 0; zp < zSize; zp++)
		{
            double zz = (z + zp) * zScale;
            int Z = static_cast<int>(zz);
            if (zz < Z) Z -= 1;
            int zl = static_cast<int>((zz - Z) * 65536);

            for (int yp = 0; yp < ySize; yp++)
			{
                double yy = (y + yp) * yScale;
                int Y = static_cast<int>(yy);
                if (yy < Y) Y -= 1;
                int yl = static_cast<int>((yy - Y) * 65536);

                for (int xp = 0; xp < xSize; xp++)
				{
                    double xx = (x + xp) * xScale;
                    int X = static_cast<int>(xx);
                    if (xx < X) X -= 1;
                    int xl = static_cast<int>((xx - X) * 65536);
                        
                    int X0 = (X + 0) * AA;
                    int X1 = (X + 1) * AA;
                        
                    int Y0 = (Y + 0);
                    int Y1 = (Y + 1);
                    int Z0 = (Z + 0);
                    int Z1 = (Z + 1);

                    int X0Y0 = (X0 + Y0) * BB;
                    int X1Y0 = (X1 + Y0) * BB;
                    int X0Y1 = (X0 + Y1) * BB;
                    int X1Y1 = (X1 + Y1) * BB;

                    int a = map[(X0Y0 + Z0) & 0xfffff];
                    int b = map[(X1Y0 + Z0) & 0xfffff];
                    int c = map[(X0Y1 + Z0) & 0xfffff];
                    int d = map[(X1Y1 + Z0) & 0xfffff];
                    int e = map[(X0Y0 + Z1) & 0xfffff];
                    int f = map[(X1Y0 + Z1) & 0xfffff];
                    int g = map[(X0Y1 + Z1) & 0xfffff];
                    int h = map[(X1Y1 + Z1) & 0xfffff];

                    int ab = a + (((b - a) * xl) >> 16);
                    int cd = c + (((d - c) * xl) >> 16);
                    int ef = e + (((f - e) * xl) >> 16);
                    int gh = g + (((h - g) * xl) >> 16);

                    int abcd = ab + (((cd - ab) * yl) >> 16);
                    int efgh = ef + (((gh - ef) * yl) >> 16);

                    int res = abcd + (((efgh - abcd) * zl) >> 16);

                    buffer[pp++] += res*pow;
                }
            }
        }
        pow /= 2;
        xScale*=2;
        yScale*=2;
        zScale*=2;
    }

    return buffer;
}
