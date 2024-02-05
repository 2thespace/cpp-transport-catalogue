#pragma once

#include "domain.h"
#include <deque>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>



namespace trans_cat
{
	class TransportCatalogue {

		// Реализуйте класс самостоятельно
	public:
		using StopGraph = graph::DirectedWeightedGraph<Stop*>;
		TransportCatalogue();
		void AddStop(const Stop& stop);
		Stop* FindStop(std::string_view stop_name)  const;
		void AddBus(const Bus& bus);
		Bus* FindBus(std::string_view bus_name) const;
		BusInfo GetBusInfo(std::string_view bus_name)  const;
		const std::set<std::string_view>* GetStopInfo(std::string_view stop_name)  const;
		void SetDistance(const StopDist& stop_dist);
		size_t GetDistance(std::string_view first_stop, std::string_view second_stop) const;
		const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;
		int GetVelocity(void);
		int GetWaitTime(void);
		void SetVelocity(int velocity);
		void SetWaitTime(int time);
	private:
		std::deque<Stop> stops_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::deque<Bus> buses_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map <Stop*, std::set<std::string_view> > stopname_to_busname_;
		std::unordered_map<StopPtrPair, size_t, StopPtrPairHasher> distance_table_;
		int bus_velocity_ = 0;
		int bus_wait_time_ = 0;

	};
}