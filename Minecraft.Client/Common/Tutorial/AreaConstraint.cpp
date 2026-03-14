#include "stdafx.h"

#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "AreaConstraint.h"
#include "../../../Minecraft.World/AABB.h"

AreaConstraint::AreaConstraint( int descriptionId, double x0, double y0, double z0, double x1, double y1, double z1, bool contains /*= true*/, bool restrictsMovement /*=true*/ )
	: TutorialConstraint( descriptionId )
{
	messageArea = AABB::newPermanent(x0+2, y0+2, z0+2, x1-2, y1-2, z1-2);
	movementArea = AABB::newPermanent(x0, y0, z0, x1, y1, z1);

	this->contains = contains;
	m_restrictsMovement = restrictsMovement;
}

AreaConstraint::~AreaConstraint()
{
	delete messageArea;
	delete movementArea;
}

bool AreaConstraint::isConstraintSatisfied(int iPad)
{
	Minecraft *minecraft = Minecraft::GetInstance();
	return messageArea->contains( minecraft->localplayers[iPad]->getPos(1) ) == contains;
}

bool AreaConstraint::isConstraintRestrictive(int iPad)
{
	return m_restrictsMovement;
}


bool AreaConstraint::canMoveToPosition(double xo, double yo, double zo, double xt, double yt, double zt)
{
	if(!m_restrictsMovement) return true;

	Vec3 *targetPos = Vec3::newTemp(xt, yt, zt);
	Minecraft *minecraft = Minecraft::GetInstance();
	
	if(movementArea->contains( targetPos ) == contains)
	{
		return true;
	}
	Vec3 *origPos = Vec3::newTemp(xo, yo, zo);

	double currDist = origPos->distanceTo(movementArea);
	double targetDist = targetPos->distanceTo(movementArea);
	return targetDist < currDist;
}