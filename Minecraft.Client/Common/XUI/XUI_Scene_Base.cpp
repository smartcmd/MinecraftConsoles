#include "stdafx.h"

#include <assert.h>
#include "../../MultiPlayerLevel.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../StatsCounter.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/net.minecraft.stats.h"
#include "../../Minecraft.h"

#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "../../../Minecraft.World/LevelData.h"
#include "XUI_CustomMessages.h"
#include "../../../Minecraft.World/Dimension.h"
#include "../../../Minecraft.World/SharedConstants.h"
#include "../../GameMode.h"
#include "../../EnderDragonRenderer.h"
#include "../../../Minecraft.World/net.minecraft.world.entity.boss.enderdragon.h"
#include "../../TexturePackRepository.h"
#include "../../TexturePack.h"
#include "../../DLCTexturePack.h"

#define PRESS_START_TIMER 0

CXuiSceneBase *CXuiSceneBase::Instance = nullptr;
DWORD CXuiSceneBase::m_dwTrialTimerLimitSecs=DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME;
//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CXuiSceneBase::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	ASSERT( CXuiSceneBase::Instance == nullptr );
	CXuiSceneBase::Instance = this;

	m_iWrongTexturePackTickC=20*5; // default 5 seconds before bringing up the upsell for not having the texture pack
	MapChildControls();

	// Display the tooltips
	HRESULT hr = S_OK;
	CXuiElement xuiElement = m_hObj;
	HXUIOBJ hTemp;


	m_hEmptyQuadrantLogo=nullptr;
	XuiElementGetChildById(m_hObj,L"EmptyQuadrantLogo",&m_hEmptyQuadrantLogo);

	D3DXVECTOR3 lastPos;
	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		for( unsigned int i = 0; i < BUTTONS_TOOLTIP_MAX; ++i )
		{
			m_visible[idx][ i ] = FALSE;
			m_iCurrentTooltipTextID[idx][i]=-1;
			hTooltipText[idx][i]=nullptr;
			hTooltipTextSmall[idx][i]=nullptr;
			// set all tooltips to shown FALSE by default
			m_Buttons[idx][i].SetShow( FALSE );
			m_ButtonsSmall[idx][i].SetShow( FALSE );
		}

		XuiElementGetPosition( m_bottomLeftAnchorPoint[idx].m_hObj, &lastPos);
		lastPos.y-=110;

		m_bCrouching[idx]=false;
		m_uiSelectedItemOpacityCountDown[idx] =0;
		m_bossHealthVisible[idx] = FALSE;

		switch(idx)
		{
		case 0:
			XuiElementGetChildById(m_hObj,L"BasePlayer0",&hTemp);
			XuiElementGetChildById(hTemp,L"XuiGamertag",&m_hGamerTagA[0]);
			break;
		case 1:
			XuiElementGetChildById(m_hObj,L"BasePlayer1",&hTemp);
			XuiElementGetChildById(hTemp,L"XuiGamertag",&m_hGamerTagA[1]);
			break;
		case 2:
			XuiElementGetChildById(m_hObj,L"BasePlayer2",&hTemp);
			XuiElementGetChildById(hTemp,L"XuiGamertag",&m_hGamerTagA[2]);
			break;
		case 3:
			XuiElementGetChildById(m_hObj,L"BasePlayer3",&hTemp);
			XuiElementGetChildById(hTemp,L"XuiGamertag",&m_hGamerTagA[3]);
			break;
		}
	}

	m_ticksWithNoBoss = 0;

	UpdatePlayerBasePositions();

	m_iQuadrantsMask=0;

	return S_OK;
}

HRESULT CXuiSceneBase::OnTimer(XUIMessageTimer *pData,BOOL& rfHandled)
{
	if(pData->nId==PRESS_START_TIMER)
	{
		XuiKillTimer(m_hObj,PRESS_START_TIMER);
		XuiElementStopTimeline(m_hObj,TRUE);
		m_PressStart.SetShow(FALSE);

		// clear the quadrants
		m_iQuadrantsMask=0;

		HXUIOBJ hObj=nullptr,hQuadrant;

		HRESULT hr=XuiControlGetVisual(m_PressStart.m_hObj,&hObj);

		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			switch(i)
			{
			case 0:
				hr=XuiElementGetChildById(hObj,L"Quadrant1",&hQuadrant);
				XuiElementSetShow(hQuadrant,FALSE);
				break;
			case 1:
				hr=XuiElementGetChildById(hObj,L"Quadrant2",&hQuadrant);
				XuiElementSetShow(hQuadrant,FALSE);
				break;
			case 2:
				hr=XuiElementGetChildById(hObj,L"Quadrant3",&hQuadrant);
				XuiElementSetShow(hQuadrant,FALSE);
				break;
			case 3:
				hr=XuiElementGetChildById(hObj,L"Quadrant4",&hQuadrant);
				XuiElementSetShow(hQuadrant,FALSE);
				break;
			}
		}
	}

	return S_OK;
}

HRESULT CXuiSceneBase::OnSkinChanged(BOOL& bHandled)
{
	// Clear visuals held for tooltips
	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		for( unsigned int i = 0; i < BUTTONS_TOOLTIP_MAX; ++i )
		{
			hTooltipText[idx][i]=nullptr;
			hTooltipTextSmall[idx][i]=nullptr;
		}
	}

	// Don't set to handled

	return S_OK;
}

void CXuiSceneBase::_TickAllBaseScenes()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	// 4J-PB - added to upsell the texture pack if you join from invite and don't have it
	// what texture pack are we using?
	bool bCheckTexturePack=false;

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		bCheckTexturePack= app.GetGameStarted() && (ui.GetMenuDisplayed(0)==false) && (app.GetXuiAction(i)==eAppAction_Idle);
	}

	// make sure there's not a mount going on before using the textures
	if(bCheckTexturePack && app.DLCInstallProcessCompleted() )
	{
		TexturePack *tPack = pMinecraft->skins->getSelected();

		if(tPack->getId()!=app.GetRequiredTexturePackID())
		{
			// we're not using the texture pack we need

			//Is it available?
			TexturePack * pRequiredTPack=pMinecraft->skins->getTexturePackById(app.GetRequiredTexturePackID());
			if(pRequiredTPack!=nullptr)
			{
				// we can switch to the required pack
				// reset the timer
				m_iWrongTexturePackTickC=20*60*5; // reset to 5 minutes

				pMinecraft->skins->selectTexturePackById(app.GetRequiredTexturePackID());

				// probably had background downloads enabled, so turn them off
				XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE_AUTO);
			}
			else
			{
				// decrement the counter
				m_iWrongTexturePackTickC--;

				if(m_iWrongTexturePackTickC==0)
				{
					// action
					app.SetAction(ProfileManager.GetPrimaryPad(),eAppAction_TexturePackRequired);

					// reset the timer
					m_iWrongTexturePackTickC=20*60*5; // reset to 5 minutes
				}
			}
		}
	}

	if (EnderDragonRenderer::bossInstance == nullptr)
	{
		if(m_ticksWithNoBoss<=20)
		{
			++m_ticksWithNoBoss;
		}
	}
	else
	{
		shared_ptr<EnderDragon> boss = EnderDragonRenderer::bossInstance;
		EnderDragonRenderer::bossInstance = nullptr;
		m_ticksWithNoBoss = 0;

		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if(pMinecraft->localplayers[i] != nullptr && pMinecraft->localplayers[i]->dimension == 1 && !ui.GetMenuDisplayed(i) && app.GetGameSettings(i,eGameSetting_DisplayHUD))
			{
				int iGuiScale;

				if(pMinecraft->localplayers[i]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
				{
					iGuiScale=app.GetGameSettings(i,eGameSetting_UISize);
				}
				else
				{
					iGuiScale=app.GetGameSettings(i,eGameSetting_UISizeSplitscreen);
				}
				m_BossHealthGroup[i].SetShow(TRUE);
				m_BossHealthText[i].SetText( app.GetString( IDS_BOSS_ENDERDRAGON_HEALTH ) );

				if(pMinecraft->localplayers[i]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
				{
					switch(iGuiScale)
					{
					case 0:
						m_pBossHealthProgress = m_BossHealthProgress1;
						m_BossHealthProgress1[i].SetShow(TRUE);
						m_BossHealthProgress2[i].SetShow(FALSE);
						m_BossHealthProgress3[i].SetShow(FALSE);
						if(m_BossHealthProgress1_small[i]!=nullptr)
						{
							m_BossHealthProgress1_small[i].SetShow(FALSE);
							m_BossHealthProgress2_small[i].SetShow(FALSE);
							m_BossHealthProgress3_small[i].SetShow(FALSE);
						}


						break;
					case 1:
						m_pBossHealthProgress = m_BossHealthProgress2;
						m_BossHealthProgress1[i].SetShow(FALSE);
						m_BossHealthProgress2[i].SetShow(TRUE);
						m_BossHealthProgress3[i].SetShow(FALSE);
						if(m_BossHealthProgress1_small[i]!=nullptr)
						{
							m_BossHealthProgress1_small[i].SetShow(FALSE);
							m_BossHealthProgress2_small[i].SetShow(FALSE);
							m_BossHealthProgress3_small[i].SetShow(FALSE);
						}

						break;
					case 2:
						m_pBossHealthProgress = m_BossHealthProgress3;
						m_BossHealthProgress1[i].SetShow(FALSE);
						m_BossHealthProgress2[i].SetShow(FALSE);
						m_BossHealthProgress3[i].SetShow(TRUE);
						if(m_BossHealthProgress1_small[i]!=nullptr)
						{
							m_BossHealthProgress1_small[i].SetShow(FALSE);
							m_BossHealthProgress2_small[i].SetShow(FALSE);
							m_BossHealthProgress3_small[i].SetShow(FALSE);
						}

						break;
					}
				}
				else
				{
					// if we have 2 player top & bottom, we can use the fullscreen bar
					if((pMinecraft->localplayers[i]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_TOP) ||
						(pMinecraft->localplayers[i]->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM))
					{
						switch(iGuiScale)
						{
						case 0:
							m_pBossHealthProgress = m_BossHealthProgress1;
							m_BossHealthProgress1[i].SetShow(TRUE);
							m_BossHealthProgress2[i].SetShow(FALSE);
							m_BossHealthProgress3[i].SetShow(FALSE);
							if(m_BossHealthProgress1_small[i]!=nullptr)
							{
								m_BossHealthProgress1_small[i].SetShow(FALSE);
								m_BossHealthProgress2_small[i].SetShow(FALSE);
								m_BossHealthProgress3_small[i].SetShow(FALSE);
							}

							break;
						case 1:
							m_pBossHealthProgress = m_BossHealthProgress2;
							m_BossHealthProgress1[i].SetShow(FALSE);
							m_BossHealthProgress2[i].SetShow(TRUE);
							m_BossHealthProgress3[i].SetShow(FALSE);
							if(m_BossHealthProgress1_small[i]!=nullptr)
							{
								m_BossHealthProgress1_small[i].SetShow(FALSE);
								m_BossHealthProgress2_small[i].SetShow(FALSE);
								m_BossHealthProgress3_small[i].SetShow(FALSE);
							}
							break;
						case 2:
							m_pBossHealthProgress = m_BossHealthProgress3;
							m_BossHealthProgress1[i].SetShow(FALSE);
							m_BossHealthProgress2[i].SetShow(FALSE);
							m_BossHealthProgress3[i].SetShow(TRUE);
							if(m_BossHealthProgress1_small[i]!=nullptr)
							{
								m_BossHealthProgress1_small[i].SetShow(FALSE);
								m_BossHealthProgress2_small[i].SetShow(FALSE);
								m_BossHealthProgress3_small[i].SetShow(FALSE);
							}
							break;
						}
					}
					else
					{
						// use the small versions
						switch(iGuiScale)
						{
						case 0:
							m_pBossHealthProgress = m_BossHealthProgress1_small;
							m_BossHealthProgress1_small[i].SetShow(TRUE);
							m_BossHealthProgress2_small[i].SetShow(FALSE);
							m_BossHealthProgress3_small[i].SetShow(FALSE);
							m_BossHealthProgress1[i].SetShow(FALSE);
							m_BossHealthProgress2[i].SetShow(FALSE);
							m_BossHealthProgress3[i].SetShow(FALSE);

							break;
						case 1:
							m_pBossHealthProgress = m_BossHealthProgress2_small;
							m_BossHealthProgress1_small[i].SetShow(FALSE);
							m_BossHealthProgress2_small[i].SetShow(TRUE);
							m_BossHealthProgress3_small[i].SetShow(FALSE);
							m_BossHealthProgress1[i].SetShow(FALSE);
							m_BossHealthProgress2[i].SetShow(FALSE);
							m_BossHealthProgress3[i].SetShow(FALSE);
							break;
						case 2:
							m_pBossHealthProgress = m_BossHealthProgress3_small;
							m_BossHealthProgress1_small[i].SetShow(FALSE);
							m_BossHealthProgress2_small[i].SetShow(FALSE);
							m_BossHealthProgress3_small[i].SetShow(TRUE);
							m_BossHealthProgress1[i].SetShow(FALSE);
							m_BossHealthProgress2[i].SetShow(FALSE);
							m_BossHealthProgress3[i].SetShow(FALSE);
							break;
						}
					}
				}

				m_pBossHealthProgress[i].SetRange(0, boss->getMaxHealth() );
				m_pBossHealthProgress[i].SetValue( boss->getSynchedHealth() );
				m_bossHealthVisible[i] = TRUE;

				_UpdateSelectedItemPos(i);
			}
			else if( m_bossHealthVisible[i] == TRUE)
			{
				m_BossHealthGroup[i].SetShow(FALSE);
				m_bossHealthVisible[i] = FALSE;

				_UpdateSelectedItemPos(i);
			}
		}
	}

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(m_uiSelectedItemOpacityCountDown[i] > 0) --m_uiSelectedItemOpacityCountDown[i];
		if( m_ticksWithNoBoss > 20 )
		{
			m_BossHealthGroup[i].SetShow(FALSE);
			m_bossHealthVisible[i] = FALSE;

			_UpdateSelectedItemPos(i);
		}

		// check if we have the timer running for the opacity
		unsigned int uiOpacityTimer=m_uiSelectedItemOpacityCountDown[i];

		if(uiOpacityTimer>0 && !ui.GetMenuDisplayed(i) && app.GetGameStarted())
		{
			if(uiOpacityTimer < (SharedConstants::TICKS_PER_SECOND * 1) )
			{
				float fStep=(80.0f)/10.0f;
				float fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));

				XuiElementSetOpacity(m_selectedItemA[i],fVal);
				XuiElementSetOpacity(m_selectedItemSmallA[i],fVal);
			}

			if( m_playerBaseScenePosition[i] == e_BaseScene_Fullscreen )
			{
				m_selectedItemA[i].SetShow(TRUE);
				m_selectedItemSmallA[i].SetShow(FALSE);
			}
			else
			{
				m_selectedItemA[i].SetShow(FALSE);
				m_selectedItemSmallA[i].SetShow(TRUE);
			}
		}
		else
		{
			m_selectedItemA[i].SetShow(FALSE);
			m_selectedItemSmallA[i].SetShow(FALSE);
		}

		unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
		float fVal;

		if(ucAlpha<80)
		{
			// if we are in a menu, set the minimum opacity for tooltips to 15%
			if(ui.GetMenuDisplayed(i) && (ucAlpha<15))
			{
				ucAlpha=15;
			}

			// check if we have the timer running for the opacity
			unsigned int uiOpacityTimer=app.GetOpacityTimer(i);
			if(uiOpacityTimer!=0)
			{
				if(uiOpacityTimer<10)
				{
					float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
					fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
				}
				else
				{
					fVal=0.01f*80.0f;
				}
			}
			else
			{
				fVal=0.01f*static_cast<float>(ucAlpha);
			}
		}
		else
		{
			// if we are in a menu, set the minimum opacity for tooltips to 15%
			if(ui.GetMenuDisplayed(i) && (ucAlpha<15))
			{
				ucAlpha=15;
			}
			fVal=0.01f*static_cast<float>(ucAlpha);
		}
		XuiElementSetOpacity(app.GetCurrentHUDScene(i),fVal);

		XUIMessage xuiMsg;
		CustomMessage_TickScene( &xuiMsg );
		XuiSendMessage( app.GetCurrentHUDScene(i), &xuiMsg );

		bool bDisplayGui=app.GetGameStarted() && !ui.GetMenuDisplayed(i) && !(app.GetXuiAction(i)==eAppAction_AutosaveSaveGameCapturedThumbnail) && app.GetGameSettings(i,eGameSetting_DisplayHUD)!=0;
		if(bDisplayGui && pMinecraft->localplayers[i] != nullptr)
		{
			XuiElementSetShow(app.GetCurrentHUDScene(i),TRUE);
		}
		else
		{
			XuiElementSetShow(app.GetCurrentHUDScene(i),FALSE);
		}
	}
}

HRESULT CXuiSceneBase::_SetEnableTooltips( unsigned int iPad, BOOL bVal )
{
	for(int i=0;i<BUTTONS_TOOLTIP_MAX;i++)
	{
		//XuiElementSetShow(m_Buttons[iPad][i].m_hObj,bVal);
		XuiControlSetEnable(m_Buttons[iPad][i].m_hObj,bVal);
	}
	return S_OK;
}


HRESULT CXuiSceneBase::_SetTooltipText( unsigned int iPad, unsigned int uiTooltip, int iTextID )
{
	ASSERT( uiTooltip < BUTTONS_TOOLTIP_MAX );

	XUIRect xuiRect, xuiRectSmall;
	HRESULT hr=S_OK;
	LPCWSTR pString=nullptr;
	float fWidth,fHeight;

	// Want to be able to show just a button (for RB LB)
	if(iTextID>=0)
	{
		pString=app.GetString(iTextID);
	}
	if(hTooltipText[iPad][uiTooltip]==nullptr)
	{
 		HXUIOBJ hObj=nullptr;
 		hr=XuiControlGetVisual(m_Buttons[iPad][uiTooltip].m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"text_ButtonText",&hTooltipText[iPad][uiTooltip]);
		hr=XuiElementGetPosition(hTooltipText[iPad][uiTooltip],&m_vPosTextInTooltip[uiTooltip]);
	}
 
	if(hTooltipTextSmall[iPad][uiTooltip]==nullptr)
	{
 		HXUIOBJ hObj=nullptr;
 		hr=XuiControlGetVisual(m_ButtonsSmall[iPad][uiTooltip].m_hObj,&hObj);
		hr=XuiElementGetChildById(hObj,L"text_ButtonText",&hTooltipTextSmall[iPad][uiTooltip]);
		hr=XuiElementGetPosition(hTooltipTextSmall[iPad][uiTooltip],&m_vPosTextInTooltipSmall[uiTooltip]);
	}

	if(iTextID>=0)
	{
		hr=XuiTextPresenterMeasureText(hTooltipText[iPad][uiTooltip], pString, &xuiRect);

		// Change the size of the whole button to be the width of the measured text, plus the position the text element starts in the visual (which is the offset by the size of the button graphic)
		XuiElementGetBounds(m_Buttons[iPad][uiTooltip].m_hObj,&fWidth, &fHeight);
		XuiElementSetBounds(m_Buttons[iPad][uiTooltip].m_hObj,xuiRect.right+1+m_vPosTextInTooltip[uiTooltip].x,fHeight);

		// Change the width of the text element to be the width of the measured text
		XuiElementGetBounds(hTooltipText[iPad][uiTooltip],&fWidth, &fHeight);
		XuiElementSetBounds(hTooltipText[iPad][uiTooltip],xuiRect.right,fHeight);


		hr=XuiTextPresenterMeasureText(hTooltipTextSmall[iPad][uiTooltip], pString, &xuiRectSmall);

		// Change the size of the whole button to be the width of the measured text, plus the position the text element starts in the visual (which is the offset by the size of the button graphic)
		XuiElementGetBounds(m_ButtonsSmall[iPad][uiTooltip].m_hObj,&fWidth, &fHeight);
		XuiElementSetBounds(m_ButtonsSmall[iPad][uiTooltip].m_hObj,xuiRectSmall.right+1+m_vPosTextInTooltipSmall[uiTooltip].x,fHeight);

		// Change the width of the text element to be the width of the measured text
		XuiElementGetBounds(hTooltipTextSmall[iPad][uiTooltip],&fWidth, &fHeight);
		XuiElementSetBounds(hTooltipTextSmall[iPad][uiTooltip],xuiRectSmall.right,fHeight);

		m_Buttons[iPad][uiTooltip].SetText(pString);
		m_ButtonsSmall[iPad][uiTooltip].SetText(pString);
	}
	else
	{
		m_Buttons[iPad][uiTooltip].SetText(L"");
		XuiElementGetBounds(m_Buttons[iPad][uiTooltip].m_hObj,&fWidth, &fHeight);
		XuiElementSetBounds(m_Buttons[iPad][uiTooltip].m_hObj,m_vPosTextInTooltip[uiTooltip].x,fHeight);

		m_ButtonsSmall[iPad][uiTooltip].SetText(L"");
		XuiElementGetBounds(m_ButtonsSmall[iPad][uiTooltip].m_hObj,&fWidth, &fHeight);
		XuiElementSetBounds(m_ButtonsSmall[iPad][uiTooltip].m_hObj,m_vPosTextInTooltipSmall[uiTooltip].x,fHeight);
	}

	m_iCurrentTooltipTextID[iPad][uiTooltip]=iTextID;

	ReLayout(iPad);

	return hr;
}

HRESULT CXuiSceneBase::_RefreshTooltips( unsigned int iPad)
{
	// if the tooltip is showing, refresh it to update the opacity
	for(int tooltip=0;tooltip<BUTTONS_TOOLTIP_MAX;tooltip++)
	{
		if(m_Buttons[iPad][tooltip].IsShown()==TRUE)
		{
			_ShowTooltip(iPad,tooltip,TRUE);
		}
	}

	return S_OK;
}

HRESULT CXuiSceneBase::_ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show )
{
	ASSERT( tooltip < BUTTONS_TOOLTIP_MAX );
	HRESULT hr;

	if ( (ProfileManager.GetLockedProfile()!=-1) && app.GetGameSettings(iPad,eGameSetting_DisplayHUD)==0 )
	{
		//hr = m_Buttons[iPad][tooltip].SetShow( FALSE );
		//hr = m_ButtonsSmall[iPad][tooltip].SetShow( FALSE );

		// turn off gamertag display in splitscreens
		XuiElementSetShow(m_hGamerTagA[iPad],FALSE);

		//m_visible[iPad][ tooltip ] = false;
	}
	//else
	{
		// check the app setting first (only if there is a player)
		if((ProfileManager.GetLockedProfile()==-1) || ui.GetMenuDisplayed(iPad) || (app.GetGameSettings(iPad,eGameSetting_Tooltips)==1 && app.GetGameSettings(iPad,eGameSetting_DisplayHUD)!=0))
		{
			hr = m_Buttons[iPad][tooltip].SetShow( show?TRUE:FALSE );
			hr = m_ButtonsSmall[iPad][tooltip].SetShow( show?TRUE:FALSE  );

			m_visible[iPad][ tooltip ] = show;

			// if we have a player, use their opacity to set the tooltip
			if(show && (ProfileManager.GetLockedProfile()!=-1))
			{
				// set the opacity of the tooltip items
				unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
				float fVal;

				if(ucAlpha<80)
				{
					// if we are in a menu, set the minimum opacity for tooltips to 15%
					if(ui.GetMenuDisplayed(iPad) && (ucAlpha<15))
					{
						ucAlpha=15;
					}

					// check if we have the timer running for the opacity
					unsigned int uiOpacityTimer=app.GetOpacityTimer(iPad);
					if(uiOpacityTimer!=0)
					{
						if(uiOpacityTimer<10)
						{
							float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
							fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
						}
						else
						{
							fVal=0.01f*80.0f;
						}
					}
					else
					{
						fVal=0.01f*static_cast<float>(ucAlpha);
					}
				}
				else
				{
					// if we are in a menu, set the minimum opacity for tooltips to 15%
					if(ui.GetMenuDisplayed(iPad) && (ucAlpha<15))
					{
						ucAlpha=15;
					}
					fVal=0.01f*static_cast<float>(ucAlpha);
				}

				m_Buttons[iPad][tooltip].SetOpacity(fVal);
				m_ButtonsSmall[iPad][tooltip].SetOpacity(fVal);
			}
		}
		else
		{
			hr = m_Buttons[iPad][tooltip].SetShow( FALSE );
			hr = m_ButtonsSmall[iPad][tooltip].SetShow( FALSE  );

			m_visible[iPad][ tooltip ] = false;
		}

		//also set the gamertags and the 4th quadrant logo
		if(ProfileManager.GetLockedProfile()!=-1)
		{
			if(app.GetGameSettings(iPad,eGameSetting_DisplaySplitscreenGamertags)==1)
			{
				unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
				float fVal;

				if(ucAlpha<80)
				{
					// check if we have the timer running for the opacity
					unsigned int uiOpacityTimer=app.GetOpacityTimer(iPad);
					if(uiOpacityTimer!=0)
					{
						if(uiOpacityTimer<10)
						{
							float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
							fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
						}
						else
						{
							fVal=0.01f*80.0f;
						}
					}
					else
					{
						fVal=0.01f*static_cast<float>(ucAlpha);
					}
				}
				else
				{
					fVal=0.01f*static_cast<float>(ucAlpha);
				}
				XuiElementSetOpacity(m_hGamerTagA[iPad],fVal);
			}

			if(iPad==ProfileManager.GetPrimaryPad())
			{
				unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
				XuiElementSetOpacity(m_hEmptyQuadrantLogo,0.01f*static_cast<float>(ucAlpha));
			}
		}
	}

	ReLayout(iPad);

	return hr;
}

HRESULT CXuiSceneBase::_ShowSafeArea( BOOL bShow )
{
	return m_SafeArea.SetShow(bShow);
}

HRESULT CXuiSceneBase::_ShowOtherPlayersBaseScene(int iPad, bool show)
{
	for( int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		if( i == iPad )
		{
			m_BasePlayerScene[i].SetShow(TRUE);

			// Fix for #61051 - TU7: Content: UI: Player specific unresponsive state may be triggered during the Save Game process in the splitscreen mode
			m_BasePlayerScene[i].EnableInput(TRUE);
		}
		else
		{
			m_BasePlayerScene[i].SetShow(show?TRUE:FALSE);

			// Fix for #61051 - TU7: Content: UI: Player specific unresponsive state may be triggered during the Save Game process in the splitscreen mode
			m_BasePlayerScene[i].EnableInput(show?TRUE:FALSE);
		}
	}
	return S_OK;
}

HRESULT CXuiSceneBase::_SetTooltipsEnabled( unsigned int iPad, bool bA, bool bB, bool bX, bool bY, bool bLT, bool bRT, bool bLB, bool bRB, bool bLS)
{
	m_Buttons[iPad][BUTTON_TOOLTIP_A].SetEnable( bA );
	m_Buttons[iPad][BUTTON_TOOLTIP_B].SetEnable( bB );
	m_Buttons[iPad][BUTTON_TOOLTIP_X].SetEnable( bX );
	m_Buttons[iPad][BUTTON_TOOLTIP_Y].SetEnable( bY );
	m_Buttons[iPad][BUTTON_TOOLTIP_LT].SetEnable( bLT );
	m_Buttons[iPad][BUTTON_TOOLTIP_RT].SetEnable( bRT );
	m_Buttons[iPad][BUTTON_TOOLTIP_LB].SetEnable( bLB );
	m_Buttons[iPad][BUTTON_TOOLTIP_RB].SetEnable( bRB );
	m_Buttons[iPad][BUTTON_TOOLTIP_LS].SetEnable( bLS );

	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_A].SetEnable( bA );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_B].SetEnable( bB );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_X].SetEnable( bX );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_Y].SetEnable( bY );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_LT].SetEnable( bLT );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_RT].SetEnable( bRT );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_LB].SetEnable( bLB );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_RB].SetEnable( bRB );
	m_ButtonsSmall[iPad][BUTTON_TOOLTIP_LS].SetEnable( bLS );
	return S_OK;
}

HRESULT CXuiSceneBase::_EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable )
{
	ASSERT( tooltip < BUTTONS_TOOLTIP_MAX );

	m_Buttons[iPad][tooltip].SetEnable( enable );

	return S_OK;
}

HRESULT CXuiSceneBase::_AnimateKeyPress(DWORD userIndex, DWORD dwKeyCode)
{
	if(m_playerBaseScenePosition[userIndex] == e_BaseScene_NotSet)
	{
		userIndex = DEFAULT_XUI_MENU_USER;
	}
	switch(dwKeyCode)
	{
		case VK_PAD_A:
			m_Buttons[userIndex][BUTTON_TOOLTIP_A].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_A].Press();
			break;
		case VK_PAD_B:
			m_Buttons[userIndex][BUTTON_TOOLTIP_B].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_B].Press();
			break;
		case VK_PAD_X:
			m_Buttons[userIndex][BUTTON_TOOLTIP_X].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_X].Press();
			break;
		case VK_PAD_Y:
			m_Buttons[userIndex][BUTTON_TOOLTIP_Y].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_Y].Press();
			break;
		case VK_PAD_LTRIGGER:
			m_Buttons[userIndex][BUTTON_TOOLTIP_LT].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_LT].Press();
			break;
		case VK_PAD_RTRIGGER:
			m_Buttons[userIndex][BUTTON_TOOLTIP_RT].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_RT].Press();
			break;
		case VK_PAD_LSHOULDER:
			m_Buttons[userIndex][BUTTON_TOOLTIP_LB].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_LB].Press();
			break;
		case VK_PAD_RSHOULDER:
			m_Buttons[userIndex][BUTTON_TOOLTIP_RB].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_RB].Press();
			break;
		case VK_PAD_LTHUMB_UP:
		case VK_PAD_LTHUMB_DOWN:
		case VK_PAD_LTHUMB_LEFT:
		case VK_PAD_LTHUMB_RIGHT:
			m_Buttons[userIndex][BUTTON_TOOLTIP_LS].Press();
			m_ButtonsSmall[userIndex][BUTTON_TOOLTIP_LS].Press();
			break;
#ifndef _CONTENT_PACKAGE
		case VK_PAD_LTHUMB_PRESS:
			app.ToggleFontRenderer();
			break;
#endif
	}
	return S_OK;
}

HRESULT CXuiSceneBase::_ShowSavingMessage( unsigned int iPad, C4JStorage::ESavingMessage eVal )
{
	switch(eVal)
	{
	case C4JStorage::ESavingMessage_None:
		XuiElementSetShow(m_SavingIcon,FALSE );
		break;
	case C4JStorage::ESavingMessage_Short:
	case C4JStorage::ESavingMessage_Long:
		XuiElementSetShow(m_SavingIcon,TRUE );
		break;
	}

	// Not needed - ReLayout(iPad);

	return S_OK;
}

HRESULT CXuiSceneBase::_ShowBackground( unsigned int iPad, BOOL bShow )
{
	HRESULT hr;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	//if(app.GetGameSettingsDebugMask(iPad) && app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_LightDarkBackground))
	{
		// get the visual
		HXUIOBJ hVisual,hNight,hDay;
		hr=XuiControlGetVisual(m_Background[iPad],&hVisual);
		hr=XuiElementGetChildById(hVisual,L"NightGroup",&hNight);
		hr=XuiElementGetChildById(hVisual,L"DayGroup",&hDay);

		if(bShow && pMinecraft->level!=nullptr)
		{
			int64_t i64TimeOfDay =0;
			// are we in the Nether? - Leave the time as 0 if we are, so we show daylight
			if(pMinecraft->level->dimension->id==0)
			{
				i64TimeOfDay = pMinecraft->level->getLevelData()->getTime() % 24000;
			}

			if(i64TimeOfDay>14000)
			{
				hr=XuiElementSetShow(hNight,TRUE);
				hr=XuiElementSetShow(hDay,FALSE);
			}
			else
			{
				hr=XuiElementSetShow(hNight,FALSE);
				hr=XuiElementSetShow(hDay,TRUE);
			}
		}
		else
		{
			hr=XuiElementSetShow(hNight,FALSE);
			hr=XuiElementSetShow(hDay,TRUE);
		}
	}
	hr=XuiElementSetShow(m_Background[iPad],bShow);

	return hr;
}

HRESULT CXuiSceneBase::_ShowDarkOverlay( unsigned int iPad, BOOL bShow )
{
	return XuiElementSetShow(m_DarkOverlay[iPad],bShow);
}

HRESULT CXuiSceneBase::_ShowLogo( unsigned int iPad, BOOL bShow )
{
	return XuiElementSetShow(m_Logo[iPad],bShow);
}

HRESULT CXuiSceneBase::_ShowPressStart(unsigned int iPad)
{
	XUIRect xuiRect;
	LPCWSTR pString=app.GetString(IDS_PRESS_START_TO_JOIN);;
	float fWidth,fHeight,fWidthChange;

	XuiSetTimer( m_hObj,PRESS_START_TIMER,3000);
	m_iQuadrantsMask|=1<<iPad;

	m_PressStart.SetShow(TRUE);
	// retrieve the visual for this quadrant

	HXUIOBJ hObj=nullptr,hQuadrant;
	HRESULT hr=XuiControlGetVisual(m_PressStart.m_hObj,&hObj);
	hr=XuiElementGetChildById(hObj,L"text_ButtonText",&hQuadrant);
	memset(&xuiRect, 0, sizeof(xuiRect));
	hr=XuiTextPresenterMeasureText(hQuadrant, pString, &xuiRect);

	XuiElementGetBounds(hQuadrant,&fWidth, &fHeight);
	fWidthChange=xuiRect.right-fWidth;

	// get the size of the button, and apply the change in size due to the text to the whole button
	XuiElementGetBounds(m_PressStart.m_hObj,&fWidth, &fHeight);

	XuiElementSetBounds(m_PressStart.m_hObj,fWidth+fWidthChange,fHeight);

	switch(iPad)
	{
	case 0:
		hr=XuiElementGetChildById(hObj,L"Quadrant1",&hQuadrant);
		break;
	case 1:
		hr=XuiElementGetChildById(hObj,L"Quadrant2",&hQuadrant);
		break;
	case 2:
		hr=XuiElementGetChildById(hObj,L"Quadrant3",&hQuadrant);
		break;
	case 3:
		hr=XuiElementGetChildById(hObj,L"Quadrant4",&hQuadrant);
		break;
	}

	XuiElementSetShow(hQuadrant,TRUE);
	int nStart, nEnd;
// 	XuiElementFindNamedFrame(  m_hObj, L"StartFlash", &nStart );
// 	XuiElementFindNamedFrame(  m_hObj, L"EndFlash", &nEnd );
// 	XuiElementPlayTimeline(  m_hObj, nStart, nStart, nEnd, TRUE, TRUE );
	XuiElementFindNamedFrame(  hObj, L"StartFlash", &nStart );
	XuiElementFindNamedFrame(  hObj, L"EndFlash", &nEnd );
	XuiElementPlayTimeline(  hObj, nStart, nStart, nEnd, TRUE, TRUE );

	return S_OK;
}

HRESULT CXuiSceneBase::_HidePressStart()
{
	return m_PressStart.SetShow(FALSE);
}

HRESULT CXuiSceneBase::_UpdateAutosaveCountdownTimer(unsigned int uiSeconds)
{
	WCHAR wcAutosaveCountdown[100];
	swprintf( wcAutosaveCountdown, 100, app.GetString(IDS_AUTOSAVE_COUNTDOWN),uiSeconds);
	m_TrialTimer.SetText(wcAutosaveCountdown);
	return S_OK;
}

HRESULT CXuiSceneBase::_ShowAutosaveCountdownTimer(BOOL bVal)
{
	m_TrialTimer.SetShow(bVal);
	return S_OK;
}

HRESULT CXuiSceneBase::_UpdateTrialTimer(unsigned int iPad)
{
	WCHAR wcTime[20];

	DWORD dwTimeTicks=static_cast<DWORD>(app.getTrialTimer());

	if(dwTimeTicks>m_dwTrialTimerLimitSecs)
	{
		dwTimeTicks=m_dwTrialTimerLimitSecs;
	}

	dwTimeTicks=m_dwTrialTimerLimitSecs-dwTimeTicks;

#ifndef _CONTENT_PACKAGE
	if(true)
#else
	// display the time - only if there's less than 3 minutes
	if(dwTimeTicks<180)
#endif
	{
		int iMins=dwTimeTicks/60;
		int iSeconds=dwTimeTicks%60;
		swprintf( wcTime, 20, L"%d:%02d",iMins,iSeconds);
		m_TrialTimer.SetText(wcTime);
	}
	else
	{
		m_TrialTimer.SetText(L"");
	}

	// are we out of time?
	if(dwTimeTicks==0)
	{
		// Trial over
		app.SetAction(iPad,eAppAction_TrialOver);
	}

	return S_OK;
}

void CXuiSceneBase::_ReduceTrialTimerValue()
{
	DWORD dwTimeTicks=static_cast<int>(app.getTrialTimer());

	if(dwTimeTicks>m_dwTrialTimerLimitSecs)
	{
		dwTimeTicks=m_dwTrialTimerLimitSecs;
	}

	m_dwTrialTimerLimitSecs-=dwTimeTicks;
}

HRESULT CXuiSceneBase::_ShowTrialTimer(BOOL bVal)
{
	m_TrialTimer.SetShow(bVal);
	return S_OK;
}

bool CXuiSceneBase::_PressStartPlaying(unsigned int iPad)
{
	return m_iQuadrantsMask&(1<<iPad)?true:false;
}

HRESULT CXuiSceneBase::_SetPlayerBaseScenePosition( unsigned int iPad, EBaseScenePosition position )
{
	// turn off the empty quadrant logo
	if(m_hEmptyQuadrantLogo!=nullptr) 
	{
		XuiElementSetShow(m_hEmptyQuadrantLogo,FALSE);
	}

	// No changes
	if( m_playerBaseScenePosition[iPad] == position )
		return S_OK;

	m_selectedItemA[iPad].SetShow(FALSE);
	m_selectedItemSmallA[iPad].SetShow(FALSE);

	if(position == e_BaseScene_NotSet)
	{
		m_playerBaseScenePosition[iPad] = position;
		return S_OK;
	}

	D3DXVECTOR3 scale,pos;//,currentpos;
	// Shift the tooltips
	D3DXVECTOR3 tooltipsPos,crouchIconPos,saveIconPos,vBackPos,vGamertagPos,vBossHealthPos;
	tooltipsPos.z=crouchIconPos.z=saveIconPos.z=vBackPos.z=vBossHealthPos.z=0.0f;
	vBackPos.x=0.0f;
	vBackPos.y=0.0f;

	m_playerBaseScenePosition[iPad] = position;
	float fTempWidth, fTooltipHeight, fTooltipHeightSmall,fGamertagWidth,fGamertagHeight,fBossHealthWidth, fBossHealthHeight;
	float fBackWidth, fBackHeight;
	// Reset the players base scene before we make any other adjustments
	pos.x = 0.0f; pos.y = 0.0f; pos.z = 0.0f;
	XuiElementSetPosition(m_BasePlayerScene[iPad], &pos );
	XuiElementGetBounds(m_TooltipGroup[iPad].m_hObj,&fTempWidth, &fTooltipHeight);
	XuiElementGetBounds(m_TooltipGroupSmall[iPad].m_hObj,&fTempWidth, &fTooltipHeightSmall);
	XuiElementGetBounds(m_Background[iPad].m_hObj,&fBackWidth, &fBackHeight);
	XuiElementGetBounds(m_hGamerTagA[iPad],&fGamertagWidth, &fGamertagHeight);
	XuiElementGetBounds(m_BossHealthGroup[iPad],&fBossHealthWidth, &fBossHealthHeight);

	if( position == e_BaseScene_Fullscreen && (RenderManager.IsHiDef() || RenderManager.IsWidescreen()) )
	{
		XuiElementSetShow( m_TooltipGroup[iPad].m_hObj, TRUE);
		XuiElementSetShow( m_TooltipGroupSmall[iPad].m_hObj, FALSE);
	}
	else
	{
		XuiElementSetShow( m_TooltipGroup[iPad].m_hObj, FALSE);
		XuiElementSetShow( m_TooltipGroupSmall[iPad].m_hObj, TRUE);
	}

	if(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen())
	{
		//640x480 ->1280x720
		scale.x = 2.0f; scale.y = 1.5f; scale.z = 1.0f;
		XuiElementSetScale(m_hObj, &scale);

		return S_OK;
	}


	if( position != e_BaseScene_Fullscreen )
	{
		// Scale up the tooltips so we can read them
		/*
		scale.x = 0.75f; scale.y = 0.75f; scale.z = 1.0f;
		XuiElementSetScale(m_TooltipGroup[iPad], &scale);
		fTooltipHeight*=scale.y;
		*/
		fTooltipHeight = fTooltipHeightSmall;

		scale.x = 0.5f; scale.y = 0.5f; scale.z = 1.0f;
		XuiElementSetScale(m_CrouchIcon[iPad], &scale);
		XuiElementSetScale(m_Logo[iPad].m_hObj, &scale);
	}
	else
	{
		// if we are not in high def mode, then we need to scale the m_BasePlayerScene scene by 2 (we're using the 640x360 scenes)
		scale.x = 1.0f; scale.y = 1.0f; scale.z = 1.0f;
		XuiElementSetScale(m_BasePlayerScene[iPad], &scale );
		XuiElementSetScale(m_TooltipGroup[iPad], &scale);
		XuiElementSetScale(m_CrouchIcon[iPad], &scale);
		XuiElementSetScale(m_Logo[iPad].m_hObj, &scale);
	}


	// The move applies to the whole scene, so we'll need to move tooltips back in some cases
	switch( position )
	{
		// No position adjustment
	case e_BaseScene_Fullscreen:
		tooltipsPos.x=SAFEZONE_HALF_WIDTH;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackWidth=XUI_BASE_SCENE_WIDTH;
		fBackHeight=XUI_BASE_SCENE_HEIGHT;

		XuiElementGetPosition(m_selectedItemA[iPad], &vBossHealthPos);
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_HALF-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Top_Left:
		tooltipsPos.x=SAFEZONE_HALF_WIDTH;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth - 10.0f;
		vGamertagPos.y=SAFEZONE_HALF_HEIGHT;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Top: // Top & Bottom - indent by a quarter screen
		pos.x += XUI_BASE_SCENE_WIDTH_QUARTER;
		tooltipsPos.x=SAFEZONE_HALF_WIDTH - XUI_BASE_SCENE_WIDTH_QUARTER;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH-XUI_BASE_SCENE_WIDTH_QUARTER;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		fBackWidth=XUI_BASE_SCENE_WIDTH;
		vBackPos.x=-XUI_BASE_SCENE_WIDTH_QUARTER;
		vBackPos.y=0.0f;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH - XUI_BASE_SCENE_WIDTH_QUARTER - fGamertagWidth - SAFEZONE_HALF_WIDTH;
		vGamertagPos.y=SAFEZONE_HALF_HEIGHT;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Bottom:
		pos.x += XUI_BASE_SCENE_WIDTH_QUARTER;
		pos.y += XUI_BASE_SCENE_HEIGHT_HALF;
		tooltipsPos.x=SAFEZONE_HALF_WIDTH - XUI_BASE_SCENE_WIDTH_QUARTER;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH-XUI_BASE_SCENE_WIDTH_QUARTER;
		crouchIconPos.y=0.0f;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		fBackWidth=XUI_BASE_SCENE_WIDTH;
		vBackPos.x=-XUI_BASE_SCENE_WIDTH_QUARTER;
		vBackPos.y=0.0f;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH - XUI_BASE_SCENE_WIDTH_QUARTER - fGamertagWidth - SAFEZONE_HALF_WIDTH;
		vGamertagPos.y=0.0f;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = 0.0f;
	break;
	case e_BaseScene_Bottom_Left:
		pos.y += XUI_BASE_SCENE_HEIGHT_HALF;
		tooltipsPos.x=SAFEZONE_HALF_WIDTH;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH;
		crouchIconPos.y=0.0f;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth- 10.0f;
		vGamertagPos.y=0.0f;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = 0.0f;
		break;
	case e_BaseScene_Bottom_Right:
		pos.x += XUI_BASE_SCENE_WIDTH_HALF;
		pos.y += XUI_BASE_SCENE_HEIGHT_HALF;
		tooltipsPos.x=0.0f;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=0.0f;
		crouchIconPos.y=0.0f;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth-SAFEZONE_HALF_WIDTH;
		vGamertagPos.y=0.0f;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = 0.0f;
		break;
	case e_BaseScene_Left:
		pos.y += XUI_BASE_SCENE_HEIGHT_QUARTER;
		tooltipsPos.x=SAFEZONE_HALF_WIDTH;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF+XUI_BASE_SCENE_HEIGHT_QUARTER-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=SAFEZONE_HALF_WIDTH;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT;
		vBackPos.x=0.0f;
		vBackPos.y=-XUI_BASE_SCENE_HEIGHT_QUARTER;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth- 10.0f;
		vGamertagPos.y=SAFEZONE_HALF_HEIGHT - XUI_BASE_SCENE_HEIGHT_QUARTER;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT-XUI_BASE_SCENE_HEIGHT_QUARTER;
		break;
	case e_BaseScene_Right:
		pos.x += XUI_BASE_SCENE_WIDTH_HALF;
		pos.y += XUI_BASE_SCENE_HEIGHT_QUARTER;
		tooltipsPos.x=0.0f;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF+XUI_BASE_SCENE_HEIGHT_QUARTER-SAFEZONE_HALF_HEIGHT-fTooltipHeight;
		crouchIconPos.x=0.0f;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT;
		vBackPos.x=0.0f;
		vBackPos.y=-XUI_BASE_SCENE_HEIGHT_QUARTER;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth-SAFEZONE_HALF_WIDTH;
		vGamertagPos.y=SAFEZONE_HALF_HEIGHT - XUI_BASE_SCENE_HEIGHT_QUARTER;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT-XUI_BASE_SCENE_HEIGHT_QUARTER;
		break;
	case e_BaseScene_Top_Right:
		pos.x += XUI_BASE_SCENE_WIDTH_HALF;
		tooltipsPos.x=0.0f;
		tooltipsPos.y=XUI_BASE_SCENE_HEIGHT_HALF-fTooltipHeight;
		crouchIconPos.x=0.0f;
		crouchIconPos.y=SAFEZONE_HALF_HEIGHT;
		fBackWidth=XUI_BASE_SCENE_WIDTH_HALF;
		fBackHeight=XUI_BASE_SCENE_HEIGHT_HALF;
		vGamertagPos.x=XUI_BASE_SCENE_WIDTH_HALF-fGamertagWidth-SAFEZONE_HALF_WIDTH;
		vGamertagPos.y=SAFEZONE_HALF_HEIGHT;
		vBossHealthPos.x = XUI_BASE_SCENE_WIDTH_QUARTER-(fBossHealthWidth/2);
		vBossHealthPos.y = SAFEZONE_HALF_HEIGHT;
		break;
	}

	XuiElementSetPosition(m_BasePlayerScene[iPad], &pos );
	XuiElementSetPosition( m_TooltipGroup[iPad].m_hObj, &tooltipsPos);
	XuiElementSetPosition( m_TooltipGroupSmall[iPad].m_hObj, &tooltipsPos);
	XuiElementSetPosition( m_CrouchIcon[iPad].m_hObj, &crouchIconPos);
	XuiElementSetPosition( m_DarkOverlay[iPad].m_hObj, &vBackPos );
	XuiElementSetBounds( m_DarkOverlay[iPad].m_hObj, fBackWidth, fBackHeight);
	XuiElementSetPosition( m_Background[iPad].m_hObj, &vBackPos );
	XuiElementSetBounds( m_Background[iPad].m_hObj, fBackWidth, fBackHeight );
	vGamertagPos.z=0.0f;
	XuiElementSetPosition( m_hGamerTagA[iPad], &vGamertagPos );
	XuiElementSetPosition( m_BossHealthGroup[iPad], &vBossHealthPos );


	// 4J Stu - If we already have some scenes open, then call this to update their positions
	// Fix for #10960 - All Lang: UI: Split-screen: Changing split screen mode (vertical/horizontal) make window layout strange
	if(Minecraft::GetInstance() != nullptr && Minecraft::GetInstance()->localplayers[iPad]!=nullptr)
	{
		// 4J-PB - Can only do this once we know what the player's UI settings are, so we need to have the player game settings read
		_UpdateSelectedItemPos(iPad);
		XUIMessage xuiMsg;
		CustomMessage_Splitscreenplayer_Struct myMsgData;
		CustomMessage_Splitscreenplayer( &xuiMsg, &myMsgData, false);
		XuiBroadcastMessage( GetPlayerBaseScene(iPad), &xuiMsg );
	}
	// tell the xui scenes that the base position has changed
	XUIMessage xuiMsg;
	CustomMessage_BasePositionChanged( &xuiMsg );
	XuiBroadcastMessage( GetPlayerBaseScene(iPad), &xuiMsg );

	return S_OK;
}

// The function uses the game mode to decide the offsets for the select item. It needs to be called after the game has loaded.
void CXuiSceneBase::_UpdateSelectedItemPos(unsigned int iPad)
{
	D3DXVECTOR3 selectedItemPos;
	selectedItemPos.x = selectedItemPos.y = selectedItemPos.z = 0.0f;
	float fSelectedItemWidth, fSelectedItemHeight;
	XuiElementGetBounds(m_selectedItemSmallA[iPad],&fSelectedItemWidth, &fSelectedItemHeight);

	float yOffset = 0.0f;

	if( m_bossHealthVisible[iPad] == TRUE )
	{
		float tempWidth;
		XuiElementGetBounds(m_BossHealthGroup[iPad],&tempWidth, &yOffset);
	}


	// Only adjust if fullscreen for now, leaving code to move others if required, but it's too far up the screen when on the bottom quadrants
	if( (m_playerBaseScenePosition[iPad] == e_BaseScene_Fullscreen) &&
		(RenderManager.IsHiDef() || RenderManager.IsWidescreen()) )
	{
		D3DXVECTOR3 selectedItemPos;
		selectedItemPos.z=0.0f;
		float scale, fTempWidth, fTooltipHeight, fTooltipHeightSmall, fSelectedItemWidth, fSelectedItemHeight, fSelectedItemSmallWidth, fSelectedItemSmallHeight;
		XuiElementGetBounds(m_TooltipGroup[iPad].m_hObj,&fTempWidth, &fTooltipHeight);
		XuiElementGetBounds(m_TooltipGroupSmall[iPad].m_hObj,&fTempWidth, &fTooltipHeightSmall);
		XuiElementGetBounds(m_selectedItemA[iPad],&fSelectedItemWidth, &fSelectedItemHeight);
		XuiElementGetBounds(m_selectedItemSmallA[iPad],&fSelectedItemSmallWidth, &fSelectedItemSmallHeight);
		if( m_playerBaseScenePosition[iPad] != e_BaseScene_Fullscreen )
		{
			fTooltipHeight = fTooltipHeightSmall;
			fSelectedItemHeight = fSelectedItemSmallHeight;

			scale = 0.5f;
		}
		else
		{
			// if we are not in high def mode, then we need to scale the m_BasePlayerScene scene by 2 (we're using the 640x360 scenes)
			scale = 1.0f;
		}

		// The move applies to the whole scene, so we'll need to move tooltips back in some cases

		selectedItemPos.y=XUI_BASE_SCENE_HEIGHT-SAFEZONE_HALF_HEIGHT-fTooltipHeight - fSelectedItemHeight;
		selectedItemPos.x = XUI_BASE_SCENE_WIDTH_HALF - (fSelectedItemWidth/2.0f);

		// Adjust selectedItemPos based on what gui is displayed


		// 4J-PB - selected the gui scale based on the slider settings, and on whether we're in Creative or Survival
		float fYOffset=0.0f;

		unsigned char 	ucGuiScale=app.GetGameSettings(iPad,eGameSetting_UISize) + 2;

		if(Minecraft::GetInstance() != nullptr && Minecraft::GetInstance()->localgameModes[iPad] != nullptr && Minecraft::GetInstance()->localgameModes[iPad]->canHurtPlayer())
		{
			// SURVIVAL MODE - Move up further because of hearts, shield and xp
			switch(ucGuiScale)
			{
			case 3:
				fYOffset = -130.0f;
				break;
			case 4:
				fYOffset = -168.0f;
				break;
			default: // 2
				fYOffset = -94.0f;
				break;
			}
		}
		else
		{
			switch(ucGuiScale)
			{
			case 3:
				fYOffset = -83.0f;
				break;
			case 4:
				fYOffset = -114.0f;
				break;
			default: // 2
				fYOffset = -58.0f;
				break;
			}
		}


		selectedItemPos.y+=fYOffset - 40.0f; // 40 for the XP display

		XuiElementSetPosition( m_selectedItemA[iPad].m_hObj, &selectedItemPos );

		//XuiElementSetPosition( m_selectedItemSmallA[iPad].m_hObj, &selectedItemPos );
	}
	else
	{
		// The move applies to the whole scene, so we'll need to move tooltips back in some cases
		switch( m_playerBaseScenePosition[iPad] )
		{
 		case e_BaseScene_Fullscreen:
			// 480 non-widescreen
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = SAFEZONE_HALF_HEIGHT + yOffset;
 			break;
		case e_BaseScene_Top_Left:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = SAFEZONE_HALF_HEIGHT + yOffset;
			break;
		case e_BaseScene_Top: // Top & Bottom - indent by a quarter screen
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = SAFEZONE_HALF_HEIGHT + yOffset;
			break;
		case e_BaseScene_Bottom:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = 0.0f + yOffset;
		break;
		case e_BaseScene_Bottom_Left:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = 0.0f + yOffset;
			break;
		case e_BaseScene_Bottom_Right:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = 0.0f + yOffset;
			break;
		case e_BaseScene_Left:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = XUI_BASE_SCENE_HEIGHT_HALF;// + yOffset; - don't need the offset for the boss health since we're displaying the item at the bottom of the screen, not the top
			break;
		case e_BaseScene_Right:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = XUI_BASE_SCENE_HEIGHT_HALF;// + yOffset; - don't need the offset for the boss health since we're displaying the item at the bottom of the screen, not the top
			break;
		case e_BaseScene_Top_Right:
			selectedItemPos.x = XUI_BASE_SCENE_WIDTH_QUARTER - (fSelectedItemWidth/2.0f);
			selectedItemPos.y = SAFEZONE_HALF_HEIGHT + yOffset;
			break;
		}

		// 4J-PB - If it's in split screen vertical, adjust the position
		// Adjust selectedItemPos based on what gui is displayed
		if((m_playerBaseScenePosition[iPad]==e_BaseScene_Left) || (m_playerBaseScenePosition[iPad]==e_BaseScene_Right))
		{
			float scale=0.5f;
			selectedItemPos.y -= (scale * 88.0f);
			if(Minecraft::GetInstance() != nullptr && Minecraft::GetInstance()->localgameModes[iPad] != nullptr && Minecraft::GetInstance()->localgameModes[iPad]->canHurtPlayer())
			{
				selectedItemPos.y -= (scale * 80.0f);
			}

			// 4J-PB - selected the gui scale based on the slider settings
			unsigned char ucGuiScale;
			float fYOffset=0.0f;
			if(m_playerBaseScenePosition[iPad]==e_BaseScene_Fullscreen)
			{
				ucGuiScale=app.GetGameSettings(iPad,eGameSetting_UISize) + 2;
			}
			else
			{
				ucGuiScale=app.GetGameSettings(iPad,eGameSetting_UISizeSplitscreen) + 2;
			}
			switch(ucGuiScale)
			{
			case 3:
				fYOffset = 55.0f;
				break;
			case 4:
				fYOffset = 45.0f;
				break;
			default: // 2
				fYOffset = 85.0f;
				break;
			}
			selectedItemPos.y+=fYOffset;
		}

		XuiElementSetPosition( m_selectedItemSmallA[iPad].m_hObj, &selectedItemPos );
		XuiElementSetPosition( m_selectedItemA[iPad].m_hObj, &selectedItemPos );
	}
}

CXuiSceneBase::EBaseScenePosition CXuiSceneBase::_GetPlayerBasePosition(int iPad)
{
	return m_playerBaseScenePosition[iPad];
}

void CXuiSceneBase::_SetEmptyQuadrantLogo(int iPad,EBaseScenePosition ePos)
{
	if(m_hEmptyQuadrantLogo!=nullptr)
	{
		for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
		{
			if(m_playerBaseScenePosition[i] == e_BaseScene_Fullscreen)
			{
				// Someone is fullscreen, so don't show this
				XuiElementSetShow(m_hEmptyQuadrantLogo,FALSE);
				return;
			}
		}

		D3DXVECTOR3 pos;

		// get the bounds of the logo

		pos.z=0.0f;
		switch( ePos )
		{
		case e_BaseScene_Top_Left:
			pos.x=64.0f;
			pos.y=36.0f;
			break;
		case e_BaseScene_Top_Right:
			pos.x=640.0+64.0f;
			pos.y=36.0f;
			break;
		case e_BaseScene_Bottom_Left:
			pos.x=64.0f;
			pos.y=360.0f+36.0f;
			break;
		case e_BaseScene_Bottom_Right:
			pos.x=640.0+64.0f;
			pos.y=360.0f+36.0f;
			break;
		}

		// set the opacity of the logo
		if(ProfileManager.GetLockedProfile()!=-1)
		{
			unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
			XuiElementSetOpacity(m_hEmptyQuadrantLogo,0.01f*static_cast<float>(ucAlpha));
		}

		XuiElementSetPosition(m_hEmptyQuadrantLogo, &pos );
		XuiElementSetShow(m_hEmptyQuadrantLogo,TRUE);
	}
}


HRESULT CXuiSceneBase::_PlayUISFX(ESoundEffect eSound)
{
	HRESULT hr;
	bool bUsingTexturepackWithAudio=false;

	// are we using a mash-up pack?
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		if(tPack->hasAudio())
		{
			bUsingTexturepackWithAudio=true;
		}
	}

	/*if(bUsingTexturepackWithAudio)
	{
		switch(eSound)
		{
		case eSFX_Back:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_Craft:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_CraftFail:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_Focus:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_Press:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_Scroll:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		default:
			hr=S_OK;
			break;
		}
	}
	else*/
	{

		switch(eSound)
		{
		case eSFX_Back:
			hr=XuiSoundPlay(m_SFXA[eSFX_Back]);
			break;
		case eSFX_Craft:
			hr=XuiSoundPlay(m_SFXA[eSFX_Craft]);
			break;
		case eSFX_CraftFail:
			hr=XuiSoundPlay(m_SFXA[eSFX_CraftFail]);
			break;
		case eSFX_Focus:
			hr=XuiSoundPlay(m_SFXA[eSFX_Focus]);
			break;
		case eSFX_Press:
			hr=XuiSoundPlay(m_SFXA[eSFX_Press]);
			break;
		case eSFX_Scroll:
			hr=XuiSoundPlay(m_SFXA[eSFX_Scroll]);
			break;
		default:
			hr=S_OK;
			break;
		}
	}
	return hr;
}


HRESULT CXuiSceneBase::_DisplayGamertag( unsigned int iPad, BOOL bDisplay )
{

	if(app.DebugSettingsOn() && (app.GetGameSettingsDebugMask()&(1L<<eDebugSetting_DebugLeaderboards)))
	{
		XuiControlSetText(m_hGamerTagA[iPad],L"WWWWWWWWWWWWWWWW");
	}
	else
	{
		// The host decides whether these are on or off
		if(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags)!=0)
		{
			if(Minecraft::GetInstance() != nullptr && Minecraft::GetInstance()->localplayers[iPad]!= nullptr)
			{
				wstring wsGamertag = convStringToWstring( ProfileManager.GetGamertag(iPad));
				XuiControlSetText(m_hGamerTagA[iPad],wsGamertag.c_str());

			}
			else
			{
				XuiControlSetText(m_hGamerTagA[iPad],L"");
			}
		}
	}
	// The host decides whether these are on or off
	if(app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_DisplaySplitscreenGamertags)!=0)
	{
		XuiElementSetShow(m_hGamerTagA[iPad],bDisplay);

		// set the opacity of the gamertag
		if((bDisplay==TRUE) &&(ProfileManager.GetLockedProfile()!=-1))
		{
			unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);
			float fVal;

			if(ucAlpha<80)
			{
				// check if we have the timer running for the opacity
				unsigned int uiOpacityTimer=app.GetOpacityTimer(ProfileManager.GetPrimaryPad());
				if(uiOpacityTimer!=0)
				{
					if(uiOpacityTimer<10)
					{
						float fStep=(80.0f-static_cast<float>(ucAlpha))/10.0f;
						fVal=0.01f*(80.0f-((10.0f-static_cast<float>(uiOpacityTimer))*fStep));
					}
					else
					{
						fVal=0.01f*80.0f;
					}
				}
				else
				{
					fVal=0.01f*static_cast<float>(ucAlpha);
				}
			}
			else
			{
				fVal=0.01f*static_cast<float>(ucAlpha);
			}
			XuiElementSetOpacity(m_hGamerTagA[iPad],0.01f*fVal);
		}
	}
	else
	{
		XuiElementSetShow(m_hGamerTagA[iPad],FALSE);
	}

	return S_OK;
}

void CXuiSceneBase::_SetSelectedItem( unsigned int iPad, const wstring& name)
{
	if(app.GetGameSettings(eGameSetting_Hints) == 0 || name.empty())
	{
		m_selectedItemA[iPad].SetShow(FALSE);
		m_selectedItemSmallA[iPad].SetShow(FALSE);
	}
	else
	{
		m_uiSelectedItemOpacityCountDown[iPad] = SharedConstants::TICKS_PER_SECOND * 3;
		if(m_playerBaseScenePosition[iPad] == e_BaseScene_Fullscreen)
		{
			m_selectedItemSmallA[iPad].SetShow(FALSE);
			m_selectedItemA[iPad].SetShow(TRUE);
			m_selectedItemA[iPad].SetText(name.c_str());

// 			D3DXVECTOR3 vPos;
// 			XuiElementGetPosition( m_selectedItemA[iPad].m_hObj, &vPos );
// 			XuiElementSetPosition( m_selectedItemA[iPad].m_hObj, &vPos );

			float fVal=0.8f;//0.01f*(float)80;
			XuiElementSetOpacity(m_selectedItemA[iPad].m_hObj,fVal);
		}
		else
		{
			m_selectedItemA[iPad].SetShow(FALSE);
			m_selectedItemSmallA[iPad].SetShow(TRUE);
			m_selectedItemSmallA[iPad].SetText(name.c_str());

// 			D3DXVECTOR3 vPos;
// 			XuiElementGetPosition( m_selectedItemSmallA[iPad].m_hObj, &vPos );
// 			XuiElementSetPosition( m_selectedItemSmallA[iPad].m_hObj, &vPos );

			float fVal=0.8f;//0.01f*(float)80;
			XuiElementSetOpacity(m_selectedItemSmallA[iPad].m_hObj,fVal);
		}
	}
}

void CXuiSceneBase::_HideAllGameUIElements()
{
	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		m_uiSelectedItemOpacityCountDown[i] = 0;
		m_selectedItemA[i].SetShow(FALSE);
		m_selectedItemSmallA[i].SetShow(FALSE);

		m_BossHealthGroup[i].SetShow(FALSE);
		m_bossHealthVisible[i] = FALSE;

		XuiElementSetShow(app.GetCurrentHUDScene(i),FALSE);

		_DisplayGamertag(i,FALSE);
	}
}

bool CXuiSceneBase::_GetBaseSceneSafeZone( unsigned int iPad, D3DXVECTOR2 &origin, float &width, float &height)
{
	if(m_playerBaseScenePosition[iPad] == e_BaseScene_NotSet) return false;

	D3DXMATRIX baseSceneMatrix;
	XuiElementGetFullXForm( m_BasePlayerScene[iPad], &baseSceneMatrix);

	origin.x = baseSceneMatrix._41;
	origin.y = baseSceneMatrix._42;

	XuiElementGetBounds( m_BasePlayerScene[iPad], &width, &height);

	switch( m_playerBaseScenePosition[iPad] )
	{
		// No position adjustment
	case e_BaseScene_Fullscreen:
		origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH * 2;

		origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT * 2;
		break;
	case e_BaseScene_Top_Left:
		width = XUI_BASE_SCENE_WIDTH_HALF;
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT * 2;
		break;
	case e_BaseScene_Top: // Top & Bottom - indent by a quarter screen
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		//origin.x += SAFEZONE_HALF_WIDTH;
		//width -= SAFEZONE_HALF_WIDTH * 2;

		origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Bottom:
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		//origin.x += SAFEZONE_HALF_WIDTH;
		//width -= SAFEZONE_HALF_WIDTH * 2;

		//origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Bottom_Left:
		width = XUI_BASE_SCENE_WIDTH_HALF;
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		//origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Bottom_Right:
		width = XUI_BASE_SCENE_WIDTH_HALF;
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		//origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		//origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT;
		break;
	case e_BaseScene_Left:
		width = XUI_BASE_SCENE_WIDTH_HALF;

		origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		//origin.y += SAFEZONE_HALF_HEIGHT;
		//height -= SAFEZONE_HALF_HEIGHT * 2;
		break;
	case e_BaseScene_Right:
		width = XUI_BASE_SCENE_WIDTH_HALF;

		//origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		//origin.y += SAFEZONE_HALF_HEIGHT;
		//height -= SAFEZONE_HALF_HEIGHT * 2;
		break;
	case e_BaseScene_Top_Right:
		width = XUI_BASE_SCENE_WIDTH_HALF;
		height = XUI_BASE_SCENE_HEIGHT_HALF;

		//origin.x += SAFEZONE_HALF_WIDTH;
		width -= SAFEZONE_HALF_WIDTH;

		origin.y += SAFEZONE_HALF_HEIGHT;
		height -= SAFEZONE_HALF_HEIGHT;
		break;
	}

	return true;
}

void CXuiSceneBase::ReLayout( unsigned int iPad )
{
	D3DXVECTOR3 lastPos, lastPosSmall;
	int lastVisible = -1;

	// Buttons are at 0,0 within the tooltip group
	lastPos.x = 0.0f;
	lastPos.y = 0.0f;
	lastPos.z = 0.0f;
	lastPosSmall.x = 0.0f;
	lastPosSmall.y = 0.0f;
	lastPosSmall.z = 0.0f;

	for( unsigned int i = 0; i < BUTTONS_TOOLTIP_MAX; ++i )
	{
		if( m_visible[iPad][ i ] == TRUE )
		{
			if( i>0 && lastVisible!=-1 )
			{
				float width, height;
				XuiElementGetBounds(m_Buttons[iPad][lastVisible].m_hObj, &width, &height);

				// 4J Stu - This is for horizontal layout, will need changed if we do vertical layout
				lastPos.x += width + m_iTooltipSpacingGap;

				XuiElementGetBounds(m_ButtonsSmall[iPad][lastVisible].m_hObj, &width, &height);
				// 4J Stu - This is for horizontal layout, will need changed if we do vertical layout
				lastPosSmall.x += width + m_iTooltipSpacingGapSmall;
			}
			XuiElementSetPosition( m_Buttons[iPad][i].m_hObj, &lastPos);
			XuiElementSetPosition( m_ButtonsSmall[iPad][i].m_hObj, &lastPosSmall);

			lastVisible = i;
		}
	}
}

void CXuiSceneBase::TickAllBaseScenes()
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		CXuiSceneBase::Instance->_TickAllBaseScenes();
	}
}

HRESULT CXuiSceneBase::SetEnableTooltips( unsigned int iPad, BOOL bVal )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_SetEnableTooltips(iPad, bVal );
	}
	return S_OK;
}

HRESULT CXuiSceneBase::SetTooltipText( unsigned int iPad, unsigned int tooltip, int iTextID )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_SetTooltipText(iPad, tooltip, iTextID );
	}
	return S_OK;
}

HRESULT CXuiSceneBase::RefreshTooltips( unsigned int iPad)
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_RefreshTooltips(iPad);
	}
	return S_OK;
}

HRESULT CXuiSceneBase::ShowTooltip( unsigned int iPad, unsigned int tooltip, bool show )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_ShowTooltip(iPad, tooltip, show );
	}
	return S_OK;
}

HRESULT CXuiSceneBase::ShowSafeArea( BOOL bShow )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_ShowSafeArea(bShow );
	}
	return S_OK;
}

HRESULT CXuiSceneBase::SetTooltips( unsigned int iPad, int iA, int iB, int iX, int iY , int iLT, int iRT, int iRB, int iLB, int iLS, bool forceUpdate /*= false*/ )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		// Enable all the tooltips. We should disable them in the scenes as required
		CXuiSceneBase::Instance->_SetTooltipsEnabled( iPad );

		int iTooptipsA[BUTTONS_TOOLTIP_MAX];
		iTooptipsA[BUTTON_TOOLTIP_A]=iA;
		iTooptipsA[BUTTON_TOOLTIP_B]=iB;
		iTooptipsA[BUTTON_TOOLTIP_X]=iX;
		iTooptipsA[BUTTON_TOOLTIP_Y]=iY;
		iTooptipsA[BUTTON_TOOLTIP_LT]=iLT;
		iTooptipsA[BUTTON_TOOLTIP_RT]=iRT;
		iTooptipsA[BUTTON_TOOLTIP_LB]=iRB;
		iTooptipsA[BUTTON_TOOLTIP_RB]=iLB;
		iTooptipsA[BUTTON_TOOLTIP_LS]=iLS;

		for(int i=0;i<BUTTONS_TOOLTIP_MAX;i++)
		{
			if(iTooptipsA[i]==-1)
			{
				CXuiSceneBase::Instance->SetTooltipText(iPad, i, -1 );
				CXuiSceneBase::Instance->_ShowTooltip(iPad, i, false );
				//CXuiSceneBase::Instance->m_iCurrentTooltipTextID[iPad][i]=-1;
			}
			else if(iTooptipsA[i]==-2)
			{
				CXuiSceneBase::Instance->_ShowTooltip(iPad, i, true );
				CXuiSceneBase::Instance->SetTooltipText(iPad, i, -2 );
			}
			else
			{
				// does the tooltip need to change?
				if(CXuiSceneBase::Instance->m_iCurrentTooltipTextID[iPad][i]!=iTooptipsA[i] || forceUpdate)
				{
					CXuiSceneBase::Instance->SetTooltipText(iPad, i, iTooptipsA[i] );
				}
				CXuiSceneBase::Instance->_ShowTooltip(iPad, i, true );
			}
		}

	}
	return S_OK;
}

HRESULT CXuiSceneBase::SetTooltipsEnabled( unsigned int iPad, bool bA, bool bB, bool bX, bool bY,bool bLT, bool bRT,  bool bLB, bool bRB, bool bLS)
{
	return CXuiSceneBase::Instance->_SetTooltipsEnabled(iPad, bA, bB, bX, bY, bLT, bRT, bLB, bRB, bLS );
}

HRESULT CXuiSceneBase::EnableTooltip( unsigned int iPad, unsigned int tooltip, bool enable )
{
	return CXuiSceneBase::Instance->_EnableTooltip(iPad, tooltip, enable);
}

HRESULT CXuiSceneBase::AnimateKeyPress(DWORD userIndex, DWORD dwKeyCode)
{
	return CXuiSceneBase::Instance->_AnimateKeyPress(userIndex, dwKeyCode );
}

HRESULT CXuiSceneBase::ShowSavingMessage( unsigned int iPad, C4JStorage::ESavingMessage eVal )
{
	if( CXuiSceneBase::Instance != nullptr )
	{
		return CXuiSceneBase::Instance->_ShowSavingMessage(iPad, eVal);
	}

	return S_OK;
}

HRESULT CXuiSceneBase::ShowBackground( unsigned int iPad, BOOL bShow )
{
	return CXuiSceneBase::Instance->_ShowBackground(iPad, bShow );
}

HRESULT CXuiSceneBase::ShowDarkOverlay( unsigned int iPad, BOOL bShow )
{
	return CXuiSceneBase::Instance->_ShowDarkOverlay(iPad, bShow );
}

HRESULT CXuiSceneBase::ShowLogo( unsigned int iPad, BOOL bShow )
{
	return CXuiSceneBase::Instance->_ShowLogo(iPad, bShow );
}

HRESULT CXuiSceneBase::ShowPressStart(unsigned int iPad)
{
	CXuiSceneBase::Instance->_ShowPressStart(iPad);
	return S_OK;
}

HRESULT CXuiSceneBase::ShowOtherPlayersBaseScene(int iPad, bool show)
{
	CXuiSceneBase::Instance->_ShowOtherPlayersBaseScene(iPad, show);
	return S_OK;
}

HRESULT CXuiSceneBase::UpdateAutosaveCountdownTimer(unsigned int uiSeconds)
{
	CXuiSceneBase::Instance->_UpdateAutosaveCountdownTimer(uiSeconds);
	return S_OK;
}

HRESULT CXuiSceneBase::ShowAutosaveCountdownTimer(BOOL bVal)
{
	CXuiSceneBase::Instance->_ShowAutosaveCountdownTimer(bVal);
	return S_OK;
}

HRESULT CXuiSceneBase::UpdateTrialTimer(unsigned int iPad)
{
	CXuiSceneBase::Instance->_UpdateTrialTimer(iPad);
	return S_OK;
}
HRESULT CXuiSceneBase::ShowTrialTimer(BOOL bVal)
{
	CXuiSceneBase::Instance->_ShowTrialTimer(bVal);
	return S_OK;
}

void CXuiSceneBase::ReduceTrialTimerValue()
{
	CXuiSceneBase::Instance->_ReduceTrialTimerValue();
}

bool CXuiSceneBase::PressStartPlaying(unsigned int iPad)
{
	return CXuiSceneBase::Instance->_PressStartPlaying(iPad);
}

HRESULT CXuiSceneBase::HidePressStart()
{
	return CXuiSceneBase::Instance->_HidePressStart();
}

HRESULT CXuiSceneBase::SetPlayerBaseScenePosition( unsigned int iPad, EBaseScenePosition position )
{
	return CXuiSceneBase::Instance->_SetPlayerBaseScenePosition(iPad, position );
}

HRESULT CXuiSceneBase::SetPlayerBasePositions(EBaseScenePosition pad0, EBaseScenePosition pad1, EBaseScenePosition pad2, EBaseScenePosition pad3)
{
	SetPlayerBaseScenePosition( 0, pad0 );
	SetPlayerBaseScenePosition( 1, pad1 );
	SetPlayerBaseScenePosition( 2, pad2 );
	SetPlayerBaseScenePosition( 3, pad3 );

	return S_OK;
}

HRESULT CXuiSceneBase::UpdatePlayerBasePositions()
{
	EBaseScenePosition padPositions[XUSER_MAX_COUNT];

	for(unsigned int idx = 0; idx < XUSER_MAX_COUNT; ++idx)
	{
		padPositions[idx] = e_BaseScene_NotSet;
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// If the game is not started (or is being held paused for a bit) then display all scenes fullscreen
	if( pMinecraft == nullptr )
	{
		for( BYTE idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			padPositions[idx] = e_BaseScene_Fullscreen;
		}
	}

	// If the game is not in split-screen, then display the primary pad at fullscreen
	else if(app.GetLocalPlayerCount()<2)
	{
		int primaryPad = ProfileManager.GetPrimaryPad();
		padPositions[primaryPad] = e_BaseScene_Fullscreen;

		// May need to turn off the player who just left
		for( BYTE idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			DisplayGamertag(idx,FALSE);
		}
	}

	// We are in splitscreen so work out where each player should be
	else
	{

		for( BYTE idx = 0; idx < XUSER_MAX_COUNT; ++idx)
		{
			if(pMinecraft->localplayers[idx] != nullptr)
			{
				if(pMinecraft->localplayers[idx]->m_iScreenSection==C4JRender::VIEWPORT_TYPE_FULLSCREEN)
				{
					DisplayGamertag(idx,FALSE);
				}
				else
				{
					DisplayGamertag(idx,TRUE);
				}

				switch( pMinecraft->localplayers[idx]->m_iScreenSection)
				{
				case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
					padPositions[idx] = e_BaseScene_Fullscreen;
					break;
				case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
					padPositions[idx] = e_BaseScene_Top;
					break;
				case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
					padPositions[idx] = e_BaseScene_Bottom;
					break;
				case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
					padPositions[idx] = e_BaseScene_Left;
					break;
				case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
					padPositions[idx] = e_BaseScene_Right;
					break;
				case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
					padPositions[idx] = e_BaseScene_Top_Left;
					break;
				case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
					padPositions[idx] = e_BaseScene_Top_Right;
					break;
				case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
					padPositions[idx] = e_BaseScene_Bottom_Left;
					break;
				case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
					padPositions[idx] = e_BaseScene_Bottom_Right;
					break;
				}
			}
			else
			{
				padPositions[idx] = e_BaseScene_NotSet;
				DisplayGamertag(idx,FALSE);
			}
		}
	}

	return SetPlayerBasePositions(padPositions[0], padPositions[1], padPositions[2], padPositions[3]);
}

CXuiSceneBase::EBaseScenePosition CXuiSceneBase::GetPlayerBasePosition(int iPad)
{
	return CXuiSceneBase::Instance->_GetPlayerBasePosition(iPad);
}

void CXuiSceneBase::UpdateSelectedItemPos(int iPad)
{
	CXuiSceneBase::Instance->_UpdateSelectedItemPos(iPad);
}

HXUIOBJ CXuiSceneBase::GetPlayerBaseScene(int iPad)
{
	return CXuiSceneBase::Instance->_GetPlayerBaseScene(iPad);
}

HRESULT CXuiSceneBase::PlayUISFX(ESoundEffect eSound)
{
		return CXuiSceneBase::Instance->_PlayUISFX(eSound);
}

void CXuiSceneBase::SetEmptyQuadrantLogo(int iScreenSection)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	EBaseScenePosition ePos=e_BaseScene_Top_Left;
	int iPad;
	// find the empty player
	for( iPad = 0; iPad < XUSER_MAX_COUNT; ++iPad)
	{
		if(pMinecraft->localplayers[iPad] == nullptr)
		{
			switch( iScreenSection)
			{
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
				ePos = e_BaseScene_Top_Left;
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
				ePos = e_BaseScene_Top_Right;
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
				ePos = e_BaseScene_Bottom_Left;
				break;
			case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
				ePos = e_BaseScene_Bottom_Right;
				break;
			}
			break;
		}
	}

	CXuiSceneBase::Instance->_SetEmptyQuadrantLogo(iPad,ePos);
}

HRESULT CXuiSceneBase::DisplayGamertag( unsigned int iPad, BOOL bDisplay )
{

	CXuiSceneBase::Instance->_DisplayGamertag(iPad,bDisplay);
	return S_OK;
}

void CXuiSceneBase::SetSelectedItem( unsigned int iPad, const wstring &name)
{
	CXuiSceneBase::Instance->_SetSelectedItem(iPad,name);
}

void CXuiSceneBase::HideAllGameUIElements()
{
	CXuiSceneBase::Instance->_HideAllGameUIElements();
}

bool CXuiSceneBase::GetBaseSceneSafeZone( unsigned int iPad, D3DXVECTOR2 &origin, float &width, float &height )
{
	return CXuiSceneBase::Instance->_GetBaseSceneSafeZone(iPad,origin,width,height);
}


#ifndef _XBOX
void CXuiSceneBase::CreateBaseSceneInstance()
{
	CXuiSceneBase *sceneBase = new CXuiSceneBase();
	BOOL handled;
	sceneBase->OnInit(nullptr,handled);
}
#endif