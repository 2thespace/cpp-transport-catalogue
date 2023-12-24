#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */
 /**
  * Парсит маршрут.
  * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
  * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
  */
Bus ParseRoute(Bus& bus) {
    if (bus.is_sircle) {
        return bus;
    }

   
    std::vector<Stop*> results= bus.stops;
    results.insert(results.end(), bus.stops.rbegin() + 1, bus.stops.rend());

    bus.stops = std::move(results);
    return bus;
}