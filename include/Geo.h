#pragma once

#include <cmath>
#include <limits>
#include <ostream>

namespace ReliefOps {

struct GeoPoint {
    double lat = 0.0;
    double lon = 0.0;
};

inline std::ostream& operator<<(std::ostream& os, const GeoPoint& p)
{
    return os << '(' << p.lat << ", " << p.lon << ')';
}

}  // namespace ReliefOps

namespace ReliefOps::Logistics {

inline constexpr double kEarthRadiusKm = 6371.0;
inline constexpr double kPi = 3.14159265358979323846;

inline double toRadians(double degrees) { return degrees * kPi / 180.0; }

inline double haversineKm(const GeoPoint& a, const GeoPoint& b)
{
    const double dLat = toRadians(b.lat - a.lat);
    const double dLon = toRadians(b.lon - a.lon);
    const double h = std::sin(dLat / 2) * std::sin(dLat / 2) +
                     std::cos(toRadians(a.lat)) * std::cos(toRadians(b.lat)) *
                     std::sin(dLon / 2) * std::sin(dLon / 2);
    return 2.0 * kEarthRadiusKm * std::asin(std::sqrt(h));
}

inline double payloadUtilisation(double loadKg, double capacityKg)
{
    return capacityKg > 0.0 ? loadKg / capacityKg : 0.0;
}

inline double fuelForDistance(double km, double litresPerKm) { return km * litresPerKm; }

inline double travelHours(double km, double speedKmh)
{
    return speedKmh > 0.0 ? km / speedKmh : std::numeric_limits<double>::infinity();
}

}  // namespace ReliefOps::Logistics
