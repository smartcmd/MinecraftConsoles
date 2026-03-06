#pragma once

class Random;

class PortalForcer
{
public:
	class PortalPosition : public Pos
	{
	public:
		int64_t lastUsed;

		PortalPosition(int x, int y, int z, int64_t time);
	};

private:
	ServerLevel *level;
	Random *random;
	unordered_map<int64_t, PortalPosition *> cachedPortals;
	vector<int64_t> cachedPortalKeys;

public:
	PortalForcer(ServerLevel *level);
	~PortalForcer();

	void force(shared_ptr<Entity> e, double xOriginal, double yOriginal, double zOriginal, float yRotOriginal);
	bool findPortal(shared_ptr<Entity> e, double xOriginal, double yOriginal, double zOriginal, float yRotOriginal);
	bool createPortal(shared_ptr<Entity> e);
	void tick(int64_t time);
};