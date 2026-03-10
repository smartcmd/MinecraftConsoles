#include "stdafx.h"
#include "FlippedIcon.h"
#include "SharedConstants.h"

FlippedIcon::FlippedIcon(Icon *base, bool horizontal, bool vertical) : base(base), horizontal(horizontal), vertical(vertical)
{
}

int FlippedIcon::getX() const
{
	return base->getX();
}

int FlippedIcon::getY() const
{
	return base->getY();
}

int FlippedIcon::getWidth() const
{
	return base->getWidth();
}

int FlippedIcon::getHeight() const
{
	return base->getHeight();
}

float FlippedIcon::getU0(bool adjust/*=false*/) const
{
	if (horizontal) return base->getU1(adjust);
	return base->getU0(adjust);
}

float FlippedIcon::getU1(bool adjust/*=false*/) const
{
	if (horizontal) return base->getU0(adjust);
	return base->getU1(adjust);
}

float FlippedIcon::getU(double offset, bool adjust/*=false*/) const
{
	float diff = getU1(adjust) - getU0(adjust);
	return getU0(adjust) + (diff * (static_cast<float>(offset) / SharedConstants::WORLD_RESOLUTION));
}

float FlippedIcon::getV0(bool adjust/*=false*/) const
{
	if (vertical) return base->getV0(adjust);
	return base->getV0(adjust);
}

float FlippedIcon::getV1(bool adjust/*=false*/) const
{
	if (vertical) return base->getV0(adjust);
	return base->getV1(adjust);
}

float FlippedIcon::getV(double offset, bool adjust/*=false*/) const
{
	float diff = getV1(adjust) - getV0(adjust);
	return getV0(adjust) + (diff * (static_cast<float>(offset) / SharedConstants::WORLD_RESOLUTION));
}

wstring FlippedIcon::getName() const
{
	return base->getName();
}

int FlippedIcon::getSourceWidth() const
{
	return base->getSourceWidth();
}

int FlippedIcon::getSourceHeight() const
{
	return base->getSourceHeight();
}

int FlippedIcon::getFlags() const
{
	return base->getFlags();
}

void FlippedIcon::setFlags(int flags)
{
	base->setFlags(flags);
}
