#include "stdafx.h"
#include "UI.h"
#include "UIScene_EndPoem.h"
#include "UIBitmapFont.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/StringHelpers.h"

UIScene_EndPoem::UIScene_EndPoem(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{

	//ui.setFontCachingCalculationBuffer(20000);

	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_bIgnoreInput = false;

	// 4J Stu - Don't need these, the AS handles the scrolling and makes it look nice
#if 0
	wstring halfScreenLineBreaks;

	if(RenderManager.IsHiDef())
	{
		// HD - 17 line page
		halfScreenLineBreaks = L"<br /><br /><br /><br /><br /><br /><br /><br /><br /><br /><br />";
	}
	else
	{
		// 480 - 14 line page
		halfScreenLineBreaks = L"<br /><br /><br /><br /><br /><br /><br /><br />";
	}
#endif

	//wchar_t startTags[64];
	//swprintf(startTags,64,L"<font size=\"%d\">",app.GetHTMLFontSize(eHTMLSize_EndPoem));
	//noNoiseString.append(halfScreenLineBreaks);
	//noNoiseString.append(halfScreenLineBreaks);
	noNoiseString.append( app.GetString(IDS_WIN_TEXT) );
	noNoiseString.append( app.GetString(IDS_WIN_TEXT_PART_2) );
	noNoiseString.append( app.GetString(IDS_WIN_TEXT_PART_3) );

	//noNoiseString.append(halfScreenLineBreaks);

	// 4J Stu - Iggy seems to strip our trailing linebreaks, so added a space to made sure it scrolls this far
	noNoiseString.append( L" " );

	noNoiseString = app.FormatHTMLString(m_iPad, noNoiseString, 0xff000000);

	Minecraft *pMinecraft = Minecraft::GetInstance();

	wstring playerName = L"";
	if(pMinecraft->localplayers[ui.GetWinUserIndex()] != NULL)
	{
		playerName = escapeXML( pMinecraft->localplayers[ui.GetWinUserIndex()]->getDisplayName() );
	}
	else
	{
		playerName = escapeXML( pMinecraft->localplayers[ProfileManager.GetPrimaryPad()]->getDisplayName() );
	}
	noNoiseString = replaceAll(noNoiseString,L"{*PLAYER*}",playerName);

	Random random(8124371);
	size_t found=noNoiseString.find(L"{*NOISE*}");
	int length;
	while (found!=string::npos)
	{
		length = random.nextInt(4) + 3;
		m_noiseLengths.push_back(length);
		found=noNoiseString.find(L"{*NOISE*}",found+1);
	}

	updateNoise();


	// 4J-JEV: Find paragraph start and end points.
	m_paragraphs = vector<wstring>();
	int lastIndex = 0;
	for (	int index = 0;
			index != wstring::npos;
			index = noiseString.find(L"<br /><br />", index+12, 12)
		)
	{
		m_paragraphs.push_back( noiseString.substr(lastIndex, index-lastIndex) );
		lastIndex = index;
	}
	//lastIndex += 12;
	m_paragraphs.push_back( noiseString.substr( lastIndex, noiseString.length() - lastIndex ) );

	//m_htmlPoem.init(noiseString.c_str());
	//m_htmlPoem.startAutoScroll();

	//wstring result = m_htmlControl.GetText();

	//wcout << result.c_str();

#if TO_BE_IMPLEMENTED
	m_scrollDir = 1;
	HRESULT hr = XuiHtmlControlSetSmoothScroll(m_htmlControl.m_hObj, XUI_SMOOTHSCROLL_VERTICAL,TRUE,AUTO_SCROLL_SPEED,1.0f,AUTO_SCROLL_SPEED);
	XuiHtmlControlVScrollBy(m_htmlControl.m_hObj,m_scrollDir * 1000);

	SetTimer(0,200);
#endif

	m_requestedLabel = 0;
}

wstring UIScene_EndPoem::getMoviePath()
{
	return L"EndPoem";
}

void UIScene_EndPoem::updateTooltips()
{
	ui.SetTooltips( XUSER_INDEX_ANY, -1, m_bIgnoreInput?-1:IDS_TOOLTIPS_CONTINUE);
}

void UIScene_EndPoem::tick()
{
	UIScene::tick();

	if( m_requestedLabel >= 0 && m_requestedLabel < m_paragraphs.size())
	{
		wstring label = m_paragraphs[m_requestedLabel];

		IggyDataValue result;
		IggyDataValue value[3];

		IggyStringUTF16 stringVal;
		stringVal.string = (IggyUTF16*)label.c_str();
		stringVal.length = label.length();
		value[0].type = IGGY_DATATYPE_string_UTF16;
		value[0].string16 = stringVal;

		value[1].type = IGGY_DATATYPE_number;
		value[1].number = m_requestedLabel;

		value[2].type = IGGY_DATATYPE_boolean;
		value[2].boolval = (m_requestedLabel == (m_paragraphs.size() - 1));

		IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetNextLabel , 3 , value );

		m_requestedLabel = -1;
	}
}

void UIScene_EndPoem::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

	if(pressed) ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			m_bIgnoreInput = true;
			Minecraft *pMinecraft = Minecraft::GetInstance();
			for(unsigned int i = 0; i < XUSER_MAX_COUNT; ++i)
			{
				if(pMinecraft->localplayers[i] != NULL)
				{
					app.SetAction(i,eAppAction_Respawn);
				}
			}

			// This just allows it to be shown
			if(pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()] != NULL) pMinecraft->localgameModes[ProfileManager.GetPrimaryPad()]->getTutorial()->showTutorialPopup(true);

			updateTooltips();
			navigateBack();

			handled = true;
		}
		break;
	case ACTION_MENU_DOWN:
	case ACTION_MENU_UP:
	case ACTION_MENU_OTHER_STICK_DOWN:
	case ACTION_MENU_OTHER_STICK_UP:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_EndPoem::handleDestroy()
{

	//ui.setFontCachingCalculationBuffer(-1);
}

void UIScene_EndPoem::handleRequestMoreData(F64 startIndex, bool up)
{
	m_requestedLabel = (int)startIndex;
}

void UIScene_EndPoem::updateNoise()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	noiseString = noNoiseString;

	int length = 0;
	wchar_t replacements[64];
	wstring replaceString = L"";
	wchar_t randomChar = L'a';
	Random *random = pMinecraft->font->random;

	bool darken = false;

	wstring tag = L"{*NOISE*}";

    auto it = m_noiseLengths.begin();
    size_t found= noiseString.find(tag);
	while (found!=string::npos && it != m_noiseLengths.end() )
	{
		length = *it;
		++it;

		replaceString = L"";
		for(int i = 0; i < length; ++i)
		{
			if (ui.UsingBitmapFont())
			{
				randomChar = SharedConstants::acceptableLetters[random->nextInt((int)SharedConstants::acceptableLetters.length())];
			}
			else
			{
				// 4J-JEV: It'd be nice to avoid null characters when using asian languages.
				static wstring acceptableLetters = L"!\"#$%&'()*+,-./0123456789:;<=>?@[\\]^_'|}~";
				randomChar = acceptableLetters[ random->nextInt((int)acceptableLetters.length()) ];
			}

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

		found=noiseString.find(tag,found+1);
	}
}