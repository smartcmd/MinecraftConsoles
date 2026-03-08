#pragma once
class Entity;
class Chunk;

class DistanceChunkSorter
{
private:
	double ix, iy, iz;

public:
    DistanceChunkSorter(shared_ptr<Entity> player);
	bool operator()(const Chunk *a, const Chunk *b) const;
};