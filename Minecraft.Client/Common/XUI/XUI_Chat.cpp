#include "stdafx.h"
#include "XUI_Chat.h"
#include "../../Minecraft.h"
#include "../../Gui.h"

HRESULT CScene_Chat::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *(int *)pInitData->pvInitData;

	MapChildControls();

	this->SetTimer(0,100);

	XuiElementGetPosition(m_hObj,&m_OriginalPosition);

	return S_OK;
}

HRESULT CScene_Chat::OnTimer( XUIMessageTimer *pXUIMessageTimer, BOOL &bHandled)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	Gui *pGui = pMinecraft->gui;

	//DWORD messagesToDisplay = min( CHAT_LINES_COUNT, pGui->getMessagesCount(m_iPad) );
	for( unsigned int i = 0; i < CHAT_LINES_COUNT; ++i )
	{
		float opacity = pGui->getOpacity(m_iPad, i);
		if( opacity > 0 )
		{
			m_Backgrounds[i].SetOpacity(opacity);
			m_Labels[i].SetOpacity(opacity);
			m_Labels[i].SetText( pGui->getMessage(m_iPad,i).c_str() );
		}
		else
		{
			m_Backgrounds[i].SetOpacity(0);
			m_Labels[i].SetOpacity(0);
		}
	}
	if(pMinecraft->localplayers[m_iPad]!= NULL)
	{
		m_Jukebox.SetText( pGui->getJukeboxMessage(m_iPad).c_str() );
		m_Jukebox.SetOpacity( pGui->getJukeboxOpacity(m_iPad) );
	}
	return S_OK;
}

HRESULT CScene_Chat::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	
	app.ReloadChatScene(m_iPad, bJoining);

	return S_OK;
}

HRESULT CScene_Chat::OffsetTextPosition( float xOffset, float yOffset /*= 0.0f*/ )
{
	D3DXVECTOR3 vPos;
	float fWidth, fHeight;
	XuiElementGetBounds( m_Backgrounds[0], &fWidth, &fHeight );
	for(unsigned int i = 0; i < CHAT_LINES_COUNT; ++i)
	{
		XuiElementGetPosition( m_Labels[i], &vPos );
		vPos.x = xOffset;
		vPos.y += yOffset;
		XuiElementSetPosition( m_Labels[i], &vPos );
		XuiElementSetBounds( m_Labels[i], fWidth - xOffset, fHeight );
	}
	return S_OK;
}