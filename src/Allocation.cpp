#include "Allocation.h"

#include "Exceptions.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ReliefOps::Inventory {

int traumaKitsPerCasualty(TriageLevel level)
{
    switch (level) {
    case TriageLevel::Red:    return 3;
    case TriageLevel::Yellow: return 2;
    case TriageLevel::Green:  return 1;
    }
    return 1;
}

ReliefCargo allocateSupplies(const DistributionDepot& depot, SupplyType type, double cargoSpaceM3)
{
    if (cargoSpaceM3 <= 0.0) {
        throw std::invalid_argument("cargo space must be positive");
    }
    const int fits = static_cast<int>(std::floor(cargoSpaceM3 / specFor(type).unitVolumeM3));
    const int units = std::min(fits, depot.releasable(type));
    if (units == 0) {
        throw InsufficientStockException(depot.name(), toString(type), std::max(fits, 1),
                                         depot.releasable(type));
    }
    return ReliefCargo::standardLot(type, units);
}

ReliefCargo allocateSupplies(const DistributionDepot& depot, TriageLevel level, int casualties)
{
    if (casualties <= 0) {
        throw std::invalid_argument("casualty count must be positive");
    }
    const int needed = casualties * traumaKitsPerCasualty(level);
    const int available = depot.releasable(SupplyType::TraumaMedicine);
    if (needed > available) {
        throw InsufficientStockException(depot.name(), toString(SupplyType::TraumaMedicine),
                                         needed, available);
    }
    return ReliefCargo::standardLot(SupplyType::TraumaMedicine, needed, level);
}

}  // namespace ReliefOps::Inventory
