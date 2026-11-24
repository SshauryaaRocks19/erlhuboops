#pragma once

#include "DistributionDepot.h"
#include "TransportVehicle.h"

#include <string>

namespace ReliefOps::Logistics {

class IncidentCommander {
public:
    explicit IncidentCommander(std::string name) : name_(std::move(name)) {}

    const std::string& name() const { return name_; }

    void overrideReserve(Inventory::DistributionDepot& depot, Inventory::SupplyType type, int newFloor);
    void authoriseOverload(TransportVehicle& vehicle, double extraKg);

private:
    static constexpr double kMaxOverloadFraction = 0.10;

    std::string name_;
};

}  // namespace ReliefOps::Logistics
