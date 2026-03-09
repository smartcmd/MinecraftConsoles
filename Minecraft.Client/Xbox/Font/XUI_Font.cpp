#include "stdafx.h"
#include "..\..\Tesselator.h"
#include "XUI_FontData.h"
#include "XUI_Font.h"

extern IDirect3DDevice9 *g_pD3DDevice;

//--------------------------------------------------------------------------------------
// Name: XUI_Font()
// Desc: Constructor
//--------------------------------------------------------------------------------------
XUI_Font::XUI_Font(int iFontData, float scaleFactor, XUI_FontData *fontData)
	: m_iFontData(iFontData), m_fScaleFactor(scaleFactor)
{
	m_fontData = fontData;
	refCount = 0;

    m_fCursorX = 0.0f;
    m_fCursorY = 0.0f;

    m_fXScaleFactor = m_fYScaleFactor = scaleFactor;

    m_fSlantFactor = 0.0f;
    m_bRotate = FALSE;
    m_dRotCos = cos( 0.0 );
    m_dRotSin = sin( 0.0 );

    m_dwNestedBeginCount = 0L;
	
	// Initialize the window
    D3DDISPLAYMODE DisplayMode;
    g_pD3DDevice->GetDisplayMode( 0, &DisplayMode );
    m_rcWindow.x1 = 0;
    m_rcWindow.y1 = 0;
    m_rcWindow.x2 = DisplayMode.Width;
    m_rcWindow.y2 = DisplayMode.Height;
}


//--------------------------------------------------------------------------------------
// Name: ~XUI_Font()
// Desc: Destructor
//--------------------------------------------------------------------------------------
XUI_Font::~XUI_Font()
{
}

//--------------------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: Get the dimensions of a text string
//--------------------------------------------------------------------------------------

VOID XUI_Font::GetTextExtent( const WCHAR* strText, FLOAT* pWidth,
                          FLOAT* pHeight, BOOL bFirstLineOnly ) const
{
    assert( pWidth != nullptr );
    assert( pHeight != nullptr );

    // Set default text extent in output parameters
    int iWidth = 0;
    FLOAT fHeight = 0.0f;

    if( strText )
    {
        // Initialize counters that keep track of text extent
        int ix = 0;
        FLOAT fy = m_fontData->getFontHeight();       // One character high to start
        if( fy > fHeight )
            fHeight = fy;

        // Loop through each character and update text extent
        DWORD letter;
        while( (letter = *strText) != 0 )
        {
            ++strText;

            // Handle newline character
            if( letter == L'\n' )
            {
                if( bFirstLineOnly )
                    break;
                ix = 0;
                fy += m_fontData->getFontYAdvance();
                // since the height has changed, test against the height extent
                if( fy > fHeight )
                    fHeight = fy;
           }

            // Handle carriage return characters by ignoring them. This helps when
            // displaying text from a file.
            if( letter == L'\r' )
                continue;

            // Translate unprintable characters
            XUI_FontData::SChar sChar = m_fontData->getChar(letter);
        
			// Get text extent for this character's glyph
			ix += sChar.getOffset();
			ix += sChar.getWAdvance();

            // Since the x widened, test against the x extent
            if (ix > iWidth) iWidth = ix;
        }
    }

    // Convert the width to a float here, load/hit/store. :(
    FLOAT fWidth = static_cast<FLOAT>(iWidth);          // Delay the use if fWidth to reduce LHS pain
    // Apply the scale factor to the result
    fHeight *= m_fYScaleFactor;
     // Store the final results
    *pHeight = fHeight;

    fWidth *= m_fXScaleFactor;
    *pWidth = fWidth;
}

//--------------------------------------------------------------------------------------
// Name: GetTextWidth()
// Desc: Returns the width in pixels of a text string
//--------------------------------------------------------------------------------------
FLOAT XUI_Font::GetTextWidth( const WCHAR* strText ) const
{
    FLOAT fTextWidth;
    FLOAT fTextHeight;
    GetTextExtent( strText, &fTextWidth, &fTextHeight );
    return fTextWidth;
}

//--------------------------------------------------------------------------------------
// Name: Begin()
// Desc: Prepares the font vertex buffers for rendering.
//--------------------------------------------------------------------------------------
VOID XUI_Font::Begin()
{
    PIXBeginNamedEvent( 0, "Text Rendering" );

    // Set state on the first call
    if( 0 == m_dwNestedBeginCount )
    {
        // Cache the global pointer into a register
        IDirect3DDevice9 *pD3dDevice = g_pD3DDevice;
        assert( pD3dDevice );

        // Set the texture scaling factor as a vertex shader constant
        //D3DSURFACE_DESC TextureDesc;
        //m_pFontTexture->GetLevelDesc( 0, &TextureDesc );		// Get the description
 
        // Set render state
		assert(m_fontData->m_pFontTexture != nullptr || m_fontData->m_iFontTexture > 0);
		if(m_fontData->m_pFontTexture != nullptr)
		{
			pD3dDevice->SetTexture( 0, m_fontData->m_pFontTexture );
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, m_fontData->m_iFontTexture);
		}
 
        //// Read the TextureDesc here to ensure no load/hit/store from GetLevelDesc()
        //FLOAT vTexScale[4];
        //vTexScale[0] = 1.0f / TextureDesc.Width;		// LHS due to int->float conversion
        //vTexScale[1] = 1.0f / TextureDesc.Height;
        //vTexScale[2] = 0.0f;
        //vTexScale[3] = 0.0f;
        //
        //pD3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        //pD3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        //pD3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
        //pD3dDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
        //pD3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
        //pD3dDevice->SetRenderState( D3DRS_ALPHAREF, 0x08 );
        //pD3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
        //pD3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
        //pD3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        //pD3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
        //pD3dDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );
        //pD3dDevice->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );
        //pD3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
        //pD3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
        //pD3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
        //pD3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
    }

    // Keep track of the nested begin/end calls.
    m_dwNestedBeginCount++;
}


//--------------------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws text as textured polygons
//--------------------------------------------------------------------------------------
VOID XUI_Font::DrawText( DWORD dwColor, const WCHAR* strText, DWORD dwFlags,
                     FLOAT fMaxPixelWidth )
{
    DrawText( m_fCursorX, m_fCursorY, dwColor, strText, dwFlags, fMaxPixelWidth );
}

//--------------------------------------------------------------------------------------
// Name: DrawShadowText()
// Desc: Draws text as textured polygons
//--------------------------------------------------------------------------------------
VOID XUI_Font::DrawShadowText( FLOAT fOriginX, FLOAT fOriginY, DWORD dwColor, DWORD dwShadowColor,
                     const WCHAR* strText, DWORD dwFlags, FLOAT fMaxPixelWidth)
{
	float fXShadow=1.0f, fYShadow=1.0f;
	// 4J Stu - Don't move the drop shadow as much
    //DrawText( fOriginX + (1*m_fXScaleFactor), fOriginY + (1*m_fYScaleFactor), dwColor, strText, dwFlags, fMaxPixelWidth, true );

	// 4J-PB - if we're in 480 widescreen, we need to draw the drop shadow at +2 pixels, so that when the scene is halved, it's at +1
	if(!RenderManager.IsHiDef())
	{
		if(RenderManager.IsWidescreen())
		{
			fXShadow=2.0f;
			fYShadow=2.0f;
		}
		//else
		//{
			// 480 SD mode - the draw text call will reposition the y
		//}
	}
    DrawText( fOriginX + fXShadow, fOriginY + fYShadow, dwColor, strText, dwFlags, fMaxPixelWidth, true );
    DrawText( fOriginX, fOriginY, dwColor, strText, dwFlags, fMaxPixelWidth );
	
	//DrawText( fOriginX + 1, fOriginY + 1, dwShadowColor, strText, dwFlags, fMaxPixelWidth);
    //DrawText( fOriginX, fOriginY, dwColor, strText, dwFlags, fMaxPixelWidth );
}

//--------------------------------------------------------------------------------------
// Name: DrawText()
// Desc: Draws text as textured polygons
//       TODO: This function should use the Begin/SetVertexData/End() API when it
//       becomes available.
//--------------------------------------------------------------------------------------
VOID XUI_Font::DrawText( FLOAT fOriginX, FLOAT fOriginY, DWORD dwColor,
                     const WCHAR* strText, DWORD dwFlags, FLOAT fMaxPixelWidth, bool darken /*= false*/ )
{
    if( nullptr == strText )    return;
    if( L'\0' == strText[0] ) return;
 
	// 4J-PB - if we're in 480 widescreen mode, we need to ensure that the font characters are aligned on an even boundary if they are a 2x multiple
	if(!RenderManager.IsHiDef())
	{
		if(RenderManager.IsWidescreen())
		{		
			int iScaleX=static_cast<int>(m_fXScaleFactor);
			int iOriginX;
			if(iScaleX%2==0)
			{
				iOriginX=static_cast<int>(fOriginX);
				if(iOriginX%2==1)
				{
					fOriginX+=1.0f;
				}
			}
			int iScaleY=static_cast<int>(m_fYScaleFactor);
			int iOriginY;
			if(iScaleY%2==0)
			{
				iOriginY=static_cast<int>(fOriginY);
				if(iOriginY%2==1)
				{
					fOriginY+=1.0f;
				}
			}
		}
		else
		{
			// 480 SD mode - y needs to be on a pixel boundary when multiplied by 1.5, so if it's an odd number, subtract 1/3 from it
			int iOriginY=static_cast<int>(fOriginY);
			if(iOriginY%2==1)
			{
				fOriginY-=1.0f/3.0f;
			}
		}
	}
    // Create a PIX user-defined event that encapsulates all of the text draw calls.
    // This makes DrawText calls easier to recognize in PIX captures, and it makes
    // them take up fewer entries in the event list.
    PIXBeginNamedEvent( dwColor, "DrawText: %S", strText );

    // Set up stuff to prepare for drawing text
    Begin();

	if (darken)
	{
        int oldAlpha = dwColor & 0xff000000;
        dwColor = (dwColor & 0xfcfcfc) >> 2;
        dwColor += oldAlpha;
    }

	float r = ((dwColor >> 16) & 0xff) / 255.0f;
	float g = ((dwColor >> 8) & 0xff) / 255.0f;
	float b = ((dwColor) & 0xff) / 255.0f;
	float a = ((dwColor >> 24) & 0xff) / 255.0f;
	if (a == 0) a = 1;
	// a = 1;
	glColor4f(r, g, b, a);

    // Set the starting screen position
    if( ( fOriginX < 0.0f ) || ( ( dwFlags & ATGFONT_RIGHT ) && ( fOriginX <= 0.0f ) ) )
    {
        fOriginX += ( m_rcWindow.x2 - m_rcWindow.x1 );
    }
	// 4J-PB - not sure what this code was intending to do, but it removed a line of text that is slightly off the top of the control, rather than having it partially render
//     if( fOriginY < 0.0f )
//     {
//         fOriginY += ( m_rcWindow.y2 - m_rcWindow.y1 );
//     }

    m_fCursorX = floorf( fOriginX );
    m_fCursorY = floorf( fOriginY );

    // Adjust for padding
    fOriginY -= m_fontData->getFontTopPadding();
	
	XUI_FontData::SChar sChar = m_fontData->getChar(L'.');
    FLOAT fEllipsesPixelWidth = m_fXScaleFactor * 3.0f * (sChar.getOffset() + sChar.getWAdvance());

    if( dwFlags & ATGFONT_TRUNCATED )
    {
        // Check if we will really need to truncate the string
        if( fMaxPixelWidth <= 0.0f )
        {
            dwFlags &= ( ~ATGFONT_TRUNCATED );
        }
        else
        {
            FLOAT w, h;
            GetTextExtent( strText, &w, &h, TRUE );

            // If not, then clear the flag
            if( w <= fMaxPixelWidth )
                dwFlags &= ( ~ATGFONT_TRUNCATED );
        }
    }

    // If vertically centered, offset the starting m_fCursorY value
    if( dwFlags & ATGFONT_CENTER_Y )
    {
        FLOAT w, h;
        GetTextExtent( strText, &w, &h );
        m_fCursorY = floorf( m_fCursorY - (h * 0.5f) );
    }

    // Add window offsets
    FLOAT Winx = static_cast<FLOAT>(m_rcWindow.x1);
    FLOAT Winy = static_cast<FLOAT>(m_rcWindow.y1);
    fOriginX += Winx;
    fOriginY += Winy;
    m_fCursorX += Winx;
    m_fCursorY += Winy;

    // Set a flag so we can determine initial justification effects
    BOOL bStartingNewLine = TRUE;

    DWORD dwNumEllipsesToDraw = 0;

    // Begin drawing the vertices


    DWORD dwNumChars = wcslen( strText ) + ( dwFlags & ATGFONT_TRUNCATED ? 3 : 0 );

    bStartingNewLine = TRUE;

    // Draw four vertices for each glyph
    while( *strText )
    {
        WCHAR letter;

        if( dwNumEllipsesToDraw )
        {
            letter = L'.';
        }
        else
        {
            // If starting text on a new line, determine justification effects
            if( bStartingNewLine )
            {
                if( dwFlags & ( ATGFONT_RIGHT | ATGFONT_CENTER_X ) )
                {
                    // Get the extent of this line
                    FLOAT w, h;
                    GetTextExtent( strText, &w, &h, TRUE );

                    // Offset this line's starting m_fCursorX value
                    if( dwFlags & ATGFONT_RIGHT )
                        m_fCursorX = floorf( fOriginX - w );
                    if( dwFlags & ATGFONT_CENTER_X )
                        m_fCursorX = floorf( fOriginX - w * 0.5f );
                }
                bStartingNewLine = FALSE;
            }

            // Get the current letter in the string
            letter = *strText++;

            // Handle the newline character
            if( letter == L'\n' )
            {
                m_fCursorX = fOriginX;
                m_fCursorY += m_fontData->getFontYAdvance() * m_fYScaleFactor;
                bStartingNewLine = TRUE;

                continue;
            }

            // Handle carriage return characters by ignoring them. This helps when
            // displaying text from a file.
            if( letter == L'\r' )
                continue;
        }

        // Translate unprintable characters
        XUI_FontData::SChar sChar = m_fontData->getChar( letter );

        FLOAT fOffset = m_fXScaleFactor * static_cast<FLOAT>(sChar.getOffset());
        FLOAT fAdvance = m_fXScaleFactor * static_cast<FLOAT>(sChar.getWAdvance());
		// 4J Use the font max width otherwise scaling doesnt look right
        FLOAT fWidth = m_fXScaleFactor * (sChar.tu2() - sChar.tu1());//( FLOAT )pGlyph->wWidth;
		FLOAT fHeight = m_fYScaleFactor * m_fontData->getFontHeight();

        if( 0 == dwNumEllipsesToDraw )
        {
            if( dwFlags & ATGFONT_TRUNCATED )
            {
                // Check if we will be exceeded the max allowed width
                if( m_fCursorX + fOffset + fWidth + fEllipsesPixelWidth + m_fSlantFactor > fOriginX + fMaxPixelWidth )
                {
                    // Yup, draw the three ellipses dots instead
                    dwNumEllipsesToDraw = 3;
                    continue;
                }
            }
        }

        // Setup the screen coordinates
        m_fCursorX += fOffset;
        FLOAT X4 = m_fCursorX;
        FLOAT X1 = X4 + m_fSlantFactor;
        FLOAT X3 = X4 + fWidth;
        FLOAT X2 = X1 + fWidth;
        FLOAT Y1 = m_fCursorY;
        FLOAT Y3 = Y1 + fHeight;
        FLOAT Y2 = Y1;
        FLOAT Y4 = Y3;

        m_fCursorX += fAdvance;

        // Add the vertices to draw this glyph

        FLOAT tu1 = sChar.tu1() / static_cast<float>(m_fontData->getImageWidth());
        FLOAT tv1 = sChar.tv1() / static_cast<float>(m_fontData->getImageHeight());
        FLOAT tu2 = sChar.tu2() / static_cast<float>(m_fontData->getImageWidth());
        FLOAT tv2 = sChar.tv2() / static_cast<float>(m_fontData->getImageHeight());

		Tesselator *t = Tesselator::getInstance();
		t->begin();
		t->vertexUV(X1, Y1, 0.0f, tu1, tv1);
		t->vertexUV(X2, Y2, 0.0f, tu2, tv1);
		t->vertexUV(X3, Y3, 0.0f, tu2, tv2);
		t->vertexUV(X4, Y4, 0.0f, tu1, tv2);
		t->end();


        // If drawing ellipses, exit when they're all drawn
        if( dwNumEllipsesToDraw )
        {
            if( --dwNumEllipsesToDraw == 0 )
                break;
        }

        dwNumChars--;
    }

    // Undo window offsets
    m_fCursorX -= Winx;
    m_fCursorY -= Winy;

    // Call End() to complete the begin/end pair for drawing text
    End();

    // Close off the user-defined event opened with PIXBeginNamedEvent.
    PIXEndNamedEvent();
}


//--------------------------------------------------------------------------------------
// Name: End()
// Desc: Paired call that restores state set in the Begin() call.
//--------------------------------------------------------------------------------------
VOID XUI_Font::End()
{
    assert( m_dwNestedBeginCount > 0 );
    if( --m_dwNestedBeginCount > 0 )
    {
        PIXEndNamedEvent();
        return;
    }

    PIXEndNamedEvent();
}