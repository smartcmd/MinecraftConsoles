#include "stdafx.h"
#include "XUI_FontRenderer.h"
#include "XUI_Font.h"
#include "XUI_FontData.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

extern IDirect3DDevice9 *g_pD3DDevice;
extern void GetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA);
extern void SetRenderAndSamplerStates(IDirect3DDevice9 *pDevice,DWORD *RenderStateA,DWORD *SamplerStateA);

XUI_FontRenderer::XUI_FontRenderer()
{
	ZeroMemory(m_loadedFontData, sizeof(XUI_FontData*) * eFontData_MAX);

	//XuiFontSetRenderer(this);

	//Minecraft *pMinecraft=Minecraft::GetInstance();

	//ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	//m_fScreenWidth=(float)pMinecraft->width_phys;
	//m_fRawWidth=(float)ssc.rawWidth;
	//m_fScreenHeight=(float)pMinecraft->height_phys;
	//m_fRawHeight=(float)ssc.rawHeight;
}

HRESULT XUI_FontRenderer::Init( float fDpi )
{
	return( S_OK );

}

VOID XUI_FontRenderer::Term()
{
	return;
}

HRESULT XUI_FontRenderer::GetCaps( DWORD * pdwCaps )
{
    if( pdwCaps != nullptr )
	{
        // setting this means XUI calls the DrawCharsToDevice method
        *pdwCaps = XUI_FONT_RENDERER_CAP_INTERNAL_GLYPH_CACHE | XUI_FONT_RENDERER_CAP_POINT_SIZE_RESPECTED | XUI_FONT_RENDERER_STYLE_DROPSHADOW;
	}
	return ( S_OK );
}

HRESULT XUI_FontRenderer::CreateFont( const TypefaceDescriptor * pTypefaceDescriptor, float fPointSize, DWORD dwStyle, DWORD dwReserved, HFONTOBJ * phFont )
{
	float fXuiSize = fPointSize * ( 16.0f / 14.0f );
	//float fXuiSize = fPointSize * ( 16.0f / 16.0f );
	fXuiSize /= 4.0f;
	fXuiSize = floor( fXuiSize );
	int xuiSize = static_cast<int>(fXuiSize * 4.0f);
	if( xuiSize < 1 ) xuiSize = 8;

	// 4J Stu - We have fonts based on multiples of 8 or 12
	// We don't want to make the text larger as then it may not fit in the box specified
	// so we decrease the size until we find one that will look ok
	while( xuiSize%8!=0 && xuiSize%12!=0 ) xuiSize -= 2;

	//app.DebugPrintf("point size is: %f, xuiSize is: %d\n", fPointSize, xuiSize);

	XUI_Font *font = nullptr;
	XUI_FontData *fontData = nullptr;
	FLOAT scale = 1;

	eFontData efontdata;
	if( xuiSize%12==0 )
	{
		scale = xuiSize/12;
		efontdata = eFontData_Mojangles_11;
	}
	else
	{
		scale = xuiSize/8;
		efontdata = eFontData_Mojangles_7;
	}

	font = m_loadedFonts[efontdata][scale];
	if (font == nullptr)
	{
		fontData = m_loadedFontData[efontdata];
		if (fontData == nullptr)
		{
			SFontData *sfontdata;
			switch (efontdata)
			{
			case eFontData_Mojangles_7:		sfontdata = &SFontData::Mojangles_7;	break;
			case eFontData_Mojangles_11:	sfontdata = &SFontData::Mojangles_11;	break;
			default:						sfontdata = nullptr;						break;
			}

			fontData = new XUI_FontData();
			fontData->Create(*sfontdata);

			m_loadedFontData[efontdata] = fontData;
		}

		font = new XUI_Font( efontdata, scale, fontData );
		m_loadedFonts[efontdata][scale] = font;
	}
	font->IncRefCount();

	*phFont = static_cast<HFONTOBJ>(font);
	return S_OK;
}

VOID XUI_FontRenderer::ReleaseFont( HFONTOBJ hFont )
{
	XUI_Font *xuiFont = static_cast<XUI_Font *>(hFont);
	if (xuiFont != nullptr)
	{
		xuiFont->DecRefCount();
		if (xuiFont->refCount <= 0)
		{
            auto it = m_loadedFonts[xuiFont->m_iFontData].find(xuiFont->m_fScaleFactor);
            if (it != m_loadedFonts[xuiFont->m_iFontData].end()) m_loadedFonts[xuiFont->m_iFontData].erase(it);
			delete hFont;
		}
	}
	return;
}

HRESULT XUI_FontRenderer::GetFontMetrics( HFONTOBJ hFont, XUIFontMetrics *pFontMetrics )
{
    if( hFont == 0 || pFontMetrics == 0 ) return E_INVALIDARG;

	XUI_Font *font = static_cast<XUI_Font *>(hFont);

	pFontMetrics->fLineHeight =		(font->m_fontData->getFontYAdvance() + 1) * font->m_fYScaleFactor;
	pFontMetrics->fMaxAscent =		font->m_fontData->getMaxAscent() * font->m_fYScaleFactor;
	pFontMetrics->fMaxDescent =		font->m_fontData->getMaxDescent() * font->m_fYScaleFactor;
	pFontMetrics->fMaxHeight =		font->m_fontData->getFontHeight() * font->m_fYScaleFactor;
	pFontMetrics->fMaxWidth =		font->m_fontData->getFontMaxWidth() * font->m_fXScaleFactor;
	pFontMetrics->fMaxAdvance =		font->m_fontData->getFontMaxWidth() * font->m_fXScaleFactor;

	//*pFontMetrics = font->m_fontMetrics; // g_fontMetrics;
    return( S_OK );
}

HRESULT XUI_FontRenderer::GetCharMetrics( HFONTOBJ hFont, WCHAR wch, XUICharMetrics *pCharMetrics )
{
    if (hFont == 0 || pCharMetrics == 0) return E_INVALIDARG;

	XUI_Font *font = static_cast<XUI_Font *>(hFont);
	XUI_FontData::SChar sChar = font->m_fontData->getChar(wch);

	pCharMetrics->fMinX = sChar.getMinX() * font->m_fYScaleFactor;
	pCharMetrics->fMinY = sChar.getMinY() * font->m_fYScaleFactor;
	pCharMetrics->fMaxX = sChar.getMaxX() * font->m_fYScaleFactor;
	pCharMetrics->fMaxY = sChar.getMaxY() * font->m_fYScaleFactor;
	pCharMetrics->fAdvance = sChar.getAdvance() * font->m_fYScaleFactor;

    return(S_OK);
}

HRESULT XUI_FontRenderer::DrawCharToTexture( HFONTOBJ hFont, WCHAR wch, HXUIDC hDC, IXuiTexture * pTexture, UINT x, UINT y, UINT width, UINT height, UINT insetX, UINT insetY )
{
    if( hFont==0 || pTexture==nullptr ) return E_INVALIDARG;
    return( S_OK );
}

HRESULT XUI_FontRenderer::DrawCharsToDevice( HFONTOBJ hFont, CharData * pCharData, DWORD dwCount, RECT *pClipRect, HXUIDC hDC, D3DXMATRIX * pWorldViewProj )
{
    if( hFont == 0 ) return E_INVALIDARG;
    if( dwCount == 0 ) return( S_OK );

	DWORD RenderStateA[8];
	DWORD SamplerStateA[5];
	XMVECTOR vconsts[20];
	XMVECTOR pconsts[20];
	XUI_Font *font = static_cast<XUI_Font *>(hFont);

	// 4J-PB - if we're in 480 Widescreen mode, we need to ensure that the font characters are aligned on an even boundary if they are a 2x multiple
	if(!RenderManager.IsHiDef())
	{
		if(RenderManager.IsWidescreen())
		{
			float fScaleX, fScaleY;
			font->GetScaleFactors(&fScaleX,&fScaleY);
			int iScaleX=fScaleX;
			int iScaleY=fScaleY;

			if(iScaleX%2==0)
			{
				int iWorldX=pWorldViewProj->_41;
				pWorldViewProj->_41 = static_cast<float>(iWorldX & -2);
			}
			if(iScaleY%2==0)
			{
				int iWorldY=pWorldViewProj->_42;
				pWorldViewProj->_42 = static_cast<float>(iWorldY & -2);
			}
		}
		else
		{
			// make x an even number for 480 4:3
			int iWorldX=pWorldViewProj->_41;
			pWorldViewProj->_41 = static_cast<float>(iWorldX & -2);

			// 480 SD mode - y needs to be on a pixel boundary when multiplied by 1.5, so if it's an odd number, subtract 1/3 from it
			int iWorldY=pWorldViewProj->_42;
			if(iWorldY%2==1)
			{
				pWorldViewProj->_42-=1.0f/3.0f;
			}
		}
	}

	g_pD3DDevice->GetVertexShaderConstantF( 0, (float *)vconsts, 20 );
	g_pD3DDevice->GetPixelShaderConstantF( 0, (float *)pconsts, 20 );
	g_pD3DDevice->SetRenderState(D3DRS_HALFPIXELOFFSET, TRUE);
	GetRenderAndSamplerStates(g_pD3DDevice, RenderStateA, SamplerStateA );
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	RenderManager.Set_matrixDirty();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1280.0f, 720.0f, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	float matrixCopy[16];
	memcpy(matrixCopy, pWorldViewProj, 64);
	matrixCopy[11] = 0.0f;
	matrixCopy[12] = floor(matrixCopy[12] + 0.5f);
	matrixCopy[13] = floor(matrixCopy[13] + 0.5f);
	matrixCopy[14] = floor(matrixCopy[14] + 0.5f);
	matrixCopy[15] = 1.0f;
	glMultMatrixf(matrixCopy);


	float lineXPos = 0.0f;
	float lineYPos = 0.0f;
	DWORD colour = 0;
	DWORD style = 0;
#if 1
	for( int i = 0; i < dwCount; i++ )
	{
		wstring string;
		string.push_back(pCharData[i].wch);
		lineYPos = pCharData[i].y;
		lineXPos = pCharData[i].x;
		colour = pCharData[i].dwColor;
		style = pCharData[i].dwStyle;

		//if(pCharData[i].wch > font->m_fontData->getMaxGlyph())
		if ( !font->m_fontData->getChar(pCharData[i].wch).hasChar() )
		{
			// Can't render this character, fallback to the default renderer
			app.OverrideFontRenderer(false,false);
			break;
		}
#else
	DWORD i = 0;
	while( i < dwCount )
	{
		wstring string;
		lineYPos = pCharData[i].y;
		lineXPos = pCharData[i].x;
		colour = pCharData[i].dwColor;
		style = pCharData[i].dwStyle;

		while(i < dwCount && pCharData[i].y == lineYPos)
		{
			string.push_back(pCharData[i].wch);
			++i;
		}
#endif

		bool dropShadow = false;
		if( (style & XUI_FONT_STYLE_DROPSHADOW) == XUI_FONT_STYLE_DROPSHADOW) dropShadow = true;

		//int yPos = (int)pCharData[i].y + (int)(font->m_fontMetrics.fLineHeight - font->m_fontMetrics.fMaxAscent)/2;
		//if( (pCharData[i].dwStyle & XUI_FONT_STYLE_VERTICAL_CENTER) == XUI_FONT_STYLE_VERTICAL_CENTER)
		//{
		//	yPos = (pClipRect->bottom - (int)font->m_fontMetrics.fLineHeight) / 2;
		//}

		if(dropShadow)
		{
			DWORD shadowColour;
			XuiGetTextDropShadowColor(hDC, &shadowColour);
			// 4J Stu - Shadow colour is currently ignored
			font->DrawShadowText( lineXPos,lineYPos,colour,shadowColour,string.c_str() );
			//drawShadow(thisChar, (int)pCharData[i].x, yPos, pCharData[i].dwColor );
		}
		else
		{
			font->DrawText( lineXPos,lineYPos,colour,string.c_str() );
			//draw(thisChar, (int)pCharData[i].x, yPos, pCharData[i].dwColor, false );
		}
	}

	g_pD3DDevice->SetVertexShaderConstantF( 0, (float *)vconsts, 20 );
	g_pD3DDevice->SetPixelShaderConstantF( 0, (float *)pconsts, 20 );
	SetRenderAndSamplerStates(g_pD3DDevice, RenderStateA, SamplerStateA );
	g_pD3DDevice->SetRenderState(D3DRS_HALFPIXELOFFSET, FALSE);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	XuiRenderRestoreState(hDC);

    return( S_OK );
}
