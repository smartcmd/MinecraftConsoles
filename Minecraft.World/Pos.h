#pragma once
// Pos implements Comparable<Pos>
// We don't REALLY need it as it's main use it to make things easy 
// to handle in the java array/list classes, but adding to help
// maintain as much original code as possible

//class Pos //implements Comparable<Pos>
class Pos
{
public:
	int x;
	int y;
	int z;

	Pos();

	Pos(int x, int y, int z);

	Pos(Pos *position);

	//@Override
	//public boolean equals(Object other)
	bool equals(void *other);

	int hashCode();
	int compareTo(Pos *pos);
	Pos *offset(int x, int y, int z);
	void set(int x, int y, int z);
	void set(Pos *pos);

	Pos *above();
	Pos *above(int steps);
	Pos *below();
	Pos *below(int steps);
	Pos *north();
	Pos *north(int steps);
	Pos *south();
	Pos *south(int steps);
	Pos *west();
	Pos *west(int steps);
	Pos *east();
	Pos *east(int steps);

	void move(int x, int y, int z);
	void move(Pos pos);
	void moveX(int steps);
	void moveY(int steps);
	void moveZ(int steps);
	void moveUp(int steps);
	void moveUp();
	void moveDown(int steps);
	void moveDown();
	void moveEast(int steps);
	void moveEast();
	void moveWest(int steps);
	void moveWest();
	void moveNorth(int steps);
	void moveNorth();
	void moveSouth(int steps);
	void moveSouth();

	double dist(int x, int y, int z);
	double dist(Pos *pos);
	float distSqr(int x, int y, int z);
	float distSqr(Pos *pos);
};