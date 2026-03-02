#pragma once

class Projectile
{
public:
	virtual void shoot(double xd, double yd, double zd, float pow, float uncertainty) = 0;
};