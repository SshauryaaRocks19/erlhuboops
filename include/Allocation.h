#pragma once

#include "DistributionDepot.h"
#include "ReliefCargo.h"

namespace ReliefOps::Inventory {

// Volume-driven: fill a given cargo space with as many units of one item as the depot can release.
ReliefCargo allocateSupplies(const DistributionDepot& depot, SupplyType type, double cargoSpaceM3);

// Triage-driven: size a trauma medicine lot from casualty count and severity.
ReliefCargo allocateSupplies(const DistributionDepot& depot, TriageLevel level, int casualties);

int traumaKitsPerCasualty(TriageLevel level);

}  // namespace ReliefOps::Inventory
