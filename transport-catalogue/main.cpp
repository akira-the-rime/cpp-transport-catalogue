#include <iostream>

#include "router.h"
#include "json.h"
#include "json_reader.h"
// #include "log_duration.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

int main() {
    const json::Document document = json::Load(std::cin);
    catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer map_renderer;

    json_reader::JsonReader reader(catalogue, map_renderer);
    json::Print(reader.HandleRequests(document), std::cout);
}