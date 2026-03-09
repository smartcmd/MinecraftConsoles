#pragma once

using namespace System;

public ref class Block
{
public:
	Block(int x, int y, int z, int dimension, int type, int data);

	void breakNaturally();
	int getX();
	int getY();
	int getZ();
	int getType();
	void setType(int id);
	void setData(int aux);
	int getData(int aux);

private:
	int m_x;
	int m_y;
	int m_z;
	int m_dimension;
	int m_type;
	int m_data;
};
