#include "stdafx.h"
#include "KeyMapping.h"

KeyMapping::KeyMapping(const wstring& name, int key)
{
	this->name = name;
	this->key = key;
	this->defaultKey = key;
}

void KeyMapping::resetToDefault()
{
	this->key = this->defaultKey;
}