// Minecraft.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "../../Minecraft.h"
#include "../../Common/Tutorial/TutorialMode.h"
#include "../../Font.h"
#include "../../../Minecraft.World/Random.h"
#include "../../../Minecraft.World/SharedConstants.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "XUI_Scene_Win.h"

BYTE CScene_Win::s_winUserIndex = 0;

const float CScene_Win::AUTO_SCROLL_SPEED = 1.0f;
const float CScene_Win::PLAYER_SCROLL_SPEED = 3.0f;

//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_Win::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iPad = *static_cast<int *>(pInitData->pvInitData);

	m_bIgnoreInput = false;

	MapChildControls();

	// Display the tooltips
	ui.SetTooltips( m_iPad, -1, IDS_TOOLTIPS_CONTINUE);

	wstring halfScreenLineBreaks;

	if(RenderManager.IsHiDef())
	{
		// HD - 17 line page
		halfScreenLineBreaks = L"<br/><br/><br/><br/><br/><br/><br/><br/><br/><br/><br/>";
	}
	else
	{
		// 480 - 14 line page
		halfScreenLineBreaks = L"<br/><br/><br/><br/><br/><br/><br/><br/>";
	}

	noNoiseString = L"<font size=\"24\"><ds>";
	noNoiseString.append(halfScreenLineBreaks);
	noNoiseString.append(halfScreenLineBreaks);
	noNoiseString.append( app.GetString(IDS_WIN_TEXT) );
	noNoiseString.append( app.GetString(IDS_WIN_TEXT_PART_2) );
	noNoiseString.append( app.GetString(IDS_WIN_TEXT_PART_3) );

	noNoiseString.append(halfScreenLineBreaks);

	noNoiseString.append( L"</ds></font>" );

	noNoiseString = app.FormatHTMLString(m_iPad, noNoiseString, 0xff000000);

	Random random(8124371);
	size_t found=noNoiseString.find_first_of(L"{");
	size_t length;
	while (found!=string::npos)
	{
		length = random.nextInt(4) + 3;
		m_noiseLengths.push_back(length);
		found=noNoiseString.find_first_of(L"{", found + 1);
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if(pMinecraft->localplayers[s_winUserIndex] != nullptr)
	{
		noNoiseString = replaceAll(noNoiseString,L"{*PLAYER*}",pMinecraft->localplayers[s_winUserIndex]->name);
	}
	else
	{
		noNoiseString = replaceAll(noNoiseString,L"{*PLAYER*}",pMinecraft->localplayers[ProfileManager.GetPrimaryPad()]->name);
	}

	updateNoise();

	m_htmlControl.SetText(noiseString.c_str());

	//wstring result = m_htmlControl.GetText();

	//wcout << result.c_str();

	m_scrollDir = 1;
	HRESULT hr = XuiHtmlControlSetSmoothScroll(m_htmlControl.m_hObj, XUI_SMOOTHSCROLL_VERTICAL,TRUE,AUTO_SCROLL_SPEED,1.0f,AUTO_SCROLL_SPEED);
	XuiHtmlControlVScrollBy(m_htmlControl.m_hObj,m_scrollDir * 1000);

	SetTimer(0,200);

	return S_OK;
}

HRESULT CScene_Win::OnTimer( XUIMessageTimer *pXUIMessageTimer, BOOL &bHandled)
{
	if(!TreeHasFocus()) return S_OK;

	// This is required to animate the "noise" strings in the text, however this makes scrolling really jumpy
	// as well as causing line lengths to change as we do not have a monspaced font. Disabling for now.
#if 0
	updateNoise();

	XUIHtmlScrollInfo scrollInfo;
	HRESULT hr =  m_htmlControl.GetVScrollInfo(&scrollInfo);
	m_htmlControl.SetText(noiseString.c_str());
	//wcout << noiseString.c_str();
	//hr = m_htmlControl.SetVScrollPos(scrollInfo.nPos+2);
	hr = m_htmlControl.SetVScrollPos(scrollInfo.nPos);
#endif
	XuiHtmlControlVScrollBy(m_htmlControl.m_hObj,m_scrollDir * 1000);

	return S_OK;
}

HRESULT CScene_Win::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_ESCAPE:
		{
			KillTimer(0);
			m_bIgnoreInput = true;
			Minecraft *pMinecraft = Minecraft::GetInstance();
			app.CloseAllPlayersXuiScenes();
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if(pMinecraft->localplayers[i] != nullptr)
				{
					app.SetAction(i,eAppAction_Respawn);
				}
			}

			// Show the other players scenes
			CXuiSceneBase::ShowOtherPlayersBaseScene(pInputData->UserIndex, true);
			// This just allows it to be shown
			if(pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()] != nullptr) pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(true);
			ui.UpdatePlayerBasePositions();

			rfHandled = TRUE;
		}
		break;
	case VK_PAD_RTHUMB_UP:
	case VK_PAD_LTHUMB_UP:
		m_scrollDir = -1;
		// Fall through
	case VK_PAD_RTHUMB_DOWN:
	case VK_PAD_LTHUMB_DOWN:
		{
			HRESULT hr = XuiHtmlControlSetSmoothScroll(m_htmlControl.m_hObj, XUI_SMOOTHSCROLL_VERTICAL,TRUE,AUTO_SCROLL_SPEED,1.0f,PLAYER_SCROLL_SPEED);
		}
		break;
	}

	return S_OK;
}

HRESULT CScene_Win::OnKeyUp(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_RTHUMB_UP:
	case VK_PAD_LTHUMB_UP:
	case VK_PAD_RTHUMB_DOWN:
	case VK_PAD_LTHUMB_DOWN:
		{
			m_scrollDir = 1;
			HRESULT hr = XuiHtmlControlSetSmoothScroll(m_htmlControl.m_hObj, XUI_SMOOTHSCROLL_VERTICAL,TRUE,AUTO_SCROLL_SPEED,1.0f,AUTO_SCROLL_SPEED);
		}
		break;
	}

	return S_OK;
}

HRESULT CScene_Win::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}

void CScene_Win::updateNoise()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	noiseString = noNoiseString;

	size_t length = 0;
	wchar_t replacements[64];
	wstring replaceString = L"";
	wchar_t randomChar = L'a';
	Random *random = pMinecraft->font->random;

	bool darken = false;

	wstring tag = L"{*NOISE*}";

    auto it = m_noiseLengths.begin();
    size_t found = noiseString.find_first_of(L"{");
	while (found!=string::npos && it != m_noiseLengths.end() )
	{
		length = *it;
		++it;

		replaceString = L"";
		for(size_t i = 0; i < length; ++i)
		{
			randomChar = SharedConstants::acceptableLetters[random->nextInt(SharedConstants::acceptableLetters.length())];

			wstring randomCharStr = L"";
			randomCharStr.push_back(randomChar);
			if(randomChar == L'<')
			{
				randomCharStr = L"&lt;";
			}
			else if (randomChar == L'>' )
			{
				randomCharStr = L"&gt;";
			}
			else if(randomChar == L'"')
			{
				randomCharStr = L"&quot;";
			}
			else if(randomChar == L'&')
			{
				randomCharStr = L"&amp;";
			}
			else if(randomChar == L'\\')
			{
				randomCharStr = L"\\\\";
			}
			else if(randomChar == L'{')
			{
				randomCharStr = L"}";
			}

			int randomVal = random->nextInt(2);
			eMinecraftColour colour = eHTMLColor_8;
			if(randomVal == 1) colour = eHTMLColor_9;
			else if(randomVal == 2) colour = eHTMLColor_a;
			ZeroMemory(replacements,64*sizeof(wchar_t));
			swprintf(replacements,64,L"<font color=\"#%08x\" shadowcolor=\"#80000000\">%ls</font>",app.GetHTMLColour(colour),randomCharStr.c_str());
			replaceString.append(replacements);
		}

		noiseString.replace( found, tag.length(), replaceString );

		//int pos = 0;
		//do {
		//	pos = random->nextInt(SharedConstants::acceptableLetters.length());
		//} while (pMinecraft->font->charWidths[ch + 32] != pMinecraft->font->charWidths[pos + 32]);
		//ib.put(listPos + 256 + random->nextInt(2) + 8 + (darken ? 16 : 0));
		//ib.put(listPos + pos + 32);

		found = noiseString.find_first_of(L"{",found+1);
	}
}

HRESULT CScene_Win::OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled)
{
	SetTimer(0,200);

	// turn off the gamertags in splitscreen for the primary player, since they are about to be made fullscreen
	CXuiSceneBase::HideAllGameUIElements();

	// Hide the other players scenes
	CXuiSceneBase::ShowOtherPlayersBaseScene(ProfileManager.GetPrimaryPad(), false);

	// This just allows it to be shown
	if(Minecraft::GetInstance()->localgameModes[ProfileManager.GetPrimaryPad()] != nullptr) Minecraft::GetInstance()->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(false);

	// Temporarily make this scene fullscreen
	CXuiSceneBase::SetPlayerBaseScenePosition( ProfileManager.GetPrimaryPad(), CXuiSceneBase::e_BaseScene_Fullscreen );

	// Display the tooltips
	ui.SetTooltips( m_iPad, -1, IDS_TOOLTIPS_CONTINUE);

	XuiHtmlControlVScrollBy(m_htmlControl.m_hObj,m_scrollDir * 1000);

	return S_OK;
}

HRESULT CScene_Win::OnNavForward(XUIMessageNavForward *pNavForwardData, BOOL& bHandled)
{
	XuiHtmlControlVScrollBy(m_htmlControl.m_hObj,0);
	XUIHtmlScrollInfo scrollInfo;
	HRESULT hr =  m_htmlControl.GetVScrollInfo(&scrollInfo);
	hr = m_htmlControl.SetVScrollPos(scrollInfo.nPos);
	KillTimer(0);

	return S_OK;
}