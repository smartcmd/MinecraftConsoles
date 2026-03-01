#include "stdafx.h"
#include "Language.h"
#include "I18n.h"

Language *I18n::lang = Language::getInstance();
wstring I18n::get(const wstring& id, ...)
{
#ifdef __PSVITA__		// 4J - vita doesn't like having a reference type as the last parameter passed to va_start - we shouldn't need this method anyway
	return L"";
#else
	va_list va;
	va_start(va, id);
    return I18n::get(id, va);
#endif
}

wstring I18n::get(const wstring& id, va_list args)
{
	return lang->getElement(id, args);
}
