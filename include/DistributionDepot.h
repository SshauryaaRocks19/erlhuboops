#pragma once

#include "Geo.h"
#include "ReliefCargo.h"

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <vector>

namespace ReliefOps::Logistics {
class IncidentCommander;
}

namespace ReliefOps::Inventory {

class DistributionDepot {
public:
    DistributionDepot(std::string name, GeoPoint location);

    const std::string& name() const { return name_; }
    const GeoPoint& location() const { return location_; }

    int& stockOf(SupplyType type);
    int stockOf(SupplyType type) const;

    void setReserveFloor(SupplyType type, int units);
    int reserveFloor(SupplyType type) const;
    int releasable(SupplyType type) const;

    DistributionDepot& operator+=(const ReliefCargo& lot);
    DistributionDepot& operator-=(const ReliefCargo& lot);

    bool hasReceived(const ReliefCargo& lot) const;
    double totalMassKg() const;
    const std::vector<std::string>& overrideLog() const { return overrideLog_; }

    friend std::ostream& operator<<(std::ostream& os, const DistributionDepot& depot);
    friend class Logistics::IncidentCommander;

private:
    static int lookup(const std::unordered_map<SupplyType, int>& table, SupplyType type);

    std::string name_;
    GeoPoint location_;
    std::unordered_map<SupplyType, int> stock_;
    std::unordered_map<SupplyType, int> reserve_;
    std::vector<ReliefCargo> receivedLots_;
    std::vector<std::string> overrideLog_;
};

}  // namespace ReliefOps::Inventory
