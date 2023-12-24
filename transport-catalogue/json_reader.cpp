#include "json_reader.h"
#include "request_handler.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
void JsonReader::LoadJSON(std::istream& in) {

    doc_ = json::Load(in);
    
}

void JsonReader::ParseRequest(trans_cat::TransportCatalogue& catalogue)
{
    auto& node = doc_.GetRoot();
    if (node.IsMap()) {
        for (auto& [key, value] : node.AsMap())
        {
            if (key == BASE) {
                ParseStopRequest(value);
            }
            else if (key == RENDER)
            {
                mapper_ = ParseRender(value);
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
  
    for (auto& [key, value] : node.AsMap())
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
    mapper.bus_label_font_size = node.AsMap().at("bus_label_font_size").AsInt();
    mapper.stop_label_font_size = node.AsMap().at("stop_label_font_size").AsInt();
    mapper.bus_label_offset.x = node.AsMap().at("bus_label_offset").AsArray()[0].AsDouble();
    mapper.bus_label_offset.y = node.AsMap().at("bus_label_offset").AsArray()[1].AsDouble();
    mapper.stop_label_offset.x = node.AsMap().at("stop_label_offset").AsArray()[0].AsDouble();
    mapper.stop_label_offset.y = node.AsMap().at("stop_label_offset").AsArray()[1].AsDouble();
    mapper.height = node.AsMap().at("height").AsDouble();
    mapper.width = node.AsMap().at("width").AsDouble();
    mapper.line_width = node.AsMap().at("line_width").AsDouble();
    mapper.padding = node.AsMap().at("padding").AsDouble();
    mapper.stop_radius = node.AsMap().at("stop_radius").AsDouble();
   // mapper.color_palette = ParseColor(node.AsMap().at("color_palette"));
    mapper.underlayer_color = ParseColor(node.AsMap().at("underlayer_color"));
    mapper.underlayer_width = node.AsMap().at("underlayer_width").AsDouble();
    // color palette parsing
    json::Array colors_pallete = node.AsMap().at("color_palette").AsArray();
    std::vector<svg::Color> color_palette;
    for (const auto& color : colors_pallete)
    {
        color_palette.push_back(ParseColor(color));
    } 
    mapper.color_palette = color_palette;
    return mapper;
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
            auto map_request = request.AsMap();
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
            auto map_request = request.AsMap();
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
    auto map_node = node.AsMap();
    coor = { map_node["latitude"].AsDouble(), map_node["longitude"].AsDouble() };
    stop.stop_name = map_node["name"].AsString();
    stop.coor = coor;
    return stop;
}

void JsonReader::ParseStateRequest(trans_cat::TransportCatalogue& catalogue, std::ostream& out)
{
    auto& node = doc_.GetRoot();
    json::Node requests;
    if (node.IsMap()) {
        for (auto& [key, value] : node.AsMap())
        {
            if (key == STAT) {
                requests = ParseState(catalogue, value);
              
            }

        }
    }
    PrintNode(requests, out);
}

json::Node JsonReader::ParseState(trans_cat::TransportCatalogue& catalogue, const json::Node& node)
{
   
    json::Array array;
    json::Dict error_dict;
    error_dict["error_message"] = json::Node{ static_cast <std::string>("not found") };
    for (auto& request : node.AsArray())
    {
        int request_id = request.AsMap().at("id").AsInt();
        error_dict["request_id"] = request_id;
        std::string type = request.AsMap().at("type").AsString();
        if (type == "Map")
        {
            auto render_options = LoadRender();
            std::ostringstream string_out;
            MapRender render;
            render.PrintSVG(catalogue, render_options, string_out);
            json::Dict map_arr;
            map_arr["request_id"] = request_id;
            map_arr["map"] = string_out.str();
            array.push_back(map_arr);
        }
        else if (type == "Bus") {
            std::string name = request.AsMap().at("name").AsString();
            auto bus_info = catalogue.GetBusInfo(name);

            if (bus_info == BusInfo{}) {
                array.push_back(error_dict);
            }
            else {
                json::Dict dict;
                dict["curvature"] = bus_info.curv;
                dict["request_id"] = request_id;
                dict["route_length"] = (double)bus_info.lenght;
                dict["stop_count"] = (int)bus_info.stops_count;
                dict["unique_stop_count"] = (int)bus_info.uniq_stops_count;
                array.push_back(dict);
            }
        }
        else if (type == "Stop")  {
            std::string name = request.AsMap().at("name").AsString();
            auto stop_info = catalogue.GetStopInfo(name);
            if (stop_info == nullptr) {
                array.push_back(error_dict);
            }
            else {
                json::Dict dict;
                json::Array bus_list;
                for (auto& bus : *stop_info)
                {
                    bus_list.push_back(json::Node(std::string(bus)));
                }
                dict["buses"] = bus_list;
                dict["request_id"] = request_id;
                array.push_back(dict);
            }
        }


    }
    
    return json::Node(array);
}

std::vector<StopDist> JsonReader::ParceStopDist(const json::Node& node)
{
    std::vector<StopDist> ret_dist;
    std::string first_stop = node.AsMap().at("name").AsString();
    std::string second_stop;
    auto distances = node.AsMap().at("road_distances");
    for (auto& [name, dist]:distances.AsMap())
    {
        second_stop = name;
        std::size_t lenght = dist.AsInt();
        ret_dist.push_back({ first_stop, second_stop, lenght });
    }
    return ret_dist;
}

