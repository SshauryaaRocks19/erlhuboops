#pragma once

#include "Equipment.h"
#include "Geo.h"
#include "TransportVehicle.h"

#include <string>

namespace ReliefOps::Logistics {

struct Corridor {
    std::string name;
    GeoPoint from;
    GeoPoint to;
    double roadIntegrity;   // 0 = impassable, 1 = intact
    bool flooded;
    bool airspaceOpen;
};

struct RouteAssessment {
    bool feasible = false;
    double distanceKm = 0.0;
    double hours = 0.0;
    double fuelL = 0.0;
    std::string note;
};

class ReliefVehicle : public TransportVehicle {
public:
    ReliefVehicle(std::string callSign, double maxPayloadKg, double fuelL, double burnPerKm, GeoPoint base);
    ReliefVehicle& operator=(const ReliefVehicle&) = delete;
    ~ReliefVehicle() override;

    virtual RouteAssessment evaluateRoute(const Corridor& c) const = 0;
    virtual std::string kind() const = 0;

    RouteAssessment depart(const Corridor& c);
    const GPSBeacon& beacon() const { return beacon_; }

protected:
    RouteAssessment plan(double km, double speedKmh, const std::string& note) const;
    static RouteAssessment blocked(const std::string& reason);

private:
    GPSBeacon beacon_;
    double burnPerKm_;
};

class AerialTransport : public ReliefVehicle {
public:
    AerialTransport(std::string callSign, double maxPayloadKg, double fuelL, double burnPerKm,
                    GeoPoint base, double cruiseKmh, EmergencyLifeSupportUnit lifeSupport);
    ~AerialTransport() override;

    RouteAssessment evaluateRoute(const Corridor& c) const override;
    std::string kind() const override { return "Aerial"; }
    const EmergencyLifeSupportUnit& lifeSupport() const { return lifeSupport_; }

private:
    double cruiseKmh_;
    EmergencyLifeSupportUnit lifeSupport_;
};

class GroundConvoy : public virtual ReliefVehicle {
public:
    GroundConvoy(std::string callSign, double maxPayloadKg, double fuelL, double burnPerKm,
                 GeoPoint base, double roadKmh);
    ~GroundConvoy() override;

    RouteAssessment evaluateRoute(const Corridor& c) const override;
    std::string kind() const override { return "Ground"; }

private:
    double roadKmh_;
};

class AmphibiousCraft : public virtual ReliefVehicle {
public:
    AmphibiousCraft(std::string callSign, double maxPayloadKg, double fuelL, double burnPerKm,
                    GeoPoint base, double waterKmh);
    ~AmphibiousCraft() override;

    RouteAssessment evaluateRoute(const Corridor& c) const override;
    std::string kind() const override { return "Amphibious"; }

private:
    double waterKmh_;
};

// Diamond: both parents share a single virtual ReliefVehicle sub-object.
class AmphibiousMedicalUnit : public GroundConvoy, public AmphibiousCraft {
public:
    AmphibiousMedicalUnit(std::string callSign, double maxPayloadKg, double fuelL, double burnPerKm,
                          GeoPoint base, double roadKmh, double waterKmh,
                          EmergencyLifeSupportUnit lifeSupport);
    ~AmphibiousMedicalUnit() override;

    RouteAssessment evaluateRoute(const Corridor& c) const override;
    std::string kind() const override { return "Amphibious medical"; }
    const EmergencyLifeSupportUnit& lifeSupport() const { return lifeSupport_; }

private:
    EmergencyLifeSupportUnit lifeSupport_;
};

}  // namespace ReliefOps::Logistics
