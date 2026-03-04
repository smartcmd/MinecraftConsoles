#pragma once
#include "..\Minecraft.World\AABB.h"

class FrustumData
{
public:
    //enum FrustumSide
    static const int RIGHT = 0; // The RIGHT side of the frustum
    static const int LEFT = 1; // The LEFT    side of the frustum
    static const int BOTTOM = 2; // The BOTTOM side of the frustum
    static const int TOP = 3; // The TOP side of the frustum
    static const int BACK = 4; // The BACK   side of the frustum
    static const int FRONT = 5; // The FRONT side of the frustum

    // Like above, instead of saying a number for the ABC and D of the plane, we
    // want to be more descriptive.
    static const int A = 0; // The X value of the plane's normal
    static const int B = 1; // The Y value of the plane's normal
    static const int C = 2; // The Z value of the plane's normal
    static const int D = 3; // The distance the plane is from the origin

    float** m_Frustum;
    floatArray proj;
    floatArray modl;
    floatArray clip;

	FrustumData();
	~FrustumData();

    bool pointInFrustum(float x, float y, float z);
    bool sphereInFrustum(float x, float y, float z, float radius);
    bool cubeFullyInFrustum(double x1, double y1, double z1, double x2, double y2, double z2);
    bool cubeInFrustum(double x1, double y1, double z1, double x2, double y2, double z2);
    bool isVisible(AABB *aabb);
};