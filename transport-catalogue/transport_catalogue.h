#pragma once

#include "geo.h"
#include <deque>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

const double PREC = 1e-5;
const uint16_t PRIME_NUMB = 13;
const uint16_t MOD = 42;

struct StopDist
{
	std::string first_stop;
	std::string second_stop;
	std::size_t dist;
	bool operator ==(const StopDist& other_stop)
	{
		return ((this->first_stop == other_stop.first_stop) &&
				(this->second_stop == other_stop.second_stop) &&
				(this->dist == other_stop.dist) );
	}
	bool operator !=(const StopDist& other_stop)
	{
		return !(*this == other_stop);
	}
};

struct Stop
{
	std::string stop_name;
	geo::Coordinates coor;
	bool operator ==(const Stop& other_stop)
	{
		return ((this->stop_name == other_stop.stop_name) &&
				(this->coor == other_stop.coor) );
	}
	bool operator !=(const Stop& other_stop)
	{
		return !(*this == other_stop);
	}
};
struct Bus
{
	std::string name;
	std::vector<Stop*> stops;
};



struct BusInfo
{
	size_t stops_count = 0;
	size_t uniq_stops_count = 0;
	size_t lenght = 0;
	double curv = 0;
	bool operator ==(const BusInfo& other_bus)
	{
		return ((this->stops_count == other_bus.stops_count) &&
				(this->uniq_stops_count == other_bus.uniq_stops_count) &&
				(std::abs(this->curv - other_bus.curv) < PREC) &&
				(this->lenght == other_bus.lenght));
	}
	bool operator !=(const BusInfo& other_bus)
	{
		return !(*this == other_bus);
	}
};
using StopPtrPair = std::pair<Stop*, Stop*>;
struct StopPtrPairHasher {
	size_t operator()(const StopPtrPair& ptr_pair) const {
		size_t first_hash = std::hash<const void*>{}(ptr_pair.first);
		size_t second_hash = std::hash<const void*>{}(ptr_pair.second);
		second_hash = (second_hash * PRIME_NUMB);
		return (first_hash + second_hash);
	}
};

namespace trans_cat
{
	class TransportCatalogue{

		// Реализуйте класс самостоятельно
	public:


		
		TransportCatalogue();
		void AddStop(const Stop& stop);
		Stop* FindStop(std::string_view stop_name)  const;
		void AddBus(const Bus& bus);
		Bus* FindBus(std::string_view bus_name) const;
		BusInfo GetBusInfo(std::string_view bus_name)  const;
		const std::set<std::string_view>* GetStopInfo(std::string_view stop_name)  const;
		void SetDistance(const StopDist& stop_dist);
		size_t GetDistance(std::string_view first_stop, std::string_view second_stop) const;
	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map <Stop*, std::set<std::string_view> > stopname_to_busname_;
		std::unordered_map<StopPtrPair, size_t, StopPtrPairHasher> distance_table_;
	};
}