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
        void FillTransportCatalogue(const json::Document& document);
        json::Document ProcessRequests(const json::Document& document) const;

    private:
        catalogue::TransportCatalogue& database_;
        map_renderer::MapRenderer* renderer_;
    };
} // request_handler