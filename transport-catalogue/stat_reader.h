#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace stat_reader {
    namespace detail {
        void OutputBusInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view query, std::string_view name, std::ostream& output);

        void OutputStopInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view query, std::string_view name, std::ostream& output);
    }

    void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue,
        std::string_view request,
        std::ostream& output);
}