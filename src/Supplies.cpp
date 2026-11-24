#include "Supplies.h"

#include <ostream>
#include <stdexcept>

namespace ReliefOps::Inventory {

namespace {

const SupplySpec kRationSpec{12.0, 0.030};
const SupplySpec kTraumaSpec{4.5, 0.012};
const SupplySpec kWaterTankSpec{210.0, 0.220};
const SupplySpec kBlanketSpec{0.9, 0.004};

}  // namespace

const SupplySpec& specFor(SupplyType type)
{
    switch (type) {
    case SupplyType::RationKit:      return kRationSpec;
    case SupplyType::TraumaMedicine: return kTraumaSpec;
    case SupplyType::WaterTank:      return kWaterTankSpec;
    case SupplyType::ThermalBlanket: return kBlanketSpec;
    }
    throw std::invalid_argument("unknown supply type");
}

const char* toString(SupplyType type)
{
    switch (type) {
    case SupplyType::RationKit:      return "RATION";
    case SupplyType::TraumaMedicine: return "TRAUMA_MED";
    case SupplyType::WaterTank:      return "WATER";
    case SupplyType::ThermalBlanket: return "BLANKET";
    }
    return "?";
}

const char* toString(TriageLevel level)
{
    switch (level) {
    case TriageLevel::Green:  return "GREEN";
    case TriageLevel::Yellow: return "YELLOW";
    case TriageLevel::Red:    return "RED";
    }
    return "?";
}

SupplyType parseSupplyType(const std::string& code)
{
    for (SupplyType type : kAllSupplyTypes) {
        if (code == toString(type)) {
            return type;
        }
    }
    throw std::invalid_argument("unknown supply code: " + code);
}

TriageLevel parseTriageLevel(const std::string& code)
{
    if (code == "GREEN") return TriageLevel::Green;
    if (code == "YELLOW") return TriageLevel::Yellow;
    if (code == "RED") return TriageLevel::Red;
    throw std::invalid_argument("unknown triage code: " + code);
}

std::ostream& operator<<(std::ostream& os, SupplyType type)
{
    return os << toString(type);
}

std::ostream& operator<<(std::ostream& os, TriageLevel level)
{
    return os << toString(level);
}

}  // namespace ReliefOps::Inventory
