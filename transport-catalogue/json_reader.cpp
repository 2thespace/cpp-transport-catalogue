#include "json_reader.h"
#include "json.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
JsonReader::JsonReader()
{
    doc_ = json::Document{ json::Builder{}.StartArray().EndArray().Build()};
}
void JsonReader::LoadJSON(std::istream& in) {

    doc_ = json::Load(in);
    
}

void JsonReader::ParseRequest(trans_cat::TransportCatalogue& catalogue)
{
    auto& node = doc_.GetRoot();
    if (node.IsDict()) {
        for (auto& [key, value] : node.AsDict())
        {
            if (key == BASE) {
                ParseStopRequest(value);
            }
            else if (key == RENDER) {
                mapper_ = ParseRender(value);
            }
            else if (key == BUS_ROUTE) {
                auto bus_info = ParseRouteSettings(value);
                catalogue.SetVelocity(bus_info.bus_velocity);
                catalogue.SetWaitTime(bus_info.bus_time);
            }

        }
    }
    for (auto& stop : stops_queue_)
    {
        catalogue.AddStop(stop);
    }
    for (auto& stops_dist : stop_dist_queue_)
    {
        for (auto& stop_dist : stops_dist)
        {
            catalogue.SetDistance(stop_dist);
        }
    }
  
    for (auto& [key, value] : node.AsDict())
    {
        if (key == BASE) {
            ParseBusRequest(value, catalogue);
        }
    }
}
svg::Color ParseColor(json::Node node)
{
    if (node.IsString()) {
        return svg::Color(node.AsString());
    }
    else if (node.IsArray()) {
        uint8_t red = static_cast<uint8_t>(node.AsArray()[0].AsInt());
        uint8_t green = static_cast<uint8_t>(node.AsArray()[1].AsInt());
        uint8_t blue = static_cast<uint8_t>(node.AsArray()[2].AsInt());
        if (node.AsArray().size() == 4)
        {
            double opacity = node.AsArray()[3].AsDouble();
            return svg::Color(svg::Rgba{ red, green, blue, opacity });
        }
        return   svg::Color(svg::Rgb(red, green, blue));
    }
    else {
        return svg::Color{};
    }
}



RenderSettings JsonReader::ParseRender(const json::Node& node)
{
    RenderSettings mapper;
    mapper.bus_label_font_size = node.AsDict().at("bus_label_font_size").AsInt();
    mapper.stop_label_font_size = node.AsDict().at("stop_label_font_size").AsInt();
    mapper.bus_label_offset.x = node.AsDict().at("bus_label_offset").AsArray()[0].AsDouble();
    mapper.bus_label_offset.y = node.AsDict().at("bus_label_offset").AsArray()[1].AsDouble();
    mapper.stop_label_offset.x = node.AsDict().at("stop_label_offset").AsArray()[0].AsDouble();
    mapper.stop_label_offset.y = node.AsDict().at("stop_label_offset").AsArray()[1].AsDouble();
    mapper.height = node.AsDict().at("height").AsDouble();
    mapper.width = node.AsDict().at("width").AsDouble();
    mapper.line_width = node.AsDict().at("line_width").AsDouble();
    mapper.padding = node.AsDict().at("padding").AsDouble();
    mapper.stop_radius = node.AsDict().at("stop_radius").AsDouble();
   // mapper.color_palette = ParseColor(node.AsDict().at("color_palette"));
    mapper.underlayer_color = ParseColor(node.AsDict().at("underlayer_color"));
    mapper.underlayer_width = node.AsDict().at("underlayer_width").AsDouble();
    // color palette parsing
    json::Array colors_pallete = node.AsDict().at("color_palette").AsArray();
    std::vector<svg::Color> color_palette;
    for (const auto& color : colors_pallete)
    {
        color_palette.push_back(ParseColor(color));
    } 
    mapper.color_palette = color_palette;
    return mapper;
}

RouteSettings JsonReader::ParseRouteSettings(const json::Node& node)
{
    RouteSettings bus_info;
    bus_info.bus_velocity = node.AsDict().at("bus_velocity").AsInt();
    bus_info.bus_time = node.AsDict().at("bus_wait_time").AsInt();
    return bus_info;
}

RenderSettings JsonReader::LoadRender()
{
    
    return mapper_;
}
void JsonReader::ParseBusRequest(const json::Node& node, trans_cat::TransportCatalogue& catalogue)
{
    if (node.IsArray()) {
        for (auto& request : node.AsArray())
        {
            auto map_request = request.AsDict();
            if (map_request.at("type").AsString() == "Bus") {
                Bus bus;
                bus.name = map_request.at("name").AsString();
                bus.is_sircle = map_request.at("is_roundtrip").AsBool();
                std::vector<Stop*> stops;
                for (auto& stop_nodes : map_request.at("stops").AsArray()) {
                    stops.push_back(catalogue.FindStop(stop_nodes.AsString()));
                }
                bus.stops = stops;
                bus = ParseRoute(bus);
                catalogue.AddBus(bus);
                buses_name_.push_back(catalogue.FindBus(bus.name)->name);
            }
        }
    }
}

void JsonReader::ParseStopRequest(const json::Node& node)
{
    if (node.IsArray()) {
        for (auto& request : node.AsArray())
        {
            auto map_request = request.AsDict();
            if (map_request.at("type").AsString() == "Stop") {
                Stop stop = ParceStop(map_request);
                std::vector<StopDist> dist = ParceStopDist(map_request);
                stops_queue_.push_back(stop);
                if (!dist.empty()) {
                    stop_dist_queue_.push_back(dist);
                }
            }
        }
    }    
}

Stop JsonReader::ParceStop(const json::Node& node)
{
    Stop stop;
    geo::Coordinates coor;
    auto map_node = node.AsDict();
    coor = { map_node["latitude"].AsDouble(), map_node["longitude"].AsDouble() };
    stop.stop_name = map_node["name"].AsString();
    stop.coor = coor;
    return stop;
}

void JsonReader::ParseStateRequest(trans_cat::TransportCatalogue& catalogue, std::ostream& out)
{
    auto& node = doc_.GetRoot();
    json::Node requests;
    if (node.IsDict()) {
        for (auto& [key, value] : node.AsDict())
        {
            if (key == STAT) {
                requests = ParseState(catalogue, value);
              
            }

        }
    }
    Print(json::Document{ requests }, out);
}

json::Node JsonReader::ParseState(trans_cat::TransportCatalogue& catalogue, const json::Node& node)
{
   
    json::Array array; // не совсем понятно как array сделать реализовать через builder
    auto array_builder = json::Builder{};
    array_builder.StartArray();
    for (auto& request : node.AsArray())
    {
        json::Builder error_builder;
        error_builder.StartDict()
            .Key("error_message").Value(json::Node{ static_cast <std::string>("not found") });
        int request_id = request.AsDict().at("id").AsInt();
        error_builder.Key("request_id").Value(request_id);
        //error_dict["request_id"] = request_id; // как добавить в уже готовый словарь еще значения тоже непонятно
        std::string type = request.AsDict().at("type").AsString();
        if (type == "Map")
        {
            auto render_options = LoadRender();
            std::ostringstream string_out;
            MapRender render;
            render.PrintSVG(catalogue, render_options, string_out);
            json::Dict map_arr = json::Builder()
                .StartDict()
                    .Key("request_id").Value(request_id)
                    .Key("map").Value(string_out.str())
                .EndDict().Build().AsDict();
            //map_arr["request_id"] = request_id;
            //map_arr["map"] = string_out.str();
            array_builder.Value(json::Node(map_arr));
           // array.push_back(map_arr);
        }
        else if (type == "Bus") {
            std::string name = request.AsDict().at("name").AsString();
            auto bus_info = catalogue.GetBusInfo(name);

            if (bus_info == BusInfo{}) {
                array_builder.Value(error_builder.EndDict().Build());
                //array.push_back(error_builder.EndDict().Build());
            }
            else {
                array_builder.StartDict()
                    .Key("curvature").Value(bus_info.curv)
                    .Key("request_id").Value(request_id)
                    .Key("route_length").Value((double)bus_info.lenght)
                    .Key("stop_count").Value((int)bus_info.stops_count)
                    .Key("unique_stop_count").Value((int)bus_info.uniq_stops_count)
                .EndDict();
                //array_builder.Value(dict);
                //array.push_back(dict);
            }
        }
        else if (type == "Stop")  {
            std::string name = request.AsDict().at("name").AsString();
            auto stop_info = catalogue.GetStopInfo(name);
            if (stop_info == nullptr) {
                array_builder.Value(error_builder.EndDict().Build());
                //array.push_back(error_builder.EndDict().Build());
            }
            else {
                //json::Dict dict;
                //json::Array bus_list;
                auto bus_list_builder = json::Builder();
                bus_list_builder.StartArray();
                for (auto& bus : *stop_info)
                {
                    bus_list_builder.Value(std::string(bus));
                    
                }
                array_builder.StartDict()
                    .Key("buses").Value(bus_list_builder.EndArray().Build())
                    .Key("request_id").Value(request_id).EndDict();
                
                //array.push_back(dict);
            }
        }
        else if (type == "Route") {
            std::string stop_from = request.AsDict().at("from").AsString();
            std::string stop_to = request.AsDict().at("to").AsString();
            std::cout << stop_from << " " << stop_to << std::endl;
        }


    }
    
    return array_builder.EndArray().Build();
}

std::vector<StopDist> JsonReader::ParceStopDist(const json::Node& node)
{
    std::vector<StopDist> ret_dist;
    std::string first_stop = node.AsDict().at("name").AsString();
    std::string second_stop;
    auto distances = node.AsDict().at("road_distances");
    for (auto& [name, dist]:distances.AsDict())
    {
        second_stop = name;
        std::size_t lenght = dist.AsInt();
        ret_dist.push_back({ first_stop, second_stop, lenght });
    }
    return ret_dist;
}

