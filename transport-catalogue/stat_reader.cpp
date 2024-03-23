#include <cstddef>
#include <numeric>
#include <set>
#include <string>

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
            transport_catalogue.OutputBusInfo(query, name, output);
        }
        else {
            transport_catalogue.OutputStopInfo(query, name, output);
        }
    }
}