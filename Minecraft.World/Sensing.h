#pragma once

class Sensing
{
private:
	Mob *mob;
	vector<weak_ptr<Entity> > seen;
	vector<weak_ptr<Entity> > unseen;

public:
	Sensing(Mob *mob);

	void tick();
	bool canSee(shared_ptr<Entity> target);
};