#include "stdafx.h"
#include "UI.h"
#include "UIControl_EnchantmentButton.h"
#include "../../../Minecraft.World/net.minecraft.world.inventory.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/StringHelpers.h"

UIControl_EnchantmentButton::UIControl_EnchantmentButton()
{
	m_index = 0;
	m_lastState = eState_Inactive;
	m_lastCost = 0;
	m_enchantmentString = L"";
	m_bHasFocus = false;

	m_textColour = app.GetHTMLColour(eTextColor_Enchant);
	m_textFocusColour = app.GetHTMLColour(eTextColor_EnchantFocus);
	m_textDisabledColour = app.GetHTMLColour(eTextColor_EnchantDisabled);
}

bool UIControl_EnchantmentButton::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eEnchantmentButton);
	bool success = UIControl_Button::setupControl(scene,parent,controlName);

	//Button specific initialisers
	m_funcChangeState = registerFastName(L"ChangeState");

	return success;
}

void UIControl_EnchantmentButton::init(int index)
{
	m_index = index;
}


void UIControl_EnchantmentButton::ReInit()
{
	UIControl_Button::ReInit();


	m_lastState = eState_Inactive;
	m_lastCost = 0;
	m_bHasFocus = false;
	updateState();
}

void UIControl_EnchantmentButton::tick()
{
	updateState();
	UIControl_Button::tick();
}

void UIControl_EnchantmentButton::render(IggyCustomDrawCallbackRegion *region)
{
	UIScene_EnchantingMenu *enchantingScene = static_cast<UIScene_EnchantingMenu *>(m_parentScene);
	EnchantmentMenu *menu = enchantingScene->getMenu();

	float width = region->x1 - region->x0;
	float height = region->y1 - region->y0;
	float xo = width/2;
	float yo = height;
	//glTranslatef(xo, yo, 50.0f);

	// Revert the scale from the setup
	float ssX = width/m_width;
	float ssY = height/m_height;
	glScalef(ssX, ssY,1.0f);

	float ss = 1.0f;

#if TO_BE_IMPLEMENTED
	if(!enchantingScene->m_bSplitscreen)
#endif
	{
		switch(enchantingScene->getSceneResolution())
		{
		case UIScene::eSceneResolution_1080:
			ss = 3.0f;
			break;
		default:
			ss = 2.0f;
			break;
		}
	}

	glScalef(ss, ss, ss);

	int cost = menu->costs[m_index];

	//if(cost != m_lastCost)
	//{
	//	updateState();
	//}

	glColor4f(1, 1, 1, 1);
	if (cost != 0)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.1f);
		Minecraft *pMinecraft = Minecraft::GetInstance();
		wstring line = std::to_wstring(cost);
		Font *font = pMinecraft->altFont;
		//int col = 0x685E4A;
		unsigned int col = m_textColour;
		if (pMinecraft->localplayers[enchantingScene->getPad()]->experienceLevel < cost && !pMinecraft->localplayers[enchantingScene->getPad()]->abilities.instabuild)
		{
			col = m_textDisabledColour;
			font->drawWordWrap(m_enchantmentString, 0, 0, static_cast<float>(m_width)/ss, col, static_cast<float>(m_height)/ss);
			font = pMinecraft->font;
			//col = (0x80ff20 & 0xfefefe) >> 1;
			//font->drawShadow(line, (bwidth - font->width(line))/ss, 7, col);
		}
		else
		{
			if (m_bHasFocus)
			{
				//col = 0xffff80;
				col = m_textFocusColour;
			}
			font->drawWordWrap(m_enchantmentString, 0, 0, static_cast<float>(m_width)/ss, col, static_cast<float>(m_height)/ss);
			font = pMinecraft->font;
			//col = 0x80ff20;
			//font->drawShadow(line, (bwidth - font->width(line))/ss, 7, col);
		}
		glDisable(GL_ALPHA_TEST);
	}
	else
	{
	}

	//Lighting::turnOff();
	glDisable(GL_RESCALE_NORMAL);
}

void UIControl_EnchantmentButton::updateState()
{
	UIScene_EnchantingMenu *enchantingScene = static_cast<UIScene_EnchantingMenu *>(m_parentScene);
	EnchantmentMenu *menu = enchantingScene->getMenu();

	EState state = eState_Inactive;

	int cost = menu->costs[m_index];

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(cost > pMinecraft->localplayers[enchantingScene->getPad()]->experienceLevel && !pMinecraft->localplayers[enchantingScene->getPad()]->abilities.instabuild)
	{
		// Dark background
		state = eState_Inactive;
	}
	else
	{
		// Light background and focus background
		if(m_bHasFocus)
		{
			state = eState_Selected;
		}
		else
		{
			state = eState_Active;
		}
	}

	if(cost != m_lastCost)
	{
		setLabel( std::to_wstring(cost) );
		m_lastCost = cost;
		m_enchantmentString = EnchantmentNames::instance.getRandomName();
	}
	if(cost == 0)
	{
		// Dark background
		state = eState_Inactive;
		setLabel(L"");
	}

	if(state != m_lastState)
	{
		IggyDataValue result;
		IggyDataValue value[1];

		value[0].type = IGGY_DATATYPE_number;
		value[0].number = static_cast<int>(state);
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcChangeState , 1 , value );

		if(out == IGGY_RESULT_SUCCESS) m_lastState = state;
	}
}

void UIControl_EnchantmentButton::setFocus(bool focus)
{
	m_bHasFocus = focus;
	updateState();
}

UIControl_EnchantmentButton::EnchantmentNames UIControl_EnchantmentButton::EnchantmentNames::instance;

UIControl_EnchantmentButton::EnchantmentNames::EnchantmentNames()
{
	wstring allWords = L"the elder scrolls klaatu berata niktu xyzzy bless curse light darkness fire air earth water hot dry cold wet ignite snuff embiggen twist shorten stretch fiddle destroy imbue galvanize enchant free limited range of towards inside sphere cube self other ball mental physical grow shrink demon elemental spirit animal creature beast humanoid undead fresh stale ";
	std::wistringstream iss(allWords);
	std::copy(std::istream_iterator< std::wstring, wchar_t, std::char_traits<wchar_t> >(iss), std::istream_iterator< std::wstring, wchar_t, std::char_traits<wchar_t> >(),std::back_inserter(words));
}

wstring UIControl_EnchantmentButton::EnchantmentNames::getRandomName()
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
