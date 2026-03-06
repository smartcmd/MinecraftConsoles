#include "stdafx.h"

#include "FlatLayerInfo.h"

void FlatLayerInfo::_init(int height, int id)
{
	this->height = height;
	this->id = id;
	data = 0;
	start = 0;
}

FlatLayerInfo::FlatLayerInfo(int height, int id)
{
	_init(height, id);
}

FlatLayerInfo::FlatLayerInfo(int height, int id, int data)
{
	_init(height, id);
	this->data = data;
}

int FlatLayerInfo::getHeight()
{
	return height;
}

void FlatLayerInfo::setHeight(int height)
{
	this->height = height;
}

int FlatLayerInfo::getId()
{
	return id;
}

void FlatLayerInfo::setId(int id)
{
	this->id = id;
}

int FlatLayerInfo::getData()
{
	return data;
}

void FlatLayerInfo::setData(int data)
{
	this->data = data;
}

int FlatLayerInfo::getStart()
{
	return start;
}

void FlatLayerInfo::setStart(int start)
{
	this->start = start;
}

wstring FlatLayerInfo::toString()
{
	wstring result = std::to_wstring(id);

	if (height > 1)
	{
		result = std::to_wstring(height) + L"x" + result;
	}
	if (data > 0)
	{
		result += L":" + std::to_wstring(data);
	}

	return result;
}