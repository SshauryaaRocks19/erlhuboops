#include "ReliefCargo.h"

#include "BinaryIO.h"

#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace ReliefOps::Inventory {

int ReliefCargo::lotsIssued_ = 0;

ReliefCargo::ReliefCargo()
    : manifestId_("UNASSIGNED"),
      type_(SupplyType::RationKit),
      units_(0),
      unitMassKg_(specFor(SupplyType::RationKit).unitMassKg),
      unitVolumeM3_(specFor(SupplyType::RationKit).unitVolumeM3),
      priority_(TriageLevel::Green)
{
}

ReliefCargo::ReliefCargo(std::string manifestId, SupplyType type, int units,
                         double unitMassKg, double unitVolumeM3, TriageLevel priority)
    : manifestId_(std::move(manifestId)),
      type_(type),
      units_(units),
      unitMassKg_(unitMassKg),
      unitVolumeM3_(unitVolumeM3),
      priority_(priority)
{
    validate();
}

void ReliefCargo::validate() const
{
    if (manifestId_.empty() || manifestId_.find_first_of(" \t\n") != std::string::npos) {
        throw std::invalid_argument("manifest id must be a single non-empty token");
    }
    if (units_ < 0) {
        throw std::invalid_argument(manifestId_ + ": unit count cannot be negative");
    }
    if (unitMassKg_ <= 0.0 || unitVolumeM3_ <= 0.0) {
        throw std::invalid_argument(manifestId_ + ": unit mass and volume must be positive");
    }
}

std::string ReliefCargo::nextManifestId()
{
    std::ostringstream id;
    id << "RC-" << std::setw(4) << std::setfill('0') << ++lotsIssued_;
    return id.str();
}

ReliefCargo ReliefCargo::standardLot(SupplyType type, int units, TriageLevel priority)
{
    const SupplySpec& spec = specFor(type);
    return ReliefCargo(nextManifestId(), type, units, spec.unitMassKg, spec.unitVolumeM3, priority);
}

void ReliefCargo::writeBinary(std::ostream& out) const
{
    io::writeString(out, manifestId_);
    io::writePod(out, static_cast<std::uint8_t>(type_));
    io::writePod(out, static_cast<std::int32_t>(units_));
    io::writePod(out, unitMassKg_);
    io::writePod(out, unitVolumeM3_);
    io::writePod(out, static_cast<std::uint8_t>(priority_));
}

ReliefCargo ReliefCargo::readBinary(std::istream& in)
{
    std::string id = io::readString(in);
    const auto type = io::readPod<std::uint8_t>(in);
    const auto units = io::readPod<std::int32_t>(in);
    const auto mass = io::readPod<double>(in);
    const auto volume = io::readPod<double>(in);
    const auto priority = io::readPod<std::uint8_t>(in);

    if (type > static_cast<std::uint8_t>(SupplyType::ThermalBlanket) ||
        priority < static_cast<std::uint8_t>(TriageLevel::Green) ||
        priority > static_cast<std::uint8_t>(TriageLevel::Red)) {
        throw PersistenceException("cargo record " + id + " has an invalid enum value");
    }
    return ReliefCargo(std::move(id), static_cast<SupplyType>(type), units, mass, volume,
                       static_cast<TriageLevel>(priority));
}

std::ostream& operator<<(std::ostream& os, const ReliefCargo& cargo)
{
    return os << cargo.manifestId_ << ' ' << cargo.type_ << ' ' << cargo.units_ << ' '
              << cargo.unitMassKg_ << ' ' << cargo.unitVolumeM3_ << ' ' << cargo.priority_;
}

std::istream& operator>>(std::istream& is, ReliefCargo& cargo)
{
    std::string id, typeCode, priorityCode;
    int units = 0;
    double mass = 0.0;
    double volume = 0.0;

    if (!(is >> id >> typeCode >> units >> mass >> volume >> priorityCode)) {
        return is;
    }
    try {
        cargo = ReliefCargo(id, parseSupplyType(typeCode), units, mass, volume,
                            parseTriageLevel(priorityCode));
    } catch (const std::invalid_argument&) {
        is.setstate(std::ios::failbit);
    }
    return is;
}

}  // namespace ReliefOps::Inventory
