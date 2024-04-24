#pragma once

#include "json.h"
#include "transport_catalogue.h"

namespace json_reader {
// --------------------------------- [Json Reader] Definition -------------------------
//                                                                                    +
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Json Reader +
	class JsonReader {
	public:
		JsonReader(catalogue::TransportCatalogue& database);
		void FillTransportCatalogue(const json::Document& document);

	private:
		catalogue::TransportCatalogue& database_;
	};
} // namespace input_reader