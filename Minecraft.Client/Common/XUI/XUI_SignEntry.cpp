#include "stdafx.h"
#include "..\..\..\MultiplayerLevel.h"
#include "..\..\..\Minecraft.World\SignTileEntity.h"
#include "..\..\..\Minecraft.World\Entity.h"
#include "..\..\..\Minecraft.World\Level.h"
#include "..\..\..\Minecraft.Client\LocalPlayer.h"
#include "..\..\..\Minecraft.Client\ClientConnection.h"
#include "..\..\..\Minecraft.Client\MultiPlayerLocalPlayer.h"
#include "XUI_SignEntry.h"

HRESULT CScene_SignEntry::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	MapChildControls();
	
	XuiControlSetText(m_ButtonDone,app.GetString(IDS_DONE));
	XuiControlSetText(m_labelEditSign,app.GetString(IDS_EDIT_SIGN_MESSAGE));

	SignEntryScreenInput* initData = static_cast<SignEntryScreenInput *>(pInitData->pvInitData);
	m_sign = initData->sign;
	
	CXuiSceneBase::ShowDarkOverlay( initData->iPad, TRUE );
	CXuiSceneBase::ShowLogo( initData->iPad, FALSE);
	ui.SetTooltips( initData->iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,initData->iPad);
	}


	for(unsigned int i = 0; i<SIGN_ENTRY_ROWS_MAX; ++i)
	{
		// Have to have the Latin alphabet here, since that's what we have on the sign in-game
		// but because the JAP/KOR/CHN fonts don't have extended European characters, let's restrict those languages to not having the extended character set, since they can't see what they are typing
		switch(XGetLanguage())
		{
		case XC_LANGUAGE_JAPANESE:
		case XC_LANGUAGE_TCHINESE:
		case XC_LANGUAGE_KOREAN:
			m_signRows[i].SetKeyboardType(C_4JInput::EKeyboardMode_Alphabet);
			break;
		default:
			m_signRows[i].SetKeyboardType(C_4JInput::EKeyboardMode_Full);
			break;
		}

		m_signRows[i].SetText( m_sign->GetMessage(i).c_str() );
		m_signRows[i].SetTextLimit(15);
		// Set the title and desc for the edit keyboard popup
		m_signRows[i].SetTitleAndText(IDS_SIGN_TITLE,IDS_SIGN_TITLE_TEXT);
	}


	return S_OK;
}

HRESULT CScene_SignEntry::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_ButtonDone)
	{
		// Set the sign text here so we on;y call the verify once it has been set, not while we're typing in to it
		for(int i=0;i<4;i++)
		{
			wstring temp=m_signRows[i].GetText();
			m_sign->SetMessage(i,temp);		
		}

		m_sign->setChanged();

		Minecraft *pMinecraft=Minecraft::GetInstance();
		// need to send the new data
		if (pMinecraft->level->isClientSide)
		{
			shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[pNotifyPressData->UserIndex];
			if(player != nullptr && player->connection && player->connection->isStarted())
			{
				player->connection->send(std::make_shared<SignUpdatePacket>(m_sign->x, m_sign->y, m_sign->z, m_sign->IsVerified(), m_sign->IsCensored(), m_sign->GetMessages()));
			}
		}
		app.CloseXuiScenes(pNotifyPressData->UserIndex);
	}
	return S_OK;
}

HRESULT CScene_SignEntry::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{
		case VK_PAD_B:
		case VK_ESCAPE:
			// user backed out, so wipe the sign
			wstring temp=L"";

			for(int i=0;i<4;i++)
			{
				m_sign->SetMessage(i,temp);
			}

			app.CloseXuiScenes(pInputData->UserIndex);
			rfHandled = TRUE;
	
			CXuiSceneBase::PlayUISFX(eSFX_Back);
		break;
	}


	return S_OK;
}