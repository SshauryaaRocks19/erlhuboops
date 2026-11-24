#pragma once

#include "ReliefCargo.h"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace ReliefOps::Logistics {

struct MissionParameters {
    std::string corridor = "unassigned";
    double distanceKm = 0.0;
    Inventory::TriageLevel priority = Inventory::TriageLevel::Green;
};

class IncidentCommander;

class TransportVehicle {
public:
    TransportVehicle(std::string callSign, double maxPayloadKg, double fuelL, std::size_t manifestSlots = 2);
    TransportVehicle(const TransportVehicle& other);
    TransportVehicle& operator=(const TransportVehicle& other);
    virtual ~TransportVehicle();

    const std::string& callSign() const { return callSign_; }
    double maxPayloadKg() const { return maxPayloadKg_; }
    double fuelL() const { return fuelL_; }
    double payloadKg() const;
    std::size_t cargoCount() const { return size_; }

    Inventory::ReliefCargo& cargoAt(std::size_t index);
    const Inventory::ReliefCargo& cargoAt(std::size_t index) const;
    MissionParameters& mission() { return mission_; }
    const MissionParameters& mission() const { return mission_; }

    TransportVehicle& operator+=(const Inventory::ReliefCargo& lot);
    bool operator==(const TransportVehicle& other) const;
    void unloadAll() { size_ = 0; }

    static int operationalCount() { return operational_; }
    static double totalDispatchedTonnes() { return dispatchedKg_ / 1000.0; }

    friend std::ostream& operator<<(std::ostream& os, const TransportVehicle& v);
    friend class IncidentCommander;

protected:
    void consumeFuel(double litres) { fuelL_ -= litres; }
    static void recordDispatch(double kg) { dispatchedKg_ += kg; }

private:
    void grow();

    std::string callSign_;
    double maxPayloadKg_;
    double fuelL_;
    Inventory::ReliefCargo* manifest_;
    std::size_t size_;
    std::size_t capacity_;
    MissionParameters mission_;

    static int operational_;
    static double dispatchedKg_;
};

}  // namespace ReliefOps::Logistics
