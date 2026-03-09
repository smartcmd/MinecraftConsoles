#include "stdafx.h"
#include "net.minecraft.world.level.biome.h"
#include "net.minecraft.world.level.newbiome.layer.h"
#include "net.minecraft.world.level.h"
#include "BiomeOverrideLayer.h"


BiomeOverrideLayer::BiomeOverrideLayer(int seedMixup) : Layer(seedMixup)
{
	m_biomeOverride = byteArray( width * height );

#ifdef _UNICODE
	wstring path = L"GAME:\\GameRules\\biomemap.bin";
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
#ifdef _WINDOWS64
	string path = "GameRules\\biomemap.bin";
#else
	string path = "GAME:\\GameRules\\biomemap.bin";
#endif
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD error = GetLastError();
		//assert(false);
		app.DebugPrintf("Biome override not found, using plains as default\n");

		memset(m_biomeOverride.data,Biome::plains->id,m_biomeOverride.length);
	}
	else
	{

#ifdef _DURANGO
		__debugbreak();	// TODO
		DWORD bytesRead,dwFileSize = 0;
#else
		DWORD bytesRead,dwFileSize = GetFileSize(file,nullptr);
#endif
		if(dwFileSize > m_biomeOverride.length)
		{
			app.DebugPrintf("Biomemap binary is too large!!\n");
			__debugbreak();
		}
		BOOL bSuccess = ReadFile(file,m_biomeOverride.data,dwFileSize,&bytesRead,nullptr);

		if(bSuccess==FALSE)
		{
			app.FatalLoadError();
		}

		CloseHandle(file);
	}
}

intArray BiomeOverrideLayer::getArea(int xo, int yo, int w, int h)
{
	intArray result = IntCache::allocate(w * h);

	int xOrigin = xo + width/2;
	int yOrigin = yo + height/2;
	if(xOrigin < 0 ) xOrigin = 0;
	if(xOrigin >= width) xOrigin = width - 1;
	if(yOrigin < 0 ) yOrigin = 0;
	if(yOrigin >= height) yOrigin = height - 1;
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			int curX = xOrigin + x;
			int curY = yOrigin + y;
			if(curX >= width) curX = width - 1;
			if(curY >= height) curY = height - 1;
			int index = curX + curY * width;

			unsigned char headerValue = m_biomeOverride[index];
			result[x + y * w] = headerValue;
		}
	}
	return result;
}