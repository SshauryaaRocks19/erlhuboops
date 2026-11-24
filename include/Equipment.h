#pragma once

#include "Geo.h"

#include <string>

namespace ReliefOps::Logistics {

class GPSBeacon {
public:
    GPSBeacon(std::string unitId, GeoPoint fix) : unitId_(std::move(unitId)), fix_(fix) {}

    const std::string& unitId() const { return unitId_; }
    const GeoPoint& lastFix() const { return fix_; }
    void moveTo(const GeoPoint& p) { fix_ = p; }
    double distanceTo(const GeoPoint& p) const { return haversineKm(fix_, p); }

private:
    std::string unitId_;
    GeoPoint fix_;
};

class EmergencyLifeSupportUnit {
public:
    EmergencyLifeSupportUnit(int stretchers, double oxygenL) : stretchers_(stretchers), oxygenL_(oxygenL) {}

    int stretchers() const { return stretchers_; }
    double oxygenL() const { return oxygenL_; }
    double oxygenHours(int patients) const
    {
        return patients > 0 ? oxygenL_ / (patients * kLitresPerPatientHour) : 0.0;
    }

private:
    static constexpr double kLitresPerPatientHour = 360.0;

    int stretchers_;
    double oxygenL_;
};

}  // namespace ReliefOps::Logistics
