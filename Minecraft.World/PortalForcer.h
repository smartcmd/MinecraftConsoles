#pragma once

class Random;

class PortalForcer
{
private:
	Random *random;

public:
	// 4J Stu Added - Java has no ctor, but we need to initialise random
	PortalForcer();

	void force(Level *level, shared_ptr<Entity> e);

public:
	bool findPortal(Level *level, shared_ptr<Entity> e);

public:
	bool createPortal(Level *level, shared_ptr<Entity> e);
};