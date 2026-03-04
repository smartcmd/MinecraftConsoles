#pragma once
using namespace std;
#include <xuirender.h>

#include "..\..\Common\UI\UIFontData.h"

// 4J This class is partially based of the ATG font implementation
//--------------------------------------------------------------------------------------
// Name: GLYPH_ATTR
// Desc: Structure to hold information about one glyph (font character image)
//--------------------------------------------------------------------------------------
typedef struct GLYPH_ATTR
{
    WORD tu1, tv1, tu2, tv2;    // Texture coordinates for the image
    SHORT wOffset;              // Pixel offset for glyph start
    SHORT wWidth;               // Pixel width of the glyph
    SHORT wAdvance;             // Pixels to advance after the glyph
    WORD wMask;                 // Channel mask
} GLYPH_ATTR;

//
// These two structures are mapped to data loaded from disk.
// DO NOT ALTER ANY ENTRIES OR YOU WILL BREAK 
// COMPATIBILITY WITH THE FONT FILE
//

// Font description

#define ATGCALCFONTFILEHEADERSIZE(x) ( sizeof(DWORD) + (sizeof(FLOAT)*4) + sizeof(WORD) + (sizeof(WCHAR)*(x)) )
#define ATGFONTFILEVERSION 5

typedef struct FontFileHeaderImage_t {
    DWORD m_dwFileVersion;          // Version of the font file (Must match FONTFILEVERSION)
    FLOAT m_fFontHeight;            // Height of the font strike in pixels
    FLOAT m_fFontTopPadding;        // Padding above the strike zone
    FLOAT m_fFontBottomPadding;     // Padding below the strike zone
    FLOAT m_fFontYAdvance;          // Number of pixels to move the cursor for a line feed
    WORD m_cMaxGlyph;               // Number of font characters (Should be an odd number to maintain DWORD Alignment)
    WCHAR m_TranslatorTable[1];     // ASCII to Glyph lookup table, NOTE: It's m_cMaxGlyph+1 in size.
                                    // Entry 0 maps to the "Unknown" glyph.    
} FontFileHeaderImage_t;

// Font strike array. Immediately follows the FontFileHeaderImage_t
// structure image

typedef struct FontFileStrikesImage_t {
    DWORD m_dwNumGlyphs;            // Size of font strike array (First entry is the unknown glyph)
    GLYPH_ATTR m_Glyphs[1];         // Array of font strike uv's etc... NOTE: It's m_dwNumGlyphs in size
} FontFileStrikesImage_t;

typedef struct _CharMetrics
{
   // units are pixels at current font size

   float fMinX;    // min x coordinate
   float fMinY;    // min y coordinate
   float fMaxX;    // max x coordinate
   float fMaxY;    // max y coordinate
   float fAdvance; // advance value
} CharMetrics;

class XUI_FontData
{
public:
	int getMaxGlyph();
	float getFontHeight();
	float getFontTopPadding();
	float getFontBottomPadding();
	float getFontYAdvance();
	float getFontMaxWidth();
	float getMaxDescent();
	float getMaxAscent();
	int getImageWidth();
	int getImageHeight();

	typedef struct 
	{
		friend class XUI_FontData;

	private:
		unsigned short m_glyphId;
		XUI_FontData *m_parent;

	public:
		bool	hasChar() { return true; }
		float	getMinX();
		float	getMinY();
		float	getMaxX();
		float	getMaxY();
		float	getAdvance();
		int		getGlyphId();
		int		tu1();
		int		tu2();
		int		tv1();
		int		tv2();			// Texture coordinates for the image
		short	getOffset();	// Pixel offset for glyph start
		short	getWidth();		// Pixel width of the glyph
		short	getWAdvance();	// Pixels to advance after the glyph
		WORD	getMask();		// Channel mask, tv2;           
	} SChar;

	SChar getChar(const wchar_t strChar);
	
	// D3D rendering objects
    D3DTexture* m_pFontTexture;
	int m_iFontTexture;

private:
	unordered_map<wchar_t, unsigned short> m_TranslatorMap;

	CharMetrics *m_characterMetrics;

	// Translator table for supporting unicode ranges
    DWORD m_cMaxGlyph;          // Number of entries in the translator table

    // Glyph data for the font
    DWORD m_dwNumGlyphs;        // Number of valid glyphs
    GLYPH_ATTR* m_Glyphs; // Array of glyphs

	DWORD m_dwNestedBeginCount;


protected:
	CFontData *m_fontData;

public:
    // Accessor functions
    inline D3DTexture* GetTexture() const
    {
        return m_pFontTexture;
    }

public:
    XUI_FontData();
    ~XUI_FontData();

    // Functions to create and destroy the internal objects
    HRESULT Create( SFontData &sfontdata );
    //HRESULT Create( D3DTexture* pFontTexture, const VOID* pFontData );
	HRESULT Create( int iFontTexture, const VOID* pFontData );
    VOID    Destroy();

    //FLOAT   GetCharAdvance( const WCHAR* strChar );
    //FLOAT   GetCharWidth( const WCHAR* strChar );
	//void	GetCharMetrics( const WCHAR* strChar, XUICharMetrics *xuiMetrics);
	//unsigned short getGlyphId(wchar_t character);
};