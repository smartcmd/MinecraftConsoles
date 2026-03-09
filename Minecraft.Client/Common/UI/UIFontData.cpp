#include "stdafx.h"
#include "UIFontData.h"

	/////////////////////////////////////////////////////
	//  --- -- --- THIS FILE IS IN UNICODE  --- -- --- //
	/////////////////////////////////////////////////////

SFontData SFontData::Mojangles_7
	= {

		/* Font Name */		"Mojangles7",

#ifdef _XBOX
		/* filename */		L"/font/Mojangles_7.png",
#else
		/* Filename */		L"/TitleUpdate/res/font/Mojangles_7.png",
#endif

		/* Glyph count */	FONTSIZE,
		/* Codepoints */	SFontData::Codepoints,

		/*img wdth,hght*/	190, 264,
		/*img cols,rows*/	FONTCOLS, FONTROWS,


		/*glyph dim x,y*/	8,13,

		/*ascent/descent*/	7.f/13.f, 8.f/13.f,

		/*advance*/			1.f/10.f,

		/*whitespace*/		5,

	};


SFontData SFontData::Mojangles_11
	= {

		/* Font Name */		"Mojangles11",

#ifdef _XBOX 
		/* filename */		L"/font/Mojangles_11.png",
#else
		/* Filename */		L"/TitleUpdate/res/font/Mojangles_11.png",
#endif

		/* Glyph count */	FONTSIZE,
		/* Codepoints */	SFontData::Codepoints,

		/*img wdth,hght*/	305, 348,
		/*img cols,rows*/	FONTCOLS, FONTROWS,

		/*glyph dim x,y*/	13,17,	

		/*ascent/descent*/	11.f/17.f, 6.f/17.f,

		/*advance*/			1.f/13.f,

		/*whitespace*/		7

	};


  // -----------------------------------------------------------------------------
  // 4J-JEV: Glyph -> Unicode Maps,
  // Unicode search tool: http://www.fileformat.info/info/unicode/char/search.htm
  //------------------------------------------------------------------------------


// Originally interpretted from 'Chars.txt', required many alterations to work correctly. (New Characters have been also added)
unsigned short SFontData::Codepoints[FONTSIZE] =
{
	// NOTE: When adding characters here, you may also want to add them to the ignore list 'Mojangles\Dev\Tools\Mojangles.txt' so we know not to panic when localisation uses them.

/*			ż		Ż		ź		Ź		ć		Ć		ń		Ń																													*/
	0x0001,	0x017C,	0x017B,	0x017A,	0x0179,	0x0107,	0x0106,	0x0144,	0x0143,	0x000A,	0x000B,	0x000C,	0x000D,	0x000E,	0x000F,	0x0010,	0x0011,	0x0012,	0x0013,	0x0014,	0x0015,	0x0016,	0x0017,

/*																					!		"		#		$		%		&		'		(		)		*		+		,		-	*/
	0x0018,	0x0019,	0x001A,	0x001B,	0x001C,	0x001D,	0x001E,	0x001F,	0x0020,	0x0000,	0x0021,	0x0022,	0x0023,	0x0024,	0x0025,	0x0026,	0x0027,	0x0028,	0x0029,	0x002A,	0x002B,	0x002C,	0x002D,

/*	.		/		0		1		2		3		4		5		6		7		8		9		:		;		<		=		>		?		@		A		B		C		D	*/
	0x002E,	0x002F,	0x0030,	0x0031,	0x0032,	0x0033,	0x0034,	0x0035,	0x0036,	0x0037,	0x0038,	0x0039,	0x003A,	0x003B,	0x003C,	0x003D,	0x003E,	0x003F,	0x0040,	0x0041,	0x0042,	0x0043,	0x0044,

/*	E		F		G		H		I		J		K		L		M		N		O		P		Q		R		S		T		U		V		W		X		Y		Z		[	*/
	0x0045,	0x0046,	0x0047,	0x0048,	0x0049,	0x004A,	0x004B,	0x004C,	0x004D,	0x004E,	0x004F,	0x0050,	0x0051,	0x0052,	0x0053,	0x0054,	0x0055,	0x0056,	0x0057,	0x0058,	0x0059,	0x005A,	0x005B,

/*	\		]		^		_		`		a		b		c		d		e		f		g		h		i		j		k		l		m		n		o		p		q		r	*/
	0x005C,	0x005D,	0x005E,	0x005F,	0x0060,	0x0061,	0x0062,	0x0063,	0x0064,	0x0065,	0x0066,	0x0067,	0x0068,	0x0069,	0x006A,	0x006B,	0x006C,	0x006D,	0x006E,	0x006F,	0x0070,	0x0071,	0x0072,

/*	s		t		u		v		w		x		y		z		{		|		}		~																							*/
	0x0073,	0x0074,	0x0075,	0x0076,	0x0077,	0x0078,	0x0079,	0x007A,	0x007B,	0x007C,	0x007D,	0x007E,	0x007F,	0x0080,	0x0081,	0x0082,	0x0083,	0x0084,	0x0085,	0x0086,	0x0087,	0x0088,	0x0089,

/*																																														*/
	0x008A,	0x008B,	0x008C,	0x008D,	0x008E,	0x008F,	0x0090,	0x0091,	0x0092,	0x0093,	0x0094,	0x0095,	0x0096,	0x0097,	0x0098,	0x0099,	0x009A,	0x009B,	0x009C,	0x009D,	0x009E,	0x009F,	0x00A0,

/*	¡		¢		£		¤		¥		¦		§		¨		©		ª		«		¬		­		®		¯		°		±		²		³		´		µ		¶		·	*/
	0x00A1,	0x00A2,	0x00A3,	0x00A4,	0x00A5,	0x00A6,	0x00A7,	0x00A8,	0x00A9,	0x00AA,	0x00AB,	0x00AC,	0x00AD,	0x00AE,	0x00AF,	0x00B0,	0x00B1,	0x00B2,	0x00B3,	0x00B4,	0x00B5,	0x00B6,	0x00B7,

/*	¸		¹		º		»		¼		½		¾		¿		À		Á		Â		Ã		Ä		Å		Æ		Ç		È		É		Ê		Ë		Ì		Í		Î	*/
	0x00B8,	0x00B9,	0x00BA,	0x00BB,	0x00BC,	0x00BD,	0x00BE,	0x00BF,	0x00C0,	0x00C1,	0x00C2,	0x00C3,	0x00C4,	0x00C5,	0x00C6,	0x00C7,	0x00C8,	0x00C9,	0x00CA,	0x00CB,	0x00CC,	0x00CD,	0x00CE,

/*	Ï		Ð		Ñ		Ò		Ó		Ô		Õ		Ö		×		Ø		Ù		Ú		Û		Ü		Ý		Þ		ß		à		á		â		ã		ä		å	*/
	0x00CF,	0x00D0,	0x00D1,	0x00D2,	0x00D3,	0x00D4,	0x00D5,	0x00D6,	0x00D7,	0x00D8,	0x00D9,	0x00DA,	0x00DB,	0x00DC,	0x00DD,	0x00DE,	0x00DF,	0x00E0,	0x00E1,	0x00E2,	0x00E3,	0x00E4,	0x00E5,

/*	æ		ç		è		é		ê		ë		ì		í		î		ï		ð		ñ		ò		ó		ô		õ		ö		÷		ø		ù		ú		û		ü	*/
	0x00E6,	0x00E7,	0x00E8,	0x00E9,	0x00EA,	0x00EB,	0x00EC,	0x00ED,	0x00EE,	0x00EF,	0x00F0,	0x00F1,	0x00F2,	0x00F3,	0x00F4,	0x00F5,	0x00F6,	0x00F7,	0x00F8,	0x00F9,	0x00FA,	0x00FB,	0x00FC,

/*	ý		þ		ÿ		Œ		œ		Š		š		Ÿ		Ž		ž		ƒ		ˣ		➄		–		—		’		‚		“		”		„		†		‡		•	*/
	0x00FD,	0x00FE,	0x00FF,	0x0152,	0x0153,	0x0160,	0x0161,	0x0178,	0x017D,	0x017E,	0x0192,	0x02E3,	0x2784,	0x2013,	0x2014,	0x2019,	0x201A,	0x201C,	0x201D,	0x201E,	0x2020,	0x2021,	0x2022,

/*	…		‰		‹		›		€		™		͝		Ş		İ		Ğ		ş		ı		ğ		ę		Ę		ó		Ó		ą		Ą		ś		Ś		ł		Ł	*/
	0x2026,	0x2030,	0x2039,	0x203A,	0x20AC,	0x2122,	0x035D,	0x015E,	0x0130,	0x011E,	0x015F,	0x0131,	0x011F,	0x0119,	0x0118,	0x00F3,	0x00D3,	0x0105,	0x0104,	0x015B,	0x015A,	0x0142,	0x0141,

/*	Ё		А		Б		В		Г		Д		Е		Ж		З		И		Й		К		Л		М		Н		О		П		Р		С		Т		У		Ф		Х	*/
	0x0401,	0x0410,	0x0411,	0x0412,	0x0413,	0x0414,	0x0415,	0x0416,	0x0417,	0x0418,	0x0419,	0x041A,	0x041B,	0x041C,	0x041D,	0x041E,	0x041F,	0x0420,	0x0421,	0x0422,	0x0423,	0x0424,	0x0425,

/*	Ц		Ч		Ш		Щ		Ъ		Ы		Ь		Э		Ю		Я		а		б		в		г		д		е		ж		з		и		й		к		л		м	*/
	0x0426,	0x0427,	0x0428,	0x0429,	0x042A,	0x042B,	0x042C,	0x042D,	0x042E,	0x042F,	0x0430,	0x0431,	0x0432,	0x0433,	0x0434,	0x0435,	0x0436,	0x0437,	0x0438,	0x0439,	0x043A,	0x043B,	0x043C,

/*	н		о		п		р		с		т		у		ф		х		ц		ч		ш		щ		ъ		ы		ь		э		ю		я		ё		χ		ψ		ω	*/
	0x043D,	0x043E,	0x043F,	0x0440,	0x0441,	0x0442,	0x0443,	0x0444,	0x0445,	0x0446,	0x0447,	0x0448,	0x0449,	0x044A,	0x044B,	0x044C,	0x044D,	0x044E,	0x044F,	0x0451,	0x03C7,	0x03C8,	0x03C9,

/*	Č		Ď		Ě		Ĺ		Ľ		Ň		Ő		Ř		Ť		Ů		Ű		č		ď		ě		ĺ		ľ		ň		ő		ř		ť		ů		ű		 	*/
	0x010C,	0x010E,	0x011A,	0x0139,	0x013D,	0x0147,	0x0150,	0x0158,	0x0164,	0x016E,	0x0170,	0x010D,	0x010F,	0x011B,	0x013A,	0x013E,	0x0148,	0x0151,	0x0159,	0x0165,	0x016F,	0x0171,	0x0020,

/*	Α		Β		Γ		Δ		Ε		Ζ		Η		Θ		Ι		Κ		Λ		Μ		Ν		Ξ		Ο		Π		Ρ		Σ		Τ		Υ		Φ		Χ		Ψ	*/
	0x0391,	0x0392,	0x0393,	0x0394,	0x0395,	0x0396,	0x0397,	0x0398,	0x0399,	0x039A,	0x039B,	0x039C,	0x039D,	0x039E,	0x039F,	0x03A0,	0x03A1,	0x03A3,	0x03A4,	0x03A5,	0x03A6,	0x03A7,	0x03A8,

/*	Ω		α		β		γ		δ		ε		ζ		η		θ		ι		κ		λ		μ		ν		ξ		ο		π		ρ		ς		σ		τ		υ		φ	*/
	0x03A9,	0x03B1,	0x03B2,	0x03B3,	0x03B4,	0x03B5,	0x03B6,	0x03B7,	0x03B8,	0x03B9,	0x03BA,	0x03BB,	0x03BC,	0x03BD,	0x03BE,	0x03BF,	0x03C0,	0x03C1,	0x03C2,	0x03C3,	0x03C4,	0x03C5,	0x03C6,

/*	Ά		Έ		Ή		Ί		Ό		Ύ		Ώ		ΐ		ά		έ		ή		ί		ϊ		ό		ύ		ώ		ŕ		΄		‘									*/
	0x0386,	0x0388,	0x0389,	0x038A,	0x038C,	0x038E,	0x038F,	0x0390,	0x03AC,	0x03AD,	0x03AE,	0x03AF,	0x03CA,	0x03CC,	0x03CD,	0x03CE,	0x0155,	0x0384,	0x2018,	0x0000,	0x0000,	0x0000,	0x0000,
};




	     ///////////////////////
		// --- CFontData --- //
	   ///////////////////////

CFontData::CFontData()
{
	m_unicodeMap = unordered_map<unsigned int, unsigned short>();

	m_sFontData = nullptr;
	m_kerningTable = nullptr;
	m_pbRawImage = nullptr;
}

CFontData::CFontData(SFontData &sFontData, int *pbRawImage)
	: m_unicodeMap( sFontData.m_uiGlyphCount + 2 )
{
	this->m_sFontData = &sFontData;

	// INITIALISE ALPHA CHANNEL //

	// Glyph Archive (1Byte per pixel).
	unsigned int archiveSize = sFontData.m_uiGlyphMapX * sFontData.m_uiGlyphMapY; 
	
	this->m_pbRawImage = new unsigned char[archiveSize];

	// 4J-JEV: Take the alpha channel from each pixel.
	for (unsigned int i = 0; i < archiveSize; i++)
	{
		this->m_pbRawImage[i] = (pbRawImage[i] & 0xFF000000) >> 24;
	}

	// CREATE UNICODE MAP //
	for (unsigned int i = 0; i < sFontData.m_uiGlyphCount; i++)
	{
		unordered_map<unsigned int, unsigned short>::value_type pair(sFontData.Codepoints[i], i);
		m_unicodeMap.insert( pair );
	}

	// CREATE KERNING TABLE //
	m_kerningTable = new unsigned short[sFontData.m_uiGlyphCount];
	for (unsigned short glyph = 0; glyph < sFontData.m_uiGlyphCount; glyph++)
	{
		int row,column;
		getPos(glyph,row,column);

		short xMax = 0, _x=0, _y=0;

		// Find the position of the topLeft corner.
		unsigned char *topLeft = m_pbRawImage, *cursor;
		moveCursor(	topLeft, column * sFontData.m_uiGlyphWidth, row * sFontData.m_uiGlyphHeight);

		assert( ((column+1)*sFontData.m_uiGlyphWidth) < sFontData.m_uiGlyphMapX );
		assert( ((row+1)*sFontData.m_uiGlyphHeight) < sFontData.m_uiGlyphMapY );

		static int XX = 79;
		// Find the furthest filled pixel to the right.
		for (short y = 0; y < sFontData.m_uiGlyphHeight; y++)
		{
			for (short x = 0; x < sFontData.m_uiGlyphWidth; x++)
			{
				cursor = topLeft;
				moveCursor(cursor, x, y);
				
				assert( (cursor-m_pbRawImage) < archiveSize );

				if ( *cursor > 0 )
				{
					if (x > xMax) xMax = x;
					_x = x;
					_y = y;
				}
			}
		}

#if _DEBUG_BLOCK_CHARS
		for (short y = 0; y < sFontData.m_uiGlyphHeight; y++)
		{
			for (short x = 0; x < sFontData.m_uiGlyphWidth; x++)
			{
				cursor = topLeft;
				moveCursor(cursor, x, y);

				if (x==0)			*cursor = 0x00;
				else if (x<=xMax)	*cursor = 0xFF;
				else				*cursor = 0x00;
			}
		}
#endif

		// 4J-JEV: Empty glyphs are considered to be whitespace.
		if (xMax == 0)	m_kerningTable[glyph] = sFontData.m_uiWhitespaceWidth;
		else			m_kerningTable[glyph] = xMax + 1;
	}

	// CACHE GLYPH ADVANCES //
	m_pfAdvanceTable = new float[sFontData.m_uiGlyphCount];
	for (unsigned short glyph = 0; glyph < sFontData.m_uiGlyphCount; glyph++)
	{
		m_pfAdvanceTable[glyph] = m_kerningTable[glyph] * m_sFontData->m_fAdvPerPixel;
	}

	// DEBUG //
#ifndef _CONTENT_PACKAGE
	for (int i = 0; i < sFontData.m_uiGlyphCount; i++)
	{
		int unicode = getUnicode(i), unicodeChar = 32, row, col;
		if ( 32 < unicode && unicode < 127 && unicode != 0x0025 )
		{
			unicodeChar = unicode;
		}

		getPos(i, row, col);

		string state = "ok";
		if (i != getGlyphId(unicode))
		{
			state = "MISSMATCHED!";

			app.DebugPrintf( "<GLYPH_%03i> %i\t%c\tU+%.4X, kerning=%i, (%2i,%2i). %s\n", 
				i, getGlyphId(unicode), unicodeChar, unicode, m_kerningTable[i], row, col, state.c_str() );
		}
	}
#endif
}

void CFontData::release()
{
	delete [] m_kerningTable;
	delete [] m_pfAdvanceTable;
	delete [] m_pbRawImage;
}

const string CFontData::getFontName()
{
	return m_sFontData->m_strFontName;
}

SFontData *CFontData::getFontData()
{
	return m_sFontData;
}

unsigned short CFontData::getGlyphId(unsigned int unicodepoint)
{
	unordered_map<unsigned int, unsigned short>::iterator out = m_unicodeMap.find(unicodepoint);
	if (out != m_unicodeMap.end())
		return out->second;
	return 0;
}

unsigned int CFontData::getUnicode(unsigned short glyphId)
{
	return m_sFontData->Codepoints[glyphId];
}

unsigned char *CFontData::topLeftPixel(int row, int col)
{
	unsigned char *out = m_pbRawImage;
	moveCursor(out, col * m_sFontData->m_uiGlyphWidth, row* m_sFontData->m_uiGlyphHeight);
	return out;
}

void CFontData::getPos(unsigned short glyphId, int &rowOut, int &colOut)
{
	rowOut = glyphId / m_sFontData->m_uiGlyphMapCols;
	colOut = glyphId % m_sFontData->m_uiGlyphMapCols;
}

float CFontData::getAdvance(unsigned short glyphId)
{
	return m_pfAdvanceTable[glyphId];
}

int CFontData::getWidth(unsigned short glyphId)
{
	return m_kerningTable[glyphId];
}

bool CFontData::glyphIsWhitespace(unsigned short glyphId)
{
	return unicodeIsWhitespace( getUnicode(glyphId) );
}

bool CFontData::unicodeIsWhitespace(unsigned int unicode)
{
	static const unsigned int MAX_WHITESPACE = 1;
	static const unsigned int whitespace[MAX_WHITESPACE] = { 
		0x0020 
	};

	for (int i=0; i<MAX_WHITESPACE; i++)
	{
		if (unicode == whitespace[i])	return true;
	}

	return false;
}

void CFontData::moveCursor(unsigned char *&cursor, unsigned int dx, unsigned int dy)
{
	cursor += (dy * m_sFontData->m_uiGlyphMapX) + dx;
}
