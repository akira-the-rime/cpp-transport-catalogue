#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <set>
#include <string>

#include "stat_reader.h"

namespace stat_reader {
    namespace detail {
        using namespace std::literals;

        void OutputBusInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view query, std::string_view name, std::ostream& output) {

            std::optional<const std::deque<catalogue::Stop*>*> to_deem = transport_catalogue.ReturnStopsForBus(name);

            if (!to_deem.has_value()) {
                output << query << " "s << name << ": not found" << std::endl;
                return;
            }

            output << query << " "s << name << ": ";
            output << to_deem.value()->size() << " stops on route, "s;
            output << transport_catalogue.ReturnAmoutOfUniqueStopsForBus(name) << " unique stops, "s;
            output << std::setprecision(6) << transport_catalogue.ComputeLength(name) << " route length"s << std::endl;
        }

        void OutputStopInfo(const catalogue::TransportCatalogue& transport_catalogue,
            std::string_view query, std::string_view name, std::ostream& output) {

            const std::optional<const std::set<catalogue::Bus*, catalogue::Compartor>*> to_deem = transport_catalogue.ReturnBusesForStop(name);

            if (!to_deem.has_value()) {
                output << query << " "s << name << ": not found" << std::endl;
                return;
            }
            else if (!to_deem.value()->size()) {
                output << query << " "s << name << ": no buses" << std::endl;
                return;
            }

            bool is_first = true;
            output << query << " "s << name << ": buses "s;
            for (const auto& bus : *to_deem.value()) {
                if (is_first) {
                    output << bus->name;
                    is_first = false;
                    continue;
                }
                output << " "s << bus->name;
            }
            output << std::endl;
        }
    }

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
            detail::OutputBusInfo(transport_catalogue, query, name, output);
        }
        else {
            detail::OutputStopInfo(transport_catalogue, query, name, output);
        }
    }
}