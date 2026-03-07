#include "stdafx.h"

#include "../../../Minecraft.World/StringHelpers.h"

#include "UIString.h"

bool UIString::setCurrentLanguage()
{
	int nextLanguage, nextLocale;
	nextLanguage = XGetLanguage();
	nextLocale = XGetLocale();

	if ( (nextLanguage != s_currentLanguage) || (nextLocale != s_currentLocale) )
	{
		s_currentLanguage = nextLanguage;
		s_currentLocale = nextLocale;
		return true;
	}

	return false;
}

int UIString::getCurrentLanguage()
{
	return s_currentLanguage;
}

UIString::UIStringCore::UIStringCore(StringBuilder wstrBuilder)
{
	m_bIsConstant = false;

	m_lastSetLanguage = m_lastSetLocale = -1;
	m_lastUpdatedLanguage = m_lastUpdatedLocale = -1;

	m_fStringBuilder = wstrBuilder;

	m_wstrCache = L"";
	update(true);
}

UIString::UIStringCore::UIStringCore(const wstring &str)
{
	m_bIsConstant = true;
	
	m_lastSetLanguage = m_lastSetLocale = -1;
	m_lastUpdatedLanguage = m_lastUpdatedLocale = -1;

	m_wstrCache = str;
}

wstring &UIString::UIStringCore::getString()
{
	if (hasNewString()) update(true);
	return m_wstrCache;
}

bool UIString::UIStringCore::hasNewString()
{
	if (m_bIsConstant) return false;
	return (m_lastSetLanguage != s_currentLanguage) || (m_lastSetLocale != s_currentLocale);
}

bool UIString::UIStringCore::update(bool force)
{
	if ( !m_bIsConstant && (force || hasNewString()) )
	{
		m_wstrCache = m_fStringBuilder();
		m_lastSetLanguage = s_currentLanguage;
		m_lastSetLocale = s_currentLocale;
		return true;
	}
	return false;
}

bool UIString::UIStringCore::needsUpdating()
{
	if (m_bIsConstant) return false;
	return (m_lastSetLanguage != s_currentLanguage) || (m_lastUpdatedLanguage != m_lastSetLanguage)
		|| (m_lastSetLocale != s_currentLocale) || (m_lastUpdatedLocale != m_lastSetLocale);
}

void UIString::UIStringCore::setUpdated()
{
	m_lastUpdatedLanguage = m_lastSetLanguage;
	m_lastUpdatedLocale = m_lastSetLocale;
}

int UIString::s_currentLanguage = -1;
int UIString::s_currentLocale = -1;

UIString::UIString()
{
	m_core = shared_ptr<UIStringCore>();
}

UIString::UIString(int ids)
{
#ifdef __PS3__
	StringBuilder builder = StringBuilder( new IdsStringBuilder(ids) );
#else
	StringBuilder builder = [ids](){ return app.GetString(ids); };
#endif
	UIStringCore *core = new UIStringCore( builder );
	m_core = shared_ptr<UIStringCore>(core);
}

UIString::UIString(StringBuilder wstrBuilder)
{
	UIStringCore *core = new UIStringCore(wstrBuilder);
	m_core = shared_ptr<UIStringCore>(core);
}

UIString::UIString(const string &constant)
{
	wstring wstr = convStringToWstring(constant);
	UIStringCore *core = new UIStringCore( wstr );
	m_core = shared_ptr<UIStringCore>(core);
}

UIString::UIString(const wstring &constant)
{
	UIStringCore *core = new UIStringCore(constant);
	m_core = shared_ptr<UIStringCore>(core);
}

UIString::UIString(const wchar_t *constant)
{
	wstring str = wstring(constant);
	UIStringCore *core = new UIStringCore(str);
	m_core = shared_ptr<UIStringCore>(core);
}

UIString::~UIString()
{
#ifndef __PS3__
	m_core = nullptr;
#endif
}

bool UIString::empty()
{
	return m_core.get() == NULL;
}

bool UIString::compare(const UIString &uiString)
{
	return m_core.get() != uiString.m_core.get();
}

bool UIString::needsUpdating()
{
	if (m_core != NULL)	return m_core->needsUpdating();
	else				return false;
}

void UIString::setUpdated()
{
	if (m_core != NULL)	m_core->setUpdated();
}

wstring &UIString::getString()
{
	static wstring blank(L"");
	if (m_core != NULL) return m_core->getString();
	else				return blank;
}

const wchar_t *UIString::c_str()
{
	return getString().c_str();
}

unsigned int UIString::length()
{
	return getString().length();
}