#pragma once
using namespace std;

#include "stdafx.h"
#include "Language.h"

class I18n
{
private:
	static Language *lang;

public:
	template<typename ...Args>
    static wstring get(Args... args)
	{
		return lang->getElement(std::forward<Args>(args)...);
	}
};