#include <iostream>
#include <numeric>

#include "stat_reader.h"

namespace stat_reader {
    void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue,
        std::string_view request,
        std::ostream& output) {
        using namespace std::literals;

        std::size_t pos = request.find_first_of(' ');
        std::string_view query, name;

        if (pos + 1 != request.size()) {
            query = request.substr(0, pos);
            name = request.substr(pos + 1, request.size() - pos);
        }

        if (query == "Bus"sv) {
            std::stringstream to_output = transport_catalogue.GetBusInfo(query, name);
            std::string read;
            std::getline(to_output, read);
            output << read << std::endl;
        }
        else {
            std::stringstream to_output = transport_catalogue.GetStopInfo(query, name);
            std::string read;
            std::getline(to_output, read);
            output << read << std::endl;
        }
    }
}