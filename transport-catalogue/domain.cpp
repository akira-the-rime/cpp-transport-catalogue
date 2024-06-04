#include "domain.h"

namespace domain {
// ------------ [Domain Structs] Realization ------------
//                                                      +
//                                                     + --------------------
// ----------------------------------------------------- Stop & Bus structs +

	bool Stop::operator==(std::string_view rhs) const {
		return std::string_view(name) == rhs;
	}

	bool Stop::operator!=(std::string_view rhs) const {
		return std::string_view(name) != rhs;
	}

	bool Bus::operator==(std::string_view rhs) const {
		return std::string_view(name) == rhs;
	}

	bool Bus::operator!=(std::string_view rhs) const {
		return std::string_view(name) != rhs;
	}

// 
// 
//                                                      + -------------
// ------------------------------------------------------ Auxiliaries +

	bool Compartor::operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->name < rhs->name;
	}

	std::size_t Hasher::operator()(const std::pair<std::string_view, std::string_view>& to_hash) const {
		std::size_t first = sw_hasher(to_hash.first);
		std::size_t second = sw_hasher(to_hash.second);
		return first + second * 37;
	}
} // namespace domain