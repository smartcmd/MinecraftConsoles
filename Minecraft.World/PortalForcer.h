#pragma once

class Random;

class PortalForcer
{
public:
	class PortalPosition : public Pos
	{
	public:
		__int64 lastUsed;

		PortalPosition(int x, int y, int z, __int64 time);
	};

private:
	ServerLevel *level;
	Random *random;
	unordered_map<__int64, PortalPosition *> cachedPortals;
	vector<__int64> cachedPortalKeys;

public:
	PortalForcer(ServerLevel *level);
	~PortalForcer();

	void force(shared_ptr<Entity> e, double xOriginal, double yOriginal, double zOriginal, float yRotOriginal);
	bool findPortal(shared_ptr<Entity> e, double xOriginal, double yOriginal, double zOriginal, float yRotOriginal);
	bool createPortal(shared_ptr<Entity> e);
	void tick(__int64 time);
};