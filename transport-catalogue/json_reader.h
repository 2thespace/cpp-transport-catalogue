#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "geo.h"
#include "json_builder.h"
#include <sstream>
#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
const std::string_view BASE = "base_requests";
const std::string_view STAT = "stat_requests";
const std::string_view RENDER = "render_settings";
const std::string_view BUS_ROUTE = "routing_settings";
const std::string_view ROUTE = "route";

svg::Color ParseColor(json::Node node);

struct CommandDescription {
    // Определяет, задана ли команда (поле command непустое)
    explicit operator bool() const {
        return !command.empty();
    }

    bool operator!() const {
        return !operator bool();
    }

    std::string command;      // Название команды
    std::string id;           // id маршрута или остановки
    std::string description;  // Параметры команды
};

class JsonReader {

public:
    JsonReader();
    void LoadJSON(std::istream& in);
    void ParseRequest(trans_cat::TransportCatalogue& catalogue);
    void ParseStateRequest(trans_cat::TransportCatalogue& catalogue, std::ostream& out);
    RenderSettings LoadRender();
private:
    json::Document doc_;
    std::deque<Stop> stops_queue_;
    std::deque<std::string_view> buses_name_;
    std::deque<std::vector<StopDist>> stop_dist_queue_;
    RenderSettings mapper_;
    
    void ParseStopRequest(const json::Node& node);
    void ParseBusRequest(const json::Node& node, trans_cat::TransportCatalogue& catalogue);
    Stop ParceStop(const json::Node& node);
    std::vector<StopDist> ParceStopDist(const json::Node& node);
    json::Node ParseState(trans_cat::TransportCatalogue& catalogue, const json::Node& node);
    RenderSettings ParseRender(const json::Node& node);
    RouteSettings ParseRouteSettings(const json::Node& node);
    const json::Node& ParseRouteGraph(trans_cat::TransportRouter& router, const std::string_view stop_from, const std::string_view stop_to, int request_id);
    
};

