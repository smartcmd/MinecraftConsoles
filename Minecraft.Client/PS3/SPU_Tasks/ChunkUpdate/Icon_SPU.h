#pragma once

#include <stdint.h>

class Icon_SPU
{

	int16_t x0;
	int16_t y0;
	int16_t x1;
	int16_t y1;

public:

// 	static const int TYPE_TERRAIN = 0;
// 	static const int TYPE_ITEM = 1;
// 
	static const float UVAdjust = (1.0f/16.0f)/256.0f;

// 	int getX() const			{ return x; }
// 	int getY() const			{ return y; }
// 	int getWidth() const		{ return (w<0) ?  -w : w; }	// can be negative, to support flipped icons (only doors for now).
// 	int getHeight() const		{ return h; }

	void set(int16_t _x, int16_t _y, int16_t _w, int16_t _h, int texWidth, int texHeight)		
	{ 
		x0 = static_cast<int16_t>(4096 * (float(_x) / texWidth));
		y0 = static_cast<int16_t>(4096 * (float(_y) / texHeight));
		x1 = x0 + static_cast<int16_t>(4096 * (float(_w) / texWidth));
		y1 = y0 + static_cast<int16_t>(4096 * (float(_h) / texHeight));
	}

	void flipHorizontal() { int16_t temp = x0; x0 = x1; x1 = temp; } 
	void flipVertical() { int16_t temp = y0; y0 = y1; y1 = temp; } 

	float getU0() const			{ return (static_cast<float>(x0) / 4096) + UVAdjust; }//sc_texWidth) + getUAdjust(); }
	float getU1() const			{ return (static_cast<float>(x1) / 4096.0f) - UVAdjust; } //sc_texWidth) - getUAdjust(); }
 	float getU(double offset) const
	{
		float diff = getU1() - getU0();
		return getU0() + (diff * (static_cast<float>(offset) / 16));//SharedConstants::WORLD_RESOLUTION));
	}

	float getV0() const			{ return (static_cast<float>(y0) / 4096.0f) + UVAdjust; } //sc_texHeight) + getVAdjust(); }
	float getV1() const			{ return (static_cast<float>(y1) / 4096.0f) - UVAdjust; } //sc_texHeight) - getVAdjust(); }
 	float getV(double offset) const
	{
		float diff = getV1() - getV0();
		return getV0() + (diff * (static_cast<float>(offset) / 16)); //SharedConstants::WORLD_RESOLUTION));
	}

// 	virtual wstring getName() const = 0;
// 	virtual int getSourceWidth() const = 0;
// 	virtual int getSourceHeight() const = 0;
};

