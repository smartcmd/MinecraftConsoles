#include "stdafx.h"


#ifdef SN_TARGET_PS3_SPU
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <spu_intrinsics.h>
#include <cell/spurs.h>
#include <cell/dma.h>
#include <math.h>
#include "..\Common\spu_assert.h"
#endif // SN_TARGET_PS3_SPU

#include "Tesselator_SPU.h"

static const bool sc_verbose = false;
// #include "..\Minecraft.World\BasicTypeContainers.h"
// #include "..\Minecraft.World\FloatBuffer.h"
// #include "..\Minecraft.World\IntBuffer.h"
// #include "..\Minecraft.World\ByteBuffer.h"

#ifdef SN_TARGET_PS3_SPU

const int GL_LINES = 4;//C4JRender::PRIMITIVE_TYPE_LINE_LIST;
const int GL_LINE_STRIP = 5;//C4JRender::PRIMITIVE_TYPE_LINE_STRIP;
const int GL_QUADS = 3;//C4JRender::PRIMITIVE_TYPE_QUAD_LIST;
const int GL_TRIANGLE_FAN = 2;//C4JRender::PRIMITIVE_TYPE_TRIANGLE_FAN;
const int GL_TRIANGLE_STRIP = 1;//C4JRender::PRIMITIVE_TYPE_TRIANGLE_STRIP;

#endif 

bool Tesselator_SPU::TRIANGLE_MODE = false;
// bool Tesselator_SPU::USE_VBO = false;

/* Things to check we are intialising in the constructor...



double u, v;
int col;
int mode;
double xo, yo, zo;
int normal;






*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// taken from http://my.safaribooksonline.com/book/programming/opengl/9780321563835/gl-half-float-oes/app01lev1sec2

// -15 stored using a single precision bias of 127
const unsigned int  HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP = 0x38000000;
// max exponent value in single precision that will be converted
// to Inf or Nan when stored as a half-float
const unsigned int  HALF_FLOAT_MAX_BIASED_EXP_AS_SINGLE_FP_EXP = 0x47800000;

// 255 is the max exponent biased value
const unsigned int  FLOAT_MAX_BIASED_EXP = (0xFF << 23);

const unsigned int  HALF_FLOAT_MAX_BIASED_EXP = (0x1F << 10);

typedef unsigned short    hfloat;

hfloat convertFloatToHFloat(float f)
{
	unsigned int    x = *(unsigned int *)&f;
	unsigned int    sign = static_cast<unsigned short>(x >> 31);
	unsigned int    mantissa;
	unsigned int    exp;
	hfloat          hf;

	// get mantissa
	mantissa = x & ((1 << 23) - 1);
	// get exponent bits
	exp = x & FLOAT_MAX_BIASED_EXP;
	if (exp >= HALF_FLOAT_MAX_BIASED_EXP_AS_SINGLE_FP_EXP)
	{
		// check if the original single precision float number is a NaN
		if (mantissa && (exp == FLOAT_MAX_BIASED_EXP))
		{
			// we have a single precision NaN
			mantissa = (1 << 23) - 1;
		}
		else
		{
			// 16-bit half-float representation stores number as Inf
			mantissa = 0;
		}
		hf = (static_cast<hfloat>(sign) << 15) | static_cast<hfloat>(HALF_FLOAT_MAX_BIASED_EXP) |
			static_cast<hfloat>(mantissa >> 13);
	}
	// check if exponent is <= -15
	else if (exp <= HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP)
	{

		// store a denorm half-float value or zero
		exp = (HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP - exp) >> 23;
		mantissa >>= (14 + exp);

		hf = (static_cast<hfloat>(sign) << 15) | static_cast<hfloat>(mantissa);
	}
	else
	{
		hf = (static_cast<hfloat>(sign) << 15) |
			static_cast<hfloat>((exp - HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP) >> 13) |
			static_cast<hfloat>(mantissa >> 13);
	}

	return hf;
}

float convertHFloatToFloat(hfloat hf)
{
	unsigned int    sign = static_cast<unsigned int>(hf >> 15);
	unsigned int    mantissa = static_cast<unsigned int>(hf & ((1 << 10) - 1));
	unsigned int    exp = (unsigned int)(hf & HALF_FLOAT_MAX_BIASED_EXP);
	unsigned int    f;

	if (exp == HALF_FLOAT_MAX_BIASED_EXP)
	{
		// we have a half-float NaN or Inf
		// half-float NaNs will be converted to a single precision NaN
		// half-float Infs will be converted to a single precision Inf
		exp = FLOAT_MAX_BIASED_EXP;
		if (mantissa)
			mantissa = (1 << 23) - 1;    // set all bits to indicate a NaN
	}
	else if (exp == 0x0)
	{
		// convert half-float zero/denorm to single precision value
		if (mantissa)
		{
			mantissa <<= 1;
			exp = HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
			// check for leading 1 in denorm mantissa
			while ((mantissa & (1 << 10)) == 0)
			{
				// for every leading 0, decrement single precision exponent by 1
				// and shift half-float mantissa value to the left
				mantissa <<= 1;
				exp -= (1 << 23);
			}
			// clamp the mantissa to 10-bits
			mantissa &= ((1 << 10) - 1);
			// shift left to generate single-precision mantissa of 23-bits
			mantissa <<= 13;
		}
	}
	else
	{
		// shift left to generate single-precision mantissa of 23-bits
		mantissa <<= 13;
		// generate single precision biased exponent value
		exp = (exp << 13) + HALF_FLOAT_MIN_BIASED_EXP_AS_SINGLE_FP_EXP;
	}

	f = (sign << 31) | exp | mantissa;
	return *((float *)&f);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// DWORD Tesselator_SPU::tlsIdx = TlsAlloc();
// 
Tesselator_SPU *Tesselator_SPU::getInstance()
{
	return nullptr;
// 	return (Tesselator_SPU *)TlsGetValue(tlsIdx);
}

// void Tesselator_SPU::CreateNewThreadStorage(int bytes)
// {
// 	Tesselator_SPU *instance = new Tesselator_SPU(bytes/4);
// 	TlsSetValue(tlsIdx, instance);
// }


																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																							
 void Tesselator_SPU::end()
 {
// //    if (!tesselating) throw new IllegalStateException("Not tesselating!");	// 4J - removed
//     tesselating = false;
//     if (vertices > 0)
// 	{
// 		// 4J - a lot of stuff taken out here for fiddling round with enable client states etc.
// 		// that don't matter for our renderer
//         if (!hasColor)
// 		{
// 			// 4J - TEMP put in fixed vertex colors if we don't have any, until we have a shader that can cope without them
// 			unsigned int *pColData = (unsigned int *)_array->data;
// 			pColData += 5;
// 			for( int i = 0; i < vertices; i++ )
// 			{
// 				*pColData = 0xffffffff;
// 				pColData += 8;
// 			}
// 		}
//         if (mode == GL_QUADS && TRIANGLE_MODE)
// 		{
//             // glDrawArrays(GL_TRIANGLES, 0, vertices); // 4J - changed for xbox
// #ifdef _XBOX
// 			RenderManager.DrawVertices(D3DPT_TRIANGLELIST,vertices,_array->data,
// 									   useCompactFormat360?C4JRender::VERTEX_TYPE_PS3_TS2_CS1:C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
// 									   useProjectedTexturePixelShader?C4JRender::PIXEL_SHADER_TYPE_PROJECTION:C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// #else
// 			RenderManager.DrawVertices(C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST,vertices,_array->data,
// 									   useCompactFormat360?C4JRender::VERTEX_TYPE_COMPRESSED:C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
// 									   useProjectedTexturePixelShader?C4JRender::PIXEL_SHADER_TYPE_PROJECTION:C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// #endif
//         }
// 		else
// 		{
// //            glDrawArrays(mode, 0, vertices);	// 4J - changed for xbox
// 			// For compact vertices, the vertexCount has to be calculated from the amount of data written, as
// 			// we insert extra fake vertices to encode supplementary data for more awkward quads that have non
// 			// axis aligned UVs (eg flowing lava/water)
// #ifdef _XBOX
// 			int vertexCount = vertices;
// 			if( useCompactFormat360 )
// 			{
// 				vertexCount = p / 2;
// 				RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PS3_TS2_CS1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// 			}
// 			else
// 			{
// 				if( useProjectedTexturePixelShader )
// 				{
// 					RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN, C4JRender::PIXEL_SHADER_TYPE_PROJECTION);
// 				}
// 				else
// 				{
// 					RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// 				}
// 			}
// #else
// 			int vertexCount = vertices;
// 			if( useCompactFormat360 )
// 			{
// 				RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_COMPRESSED, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// 			}
// 			else
// 			{
// 				if( useProjectedTexturePixelShader )
// 				{
// 					RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN, C4JRender::PIXEL_SHADER_TYPE_PROJECTION);
// 				}
// 				else
// 				{
// 					RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
// 				}
// 			}
// #endif
//         }
//         glDisableClientState(GL_VERTEX_ARRAY);
//         if (hasTexture) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//         if (hasColor) glDisableClientState(GL_COLOR_ARRAY);
//         if (hasNormal) glDisableClientState(GL_NORMAL_ARRAY);
//     }
// 
//     clear();
}

void Tesselator_SPU::clear()
{
    vertices = 0;

    p = 0;
    count = 0;
}

void Tesselator_SPU::begin()
{
    begin(GL_QUADS);
	bounds.reset();
}
 
void Tesselator_SPU::useProjectedTexture(bool enable)
{
	useProjectedTexturePixelShader = enable;
}

void Tesselator_SPU::useCompactVertices(bool enable)
{
	useCompactFormat360 = enable;
}

bool Tesselator_SPU::setMipmapEnable(bool enable)
{
	bool prev = mipmapEnable;
	mipmapEnable = enable;
	return prev;
}

void Tesselator_SPU::begin(int mode)
{
	/*	// 4J - removed
    if (tesselating) {
        throw new IllegalStateException("Already tesselating!");
    } */
    tesselating = true;

    clear();
    this->mode = mode;
    hasNormal = false;
    hasColor = false;
    hasTexture = false;
	hasTexture2 = false;
    _noColor = false;
}

void Tesselator_SPU::tex(float u, float v)
{
    hasTexture = true;
    this->u = u;
    this->v = v;
}

void Tesselator_SPU::tex2(int tex2)
{
    hasTexture2 = true;
	this->_tex2 = tex2;
}

void Tesselator_SPU::color(float r, float g, float b)
{
    color(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255));
}

void Tesselator_SPU::color(float r, float g, float b, float a)
{
    color(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), static_cast<int>(a * 255));
}

void Tesselator_SPU::color(int r, int g, int b)
{
    color(r, g, b, 255);
}

void Tesselator_SPU::color(int r, int g, int b, int a)
{
    if (_noColor) return;

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (a > 255) a = 255;
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (b < 0) b = 0;
    if (a < 0) a = 0;

    hasColor = true;
	// 4J - removed little-endian option
    col = (r << 24) | (g << 16) | (b << 8) | (a);
}

void Tesselator_SPU::color(byte r, byte g, byte b)
{
	color(r & 0xff, g & 0xff, b & 0xff);
}

void Tesselator_SPU::vertexUV(float x, float y, float z, float u, float v)
{
    tex(u, v);
    vertex(x, y, z);
}

// Pack the 4 vertices of a quad up into a compact format. This is structured as 8 bytes per vertex,
// arranged in blocks of 4 vertices per quad. Currently this is (one letter per nyblle):
//
// cccc xxyy zzll rgbi		(vertex 0)
// umin xxyy zzll rgbi		(vertex 1)
// vmin xxyy zzll rgbi		(vertex 2)
// udvd xxyy zzll rgbi		(vertex 3)
//
// where: cccc		 is a 15-bit (5 bits per x/y/z) origin position / offset for the whole quad. Each
//					 component is unsigned, and offset by 16 so has a range 0 to 31 actually representing -16 to 15
//        xx,yy,zz   are 8-bit deltas from this origin to each vertex. These are unsigned 1.7 fixed point, ie
//                   representing a range of 0 to 1.9921875
//		  rgb        is 4:4:4 RGB
//        umin, vmin are 3:13 unsigned fixed point UVs reprenting the min u and v required by the quad
//        ud,vd		 are 8-bit unsigned fixed pont UV deltas, which can be added to umin/vmin to get umax, vmax
//					 and therefore define the 4 corners of an axis aligned UV mapping
//        i          is a code per vertex that indicates which of umin/umax should be used for u, and which
//					 of vmin/vmax should be used for v for this vertex. The coding is:
//						0 - u = umin, v = vmin
//						1 - u = umin, v = vmax
//						2 - u = umax, v = vmin
//						3 - u = umax, v = vmax
//						4 - not axis aligned, use uv stored in the vertex data 4 on from this one
//		  ll		 is an 8-bit (4 bit per u/v) index into the current lighting texture
//
// For quads that don't have axis aligned UVs (ie have a code for 4 in i as described above) the 8 byte vertex
// is followed by a further 8 bytes which have explicit UVs defined for each vertex:
//
// 0000 0000 uuuu vvvv		(vertex 0)
// 0000 0000 uuuu vvvv		(vertex 1)
// 0000 0000 uuuu vvvv		(vertex 2)
// 0000 0000 uuuu vvvv		(vertex 3)
//

void Tesselator_SPU::packCompactQuad()
{
	// Offset x/y/z by 16 so that we can deal with a -16 -> 16 range
	for( int i = 0; i < 4; i++ )
	{
		m_ix[i] += 16 * 128;
		m_iy[i] += 16 * 128;
		m_iz[i] += 16 * 128;
	}
	// Find min x/y/z
	unsigned int minx = m_ix[0];
	unsigned int miny = m_iy[0];
	unsigned int minz = m_iz[0];
	for( int i = 1; i < 4; i++ )
	{
		if( m_ix[i] < minx ) minx = m_ix[i];
		if( m_iy[i] < miny ) miny = m_iy[i];
		if( m_iz[i] < minz ) minz = m_iz[i];
	}
	// Everything has been scaled by a factor of 128 to get it into an int, and so
	// the minimum now should be in the range of (0->32) * 128. Get the base x/y/z
	// that our quad will be referenced from now, which can be stored in 5 bits
	unsigned int basex = ( minx >> 7 );
	unsigned int basey = ( miny >> 7 );
	unsigned int basez = ( minz >> 7 );
	// If the min is 32, then this whole quad must be in that plane - make the min 15 instead so
	// we can still offset from that with our delta to get to the exact edge
	if( basex == 32 ) basex = 31;
	if( basey == 32 ) basey = 31;
	if( basez == 32 ) basez = 31;
	// Now get deltas to each vertex - these have an 8-bit range so they can span a
	// full unit range from the base position
	for( int i = 0; i < 4; i++ )
	{
		m_ix[i] -= basex << 7;
		m_iy[i] -= basey << 7;
		m_iz[i] -= basez << 7;
	}
	// Now write the data out
	unsigned int *data = (unsigned int *)&_array->data[p];
	
	for( int i = 0; i < 4; i++ )
	{
		data[i * 2 + 0] = ( m_ix[i] << 8 ) | ( m_iy[i] );
		data[i * 2 + 1] = ( m_iz[i] << 24 ) | ( m_clr[i] );
	}
	data[0] |= ( basex << 26 ) | ( basey << 21 )| ( basez << 16 );

	// Now process UVs. First find min & max U & V
	unsigned int minu = m_u[0];
	unsigned int minv = m_v[0];
	unsigned int maxu = m_u[0];
	unsigned int maxv = m_v[0];

	for( int i = 1; i < 4; i++ )
	{
		if( m_u[i] < minu ) minu = m_u[i];
		if( m_v[i] < minv ) minv = m_v[i];
		if( m_u[i] > maxu ) maxu = m_u[i];
		if( m_v[i] > maxv ) maxv = m_v[i];
	}
	// In nearly all cases, all our UVs should be axis aligned for this quad. So the only values they should
	// have in each dimension should be the min/max. We're going to store:
	// (1) minu/maxu (16 bits each, only actuall needs to store 14 bits to get a 0 to 2 range for each
	// (2) du/dv ( ie maxu-minu, maxv-minv) - 8 bits each, to store a range of 0 to 15.9375 texels. This
	// should be enough to map the full UV range of a single 16x16 region of the terrain texture, since
	// we always pull UVs in by 1/16th of their range at the sides
	unsigned int du = maxu - minu;
	unsigned int dv = maxv - minv;
	if( du > 255 )	du = 255;
	if( dv > 255 )	dv = 255;
	// Check if this quad has UVs that can be referenced this way. This should only happen for flowing water
	// and lava, where the texture coordinates are rotated for the top surface of the tile.
	bool axisAligned = true;
	for( int i = 0; i < 4; i++ )
	{
		if(! ( ( ( m_u[i] == minu ) || ( m_u[i] == maxu ) ) &&
			   ( ( m_v[i] == minv ) || ( m_v[i] == maxv ) ) ) )
		{
			axisAligned = false;
		}
	}

	if( axisAligned )
	{
		// Now go through each vertex, and work out which of the min/max should be used for each dimension,
		// and store
		for( int i = 0; i < 4; i++ )
		{
			unsigned int code = 0;
			if( m_u[i] == maxu ) code |= 2;
			if( m_v[i] == maxv ) code |= 1;
			data[i * 2 + 1] |= code;
			data[i * 2 + 1] |= m_t2[i] << 16;
		}
		// Finally, store the minu/minv/du/dv
		data[1 * 2 + 0] |= minu << 16;
		data[2 * 2 + 0] |= minv << 16;
		data[3 * 2 + 0] |= ( du << 24 | dv << 16 );

		incData(4 * 2);
	}
	else
	{
		// The UVs aren't axis aligned - store them in the next 4 vertices. These will be indexed from
		// our base vertices because we'll set a special code (4) for the UVs. They won't be drawn as actual
		// verts when these extra vertices go through the vertex shader, because we'll make sure that
		// they get interpreted as a zero area quad and so they'll be quickly eliminated from rendering post-tranform

		for( int i = 0; i < 4; i++ )
		{
			data[i * 2 + 1] |= ( 4 );	// The special code to indicate they need further data to be fetched
			data[i * 2 + 1] |= m_t2[i] << 16;
			data[8 + i * 2] = 0;	// This includes x/y coordinate of each vert as (0,0) so they will be interpreted as a zero area quad
			data[9 + i * 2] = m_u[i] << 16 | m_v[i];
		}

		// Extra 8 bytes required
		incData(8 * 2);
	}
}

void Tesselator_SPU::vertex(float x, float y, float z)
{
	bounds.addVert(x+xo, y+yo, z+zo);	// 4J MGH - added
	count++;

	// Signal to pixel shader whether to use mipmapping or not, by putting u into > 1 range if it is to be disabled
	float uu = mipmapEnable ? u : (u + 1.0f);

	// 4J - this format added for 360 to keep memory size of tesselated tiles down -
	// see comments in packCompactQuad() for exact format
	if( useCompactFormat360 )
	{
		unsigned int ucol = static_cast<unsigned int>(col);

#ifdef _XBOX
		// Pack as 4:4:4 RGB_
		unsigned short packedcol = (((col & 0xf0000000 ) >> 16 ) |
								    ((col & 0x00f00000 ) >> 12 ) |
								    ((col & 0x0000f000 ) >> 8 ));
		int ipackedcol = ((int)packedcol) & 0xffff;	// 0 to 65535 range

		int quadIdx = vertices % 4;
		m_ix[ quadIdx ] = (unsigned int)((x + xo) * 128.0f);
		m_iy[ quadIdx ] = (unsigned int)((y + yo) * 128.0f);
		m_iz[ quadIdx ] = (unsigned int)((z + zo) * 128.0f);
		m_clr[ quadIdx ] = (unsigned int)ipackedcol;
		m_u[ quadIdx ] = (int)(uu * 4096.0f);
		m_v[ quadIdx ] = (int)(v * 4096.0f);
		m_t2[ quadIdx ] = ( ( _tex2 & 0x00f00000 ) >> 20 ) | ( _tex2 & 0x000000f0 );
		if( quadIdx == 3 )
		{
			packCompactQuad();
		}
#else
		unsigned short packedcol = ((col & 0xf8000000 ) >> 16 ) |
								   ((col & 0x00fc0000 ) >> 13 ) |
								   ((col & 0x0000f800 ) >> 11 );
		int ipackedcol = static_cast<int>(packedcol) & 0xffff;	// 0 to 65535 range

		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

		int16_t* pShortData =  (int16_t*)&_array->data[p];
#ifdef __PS3__
#define INT_ROUND(x) (int)(floorf(x+0.5))
		float tex2U = ((int16_t*)&_tex2)[1] + 8;
		float tex2V = ((int16_t*)&_tex2)[0] + 8;
		float colVal1 = ((col&0xff000000)>>24)/256.0f;
		float colVal2 = ((col&0x00ff0000)>>16)/256.0f;
		float colVal3 = ((col&0x0000ff00)>>8)/256.0f;

		// 		pShortData[0] = convertFloatToHFloat(x + xo);
		// 		pShortData[1] = convertFloatToHFloat(y + yo);
		// 		pShortData[2] = convertFloatToHFloat(z + zo);
		// 		pShortData[3] = convertFloatToHFloat(uu);
		// 		pShortData[4] = convertFloatToHFloat(tex2U + colVal1);
		// 		pShortData[5] = convertFloatToHFloat(tex2V + colVal2);
		// 		pShortData[6] = convertFloatToHFloat(colVal3);
		// 		pShortData[7] = convertFloatToHFloat(v);

		pShortData[0] = ((INT_ROUND((x + xo ) * 1024.0f))&0xffff);
		pShortData[1] = ((INT_ROUND((y + yo ) * 1024.0f))&0xffff);
		pShortData[2] = ((INT_ROUND((z + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = ((INT_ROUND(uu * 8192.0f))&0xffff);
		pShortData[5] = ((INT_ROUND(v * 8192.0f))&0xffff);
		pShortData[6] = ((INT_ROUND(tex2U * (8192.0f/256.0f)))&0xffff);
		pShortData[7] = ((INT_ROUND(tex2V * (8192.0f/256.0f)))&0xffff);
		incData(4);
#else
		pShortData[0] = (static_cast<int>((x + xo) * 1024.0f)&0xffff);
		pShortData[1] = (static_cast<int>((y + yo) * 1024.0f)&0xffff);
		pShortData[2] = (static_cast<int>((z + zo) * 1024.0f)&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (static_cast<int>(uu * 8192.0f)&0xffff);
		pShortData[5] = (static_cast<int>(v * 8192.0f)&0xffff);
		pShortData[6] = ((int16_t*)&_tex2)[0];
		pShortData[7] = ((int16_t*)&_tex2)[1];
		incData(4);
#endif

#endif

		vertices++;
#ifdef _XBOX
		if (vertices % 4 == 0 && ( ( p >= size - 8 * 2 ) || ( ( p / 2 ) >= 65532 ) ) )		// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
#else
		if (vertices % 4 == 0 && ( ( p >= size - 4 * 4 ) || ( ( p / 4 ) >= 65532 ) ) )		// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
#endif
		{
// 			end();
			tesselating = true;
		}
	}
	else
	{
		if (mode == GL_QUADS && TRIANGLE_MODE && count % 4 == 0)
		{
			for (int i = 0; i < 2; i++)
			{
				int offs = 8 * (3 - i);
				if (hasTexture)
				{
					_array->data[p + 3] = _array->data[p - offs + 3];
					_array->data[p + 4] = _array->data[p - offs + 4];
				}
				if (hasColor)
				{
					_array->data[p + 5] = _array->data[p - offs + 5];
				}

				_array->data[p + 0] = _array->data[p - offs + 0];
				_array->data[p + 1] = _array->data[p - offs + 1];
				_array->data[p + 2] = _array->data[p - offs + 2];

				vertices++;
				incData(8);
			}
		}

		if (hasTexture)
		{
			float *fdata = (float *)(_array->data + p + 3);
			*fdata++ = uu;
			*fdata++ = v;
		}
		if (hasColor)
		{
			_array->data[p + 5] = col;
		}
		if (hasNormal)
		{
			_array->data[p + 6] = _normal;
		}
		if (hasTexture2)
		{
#ifdef _XBOX
			_array->data[p + 7] = ( ( _tex2 >> 16 ) & 0xffff ) | ( _tex2 << 16 );
#else
#ifdef __PS3__
			int16_t tex2U = ((int16_t*)&_tex2)[1] + 8;
			int16_t tex2V = ((int16_t*)&_tex2)[0] + 8;
			int16_t* pShortArray = (int16_t*)&_array->data[p + 7];
			pShortArray[0] = tex2U;
			pShortArray[1] = tex2V;
#else
			_array->data[p + 7] = _tex2;
#endif
#endif
		}
		else
		{
			// -512 each for u/v will mean that the renderer will use global settings (set via
			// RenderManager.StateSetVertexTextureUV) rather than these local ones
			*(unsigned int *)(&_array->data[p + 7]) = 0xfe00fe00;
		}

		float *fdata = (float *)(_array->data + p);
		*fdata++ = (x + xo);
		*fdata++ = (y + yo);
		*fdata++ = (z + zo);
		incData(8);

		vertices++;
		if (vertices % 4 == 0 && p >= size - 8 * 4)
		{
//			end();
			tesselating = true;
		}
	}
}

void Tesselator_SPU::color(int c)
{
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b);
}

void Tesselator_SPU::color(int c, int alpha)
{
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b, alpha);
}

void Tesselator_SPU::noColor()
{
    _noColor = true;
}

void Tesselator_SPU::normal(float x, float y, float z)
{
    hasNormal = true;
    byte xx = static_cast<byte>(x * 127);
    byte yy = static_cast<byte>(y * 127);
    byte zz = static_cast<byte>(z * 127);

    _normal = (xx & 0xff) | ((yy & 0xff) << 8) | ((zz & 0xff) << 16);
}

void Tesselator_SPU::offset(float xo, float yo, float zo)
{
    this->xo = xo;
    this->yo = yo;
    this->zo = zo;
}

void Tesselator_SPU::addOffset(float x, float y, float z)
{
    xo += x;
    yo += y;
    zo += z;
}

void Tesselator_SPU::incData( int numInts )
{
	p+=numInts;

#ifdef SN_TARGET_PS3_SPU
	if(p > 4096)
	{
		int dmaSize = p*4;
		unsigned int dmaTag = 2;
		void* src = _array->data;
		uintptr_t dest = (uintptr_t)(((char*)m_PPUArray) + m_PPUOffset);
		if(sc_verbose)
			spu_print("Tesselator : DMA SPU->PPU : 0x%08x -> 0x%08x : size : %d bytes\n", (unsigned int)src, (unsigned int)dest, dmaSize);
		cellDmaLargePut(src, dest, dmaSize, dmaTag, 0, 0);
		cellDmaWaitTagStatusAll(1 << dmaTag);	

// 		int copySize = (p-4096);
// 		for(int i=0;i<copySize;i++)
// 			_array->data[i] = _array->data[4096+i];
		//p -= 4096;
		p=0;
		m_PPUOffset += dmaSize;//16384;
	}
#endif // SN_TARGET_PS3_SPU
}

void Tesselator_SPU::endData()
{
#ifdef SN_TARGET_PS3_SPU
	int dmaSize = p*4;
	unsigned int dmaTag = 2;
	void* src = _array->data;
	uintptr_t dest = (uintptr_t)(((char*)m_PPUArray) + m_PPUOffset);
	if(sc_verbose)
		spu_print("Tesselator : DMA SPU->PPU : 0x%08x -> 0x%08x : size : %d bytes\n", (unsigned int)src, (unsigned int)dest, dmaSize);
	cellDmaLargePut(src, dest, dmaSize, dmaTag, 0, 0);
	cellDmaWaitTagStatusAll(1 << dmaTag);	
	if(sc_verbose)
		spu_print("endData - Tesselator : DMA SPU->PPU complete : %d verts, %d bytes in total\n", vertices, m_PPUOffset+ dmaSize);
 	p=0;
	m_PPUOffset += dmaSize;
#else // SN_TARGET_PS3_SPU
	m_PPUOffset = p*4;
#endif
}

void Tesselator_SPU::beginData()
{
	p = 0;
}
