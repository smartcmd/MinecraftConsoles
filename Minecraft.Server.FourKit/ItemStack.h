#pragma once

public ref class ItemStack
{
public:
	ItemStack(int typeId, int amount)
		: ItemStack(typeId, amount, 0)
	{
	}

	ItemStack(int typeId, int amount, int data)
		: m_typeId(typeId), m_amount(amount), m_data(data)
	{
	}

	int getTypeId() { return m_typeId; }
	int getAmount() { return m_amount; }
	int getData() { return m_data; }

private:
	int m_typeId;
	int m_amount;
	int m_data;
};