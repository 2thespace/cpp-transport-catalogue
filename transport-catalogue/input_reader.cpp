#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <string>


std::vector<std::string_view> Split(std::string_view string, char delim);

/*
* Парсит строку вида Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya и возвращает 
* первое найденное расстояние и остановку
*/
StopDist ParseOneDistanse(std::string_view first_stop, std::string_view str)
{
    auto first_m = str.find('m');
    if (first_m == str.npos) {
        return {};
    }
    auto first_dist_pos = str.substr(0, first_m).find_last_of(',');
    auto dist_string = str.substr(first_dist_pos + 1, str.size());
    first_m = dist_string.find('m');
    auto find_to = dist_string.find("to");
    auto second_stop = dist_string.substr(find_to, dist_string.size());
    second_stop = second_stop.substr(second_stop.find_first_of(' ') + 1);
    size_t distance = std::stoi(std::string(dist_string.substr(0, first_m)));
    return { std::string(first_stop), std::string(second_stop), distance };
}

/*
* парсит строку вида Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
* и возвращает все найденные расстояния и остановки в виде вектора
*/
std::vector<StopDist> ParseDistanse(std::string_view first_stop, std::string_view str)
{
    
    auto str_without_delim = Split(str, ',');
    std::vector<StopDist> output_distance;
    output_distance.reserve(str_without_delim.size());
    for (auto one_string : str_without_delim)
    {
        StopDist stop_dist = ParseOneDistanse(first_stop, one_string);
        if (stop_dist != StopDist{}) {
            output_distance.push_back(stop_dist);
        }

    }
    
    return output_distance;
}
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}


void InputReader::ApplyCommands([[maybe_unused]] trans_cat::TransportCatalogue& catalogue) const {
    // Реализуйте метод самостоятельно
   
    std::deque<CommandDescription> queue_command;
    std::deque<std::vector<StopDist>> queue_stops;
    std::string name;
    for (auto it = commands_.begin(); it != commands_.end(); it++)
    {
        if (it->command == "Stop") {
            queue_command.push_front(*it);
        }
        else if(it->command == "Bus")  {
            queue_command.push_back(*it);
        }
    }
    for (auto& command : queue_command)
    {
        if (command.command == "Stop") {
            geo::Coordinates coor = ParseCoordinates(command.description);
            name = command.id;
            auto stops_distance = ParseDistanse(name, command.description);
            if (!stops_distance.empty()){
                queue_stops.push_back(stops_distance);
            }
            catalogue.AddStop({ name, coor });
        }
        else if (command.command == "Bus") {
            name = command.id;
            auto stops = ParseRoute(command.description);
            Bus bus;
            for (auto stop : stops)
            {
                bus.stops.push_back(catalogue.FindStop(stop));
            }
            bus.name = name;
            catalogue.AddBus(bus);
        }
    }
    
    for (auto& distances : queue_stops)
    {
        for (auto& distance_stop:distances)
        {
            catalogue.SetDistance(distance_stop);
        }
    }

}
void InputReader::ParseRequest(std::istream& in)
{
    int base_request_count;
    in >> base_request_count >> std::ws;
    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(in, line);
        this->ParseLine(line);
    }
}

