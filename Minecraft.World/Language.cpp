#include "stdafx.h"
#include "Language.h"

// 4J - TODO - properly implement

Language *Language::singleton = new Language();

Language::Language()
{
}

Language *Language::getInstance()
{
	return singleton;
}

std::wstring Language::getElementName(const std::wstring& elementId)
{
	return elementId;
}

std::wstring Language::getElementDescription(const std::wstring& elementId)
{
	return elementId;
}