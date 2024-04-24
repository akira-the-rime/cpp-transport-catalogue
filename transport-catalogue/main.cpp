#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

int main() {
    const json::Document document = json::Load(std::cin);
    catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer map_renderer;

    json_reader::JsonReader reader(catalogue, map_renderer);
    reader.HandleBaseRequests(document);
    reader.HandleRenderRequests(document);

    json::Print(reader.HandleStatRequests(document), std::cout);
}