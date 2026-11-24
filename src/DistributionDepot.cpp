#include "DistributionDepot.h"

#include "Exceptions.h"
#include "ManifestFormat.h"

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <stdexcept>

namespace ReliefOps::Inventory {

DistributionDepot::DistributionDepot(std::string name, GeoPoint location)
    : name_(std::move(name)), location_(location)
{
    for (SupplyType type : kAllSupplyTypes) {
        stock_[type] = 0;
        reserve_[type] = 0;
    }
}

int DistributionDepot::lookup(const std::unordered_map<SupplyType, int>& table, SupplyType type)
{
    const auto it = table.find(type);
    return it == table.end() ? 0 : it->second;
}

int& DistributionDepot::stockOf(SupplyType type)
{
    return stock_[type];
}

int DistributionDepot::stockOf(SupplyType type) const
{
    return lookup(stock_, type);
}

void DistributionDepot::setReserveFloor(SupplyType type, int units)
{
    if (units < 0) {
        throw std::invalid_argument("reserve floor cannot be negative");
    }
    reserve_[type] = units;
}

int DistributionDepot::reserveFloor(SupplyType type) const
{
    return lookup(reserve_, type);
}

int DistributionDepot::releasable(SupplyType type) const
{
    return std::max(0, stockOf(type) - reserveFloor(type));
}

DistributionDepot& DistributionDepot::operator+=(const ReliefCargo& lot)
{
    if (hasReceived(lot)) {
        throw DuplicateCargoException(lot.manifestId());
    }
    receivedLots_.push_back(lot);
    stock_[lot.type()] += lot.units();
    return *this;
}

DistributionDepot& DistributionDepot::operator-=(const ReliefCargo& lot)
{
    const int available = releasable(lot.type());
    if (lot.units() > available) {
        throw InsufficientStockException(name_, toString(lot.type()), lot.units(), available);
    }
    stock_[lot.type()] -= lot.units();
    return *this;
}

bool DistributionDepot::hasReceived(const ReliefCargo& lot) const
{
    return std::find(receivedLots_.begin(), receivedLots_.end(), lot) != receivedLots_.end();
}

double DistributionDepot::totalMassKg() const
{
    double total = 0.0;
    for (const auto& [type, units] : stock_) {
        total += units * specFor(type).unitMassKg;
    }
    return total;
}

std::ostream& operator<<(std::ostream& os, const DistributionDepot& depot)
{
    os << "Depot " << depot.name_ << ' ' << depot.location_ << '\n'
       << rule(44)
       << std::left << std::setw(14) << "Item"
       << std::right << std::setw(10) << "On hand"
       << std::setw(10) << "Reserve"
       << std::setw(10) << "Free" << '\n'
       << rule(44);
    for (SupplyType type : kAllSupplyTypes) {
        os << std::left << std::setw(14) << toString(type)
           << std::right << std::setw(10) << depot.stockOf(type)
           << std::setw(10) << depot.reserveFloor(type)
           << std::setw(10) << depot.releasable(type) << '\n';
    }
    os << rule(44) << "Total mass on hand: " << tonnes(depot.totalMassKg()) << '\n';
    return os;
}

}  // namespace ReliefOps::Inventory
