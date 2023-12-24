#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */inline const double EPSILON = 1e-6;
 bool IsZero(double value) {
     return std::abs(value) < EPSILON;
 }

 
 // Проецирует широту и долготу в координаты внутри SVG-изображения
 svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
     return {
         (coords.lng - min_lon_) * zoom_coeff_ + padding_,
         (max_lat_ - coords.lat) * zoom_coeff_ + padding_
     };
 }

 svg::Polyline DrawBus(const Bus *bus, map_render options, SphereProjector proj, svg::Color color)
 {
     svg::Polyline line;
     std::vector<geo::Coordinates> geo_coords;


     for (auto& stop : bus->stops)
     {
         geo_coords.push_back(stop->coor);
     }
     // Создаём проектор сферических координат на карту

     for (const auto geo_coord : geo_coords) {
         
         line.AddPoint(proj(geo_coord));
     }
     line.SetStrokeWidth(options.line_width);
     line.SetFillColor(svg::NoneColor);
     line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
     line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
     line.SetStrokeColor(color);
     return line;

 }
 
 svg::Text DrawBusName(std::string_view bus_name, svg::Point coor, map_render options, svg::Color color, bool is_undertext = false)
 {
     svg::Text text;
     /* общие свойства */
     text.SetPosition(coor);
     text.SetOffset((options.bus_label_offset));
     text.SetFontSize(options.bus_label_font_size);
     text.SetFontFamily("Verdana");
     text.SetFontWeight("bold");
     text.SetFillColor(color);
     text.SetData(std::string(bus_name));
     /* подложка */
     if (is_undertext) {
         text.SetFillColor(options.underlayer_color);
         text.SetStrokeColor(options.underlayer_color);
         text.SetStrokeWidth(options.underlayer_width);
         text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
         text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
     }


     return text;
 }
 
 svg::Circle DrawStopCircle(const Stop* stop, map_render options, SphereProjector proj)
 {
     svg::Circle circle;
     circle.SetCenter(proj(stop->coor));
     circle.SetRadius(options.stop_radius);
     circle.SetFillColor(svg::Color("white"));
     return circle;
 }

 svg::Text DrawStopText(std::string_view stop_name, map_render options, svg::Point coor, bool is_undertext = false)
 {
     svg::Text text;
     /* только для обычного текста */
     text.SetFillColor(svg::Color("black"));
     /* общие свойства */
     text.SetPosition(coor);
     text.SetData(std::string(stop_name));
     text.SetOffset(options.stop_label_offset);
     text.SetFontSize(options.stop_label_font_size);
     text.SetFontFamily("Verdana");
     /* только для подложки */
     if (is_undertext) {
         text.SetFillColor(options.underlayer_color);
         text.SetStrokeColor(options.underlayer_color);
         text.SetStrokeWidth(options.underlayer_width);
         text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
         text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
     }
     return text;
 }
 
 void PrintSVG(trans_cat::TransportCatalogue& catalogue, map_render options, std::ostream& out)
 {
     svg::Document doc;
     auto all_buses = catalogue.GetSortedAllBuses();
     std::vector<geo::Coordinates> geo_coords;
     for (const auto& [bus_name, bus] : all_buses)
     {
         for (const auto& stop : bus->stops)
         {
             geo_coords.push_back(stop->coor);
         }
     }
     const SphereProjector proj{
        geo_coords.begin(), geo_coords.end(), options.width, options.height, options.padding
     };
     int i = 0;
     std::vector<svg::Polyline> lines;
     std::vector<svg::Text> names;
     std::map<std::string_view , svg::Circle> stops_dict;
     
     for (const auto& [bus_name, bus] : all_buses)
     {  
         /* добавление линий маршрутов */
         auto index = i % options.color_palette.size(); // ограничиваем индекс
         auto& color = options.color_palette[index];
         /* рисуем линию остановки*/
         svg::Polyline line = DrawBus( bus, options, proj, color);
         lines.push_back(line);

         /* отображаем названием остановки или маршрута */
         std::vector < svg::Point> coors;
         coors.push_back(proj(bus->stops[0]->coor));
         if (!bus->is_sircle) {
             if ( bus->stops[0] != bus->stops[bus->stops.size() / 2 ] ) {
              
                 auto stop_coor = bus->stops[bus->stops.size() / 2 ]->coor;
                 coors.push_back(proj(stop_coor));
             }
         }
         /*  добавление названий маршрута */
         for (auto& coor : coors)
         {
             svg::Text undertext = DrawBusName(bus->name, coor, options, color, true);
             svg::Text text = DrawBusName(bus->name, coor, options, color);
             names.push_back(undertext);
             names.push_back(text);
         }
         /* добавление окружностей в лексикографический словарь по остановкам*/
         for (auto& stop : bus->stops)
         {
             if (!stops_dict.count(stop->stop_name)) {
                 stops_dict[stop->stop_name] = DrawStopCircle(stop, options, proj);
             }
         }
         bus->stops.empty() ? i : i++;
         
     }
     /* отрисовка линий маршрутов */
     for (auto& line : lines)
     {
         doc.Add(line);
     }
     /* отрисовка названий маршрутов */
     for (auto& name : names)
     {
         doc.Add(name);
     }
     /* отрисовка окружностей */
     for (auto& [stop_name, circle] : stops_dict)
     {
         doc.Add(circle);
     }
     /* отрисовка названий остановок */
     for (auto& [stop_name, circle] : stops_dict)
     {
         auto stop_undertext = DrawStopText(stop_name, options, circle.GetCenter(), true);
         auto stop_text = DrawStopText(stop_name, options, circle.GetCenter());
         doc.Add(stop_undertext);
         doc.Add(stop_text);
     }
     /* рендер svg файла */
     doc.Render(out);
 }
