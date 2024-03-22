#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"

namespace catalogue {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name) != rhs;
		}
	};

	// � ���� ���������� ���, ��� Bus ���. ��������� ��� ���������� ���� std::string, �� ��� ���������� ���������� ��������� ���� ����� �������� ����� ����������.
    // � �� �������� ���������, ������� �����. ������ �������� �������� ���������� � TransportCatalogue.
	struct Bus {
		std::string name;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name) != rhs;
		}
	};

	struct Compartor {
		using is_transparent = std::false_type;
		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	class TransportCatalogue {
	public:
		// � ������ ����� ��������� � AddBus() ������, ��� ����� �� �����������. � ��� �����, ������ � �� ����, ��� ������ �� �����.
		void AddStop(std::string_view name, geo::Coordinates&& coordinates);
		void AddBus(std::string_view bus, const std::vector<std::string_view>& proper_stops);
		Stop* FindStop(std::string_view stop);
		std::size_t ReturnAmoutOfUniqueStopsForBus(std::string_view name) const;
		std::optional<const std::deque<Stop*>*> ReturnStopsForBus(std::string_view name) const;
		std::optional<const std::set<Bus*, Compartor>*> ReturnBusesForStop(std::string_view name) const;
		double ComputeLength(std::string_view name) const;
	private:
		std::deque<Stop> deque_stops;
		std::deque<Bus> deque_buses;

		std::unordered_map<std::string_view, std::set<Bus*, Compartor>> stops;
		std::unordered_map<std::string_view, std::unordered_set<Stop*>> buses;  
		// ������� deque � buses_wd, ��� ��� � ���������� ��� ���������� ���������, ��������, �������� ��������� ����� �������� � ���� deque � ��������� �� ���.
		// Vector ������������ ��� ��������� � ������ �� ��� �������� ��� ���������� �������.
		std::unordered_map<std::string_view, std::deque<Stop*>> buses_wd;
	};
}