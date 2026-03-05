#include "stdafx.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "Textures.h"
#include "..\Minecraft.World\ArrayWithLength.h"
#include "BufferedImage.h"

#ifdef _XBOX
typedef struct
{
	unsigned int   filesz;
	unsigned short creator1;
	unsigned short creator2;
	unsigned int	 bmp_offset;
	unsigned int	 header_sz;
	unsigned int	 width;
	unsigned int	 height;
	unsigned short nplanes;
	unsigned short bitspp;
	unsigned int	 compress_type;
	unsigned int	 bmp_bytesz;
	int			 hres;
	int			 vres;
	unsigned int	 ncolors;
	unsigned int	 nimpcolors;
} BITMAPINFOHEADER;
#endif

BufferedImage::BufferedImage(int width,int height,int type)
{
	data[0] = new int[width*height];

	for( int i = 1 ; i < 10; i++ )
	{
		data[i] = NULL;
	}
	this->width = width;
	this->height = height;
}

void BufferedImage::ByteFlip4(unsigned int &data)
{
	data = ( data >> 24 ) |
		( ( data >> 8 ) & 0x0000ff00 ) |
		( ( data << 8 ) & 0x00ff0000 ) |
		( data << 24 );
}
// Loads a bitmap into a buffered image - only currently supports the 2 types of 32-bit image that we've made so far
// and determines which of these is which by the compression method. Compression method 3 is a 32-bit image with only
// 24-bits used (ie no alpha channel) whereas method 0 is a full 32-bit image with a valid alpha channel.
BufferedImage::BufferedImage(const wstring& File, bool filenameHasExtension /*=false*/, bool bTitleUpdateTexture /*=false*/, const wstring &drive /*=L""*/)
{
	HRESULT hr;
	wstring wDrive;
	wstring filePath;
	filePath = File;

	wDrive = drive;
	if(wDrive.empty())
	{
#ifdef _XBOX
		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
#ifdef _TU_BUILD
			wDrive=L"UPDATE:\\";
#else

			wDrive=L"GAME:\\res\\TitleUpdate\\";
#endif
		}
		else
		{
			wDrive=L"GAME:\\";
		}
#else

#ifdef __PS3__

		char *pchUsrDir;
		if(app.GetBootedFromDiscPatch())
		{
			const char *pchTextureName=wstringtofilename(File);
			pchUsrDir = app.GetBDUsrDirPath(pchTextureName);
		}
		else
		{
			pchUsrDir=getUsrDirPath();
		}

		wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
			wDrive= wstr + L"\\Common\\res\\TitleUpdate\\";
		}
		else
		{
			wDrive= wstr + L"/Common/";
		}
#elif __PSVITA__

		/*char *pchUsrDir=getUsrDirPath();

		wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
			wDrive= wstr + L"\\Common\\res\\TitleUpdate\\";
		}
		else
		{
			wDrive= wstr + L"/Common/";
		}*/

		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
			wDrive= L"Common\\res\\TitleUpdate\\";
		}
		else
		{
			wDrive= L"Common/";
		}
#else
		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
			wDrive= L"Common\\res\\TitleUpdate\\";
		}
		else
		{
			wDrive= L"Common/";
		}
#endif

#endif
	}

	for( int l = 0 ; l < 10; l++ )
	{
		data[l] = NULL;
	}

	for( int l = 0; l < 10; l++ )
	{
		wstring name;
		wstring mipMapPath = L"";
		if( l != 0 )
		{
			mipMapPath = L"MipMapLevel" + std::to_wstring(l+1);
		}
		if( filenameHasExtension )
		{
			name = wDrive + L"res" + filePath.substr(0,filePath.length());
		}
		else
		{
			name = wDrive + L"res" + filePath.substr(0,filePath.length()-4) + mipMapPath + L".png";
		}

		const char *pchTextureName=wstringtofilename(name);

#ifndef _CONTENT_PACKAGE
		app.DebugPrintf("\n--- Loading TEXTURE - %s\n\n",pchTextureName);
#endif

		D3DXIMAGE_INFO ImageInfo;
		ZeroMemory(&ImageInfo,sizeof(D3DXIMAGE_INFO));
		hr=RenderManager.LoadTextureData(pchTextureName,&ImageInfo,&data[l]);


		if(hr!=ERROR_SUCCESS)
		{
			// 4J - If we haven't loaded the non-mipmap version then exit the game
			if( l == 0 )
			{
				app.FatalLoadError();
			}
			return;
		}

		if( l == 0 )
		{
			width=ImageInfo.Width;
			height=ImageInfo.Height;
		}
	}
}

BufferedImage::BufferedImage(DLCPack *dlcPack, const wstring& File, bool filenameHasExtension /*= false*/ )
{
	HRESULT hr;
	wstring filePath = File;
	BYTE *pbData = NULL;
	DWORD dwBytes = 0;

	for( int l = 0 ; l < 10; l++ )
	{
		data[l] = NULL;
	}

	for( int l = 0; l < 10; l++ )
	{
		wstring name;
		wstring mipMapPath = L"";
		if( l != 0 )
		{
			mipMapPath = L"MipMapLevel" + std::to_wstring(l+1);
		}
		if( filenameHasExtension )
		{
			name = L"res" + filePath.substr(0,filePath.length());
		}
		else
		{
			name = L"res" + filePath.substr(0,filePath.length()-4) + mipMapPath + L".png";
		}

		if(!dlcPack->doesPackContainFile(DLCManager::e_DLCType_All, name))
		{
			// 4J - If we haven't loaded the non-mipmap version then exit the game
			if( l == 0 )
			{
				app.FatalLoadError();
			}
			return;
		}

		DLCFile *dlcFile = dlcPack->getFile(DLCManager::e_DLCType_All, name);
		pbData = dlcFile->getData(dwBytes);
		if(pbData == NULL || dwBytes == 0)
		{
			// 4J - If we haven't loaded the non-mipmap version then exit the game
			if( l == 0 )
			{
				app.FatalLoadError();
			}
			return;
		}

		D3DXIMAGE_INFO ImageInfo;
		ZeroMemory(&ImageInfo,sizeof(D3DXIMAGE_INFO));
		hr=RenderManager.LoadTextureData(pbData,dwBytes,&ImageInfo,&data[l]);


		if(hr!=ERROR_SUCCESS)
		{
			// 4J - If we haven't loaded the non-mipmap version then exit the game
			if( l == 0 )
			{
				app.FatalLoadError();
			}
			return;
		}

		if( l == 0 )
		{
			width=ImageInfo.Width;
			height=ImageInfo.Height;
		}
	}
}


BufferedImage::BufferedImage(BYTE *pbData, DWORD dwBytes)
{
	int iCurrentByte=0;
	for( int l = 0 ; l < 10; l++ )
	{
		data[l] = NULL;
	}

	D3DXIMAGE_INFO ImageInfo;
	ZeroMemory(&ImageInfo,sizeof(D3DXIMAGE_INFO));
	HRESULT hr=RenderManager.LoadTextureData(pbData,dwBytes,&ImageInfo,&data[0]);

	if(hr==ERROR_SUCCESS)
	{
		width=ImageInfo.Width;
		height=ImageInfo.Height;
	}
	else
	{
		app.FatalLoadError();
	}
}

BufferedImage::~BufferedImage()
{
	for(int i  = 0; i < 10; i++ )
	{
		delete[] data[i];
	}
}

int BufferedImage::getWidth()
{
	return width;
}

int BufferedImage::getHeight()
{
	return height;
}

void BufferedImage::getRGB(int startX, int startY, int w, int h, intArray out,int offset,int scansize, int level)
{
	int ww = width >> level;
	for( int y = 0; y < h; y++ )
	{
		for( int x = 0; x < w; x++ )
		{
			out[ y * scansize + offset + x] = data[level][ startX + x + ww * ( startY + y ) ];
		}
	}
}

int *BufferedImage::getData()
{
	return data[0];
}

int *BufferedImage::getData(int level)
{
	return data[level];
}

Graphics *BufferedImage::getGraphics()
{
	return NULL;
}

//Returns the transparency. Returns either OPAQUE, BITMASK, or TRANSLUCENT.
//Specified by:
//getTransparency in interface Transparency
//Returns:
//the transparency of this BufferedImage.
int BufferedImage::getTransparency()
{
	// TODO - 4J Implement?
	return 0;
}

//Returns a subimage defined by a specified rectangular region. The returned BufferedImage shares the same data array as the original image.
//Parameters:
//x, y - the coordinates of the upper-left corner of the specified rectangular region
//w - the width of the specified rectangular region
//h - the height of the specified rectangular region
//Returns:
//a BufferedImage that is the subimage of this BufferedImage.
BufferedImage *BufferedImage::getSubimage(int x ,int y, int w, int h)
{
	// TODO - 4J Implement

	BufferedImage *img = new BufferedImage(w,h,0);
	intArray arrayWrapper(img->data[0], w*h);
	this->getRGB(x, y, w, h, arrayWrapper,0,w);

	int level = 1;
	while(getData(level) != NULL)
	{
		int ww = w >> level;
		int hh = h >> level;
		int xx = x >> level;
		int yy = y >> level;
		img->data[level] = new int[ww*hh];
		intArray arrayWrapper(img->data[level], ww*hh);
		this->getRGB(xx, yy, ww, hh, arrayWrapper,0,ww,level);

		++level;
	}

	return img;
}


void BufferedImage::preMultiplyAlpha()
{
	int *curData = data[0];

	int cur = 0;
	int alpha = 0;
	int r = 0;
	int g = 0;
	int b = 0;

	int total = width * height;
	for(unsigned int i = 0; i < total; ++i)
	{
		cur = curData[i];
		alpha = (cur >> 24) & 0xff;
		r = ((cur >> 16) & 0xff) * (float)alpha/255;
		g = ((cur >> 8) & 0xff) * (float)alpha/255;
		b = (cur & 0xff) * (float)alpha/255;

		curData[i] = (r << 16) | (g << 8) | (b ) | (alpha << 24);
	}
}
