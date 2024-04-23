#pragma once

namespace geo {
// ----------------------------------- [Geography] Definition -------------------------
//                                                                                    +
//                                                                                    + -----------
// ------------------------------------------------------------------------------------ Geography +
    struct Coordinates final {
        double lat;
        double lng;

        bool operator==(const Coordinates& other) const {
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(const Coordinates& other) const {
            return !(*this == other);
        }
    };

    double ComputeDistance(Coordinates from, Coordinates to);
} // namespace geo