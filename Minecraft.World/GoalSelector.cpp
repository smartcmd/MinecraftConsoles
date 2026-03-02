#include "stdafx.h"
#include "Goal.h"
#include "GoalSelector.h"


GoalSelector::InternalGoal::InternalGoal(int prio, Goal *goal, bool canDeletePointer)
{
	this->prio = prio;
	this->goal = goal;
	this->canDeletePointer = canDeletePointer;
}

GoalSelector::GoalSelector()
{
	tickCount = 0;
	newGoalRate = 3;
}

GoalSelector::~GoalSelector()
{
	for(AUTO_VAR(it, goals.begin()); it != goals.end(); ++it)
	{
		if((*it)->canDeletePointer) delete (*it)->goal;
		delete (*it);
	}
}

void GoalSelector::addGoal(int prio, Goal *goal, bool canDeletePointer /*= true*/) // 4J Added canDelete param
{
	goals.push_back(new InternalGoal(prio, goal, canDeletePointer));
}

void GoalSelector::removeGoal(Goal *toRemove)
{
	for(AUTO_VAR(it, goals.begin()); it != goals.end(); )
	{
		InternalGoal *ig = *it;
		Goal *goal = ig->goal;

		if (goal == toRemove)
		{
			AUTO_VAR(it2, find(usingGoals.begin(), usingGoals.end(), ig) );
			if (it2 != usingGoals.end())
			{
				goal->stop();
				usingGoals.erase(it2);
			}

			if(ig->canDeletePointer) delete ig->goal;
			delete ig;
			it = goals.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void GoalSelector::tick()
{
	vector<InternalGoal *> toStart;

	if(tickCount++ % newGoalRate == 0)
	{
		//for (InternalGoal ig : goals)
		for(AUTO_VAR(it, goals.begin()); it != goals.end(); ++it)
		{
			InternalGoal *ig = *it;
			//bool isUsing = usingGoals.contains(ig);
			AUTO_VAR(usingIt, find(usingGoals.begin(), usingGoals.end(), ig));

			//if (isUsing)
			if(usingIt != usingGoals.end())
			{
				if (!canUseInSystem(ig) || !canContinueToUse(ig))
				{
					ig->goal->stop();
					//usingGoals.remove(ig);
					usingGoals.erase(usingIt);
				}
				else continue;
			}

			if (!canUseInSystem(ig) || !ig->goal->canUse()) continue;

			toStart.push_back(ig);
			usingGoals.push_back(ig);
		}
	}
	else
	{
		for(AUTO_VAR(it, usingGoals.begin() ); it != usingGoals.end(); )
		{
			InternalGoal *ig = *it;
			if (!ig->goal->canContinueToUse())
			{
				ig->goal->stop();
				it = usingGoals.erase(it);
			}
			else
			{
				++it;
			}
		}
	}


	//bool debug = false;
	//if (debug && toStart.size() > 0) System.out.println("Starting: ");
	//for (InternalGoal ig : toStart)
	for(AUTO_VAR(it, toStart.begin()); it != toStart.end(); ++it)
	{
		//if (debug) System.out.println(ig.goal.toString() + ", ");
		(*it)->goal->start();
	}

	//if (debug && usingGoals.size() > 0) System.out.println("Running: ");
	//for (InternalGoal ig : usingGoals)
	for(AUTO_VAR(it, usingGoals.begin()); it != usingGoals.end(); ++it)
	{
		//if (debug) System.out.println(ig.goal.toString());
		(*it)->goal->tick();
	}
}

vector<GoalSelector::InternalGoal *> *GoalSelector::getRunningGoals()
{
	return &usingGoals;
}

bool GoalSelector::canContinueToUse(InternalGoal *ig)
{
	return ig->goal->canContinueToUse();
}

bool GoalSelector::canUseInSystem(GoalSelector::InternalGoal *goal)
{
	//for (InternalGoal ig : goals)
	for(AUTO_VAR(it, goals.begin()); it != goals.end(); ++it)
	{
		InternalGoal *ig = *it;
		if (ig == goal) continue;

		AUTO_VAR(usingIt, find(usingGoals.begin(), usingGoals.end(), ig));

		if (goal->prio >= ig->prio)
		{
			if (usingIt != usingGoals.end() && !canCoExist(goal, ig)) return false;
		}
		else if (usingIt != usingGoals.end() && !ig->goal->canInterrupt()) return false;
	}

	return true;
}

bool GoalSelector::canCoExist(GoalSelector::InternalGoal *goalA, GoalSelector::InternalGoal *goalB)
{
	return (goalA->goal->getRequiredControlFlags() & goalB->goal->getRequiredControlFlags()) == 0;
}

void GoalSelector::setNewGoalRate(int newGoalRate)
{
	this->newGoalRate = newGoalRate;
}

void GoalSelector::setLevel(Level *level)
{
	for(AUTO_VAR(it, goals.begin()); it != goals.end(); ++it)
	{
		InternalGoal *ig = *it;
		ig->goal->setLevel(level);
	}
}