#pragma once

#include "TutorialEnum.h"
#include "TutorialConstraint.h"

class AABB;
class Tutorial;
class GameType;

class ChangeStateConstraint : public TutorialConstraint
{
private:
	AABB *movementArea;
	bool contains; // If true we must stay in this area, if false must stay out of this area
	bool m_changeGameMode;
	GameType *m_targetGameMode;
	GameType *m_changedFromGameMode;

	eTutorial_State m_targetState;
	eTutorial_State *m_sourceStates;
	DWORD m_sourceStatesCount;

	bool m_bHasChanged;
	eTutorial_State m_changedFromState;

	bool m_bComplete;

	Tutorial *m_tutorial;

public:
	virtual ConstraintType getType() { return e_ConstraintChangeState; }

	ChangeStateConstraint( Tutorial *tutorial, eTutorial_State targetState, eTutorial_State sourceStates[], DWORD sourceStatesCount, double x0, double y0, double z0, double x1, double y1, double z1, bool contains = true, bool changeGameMode = false, GameType *targetGameMode = nullptr );
	~ChangeStateConstraint();

	virtual void tick(int iPad);
};