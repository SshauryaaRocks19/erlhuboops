#pragma once

#include "ReliefCargo.h"

#include <cstddef>
#include <ostream>

namespace ReliefOps::Inventory {

inline constexpr std::size_t kManifestWidth = 62;

std::ostream& manifestHeader(std::ostream& os);

class rule {
public:
    explicit rule(std::size_t width = kManifestWidth, char fill = '-') : width_(width), fill_(fill) {}
    friend std::ostream& operator<<(std::ostream& os, const rule& r);

private:
    std::size_t width_;
    char fill_;
};

class tonnes {
public:
    explicit tonnes(double kilograms, int precision = 2) : kg_(kilograms), precision_(precision) {}
    friend std::ostream& operator<<(std::ostream& os, const tonnes& t);

private:
    double kg_;
    int precision_;
};

class row {
public:
    explicit row(const ReliefCargo& lot) : lot_(lot) {}
    friend std::ostream& operator<<(std::ostream& os, const row& r);

private:
    const ReliefCargo& lot_;
};

}  // namespace ReliefOps::Inventory
