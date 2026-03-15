#include "Block.h"
#include "NativeBlockCallbacks.h"

Block::Block(int x, int y, int z, int dimension, int type, int data)
	: m_x(x), m_y(y), m_z(z), m_dimension(dimension), m_type(type), m_data(data) {}

void Block::breakNaturally()
{
	NativeBlockCallbacks::BreakNaturally(m_x, m_y, m_z, m_dimension);
	m_type = 0;
	m_data = 0;
}

int Block::getX()
{
	return m_x;
}

int Block::getY()
{
	return m_y;
}

int Block::getZ()
{
	return m_z;
}

int Block::getType()
{
	m_type = NativeBlockCallbacks::GetBlockType(m_x, m_y, m_z, m_dimension);
	return m_type;
}

void Block::setType(int id)
{
	m_type = id;
	NativeBlockCallbacks::SetBlockType(m_x, m_y, m_z, m_dimension, id);
}

void Block::setData(int aux)
{
	m_data = aux;
	NativeBlockCallbacks::SetBlockData(m_x, m_y, m_z, m_dimension, aux);
}

int Block::getData(int aux)
{
	m_data = NativeBlockCallbacks::GetBlockData(m_x, m_y, m_z, m_dimension);
	return m_data;
}
