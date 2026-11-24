#include "Persistence.h"

#include "BinaryIO.h"

#include <cstdint>
#include <sstream>

namespace ReliefOps::Logistics {

namespace {

constexpr std::uint32_t kTransitMagic = 0x544C5245;   // "ERLT"
constexpr std::uint32_t kAuditMagic = 0x4D4C5245;     // "ERLM"
constexpr std::uint16_t kFormatVersion = 1;

}  // namespace

void saveTransitLog(const std::string& path, const std::vector<TransitRecord>& log)
{
    std::ostringstream payload(std::ios::binary);
    io::writePod(payload, static_cast<std::uint32_t>(log.size()));
    for (const TransitRecord& r : log) {
        io::writeString(payload, r.callSign);
        io::writeString(payload, r.corridor);
        io::writePod(payload, r.payloadKg);
        io::writePod(payload, r.distanceKm);
        io::writePod(payload, static_cast<std::uint8_t>(r.delivered ? 1 : 0));
    }
    io::writeArchive(path, kTransitMagic, kFormatVersion, payload.str());
}

std::vector<TransitRecord> loadTransitLog(const std::string& path)
{
    std::istringstream in(io::readArchive(path, kTransitMagic, kFormatVersion), std::ios::binary);
    const auto count = io::readPod<std::uint32_t>(in);
    std::vector<TransitRecord> log;
    log.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        TransitRecord r;
        r.callSign = io::readString(in);
        r.corridor = io::readString(in);
        r.payloadKg = io::readPod<double>(in);
        r.distanceKm = io::readPod<double>(in);
        r.delivered = io::readPod<std::uint8_t>(in) != 0;
        log.push_back(r);
    }
    return log;
}

void saveManifestAudit(const std::string& path, const std::vector<Inventory::ReliefCargo>& lots)
{
    std::ostringstream payload(std::ios::binary);
    io::writePod(payload, static_cast<std::uint32_t>(lots.size()));
    for (const auto& lot : lots) {
        lot.writeBinary(payload);
    }
    io::writeArchive(path, kAuditMagic, kFormatVersion, payload.str());
}

std::vector<Inventory::ReliefCargo> loadManifestAudit(const std::string& path)
{
    std::istringstream in(io::readArchive(path, kAuditMagic, kFormatVersion), std::ios::binary);
    const auto count = io::readPod<std::uint32_t>(in);
    std::vector<Inventory::ReliefCargo> lots;
    lots.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        lots.push_back(Inventory::ReliefCargo::readBinary(in));
    }
    return lots;
}

}  // namespace ReliefOps::Logistics
