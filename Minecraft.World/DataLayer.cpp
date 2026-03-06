#include "stdafx.h"
#include "DataLayer.h"

DataLayer::DataLayer(int length, int depthBits) : depthBits(depthBits), depthBitsPlusFour(depthBits + 4)
{
	data = byteArray(length >> 1);
}

DataLayer::DataLayer(byteArray data, int depthBits) : depthBits(depthBits), depthBitsPlusFour(depthBits + 4)
{
	this->data = data;
}

DataLayer::~DataLayer()
{
	delete[] data.data;
}

int DataLayer::get(int x, int y, int z)
{
    int pos = (x << depthBitsPlusFour | z << depthBits | y);
    int slot = pos >> 1;
    int part = pos & 1;

    if (part == 0)
	{
        return data[slot] & 0xf;
    } else
	{
        return (data[slot] >> 4) & 0xf;
    }
}

void DataLayer::set(int x, int y, int z, int val)
{
    int pos = (x << depthBitsPlusFour | z << depthBits | y);

    int slot = pos >> 1;
    int part = pos & 1;

    if (part == 0)
	{
        data[slot] = (uint8_t) ((data[slot] & 0xf0) | (val & 0xf));
    } else
	{
        data[slot] = (uint8_t) ((data[slot] & 0x0f) | ((val & 0xf) << 4));
    }
}

bool DataLayer::isValid()
{
	return data.data != NULL;
}

void DataLayer::setAll(int br)
{
    uint8_t val = (uint8_t) (br & (br << 4));
    for (unsigned int i = 0; i < data.length; i++)
	{
        data[i] = val;
    }
}