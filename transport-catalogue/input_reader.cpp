#include <algorithm>
#include <iterator>
#include <utility>

#include "input_reader.h"

namespace input_reader {
    namespace detail {
        // Парсит строку вида "10.123, -30.1837" и возвращает пару координат (широта, долгота)
        geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return { nan, nan };
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return { lat, lng };
        }

        // Удаляет пробелы в начале и конце строки
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        // Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        // Парсит маршрут.
        // Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
        // Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return { std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1)) };
        }

        std::pair<std::string_view, std::size_t> ParseDestination(std::string_view destination) {
            auto m = destination.find('m');

            std::size_t length = std::stoi(std::string(destination.substr(0, m)));
            const std::string_view stop_name = destination.substr(m + 5, destination.size());

            return { stop_name, length };
        }

        std::unordered_map<std::string_view, std::size_t> ParseDestinations(std::string_view str) {
            std::unordered_map<std::string_view, std::size_t> to_return;

            auto current_comma = str.find(',');
            auto next_comma = str.find(',', current_comma + 1);

            if (next_comma == std::string_view::npos) {
                return {};
            }

            current_comma = next_comma;
            while (true) {
                next_comma = str.find(',', current_comma + 1);

                if (next_comma == std::string_view::npos) {
                    to_return.insert(ParseDestination(str.substr(current_comma + 2, str.size())));
                    break;
                }

                to_return.insert(ParseDestination(str.substr(current_comma + 2, next_comma - current_comma - 2)));
                current_comma = next_comma;
            }

            return to_return;
        }
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = detail::ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
        using namespace std::literals;
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, std::size_t>> stops_and_destinations;
        std::unordered_map<std::string_view, std::vector<std::string_view>> busses;

        for (const auto& command : commands_) {
            if (std::string_view(command.command) == "Bus"sv) {
                busses[command.id] = detail::ParseRoute(command.description);
            }
            else {
                catalogue.AddStop(std::string(command.id), detail::ParseCoordinates(command.description));
                auto to_check = detail::ParseDestinations(command.description);

                if (to_check.size()) {
                    stops_and_destinations[command.id] = std::move(to_check);
                }
            }
        }

        for (const auto& [stop, destinations] : stops_and_destinations) {
            catalogue.AddDestination(std::string(stop), destinations);
        }

        for (const auto& [bus, proper_stops] : busses) {
            catalogue.AddBus(std::string(bus), proper_stops);
        }
    }
}