#include "stdafx.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../Font.h"
#include "../../Lighting.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "XUI_Scene_Enchant.h"
#include "XUI_Ctrl_EnchantButton.h"
#include "XUI_Ctrl_EnchantmentButtonText.h"
#include "../../Minecraft.h"
#include "../../TexturePackRepository.h"
#include "../../TexturePack.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlEnchantmentButtonText::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	HRESULT hr=S_OK;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
	m_fScreenWidth=(float)pMinecraft->width_phys;
	m_fRawWidth=(float)ssc.rawWidth;
	m_fScreenHeight=(float)pMinecraft->height_phys;
	m_fRawHeight=(float)ssc.rawHeight;

	HXUIOBJ parent = m_hObj;
	HXUICLASS hcInventoryClass = XuiFindClass( L"CXuiCtrlEnchantmentButton" );
	HXUICLASS currentClass;

	do
	{
		XuiElementGetParent(parent,&parent);
		currentClass = XuiGetObjectClass( parent );
	} while (parent != NULL && !XuiClassDerivesFrom( currentClass, hcInventoryClass ) );

	assert( parent != NULL );

	VOID *pObj;
	XuiObjectFromHandle( parent, &pObj );
	m_parentControl = (CXuiCtrlEnchantmentButton *)pObj;

	m_lastCost = 0;
	m_enchantmentString = L"";

	m_textColour = app.GetHTMLColour(eTextColor_Enchant);
	m_textFocusColour = app.GetHTMLColour(eTextColor_EnchantFocus);
	m_textDisabledColour = app.GetHTMLColour(eTextColor_EnchantDisabled);

	return hr;
}

HRESULT CXuiCtrlEnchantmentButtonText::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
{
	HXUIDC hDC = pRenderData->hDC;
	CXuiControl xuiControl(m_hObj);

	// build and render with the game call

	RenderManager.Set_matrixDirty();

	Minecraft *pMinecraft=Minecraft::GetInstance();

	D3DXMATRIX matrix;
	xuiControl.GetFullXForm(&matrix);
	float bwidth,bheight;
	xuiControl.GetBounds(&bwidth,&bheight);

	// Annoyingly, XUI renders everything to a z of 0 so if we want to render anything that needs the z-buffer on top of it, then we need to clear it.
	// Clear just the region required for this control.
	D3DRECT clearRect;
	clearRect.x1 = (int)(matrix._41) - 2;
	clearRect.y1 = (int)(matrix._42) - 2;
	clearRect.x2 = (int)(matrix._41 + ( bwidth  * matrix._11 )) + 2;
	clearRect.y2 = (int)(matrix._42 + ( bheight * matrix._22 )) + 2;

	RenderManager.Clear(GL_DEPTH_BUFFER_BIT, &clearRect);
	//					glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, m_fScreenWidth, m_fScreenHeight, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);


	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120, 1, 0, 0);
	//Lighting::turnOnGui();
	glPopMatrix();



	EnchantmentMenu *menu = m_parentControl->m_containerScene->getMenu();

	float xo = matrix._41;
	float yo = matrix._42;
	glTranslatef(xo, yo, 50.0f);

	float ss = 1;

	if(!m_parentControl->m_containerScene->m_bSplitscreen)
	{
		ss = 2;
	}

	glScalef(ss, ss, ss);

	int cost = menu->costs[m_parentControl->m_index];

	if(cost != m_lastCost)
	{
		m_lastCost = cost;
		m_enchantmentString = EnchantmentNames::instance.getRandomName();
	}

	glColor4f(1, 1, 1, 1);
	if (cost != 0)
	{
		wstring line = std::to_wstring(cost);
		Font *font = pMinecraft->altFont;
		//int col = 0x685E4A;
		unsigned int col = m_textColour;
		if (pMinecraft->localplayers[m_parentControl->m_iPad]->experienceLevel < cost && !pMinecraft->localplayers[m_parentControl->m_iPad]->abilities.instabuild)
		{
			col = m_textDisabledColour;
			font->drawWordWrap(m_enchantmentString, 0, 0, bwidth/ss, col, bheight/ss);
			font = pMinecraft->font;
			//col = (0x80ff20 & 0xfefefe) >> 1;
			//font->drawShadow(line, (bwidth - font->width(line))/ss, 7, col);
		}
		else
		{
			if (m_parentControl->HasFocus())
			{
				//col = 0xffff80;
				col = m_textFocusColour;
			}
			font->drawWordWrap(m_enchantmentString, 0, 0, bwidth/ss, col, bheight/ss);
			font = pMinecraft->font;
			//col = 0x80ff20;
			//font->drawShadow(line, (bwidth - font->width(line))/ss, 7, col);
		}
	}
	else
	{
	}

	//Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);

	XuiRenderRestoreState(hDC);

	bHandled = TRUE;

	return S_OK;
}

CXuiCtrlEnchantmentButtonText::EnchantmentNames CXuiCtrlEnchantmentButtonText::EnchantmentNames::instance;

CXuiCtrlEnchantmentButtonText::EnchantmentNames::EnchantmentNames()
{
	wstring allWords = L"the elder scrolls klaatu berata niktu xyzzy bless curse light darkness fire air earth water hot dry cold wet ignite snuff embiggen twist shorten stretch fiddle destroy imbue galvanize enchant free limited range of towards inside sphere cube self other ball mental physical grow shrink demon elemental spirit animal creature beast humanoid undead fresh stale ";
	std::wistringstream iss(allWords);
	std::copy(std::istream_iterator< std::wstring, wchar_t, std::char_traits<wchar_t> >(iss), std::istream_iterator< std::wstring, wchar_t, std::char_traits<wchar_t> >(),std::back_inserter(words));
}

wstring CXuiCtrlEnchantmentButtonText::EnchantmentNames::getRandomName()
{
	int wordCount = random.nextInt(2) + 3;
	wstring word = L"";
	for (int i = 0; i < wordCount; i++)
	{
		if (i > 0) word += L" ";
		word += words[random.nextInt(words.size())];
	}
	return word;
}