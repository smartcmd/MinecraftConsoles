#pragma once
// #include "Synth.h"



class ImprovedNoise_SPU// : public Synth
{
public:
    int p[512];

    double scale;
    double xo, yo, zo;


    double lerp(double t, double a, double b);
    double grad2(int hash, double x, double z);
    double grad(int hash, double x, double y, double z);

    void add(double* buffer, double _x, double _y, double _z, int xSize, int ySize, int zSize, double xs, double ys, double zs, double pow);
};
