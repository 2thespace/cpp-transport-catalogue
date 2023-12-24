#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (RenderSettingser) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
bool StopDist::operator ==(const StopDist& other_stop)
{
	return ((this->first_stop == other_stop.first_stop) &&
		(this->second_stop == other_stop.second_stop) &&
		(this->dist == other_stop.dist));
}

bool StopDist::operator !=(const StopDist& other_stop)
{
	return !(*this == other_stop);
}

bool Stop::operator ==(const Stop& other_stop)
{
	return ((this->stop_name == other_stop.stop_name) &&
		(this->coor == other_stop.coor));
}

bool Stop::operator !=(const Stop& other_stop)
{
	return !(*this == other_stop);
}

bool BusInfo::operator ==(const BusInfo& other_bus)
{
	return ((this->stops_count == other_bus.stops_count) &&
		(this->uniq_stops_count == other_bus.uniq_stops_count) &&
		(std::abs(this->curv - other_bus.curv) < PREC) &&
		(this->lenght == other_bus.lenght));
}

bool BusInfo::operator !=(const BusInfo& other_bus)
{
	return !(*this == other_bus);
}

size_t StopPtrPairHasher::operator()(const StopPtrPair& ptr_pair) const {
	size_t first_hash = std::hash<const void*>{}(ptr_pair.first);
	size_t second_hash = std::hash<const void*>{}(ptr_pair.second);
	second_hash = (second_hash * PRIME_NUMB);
	return (first_hash + second_hash);
}
