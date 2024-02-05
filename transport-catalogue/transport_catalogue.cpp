#include "transport_catalogue.h"
#include <unordered_set>
#include "graph.h"

namespace trans_cat
{
	using namespace std;
	using StopGraph = graph::DirectedWeightedGraph<Stop*>;

	TransportCatalogue::TransportCatalogue()
	{

	}

	void TransportCatalogue::AddStop(const Stop& stop)
	{
		stops_.push_back(stop);
		auto new_stop = &stops_.back();
		auto& stop_name = new_stop->stop_name;
		stopname_to_stop_[stop_name] = new_stop;
		stopname_to_busname_[new_stop];
	}

	Stop* TransportCatalogue::FindStop(string_view stop_name) const
	{
		if (stopname_to_stop_.count(stop_name)) {
			return stopname_to_stop_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddBus(const Bus& bus)
	{
		buses_.push_back(bus);
		auto new_bus = &buses_.back();
		auto& bus_name = new_bus->name;
		busname_to_bus_[bus_name] = new_bus;
		for (auto stop : new_bus->stops)
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
	BusInfo TransportCatalogue::GetBusInfo(string_view  bus_name) const
	{
		Bus* p_bus = FindBus(bus_name);
		if (p_bus == nullptr) {
			return {};
		}
		Bus bus = *p_bus;
		size_t all_stops = bus.stops.size();
		double geo_length = 0;
		std::size_t road_length = 0;
		unordered_set<string> uniq_stop;
		bool is_first = true;
		auto prev_stop = bus.stops[0];
		for (auto stop : bus.stops)
		{

			uniq_stop.insert(stop->stop_name);
			if (is_first) {
				is_first = false;
				continue;
			}

			geo_length += ComputeDistance(prev_stop->coor, stop->coor);
			road_length += GetDistance(prev_stop->stop_name, stop->stop_name);
			prev_stop = stop;

		}

		return { all_stops, uniq_stop.size(), road_length, road_length / geo_length };
	}

	const set<string_view>* TransportCatalogue::GetStopInfo(string_view stop_name)  const
	{

		Stop* key = FindStop(stop_name);
		if (stopname_to_busname_.count(key)) {
			return &stopname_to_busname_.at(key);
		}
		return nullptr;
	}
	void TransportCatalogue::SetDistance(const StopDist& stop_dist)
	{
		auto first_stop = stop_dist.first_stop;
		auto second_stop = stop_dist.second_stop;
		size_t dist = stop_dist.dist;
		auto first_ptr_stop = FindStop(first_stop);
		auto second_ptr_stop = FindStop(second_stop);
		StopPtrPair stop_pair = { first_ptr_stop, second_ptr_stop };
		distance_table_[stop_pair] = dist;

	}

	size_t TransportCatalogue::GetDistance(std::string_view first_stop, std::string_view second_stop) const
	{
		auto first_ptr_stop = FindStop(first_stop);
		auto second_ptr_stop = FindStop(second_stop);
		StopPtrPair stop_pair = { first_ptr_stop, second_ptr_stop };
		if (distance_table_.count(stop_pair)) {
			return distance_table_.at(stop_pair);
		}
		else if (StopPtrPair new_stop_pair = { second_ptr_stop, first_ptr_stop }; distance_table_.count(new_stop_pair)) {
			return distance_table_.at(new_stop_pair);
		}
		else {
			return 0;
		}
	}

	const std::map<std::string_view, const Bus*> TransportCatalogue::GetSortedAllBuses() const {
		std::map<std::string_view, const Bus*> result;
		for (const auto& bus : busname_to_bus_) {
			result.emplace(bus);
		}
		return result;
	}

	int TransportCatalogue::GetVelocity(void)
	{
		return bus_velocity_;
	}

	int TransportCatalogue::GetWaitTime(void)
	{
		return bus_wait_time_;
	}

	void TransportCatalogue::SetVelocity(int velocity) 
	{
		bus_velocity_ = velocity;
	}

	void TransportCatalogue::SetWaitTime(int time)
	{
		bus_wait_time_ = time;
	}

} // namespace transport_cat


