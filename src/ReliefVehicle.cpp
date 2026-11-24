#include "ReliefVehicle.h"

#include "Exceptions.h"

#include <iostream>

namespace ReliefOps::Logistics {

namespace {

constexpr double kMinRoadIntegrity = 0.4;
constexpr double kRoadWindingFactor = 1.4;
constexpr double kWaterWindingFactor = 1.2;

}  // namespace

ReliefVehicle::ReliefVehicle(std::string callSign, double maxPayloadKg, double fuelL,
                             double burnPerKm, GeoPoint base)
    : TransportVehicle(callSign, maxPayloadKg, fuelL),
      beacon_("GPS-" + callSign, base),
      burnPerKm_(burnPerKm)
{
}

ReliefVehicle::~ReliefVehicle()
{
    std::cout << "    ~ReliefVehicle " << callSign() << '\n';
}

RouteAssessment ReliefVehicle::plan(double km, double speedKmh, const std::string& note) const
{
    RouteAssessment r;
    r.feasible = true;
    r.distanceKm = km;
    r.hours = travelHours(km, speedKmh);
    r.fuelL = fuelForDistance(km, burnPerKm_);
    r.note = note;
    return r;
}

RouteAssessment ReliefVehicle::blocked(const std::string& reason)
{
    RouteAssessment r;
    r.note = reason;
    return r;
}

RouteAssessment ReliefVehicle::depart(const Corridor& c)
{
    const RouteAssessment route = evaluateRoute(c);
    if (!route.feasible) {
        throw DamagedCorridorException(c.name, route.note);
    }
    if (route.fuelL > fuelL()) {
        throw FuelDepletionException(callSign(), route.fuelL, fuelL());
    }
    consumeFuel(route.fuelL);
    recordDispatch(payloadKg());
    beacon_.moveTo(c.to);
    mission().corridor = c.name;
    mission().distanceKm = route.distanceKm;
    return route;
}

AerialTransport::AerialTransport(std::string callSign, double maxPayloadKg, double fuelL,
                                 double burnPerKm, GeoPoint base, double cruiseKmh,
                                 EmergencyLifeSupportUnit lifeSupport)
    : ReliefVehicle(std::move(callSign), maxPayloadKg, fuelL, burnPerKm, base),
      cruiseKmh_(cruiseKmh),
      lifeSupport_(lifeSupport)
{
}

AerialTransport::~AerialTransport()
{
    std::cout << "    ~AerialTransport " << callSign() << '\n';
}

RouteAssessment AerialTransport::evaluateRoute(const Corridor& c) const
{
    if (!c.airspaceOpen) {
        return blocked("airspace closed");
    }
    return plan(haversineKm(c.from, c.to), cruiseKmh_, "direct flight");
}

GroundConvoy::GroundConvoy(std::string callSign, double maxPayloadKg, double fuelL,
                           double burnPerKm, GeoPoint base, double roadKmh)
    : ReliefVehicle(std::move(callSign), maxPayloadKg, fuelL, burnPerKm, base),
      roadKmh_(roadKmh)
{
}

GroundConvoy::~GroundConvoy()
{
    std::cout << "    ~GroundConvoy " << callSign() << '\n';
}

RouteAssessment GroundConvoy::evaluateRoute(const Corridor& c) const
{
    if (c.flooded) {
        return blocked("road under water");
    }
    if (c.roadIntegrity < kMinRoadIntegrity) {
        return blocked("road damaged by landslide");
    }
    const double km = haversineKm(c.from, c.to) * kRoadWindingFactor;
    return plan(km, roadKmh_ * c.roadIntegrity, "road");
}

AmphibiousCraft::AmphibiousCraft(std::string callSign, double maxPayloadKg, double fuelL,
                                 double burnPerKm, GeoPoint base, double waterKmh)
    : ReliefVehicle(std::move(callSign), maxPayloadKg, fuelL, burnPerKm, base),
      waterKmh_(waterKmh)
{
}

AmphibiousCraft::~AmphibiousCraft()
{
    std::cout << "    ~AmphibiousCraft " << callSign() << '\n';
}

RouteAssessment AmphibiousCraft::evaluateRoute(const Corridor& c) const
{
    if (!c.flooded) {
        return blocked("no navigable water");
    }
    const double km = haversineKm(c.from, c.to) * kWaterWindingFactor;
    return plan(km, waterKmh_, "water");
}

AmphibiousMedicalUnit::AmphibiousMedicalUnit(std::string callSign, double maxPayloadKg, double fuelL,
                                             double burnPerKm, GeoPoint base, double roadKmh,
                                             double waterKmh, EmergencyLifeSupportUnit lifeSupport)
    : ReliefVehicle(callSign, maxPayloadKg, fuelL, burnPerKm, base),
      GroundConvoy(callSign, maxPayloadKg, fuelL, burnPerKm, base, roadKmh),
      AmphibiousCraft(callSign, maxPayloadKg, fuelL, burnPerKm, base, waterKmh),
      lifeSupport_(lifeSupport)
{
}

AmphibiousMedicalUnit::~AmphibiousMedicalUnit()
{
    std::cout << "    ~AmphibiousMedicalUnit " << callSign() << '\n';
}

RouteAssessment AmphibiousMedicalUnit::evaluateRoute(const Corridor& c) const
{
    const RouteAssessment road = GroundConvoy::evaluateRoute(c);
    const RouteAssessment water = AmphibiousCraft::evaluateRoute(c);
    if (road.feasible && water.feasible) {
        return road.hours <= water.hours ? road : water;
    }
    if (road.feasible) {
        return road;
    }
    if (water.feasible) {
        return water;
    }
    return blocked(road.note + ", " + water.note);
}

}  // namespace ReliefOps::Logistics
