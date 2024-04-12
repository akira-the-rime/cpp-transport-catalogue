#include <iomanip>
#include <iostream>
#include <numeric>

#include "stat_reader.h"

namespace stat_reader {
    namespace detail {
        using namespace std::literals;

        void PrintBusInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view name,
            std::ostream& output) {
            const catalogue::BusInfo to_output = transport_catalogue.GetBusInfo(name);

            if (!to_output.is_found) {
                output << "Bus "s << to_output.name << ": not found"s << std::endl;
                return;
            }

            output << "Bus "s << to_output.name << ": "s;
            output << to_output.stops_on_route << " stops on route, "s;
            output << to_output.unique_stops << " unique stops, "s;
            output << to_output.actual_distance << " route length, "s;
            output << std::setprecision(6) << to_output.curvature << " curvature"s << std::endl;
        }

        void PrintStopInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view name,
            std::ostream& output) {
            const catalogue::StopInfo to_output = transport_catalogue.GetStopInfo(name);

            if (!to_output.is_found) {
                output << "Stop "s << to_output.name << ": not found"s << std::endl;
                return;
            }
            else if (!to_output.bus_names.size()) {
                output << "Stop "s << to_output.name << ": no buses"s << std::endl;
                return;
            }

            bool is_first = true;
            output << "Stop "s << to_output.name << ": buses "s;
            for (const auto& bus : to_output.bus_names) {
                if (is_first) {
                    output << bus;
                    is_first = false;
                    continue;
                }
                output << " "s << bus;
            }
            output << std::endl;
        }
    }

    void ParseAndPrintStat(const catalogue::TransportCatalogue& transport_catalogue,
        std::string_view request,
        std::ostream& output) {
        using namespace std::literals;

        const std::size_t pos = request.find_first_of(' ');
        std::string_view query, name;

        if (pos + 1 != request.size()) {
            query = request.substr(0, pos);
            name = request.substr(pos + 1, request.size() - pos);
        }

        if (query == "Bus"sv) {
            detail::PrintBusInfo(transport_catalogue, name, output);
        }
        else {
            detail::PrintStopInfo(transport_catalogue, name, output);
        }
    }
}