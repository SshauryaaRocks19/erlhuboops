#include "ManifestFormat.h"

#include <iomanip>
#include <string>

namespace ReliefOps::Inventory {

namespace {

class StreamStateGuard {
public:
    explicit StreamStateGuard(std::ostream& os)
        : os_(os), flags_(os.flags()), precision_(os.precision()), fill_(os.fill())
    {
    }
    ~StreamStateGuard()
    {
        os_.flags(flags_);
        os_.precision(precision_);
        os_.fill(fill_);
    }
    StreamStateGuard(const StreamStateGuard&) = delete;
    StreamStateGuard& operator=(const StreamStateGuard&) = delete;

private:
    std::ostream& os_;
    std::ios::fmtflags flags_;
    std::streamsize precision_;
    char fill_;
};

}  // namespace

std::ostream& manifestHeader(std::ostream& os)
{
    StreamStateGuard guard(os);
    os << rule()
       << std::left << std::setw(10) << "Lot"
       << std::setw(12) << "Item"
       << std::right << std::setw(7) << "Units"
       << std::setw(12) << "Mass (kg)"
       << std::setw(11) << "Vol (m3)"
       << std::setw(10) << "Triage" << '\n'
       << rule();
    return os;
}

std::ostream& operator<<(std::ostream& os, const rule& r)
{
    return os << std::string(r.width_, r.fill_) << '\n';
}

std::ostream& operator<<(std::ostream& os, const tonnes& t)
{
    StreamStateGuard guard(os);
    return os << std::fixed << std::setprecision(t.precision_) << t.kg_ / 1000.0 << " t";
}

std::ostream& operator<<(std::ostream& os, const row& r)
{
    StreamStateGuard guard(os);
    const ReliefCargo& lot = r.lot_;
    os << std::left << std::setw(10) << lot.manifestId()
       << std::setw(12) << toString(lot.type())
       << std::right << std::setw(7) << lot.units()
       << std::fixed << std::setprecision(1) << std::setw(12) << lot.massKg()
       << std::setprecision(2) << std::setw(11) << lot.volumeM3()
       << std::setw(10) << toString(lot.priority()) << '\n';
    return os;
}

}  // namespace ReliefOps::Inventory
