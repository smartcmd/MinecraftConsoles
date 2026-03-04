#include "stdafx.h"

#include "..\..\ItemRenderer.h"
#include "..\..\GameRenderer.h"
#include "..\..\TileRenderer.h"
#include "..\..\Lighting.h"
#include "..\..\ScreenSizeCalculator.h"
#include "..\..\LocalPlayer.h"
#include "..\..\..\Minecraft.World\ItemInstance.h"
#include "..\..\..\Minecraft.World\Item.h"
#include "..\..\..\Minecraft.World\Tile.h"
#include "XUI_Ctrl_MinecraftSlot.h"

//-----------------------------------------------------------------------------
//  CXuiCtrlMinecraftSlot class
//-----------------------------------------------------------------------------

// The xzp path icons for the leaderboard

LPCWSTR CXuiCtrlMinecraftSlot::xzpIcons[15]=
{
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Skeleton.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Creeper.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_SpiderJockey.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Spider.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Zombie.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_ZombiePigman.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Swam.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Walked.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Fallen.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Portal.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Climbed.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Ghast.png",
	L"Graphics\\Leaderboard\\LeaderBoard_Icon_Slime.png",
	L"Graphics\\CraftIcons\\icon_structures.png",
	L"Graphics\\CraftIcons\\icon_tools.png",
};



//-----------------------------------------------------------------------------
CXuiCtrlMinecraftSlot::CXuiCtrlMinecraftSlot() :
	//m_hBrush(NULL),
	m_bDirty(FALSE),
		m_iPassThroughDataAssociation(0),
		m_iPassThroughIdAssociation(0),
		m_fScale(1.0f),
		m_fAlpha(1.0f),
		m_iID(0),
		m_iCount(0),
		m_iAuxVal(0),
		m_bDecorations(false),
		m_isFoil(false),
		m_popTime(0)
{
	m_pItemRenderer = new ItemRenderer();
	m_item = nullptr;

	m_bScreenWidthSetup = false;

	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(pMinecraft != NULL)
	{
		m_fScreenWidth=(float)pMinecraft->width_phys;
		m_fScreenHeight=(float)pMinecraft->height_phys;
		m_bScreenWidthSetup = true;
	}
}

CXuiCtrlMinecraftSlot::~CXuiCtrlMinecraftSlot()
{
	delete m_pItemRenderer;
}

VOID CXuiCtrlMinecraftSlot::SetPassThroughDataAssociation(unsigned int iID, unsigned int iData)
{
	m_item = nullptr;
	m_iPassThroughIdAssociation = iID;
	m_iPassThroughDataAssociation = iData;
}

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlMinecraftSlot::OnGetSourceImage(XUIMessageGetSourceImage* pData, BOOL& rfHandled)
{
	XUIMessage Message;
	CustomMessage_GetSlotItem_Struct MsgGetSlotItem;
	HRESULT hr;
	HXUIOBJ hObj;

	CustomMessage_GetSlotItem(&Message, &MsgGetSlotItem, m_iPassThroughIdAssociation, m_iPassThroughDataAssociation);

	hr = GetParent(&hObj);

	if (HRESULT_SUCCEEDED(hr))
	{
		hr = XuiBubbleMessage(hObj, &Message);

		if(hr == XUI_ERR_SOURCEDATA_ITEM)
		{
			// Go up the parent chain one more
			HXUIOBJ hParent;
			hr = XuiElementGetParent(hObj,&hParent);
			hr = XuiBubbleMessage(hParent, &Message);
		}

		if (Message.bHandled)
		{
			pData->szPath = MsgGetSlotItem.szPath;
			pData->bDirty = MsgGetSlotItem.bDirty;

			if(MsgGetSlotItem.item != NULL)
			{
				m_item = MsgGetSlotItem.item;
				m_iID = m_item->id;
				m_iPad	= GET_SLOTDISPLAY_USERINDEX_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);
				m_fAlpha		= ((float)GET_SLOTDISPLAY_ALPHA_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField))/31.0f;
				m_bDecorations	= GET_SLOTDISPLAY_DECORATIONS_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);
				m_fScale		= ((float)GET_SLOTDISPLAY_SCALE_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField))/10.0f;
			}
			else
			{
				m_iID = GET_SLOTDISPLAY_ID_FROM_ITEM_BITMASK(MsgGetSlotItem.iItemBitField);

				// if the id is greater than or equal to 32000, then it's an xzp icon, not a game icon
				if(m_iID<32000)
				{		
					// 4J Stu - Some parent controls may overide this, others will leave it as what we passed in

					m_iPad	= GET_SLOTDISPLAY_USERINDEX_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);
					m_fAlpha		= ((float)GET_SLOTDISPLAY_ALPHA_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField))/31.0f;
					m_bDecorations	= GET_SLOTDISPLAY_DECORATIONS_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);
					m_iCount		= GET_SLOTDISPLAY_COUNT_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);
					m_fScale		= ((float)GET_SLOTDISPLAY_SCALE_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField))/10.0f;
					m_popTime		= GET_SLOTDISPLAY_POPTIME_FROM_DATA_BITMASK(MsgGetSlotItem.iDataBitField);

					m_iAuxVal		= GET_SLOTDISPLAY_AUXVAL_FROM_ITEM_BITMASK(MsgGetSlotItem.iItemBitField);

					//m_iID		= iID;

					m_isFoil	= GET_SLOTDISPLAY_FOIL_FROM_ITEM_BITMASK(MsgGetSlotItem.iItemBitField);
				}
				else
				{
					pData->szPath = xzpIcons[m_iID-32000];
				}

				if(m_item != NULL && (m_item->id != m_iID || m_item->getAuxValue() != m_iAuxVal || m_item->GetCount() != m_iCount) ) m_item = nullptr;
			}


			rfHandled = TRUE;
			return hr;
		}
		else
		{
			pData->szPath = L"";
		}
	}

	pData->bDirty = m_bDirty;
	m_bDirty = FALSE;
	rfHandled = TRUE;
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CXuiCtrlMinecraftSlot::OnInit(XUIMessageInit* pInitData, BOOL& rfHandled)
{
	//DWORD dwPropId;
	HRESULT hr=S_OK;
	return hr;
}

//-----------------------------------------------------------------------------

HRESULT CXuiCtrlMinecraftSlot::OnRender(XUIMessageRender *pRenderData, BOOL &bHandled )
{
	// Force an update of the id
	XUIMessage Message;
	XUIMessageGetSourceImage MsgGetImage;
	HRESULT hr;
	XuiMessageGetSourceImage(&Message, &MsgGetImage, m_iPassThroughIdAssociation, m_iPassThroughDataAssociation, FALSE);
	hr = XuiSendMessage(m_hObj, &Message);

	// We cannot have an Item with id 0
	if(m_item != NULL || (m_iID > 0 && m_iID<32000) )
	{
		HXUIDC hDC = pRenderData->hDC;
		CXuiControl xuiControl(m_hObj);
		if(m_item == NULL) m_item = shared_ptr<ItemInstance>( new ItemInstance(m_iID, m_iCount, m_iAuxVal) );

		// build and render with the game call

		RenderManager.Set_matrixDirty();

		Minecraft *pMinecraft=Minecraft::GetInstance();

		D3DXMATRIX matrix;
		xuiControl.GetFullXForm(&matrix);
		float bwidth,bheight;
		xuiControl.GetBounds(&bwidth,&bheight);

		float x = matrix._41;
		float y = matrix._42;

		// Base scale on height of this control, compared to height of what the item renderer normally renders (16 pixels high). Potentially
		// we might want separate x & y scales here

		float scaleX = bwidth / 16.0f;
		float scaleY = bheight / 16.0f;				

		// apply any scale in the matrix too
		scaleX *= matrix._11;
		scaleY *= matrix._22;					

		// Annoyingly, XUI renders everything to a z of 0 so if we want to render anything that needs the z-buffer on top of it, then we need to clear it.
		// Clear just the region required for this control.
		D3DRECT clearRect; 
		clearRect.x1 = (int)(matrix._41) - 2;
		clearRect.y1 = (int)(matrix._42) - 2;
		clearRect.x2 = (int)(matrix._41 + ( bwidth  * matrix._11 )) + 2;
		clearRect.y2 = (int)(matrix._42 + ( bheight * matrix._22 )) + 2;

		if(!m_bScreenWidthSetup)
		{
			Minecraft *pMinecraft=Minecraft::GetInstance();
			if(pMinecraft != NULL)
			{
				m_fScreenWidth=(float)pMinecraft->width_phys;
				m_fScreenHeight=(float)pMinecraft->height_phys;
				m_bScreenWidthSetup = true;
			}
		}

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
		Lighting::turnOn();
		glPopMatrix();



		//Make sure that pMinecraft->player is the correct player so that player specific rendering
		// eg clock and compass, are rendered correctly
		shared_ptr<MultiplayerLocalPlayer> oldPlayer = pMinecraft->player;

		if( m_iPad >= 0 && m_iPad < XUSER_MAX_COUNT ) pMinecraft->player = pMinecraft->localplayers[m_iPad];

		float pop = m_popTime;
		if (pop > 0)
		{
			glPushMatrix();
			float squeeze = 1 + pop / (float) Inventory::POP_TIME_DURATION;
			float sx = x;
			float sy = y;
			float sxoffs = 8 * scaleX;
			float syoffs = 12 * scaleY;
			glTranslatef((float)(sx + sxoffs), (float)(sy + syoffs), 0);
			glScalef(1 / squeeze, (squeeze + 1) / 2, 1);
			glTranslatef((float)-(sx + sxoffs), (float)-(sy + syoffs), 0);
		}

		m_pItemRenderer->renderAndDecorateItem(pMinecraft->font, pMinecraft->textures, m_item, x, y,scaleX,scaleY,m_fAlpha,m_isFoil,false);

		if (pop > 0)
		{
			glPopMatrix();
		}

		if(m_bDecorations)
		{
			if((scaleX!=1.0f) ||(scaleY!=1.0f))
			{				
				glPushMatrix();		
				glScalef(scaleX, scaleY, 1.0f);
				int iX= (int)(0.5f+((float)x)/scaleX);
				int iY= (int)(0.5f+((float)y)/scaleY);

				m_pItemRenderer->renderGuiItemDecorations(pMinecraft->font, pMinecraft->textures, m_item, iX, iY, m_fAlpha);
				glPopMatrix();
			}
			else
			{
				m_pItemRenderer->renderGuiItemDecorations(pMinecraft->font, pMinecraft->textures, m_item, (int)x, (int)y, m_fAlpha);
			}
		}

		pMinecraft->player = oldPlayer;

		Lighting::turnOff();
		glDisable(GL_RESCALE_NORMAL);

		XuiRenderRestoreState(hDC);

		bHandled = TRUE;
	}
	return S_OK;
}


void CXuiCtrlMinecraftSlot::SetIcon(int iPad, int iId,int iAuxVal, int iCount, int iScale, unsigned int uiAlpha,bool bDecorations,BOOL bShow, bool isFoil)
{
	m_item = nullptr;
	m_iID=iId;
	m_iAuxVal=iAuxVal;

	// aux value for diggers can go as high as 1561
	//const _Tier *_Tier::DIAMOND = new _Tier(3, 1561, 8, 3); //
	// setMaxDamage(tier->getUses()); 

	// 	int ItemInstance::getDamageValue()
	// 	{
	// 		return auxValue;
	// 	}


	if( (m_iAuxVal & 0xFF) == 0xFF) // 4J Stu - If the aux value is set to match any
		m_iAuxVal = 0;

	m_iCount=iCount;
	m_fScale	= (float)(iScale)/10.0f;
	//m_uiAlpha=uiAlpha;
	m_fAlpha       =((float)(uiAlpha)) / 255.0f;
	m_bDecorations = bDecorations;
	// 	mif(bDecorations) m_iDecorations=1;
	// 	else m_iDecorations=0;

	m_iPad = iPad;

	m_isFoil = isFoil;

	XuiElementSetShow(m_hObj,bShow);
}

void CXuiCtrlMinecraftSlot::SetIcon(int iPad, shared_ptr<ItemInstance> item, int iScale, unsigned int uiAlpha,bool bDecorations, BOOL bShow)
{
	m_item = item;
	m_isFoil = item->isFoil();
	m_iPad = iPad;
	m_fScale = (float)(iScale)/10.0f;
	m_fAlpha =((float)(uiAlpha)) / 31;
	m_bDecorations = bDecorations;
	m_bDirty = TRUE;
	XuiElementSetShow(m_hObj,bShow);
}
