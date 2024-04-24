#include <iostream>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

int main() {
    const json::Document document = json::Load(std::cin);
    catalogue::TransportCatalogue catalogue;

    json_reader::JsonReader reader(catalogue);
    reader.FillTransportCatalogue(document);

    map_renderer::MapRenderer map_renderer;
    // map_renderer.HandleRenderRequests(document);
    // map_renderer.CreateMap();

    request_handler::RequestHandler handler(catalogue, map_renderer);
    json::Document to_print = handler.ProcessRequests(document);
    json::Print(to_print, std::cout);
}