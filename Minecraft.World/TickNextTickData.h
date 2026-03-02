#pragma once

#include <cstdint>

// 4J Stu - In Java TickNextTickData implements Comparable<TickNextTickData>
// We don't need to do that as it is only as helper for the java sdk sorting operations

class TickNextTickData
{
private:
	static int64_t C;

public:
	int x, y, z, tileId;
	int64_t m_delay;

private:
	int64_t c;

public:
	TickNextTickData(int x, int y, int z, int tileId);

	bool equals(const void *o) const;
	int hashCode() const;
	TickNextTickData *delay(int64_t l);
	int compareTo(const TickNextTickData *tnd) const;

	static bool compare_fnct(const TickNextTickData &x, const TickNextTickData &y);
	static int hash_fnct(const TickNextTickData &k);
	static bool eq_test(const TickNextTickData &x, const TickNextTickData &y);
};

struct TickNextTickDataKeyHash
{
	int operator() (const TickNextTickData &k) const 
	{ return TickNextTickData::hash_fnct (k); }
};

struct TickNextTickDataKeyEq
{
	bool operator() (const TickNextTickData &x, const TickNextTickData &y) const 
	{ return TickNextTickData::eq_test (x, y); }
};

struct TickNextTickDataKeyCompare
{
	bool operator() (const TickNextTickData &x, const TickNextTickData &y) const 
	{ return TickNextTickData::compare_fnct (x, y); }

};