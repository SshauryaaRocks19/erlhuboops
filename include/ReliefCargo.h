#pragma once

#include "Supplies.h"

#include <iosfwd>
#include <string>

namespace ReliefOps::Inventory {

class ReliefCargo {
public:
    ReliefCargo();
    ReliefCargo(std::string manifestId, SupplyType type, int units,
                double unitMassKg, double unitVolumeM3,
                TriageLevel priority = TriageLevel::Green);

    static ReliefCargo standardLot(SupplyType type, int units,
                                   TriageLevel priority = TriageLevel::Green);
    static std::string nextManifestId();

    const std::string& manifestId() const { return manifestId_; }
    SupplyType type() const { return type_; }
    int units() const { return units_; }
    double unitMassKg() const { return unitMassKg_; }
    double unitVolumeM3() const { return unitVolumeM3_; }
    TriageLevel priority() const { return priority_; }

    double massKg() const { return units_ * unitMassKg_; }
    double volumeM3() const { return units_ * unitVolumeM3_; }
    bool empty() const { return units_ == 0; }

    bool operator==(const ReliefCargo& other) const { return manifestId_ == other.manifestId_; }
    bool operator!=(const ReliefCargo& other) const { return !(*this == other); }

    void writeBinary(std::ostream& out) const;
    static ReliefCargo readBinary(std::istream& in);

    friend std::ostream& operator<<(std::ostream& os, const ReliefCargo& cargo);
    friend std::istream& operator>>(std::istream& is, ReliefCargo& cargo);

private:
    void validate() const;

    std::string manifestId_;
    SupplyType type_;
    int units_;
    double unitMassKg_;
    double unitVolumeM3_;
    TriageLevel priority_;

    static int lotsIssued_;
};

}  // namespace ReliefOps::Inventory
