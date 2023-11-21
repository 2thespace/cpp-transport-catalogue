#include "transport_catalogue.h"
#include <unordered_set>


namespace trans_cat
{
	using namespace std;
	TransportCatalogue::TransportCatalogue()
	{

	}

	void TransportCatalogue::AddStop(Stop stop)
	{
		stops_.push_back(stop);
		// конструкция выглядит сложной и есть ощущение будто можно было сделать
		// как то проще, но при других вариантах у меня слетал хеш поскольку string 
		// на который указывал string_view изменялся
		stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
		stopname_to_busname_[&stops_.back()];
	}

	Stop* TransportCatalogue::FindStop(string_view stop_name) const
	{
		if (stopname_to_stop_.count(stop_name)) {
			return stopname_to_stop_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddBus(Bus bus, const string& bus_name)
	{
		buses_.push_back(bus);
		busname_to_bus_[bus_name] = &buses_.back();
		for (auto stop : bus)
		{
			stopname_to_busname_[stop].insert(bus_name);
		}
	}

	Bus* TransportCatalogue::FindBus(string_view bus_name) const
	{
		if (busname_to_bus_.count(bus_name)) {
			return busname_to_bus_.at(bus_name);
		}
		return nullptr;
	}
#include <iostream>
	Bus_info TransportCatalogue::GetBusInfo(string_view  bus_name) const
	{
		Bus* p_bus = FindBus(bus_name);
		if (p_bus == nullptr) {
			return {};
		}
		Bus bus = *p_bus;
		size_t all_stops = bus.size();
		double length = 0;
		unordered_set<string> uniq_stop;
		bool is_first = true;
		auto prev_stop = bus[0];
		for (auto stop : bus)
		{

			uniq_stop.insert(stop->stop_name);
			if (is_first) {
				is_first = false;
				continue;
			}
			length += ComputeDistance(prev_stop->coor, stop->coor);
			prev_stop = stop;

		}
		return { all_stops, uniq_stop.size(), length };
	}

	const set<string_view>* TransportCatalogue::GetStopInfo(string_view stop_name)  const
	{

		Stop* key = FindStop(stop_name);
		if (stopname_to_busname_.count(key)) {
			return &stopname_to_busname_.at(key);
		}
		return nullptr;
	}

}
