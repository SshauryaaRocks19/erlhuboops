#include "TransportVehicle.h"

#include "Exceptions.h"
#include "Geo.h"
#include "ManifestFormat.h"

#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace ReliefOps::Logistics {

using Inventory::ReliefCargo;

int TransportVehicle::operational_ = 0;
double TransportVehicle::dispatchedKg_ = 0.0;

TransportVehicle::TransportVehicle(std::string callSign, double maxPayloadKg, double fuelL,
                                   std::size_t manifestSlots)
    : callSign_(std::move(callSign)),
      maxPayloadKg_(maxPayloadKg),
      fuelL_(fuelL),
      manifest_(new ReliefCargo[manifestSlots > 0 ? manifestSlots : 1]),
      size_(0),
      capacity_(manifestSlots > 0 ? manifestSlots : 1)
{
    ++operational_;
}

TransportVehicle::TransportVehicle(const TransportVehicle& other)
    : callSign_(other.callSign_),
      maxPayloadKg_(other.maxPayloadKg_),
      fuelL_(other.fuelL_),
      manifest_(new ReliefCargo[other.capacity_]),
      size_(other.size_),
      capacity_(other.capacity_),
      mission_(other.mission_)
{
    for (std::size_t i = 0; i < size_; ++i) {
        manifest_[i] = other.manifest_[i];
    }
    ++operational_;
}

TransportVehicle& TransportVehicle::operator=(const TransportVehicle& other)
{
    if (this != &other) {
        TransportVehicle copy(other);
        std::swap(callSign_, copy.callSign_);
        std::swap(maxPayloadKg_, copy.maxPayloadKg_);
        std::swap(fuelL_, copy.fuelL_);
        std::swap(manifest_, copy.manifest_);
        std::swap(size_, copy.size_);
        std::swap(capacity_, copy.capacity_);
        std::swap(mission_, copy.mission_);
    }
    return *this;
}

TransportVehicle::~TransportVehicle()
{
    delete[] manifest_;
    --operational_;
}

double TransportVehicle::payloadKg() const
{
    double total = 0.0;
    for (std::size_t i = 0; i < size_; ++i) {
        total += manifest_[i].massKg();
    }
    return total;
}

ReliefCargo& TransportVehicle::cargoAt(std::size_t index)
{
    if (index >= size_) {
        throw std::out_of_range(callSign_ + ": no cargo at slot " + std::to_string(index));
    }
    return manifest_[index];
}

const ReliefCargo& TransportVehicle::cargoAt(std::size_t index) const
{
    return const_cast<TransportVehicle*>(this)->cargoAt(index);
}

void TransportVehicle::grow()
{
    const std::size_t newCapacity = capacity_ * 2;
    ReliefCargo* bigger = new ReliefCargo[newCapacity];
    for (std::size_t i = 0; i < size_; ++i) {
        bigger[i] = manifest_[i];
    }
    delete[] manifest_;
    manifest_ = bigger;
    capacity_ = newCapacity;
}

TransportVehicle& TransportVehicle::operator+=(const ReliefCargo& lot)
{
    for (std::size_t i = 0; i < size_; ++i) {
        if (manifest_[i] == lot) {
            throw DuplicateCargoException(lot.manifestId());
        }
    }
    const double after = payloadKg() + lot.massKg();
    if (after > maxPayloadKg_) {
        throw CargoOverweightException(callSign_, after, maxPayloadKg_);
    }
    if (size_ == capacity_) {
        grow();
    }
    manifest_[size_++] = lot;
    return *this;
}

bool TransportVehicle::operator==(const TransportVehicle& other) const
{
    if (size_ != other.size_) {
        return false;
    }
    for (std::size_t i = 0; i < size_; ++i) {
        if (manifest_[i] != other.manifest_[i]) {
            return false;
        }
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const TransportVehicle& v)
{
    os << v.callSign_ << "  payload " << Inventory::tonnes(v.payloadKg()) << " / "
       << Inventory::tonnes(v.maxPayloadKg_) << " ("
       << std::fixed << std::setprecision(0)
       << payloadUtilisation(v.payloadKg(), v.maxPayloadKg_) * 100.0 << "%)"
       << "  fuel " << v.fuelL_ << " L\n";
    os.unsetf(std::ios::fixed);
    os << std::setprecision(6) << Inventory::manifestHeader;
    for (std::size_t i = 0; i < v.size_; ++i) {
        os << Inventory::row(v.manifest_[i]);
    }
    return os;
}

}  // namespace ReliefOps::Logistics
