#include "stat_reader.h"
#include <iomanip>


void ParseAndPrintStat(const trans_cat::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    // Реализуйте самостоятельно
    size_t first_space = request.find(' ');
    auto command = request.substr(0, first_space);
    request = request.substr(first_space);
    if (command == "Bus")
    {
        std::string_view bus = request.substr(request.find_first_not_of(' '), request.size());
        Bus_info bus_info = tansport_catalogue.GetBusInfo(bus);
        if (bus_info == Bus_info{}) {
            output << "Bus " << bus << ": not found\n";
            return;
        }
        output << "Bus " << bus << ": " << bus_info.stops_count << " stops on route, " \
            << bus_info.uniq_stops_count << " unique stops, " \
            << std::setprecision(6) << bus_info.lenght \
            << " route length\n";

    }
    if (command == "Stop")
    {
        std::string_view stop = request.substr(request.find_first_not_of(' '), request.size());
        auto buses = tansport_catalogue.GetStopInfo(stop);
        if (buses == nullptr) {
            output << "Stop " << stop << ": not found\n";
            return;
        }
        else if (buses->empty()) {
            output << "Stop " << stop << ": no buses\n";
            return;
        }
        else {
            output << "Stop " << stop << ": buses ";
            for (auto bus : *buses)
            {
                output << bus << " ";
            }
        }
        output << '\n';
    }

}