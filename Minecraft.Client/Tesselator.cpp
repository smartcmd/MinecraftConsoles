#include "stdafx.h"
#include "Tesselator.h"
#include "..\Minecraft.World\BasicTypeContainers.h"
#include "..\Minecraft.World\FloatBuffer.h"
#include "..\Minecraft.World\IntBuffer.h"
#include "..\Minecraft.World\ByteBuffer.h"

bool Tesselator::TRIANGLE_MODE = false;
bool Tesselator::USE_VBO = false;

/* Things to check we are intialising in the constructor...



double u, v;
int col;
int mode;
double xo, yo, zo;
int normal;






*/
DWORD Tesselator::tlsIdx = TlsAlloc();

Tesselator *Tesselator::getInstance()
{
	return (Tesselator *)TlsGetValue(tlsIdx);
}

void Tesselator::CreateNewThreadStorage(int bytes)
{
	Tesselator *instance = new Tesselator(bytes/4);
	TlsSetValue(tlsIdx, instance);
}

Tesselator::Tesselator(int size)
{
	// 4J - this block of things moved to constructor from general initialisations round Java class
	vertices = 0;
	hasColor = false;
	hasTexture = false;
	hasTexture2 = false;
	hasNormal = false;
	p = 0;
	count = 0;
	_noColor = false;
	tesselating = false;
	vboMode = false;
	vboId = 0;
	vboCounts = 10;

	// 4J - adding these things to constructor just to be sure that they are initialised with something
	u = v = 0;
	col = 0;
	mode = 0;
	xo = yo = zo = 0;
	xoo = yoo = zoo = 0;		// 4J added
	_normal = 0;

	useCompactFormat360 = false;	// 4J added
	mipmapEnable = true;			// 4J added
	useProjectedTexturePixelShader = false;	// 4J added

	this->size = size;

	_array = new intArray(size);

	vboMode = USE_VBO;	// 4J removed - && GLContext.getCapabilities().GL_ARB_vertex_buffer_object;
	if (vboMode)
	{
		vboIds = MemoryTracker::createIntBuffer(vboCounts);
		ARBVertexBufferObject::glGenBuffersARB(vboIds);
	}

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Use this to defer primitives that use icons with alpha
	alphaCutOutEnabled = false;

	// this is the cut out enabled vertex array
	_array2 = new intArray(size);
    vertices2 = 0;
    p2 = 0;
#endif
}

Tesselator *Tesselator::getUniqueInstance(int size)
{
    return new Tesselator(size);
}

void Tesselator::end()
{
//    if (!tesselating) throw new IllegalStateException("Not tesselating!");	// 4J - removed
    tesselating = false;
#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Check both counts for valid vertices
	if( vertices > 0 || vertices2 > 0 )
#else
    if (vertices > 0)
#endif
	{
		// 4J - a lot of stuff taken out here for fiddling round with enable client states etc.
		// that don't matter for our renderer
        if (!hasColor)
		{
			// 4J - TEMP put in fixed vertex colors if we don't have any, until we have a shader that can cope without them
			unsigned int *pColData = (unsigned int *)_array->data;
			pColData += 5;
			for( int i = 0; i < vertices; i++ )
			{
				*pColData = 0xffffffff;
				pColData += 8;
			}
#ifdef __PSVITA__
			// AP - alpha cut out is expensive on vita. Check both counts for valid vertices
			pColData = (unsigned int *)_array2->data;
			pColData += 5;
			for( int i = 0; i < vertices2; i++ )
			{
				*pColData = 0xffffffff;
				pColData += 8;
		}
#endif
		}
        if (mode == GL_QUADS && TRIANGLE_MODE)
		{
            // glDrawArrays(GL_TRIANGLES, 0, vertices); // 4J - changed for xbox
#ifdef _XBOX
			RenderManager.DrawVertices(D3DPT_TRIANGLELIST,vertices,_array->data,
									   useCompactFormat360?C4JRender::VERTEX_TYPE_PS3_TS2_CS1:C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
									   useProjectedTexturePixelShader?C4JRender::PIXEL_SHADER_TYPE_PROJECTION:C4JRender::PIXEL_SHADER_TYPE_STANDARD);
#else
			RenderManager.DrawVertices(C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST,vertices,_array->data,
									   useCompactFormat360?C4JRender::VERTEX_TYPE_COMPRESSED:C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1,
									   useProjectedTexturePixelShader?C4JRender::PIXEL_SHADER_TYPE_PROJECTION:C4JRender::PIXEL_SHADER_TYPE_STANDARD);
#endif
        }
		else
		{
//            glDrawArrays(mode, 0, vertices);	// 4J - changed for xbox
			// For compact vertices, the vertexCount has to be calculated from the amount of data written, as
			// we insert extra fake vertices to encode supplementary data for more awkward quads that have non
			// axis aligned UVs (eg flowing lava/water)
#ifdef _XBOX
			int vertexCount = vertices;
			if( useCompactFormat360 )
			{
				vertexCount = p / 2;
				RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PS3_TS2_CS1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
			}
			else
			{
				if( useProjectedTexturePixelShader )
				{
					RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN, C4JRender::PIXEL_SHADER_TYPE_PROJECTION);
				}
				else
				{
					RenderManager.DrawVertices((D3DPRIMITIVETYPE)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
				}
			}
#else
			int vertexCount = vertices;
			if( useCompactFormat360 )
			{
#ifdef __PSVITA__
				// AP - alpha cut out is expensive on vita. Render non-cut out stuff first then send the cut out stuff
				if( vertexCount )
				{
				RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_COMPRESSED, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
				}
				if( vertices2 )
				{
					RenderManager.DrawVerticesCutOut((C4JRender::ePrimitiveType)mode,vertices2,_array2->data,C4JRender::VERTEX_TYPE_COMPRESSED, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
				}
#else

				RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_COMPRESSED, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
#endif
			}
			else
			{
				if( useProjectedTexturePixelShader )
				{
					RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1_TEXGEN, C4JRender::PIXEL_SHADER_TYPE_PROJECTION);
				}
				else
				{
					RenderManager.DrawVertices((C4JRender::ePrimitiveType)mode,vertexCount,_array->data,C4JRender::VERTEX_TYPE_PF3_TF2_CB4_NB4_XW1, C4JRender::PIXEL_SHADER_TYPE_STANDARD);
				}
			}
#endif
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        if (hasTexture) glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (hasColor) glDisableClientState(GL_COLOR_ARRAY);
        if (hasNormal) glDisableClientState(GL_NORMAL_ARRAY);
    }

    clear();
}

void Tesselator::clear()
{
    vertices = 0;

    p = 0;
    count = 0;

#ifdef __PSVITA__
	// AP - alpha cut out is expensive on vita. Clear the cut out variables
    vertices2 = 0;
    p2 = 0;
#endif
}

void Tesselator::begin()
{
    begin(GL_QUADS);
	bounds.reset();	// 4J MGH - added
}
 
void Tesselator::useProjectedTexture(bool enable)
{
	useProjectedTexturePixelShader = enable;
}

void Tesselator::useCompactVertices(bool enable)
{
	useCompactFormat360 = enable;
}

bool Tesselator::getCompactVertices()
{
	return useCompactFormat360;
}

bool Tesselator::setMipmapEnable(bool enable)
{
	bool prev = mipmapEnable;
	mipmapEnable = enable;
	return prev;
}

#ifdef __PSVITA__
// AP - alpha cut out is expensive on vita. Use this to defer primitives that use icons with alpha
void Tesselator::setAlphaCutOut(bool enable)
{
	alphaCutOutEnabled = enable;
}

// AP - was any cut out geometry added since the last call to Clear
bool Tesselator::getCutOutFound()
{
	if( vertices2 )
		return true;

	return false;
}
#endif

void Tesselator::begin(int mode)
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

void Tesselator::tex(float u, float v)
{
    hasTexture = true;
    this->u = u;
    this->v = v;
}

void Tesselator::tex2(int tex2)
{
    hasTexture2 = true;
	this->_tex2 = tex2;
}

void Tesselator::color(float r, float g, float b)
{
    color((int) (r * 255), (int) (g * 255), (int) (b * 255));
}

void Tesselator::color(float r, float g, float b, float a)
{
    color((int) (r * 255), (int) (g * 255), (int) (b * 255), (int) (a * 255));
}

void Tesselator::color(int r, int g, int b)
{
    color(r, g, b, 255);
}

void Tesselator::color(int r, int g, int b, int a)
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

void Tesselator::color(byte r, byte g, byte b)
{
	color(r & 0xff, g & 0xff, b & 0xff);
}

void Tesselator::vertexUV(float x, float y, float z, float u, float v)
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

void Tesselator::packCompactQuad()
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

		p += 4 * 2;
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
		p += 8 * 2;
	}
}

#ifdef __PSVITA__
void Tesselator::tileQuad(float x1, float y1, float z1, float u1, float v1, float r1, float g1, float b1, int tex1,
						  float x2, float y2, float z2, float u2, float v2, float r2, float g2, float b2, int tex2,
						  float x3, float y3, float z3, float u3, float v3, float r3, float g3, float b3, int tex3,
						  float x4, float y4, float z4, float u4, float v4, float r4, float g4, float b4, int tex4
						  )
{
    hasTexture = true;
    hasTexture2 = true;
	hasColor = true;

    count+=4;

	// AP - alpha cut out is expensive on vita. This will choose the correct data buffer depending on cut out enabled
	int16_t* pShortData;
	if( !alphaCutOutEnabled )
	{
		pShortData =  (int16_t*)&_array->data[p];
		p += 16;
		vertices+=4;
	}
	else
	{
		pShortData =  (int16_t*)&_array2->data[p2];
		p2 += 16;
		vertices2+=4;
	}

		int r = ((int) (r1 * 31)) << 11;
		int g = ((int) (g1 * 63)) << 5;
		int b = ((int) (b1 * 31));
		int ipackedcol = r | g | b;
		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

		bounds.addVert(x1+xo, y1+yo, z1+zo);	// 4J MGH - added
		pShortData[0] = (((int)((x1 + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y1 + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z1 + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(u1 * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v1 * 8192.0f))&0xffff);
		((int *)pShortData)[3] = tex1;
		pShortData += 8;

		r = ((int) (r2 * 31)) << 11;
		g = ((int) (g2 * 63)) << 5;
		b = ((int) (b2 * 31));
		ipackedcol = r | g | b;
		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

		bounds.addVert(x2+xo, y2+yo, z2+zo);	// 4J MGH - added
		pShortData[0] = (((int)((x2 + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y2 + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z2 + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(u2 * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v2 * 8192.0f))&0xffff);
		((int *)pShortData)[3] = tex2;
		pShortData += 8;

		r = ((int) (r3 * 31)) << 11;
		g = ((int) (g3 * 63)) << 5;
		b = ((int) (b3 * 31));
		ipackedcol = r | g | b;
		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

		bounds.addVert(x3+xo, y3+yo, z3+zo);	// 4J MGH - added
		pShortData[0] = (((int)((x3 + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y3 + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z3 + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(u3 * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v3 * 8192.0f))&0xffff);
		((int *)pShortData)[3] = tex3;
		pShortData += 8;

		r = ((int) (r4 * 31)) << 11;
		g = ((int) (g4 * 63)) << 5;
		b = ((int) (b4 * 31));
		ipackedcol = r | g | b;
		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

		bounds.addVert(x4+xo, y4+yo, z4+zo);	// 4J MGH - added
		pShortData[0] = (((int)((x4 + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y4 + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z4 + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(u4 * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v4 * 8192.0f))&0xffff);
		((int *)pShortData)[3] = tex4;

	// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
	if ( (!alphaCutOutEnabled && vertices % 4 == 0 && ( ( p >= size - 4 * 4 ) || ( ( p / 4 ) >= 65532 ) )) ||
		(alphaCutOutEnabled && vertices2 % 4 == 0 && ( ( p2 >= size - 4 * 4 ) || ( ( p2 / 4 ) >= 65532 ) )) )
	{
		end();
		tesselating = true;
	}
}

void Tesselator::tileRainQuad(float x1, float y1, float z1, float u1, float v1, 
							float x2, float y2, float z2, float u2, float v2, 
							float x3, float y3, float z3, float u3, float v3, 
							float x4, float y4, float z4, float u4, float v4, 
							float r1, float g1, float b1, float a1,
							float r2, float g2, float b2, float a2,
							int tex1
							)
{
    hasTexture = true;
    hasTexture2 = true;
	hasColor = true;

	float* pfData = (float*)&_array->data[p];

    count+=4;
	p += 4 * 8;
	vertices+=4;

		unsigned int col1 = ((int)(r1*255) << 24) | ((int)(g1*255) << 16) | ((int)(b1*255) << 8) | (int)(a1*255);

		bounds.addVert(x1+xo, y1+yo, z1+zo);
		pfData[0] = (x1 + xo);
		pfData[1] = (y1 + yo);
		pfData[2] = (z1 + zo);
		pfData[3] = u1;
		pfData[4] = v1;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = tex1;
		pfData += 8;

		bounds.addVert(x2+xo, y2+yo, z2+zo);
		pfData[0] = (x2 + xo);
		pfData[1] = (y2 + yo);
		pfData[2] = (z2 + zo);
		pfData[3] = u2;
		pfData[4] = v2;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = tex1;
		pfData += 8;

		col1 = ((int)(r2*255) << 24) | ((int)(g2*255) << 16) | ((int)(b2*255) << 8) | (int)(a2*255);

		bounds.addVert(x3+xo, y3+yo, z3+zo);
		pfData[0] = (x3 + xo);
		pfData[1] = (y3 + yo);
		pfData[2] = (z3 + zo);
		pfData[3] = u3;
		pfData[4] = v3;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = tex1;
		pfData += 8;

		bounds.addVert(x4+xo, y4+yo, z4+zo);
		pfData[0] = (x4 + xo);
		pfData[1] = (y4 + yo);
		pfData[2] = (z4 + zo);
		pfData[3] = u4;
		pfData[4] = v4;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = tex1;
		pfData += 8;

	if (vertices % 4 == 0 && p >= size - 8 * 4)
	{
		end();
		tesselating = true;
	}
}

void Tesselator::tileParticleQuad(float x1, float y1, float z1, float u1, float v1, 
							float x2, float y2, float z2, float u2, float v2, 
							float x3, float y3, float z3, float u3, float v3, 
							float x4, float y4, float z4, float u4, float v4, 
							float r1, float g1, float b1, float a1)
{
    hasTexture = true;
    hasTexture2 = true;
	hasColor = true;

	float* pfData = (float*)&_array->data[p];

    count+=4;
	p += 4 * 8;
	vertices+=4;

		unsigned int col1 = ((int)(r1*255) << 24) | ((int)(g1*255) << 16) | ((int)(b1*255) << 8) | (int)(a1*255);

		bounds.addVert(x1+xo, y1+yo, z1+zo);
		pfData[0] = (x1 + xo);
		pfData[1] = (y1 + yo);
		pfData[2] = (z1 + zo);
		pfData[3] = u1;
		pfData[4] = v1;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = _tex2;
		pfData += 8;

		bounds.addVert(x2+xo, y2+yo, z2+zo);
		pfData[0] = (x2 + xo);
		pfData[1] = (y2 + yo);
		pfData[2] = (z2 + zo);
		pfData[3] = u2;
		pfData[4] = v2;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = _tex2;
		pfData += 8;

		bounds.addVert(x3+xo, y3+yo, z3+zo);
		pfData[0] = (x3 + xo);
		pfData[1] = (y3 + yo);
		pfData[2] = (z3 + zo);
		pfData[3] = u3;
		pfData[4] = v3;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = _tex2;
		pfData += 8;

		bounds.addVert(x4+xo, y4+yo, z4+zo);
		pfData[0] = (x4 + xo);
		pfData[1] = (y4 + yo);
		pfData[2] = (z4 + zo);
		pfData[3] = u4;
		pfData[4] = v4;
		((int*)pfData)[5] = col1;
		((int*)pfData)[7] = _tex2;
		pfData += 8;

	if (vertices % 4 == 0 && p >= size - 8 * 4)
	{
		end();
		tesselating = true;
	}
}
#endif

typedef unsigned short    hfloat;
extern hfloat convertFloatToHFloat(float f);
extern float convertHFloatToFloat(hfloat hf);

void Tesselator::vertex(float x, float y, float z)
{
	bounds.addVert(x+xo, y+yo, z+zo);	// 4J MGH - added
    count++;

	// Signal to pixel shader whether to use mipmapping or not, by putting u into > 1 range if it is to be disabled
	float uu = mipmapEnable ? u : (u + 1.0f);

	// 4J - this format added for 360 to keep memory size of tesselated tiles down -
	// see comments in packCompactQuad() for exact format
	if( useCompactFormat360 )
	{
		unsigned int ucol = (unsigned int)col;

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
		int ipackedcol = ((int)packedcol) & 0xffff;	// 0 to 65535 range

		ipackedcol -= 32768;	// -32768 to 32767 range
		ipackedcol &= 0xffff;

#ifdef __PSVITA__
		// AP - alpha cut out is expensive on vita. This will choose the correct data buffer depending on cut out enabled
		int16_t* pShortData;
		if( !alphaCutOutEnabled )
		{
			pShortData =  (int16_t*)&_array->data[p];
		}
		else
		{
			pShortData =  (int16_t*)&_array2->data[p2];
		}
#else
		int16_t* pShortData =  (int16_t*)&_array->data[p];

#endif



#ifdef __PS3__
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

		pShortData[0] = (((int)((x + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(uu * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v * 8192.0f))&0xffff);
		pShortData[6] = (((int)(tex2U * (8192.0f/256.0f)))&0xffff);
		pShortData[7] = (((int)(tex2V * (8192.0f/256.0f)))&0xffff);

		p += 4;
#else
		pShortData[0] = (((int)((x + xo ) * 1024.0f))&0xffff);
		pShortData[1] = (((int)((y + yo ) * 1024.0f))&0xffff);
		pShortData[2] = (((int)((z + zo ) * 1024.0f))&0xffff);
		pShortData[3] = ipackedcol;
		pShortData[4] = (((int)(uu * 8192.0f))&0xffff);
		pShortData[5] = (((int)(v * 8192.0f))&0xffff);
		int16_t u2 = ((int16_t*)&_tex2)[0];
		int16_t v2 = ((int16_t*)&_tex2)[1];
#if defined _XBOX_ONE || defined __ORBIS__ 
		// Optimisation - pack the second UVs into a single short (they could actually go in a byte), which frees up a short to store the x offset for this chunk in the vertex itself.
		// This means that when rendering chunks, we don't need to update the vertex constants that specify the location for a chunk, when only the x offset has changed.
		pShortData[6] = ( u2 << 8 ) | v2;
		pShortData[7] = -xoo;
#else
		pShortData[6] = u2;
		pShortData[7] = v2;
#endif

#ifdef __PSVITA__
		// AP - alpha cut out is expensive on vita. This will choose the correct data buffer depending on cut out enabled
		if( !alphaCutOutEnabled )
		{
			p += 4;
		}
		else
		{
			p2 += 4;
		}
#else
		p += 4;
#endif

#endif

#endif

#ifdef __PSVITA__
		// AP - alpha cut out is expensive on vita. Increase the correct vertices depending on cut out enabled
		if( !alphaCutOutEnabled )
		{
			vertices++;
		}
		else
		{
			vertices2++;
		}
#else

		vertices++;
#endif

#ifdef _XBOX
		if (vertices % 4 == 0 && ( ( p >= size - 8 * 2 ) || ( ( p / 2 ) >= 65532 ) ) )		// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
#else

#ifdef __PSVITA__
		// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
		if ( (!alphaCutOutEnabled && vertices % 4 == 0 && ( ( p >= size - 4 * 4 ) || ( ( p / 4 ) >= 65532 ) )) ||
			(alphaCutOutEnabled && vertices2 % 4 == 0 && ( ( p2 >= size - 4 * 4 ) || ( ( p2 / 4 ) >= 65532 ) )) )
#else

		if (vertices % 4 == 0 && ( ( p >= size - 4 * 4 ) || ( ( p / 4 ) >= 65532 ) ) )		// Max 65535 verts in D3D, so 65532 is the last point at the end of a quad to catch it
#endif

#endif
		{
			end();
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
				p += 8;
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
		p += 8;

		vertices++;
		if (vertices % 4 == 0 && p >= size - 8 * 4)
		{
			end();
			tesselating = true;
		}
	}
}

void Tesselator::color(int c)
{
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b);
}

void Tesselator::color(int c, int alpha)
{
    int r = ((c >> 16) & 255);
    int g = ((c >> 8) & 255);
    int b = ((c) & 255);
    color(r, g, b, alpha);
}

void Tesselator::noColor()
{
    _noColor = true;
}

#ifdef __PS3__
uint32_t _ConvertF32toX11Y11Z10N(float x, float y, float z)
{
	//                      11111111111 X 0x000007FF
	//           1111111111100000000000 Y 0x003FF800
	// 11111111110000000000000000000000 Z 0xFFC00000
	// ZZZZZZZZZZYYYYYYYYYYYXXXXXXXXXXX
	// #defines for X11Y11Z10N format
#define X11Y11Z10N_X_MASK 0x000007FF
#define X11Y11Z10N_X_BITS 11
#define X11Y11Z10N_X_SHIFT 0

#define X11Y11Z10N_Y_MASK 0x003FF800
#define X11Y11Z10N_Y_BITS 11
#define X11Y11Z10N_Y_SHIFT 11

#define X11Y11Z10N_Z_MASK 0xFFC00000
#define X11Y11Z10N_Z_BITS 10
#define X11Y11Z10N_Z_SHIFT 22

#ifndef _CONTENT_PACKAGE
	if (x<-1.0f || x>1.0f)	{ printf("Value (%5.3f) should be in range [-1..1].  Conversion will clamp to X11Y11Z10N.\n", x); }
	if (y<-1.0f || y>1.0f)	{ printf("Value (%5.3f) should be in range [-1..1].  Conversion will clamp to X11Y11Z10N.\n", y); }
	if (z<-1.0f || z>1.0f)	{ printf("Value (%5.3f) should be in range [-1..1].  Conversion will clamp to X11Y11Z10N.\n", z); }
#endif

	const uint32_t uX = ((int32_t(max(min(((x)*2047.f - 1.f)*0.5f, 1023.f), -1024.f)) & (X11Y11Z10N_X_MASK >> X11Y11Z10N_X_SHIFT)) << X11Y11Z10N_X_SHIFT);
	const uint32_t uY = ((int32_t(max(min(((y)*2047.f - 1.f)*0.5f, 1023.f), -1024.f)) & (X11Y11Z10N_Y_MASK >> X11Y11Z10N_Y_SHIFT)) << X11Y11Z10N_Y_SHIFT);
	const uint32_t uZ = ((int32_t(max(min(((z)*1023.f - 1.f)*0.5f,  511.f), -512.f )) & (X11Y11Z10N_Z_MASK >> X11Y11Z10N_Z_SHIFT)) << X11Y11Z10N_Z_SHIFT);
	const uint32_t xyz = uX | uY | uZ;
	return xyz;
}
#endif // __PS3__

void Tesselator::normal(float x, float y, float z)
{
    hasNormal = true;

#ifdef __PS3__
	_normal = _ConvertF32toX11Y11Z10N(x,y,z);
#elif __PSVITA__
	// AP - casting a negative value to 'byte' on Vita results in zero. changed to a signed 8 value
	int8_t xx = (int8_t) (x * 127);
	int8_t yy = (int8_t) (y * 127);
	int8_t zz = (int8_t) (z * 127);
	_normal = (xx & 0xff) | ((yy & 0xff) << 8) | ((zz & 0xff) << 16);
#else
	byte xx = (byte) (x * 127);
	byte yy = (byte) (y * 127);
	byte zz = (byte) (z * 127);
	_normal = (xx & 0xff) | ((yy & 0xff) << 8) | ((zz & 0xff) << 16);
#endif
}

void Tesselator::offset(float xo, float yo, float zo)
{
    this->xo = xo;
    this->yo = yo;
    this->zo = zo;

	// 4J added
    this->xoo = xo;
    this->yoo = yo;
    this->zoo = zo;
}

void Tesselator::addOffset(float x, float y, float z)
{
    xo += x;
    yo += y;
    zo += z;
}

bool Tesselator::hasMaxVertices()
{
#ifdef __ORBIS__
	// On PS4, the way we push data to the command buffer has a maximum size of a single command packet of 2^16 bytes,
	// and the effective maximum size will be slightly less than that due to packet headers and padding.
	int bytes = vertices * (useCompactFormat360?16:32);

	return bytes > 60 * 1024;
#else
	return false;
#endif
}