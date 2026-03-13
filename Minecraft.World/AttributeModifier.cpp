#include "stdafx.h"

#include "AttributeModifier.h"
#include "HtmlString.h"

void AttributeModifier::_init(eMODIFIER_ID id, const wstring name, double amount, int operation)
{
	assert(operation < TOTAL_OPERATIONS);
	this->amount = amount;
	this->operation = operation;
	this->name = name;
	this->id = id;
	this->serialize = true;
}

AttributeModifier::AttributeModifier(double amount, int operation)
{
	// Create an anonymous attribute
	_init(eModifierId_ANONYMOUS, name, amount, operation);
}

AttributeModifier::AttributeModifier(eMODIFIER_ID id, double amount, int operation)
{
	_init(id, name, amount, operation);

	//Validate.notEmpty(name, "Modifier name cannot be empty");
	//Validate.inclusiveBetween(0, TOTAL_OPERATIONS - 1, operation, "Invalid operation");
}

eMODIFIER_ID AttributeModifier::getId()
{
	return id;
}

wstring AttributeModifier::getName()
{
	return name;
}

int AttributeModifier::getOperation()
{
	return operation;
}

double AttributeModifier::getAmount()
{
	return amount;
}

bool AttributeModifier::isSerializable()
{
	return serialize;
}

AttributeModifier *AttributeModifier::setSerialize(bool serialize)
{
	this->serialize = serialize;
	return this;
}

bool AttributeModifier::equals(AttributeModifier *modifier)
{
	if (this == modifier) return true;
	if (modifier == nullptr) return false; //|| getClass() != o.getClass()) return false;

	if (id != modifier->id) return false;

	return true;
}

wstring AttributeModifier::toString()
{
	return L"";

	/*return L"AttributeModifier{" +
	L"amount=" + amount +
	L", operation=" + operation +
	L", name='" + name + '\'' +
	L", id=" + id +
	L", serialize=" + serialize +
	L'}';*/
}

HtmlString AttributeModifier::getHoverText(eATTRIBUTE_ID attribute)
{
	double amount = getAmount();
	double displayAmount;

	if (getOperation() == AttributeModifier::OPERATION_MULTIPLY_BASE || getOperation() == AttributeModifier::OPERATION_MULTIPLY_TOTAL)
	{
		displayAmount = getAmount() * 100.0f;
	}
	else
	{
		displayAmount = getAmount();
	}

	eMinecraftColour color;

	if (amount > 0)
	{
		color = eHTMLColor_9;
	}
	else if (amount < 0)
	{
		displayAmount *= -1;
		color = eHTMLColor_c;
	}

	bool percentage = false;
	switch(getOperation())
	{
	case AttributeModifier::OPERATION_ADDITION:
		percentage = false;
		break;
	case AttributeModifier::OPERATION_MULTIPLY_BASE:
	case AttributeModifier::OPERATION_MULTIPLY_TOTAL:
		percentage = true;
		break;
	default:
		// No other operations
		assert(0);
	}

	wchar_t formatted[256];
	swprintf(formatted, 256, L"%ls%d%ls %ls", (amount > 0 ? L"+" : L"-"), static_cast<int>(displayAmount), (percentage ? L"%" : L""), app.GetString(Attribute::getName(attribute)));

	return HtmlString(formatted, color);
}