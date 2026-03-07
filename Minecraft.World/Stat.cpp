#include "stdafx.h"
#include "NumberFormaters.h"
#include "StatFormatter.h"
#include "Stats.h"
#include "Stat.h"

Stat::DefaultFormat *Stat::defaultFormatter = new DefaultFormat();
Stat::TimeFormatter *Stat::timeFormatter = new TimeFormatter();
Stat::DistanceFormatter *Stat::distanceFormatter = new DistanceFormatter();

// 4J Stu - Changed this to take in a printf format string instead
DecimalFormat *Stat::decimalFormat = new DecimalFormat(L"%0(3).2f");

void Stat::_init()
{
	awardLocallyOnly = false;
}

Stat::Stat(int id, const wstring& name, StatFormatter *formatter) : id(id), name(name), formatter(formatter)
{
	_init();
}

Stat::Stat(int id, const wstring& name) : id(id), name(name), formatter(defaultFormatter)
{
    _init();
}

Stat *Stat::setAwardLocallyOnly()
{
    awardLocallyOnly = true;
    return this;
}

Stat *Stat::postConstruct()
{
    //if (Stats::statsById->containsKey(id))
	//{
        //throw new RuntimeException("Duplicate stat id: \"" + Stats::statsById->get(id)->name + "\" and \"" + name + "\" at id " + id);	4J - TODO
    //}
    Stats::all->push_back(this);

	pair<int, Stat *> id1(id,this);
#ifdef __PS3__
	Stats::statsById->emplace(id1 );//	assert(0); // MGH - TODO - FIX - find out where this move function comes from
#else
	Stats::statsById->emplace( move(id1) );
#endif // __PS3__

    return this;
}

bool Stat::isAchievement()
{
    return false;
}

wstring Stat::format(int value)
{
    return ((StatFormatter *)formatter)->format(value);
}

wstring Stat::toString()
{
    return name;
}

wstring Stat::TimeFormatter::format(int value)
{
    double seconds = value / 20.0;
    double minutes = seconds / 60.0;
    double hours = minutes / 60.0;
    double days = hours / 24.0;
    double years = days / 365.0;

    if (years > 0.5)
	{
        return decimalFormat->format(years) + L" y";
    }
	else if (days > 0.5)
	{
        return decimalFormat->format(days) + L" d";
    }
	else if (hours > 0.5)
	{
        return decimalFormat->format(hours) + L" h";
    }
	else if (minutes > 0.5)
	{
        return decimalFormat->format(minutes) + L" m";
    }

    return std::to_wstring(seconds) + L" s";
}

wstring Stat::DefaultFormat::format(int value)
{
	return NumberFormat::format( value ); //numberFormat->format(value);
}

wstring Stat::DistanceFormatter::format(int cm)
{
    double meters = cm / 100.0;
    double kilometers = meters / 1000.0;

    if (kilometers > 0.5)
	{
        return decimalFormat->format(kilometers) + L" km";

    } else if (meters > 0.5)
	{
        return decimalFormat->format(meters) + L" m";
    }
    return std::to_wstring(cm) + L" cm";
}
