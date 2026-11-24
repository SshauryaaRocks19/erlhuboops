#pragma once

#include "Exceptions.h"

#include <cstdint>
#include <istream>
#include <ostream>
#include <string>
#include <type_traits>

namespace ReliefOps::io {

template <typename T>
void writePod(std::ostream& out, const T& value)
{
    static_assert(std::is_trivially_copyable_v<T>, "writePod needs a trivially copyable type");
    out.write(reinterpret_cast<const char*>(&value), sizeof value);
}

template <typename T>
T readPod(std::istream& in)
{
    static_assert(std::is_trivially_copyable_v<T>, "readPod needs a trivially copyable type");
    T value{};
    if (!in.read(reinterpret_cast<char*>(&value), sizeof value)) {
        throw PersistenceException("record truncated");
    }
    return value;
}

void writeString(std::ostream& out, const std::string& text);
std::string readString(std::istream& in);

std::uint32_t checksum(const std::string& bytes);

// Header layout: magic(u32) version(u16) payloadSize(u64) checksum(u32), then the payload.
// The file is written to "<path>.tmp" and renamed over the target only after a clean close.
void writeArchive(const std::string& path, std::uint32_t magic, std::uint16_t version,
                  const std::string& payload);
std::string readArchive(const std::string& path, std::uint32_t magic, std::uint16_t version);

}  // namespace ReliefOps::io
