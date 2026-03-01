#include "stdafx.h"

#include "BufferedImage.h"
#include "UIFontData.h"

#include <unordered_set>

#include "UIBitmapFont.h"


/////////////////////////////
// UI Abstract Bitmap Font //
/////////////////////////////

UIAbstractBitmapFont::~UIAbstractBitmapFont()
{
	if (m_registered) IggyFontRemoveUTF8( m_fontname.c_str(),-1,IGGY_FONTFLAG_none );
	delete m_bitmapFontProvider;
}


UIAbstractBitmapFont::UIAbstractBitmapFont(const string &fontname)
{
	m_fontname = fontname;

	m_registered = false;

	m_bitmapFontProvider = new IggyBitmapFontProvider();
	m_bitmapFontProvider->get_font_metrics = &UIAbstractBitmapFont::GetFontMetrics_Callback;
	m_bitmapFontProvider->get_glyph_for_codepoint = &UIAbstractBitmapFont::GetCodepointGlyph_Callback;
	m_bitmapFontProvider->get_glyph_metrics = &UIAbstractBitmapFont::GetGlyphMetrics_Callback;
	m_bitmapFontProvider->is_empty = &UIAbstractBitmapFont::IsGlyphEmpty_Callback;
	m_bitmapFontProvider->get_kerning = &UIAbstractBitmapFont::GetKerningForGlyphPair_Callback;
	m_bitmapFontProvider->can_bitmap = &UIAbstractBitmapFont::CanProvideBitmap_Callback;
	m_bitmapFontProvider->get_bitmap = &UIAbstractBitmapFont::GetGlyphBitmap_Callback;
	m_bitmapFontProvider->free_bitmap = &UIAbstractBitmapFont::FreeGlyphBitmap_Callback;
	m_bitmapFontProvider->userdata = this;
}

void UIAbstractBitmapFont::registerFont()
{
	if (!m_registered)
	{
		// 4J-JEV: These only need registering the once when we first use this font in Iggy.
		m_bitmapFontProvider->num_glyphs = m_numGlyphs;
		IggyFontInstallBitmapUTF8( m_bitmapFontProvider, m_fontname.c_str(), -1, IGGY_FONTFLAG_none );
		m_registered = true;
	}

	// 4J-JEV: Reset the font redirect to these fonts (we must do this everytime in-case we switched away elsewhere).
	IggyFontSetIndirectUTF8( m_fontname.c_str(), -1, IGGY_FONTFLAG_all, m_fontname.c_str(), -1, IGGY_FONTFLAG_none );
}

IggyFontMetrics * RADLINK UIAbstractBitmapFont::GetFontMetrics_Callback(void *user_context,IggyFontMetrics *metrics)
{
	return ((UIAbstractBitmapFont *) user_context)->GetFontMetrics(metrics);
}

S32 RADLINK UIAbstractBitmapFont::GetCodepointGlyph_Callback(void *user_context,U32 codepoint)
{
	return ((UIAbstractBitmapFont *) user_context)->GetCodepointGlyph(codepoint);
}

IggyGlyphMetrics * RADLINK UIAbstractBitmapFont::GetGlyphMetrics_Callback(void *user_context,S32 glyph,IggyGlyphMetrics *metrics)
{
	return ((UIAbstractBitmapFont *) user_context)->GetGlyphMetrics(glyph,metrics);
}

rrbool RADLINK UIAbstractBitmapFont::IsGlyphEmpty_Callback(void *user_context,S32 glyph)
{
	return ((UIAbstractBitmapFont *) user_context)->IsGlyphEmpty(glyph);
}

F32 RADLINK UIAbstractBitmapFont::GetKerningForGlyphPair_Callback(void *user_context,S32 first_glyph,S32 second_glyph)
{
	return ((UIAbstractBitmapFont *) user_context)->GetKerningForGlyphPair(first_glyph,second_glyph);
}

rrbool RADLINK UIAbstractBitmapFont::CanProvideBitmap_Callback(void *user_context,S32 glyph,F32 pixel_scale)
{
	return ((UIAbstractBitmapFont *) user_context)->CanProvideBitmap(glyph,pixel_scale);
}

rrbool RADLINK UIAbstractBitmapFont::GetGlyphBitmap_Callback(void *user_context,S32 glyph,F32 pixel_scale,IggyBitmapCharacter *bitmap)
{
	return ((UIAbstractBitmapFont *) user_context)->GetGlyphBitmap(glyph,pixel_scale,bitmap);
}

void RADLINK UIAbstractBitmapFont::FreeGlyphBitmap_Callback(void *user_context,S32 glyph,F32 pixel_scale,IggyBitmapCharacter *bitmap)
{
	return ((UIAbstractBitmapFont *) user_context)->FreeGlyphBitmap(glyph,pixel_scale,bitmap);
}

UIBitmapFont::UIBitmapFont(	SFontData &sfontdata )
	:	UIAbstractBitmapFont( sfontdata.m_strFontName )
{
	m_numGlyphs = sfontdata.m_uiGlyphCount;

	BufferedImage bimg(sfontdata.m_wstrFilename);
	int *bimgData = bimg.getData();
	
	m_cFontData = new CFontData(sfontdata, bimgData);
	
	//delete [] bimgData;
}

UIBitmapFont::~UIBitmapFont()
{
	m_cFontData->release();
}

//Callback function type for returning vertical font metrics
IggyFontMetrics *UIBitmapFont::GetFontMetrics(IggyFontMetrics *metrics)
{
	//Description
	// Vertical metrics for a font 
	//Members
	// ascent - extent of characters above baseline (positive) 
	// descent - extent of characters below baseline (positive)  
	// line_gap - spacing between one row's descent and the next line's ascent 
	// average_glyph_width_for_tab_stops - spacing of "average" character for computing default tab stops 
	// largest_glyph_bbox_y1 - lowest point below baseline of any character in the font 

	metrics->ascent = m_cFontData->getFontData()->m_fAscent;
	metrics->descent = m_cFontData->getFontData()->m_fDescent;
	
	metrics->average_glyph_width_for_tab_stops = 8.0f;

	// This is my best guess, there's no reference to a specific glyph here 
	// so aren't these just exactly the same.
	metrics->largest_glyph_bbox_y1 = metrics->descent;

	// metrics->line_gap; // 4J-JEV: Sean said this does nothing.

	return metrics;
}

//Callback function type for mapping 32-bit unicode code point to internal font glyph number; use IGGY_GLYPH_INVALID to mean "invalid character"
S32 UIBitmapFont::GetCodepointGlyph(U32 codepoint)
{
	// 4J-JEV: Change "right single quotation marks" to apostrophies.
	if (codepoint == 0x2019) codepoint = 0x27;

	return m_cFontData->getGlyphId(codepoint);
}

//Callback function type for returning horizontal metrics for each glyph 
IggyGlyphMetrics * UIBitmapFont::GetGlyphMetrics(S32 glyph,IggyGlyphMetrics *metrics)
{
	// 4J-JEV: Information about 'Glyph Metrics'.
	// http://freetype.sourceforge.net/freetype2/docs/glyphs/glyphs-3.html - Overview.
	// http://en.wikipedia.org/wiki/Kerning#Kerning_values - 'Font Units'

	//Description
	// Horizontal metrics for a glyph 
	//Members
	// x0 y0 x1 y1 - bounding box 
	// advance - horizontal distance to move character origin after drawing this glyph 


	/* 4J-JEV:		*IMPORTANT*
	 *
	 * I believe these are measured wrt the scale mentioned in GetGlyphBitmap
	 * i.e. 1.0f == pixel_scale,
	 *
	 * However we do not have that information here, then all these values need to be 
	 * the same for every scale in this font.
	 *
	 * We have 2 scales of bitmap glyph, and we can only scale these up by powers of 2 
	 * otherwise the fonts will become blurry. The appropriate glyph is chosen in
	 * 'GetGlyphBitmap' however we need to set the horizontal sizes here.
	 */

	float glyphAdvance = m_cFontData->getAdvance(glyph);

	// 4J-JEV:	Anything outside this measurement will be
	// cut off if it's at the start or end of the row.
	metrics->x0 = 0.0f;

	if ( m_cFontData->glyphIsWhitespace(glyph) )
		metrics->x1 = 0.0f;
	else
		metrics->x1 = glyphAdvance;

	// The next Glyph just starts right after this one.
	metrics->advance = glyphAdvance;

	//app.DebugPrintf("[UIBitmapFont] GetGlyphMetrics:\n\tmetrics->advance == %f,\n", metrics->advance);

	// These don't do anything either.
	metrics->y0 = 0.0f;	metrics->y1 = 1.0f;

	return metrics;
}

//Callback function type that should return true iff the glyph has no visible elements
rrbool UIBitmapFont::IsGlyphEmpty (S32 glyph)
{
	if (m_cFontData->glyphIsWhitespace(glyph))	return true;
	return false;//app.DebugPrintf("Is glyph %d empty? %s\n",glyph,isEmpty?"TRUE":"FALSE");
}

//Callback function type for returning the kerning amount for a given pair of glyphs 
F32 UIBitmapFont::GetKerningForGlyphPair(S32 first_glyph,S32 second_glyph)
{
	//UIBitmapFont *uiFont = (UIBitmapFont *) user_context;
	//app.DebugPrintf("Get kerning for glyph pair %d,%d\n",first_glyph,second_glyph);

	// 4J-JEV: Yet another field that doesn't do anything.
	// Only set out of paranoia.
	return 0.0f;
}

//Callback function type used for reporting whether a bitmap supports a given glyph at the given scale
rrbool UIBitmapFont::CanProvideBitmap(S32 glyph,F32 pixel_scale)
{
	//app.DebugPrintf("Can provide bitmap for glyph %d at scale %f? %s\n",glyph,pixel_scale,canProvideBitmap?"TRUE":"FALSE");
	return true;
}

//	Description
//	Callback function type used for getting the bitmap for a given glyph
//	Parameters
//	glyph  The glyph to compute/get the bitmap for  
//	pixel_scale  The scale factor (pseudo point size) requested by the textfield,adjusted for display resolution  
//	bitmap  The structure to store the bitmap into  
rrbool UIBitmapFont::GetGlyphBitmap(S32 glyph,F32 pixel_scale,IggyBitmapCharacter *bitmap)
{
	//Description
	// Data structure used to return to Iggy the bitmap to use for a glyph 
	//Members
	// pixels_one_per_byte - pixels startin with the top-left-most; 0 is transparent and 255 is opaque 
	// width_in_pixels - this is the width of the bitmap data 
	// height_in_pixels - this is the height of the bitmap data 
	// stride_in_bytes - the distance from one row to the next 
	// oversample - this is the amount of oversampling (0 or 1 = not oversample,2 = 2x oversampled,4 = 4x oversampled) 
	// point_sample - if true,the bitmap will be drawn with point sampling; if false,it will be drawn with bilinear 
	// top_left_x - the offset of the top left corner from the character origin 
	// top_left_y - the offset of the top left corner from the character origin 
	// pixel_scale_correct - the pixel_scale at which this character should be displayed at displayed_width_in_pixels 
	// pixel_scale_min - the smallest pixel_scale to allow using this character (scaled down) 
	// pixel_scale_max - the largest pixels cale to allow using this character (scaled up) 
	// user_context_for_free - you can use this to store data to access on the corresponding free call 
	
	int row = 0,col = 0;
	m_cFontData->getPos(glyph,row,col);

	// Skip to glyph start.
	bitmap->pixels_one_per_byte = m_cFontData->topLeftPixel(row,col);

	// Choose a reasonable glyph scale.
	float glyphScale = 1.0f, truePixelScale = 1.0f / m_cFontData->getFontData()->m_fAdvPerPixel;
	F32 targetPixelScale = pixel_scale;
	//if(!RenderManager.IsWidescreen())
	//{
	//	// Fix for different scales in 480
	//	targetPixelScale = pixel_scale*2/3;
	//}
	while ( (0.5f + glyphScale) * truePixelScale <  targetPixelScale)
		glyphScale++;

	// 4J-JEV: Debug code to check which font sizes are being used.
#if (!defined _CONTENT_PACKAGE) && (VERBOSE_FONT_OUTPUT > 0)

	struct DebugData 
	{
		string name;
		long scale;
		long mul;

		bool operator==(const DebugData& dd) const
		{
			if ( name.compare(dd.name) != 0 )	return false;
			else if (scale != dd.scale)			return false;
			else if (mul != dd.mul)				return false;
			else								return true;
		}
	};

	static long long lastPrint = System::currentTimeMillis();
	static unordered_set<DebugData> debug_fontSizesRequested;

	{
		DebugData dData = { m_cFontData->getFontName(), (long) pixel_scale, (long) glyphScale };
		debug_fontSizesRequested.insert(dData);

		if ( (lastPrint - System::currentTimeMillis()) > VERBOSE_FONT_OUTPUT )
		{
			app.DebugPrintf("<UIBitmapFont> Requested font/sizes:\n");

			unordered_set<DebugData>::iterator itr;
			for (	itr = debug_fontSizesRequested.begin();
					itr != debug_fontSizesRequested.end();
					itr++
				)
			{
				app.DebugPrintf("<UIBitmapFont>\t- %s:%i\t(x%i)\n",  itr->name.c_str(), itr->scale, itr->mul);
			}
			
			lastPrint = System::currentTimeMillis();
			debug_fontSizesRequested.clear();
		}
	}
#endif

	//app.DebugPrintf("Request glyph_%d (U+%.4X) at %f, converted to %f (%f)\n",
	//				glyph, GetUnicode(glyph), pixel_scale, targetPixelScale, glyphScale);

	// It is not necessary to shrink the glyph width here
	// as its already been done in 'GetGlyphMetrics' by:
	// > metrics->x1 = m_kerningTable[glyph] * ratio;
	bitmap->width_in_pixels = m_cFontData->getFontData()->m_uiGlyphWidth;
	bitmap->height_in_pixels = m_cFontData->getFontData()->m_uiGlyphHeight;

	/* 4J-JEV: This is to do with glyph placement,
	 * and not the position in the archive.
	 * I don't know why the 0.65 is needed, or what it represents,
	 * although it doesn't look like its the baseline.
	 */
	bitmap->top_left_x = 0;

	// 4J-PB - this was chopping off the top of the characters, so accented ones were losing a couple of pixels at the top
	// DaveK has reduced the height of the accented capitalised characters, and we've dropped this from 0.65 to 0.64 
	bitmap->top_left_y = -((S32) m_cFontData->getFontData()->m_uiGlyphHeight) * m_cFontData->getFontData()->m_fAscent;

	bitmap->oversample = 0;
	bitmap->point_sample = true;

	// 4J-JEV:
	// pixel_scale == font size chosen in flash.
	// bitmap->pixel_scale_correct = (float) m_glyphHeight;	// Scales the glyph to desired size.
	// bitmap->pixel_scale_correct = pixel_scale;			// Always the same size (not desired size).
	// bitmap->pixel_scale_correct = pixel_scale * 0.5;		// Doubles original size.
	// bitmap->pixel_scale_correct = pixel_scale * 2;		// Halves original size.
	
	// Actual scale, and possible range of scales.
	bitmap->pixel_scale_correct = pixel_scale / glyphScale;
	bitmap->pixel_scale_max = 99.0f;
	bitmap->pixel_scale_min = 0.0f;

	/* 4J-JEV: Some of Sean's code.
	int glyphScaleMin = 1;
    int glyphScaleMax = 3;
    float actualScale = pixel_scale / glyphScale;
    bitmap->pixel_scale_correct = actualScale;
    bitmap->pixel_scale_min = actualScale * glyphScaleMin * 0.999f;
    bitmap->pixel_scale_max = actualScale * glyphScaleMax * 1.001f; */

	// 4J-JEV: Nothing to do with glyph placement,
	// entirely to do with cropping your glyph out of an archive.
	bitmap->stride_in_bytes = m_cFontData->getFontData()->m_uiGlyphMapX;

	// 4J-JEV: Additional information needed to release memory afterwards.
	bitmap->user_context_for_free = NULL;

	return true;
}

//Callback function type for freeing a bitmap shape returned by GetGlyphBitmap
void UIBitmapFont::FreeGlyphBitmap(S32 glyph,F32 pixel_scale,IggyBitmapCharacter *bitmap)
{
	// We don't need to free anything,it just comes from the archive.
	//app.DebugPrintf("Free bitmap for glyph %d at scale %f\n",glyph,pixel_scale);
}
