#pragma once
using namespace std;

class Goal;

class GoalSelector
{
private:
	class InternalGoal
	{
	public:
		// 4J Added canDelete param
		InternalGoal(int prio, Goal *goal, bool canDeletePointer);

		Goal *goal;
		int prio;
		bool canDeletePointer;
	};

private:
	vector<InternalGoal *> goals;
	vector<InternalGoal *> usingGoals;
	int tickCount;
	int newGoalRate;

public:
	GoalSelector();
	~GoalSelector();

	// 4J Added canDelete param
	void addGoal(int prio, Goal *goal, bool canDeletePointer = true);
	void removeGoal(Goal *toRemove);
	void tick();
	vector<InternalGoal *> *getRunningGoals();

private:
	bool canContinueToUse(InternalGoal *ig);
	bool canUseInSystem(InternalGoal *goal);
	bool canCoExist(InternalGoal *goalA, InternalGoal *goalB);

public:
	void setNewGoalRate(int newGoalRate);

	// 4J Added override to update ai elements when loading entity from schematics
	void setLevel(Level *level);
};