#include "stdafx.h"
#include "SimplexNoise.h"

int SimplexNoise::grad3[12][3] = { { 1, 1, 0 }, { -1, 1, 0 }, { 1, -1, 0 }, { -1, -1, 0 }, { 1, 0, 1 }, { -1, 0, 1 }, { 1, 0, -1 }, { -1, 0, -1 }, { 0, 1, 1 }, { 0, -1, 1 }, { 0, 1, -1 }, { 0, -1, -1 } };

double SimplexNoise::F2 = 0.5 * (sqrt(3.0) - 1.0);
double SimplexNoise::G2 = (3.0 - sqrt(3.0)) / 6.0;
double SimplexNoise::F3 = 1.0 / 3.0;
double SimplexNoise::G3 = 1.0 / 6.0;

SimplexNoise::SimplexNoise()
{
	Random random;
	init(&random);
}

SimplexNoise::SimplexNoise(Random *random)
{
	init(random);
}

void SimplexNoise::init(Random *random)
{
	p = new int[512];

    xo = random->nextDouble() * 256;
    yo = random->nextDouble() * 256;
    zo = random->nextDouble() * 256;
    for (int i = 0; i < 256; i++)
	{
        p[i] = i;
    }

    for (int i = 0; i < 256; i++)
	{
        int j = random->nextInt(256 - i) + i;
        int tmp = p[i];
        p[i] = p[j];
        p[j] = tmp;

        p[i + 256] = p[i];
    }
}

SimplexNoise::~SimplexNoise()
{
	delete [] p;
}

int SimplexNoise::fastfloor(double x)
{
	return x > 0 ? static_cast<int>(x) : static_cast<int>(x) - 1;
}

double SimplexNoise::dot(int *g, double x, double y)
{
	return g[0] * x + g[1] * y;
}

double SimplexNoise::dot(int *g, double x, double y, double z)
{
	return g[0] * x + g[1] * y + g[2] * z;
}

double SimplexNoise::getValue(double xin, double yin)
{
    double n0, n1, n2; // Noise contributions from the three corners
    // Skew the input space to determine which simplex cell we're in
    double s = (xin + yin) * F2; // Hairy factor for 2D
    int i = fastfloor(xin + s);
    int j = fastfloor(yin + s);
    double t = (i + j) * G2;
    double X0 = i - t; // Unskew the cell origin back to (x,y) space
    double Y0 = j - t;
    double x0 = xin - X0; // The x,y distances from the cell origin
    double y0 = yin - Y0;
    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if (x0 > y0) {
        i1 = 1;
        j1 = 0;
    } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
    else {
        i1 = 0;
        j1 = 1;
    } // upper triangle, YX order: (0,0)->(0,1)->(1,1)
    // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
    // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
    // c = (3-sqrt(3))/6
    double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
    double y1 = y0 - j1 + G2;
    double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
    double y2 = y0 - 1.0 + 2.0 * G2;
    // Work out the hashed gradient indices of the three simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int gi0 = p[ii + p[jj]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1]] % 12;
    int gi2 = p[ii + 1 + p[jj + 1]] % 12;
    // Calculate the contribution from the three corners
    double t0 = 0.5 - x0 * x0 - y0 * y0;
    if (t0 < 0) n0 = 0.0;
    else {
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
    }
    double t1 = 0.5 - x1 * x1 - y1 * y1;
    if (t1 < 0) n1 = 0.0;
    else {
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
    }
    double t2 = 0.5 - x2 * x2 - y2 * y2;
    if (t2 < 0) n2 = 0.0;
    else {
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to return values in the interval [-1,1].
    return 70.0 * (n0 + n1 + n2);
}

double SimplexNoise::getValue(double xin, double yin, double zin)
{
    double n0, n1, n2, n3;
    double s = (xin + yin + zin) * F3;
    int i = fastfloor(xin + s);
    int j = fastfloor(yin + s);
    int k = fastfloor(zin + s);

    double t = (i + j + k) * G3;
    double X0 = i - t;
    double Y0 = j - t;
    double Z0 = k - t;
    double x0 = xin - X0;
    double y0 = yin - Y0;
    double z0 = zin - Z0;
    int i1, j1, k1;
    int i2, j2, k2;
    if (x0 >= y0)
	{
        if (y0 >= z0) 
		{
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        } // X Y Z order
        else if (x0 >= z0)
		{
            i1 = 1;
            j1 = 0;
            k1 = 0;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        } // X Z Y order
        else
		{
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 1;
            j2 = 0;
            k2 = 1;
        } // Z X Y order
    } 
	else
	{ // x0<y0
        if (y0 < z0)
		{
            i1 = 0;
            j1 = 0;
            k1 = 1;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } // Z Y X order
        else if (x0 < z0)
		{
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 0;
            j2 = 1;
            k2 = 1;
        } // Y Z X order
        else
		{
            i1 = 0;
            j1 = 1;
            k1 = 0;
            i2 = 1;
            j2 = 1;
            k2 = 0;
        } // Y X Z order
    }
    // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
    // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
    // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
    // c = 1/6.   

    double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
    double y1 = y0 - j1 + G3;
    double z1 = z0 - k1 + G3;
    double x2 = x0 - i2 + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
    double y2 = y0 - j2 + 2.0 * G3;
    double z2 = z0 - k2 + 2.0 * G3;
    double x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
    double y3 = y0 - 1.0 + 3.0 * G3;
    double z3 = z0 - 1.0 + 3.0 * G3;
    // Work out the hashed gradient indices of the four simplex corners
    int ii = i & 255;
    int jj = j & 255;
    int kk = k & 255;
    int gi0 = p[ii + p[jj + p[kk]]] % 12;
    int gi1 = p[ii + i1 + p[jj + j1 + p[kk + k1]]] % 12;
    int gi2 = p[ii + i2 + p[jj + j2 + p[kk + k2]]] % 12;
    int gi3 = p[ii + 1 + p[jj + 1 + p[kk + 1]]] % 12;
    // Calculate the contribution from the four corners
    double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
    if (t0 < 0) n0 = 0.0;
    else
	{
        t0 *= t0;
        n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
    }
    double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) n1 = 0.0;
    else
	{
        t1 *= t1;
        n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
    }
    double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) n2 = 0.0;
    else
	{
        t2 *= t2;
        n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
    }
    double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) n3 = 0.0;
    else
	{
        t3 *= t3;
        n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
    }
    // Add contributions from each corner to get the final noise value.
    // The result is scaled to stay just inside [-1,1]
    return 32.0 * (n0 + n1 + n2 + n3);
}

void SimplexNoise::add(doubleArray buffer, double _x, double _y, int xSize, int ySize, double xs, double ys, double pow)
{
    int pp = 0;
    for (int xx = 0; xx < xSize; xx++)
	{
        double xin = (_x + xx) * xs + xo;
        for (int yy = 0; yy < ySize; yy++)
		{
            double yin = (_y + yy) * ys + yo;
                
            double n0, n1, n2;
            double s = (xin + yin) * F2; // Hairy factor for 2D
            int i = fastfloor(xin + s);
            int j = fastfloor(yin + s);
            double t = (i + j) * G2;
            double X0 = i - t; // Unskew the cell origin back to (x,y) space
            double Y0 = j - t;
            double x0 = xin - X0; // The x,y distances from the cell origin
            double y0 = yin - Y0;
            // For the 2D case, the simplex shape is an equilateral triangle.
            // Determine which simplex we are in.
            int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
            if (x0 > y0)
			{
                i1 = 1;
                j1 = 0;
            } // lower triangle, XY order: (0,0)->(1,0)->(1,1)
            else
			{
                i1 = 0;
                j1 = 1;
            } // upper triangle, YX order: (0,0)->(0,1)->(1,1)
            // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
            // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
            // c = (3-sqrt(3))/6
            double x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
            double y1 = y0 - j1 + G2;
            double x2 = x0 - 1.0 + 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
            double y2 = y0 - 1.0 + 2.0 * G2;
            // Work out the hashed gradient indices of the three simplex corners
            int ii = i & 255;
            int jj = j & 255;
            int gi0 = p[ii + p[jj]] % 12;
            int gi1 = p[ii + i1 + p[jj + j1]] % 12;
            int gi2 = p[ii + 1 + p[jj + 1]] % 12;
            // Calculate the contribution from the three corners
            double t0 = 0.5 - x0 * x0 - y0 * y0;
            if (t0 < 0) n0 = 0.0;
            else
			{
                t0 *= t0;
                n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
            }
            double t1 = 0.5 - x1 * x1 - y1 * y1;
            if (t1 < 0) n1 = 0.0;
            else
			{
                t1 *= t1;
                n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
            }
            double t2 = 0.5 - x2 * x2 - y2 * y2;
            if (t2 < 0) n2 = 0.0;
            else
			{
                t2 *= t2;
                n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
            }
            // Add contributions from each corner to get the final noise value.
            // The result is scaled to return values in the interval [-1,1].
            buffer[pp++] += (70.0 * (n0 + n1 + n2))*pow;                    
        }
    }

}
void SimplexNoise::add(doubleArray buffer, double _x, double _y, double _z, int xSize, int ySize, int zSize, double xs, double ys, double zs, double pow)
{
    int pp = 0;
    for (int xx = 0; xx < xSize; xx++)
	{
        double xin = (_x + xx) * xs + xo;
        for (int zz = 0; zz < zSize; zz++)
		{
            double zin = (_z + zz) * zs + zo;
            for (int yy = 0; yy < ySize; yy++)
			{
                double yin = (_y + yy) * ys + yo;
                    
                double n0, n1, n2, n3;
                double s = (xin + yin + zin) * F3;
                int i = fastfloor(xin + s);
                int j = fastfloor(yin + s);
                int k = fastfloor(zin + s);
                double t = (i + j + k) * G3;
                double X0 = i - t;
                double Y0 = j - t;
                double Z0 = k - t;
                double x0 = xin - X0;
                double y0 = yin - Y0;
                double z0 = zin - Z0;
                int i1, j1, k1;
                int i2, j2, k2;
                if (x0 >= y0)
				{
                    if (y0 >= z0)
					{
                        i1 = 1;
                        j1 = 0;
                        k1 = 0;
                        i2 = 1;
                        j2 = 1;
                        k2 = 0;
                    } // X Y Z order
                    else if (x0 >= z0)
					{
                        i1 = 1;
                        j1 = 0;
                        k1 = 0;
                        i2 = 1;
                        j2 = 0;
                        k2 = 1;
                    } // X Z Y order
                    else
					{
                        i1 = 0;
                        j1 = 0;
                        k1 = 1;
                        i2 = 1;
                        j2 = 0;
                        k2 = 1;
                    } // Z X Y order
                }
				else
				{ // x0<y0
                    if (y0 < z0)
					{
                        i1 = 0;
                        j1 = 0;
                        k1 = 1;
                        i2 = 0;
                        j2 = 1;
                        k2 = 1;
                    } // Z Y X order
                    else if (x0 < z0)
					{
                        i1 = 0;
                        j1 = 1;
                        k1 = 0;
                        i2 = 0;
                        j2 = 1;
                        k2 = 1;
                    } // Y Z X order
                    else
					{
                        i1 = 0;
                        j1 = 1;
                        k1 = 0;
                        i2 = 1;
                        j2 = 1;
                        k2 = 0;
                    } // Y X Z order
                }
                // A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
                // a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
                // a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
                // c = 1/6.   

                double x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
                double y1 = y0 - j1 + G3;
                double z1 = z0 - k1 + G3;
                double x2 = x0 - i2 + 2.0 * G3; // Offsets for third corner in (x,y,z) coords
                double y2 = y0 - j2 + 2.0 * G3;
                double z2 = z0 - k2 + 2.0 * G3;
                double x3 = x0 - 1.0 + 3.0 * G3; // Offsets for last corner in (x,y,z) coords
                double y3 = y0 - 1.0 + 3.0 * G3;
                double z3 = z0 - 1.0 + 3.0 * G3;
                // Work out the hashed gradient indices of the four simplex corners
                int ii = i & 255;
                int jj = j & 255;
                int kk = k & 255;
                int gi0 = p[ii + p[jj + p[kk]]] % 12;
                int gi1 = p[ii + i1 + p[jj + j1 + p[kk + k1]]] % 12;
                int gi2 = p[ii + i2 + p[jj + j2 + p[kk + k2]]] % 12;
                int gi3 = p[ii + 1 + p[jj + 1 + p[kk + 1]]] % 12;
                // Calculate the contribution from the four corners
                double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
                if (t0 < 0) n0 = 0.0;
                else
				{
                    t0 *= t0;
                    n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
                }
                double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
                if (t1 < 0) n1 = 0.0;
                else
				{
                    t1 *= t1;
                    n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
                }
                double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
                if (t2 < 0) n2 = 0.0;
                else
				{
                    t2 *= t2;
                    n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
                }
                double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
                if (t3 < 0) n3 = 0.0;
                else
				{
                    t3 *= t3;
                    n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
                }
                // Add contributions from each corner to get the final noise value.
                // The result is scaled to stay just inside [-1,1]
                buffer[pp++] += (32.0 * (n0 + n1 + n2 + n3))*pow;
            }
        }
    }
}