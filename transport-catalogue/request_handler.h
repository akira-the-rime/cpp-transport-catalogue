#pragma once

#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
 
namespace request_handler {
// ------------------------------- [Request Handler] Definition -----------------------
//                                                                                    +
//                                                                                    + -----------------
// ------------------------------------------------------------------------------------ Request Handler +

    class RequestHandler final {
    public:
        RequestHandler(catalogue::TransportCatalogue& database);
        RequestHandler(catalogue::TransportCatalogue& database, map_renderer::MapRenderer& renderer);
        json::Document ProcessRequests(const json::Document& document) const;

    private:
        json::Node ProcessMapRequest(const json::Dict& to_parse, const json::Document& document) const;
        json::Node ProcessStopRequest(const json::Dict& to_parse) const;
        json::Node ProcessBusRequest(const json::Dict& to_parse) const;

        catalogue::TransportCatalogue& database_;
        map_renderer::MapRenderer* renderer_;
    };
} // request_handler