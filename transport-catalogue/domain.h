#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include "geo.h"
const double PREC = 1e-5;
const uint16_t PRIME_NUMB = 13;
const uint16_t MOD = 42;

struct StopDist
{
	std::string first_stop;
	std::string second_stop;
	std::size_t dist;
	bool operator ==(const StopDist& other_stop);
	bool operator !=(const StopDist& other_stop);
};

struct Stop
{
	std::string stop_name;
	geo::Coordinates coor;
	bool operator ==(const Stop& other_stop);
	bool operator !=(const Stop& other_stop);
};

struct Bus
{
	std::string name;
	std::vector<Stop*> stops;
	int velocity;
	int wait_time;
	bool is_sircle;
};




struct BusInfo
{
	size_t stops_count = 0;
	size_t uniq_stops_count = 0;
	size_t lenght = 0;
	double curv = 0;
	bool operator ==(const BusInfo& other_bus);
	bool operator !=(const BusInfo& other_bus);
};
using StopPtrPair = std::pair<Stop*, Stop*>;

struct StopPtrPairHasher {
	size_t operator()(const StopPtrPair& ptr_pair) const;
};