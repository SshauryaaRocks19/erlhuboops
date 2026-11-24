#pragma once

#include <iosfwd>
#include <string>

namespace ReliefOps::Inventory {

enum class SupplyType { RationKit, TraumaMedicine, WaterTank, ThermalBlanket };

// START triage colours, ordered so that a higher value is more urgent.
enum class TriageLevel { Green = 1, Yellow = 2, Red = 3 };

struct SupplySpec {
    double unitMassKg;
    double unitVolumeM3;
};

inline constexpr SupplyType kAllSupplyTypes[] = {
    SupplyType::RationKit,
    SupplyType::TraumaMedicine,
    SupplyType::WaterTank,
    SupplyType::ThermalBlanket,
};

const SupplySpec& specFor(SupplyType type);

const char* toString(SupplyType type);
const char* toString(TriageLevel level);

SupplyType parseSupplyType(const std::string& code);
TriageLevel parseTriageLevel(const std::string& code);

std::ostream& operator<<(std::ostream& os, SupplyType type);
std::ostream& operator<<(std::ostream& os, TriageLevel level);

}  // namespace ReliefOps::Inventory
