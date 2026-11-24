#pragma once

#include "ReliefCargo.h"

#include <string>
#include <vector>

namespace ReliefOps::Logistics {

struct TransitRecord {
    std::string callSign;
    std::string corridor;
    double payloadKg = 0.0;
    double distanceKm = 0.0;
    bool delivered = false;
};

void saveTransitLog(const std::string& path, const std::vector<TransitRecord>& log);
std::vector<TransitRecord> loadTransitLog(const std::string& path);

void saveManifestAudit(const std::string& path, const std::vector<Inventory::ReliefCargo>& lots);
std::vector<Inventory::ReliefCargo> loadManifestAudit(const std::string& path);

}  // namespace ReliefOps::Logistics
