#pragma once

#include <string>

class Language
{
private:
	static Language *singleton;
public:
	Language();
    static Language *getInstance();
    template<typename...Args>
    inline std::wstring getElement(const std::wstring& elementId, Args...)
    {
        return elementId;
    }
    std::wstring getElementName(const std::wstring& elementId);
    std::wstring getElementDescription(const std::wstring& elementId);
};