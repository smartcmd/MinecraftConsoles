#include "stdafx.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.inventory.h"

#include "..\..\Minecraft.h"
#include "..\..\ScreenSizeCalculator.h"
#include "..\..\TileEntityRenderDispatcher.h"
#include "..\..\EnchantTableRenderer.h"
#include "..\..\Lighting.h"
#include "..\..\LocalPlayer.h"

#include "XUI_Scene_Enchant.h"

#include "XUI_Ctrl_EnchantmentBook.h"
#include "..\..\BookModel.h"
#include "..\..\Options.h"

//-----------------------------------------------------------------------------
//  CXuiCtrlEnchantmentBook class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
CXuiCtrlEnchantmentBook::CXuiCtrlEnchantmentBook() :
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

	model = NULL;

	time = 0;
	flip = oFlip = flipT = flipA = 0.0f;
	open = oOpen = 0.0f;
}

CXuiCtrlEnchantmentBook::~CXuiCtrlEnchantmentBook()
{
	//if(model != NULL) delete model;
}

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlEnchantmentBook::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;

	HXUIOBJ parent = m_hObj;
	HXUICLASS hcInventoryClass = XuiFindClass( L"CXuiSceneEnchant" );
	HXUICLASS currentClass;

	do
	{
		XuiElementGetParent(parent,&parent);
		currentClass = XuiGetObjectClass( parent );
	} while (parent != NULL && !XuiClassDerivesFrom( currentClass, hcInventoryClass ) );

	assert( parent != NULL );

	VOID *pObj;
	XuiObjectFromHandle( parent, &pObj );
	m_containerScene = (CXuiSceneEnchant *)pObj;

	last = nullptr;

	m_iPad = m_containerScene->getPad();

	return hr;
}

HRESULT CXuiCtrlEnchantmentBook::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
{
#ifdef _XBOX
	HXUIDC hDC = pRenderData->hDC;

	// build and render with the game call

	RenderManager.Set_matrixDirty();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	float alpha = 1.0f;
	//GetOpacity( &alpha );


	D3DXMATRIX matrix;
	GetFullXForm(&matrix);

	float bwidth,bheight;
	GetBounds(&bwidth,&bheight);

	glColor4f(1, 1, 1, alpha);

	// Annoyingly, XUI renders everything to a z of 0 so if we want to render anything that needs the z-buffer on top of it, then we need to clear it.
	// Clear just the region required for this control.
	D3DRECT clearRect; 
	clearRect.x1 = (int)(matrix._41) - 2;
	clearRect.y1 = (int)(matrix._42) - 2;
	clearRect.x2 = (int)(matrix._41 + ( bwidth  * matrix._11 )) + 2;
	clearRect.y2 = (int)(matrix._42 + ( bheight * matrix._22 )) + 2;

	RenderManager.Clear(GL_DEPTH_BUFFER_BIT, &clearRect);

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_fRawWidth, m_fRawHeight, 0, 1000, 3000);
	//gluPerspective(90, (float) (320 / 240.0f), 0.5f, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	float xo = ( (matrix._41 + ( (bwidth*matrix._11)/2) ) / m_fScreenWidth ) * m_fRawWidth;
	float yo = ( (matrix._42 + ((bheight*matrix._22)/2) ) / m_fScreenHeight ) * m_fRawHeight;

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);

	glPushMatrix();
	glTranslatef(xo, yo, 50.0f);
	float ss;

	// Base scale on height of this control
	// Potentially we might want separate x & y scales here
	ss = ( ( (bheight*matrix._22) / m_fScreenHeight ) * m_fRawHeight ) * 1.5f;

	glScalef(-ss, ss, ss);
	//glRotatef(180, 0, 0, 1);

	glRotatef(45 + 90, 0, 1, 0);
	Lighting::turnOn();
	glRotatef(-45 - 90, 0, 1, 0);

    //float sss = 4;

    //glTranslatef(0, 3.3f, -16);
    //glScalef(sss, sss, sss);

	int tex = pMinecraft->textures->loadTexture(TN_ITEM_BOOK); // 4J was L"/1_2_2/item/book.png"
    pMinecraft->textures->bind(tex);

    glRotatef(20, 1, 0, 0);

	float a = 1;
    float o = oOpen + (open - oOpen) * a;
    glTranslatef((1 - o) * 0.2f, (1 - o) * 0.1f, (1 - o) * 0.25f);
    glRotatef(-(1 - o) * 90 - 90, 0, 1, 0);
    glRotatef(180, 1, 0, 0);

    float ff1 = oFlip + (flip - oFlip) * a + 0.25f;
    float ff2 = oFlip + (flip - oFlip) * a + 0.75f;
    ff1 = (ff1 - floor(ff1)) * 1.6f - 0.3f;
    ff2 = (ff2 - floor(ff2)) * 1.6f - 0.3f;

    if (ff1 < 0) ff1 = 0;
    if (ff2 < 0) ff2 = 0;
    if (ff1 > 1) ff1 = 1;
    if (ff2 > 1) ff2 = 1;

	glEnable(GL_CULL_FACE);

	if(model == NULL)
	{
		// Share the model the the EnchantTableRenderer

		EnchantTableRenderer *etr = (EnchantTableRenderer*)TileEntityRenderDispatcher::instance->getRenderer(eTYPE_ENCHANTMENTTABLEENTITY);
		if(etr != NULL)
		{
			model = etr->bookModel;
		}
		else
		{
			model = new BookModel();
		}
	}

    model->render(NULL, 0, ff1, ff2, o, 0, 1 / 16.0f,true);
	glDisable(GL_CULL_FACE);

    glPopMatrix();
	Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);

	XuiRenderRestoreState(hDC);

	tickBook();

	bHandled = TRUE;

#endif
	return S_OK;
}

//HRESULT CXuiCtrlEnchantmentBook::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
//{
//	HXUIDC hDC = pRenderData->hDC;
//	
//	RenderManager.Set_matrixDirty();
//
//	Minecraft *minecraft = Minecraft::GetInstance();
//
//	// 4J JEV: Inputs in the java, dunno what they are.
//	float a = 1;
//
//	D3DXMATRIX matrix;
//	CXuiControl xuiControl(m_hObj);
//	xuiControl.GetFullXForm(&matrix);
//	float bwidth,bheight;
//	xuiControl.GetBounds(&bwidth,&bheight);
//
//	D3DXVECTOR3 vec, vecP, vecPP;
//	xuiControl.GetPosition(&vec);
//
//	CXuiElement parent, scene;
//	xuiControl.GetParent(&parent);
//	parent.GetPosition(&vecP);
//	parent.GetParent(&scene);
//	scene.GetPosition(&vecPP);
//
//	float xo = ( (matrix._41 + ( (bwidth*matrix._11)/2) ) / m_fScreenWidth ) * m_fRawWidth;
//	float yo = ( (matrix._42 + (bheight*matrix._22) ) / m_fScreenHeight ) * m_fRawHeight;
//
//    glColor4f(1, 1, 1, 1);
//
//    glPushMatrix();
//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//
//    ScreenSizeCalculator ssc = ScreenSizeCalculator(minecraft->options, minecraft->width, minecraft->height);
//
//    //glViewport((int) (ssc.getWidth() - 320) / 2 * ssc.scale, (int) (ssc.getHeight() - 240) / 2 * ssc.scale, (int) (320 * ssc.scale), (int) (240 * ssc.scale));
//
//	// 4J JEV: Trying to position it within the XUI element
//	float xPos, yPos;
//	xPos = (vec.x + vecP.x + vecPP.x) / (minecraft->width/2); xPos = xPos - 1; xPos = 2*xPos/3;
//	yPos = (vec.y + vecP.y + vecPP.y) / (minecraft->height/2); yPos = 1 - yPos; yPos = 2*yPos/3;
//	glTranslatef(xPos, yPos, 0);
//
//    gluPerspective(90, (float) (320 / 240.0f), 9, 80);
//
//    float sss = 1;
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    Lighting::turnOn();
//
//    glTranslatef(0, 3.3f, -16);
//    glScalef(sss, sss, sss);
//
//    float ss = 5;
//
//    glScalef(ss, ss, ss);
//    glRotatef(180, 0, 0, 1);
//
//	int tex = minecraft->textures->loadTexture(TN_ITEM_BOOK); // 4J was L"/1_2_2/item/book.png"
//    minecraft->textures->bind(tex);
//
//    glRotatef(20, 1, 0, 0);
//
//    float o = oOpen + (open - oOpen) * a;
//    glTranslatef((1 - o) * 0.2f, (1 - o) * 0.1f, (1 - o) * 0.25f);
//    glRotatef(-(1 - o) * 90 - 90, 0, 1, 0);
//    glRotatef(180, 1, 0, 0);
//
//    float ff1 = oFlip + (flip - oFlip) * a + 0.25f;
//    float ff2 = oFlip + (flip - oFlip) * a + 0.75f;
//    ff1 = (ff1 - floor(ff1)) * 1.6f - 0.3f;
//    ff2 = (ff2 - floor(ff2)) * 1.6f - 0.3f;
//
//    if (ff1 < 0) ff1 = 0;
//    if (ff2 < 0) ff2 = 0;
//    if (ff1 > 1) ff1 = 1;
//    if (ff2 > 1) ff2 = 1;
//
//    glEnable(GL_RESCALE_NORMAL);
//
//    model.render(NULL, 0, ff1, ff2, o, 0, 1 / 16.0f,true);
//
//    glDisable(GL_RESCALE_NORMAL);
//    Lighting::turnOff();
//    glMatrixMode(GL_PROJECTION);
//    //glViewport(0, 0, minecraft->width, minecraft->height);
//    glPopMatrix();
//    glMatrixMode(GL_MODELVIEW);
//    glPopMatrix();
//
//    Lighting::turnOff();
//    glColor4f(1, 1, 1, 1);
//
//	XuiRenderRestoreState(hDC);
//
//	tickBook();
//
//	bHandled = TRUE;
//
//	return S_OK;
//}

void CXuiCtrlEnchantmentBook::tickBook()
{
	EnchantmentMenu *menu = m_containerScene->getMenu();
	shared_ptr<ItemInstance> current = menu->getSlot(0)->getItem();
	if (!ItemInstance::matches(current, last))
	{
        last = current;

		do
		{
			flipT += random.nextInt(4) - random.nextInt(4);
		} while (flip <= flipT + 1 && flip >= flipT - 1);
    }

    time++;
    oFlip = flip;
    oOpen = open;

	bool shouldBeOpen = false;
    for (int i = 0; i < 3; i++)
	{
        if (menu->costs[i] != 0)
		{
            shouldBeOpen = true;
        }
    } 

    if (shouldBeOpen) open += 0.2f;
    else open -= 0.2f;
    if (open < 0) open = 0;
    if (open > 1) open = 1;


    float diff = (flipT - flip) * 0.4f;
    float max = 0.2f;
    if (diff < -max) diff = -max;
    if (diff > +max) diff = +max;
    flipA += (diff - flipA) * 0.9f;

    flip = flip + flipA;
}