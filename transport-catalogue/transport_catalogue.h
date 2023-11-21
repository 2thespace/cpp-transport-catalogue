#pragma once

#include "geo.h"
#include <deque>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
const double PREC = 1e-5;

struct Stop
{
	std::string stop_name;
	geo::Coordinates coor;
	bool operator ==(const Stop& other_stop)
	{
		return ((this->stop_name == other_stop.stop_name) &&
			(this->coor == other_stop.coor));
	}
	bool operator !=(const Stop& other_stop)
	{
		return !(*this == other_stop);
	}
};
using Bus = std::vector<Stop*>;

struct Bus_info
{
	size_t stops_count = 0;
	size_t uniq_stops_count = 0;
	double lenght = 0;
	bool operator ==(const Bus_info& other_bus)
	{
		return ((this->stops_count == other_bus.stops_count) &&
			(this->uniq_stops_count == other_bus.uniq_stops_count) &&
			(std::abs(this->lenght - other_bus.lenght) < PREC));
	}
	bool operator !=(const Bus_info& other_bus)
	{
		return !(*this == other_bus);
	}
};


namespace trans_cat
{
	class TransportCatalogue{

		// Реализуйте класс самостоятельно
	public:
		TransportCatalogue();
		void AddStop(Stop stop);
		Stop* FindStop(std::string_view stop_name)  const;
		void AddBus(Bus bus, const std::string& bus_name);
		Bus* FindBus(std::string_view bus_name) const;
		Bus_info GetBusInfo(std::string_view bus_name)  const;
		const std::set<std::string_view>* GetStopInfo(std::string_view stop_name)  const;
	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map <Stop*, std::set<std::string_view> > stopname_to_busname_;

	};
}