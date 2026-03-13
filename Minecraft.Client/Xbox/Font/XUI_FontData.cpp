#include "stdafx.h"
#include "../../stubs.h"
#include "../../Minecraft.h"
#include "../../Textures.h"
#include "XUI_FontData.h"
#include "../../../Minecraft.World/StringHelpers.h"


#define USE_NEW 0


extern IDirect3DDevice9 *g_pD3DDevice;

int XUI_FontData::getMaxGlyph()
{
	return m_fontData->getFontData()->m_uiGlyphCount;
}

float XUI_FontData::getFontHeight() 
{ 
	return m_fontData->getFontData()->m_uiGlyphHeight; 
}

float XUI_FontData::getFontTopPadding()
{ 
	return 0;
}

float XUI_FontData::getFontBottomPadding()
{
	return 0;
}

float XUI_FontData::getFontYAdvance()
{
	return m_fontData->getFontData()->m_uiGlyphHeight - 1;
}

float XUI_FontData::getFontMaxWidth()
{
	return m_fontData->getFontData()->m_uiGlyphWidth;
}

float XUI_FontData::getMaxDescent()
{ 
	return 0;
}

float XUI_FontData::getMaxAscent()
{
	return m_fontData->getFontData()->m_uiGlyphHeight;
}

int XUI_FontData::getImageWidth()
{
	return m_fontData->getFontData()->m_uiGlyphMapX;
}

int XUI_FontData::getImageHeight()
{
	return m_fontData->getFontData()->m_uiGlyphMapY;
}

float XUI_FontData::SChar::getMinX()
{
	return 0.0f;
}

float XUI_FontData::SChar::getMaxX()
{
	return static_cast<float>(m_parent->m_fontData->getFontData()->m_uiGlyphWidth);
}

float XUI_FontData::SChar::getMinY()
{
	return 0.0f;
}

float XUI_FontData::SChar::getMaxY()
{
	return 0.0f; //m_parent->m_fontData->getFontData()->m_uiGlyphHeight;
}

float XUI_FontData::SChar::getAdvance()
{
	return static_cast<float>(m_parent->m_fontData->getWidth(m_glyphId));
}

int XUI_FontData::SChar::getGlyphId()
{
	return m_glyphId;
}

#define USE_NEW_UV 1

int XUI_FontData::SChar::tu1()
{
#if USE_NEW_UV
	int row = 0, col = 0;
	m_parent->m_fontData->getPos(m_glyphId, row, col);
	return col * m_parent->m_fontData->getFontData()->m_uiGlyphWidth;
#else
	return m_parent->m_Glyphs[m_glyphId].tu1;
#endif
}

int XUI_FontData::SChar::tu2()
{
#if USE_NEW_UV
	return tu1() + m_parent->m_fontData->getFontData()->m_uiGlyphWidth;
#else
	return m_parent->m_Glyphs[m_glyphId].tu2;
#endif
}

int XUI_FontData::SChar::tv1()
{
#if USE_NEW_UV
	int row = 0, col = 0;
	m_parent->m_fontData->getPos(m_glyphId, row, col);
	return row * m_parent->m_fontData->getFontData()->m_uiGlyphHeight + 1;
#else
	return m_parent->m_Glyphs[m_glyphId].tv1;
#endif
}

int XUI_FontData::SChar::tv2()
{
#if USE_NEW_UV
	return tv1() + m_parent->m_fontData->getFontData()->m_uiGlyphHeight;
#else
	return m_parent->m_Glyphs[m_glyphId].tv2;
#endif
}

short XUI_FontData::SChar::getOffset()
{
	return 0;
}

short XUI_FontData::SChar::getWAdvance()
{
	return 0;
}

XUI_FontData::SChar XUI_FontData::getChar(const wchar_t strChar)
{
	SChar out;
	out.m_glyphId = m_fontData->getGlyphId((unsigned int) strChar);
	out.m_parent = this;
	return out;
}

//--------------------------------------------------------------------------------------
// Name: XUI_FontData()
// Desc: Constructor
//--------------------------------------------------------------------------------------
XUI_FontData::XUI_FontData()
{
    m_pFontTexture = nullptr;
	m_iFontTexture = -1;

    m_dwNumGlyphs = 0L;
    m_Glyphs = nullptr;

    m_cMaxGlyph = 0;

    m_dwNestedBeginCount = 0L;
}


//--------------------------------------------------------------------------------------
// Name: ~XUI_FontData()
// Desc: Destructor
//--------------------------------------------------------------------------------------
XUI_FontData::~XUI_FontData()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: Create the font's internal objects (texture and array of glyph info)
//       using the XPR packed resource file
//--------------------------------------------------------------------------------------
HRESULT XUI_FontData::Create( SFontData &sfontdata )
{
#ifndef _CONTENT_PACKAGE
	app.DebugPrintf("Attempting to load font data for font: '%s'\n", sfontdata.m_strFontName.c_str());
#endif

	BufferedImage *img = new BufferedImage(sfontdata.m_wstrFilename, false, true);
	
	m_iFontTexture = Minecraft::GetInstance()->textures->getTexture(img, C4JRender::TEXTURE_FORMAT_RxGyBzAw, false);

	int imgWidth = img->getWidth(), imgHeight = img->getHeight();
	intArray rawPixels(imgWidth * imgHeight);
    img->getRGB(0, 0, imgWidth, imgHeight, rawPixels, 0, imgWidth);
	delete img;

	m_fontData = new CFontData( sfontdata, rawPixels.data );

	if (rawPixels.data != nullptr) delete [] rawPixels.data;

#if 0
	{  // 4J-JEV: Load in FontData (ABC) file, and initialize member variables from it.

	const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(nullptr);

	//wsprintfW(szResourceLocator,L"section://%X,%s#%s",c_ModuleHandle,L"media", L"media/font/Mojangles_10.abc");
	wsprintfW(szResourceLocator,L"section://%X,%s#%s%s%s",c_ModuleHandle,L"media", L"media/font/",strFontFileName.c_str(),L".abc");

	BYTE *buffer;
	UINT bufferSize;
	hr = XuiResourceLoadAllNoLoc(
         szResourceLocator,
         &buffer,
         &bufferSize
	);
	if( FAILED(hr) ) app.FatalLoadError();

    //return Create( tex, buffer );
	hr = Create( m_iFontTexture, buffer );
	XuiFree(buffer);
	}

		// The ABC's are wrong, so recalc
	// TODO 4J Stu - This isn't going to change every time we run the app, so really the FontMaker tool needs
	// changed, or at the very least the .abc files need pre-processed to store the values we want
	int rowV = 0;
	int rowXOffset = 0;


    for (unsigned int i = 0; i < 299; i++)
	{
		// Translate unprintable characters
		GLYPH_ATTR* pGlyph;

		DWORD letter = m_fontData->getGlyphId(i);
		if( letter == 0 || letter >= 280 ) continue;
		pGlyph = (GLYPH_ATTR*)&m_Glyphs[letter];                 // Get the requested glyph

		// 4J Stu - The original ABC's were generated for a font height that is 1 pixel higher than our cleaned up version
		// We adjust for 1 pixel padding in the y at the top of each box.
		pGlyph->tv1++;

		if( pGlyph->tv1 != rowV )
		{
			rowV = pGlyph->tv1;
			rowXOffset = 0;
		}
		if( pGlyph->wOffset > 0 )
		{
			rowXOffset += pGlyph->wOffset;
			pGlyph->wOffset = 0;
		}

		pGlyph->tu1 -= rowXOffset;
		pGlyph->tu2 -= rowXOffset;

		int x = pGlyph->tu2-1;
		int emptyColumnX = x;
		for (; x >= pGlyph->tu1; x--)
		{
            bool emptyColumn = true;
			for (int y = pGlyph->tv1; y < pGlyph->tv2; y++)
			{
				int rawPix = rawPixels[x + (y*imgWidth)];
                DWORD pixel = rawPixels[x + (y*imgWidth)] & 0xff000000;
                if (pixel > 0) emptyColumn = false;
            }

            if (!emptyColumn && emptyColumnX == pGlyph->tu2-1)
			{
				emptyColumnX = x;
            }
        }
		if(emptyColumnX != pGlyph->tu2-1)
		{
			pGlyph->wWidth = emptyColumnX-pGlyph->tu1;
			pGlyph->wAdvance = pGlyph->wWidth + 1;
		}
    }
#endif

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: Create the font's internal objects (texture and array of glyph info)
//--------------------------------------------------------------------------------------
//HRESULT XUI_FontData::Create( D3DTexture* pFontTexture, const VOID* pFontData )
HRESULT XUI_FontData::Create( int iFontTexture, const VOID* pFontData )
{
    // Save a copy of the texture
    //m_pFontTexture = pFontTexture;
#if 0
	m_iFontTexture = iFontTexture;

    // Check version of file (to make sure it matches up with the FontMaker tool)
    const BYTE* pData = static_cast<const BYTE*>(pFontData);
    DWORD dwFileVersion = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_dwFileVersion;

    if( dwFileVersion == ATGFONTFILEVERSION )
    {
        //m_fFontHeight = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_fFontHeight;
        //m_fFontTopPadding = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_fFontTopPadding;
        //m_fFontBottomPadding = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_fFontBottomPadding;
        //m_fFontYAdvance = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_fFontYAdvance;

        // Point to the translator string which immediately follows the 4 floats
        m_cMaxGlyph = reinterpret_cast<const FontFileHeaderImage_t *>(pData)->m_cMaxGlyph;
  
        WCHAR* translatorTable = const_cast<FontFileHeaderImage_t*>(reinterpret_cast<const FontFileHeaderImage_t *>(pData))->m_TranslatorTable;

		// 4J Stu - This map saves us some memory because the translatorTable is largely empty
		// If we were ever to use >50% of the table then we should store it and use directly rather than the map
		for( unsigned short i = 0; i < m_cMaxGlyph + 1; ++i )
		{
			if( translatorTable[i] == 0 ) continue;
			m_TranslatorMap.insert( unordered_map<wchar_t, unsigned short>::value_type(i, translatorTable[i]) );
		}

        pData += ATGCALCFONTFILEHEADERSIZE( m_cMaxGlyph + 1 );

        // Read the glyph attributes from the file
        m_dwNumGlyphs = reinterpret_cast<const FontFileStrikesImage_t *>(pData)->m_dwNumGlyphs;
		m_Glyphs = new GLYPH_ATTR[m_dwNumGlyphs];
		memcpy(m_Glyphs, reinterpret_cast<const FontFileStrikesImage_t *>(pData)->m_Glyphs, sizeof(GLYPH_ATTR) * m_dwNumGlyphs);

		//m_dwNumGlyphs = m_fontData->getFontData()->m_uiGlyphCount;
    }
    else
    {
		app.DebugPrintf( "Incorrect version number on font file!\n" );
        return E_FAIL;
    }
#endif
    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: Destroy the font object
//--------------------------------------------------------------------------------------
VOID XUI_FontData::Destroy()
{
	if(m_pFontTexture!=nullptr)
	{
		m_pFontTexture->Release();
		delete m_pFontTexture;
	}

	m_fontData->release();

    m_pFontTexture = nullptr;
    m_dwNumGlyphs = 0L;
    m_cMaxGlyph = 0;
	
    m_dwNestedBeginCount = 0L;
}

/*
FLOAT XUI_FontData::GetCharAdvance( const WCHAR* strChar )
{
	unsigned int uiChar = (unsigned int) *strChar;
	return 0.0f;// m_fontData.getAdvance(m_fontData.getGlyphId(uiChar));
}

FLOAT XUI_FontData::GetCharWidth( const WCHAR* strChar )
{
        return 0.0f;
}

void XUI_FontData::GetCharMetrics( const WCHAR* strChar, XUICharMetrics *xuiMetrics)
{
	unsigned int uiChar = (unsigned int) *strChar;
	unsigned short usGlyph = m_fontData->getGlyphId(uiChar);

	xuiMetrics->fAdvance = m_fontData->getWidth(usGlyph); //.getAdvance(usGlyph) * (float) m_fontData.getFontData()->m_uiGlyphHeight;
	xuiMetrics->fMaxX = (float) m_fontData->getFontData()->m_uiGlyphWidth;
	xuiMetrics->fMinX = 0.0f;
	xuiMetrics->fMaxY = 0;// m_fontData.getFontData()->m_fAscent * (float) m_fontData.getFontData()->m_uiGlyphHeight;
	xuiMetrics->fMinY = 0;//m_fontData.getFontData()->m_fDescent * (float) m_fontData.getFontData()->m_uiGlyphHeight;
}

unsigned short XUI_FontData::getGlyphId(wchar_t character)
		{
	return m_fontData->getGlyphId( (unsigned int) character );
}
*/