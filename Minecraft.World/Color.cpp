#include "stdafx.h"

#include "Color.h"

//Creates an opaque sRGB color with the specified red, green, and blue values in the range (0.0 - 1.0).
//Alpha is defaulted to 1.0. The actual color used in rendering depends on finding the best match given the color space
//available for a particular output device.
//Parameters:
//r - the red component
//g - the green component
//b - the blue component
//Throws:
//IllegalArgumentException - if r, g or b are outside of the range 0.0 to 1.0, inclusive
Color::Color( float r, float g, float b)
{
	assert( r >= 0.0f && r <= 1.0f );
	assert( g >= 0.0f && g <= 1.0f );
	assert( b >= 0.0f && b <= 1.0f );

	//argb
	colour = ( (0xFF<<24) | ( static_cast<int>(r * 255)<<16 ) | ( static_cast<int>(g * 255)<<8 ) | static_cast<int>(b * 255) );
}

Color::Color( int r, int g, int b)
{
	colour = ( (0xFF<<24) | ( (r&0xff)<<16 ) | ( (g&0xff)<<8 ) | ( (b&0xff) ) );
}


//Creates a Color object based on the specified values for the HSB color model.
//The s and b components should be floating-point values between zero and one (numbers in the range 0.0-1.0).
//The h component can be any floating-point number. The floor of this number is subtracted from it to create a fraction between 0 and 1.
//This fractional number is then multiplied by 360 to produce the hue angle in the HSB color model.
//
//Parameters:
//h - the hue component
//s - the saturation of the color
//b - the brightness of the color
//Returns:
//a Color object with the specified hue, saturation, and brightness.
Color Color::getHSBColor(float hue, float saturation, float brightness)
{
	int r = 0, g = 0, b = 0;
    if (saturation == 0)
	{
	    r = g = b = static_cast<int>(brightness * 255.0f + 0.5f);
	}
	else
	{
	    float h = (hue - (float)floor(hue)) * 6.0f;
	    float f = h - (float)floor(h);
	    float p = brightness * (1.0f - saturation);
	    float q = brightness * (1.0f - saturation * f);
	    float t = brightness * (1.0f - (saturation * (1.0f - f)));
	    switch (static_cast<int>(h))
		{
			case 0:
				r = static_cast<int>(brightness * 255.0f + 0.5f);
				g = static_cast<int>(t * 255.0f + 0.5f);
				b = static_cast<int>(p * 255.0f + 0.5f);
			break;
			case 1:
				r = static_cast<int>(q * 255.0f + 0.5f);
				g = static_cast<int>(brightness * 255.0f + 0.5f);
				b = static_cast<int>(p * 255.0f + 0.5f);
			break;
			case 2:
				r = static_cast<int>(p * 255.0f + 0.5f);
				g = static_cast<int>(brightness * 255.0f + 0.5f);
				b = static_cast<int>(t * 255.0f + 0.5f);
			break;
			case 3:
				r = static_cast<int>(p * 255.0f + 0.5f);
				g = static_cast<int>(q * 255.0f + 0.5f);
				b = static_cast<int>(brightness * 255.0f + 0.5f);
			break;
			case 4:
				r = static_cast<int>(t * 255.0f + 0.5f);
				g = static_cast<int>(p * 255.0f + 0.5f);
				b = static_cast<int>(brightness * 255.0f + 0.5f);
			break;
			case 5:
				r = static_cast<int>(brightness * 255.0f + 0.5f);
				g = static_cast<int>(p * 255.0f + 0.5f);
				b = static_cast<int>(q * 255.0f + 0.5f);
			break;
	    }
	}


	return Color( r, g, b );
}

int Color::getRGB()
{
	return colour;
}