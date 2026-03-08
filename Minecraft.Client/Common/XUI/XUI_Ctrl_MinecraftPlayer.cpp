#include "stdafx.h"
#include "../../Minecraft.h"
#include "../../ScreenSizeCalculator.h"
#include "../../EntityRenderDispatcher.h"
#include "../../Lighting.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "XUI_Ctrl_MinecraftPlayer.h"
#include "XUI_Scene_AbstractContainer.h"
#include "XUI_Scene_Inventory.h"
#include "../../Options.h"

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftPlayer class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CXuiCtrlMinecraftPlayer::CXuiCtrlMinecraftPlayer() :
	m_bDirty(FALSE),
	m_fScale(1.0f),
	m_fAlpha(1.0f)
{
	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=(float)pMinecraft->width_phys;
	m_fRawWidth=(float)ssc.rawWidth;
	m_fScreenHeight=(float)pMinecraft->height_phys;
	m_fRawHeight=(float)ssc.rawHeight;
}

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlMinecraftPlayer::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;

	HXUIOBJ parent = m_hObj;
	HXUICLASS hcInventoryClass = XuiFindClass( L"CXuiSceneInventory" );
	HXUICLASS currentClass;

	do
	{
		XuiElementGetParent(parent,&parent);
		currentClass = XuiGetObjectClass( parent );
	} while (parent != NULL && !XuiClassDerivesFrom( currentClass, hcInventoryClass ) );

	assert( parent != NULL );

	VOID *pObj;
	XuiObjectFromHandle( parent, &pObj );
	m_containerScene = (CXuiSceneInventory *)pObj;

	m_iPad = m_containerScene->getPad();

	return hr;
}

HRESULT CXuiCtrlMinecraftPlayer::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
{
#ifdef _XBOX
	HXUIDC hDC = pRenderData->hDC;

	// build and render with the game call

	RenderManager.Set_matrixDirty();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	glColor4f(1, 1, 1, 1);
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_fRawWidth, m_fRawHeight, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);


	D3DXMATRIX matrix;
	CXuiControl xuiControl(m_hObj);
	xuiControl.GetFullXForm(&matrix);
	float bwidth,bheight;
	xuiControl.GetBounds(&bwidth,&bheight);

	float xo = ( (matrix._41 + ( (bwidth*matrix._11)/2) ) / m_fScreenWidth ) * m_fRawWidth;
	float yo = ( (matrix._42 + (bheight*matrix._22) ) / m_fScreenHeight ) * m_fRawHeight;

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);

	glPushMatrix();
	glTranslatef(xo, yo - 3.5f, 50.0f);
	float ss;// = 26;
	
	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		ss = 24;
	}
	else
	{
		if(app.GetLocalPlayerCount()>1)
		{
			ss = 13;
		}
		else
		{
			ss = 26;
		}
	}
	// Base scale on height of this control
	// Potentially we might want separate x & y scales here
	//ss = ( ( bheight / m_fScreenHeight ) * m_fRawHeight );
	// For testing split screen - this scale is correct for 4 player split screen

	// how many local players do we have?
// 	int iPlayerC=0;
// 
// 	for(int i=0;i<XUSER_MAX_COUNT;i++)
// 	{
// 		if(pMinecraft->localplayers[i] != NULL)
// 		{
// 			iPlayerC++;
// 		}
// 	}
// 
// 	switch(iPlayerC)
// 	{
// 	case 1:
// 		break;
// 	case 2:
// 	case 3:
// 	case 4:
// 		ss *= 0.5f;
// 		break;
// 	}

	glScalef(-ss, ss, ss);
	glRotatef(180, 0, 0, 1);

	float oybr = pMinecraft->localplayers[m_iPad]->yBodyRot;
	float oyr = pMinecraft->localplayers[m_iPad]->yRot;
	float oxr = pMinecraft->localplayers[m_iPad]->xRot;
	float oyhr = pMinecraft->localplayers[m_iPad]->yHeadRot;

	D3DXVECTOR3 pointerPosition = m_containerScene->GetCursorScreenPosition();
	//printf("Pointer screen position is x:%f, y:%f, z:%f\n",pointerPosition.x, pointerPosition.y, pointerPosition.z);

	float xd = ( matrix._41 + ( (bwidth*matrix._11)/2) ) - pointerPosition.x;

	// Need to base Y on head position, not centre of mass
	float yd = ( matrix._42 + ( (bheight*matrix._22) / 2) - 40 ) - pointerPosition.y;

	glRotatef(45 + 90, 0, 1, 0);
	Lighting::turnOn();
	glRotatef(-45 - 90, 0, 1, 0);

	glRotatef(-(float) atan(yd / 40.0f) * 20, 1, 0, 0);

	pMinecraft->localplayers[m_iPad]->yBodyRot = (float) atan(xd / 40.0f) * 20;
	pMinecraft->localplayers[m_iPad]->yRot = (float) atan(xd / 40.0f) * 40;
	pMinecraft->localplayers[m_iPad]->xRot = -(float) atan(yd / 40.0f) * 20;
	pMinecraft->localplayers[m_iPad]->yHeadRot = pMinecraft->localplayers[m_iPad]->yRot;
	//pMinecraft->localplayers[m_iPad]->glow = 1;
	glTranslatef(0, pMinecraft->localplayers[m_iPad]->heightOffset, 0);
	EntityRenderDispatcher::instance->playerRotY = 180;

	// 4J Stu - Turning on hideGui while we do this stops the name rendering in split-screen
	bool wasHidingGui = pMinecraft->options->hideGui;
	pMinecraft->options->hideGui = true;
	EntityRenderDispatcher::instance->render(pMinecraft->localplayers[m_iPad], 0, 0, 0, 0, 1,false,false);
	pMinecraft->options->hideGui = wasHidingGui;
	//pMinecraft->localplayers[m_iPad]->glow = 0;

	pMinecraft->localplayers[m_iPad]->yBodyRot = oybr;
	pMinecraft->localplayers[m_iPad]->yRot = oyr;
	pMinecraft->localplayers[m_iPad]->xRot = oxr;
	pMinecraft->localplayers[m_iPad]->yHeadRot = oyhr;
	glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);

	XuiRenderRestoreState(hDC);

	bHandled = TRUE;
#endif
	return S_OK;

}



