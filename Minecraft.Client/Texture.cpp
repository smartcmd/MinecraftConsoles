#include "stdafx.h"
#include "../Minecraft.World/ByteBuffer.h"
#include "Rect2i.h"
#include "TextureManager.h"
#include "Texture.h"

#ifdef __PS3__
#include "PS3/SPU_Tasks/Texture_blit/Texture_blit.h"
#include "C4JSpursJob.h"
static const int sc_maxTextureBlits = 256;
static Texture_blit_DataIn g_textureBlitDataIn[sc_maxTextureBlits] __attribute__((__aligned__(16)));
static int g_currentTexBlit = 0;
C4JSpursJobQueue::Port* g_texBlitJobQueuePort;
// #define DISABLE_SPU_CODE
#endif //__PS3__

#define MAX_MIP_LEVELS 5

Texture::Texture(const wstring &name, int mode, int width, int height, int depth, int wrapMode, int format, int minFilter, int magFilter, bool mipMap)
{
	_init(name, mode, width, height, depth, wrapMode, format, minFilter, magFilter, mipMap);
}

void Texture::_init(const wstring &name, int mode, int width, int height, int depth, int wrapMode, int format, int minFilter, int magFilter, bool mipMap)
{
#ifdef __PS3__
	if(g_texBlitJobQueuePort == NULL)
		g_texBlitJobQueuePort = new C4JSpursJobQueue::Port("C4JSpursJob_Texture_blit");
#endif
	this->name = name;
	this->mode = mode;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->format = format;
	this->minFilter = minFilter;
	this->magFilter = magFilter;
	this->wrapMode = wrapMode;	
	immediateUpdate = false;
	m_bInitialised = false;
	for( int i = 0 ; i < 10; i++ )
	{
		data[i] = NULL;
	}

	rect = new Rect2i(0, 0, width, height);
	// 4J Removed 1D and 3D
	//if (height == 1 && depth == 1)
	//{
	//	type = GL_TEXTURE_1D;
	//}
	//else if(depth == 1)
	//{
		type = GL_TEXTURE_2D;
	//}
	//else
	//{
	//	type = GL_TEXTURE_3D;
	//}

	mipmapped = mipMap || (minFilter != GL_NEAREST && minFilter != GL_LINEAR) ||
		(magFilter != GL_NEAREST && magFilter != GL_LINEAR);
	m_iMipLevels=1;
	
	if(mipmapped)
	{
		// 4J-PB - In the new XDK, the CreateTexture will fail if the number of mipmaps is higher than the width & height passed in will allow!
		int iWidthMips=1;
		int iHeightMips=1;
		while((8<<iWidthMips)<width) iWidthMips++;
		while((8<<iHeightMips)<height) iHeightMips++;

		m_iMipLevels=(iWidthMips<iHeightMips)?iWidthMips:iHeightMips;
	
		// TODO - The render libs currently limit max mip map levels to 5
		if(m_iMipLevels > MAX_MIP_LEVELS) m_iMipLevels = MAX_MIP_LEVELS;
	}

#ifdef __PSVITA__
	// vita doesn't have a mipmap conditional shader because it's too slow so make sure this texture don't look awful at the lower mips
	if( name == L"terrain" )
	{
		m_iMipLevels = 3;
	}
#endif

	if (mode != TM_CONTAINER)
	{
		glId = glGenTextures();

		glBindTexture(type, glId);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode);
	}
	else
	{
		glId = -1;
	}

	managerId = TextureManager::getInstance()->createTextureID();
}

void Texture::_init(const wstring &name, int mode, int width, int height, int depth, int wrapMode, int format, int minFilter, int magFilter, BufferedImage *image, bool mipMap)
{
	_init(name, mode, width, height, depth, wrapMode, format, minFilter, magFilter, mipMap);
	if (image == NULL)
	{
		if (width == -1 || height == -1)
		{
			valid = false;
		}
		else
		{
			byteArray tempBytes = byteArray(width * height * depth * 4);
			for (int index = 0; index < tempBytes.length; index++)
			{
				tempBytes[index] = 0;
			}
#ifdef __PS3__
			data[0] = new ByteBuffer_IO(tempBytes.length);
#else
			data[0] = ByteBuffer::allocateDirect(tempBytes.length);
#endif // __{S3__
			data[0]->clear();
			data[0]->put(tempBytes);
			data[0]->position(0)->limit(tempBytes.length);

			delete [] tempBytes.data;

			if(mipmapped)
			{
				for(unsigned int level = 1; level < m_iMipLevels; ++level)
				{
					int ww = width >> level;
					int hh = height >> height;

					byteArray tempBytes = byteArray(ww * hh * depth * 4);
					for (int index = 0; index < tempBytes.length; index++)
					{
						tempBytes[index] = 0;
					}

#ifdef __PS3__
					data[level] = new ByteBuffer_IO(tempBytes.length);
#else
					data[level] = ByteBuffer::allocateDirect(tempBytes.length);
#endif // __PS3__
					data[level]->clear();
					data[level]->put(tempBytes);
					data[level]->position(0)->limit(tempBytes.length);

					delete [] tempBytes.data;
				}
			}

			if (immediateUpdate)
			{
				updateOnGPU();
			}
			else
			{
				updated = false;
			}
		}
	}
	else
	{
		valid = true;

		transferFromImage(image);

		if (mode != TM_CONTAINER)
		{
			updateOnGPU();
			immediateUpdate = false;
		}
	}
}

Texture::Texture(const wstring &name, int mode, int width, int height, int wrapMode, int format, int minFilter, int magFilter, BufferedImage *image, bool mipMap)
{
	_init(name, mode, width, height, 1, wrapMode, format, minFilter, magFilter, image, mipMap);
}

Texture::Texture(const wstring &name, int mode, int width, int height, int depth, int wrapMode, int format, int minFilter, int magFilter, BufferedImage *image, bool mipMap)
{
	_init(name, mode, width, height, depth, wrapMode, format, minFilter, magFilter, image, mipMap);
}

Texture::~Texture()
{
	delete rect;
	
	for(int i  = 0; i < 10; i++ )
	{
		if(data[i] != NULL) delete data[i];
	}

	if(glId >= 0)
	{
		glDeleteTextures(glId);
	}
}

const Rect2i *Texture::getRect()
{
	return rect;
}

void Texture::fill(const Rect2i *rect, int color)
{
	// 4J Remove 3D
	//if (type == GL_TEXTURE_3D)
	//{
	//	return;
	//}

	Rect2i *myRect = new Rect2i(0, 0, width, height);
	myRect->intersect(rect);
	data[0]->position(0);
	for (int y = myRect->getY(); y < (myRect->getY() + myRect->getHeight()); y++)
	{
		int line = y * width * 4;
		for (int x = myRect->getX(); x < (myRect->getX() + myRect->getWidth()); x++)
		{
			data[0]->put(line + x * 4 + 0, (BYTE)((color >> 24) & 0x000000ff));
			data[0]->put(line + x * 4 + 1, (BYTE)((color >> 16) & 0x000000ff));
			data[0]->put(line + x * 4 + 2, (BYTE)((color >>  8) & 0x000000ff));
			data[0]->put(line + x * 4 + 3, (BYTE)((color >>  0) & 0x000000ff));
		}
	}
	delete myRect;

	if (immediateUpdate)
	{
		updateOnGPU();
	}
	else
	{
		updated = false;
	}
}

void Texture::writeAsBMP(const wstring &name)
{
	// 4J Don't need
#if 0
	if (type == GL_TEXTURE_3D)
	{
		return;
	}

	File *outFile = new File(name);
	if (outFile.exists())
	{
		outFile.delete();
	}

	DataOutputStream *outStream = NULL;
	//try {
	outStream = new DataOutputStream(new FileOutputStream(outFile));
	//} catch (IOException e) {
	// Unable to open file for writing for some reason
	//	return;
	//}

	//try {
	// Write the header
	outStream->writeShort((short)0x424d);            // 0x0000: ID - 'BM'
	int byteSize = width * height * 4 + 54;
	outStream->writeByte((byte)(byteSize >>  0));    // 0x0002: Raw file size
	outStream->writeByte((byte)(byteSize >>  8));
	outStream->writeByte((byte)(byteSize >> 16));
	outStream->writeByte((byte)(byteSize >> 24));
	outStream->writeInt(0);                          // 0x0006: Reserved
	outStream->writeByte(54);                        // 0x000A: Start of pixel data
	outStream->writeByte(0);
	outStream->writeByte(0);
	outStream->writeByte(0);
	outStream->writeByte(40);                        // 0x000E: Size of secondary header
	outStream->writeByte(0);
	outStream->writeByte(0);
	outStream->writeByte(0);
	outStream->writeByte((byte)(width >>  0));       // 0x0012: Image width, in pixels
	outStream->writeByte((byte)(width >>  8));
	outStream->writeByte((byte)(width >> 16));
	outStream->writeByte((byte)(width >> 24));
	outStream->writeByte((byte)(height >>  0));      // 0x0016: Image height, in pixels
	outStream->writeByte((byte)(height >>  8));
	outStream->writeByte((byte)(height >> 16));
	outStream->writeByte((byte)(height >> 24));
	outStream->writeByte(1);                         // 0x001A: Number of color planes, must be 1
	outStream->writeByte(0);
	outStream->writeByte(32);                        // 0x001C: Bit depth (32bpp)
	outStream->writeByte(0);
	outStream->writeInt(0);                          // 0x001E: Compression mode (BI_RGB, uncompressed)
	int bufSize = width * height * 4;
	outStream->writeInt((byte)(bufSize >>  0));      // 0x0022: Raw size of bitmap data
	outStream->writeInt((byte)(bufSize >>  8));
	outStream->writeInt((byte)(bufSize >> 16));
	outStream->writeInt((byte)(bufSize >> 24));
	outStream->writeInt(0);                          // 0x0026: Horizontal resolution in ppm
	outStream->writeInt(0);                          // 0x002A: Vertical resolution in ppm
	outStream->writeInt(0);                          // 0x002E: Palette size (0 to match bit depth)
	outStream->writeInt(0);                          // 0x0032: Number of important colors, 0 for all

	// Pixels follow in inverted Y order
	byte[] bytes = new byte[width * height * 4];
	data.position(0);
	data.get(bytes);
	for (int y = height - 1; y >= 0; y--)
	{
		int line = y * width * 4;
		for (int x = 0; x < width; x++)
		{
			outStream->writeByte(bytes[line + x * 4 + 2]);
			outStream->writeByte(bytes[line + x * 4 + 1]);
			outStream->writeByte(bytes[line + x * 4 + 0]);
			outStream->writeByte(bytes[line + x * 4 + 3]);
		}
	}

	outStream->close();
	//} catch (IOException e) {
	// Unable to write to the file for some reason
	//	return;
	//}
#endif
}

void Texture::writeAsPNG(const wstring &filename)
{
	// 4J Don't need
#if 0
	BufferedImage *image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
	ByteBuffer *buffer = this->getData();
	byte[] bytes = new byte[width * height * 4];

	buffer.position(0);
	buffer.get(bytes);

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int pos = (y * width * 4) + x * 4;
			int col = 0;

			col |= (bytes[pos + 2] & 0xFF) << 0;
			col |= (bytes[pos + 1] & 0xFF) << 8;
			col |= (bytes[pos + 0] & 0xFF) << 16;
			col |= (bytes[pos + 3] & 0xFF) << 24;

			image.setRGB(x, y, col);
		}
	}

	data.position(width * height * 4);

	//try {
	ImageIO::write(image, L"png", new File(Minecraft.getWorkingDirectory(), filename));
	//} catch (IOException e) {
	//	e.printStackTrace();
	//}
#endif
}

void Texture::blit(int x, int y, Texture *source)
{
	blit(x, y, source, false);
}

void Texture::blit(int x, int y, Texture *source, bool rotated)
{
	// 4J Remove 3D
	//if (type == GL_TEXTURE_3D)
	//{
	//	return;
	//}

	for(unsigned int level = 0; level < m_iMipLevels; ++level)
	{
		ByteBuffer *srcBuffer = source->getData(level);

		if(srcBuffer == NULL) break;

		int yy = y >> level;
		int xx = x >> level;
		int hh = height >> level;
		int ww = width >> level;
		int shh = source->getHeight() >> level;
		int sww = source->getWidth() >> level;

		data[level]->position(0);
		srcBuffer->position(0);

#if defined __PS3__ && !defined DISABLE_SPU_CODE
		if(g_texBlitJobQueuePort->hasCompleted())
		{
			// all outstanding blits have completed, so reset to the start of the blit list
			g_currentTexBlit = 0;
		}
		Texture_blit_DataIn& dataIn = g_textureBlitDataIn[g_currentTexBlit];
		g_currentTexBlit++;
		if(g_currentTexBlit >= sc_maxTextureBlits)
		{
			app.DebugPrintf("ran out of tex blit slots, stalling for completion\n");
			g_texBlitJobQueuePort->waitForCompletion();
			g_currentTexBlit = 0;
		}
		dataIn.pSrcData = srcBuffer->getBuffer();
		dataIn.pDstData = data[level]->getBuffer();
		dataIn.yy = yy;
		dataIn.xx = xx;
		dataIn.hh = hh;
		dataIn.ww = ww;
		dataIn.shh = shh;
		dataIn.sww = sww;
		dataIn.rotated = rotated;

		C4JSpursJob_Texture_blit blitJob(&dataIn);
		g_texBlitJobQueuePort->submitJob(&blitJob);
// 		p.waitForCompletion();

#elif __PSVITA__
		unsigned int *src = (unsigned int *) srcBuffer->getBuffer();
		unsigned int *dst = (unsigned int *) data[level]->getBuffer();

		for (int srcY = 0; srcY < shh; srcY++)
		{
			int dstY = yy + srcY;
			int srcLine = srcY * sww;
			int dstLine = dstY * ww;

			if (rotated)
			{
				dstY = yy + (shh - srcY);
			}

			if (!rotated)
			{
				memcpy(dst + dstLine + xx, src + srcLine, sww * 4);
			}
			else
			{
				for (int srcX = 0; srcX < sww; srcX++)
				{
					int dstPos = dstLine + (srcX + xx);
					int srcPos = srcLine + srcX;

					if (rotated)
					{
						dstPos = (xx + srcX * ww) + dstY;
					}

					dst[dstPos] = src[srcPos];
				}
			}
		}
#else

		for (int srcY = 0; srcY < shh; srcY++)
		{
			int dstY = yy + srcY;
			int srcLine = srcY * sww * 4;
			int dstLine = dstY * ww * 4;

			if (rotated)
			{
				dstY = yy + (shh - srcY);
			}

			for (int srcX = 0; srcX < sww; srcX++)
			{
				int dstPos = dstLine + (srcX + xx) * 4;
				int srcPos = srcLine + srcX * 4;

				if (rotated)
				{
					dstPos = (xx + srcX * ww * 4) + dstY * 4;
				}

				data[level]->put(dstPos + 0, srcBuffer->get(srcPos + 0));
				data[level]->put(dstPos + 1, srcBuffer->get(srcPos + 1));
				data[level]->put(dstPos + 2, srcBuffer->get(srcPos + 2));
				data[level]->put(dstPos + 3, srcBuffer->get(srcPos + 3));
			}
		}
		// Don't delete this, as it belongs to the source texture
		//delete srcBuffer;
#endif
		data[level]->position(ww * hh * 4);
	}

	if (immediateUpdate)
	{
		updateOnGPU();
	}
	else
	{
		updated = false;
	}
}

void Texture::transferFromBuffer(intArray buffer)
{
	if (depth == 1)
	{
		return;
	}

// #ifdef __PS3__
// 	int byteRemapRGBA[] = { 3, 0, 1, 2 };
// 	int byteRemapBGRA[] = { 3, 2, 1, 0 };
// #else
	int byteRemapRGBA[] = { 0, 1, 2, 3 };
	int byteRemapBGRA[] = { 2, 1, 0, 3 };
// #endif
	int *byteRemap = ((format == TFMT_BGRA) ? byteRemapBGRA : byteRemapRGBA);

	for (int z = 0; z < depth; z++)
	{
		int plane = z * height * width * 4;
		for (int y = 0; y < height; y++)
		{
			int column = plane + y * width * 4;
			for (int x = 0; x < width; x++)
			{
				int texel = column + x * 4;
				data[0]->position(0);
				data[0]->put(texel + byteRemap[0], (byte)((buffer[texel >> 2] >> 24) & 0xff));
				data[0]->put(texel + byteRemap[1], (byte)((buffer[texel >> 2] >> 16) & 0xff));
				data[0]->put(texel + byteRemap[2], (byte)((buffer[texel >> 2] >>  8) & 0xff));
				data[0]->put(texel + byteRemap[3], (byte)((buffer[texel >> 2] >>  0) & 0xff));
			}
		}
	}

	data[0]->position(width * height * depth * 4);

	updateOnGPU();
}

void Texture::transferFromImage(BufferedImage *image)
{
	// 4J Remove 3D
	//if (type == GL_TEXTURE_3D)
	//{
	//	return;
	//}

	int imgWidth = image->getWidth();
	int imgHeight = image->getHeight();
	if (imgWidth > width || imgHeight > height)
	{
		//Minecraft::GetInstance().getLogger().warning("transferFromImage called with a BufferedImage with dimensions (" +
		//	imgWidth + ", " + imgHeight + ") larger than the Texture dimensions (" + width +
		//	", " + height + "). Ignoring.");
		app.DebugPrintf("transferFromImage called with a BufferedImage with dimensions (%d, %d) larger than the Texture dimensions (%d, %d). Ignoring.\n", imgWidth, imgHeight, width, height);
		return;
	}

// #ifdef __PS3__
// 	int byteRemapRGBA[] = { 0, 1, 2, 3 };
// 	int byteRemapBGRA[] = { 2, 1, 0, 3 };
// #else
#ifdef _XBOX
	int byteRemapRGBA[] = { 0, 1, 2, 3 };
#else
	int byteRemapRGBA[] = { 3, 0, 1, 2 };
#endif
	int byteRemapBGRA[] = { 3, 2, 1, 0 };
// #endif
	int *byteRemap = ((format == TFMT_BGRA) ? byteRemapBGRA : byteRemapRGBA);

	intArray tempPixels = intArray(width * height);
	int transparency = image->getTransparency();
	image->getRGB(0, 0, width, height, tempPixels, 0, imgWidth);

	byteArray tempBytes = byteArray(width * height * 4);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int intIndex = y * width + x;
			int byteIndex = intIndex * 4;

			// Pull ARGB bytes into either RGBA or BGRA depending on format

			tempBytes[byteIndex + byteRemap[0]] = (byte)((tempPixels[intIndex] >> 24) & 0xff);
			tempBytes[byteIndex + byteRemap[1]] = (byte)((tempPixels[intIndex] >> 16) & 0xff);
			tempBytes[byteIndex + byteRemap[2]] = (byte)((tempPixels[intIndex] >>  8) & 0xff);
			tempBytes[byteIndex + byteRemap[3]] = (byte)((tempPixels[intIndex] >>  0) & 0xff);
		}
	}

	for(int i  = 0; i < 10; i++ )
	{
		if(data[i] != NULL)
		{
			delete data[i];
			data[i] = NULL;
		}
	}

	MemSect(51);
#ifdef __PS3__
	data[0] = new ByteBuffer_IO(tempBytes.length);
#else
	data[0] = ByteBuffer::allocateDirect(tempBytes.length);
#endif // __{S3__
	MemSect(0);
	data[0]->clear();
	data[0]->put(tempBytes);
	data[0]->limit(tempBytes.length);

	delete [] tempBytes.data;

	if(mipmapped || image->getData(1) != NULL)
	{
		mipmapped = true;
		for(unsigned int level = 1; level < MAX_MIP_LEVELS; ++level)
		{
			int ww = width >> level;
			int hh = height >> level;

			byteArray tempBytes = byteArray(ww * hh * 4);
			unsigned int *tempData = new unsigned int[ww * hh];

			if( image->getData( level ) )
			{
				memcpy( tempData, image->getData( level ), ww * hh * 4);
				for (int y = 0; y < hh; y++)
				{
					for (int x = 0; x < ww; x++)
					{
						int intIndex = y * ww + x;
						int byteIndex = intIndex * 4;

						// Pull ARGB bytes into either RGBA or BGRA depending on format

						tempBytes[byteIndex + byteRemap[0]] = (byte)((tempData[intIndex] >> 24) & 0xff);
						tempBytes[byteIndex + byteRemap[1]] = (byte)((tempData[intIndex] >> 16) & 0xff);
						tempBytes[byteIndex + byteRemap[2]] = (byte)((tempData[intIndex] >>  8) & 0xff);
						tempBytes[byteIndex + byteRemap[3]] = (byte)((tempData[intIndex] >>  0) & 0xff);
					}
				}
			}
			else
			{
				int ow = width >> (level - 1);
				
				for (int x = 0; x < ww; x++)
					for (int y = 0; y < hh; y++)
					{						
						int c0 = data[level - 1]->getInt(((x * 2 + 0) + (y * 2 + 0) * ow) * 4);
						int c1 = data[level - 1]->getInt(((x * 2 + 1) + (y * 2 + 0) * ow) * 4);
						int c2 = data[level - 1]->getInt(((x * 2 + 1) + (y * 2 + 1) * ow) * 4);
						int c3 = data[level - 1]->getInt(((x * 2 + 0) + (y * 2 + 1) * ow) * 4);
#ifndef _XBOX
						// 4J - convert our RGBA texels to ARGB that crispBlend is expecting
						c0 = ( ( c0 >> 8 ) & 0x00ffffff ) | ( c0 << 24 );
						c1 = ( ( c1 >> 8 ) & 0x00ffffff ) | ( c1 << 24 );
						c2 = ( ( c2 >> 8 ) & 0x00ffffff ) | ( c2 << 24 );
						c3 = ( ( c3 >> 8 ) & 0x00ffffff ) | ( c3 << 24 );
#endif
						int col = crispBlend(crispBlend(c0, c1), crispBlend(c2, c3));
						// 4J - and back from ARGB -> RGBA
						//col = ( col << 8 ) | (( col >> 24 ) & 0xff);
						//tempData[x + y * ww] = col;

						int intIndex = y * ww + x;
						int byteIndex = intIndex * 4;

						// Pull ARGB bytes into either RGBA or BGRA depending on format

						tempBytes[byteIndex + byteRemap[0]] = (byte)((col >> 24) & 0xff);
						tempBytes[byteIndex + byteRemap[1]] = (byte)((col >> 16) & 0xff);
						tempBytes[byteIndex + byteRemap[2]] = (byte)((col >>  8) & 0xff);
						tempBytes[byteIndex + byteRemap[3]] = (byte)((col >>  0) & 0xff);
					}
			}

			MemSect(51);
#ifdef __PS3__
			data[level] = new ByteBuffer_IO(tempBytes.length);
#else
			data[level] = ByteBuffer::allocateDirect(tempBytes.length);
#endif // __{S3__
			MemSect(0);
			data[level]->clear();
			data[level]->put(tempBytes);
			data[level]->limit(tempBytes.length);
			delete [] tempBytes.data;
			delete [] tempData;
		}
	}	

	delete [] tempPixels.data;

	if (immediateUpdate)
	{
		updateOnGPU();
	}
	else
	{
		updated = false;
	}
}

// 4J Kept from older versions for where we create mip-maps for levels that do not have pre-made graphics
int Texture::crispBlend(int c0, int c1)
{
	int a0 = (int) (((c0 & 0xff000000) >> 24)) & 0xff;
	int a1 = (int) (((c1 & 0xff000000) >> 24)) & 0xff;

	int a = 255;
	if (a0 + a1 < 255)
	{
		a = 0;
		a0 = 1;
		a1 = 1;
	}
	else if (a0 > a1)
	{
		a0 = 255;
		a1 = 1;
	}
	else
	{
		a0 = 1;
		a1 = 255;

	}

	int r0 = ((c0 >> 16) & 0xff) * a0;
	int g0 = ((c0 >> 8) & 0xff) * a0;
	int b0 = ((c0) & 0xff) * a0;

	int r1 = ((c1 >> 16) & 0xff) * a1;
	int g1 = ((c1 >> 8) & 0xff) * a1;
	int b1 = ((c1) & 0xff) * a1;

	int r = (r0 + r1) / (a0 + a1);
	int g = (g0 + g1) / (a0 + a1);
	int b = (b0 + b1) / (a0 + a1);

	return (a << 24) | (r << 16) | (g << 8) | b;
}

int Texture::getManagerId()
{
	return managerId;
}

int Texture::getGlId()
{
	return glId;
}

int Texture::getWidth()
{
	return width;
}

int Texture::getHeight()
{
	return height;
}

wstring Texture::getName()
{
	return name;
}

void Texture::setImmediateUpdate(bool immediateUpdate)
{
	this->immediateUpdate = immediateUpdate;
}

void Texture::bind(int mipMapIndex)
{
	// 4J Removed 3D
	//if (depth == 1)
	//{
		glEnable(GL_TEXTURE_2D);
	//}
	//else
	//{
	//	glEnable(GL_TEXTURE_3D);
	//}

	glActiveTexture(GL_TEXTURE0 + mipMapIndex);
	glBindTexture(type, glId);
	if (!updated)
	{
		updateOnGPU();
	}
}

void Texture::updateOnGPU()
{
	data[0]->flip();
	if(mipmapped)
	{
		for (int level = 1; level < m_iMipLevels; level++)
		{
			if(data[level] == NULL) break;
		
			data[level]->flip();
		}
	}
	// 4J remove 3D and 1D
	//if (height != 1 && depth != 1)
	//{
	//	glTexImage3D(type, 0, format, width, height, depth, 0, format, GL_UNSIGNED_BYTE, data);
	//}
	//else if(height != 1)
	//{
		// 4J Added check so we can differentiate between which RenderManager function to call
		if(!m_bInitialised)
		{			
			RenderManager.TextureSetTextureLevels(m_iMipLevels);	// 4J added

#ifdef __PSVITA__
			// AP - replace the dynamic ram buffer to one that points to a newly allocated video ram texture buffer. This means we don't have to memcpy
			// the ram based buffer to it any more inside RenderManager.TextureDataUpdate
			unsigned char *newData = RenderManager.TextureData(width,height,data[0]->getBuffer(),0,C4JRender::TEXTURE_FORMAT_RxGyBzAw);
			ByteBuffer *oldBuffer = data[0];
			data[0] = new ByteBuffer(data[0]->getSize(), (byte*) newData);
			delete oldBuffer;
			newData += width * height * 4;
#else
			RenderManager.TextureData(width,height,data[0]->getBuffer(),0,C4JRender::TEXTURE_FORMAT_RxGyBzAw);
#endif

			if(mipmapped)
			{
				for (int level = 1; level < m_iMipLevels; level++)
				{
					int levelWidth = width >> level;
					int levelHeight = height >> level;

#ifdef __PSVITA__
					// AP - replace the dynamic ram buffer to one that points to a newly allocated video ram texture buffer. This means we don't have to memcpy
					// the ram based buffer to it any more inside RenderManager.TextureDataUpdate
					RenderManager.TextureDataUpdate(0, 0,levelWidth,levelHeight,data[level]->getBuffer(),level);
					ByteBuffer *oldBuffer = data[level];
					data[level] = new ByteBuffer(data[level]->getSize(), (byte*) newData);
					delete oldBuffer;
					newData += levelWidth * levelHeight * 4;
#else
					RenderManager.TextureData(levelWidth,levelHeight,data[level]->getBuffer(),level,C4JRender::TEXTURE_FORMAT_RxGyBzAw);
#endif
				}
			}

			m_bInitialised = true;
		}
		else
		{
#ifdef _XBOX
			RenderManager.TextureDataUpdate(data[0]->getBuffer(),0);
#else
			RenderManager.TextureDataUpdate(0, 0,width,height,data[0]->getBuffer(),0);
#endif

			if(mipmapped)
			{
				if (RenderManager.TextureGetTextureLevels() > 1)
				{
					for (int level = 1; level < m_iMipLevels; level++)
					{
						int levelWidth = width >> level;
						int levelHeight = height >> level;
					
#ifdef _XBOX
						RenderManager.TextureDataUpdate(data[level]->getBuffer(),level);
#else
						RenderManager.TextureDataUpdate(0, 0,levelWidth,levelHeight,data[level]->getBuffer(),level);
#endif
					}
				}
			}
		}
		//glTexImage2D(type, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	//}
	//else
	//{
	//	glTexImage1D(type, 0, format, width, 0, format, GL_UNSIGNED_BYTE, data);
	//}
	updated = true;
}

ByteBuffer *Texture::getData(unsigned int level)
{
	return data[level];
}
