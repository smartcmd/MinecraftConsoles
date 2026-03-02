#pragma once

class Path 
{
	friend class PathFinder;

private:
	NodeArray nodes;
	int index;
	int length;

public:
	Path(NodeArray nodes);
	~Path();

	void next();
	bool isDone();
	Node *last() ;
	Node *get(int i);
	int getSize();
	void setSize(int length);
	int getIndex();
	void setIndex(int index);
	Vec3 *getPos(shared_ptr<Entity> e, int index);
	NodeArray Getarray();
	Vec3 *currentPos(shared_ptr<Entity> e);
	Vec3 *currentPos();
	bool sameAs(Path *path);
	bool endsIn(Vec3 *pos);
	bool endsInXZ(Vec3 *pos);
};
