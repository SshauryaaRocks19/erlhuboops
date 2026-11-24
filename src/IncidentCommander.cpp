#include "IncidentCommander.h"

#include <stdexcept>

namespace ReliefOps::Logistics {

void IncidentCommander::overrideReserve(Inventory::DistributionDepot& depot,
                                        Inventory::SupplyType type, int newFloor)
{
    if (newFloor < 0) {
        throw std::invalid_argument("reserve floor cannot be negative");
    }
    const int old = depot.reserve_[type];
    depot.reserve_[type] = newFloor;
    depot.overrideLog_.push_back(name_ + " lowered " + Inventory::toString(type) + " reserve " +
                                 std::to_string(old) + " -> " + std::to_string(newFloor));
}

void IncidentCommander::authoriseOverload(TransportVehicle& vehicle, double extraKg)
{
    if (extraKg <= 0.0 || extraKg > vehicle.maxPayloadKg_ * kMaxOverloadFraction) {
        throw std::invalid_argument("overload must be positive and at most 10% of rated payload");
    }
    vehicle.maxPayloadKg_ += extraKg;
}

}  // namespace ReliefOps::Logistics
