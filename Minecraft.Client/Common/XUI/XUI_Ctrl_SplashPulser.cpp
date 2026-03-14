#include "stdafx.h"
#include "../../Minecraft.h"
#include "../../ScreenSizeCalculator.h"
#include "../../Lighting.h"
#include "XUI_Ctrl_SplashPulser.h"
#include "../../Font.h"
#include "../../../Minecraft.World/Mth.h"
#include "../../../Minecraft.World/System.h"

//-----------------------------------------------------------------------------
//  CXuiCtrlSplashPulser class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CXuiCtrlSplashPulser::CXuiCtrlSplashPulser() :
	m_bDirty(FALSE),
	m_fScale(1.0f),
	m_fAlpha(1.0f)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=static_cast<float>(pMinecraft->width_phys);
	m_fRawWidth=static_cast<float>(ssc.rawWidth);
	m_fScreenHeight=static_cast<float>(pMinecraft->height_phys);
	m_fRawHeight=static_cast<float>(ssc.rawHeight);
}

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlSplashPulser::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;
	return hr;
}

HRESULT CXuiCtrlSplashPulser::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
{
#ifdef _XBOX
	HXUIDC hDC = pRenderData->hDC;
	
	Minecraft *pMinecraft=Minecraft::GetInstance();
	Font *font = pMinecraft->font;

	wstring splash( GetText() );

	// build and render with the game call

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

	glPushMatrix();

	D3DXMATRIX matrix;
	CXuiControl xuiControl(m_hObj);
	xuiControl.GetFullXForm(&matrix);
	float bwidth,bheight;
	xuiControl.GetBounds(&bwidth,&bheight);

	float xo = (matrix._41 + (bwidth*matrix._11)/2 );
	float yo = (matrix._42 + (bheight*matrix._22) );
    glTranslatef(xo, yo, 0);

    glRotatef(-17, 0, 0, 1);
    float sss = 1.8f - Mth::abs(Mth::sin(System::currentTimeMillis() % 1000 / 1000.0f * PI * 2) * 0.1f);
	sss*=(m_fScreenWidth/m_fRawWidth);

    sss = sss * 100 / (font->width(splash) + 8 * 4);
    glScalef(sss, sss, sss);
    //drawCenteredString(font, splash, 0, -8, 0xffff00);
	font->drawShadow(splash, 0 - (font->width(splash)) / 2, -8, 0xffff00);
    glPopMatrix();

	glDisable(GL_RESCALE_NORMAL);
	
	glEnable(GL_DEPTH_TEST);

	XuiRenderRestoreState(hDC);

	bHandled = TRUE;
#endif
	return S_OK;
}



